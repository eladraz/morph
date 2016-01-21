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

#ifndef __TBA_CLR_FORMAT_METADATAHEADER_H
#define __TBA_CLR_FORMAT_METADATAHEADER_H

/*
 * metadataHeader.h
 *
 * The format description of the meta-data header
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/stream/stringerStream.h"
#include "xStl/stream/memoryAccesserStream.h"
#include "format/coreHeadersTypes.h"

/*
 * Forward deceleration for output streams
 */
#ifdef TRACED_CLR
class MetadataHeader;
cStringerStream& operator << (cStringerStream& out,
                              const MetadataHeader& object);
#endif // TRACED_CLR

/*
 * The MetadataHeader gets a basic-input stream and read the header of the
 * meta-data directory of the CLI file-format.
 *
 * Usage:
 *    cNtPrivateDirectory metaDirectory(...);
 *    // And read the header
 *    MetadataHeader metaHeader(*(metaDirectory.getData()->fork()));
 *
 * For stream accessing,
 *   See MetadataHeader::getStreamByName
 *   See MetadataHeader::getStreams
 *
 * NOTE: This module is thread-safe and can be accessed from different number
 *       of threads. The function 'getStreamByName' can also be accessed from
 *       multi-threads
 */
#pragma pack(push)
#pragma pack(1)
class MetadataHeader {
public:
    /*
     * Demarshled constructor. Parsed a stream and fill the class with the
     * information regarding to the meta-data header
     *
     * metadata  - The PE memory-accesser.
     * directory - The accessable size of the stream which contains the meta-
     *             data. All RVA are relative to 0 position of the stream!
     *
     * Throw exception if the meta-data format is invalid or the version
     * number is unknown.
     *
     * NOTE: The stream must support the intel little-endian notation, otherwise
     *       exception will be raised
     */
    MetadataHeader(const cMemoryAccesserStreamPtr& metadata,
                   const IMAGE_DATA_DIRECTORY& directory);

    /*
     * The deceleration of the header.
     *
     * NOTE: Use the MetadataHeader serialized function in order to read/write
     *       stream information
     */
    struct Header {
        // The signature of the meta-data-header. Constant value: 0x424A5342
        uint32 m_signature;
        // The version of the metadata header: 1.0
        uint16 m_majorVersion;
        uint16 m_minorVersion;
        // Reserved
        uint32 m_reserved;
        // The version string. UTF8 encoding (length+char*)
        cString m_version;
        // Reserved. Always 0
        uint16 m_flags;
        // The number of streams
        uint16 m_streams;
    };

    // The signature of the metadata blob. Notice how this number also check the
    // endian attributes of the stream
    enum { METADATA_HEADER_SIGNATURE = 0x424A5342 };

    /*
     * The deceleration of stream header.
     */
    struct StreamHeader {
        // The offset and the size of the stream
        IMAGE_DATA_DIRECTORY m_stream;
        // The name of the stream encoded as null-terminate ansi string
        cString m_name;
    };

    /*
     * Return the meta-data header
     */
    const Header& getMetadataHeader() const;

    /*
     * Return a list containing all streams inside the metadata
     */
    const cList<StreamHeader>& getStreams() const;

    /*
     * Get a stream by it's name
     *
     * name      - The name of stream
     * stream    - Will be filled with the stream pointer of a new constructed
     *             memory-accesser-stream. If the stream couldn't be found it
     *             will point to null-pointer object.
     * directory - Will be filled with the directory attributes.
     * shouldRelocate - Set to true in order to indicate that the offset of the
     *                  new stream should start at 0 offset. False will generate
     *                  a new stream which 0 is RVA of the metadata-header
     *
     * Return true if the stream exist and 'stream' and 'directory' are filled
     * with the relevant information.
     */
    bool getStreamByName(const cString& name,
                         cMemoryAccesserStreamPtr& stream,
                         IMAGE_DATA_DIRECTORY& directory,
                         bool shouldRelocate = true);

    /*
     * Return the first RVA address of this stream. Used for reveresed link
     * calculations...
     */
    addressNumericValue getClrStartAddress() const;

private:
    // Deny copy-constructor and operator =
    MetadataHeader(const MetadataHeader& other);
    MetadataHeader& operator = (const MetadataHeader& other);

    // The friendly trace
    #ifdef TRACED_CLR
   friend cStringerStream& operator << (cStringerStream& out,
                                         const MetadataHeader& object);
    #endif // TRACED_CLR

    // The header fields
    Header m_header;
    // The list of all streams in the system
    cList<StreamHeader> m_streams;

    // The metadata stream information
    cMemoryAccesserStreamPtr m_metaStream;
    // The known attributes of the new stream
    IMAGE_DATA_DIRECTORY m_directory;
};
#pragma pack(pop)

#endif // __TBA_CLR_FORMAT_METADATAHEADER_H
