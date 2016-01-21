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

#ifndef __TBA_CLR_FORMAT_METADATASTREAM_H
#define __TBA_CLR_FORMAT_METADATASTREAM_H

/*
 * metadataStream.h
 *
 * Parse the #~ of the CIL file-format. The metadata stream contains all class/
 * methods and execution context needed for the instruction-set and loading
 * order.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/stringerStream.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataHeader.h"

/*
 * Forward deceleration for output streams
 */
#ifdef TRACED_CLR
class MetadataStream;
cStringerStream& operator << (cStringerStream& out,
                              const MetadataStream& object);
#endif // TRACED_CLR

/*
 *
 * NOTE: This class implements the basicInput interface to allow all the table
 *       parser class to read information in the following way:
 *           FirstTable:
 *              metadataStream.streamReadUint16(bla);
 *              metadataStream.streamReadUint16(bla1);
 *              metadataStream.streamReadUint32(rva.offset);
 *              metadataStream.streamReadUint32(rva.size);
 *              metadataStream.readStringToken(name);
 *           SecondTable:
 *              metadataStream.readStringToken(name);
 *              metadataStream.readGuidToken(guid);
 *              metadataStream.readBlobToken(guid);
 *       This way the following tables can be deserialized in a more convinet
 *       way.
 *
 * TODO! Add CoreStreamParse inherit
 */
class MetadataStream : public basicInput {
public:
    // The different number of tables
    enum { NUMBER_OF_TABLES = 64 };

    /*
     * Default constructor. Load all information from the CLI file...
     * Using the 'header' of the CLI file to get the #~ stream and start reading
     * all tables.
     *
     * header - The CLI header. Must have #~ stream.
     *
     * Throw exception if the CLI file is corrupted or the tables cannot be
     * parsed by this stream
     */
    MetadataStream(MetadataHeader& header);

    /*
     * Return the number of tables for table ID. Return 0 if the table is not
     * valid.
     *
     * index - A number between 0 to 63 (NUMBER_OF_TABLES-1) of the table ID
     *
     * Throw exception if the table index is invalid
     */
    uint getNumberOfRows(uint index) const;

    // The internal struct used for the header of #~ stream
    struct Header {
        // Reserved must be 0
        uint32 m_reserved;
        // The header major version. Equal to 1
        uint8 m_majorVersion;
        // The header minor version. Equal to 0
        uint8 m_minorVersion;
        // Bit-vector which indicate the size of the different heaps. 1 means
        // larger than 64k, 0 means lower. Used for pointers size
        uint8 m_heapSizes;
        // Reserved must be 1
        uint8 m_reserved1;
        // Bit-vector of presents tables (64bit)
        uint32 m_validLow;
        uint32 m_validHigh;
        // Bit-vector of sorted tables (64bit)
        uint32 m_sortedLow;
        uint32 m_sortedHigh;
    };

    enum HeapSize {
        // Size of #String is greater or equal to 64kb
        HEAPSIZE_LARGE_STRING = 1,
        // Size of #GUID is greater or equal to 64kb
        HEAPSIZE_LARGE_GUID   = 2,
        // Size of #Blob is greater or equal to 64kb
        HEAPSIZE_LARGE_BLOB   = 4
    };

    /*
     * All the CLR based on little-endian implementation
     */
    littleEndianImpl;

    /*
     * All the following functions inherited from the basicInput interface and
     * implemented as a simple delegation to "m_metaStream"->function.
     *
     * All of these function might throw exception if the stream is invalid
     */
    virtual uint read(void *buffer, const uint length);
    virtual uint getPointer() const;
    virtual uint length() const;
    virtual uint getPipeReadBestRequest() const;
    virtual bool isEOS();
    virtual void seek(const int distance, const seekMethod method);

    /*
     * Read a 16bit/32bit value according to the string heap-size
     */
    mdToken readStringToken();

    /*
     * Read a 16bit/32bit value according to the Blob heap-size
     */
    mdToken readBlobToken();

    /*
     * Read a 16bit/32bit value according to the GUID heap-size
     */
    mdToken readGuidToken();

    /*
     * Read a 16bit/32bit value according to the table ID. For table which have
     * more than 65535 rows the token are encoded as 32bit number
     *
     * index - The table ID
     *
     * Throw exception if the table index is invalid
     */
    mdToken readTableToken(uint index);

    /*
     * Read a 16bit/32bit value according to the sizes of the Typedef/TypeRef or
     * TypeSpec tables.
     */
    mdToken readTypeDefOrRefToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the Fields/Param or
     * properties tables
     */
    mdToken readHasConstantToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the MethodDef or
     * MemberRef tables
     */
    mdToken readMethodDefOrRef();

    /*
     * Read a 16bit/32bit value according to the sizes of the TypedefDef or
     * MethodDef tables
     */
    mdToken readTypeOrMethodDef();

    /*
     * Read a 16bit/32bit value according to the sizes of the tables except
     * CustomAttribute table.
     */
    mdToken readHasCustomAttributeToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the MethodDef or
     * MemberRef tables.
     * TODO! Documentation error
     */
    mdToken readCustomAttributeTypeToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the TypeDef, Method
     * or Assembly tabels
     */
    mdToken readHasDeclSecurityToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the Module, ModuleRef
     * AssemblyRef or TypeRef tables
     */
    mdToken readResolutionScopeToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the TypeRef, ModuleRef
     * MethodDef or TypeSpec tables
     */
    mdToken readMemberRefParentToken();

    /*
     * Read a 16bit/32bit value according to the sizes of the Event, Property
     * tables
     */
    mdToken readHasHasSemanticsToken();

protected:
    // Default empty implementation of the initSeek. The seek function is not
    // needed here, since the seek implementation is delegation to
    // cMemoryAccesserStream implementation
    virtual void initSeek() {};

private:
    // The tracing function is the only friend of this class
    #ifdef TRACED_CLR
    friend cStringerStream& operator << (cStringerStream& out,
                                         const MetadataStream& object);
    #endif // TRACED_CLR


    // Deny operator = and copy-constructor
    MetadataStream(const MetadataStream& other);
    MetadataStream& operator = (const MetadataStream& other);

    /*
     * Return true if table 'index' is valid or sorted
     *
     * index - A number between 0 to 63 (NUMBER_OF_TABLES-1) of the table ID
     *
     * Throw exception if the table is invalid
     */
    bool isTableValid(uint index) const;
    bool isTableSorted(uint index) const;

    // The string which represent the metadata stream (#~)
    static const char gMetaStreamStringID[];

    // The stream of the meta-data from the beginning of the #~ stream
    cMemoryAccesserStreamPtr m_metaStream;
    // The directory for the meta-stream. Used for RVA
    IMAGE_DATA_DIRECTORY m_metaDir;

    // Cached value of the m_header.m_heapSizes
    bool m_largeString;
    bool m_largeGUID;
    bool m_largeBlob;

    // The header of the #~ stream
    Header m_header;
    // The number of rows for each table. 0 means that the table is invalid
    uint32 m_rows[NUMBER_OF_TABLES];
};

#endif // __TBA_CLR_FORMAT_METADATASTREAM_H
