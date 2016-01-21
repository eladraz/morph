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
 * ParamTable.cpp
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
#include "format/tables/TablesID.h"
#include "format/tables/ParamTable.h"

ParamTable::ParamTable(MetadataStream& stream,
                       mdToken token) :
    Table(token)
{
    stream.streamReadUint16(m_header.m_flags);
    stream.streamReadUint16(m_header.m_sequence);
    m_header.m_name = stream.readStringToken();
}

TablesID ParamTable::getIndex() const
{
    return TABLE_PARAM_TABLE;
}

mdToken ParamTable::getName(void) const
{
    return m_header.m_name;
}

#ifdef TRACED_CLR
void ParamTable::out(cStringerStream& out,
                     StringsTableFormatter& formatter) const
{
    out << "Flags:      " << HEXWORD(m_header.m_flags) << "   ";

    if ((m_header.m_flags & pdIn) != 0)
        out << "[in] ";
    if ((m_header.m_flags & pdOut) != 0)
        out << "[out] ";
    if ((m_header.m_flags & pdOptional) != 0)
        out << "optional ";
    if ((m_header.m_flags & pdHasDefault) != 0)
        out << "has-default ";
    if ((m_header.m_flags & pdHasFieldMarshal) != 0)
        out << "field-marshal ";
    out << endl;


    out << "Sequence:   " << HEXWORD(m_header.m_sequence) << endl;
    out << "Name:       " << formatter.getString(m_header.m_name) << endl;
}

cString ParamTable::getTableName() const
{
    return "ParamTable";
}

cString ParamTable::getTableMSILname() const
{
    return ".param";
}
#endif // TRACED_CLR

