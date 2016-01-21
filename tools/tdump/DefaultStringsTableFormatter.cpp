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
 * This module is only good if TRACED CLR is defined!
 */
#ifdef TRACED_CLR

/*
 * DefaultStringsTableFormatter.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/utils/dumpMemory.h"
#include "xStl/except/trace.h"
#include "xStl/stream/stringerStream.h"
#include "pe/datastruct.h"
#include "format/MSILStreams.h"
#include "format/coreHeadersTypes.h"
#include "DefaultStringsTableFormatter.h"


DefaultStringsTableFormatter::DefaultStringsTableFormatter(MSILStreams& streams) :
    m_streams(streams)
{
}

cString DefaultStringsTableFormatter::getString(mdToken strID) const
{
    cMemoryAccesserStreamPtr strings = m_streams.getStringsStream();
    strings->seek(strID, basicInput::IO_SEEK_SET);
    return strings->readAsciiNullString(MAX_STRING);
}

cString DefaultStringsTableFormatter::getGuidString(mdToken guidID) const
{
    if (guidID == 0)
        return "NO GUID";

    // Read the GUID
    GUID gGuid;
    cMemoryAccesserStreamPtr guids = m_streams.getGUIDStream();
    guids->seek((guidID-1) * GUID_SIZE, basicInput::IO_SEEK_SET);
    guids->pipeRead(&gGuid, GUID_SIZE);

    // Format the GUID
    cString ret(HEXDWORD(gGuid.Data1)); ret+= "-";
    ret+= HEXWORD(gGuid.Data2); ret+= "-";
    ret+= HEXWORD(gGuid.Data3); ret+= "-";
    for (uint i = 0; i < 8; i++)
        ret+= HEXBYTE(gGuid.Data4[i]);

    return ret;
}

#endif // TRACED_CLR
