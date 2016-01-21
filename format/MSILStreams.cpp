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
 * MSILStreams.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "pe/datastruct.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataHeader.h"
#include "format/MSILStreams.h"

const char MSILStreams::gStringsStreamStringID[] = "#Strings";
const char MSILStreams::gGUIDStreamStringID[] = "#GUID";
const char MSILStreams::gBlobStreamStringID[] = "#Blob";
const char MSILStreams::gUSStreamStringID[] = "#US";

MSILStreams::MSILStreams(MetadataHeader& header) :
    m_isUserString(false)
{
    // Read the strings
    CHECK(header.getStreamByName(MSILStreams::gStringsStreamStringID,
          m_stringStream,
          m_stringDirectory));
    CHECK(header.getStreamByName(MSILStreams::gGUIDStreamStringID,
          m_guidStream,
          m_guidDirectory));
    CHECK(header.getStreamByName(MSILStreams::gBlobStreamStringID,
          m_blobStream,
          m_blobDirectory));

    m_isUserString = header.getStreamByName(MSILStreams::gUSStreamStringID,
          m_userStringsStream,
          m_userStringsDirectory);
}

const cMemoryAccesserStreamPtr& MSILStreams::getBlobStream() const
{
    return m_blobStream;
}

const cMemoryAccesserStreamPtr& MSILStreams::getStringsStream() const
{
    return m_stringStream;
}

const cMemoryAccesserStreamPtr& MSILStreams::getUserStringsStream() const
{
    CHECK(m_isUserString);
    return m_userStringsStream;
}

bool MSILStreams::isUserStringsExist() const
{
    return m_isUserString;
}

const cMemoryAccesserStreamPtr& MSILStreams::getGUIDStream() const
{
    return m_guidStream;
}
