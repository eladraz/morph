/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

/*
 * metadataStream.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/utils/dumpMemory.h"
#include "xStl/except/trace.h"
#include "xStl/stream/memoryAccesserStream.h"
#include "format/metadataHeader.h"
#include "format/metadataStream.h"
#include "format/tables/TablesID.h"

const char MetadataStream::gMetaStreamStringID[] = "#~";

MetadataStream::MetadataStream(MetadataHeader& header) :
    m_largeString(false),
    m_largeGUID(false),
    m_largeBlob(false)
{
    // Reset the rows
    uint i;
    for (i = 0; i < NUMBER_OF_TABLES; i++)
        m_rows[i] = 0;

    // Generate new stream
    CHECK(header.getStreamByName(gMetaStreamStringID,
                                 m_metaStream,
                                 m_metaDir));

    // Parse the stream
    m_metaStream->streamReadUint32(m_header.m_reserved);
    m_metaStream->streamReadUint8(m_header.m_majorVersion);
    m_metaStream->streamReadUint8(m_header.m_minorVersion);
    m_metaStream->streamReadUint8(m_header.m_heapSizes);
    m_metaStream->streamReadUint8(m_header.m_reserved1);

    m_largeString = ((m_header.m_heapSizes & HEAPSIZE_LARGE_STRING) != 0);
    m_largeGUID = ((m_header.m_heapSizes & HEAPSIZE_LARGE_GUID) != 0);
    m_largeBlob = ((m_header.m_heapSizes & HEAPSIZE_LARGE_BLOB) != 0);

    // Start reading tables
    m_metaStream->streamReadUint32(m_header.m_validLow);
    m_metaStream->streamReadUint32(m_header.m_validHigh);
    m_metaStream->streamReadUint32(m_header.m_sortedLow);
    m_metaStream->streamReadUint32(m_header.m_sortedHigh);

    // Read the number of rows
    for (i = 0; i < NUMBER_OF_TABLES; i++)
    {
        if (isTableValid(i))
            m_metaStream->streamReadUint32(m_rows[i]);
    }
}

uint MetadataStream::getNumberOfRows(uint index) const
{
    CHECK(index < NUMBER_OF_TABLES);
    return m_rows[index];
}

bool MetadataStream::isTableValid(uint index) const
{
    CHECK(index < NUMBER_OF_TABLES);

    // Check the lowest 32bit
    if (index < 32)
        return (m_header.m_validLow & (1 << index)) != 0;

    // Check the highest 32bit
    return (m_header.m_validHigh & (1 << (index - 32))) != 0;
}

bool MetadataStream::isTableSorted(uint index) const
{
    CHECK(index < NUMBER_OF_TABLES);

    // Check the lowest 32bit
    if (index < 32)
        return (m_header.m_sortedLow & (1 << index)) != 0;

    // Check the highest 32bit
    return (m_header.m_sortedHigh & (1 << (index - 32))) != 0;
}

//////////////////////////////////////////////////////////////////////////
// basicInput interface implementation

uint MetadataStream::read(void *buffer, const uint length)
{
    return m_metaStream->read(buffer, length);
}

uint MetadataStream::getPointer() const
{
    return m_metaStream->getPointer();
}

uint MetadataStream::length() const
{
    return m_metaStream->length();
}

uint MetadataStream::getPipeReadBestRequest() const
{
    return m_metaStream->getPipeReadBestRequest();
}

bool MetadataStream::isEOS()
{
    return m_metaStream->isEOS();
}

void MetadataStream::seek(const int distance, const seekMethod method)
{
    m_metaStream->seek(distance, method);
}

// MetadataStream private IO interface

mdToken MetadataStream::readStringToken()
{
    if (m_largeString)
    {
        uint32 index;
        m_metaStream->streamReadUint32(index);
        return index;
    } else
    {
        uint16 index;
        m_metaStream->streamReadUint16(index);
        return index;
    }
}

mdToken MetadataStream::readGuidToken()
{
    if (m_largeGUID)
    {
        uint32 index;
        m_metaStream->streamReadUint32(index);
        return index;
    } else
    {
        uint16 index;
        m_metaStream->streamReadUint16(index);
        return index;
    }
}

mdToken MetadataStream::readBlobToken()
{
    if (m_largeBlob)
    {
        uint32 index;
        m_metaStream->streamReadUint32(index);
        return index;
    } else
    {
        uint16 index;
        m_metaStream->streamReadUint16(index);
        return index;
    }
}

mdToken MetadataStream::readTableToken(uint index)
{
    if (getNumberOfRows(index) > 0xFFFF)
    {
        uint32 ret;
        m_metaStream->streamReadUint32(ret);
        return ret;
    } else
    {
        uint16 ret;
        m_metaStream->streamReadUint16(ret);
        return ret;
    }
}

// The different cached reading values
enum CachedTableSizes {
    TABLE_UNKNOWN_SIZE,
    TABLE_32_BIT,
    TABLE_16_BIT
};

mdToken MetadataStream::readTypeDefOrRefToken()
{
    // Cache reading
    static CachedTableSizes gTypeDefOrRefSize = TABLE_UNKNOWN_SIZE;
    if (gTypeDefOrRefSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_TYPEDEF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPESPEC_TABLE) > 0xFFFF))
            gTypeDefOrRefSize = TABLE_32_BIT;
        else
            gTypeDefOrRefSize = TABLE_16_BIT;
    }

    // Get the table size
    if (gTypeDefOrRefSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readHasConstantToken()
{
    // Cache reading
    static CachedTableSizes gHasConstantSize = TABLE_UNKNOWN_SIZE;
    if (gHasConstantSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_FIELD_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_PARAM_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_PROPERTY_TABLE) > 0xFFFF))
            gHasConstantSize = TABLE_32_BIT;
        else
            gHasConstantSize = TABLE_16_BIT;
    }

    if (gHasConstantSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readMethodDefOrRef()
{
    // Cache reading
    static CachedTableSizes gMethodDefOrRefSize = TABLE_UNKNOWN_SIZE;
    if (gMethodDefOrRefSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_MEMBERREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_METHOD_TABLE) > 0xFFFF))
            gMethodDefOrRefSize = TABLE_32_BIT;
        else
            gMethodDefOrRefSize = TABLE_16_BIT;
    }

    if (gMethodDefOrRefSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readTypeOrMethodDef()
{
    // Cache reading
    static CachedTableSizes gTypeOrMethodDefSize = TABLE_UNKNOWN_SIZE;
    if (gTypeOrMethodDefSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_TYPEDEF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_METHOD_TABLE) > 0xFFFF))
            gTypeOrMethodDefSize = TABLE_32_BIT;
        else
            gTypeOrMethodDefSize = TABLE_16_BIT;
    }

    if (gTypeOrMethodDefSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readHasHasSemanticsToken()
{
    // Cache reading
    static CachedTableSizes gHasHasSemanticsSize = TABLE_UNKNOWN_SIZE;
    if (gHasHasSemanticsSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_PROPERTY_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_EVENT_TABLE) > 0xFFFF))
            gHasHasSemanticsSize = TABLE_32_BIT;
        else
            gHasHasSemanticsSize = TABLE_16_BIT;
    }

    if (gHasHasSemanticsSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readHasCustomAttributeToken()
{
    // Cache reading
    static CachedTableSizes gHasCustomAttributeSize = TABLE_UNKNOWN_SIZE;
    if (gHasCustomAttributeSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_METHOD_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_FIELD_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPEDEF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_PARAM_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_INTERFACEIMPL_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MEMBERREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MODULE_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_DECLSECURITY_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_PROPERTY_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_EVENT_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_STANDALONGESIG_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MODULEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPESPEC_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_ASSEMBLY_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_ASSEMBLYREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_FILE_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_EXPORTTYPE_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MANIFESTRESOURCE_TABLE) > 0xFFFF))
            gHasCustomAttributeSize = TABLE_32_BIT;
        else
            gHasCustomAttributeSize = TABLE_16_BIT;
    }

    if (gHasCustomAttributeSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readCustomAttributeTypeToken()
{
    // TODO! Documentation error
    return readMethodDefOrRef();
}

mdToken MetadataStream::readHasDeclSecurityToken()
{
    // Cache reading
    static CachedTableSizes gHasDeclSecuritySize = TABLE_UNKNOWN_SIZE;
    if (gHasDeclSecuritySize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_TYPEDEF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_ASSEMBLY_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_METHOD_TABLE) > 0xFFFF))
            gHasDeclSecuritySize = TABLE_32_BIT;
        else
            gHasDeclSecuritySize = TABLE_16_BIT;
    }

    if (gHasDeclSecuritySize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readResolutionScopeToken()
{
    // Cache reading
    static CachedTableSizes gResolutionScopeSize = TABLE_UNKNOWN_SIZE;
    if (gResolutionScopeSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_MODULE_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MODULEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_ASSEMBLYREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPEREF_TABLE) > 0xFFFF))
            gResolutionScopeSize = TABLE_32_BIT;
        else
            gResolutionScopeSize = TABLE_16_BIT;
    }

    if (gResolutionScopeSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

mdToken MetadataStream::readMemberRefParentToken()
{
    // Cache reading
    static CachedTableSizes gMemberRefParentSize = TABLE_UNKNOWN_SIZE;
    if (gMemberRefParentSize == TABLE_UNKNOWN_SIZE)
    {
        if ((getNumberOfRows(TABLE_TYPEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_MODULEREF_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_METHOD_TABLE) > 0xFFFF) ||
            (getNumberOfRows(TABLE_TYPESPEC_TABLE) > 0xFFFF))
            gMemberRefParentSize = TABLE_32_BIT;
        else
            gMemberRefParentSize = TABLE_16_BIT;
    }

    if (gMemberRefParentSize == TABLE_32_BIT)
    {
        uint32 ret; m_metaStream->streamReadUint32(ret); return ret;
    } else
    {
        uint16 ret; m_metaStream->streamReadUint16(ret); return ret;
    }
}

//////////////////////////////////////////////////////////////////////////
// Trace function

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const MetadataStream& object)
{
    out << "Metadata" << endl;
    out << "========" << endl;
    out << "Version:   " << (uint32)(object.m_header.m_majorVersion) << "."
                         << (uint32)(object.m_header.m_minorVersion) << endl;

    if (object.m_largeString)
        out << "#Strings contains large pointers" << endl;
    if (object.m_largeGUID)
        out << "#Strings contains large pointers" << endl;
    if (object.m_largeBlob)
        out << "#Strings contains large pointers" << endl;

    /*
    for (uint i = 0; i < MetadataStream::NUMBER_OF_TABLES; i++)
    {
        if (object.isTableValid(i))
            out << "Table " << i << " is valid..  "
                << HEXDWORD(object.m_rows[i]) << endl;
    }

    out << endl << endl;
    out << DumpMemory(*(object.m_metaStream->getMemoryAccesser()),
                     object.m_metaStream->getMemoryAccesserStartAddress(),
                     object.m_metaStream->getMemoryAccesserEndAddress(),

                     object.m_metaStream->getMemoryAccesserStartAddress() -
                     object.m_metaDir.VirtualAddress)
        << endl;
    */

    return out;
}
#endif // TRACED_CLR
