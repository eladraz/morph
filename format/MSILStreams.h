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

#ifndef __TBA_CLR_FORMAT_MSILSTREAMS_H
#define __TBA_CLR_FORMAT_MSILSTREAMS_H

/*
 * MSILStreams.h
 *
 * This module contains all streams pointers which aren't cached:
 *   #Blob, #GUID, #Strings
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "pe/datastruct.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataHeader.h"

/*
 * NOTE: This module is not thread-safe and must be constructed for each stream
 */
class MSILStreams {
public:
    /*
     * Constructor. From the MSIL header instance pointers into the following
     * streams:
     *     - Blob
     *     - GUID
     *     - Strings
     *     - User-String [Optional]
     */
    MSILStreams(MetadataHeader& header);

    /*
     * Return a pointer to #Blob stream, where 0 is the beginning of the stream
     *
     * NOTE: The pointer is share between all function which call this function.
     *       So, only one thread can access in a time to this stream (Unless the
     *       stream is forked, of-course). Another thing, the stream IO pointer
     *       is random and depends on the previous functions call
     */
    const cMemoryAccesserStreamPtr& getBlobStream() const;

    /*
     * Return a pointer to #Strings stream, where 0 is the beginning of the stream
     *
     * NOTE: The pointer is share between all function which call this function.
     *       So, only one thread can access in a time to this stream (Unless the
     *       stream is forked, of-course). Another thing, the stream IO pointer
     *       is random and depends on the previous functions call
     */
    const cMemoryAccesserStreamPtr& getStringsStream() const;

    /*
     * Return a pointer to #US stream, where 0 is the beginning of the stream
     *
     * Throw exception if the #US stream not exist in the file.
     * See isUserStringsExist()
     *
     * NOTE: The pointer is share between all function which call this function.
     *       So, only one thread can access in a time to this stream (Unless the
     *       stream is forked, of-course). Another thing, the stream IO pointer
     *       is random and depends on the previous functions call
     */
    const cMemoryAccesserStreamPtr& getUserStringsStream() const;

    /*
     * Return true if the #US exist in the file, false otherwise
     */
    bool isUserStringsExist() const;

    /*
     * Return a pointer to #GUID stream, where 0 is the beginning of the stream
     *
     * NOTE: The pointer is share between all function which call this function.
     *       So, only one thread can access in a time to this stream (Unless the
     *       stream is forked, of-course). Another thing, the stream IO pointer
     *       is random and depends on the previous functions call
     */
    const cMemoryAccesserStreamPtr& getGUIDStream() const;

    // The name #Strings
    static const char gStringsStreamStringID[];
    // The name #GUID
    static const char gGUIDStreamStringID[];
    // The name #Blob
    static const char gBlobStreamStringID[];
    // The name #US
    static const char gUSStreamStringID[];
private:
    // Deny copy-constructor and operator =
    MSILStreams(const MSILStreams& other);
    MSILStreams& operator = (const MSILStreams& other);

    // The stream for the #Strings
    mutable cMemoryAccesserStreamPtr m_stringStream;
    IMAGE_DATA_DIRECTORY m_stringDirectory;
    // The stream for the #GUID
    mutable cMemoryAccesserStreamPtr m_guidStream;
    IMAGE_DATA_DIRECTORY m_guidDirectory;
    // The stream for the #Blob
    mutable cMemoryAccesserStreamPtr m_blobStream;
    IMAGE_DATA_DIRECTORY m_blobDirectory;
    // The stream for the #US
    bool m_isUserString;
    mutable cMemoryAccesserStreamPtr m_userStringsStream;
    IMAGE_DATA_DIRECTORY m_userStringsDirectory;
};

#endif // __TBA_CLR_FORMAT_MSILSTREAMS_H
