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
 * MetadataHeader.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "format/metadataHeader.h"

MetadataHeader::MetadataHeader(const cMemoryAccesserStreamPtr& metadata,
                               const IMAGE_DATA_DIRECTORY& directory) :
    m_metaStream(metadata->forkRegion(0,
                                      directory.Size)),
    m_directory(directory)
{
    // Read the signature
    m_metaStream->streamReadUint32(m_header.m_signature);
    CHECK(m_header.m_signature == METADATA_HEADER_SIGNATURE);
    // Read the version
    m_metaStream->streamReadUint16(m_header.m_majorVersion);
    m_metaStream->streamReadUint16(m_header.m_minorVersion);

    m_metaStream->streamReadUint32(m_header.m_reserved);

    // Read the version string. UTF-8 encoding. Length is 32bit!!
    uint32 versionStringLength;
    m_metaStream->streamReadUint32(versionStringLength);
    m_header.m_version =
        m_metaStream->readFixedSizeString(versionStringLength, 1);

    m_metaStream->streamReadUint16(m_header.m_flags);
    m_metaStream->streamReadUint16(m_header.m_streams);

    // Read all streams in the header
    for (uint i = 0; i < m_header.m_streams; i++)
    {
        StreamHeader newHeader;
        // Read the image-directory
        m_metaStream->pipeReadNotEos(&newHeader.m_stream,
                                sizeof(newHeader.m_stream));
        // Read the stream name
        newHeader.m_name = m_metaStream->readAsciiNullString();

        // Read the padding
        cBuffer temp;
        uint mod = ((newHeader.m_name.length() + 1) % (sizeof(uint32)));
        if (mod != 0)
            m_metaStream->pipeRead(temp, sizeof(uint32) - mod);

        // And append the stream into the linked-list
        m_streams.append(newHeader);
    }
}

const MetadataHeader::Header& MetadataHeader::getMetadataHeader() const
{
    return m_header;
}

const cList<MetadataHeader::StreamHeader>& MetadataHeader::getStreams() const
{
    return m_streams;
}

bool MetadataHeader::getStreamByName(const cString& name,
                                     cMemoryAccesserStreamPtr& stream,
                                     IMAGE_DATA_DIRECTORY& directory,
                                     bool shouldRelocate)
{
    cList<MetadataHeader::StreamHeader>::iterator i = m_streams.begin();
    for (; i != m_streams.end(); ++i)
    {
        if ((*i).m_name.icompare(name) == cString::EqualTo)
        {
            directory = (*i).m_stream;
            if (shouldRelocate)
            {
                stream = m_metaStream->forkRegion(
                    directory.VirtualAddress,
                    directory.VirtualAddress + directory.Size);
            } else
            {
                stream = m_metaStream->forkRegion(0, m_directory.Size);
                stream->seek(directory.VirtualAddress, basicInput::IO_SEEK_SET);
            }
            return true;
        }
    }

    // Cannot find the stream
    stream = cMemoryAccesserStreamPtr(NULL);
    return false;
}

addressNumericValue MetadataHeader::getClrStartAddress() const
{
    return m_metaStream->getMemoryAccesserStartAddress();
}


#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const MetadataHeader& object)
{
    out << "Metadata Header" << endl;
    out << "===============" << endl;
    out << "Signature:     " << HEXDWORD(object.m_header.m_signature) << endl;
    out << "Version:       " << (uint32)object.m_header.m_majorVersion << "."
                             << (uint32)object.m_header.m_minorVersion << endl;
    out << "               " << object.m_header.m_version << endl;
    out << "Flags:         " << object.m_header.m_flags << endl;

    cList<MetadataHeader::StreamHeader>::iterator i =
        object.m_streams.begin();
    for (; i != object.m_streams.end(); ++i)
    {
        out << "    " << cString::pad((*i).m_name, 20)
            << "   Start: " << HEXDWORD((*i).m_stream.VirtualAddress)
            << "   Size:  " << HEXNUMBER((*i).m_stream.Size) << endl;
    }
    return out;
}
#endif // TRACED_CLR
