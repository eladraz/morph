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
 * FieldRVATable.cpp
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
#include "format/tables/FieldRVATable.h"

FieldRVATable::FieldRVATable(MetadataStream& stream,
                       mdToken token) :
    Table(token)
{
    // Start reading the table
    stream.streamReadUint32(m_header.m_fieldRva);
    m_header.m_field = EncodingUtils::buildToken(TABLE_FIELD_TABLE,
                            stream.readTableToken(TABLE_FIELD_TABLE));
}

TablesID FieldRVATable::getIndex() const
{
    return TABLE_FIELDRVA_TABLE;
}

const FieldRVATable::Header& FieldRVATable::getHeader() const
{
    return m_header;
}

#ifdef TRACED_CLR
void FieldRVATable::out(cStringerStream& out,
                     StringsTableFormatter& formatter) const
{
    out << "Field RVA: " << HEXWORD(m_header.m_fieldRva) << endl;
    out << "Field:     " << HEXWORD(m_header.m_field) << endl;
}

cString FieldRVATable::getTableName() const
{
    return "FieldRVATable";
}

cString FieldRVATable::getTableMSILname() const
{
    return ".fieldRva";
}
#endif // TRACED_CLR
