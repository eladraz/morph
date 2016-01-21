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
 * TypedefTable.cpp
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
#include "format/tables/TypedefTable.h"

TypedefTable::TypedefTable(MetadataStream& stream,
                           mdToken token) :
    Table(token)
{
    stream.streamReadUint32(m_header.m_flags);
    m_header.m_name = stream.readStringToken();
    m_header.m_namespace = stream.readStringToken();

    // Read TypeDefOrRef index
    m_header.m_extends = EncodingUtils::unpackTypeDefOrRefToken(
                                    stream.readTypeDefOrRefToken());
    // Index into Filed-table
    m_header.m_fields = EncodingUtils::buildToken(TABLE_FIELD_TABLE,
                            stream.readTableToken(TABLE_FIELD_TABLE));
    // Index into method table
    m_header.m_methods = EncodingUtils::buildToken(TABLE_METHOD_TABLE,
                             stream.readTableToken(TABLE_METHOD_TABLE));
}

TablesID TypedefTable::getIndex() const
{
    return TABLE_TYPEDEF_TABLE;
}

const TypedefTable::Header& TypedefTable::getHeader() const
{
    return m_header;
}

mdToken TypedefTable::calculateEndMethodToken(const MetadataTables& tables) const
{
    if (tables.getNumberOfRows(TABLE_TYPEDEF_TABLE) ==
        EncodingUtils::getTokenPosition(getToken()))
    {
        // The last method is the end for this typedef
        return EncodingUtils::buildToken(TABLE_METHOD_TABLE,
                  tables.getNumberOfRows(TABLE_METHOD_TABLE) + 1);
    } else
    {
        // Get the next table (Which is the current token + 1)
        TablePtr nextTable = tables.getTableByToken(getToken() + 1);
        return ((TypedefTable&)(*nextTable)).getHeader().m_methods;
    }
}

mdToken TypedefTable::calculateEndFieldToken(const MetadataTables& tables) const
{
    if (tables.getNumberOfRows(TABLE_TYPEDEF_TABLE) ==
        EncodingUtils::getTokenPosition(getToken()))
    {
        // The last method is the end for this typedef
        return EncodingUtils::buildToken(TABLE_FIELD_TABLE,
                  tables.getNumberOfRows(TABLE_FIELD_TABLE) + 1);
    } else
    {
        // Get the next table (Which is the current token + 1)
        TablePtr nextTable = tables.getTableByToken(getToken() + 1);
        return ((TypedefTable&)(*nextTable)).getHeader().m_fields;
    }
}

#ifdef TRACED_CLR
void TypedefTable::out(cStringerStream& out,
                       StringsTableFormatter& formatter) const
{
    out << "Flags:         " << HEXDWORD(m_header.m_flags) << "   "; //
    if ((m_header.m_flags & tdInterface) != 0)
        out << "interface ";
    else
        out << "class ";


    if ((m_header.m_flags & tdSpecialName) != 0)
        out << "special ";
    if ((m_header.m_flags & tdBeforeFieldInit) != 0)
        out << "init ";
    if ((m_header.m_flags & tdUnicodeClass) != 0)
        out << "unicode ";
    else
        out << "ansi ";
    if ((m_header.m_flags & tdAbstract) != 0)
        out << "abstract ";
    if ((m_header.m_flags & tdSealed) != 0)
        out << "seal ";
    if ((m_header.m_flags & tdSequentialLayout) != 0)
        out << "sequential ";

    out << endl;

    /*
    if ((m_header.m_flags & tdVisibilityMask) != 0)
        out << "tdVisibilityMask  ";
    if ((m_header.m_flags & tdPublic) != 0)
        out << "tdPublic  ";
    if ((m_header.m_flags & tdNestedPublic) != 0)
        out << "tdNestedPublic  ";
    if ((m_header.m_flags & ) != 0)
        out << "  ";
        /**/

    out << "Name:          " << formatter.getString(m_header.m_name) << endl;
    out << "Namespace:     " << formatter.getString(m_header.m_namespace)
        << endl;
    out << "Extends:       " << HEXDWORD(m_header.m_extends) << endl;
    out << "Fields:        " << HEXDWORD(m_header.m_fields) << endl;
    out << "Methods:       " << HEXDWORD(m_header.m_methods) << endl;
}

cString TypedefTable::getTableName() const
{
    return "TypedefTable";
}

cString TypedefTable::getTableMSILname() const
{
    return ".typedef";
}
#endif // TRACED_CLR

mdToken TypedefTable::getMethodList(void) const
{
    return m_header.m_methods;
}

mdToken TypedefTable::getName(void) const
{
    return m_header.m_name;
}
