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
 * MethodTable.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataStream.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/MethodTable.h"

MethodTable::MethodTable(MetadataStream& stream,
                         mdToken token) :
    Table(token)
{
    // Start reading the table
    stream.streamReadUint32(m_header.m_rva);
    stream.streamReadUint16(m_header.m_implementationFlags);
    stream.streamReadUint16(m_header.m_flags);
    m_header.m_name = stream.readStringToken();
    m_header.m_signature = stream.readBlobToken();
    m_header.m_params = EncodingUtils::buildToken(TABLE_PARAM_TABLE,
                            stream.readTableToken(TABLE_PARAM_TABLE));
}

const MethodTable::Header& MethodTable::getHeader() const
{
    return m_header;
}

TablesID MethodTable::getIndex() const
{
    return TABLE_METHOD_TABLE;
}

mdToken MethodTable::calculateEndParamToken(MetadataTables& tables) const
{
    if (tables.getNumberOfRows(TABLE_METHOD_TABLE) ==
              EncodingUtils::getTokenPosition(getToken()))
    {
        // The last method is the end for this typedef
        return EncodingUtils::buildToken(TABLE_PARAM_TABLE,
                  tables.getNumberOfRows(TABLE_PARAM_TABLE) + 1);
    } else
    {
        // Get the next table (Which is the current token + 1)
        TablePtr nextTable = tables.getTableByToken(getToken() + 1);
        return ((MethodTable&)(*nextTable)).getHeader().m_params;
    }
}

#ifdef TRACED_CLR
void MethodTable::out(cStringerStream& out,
                      StringsTableFormatter& formatter) const
{
    out << "Flags:      " << HEXWORD(m_header.m_flags) << "   ";

    switch (m_header.m_flags & mdMemberAccessMask)
    {
    case mdPrivateScope: out << "privatescope "; break;
    case mdPrivate:      out << "private "; break;
    case mdFamANDAssem:  out << "pubFamilyAndAssembly"; break;
    case mdAssembly:     out << "pubAssembly "; break;
    case mdFamily:       out << "pubFamily "; break;
    case mdFamORAssem:   out << "pubFamilyOrAssembly"; break;
    case mdPublic:       out << "public "; break;
    }

    if ((m_header.m_flags & mdStatic) != 0)
        out << "static ";
    if ((m_header.m_flags & mdFinal) != 0)
        out << "final ";
    if ((m_header.m_flags & mdVirtual) != 0)
        out << "virtual ";
    if ((m_header.m_flags & mdAbstract) != 0)
        out << "abstract ";
    if ((m_header.m_flags & mdSpecialName) != 0)
        out << "specialname ";
    if ((m_header.m_flags & mdPinvokeImpl) != 0)
        out << "pinvokeimpl ";
    if ((m_header.m_flags & mdNewSlot) != 0)
        out << "newslot ";
    out << endl;

    out << "Impl:       " << HEXWORD(m_header.m_implementationFlags) << "   ";
    switch (m_header.m_implementationFlags & miCodeTypeMask)
    {
    case miIL:       out << "IL "; break;
    case miNative:   out << "Native "; break;
    case miOPTIL:    out << "OPTIL "; break;
    case miRuntime:  out << "Runtime "; break;
    }

    if ((m_header.m_implementationFlags & miUnmanaged) != 0)
        out << "unmanaged ";
    if ((m_header.m_implementationFlags & miSynchronized) != 0)
        out << "synchronized ";
    out << endl;

    out << "RVA:        " << HEXDWORD(m_header.m_rva) << endl;
    out << "Name:       " << formatter.getString(m_header.m_name) << endl;
    out << "Signature:  " << HEXDWORD(m_header.m_signature) << endl;
    out << "Params:     " << HEXDWORD(m_header.m_params) << endl;
}

cString MethodTable::getTableName() const
{
    return "MethodTable";
}

cString MethodTable::getTableMSILname() const
{
    return ".method";
}
#endif // TRACED_CLR

mdToken MethodTable::getName(void) const
{
    return m_header.m_name;
}

mdToken MethodTable::getParamListIndex(void) const
{
    return m_header.m_params;
}