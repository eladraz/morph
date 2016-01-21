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
 * NestedClassTable.cpp
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
#include "format/tables/NestedClassTable.h"

NestedClassTable::NestedClassTable(MetadataStream& stream,
                                   mdToken token) :
    Table(token)
{
    m_header.m_nestedClass = stream.readTableToken(TABLE_TYPEDEF_TABLE);
    m_header.m_enclosingClass = stream.readTableToken(TABLE_TYPEDEF_TABLE);
}

TablesID NestedClassTable::getIndex() const
{
    return TABLE_NESTEDCLASS_TABLE;
}

const NestedClassTable::Header& NestedClassTable::getHeader() const
{
    return m_header;
}

#ifdef TRACED_CLR
void NestedClassTable::out(cStringerStream& out,
                       StringsTableFormatter& formatter) const
{
    out << "Nested class:    " << HEXDWORD(m_header.m_nestedClass) << endl;
    out << "Enclosing class: " << HEXDWORD(m_header.m_enclosingClass) << endl;
}

cString NestedClassTable::getTableName() const
{
    return "NestedClassTable";
}

cString NestedClassTable::getTableMSILname() const
{
    return "";
}
#endif // TRACED_CLR

