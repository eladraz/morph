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
 * MetadataTables.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/except/trace.h"
#include "format/coreHeadersTypes.h"
#include "format/EncodingUtils.h"
#include "format/metadataHeader.h"
#include "format/metadataStream.h"
#include "format/tables/Table.h"
#include "format/tables/TableFactory.h"
#include "format/tables/TablesID.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/MetadataTables.h"

MetadataTables::MetadataTables(MetadataHeader& metaHeader) :
    // Read the #~ stream
    m_metaStream(metaHeader)
{
    // Reads the tables
    for (uint j = 0; j < MetadataStream::NUMBER_OF_TABLES; j++)
    {
        uint size = m_metaStream.getNumberOfRows(j);
        m_tables[j].changeSize(size);
        for (uint i = 0; i < size; i++)
        {
            m_tables[j][i] = TableFactory::readTable(m_metaStream, j, i);
        }
    }
}

const TablePtr& MetadataTables::getTableByToken(mdToken token) const
{
    uint id = EncodingUtils::getTokenTableIndex(token);
    uint position = EncodingUtils::getTokenPosition(token);

    CHECK(position > 0);
    position--;

    // Some checking
    CHECK(id < MetadataStream::NUMBER_OF_TABLES);

    if (position >= m_tables[id].getSize())
    {
        CHECK_FAIL();
    }


    return m_tables[id][position];
}

uint MetadataTables::getNumberOfRows(uint indexID) const
{
    return m_metaStream.getNumberOfRows(indexID);
}

mdToken MetadataTables::getTypedefParent(mdToken token) const
{
    uint id = EncodingUtils::getTokenTableIndex(token);
    uint position = EncodingUtils::getTokenPosition(token);
    CHECK(position > 0);
    position--;

    if (id == TABLE_MEMBERREF_TABLE)
    {
        return ((const MemberRefTable&)(*getTableByToken(token))).getHeader().m_class;
    }

    if ((id != TABLE_FIELD_TABLE) && (id != TABLE_METHOD_TABLE))
        // Unknown token ID
        CHECK_FAIL();

    // Get Typedef by field/method

    // Scan all typedef tables
    uint typedefTablesSize = getNumberOfRows(TABLE_TYPEDEF_TABLE);
    // See comment inside loop
    mdToken defaultEndToken = EncodingUtils::buildToken(id,
                                                        getNumberOfRows(id));
    for (uint i = 0; i < typedefTablesSize; i++)
    {
        // The typedef table
        const TypedefTable& typedefTable((const TypedefTable&)*(m_tables[TABLE_TYPEDEF_TABLE][i]));
        // The start token for field/method
        mdToken startToken;
        // The end token is either the size of the table or the next typedef
        // start pointer
        mdToken endToken = defaultEndToken;

        // From the typedef get the right pointer (either method or field)
        if (id == TABLE_FIELD_TABLE)
        {
            startToken = typedefTable.getHeader().m_fields;
            endToken = typedefTable.calculateEndFieldToken(*this);
        }
        if (id == TABLE_METHOD_TABLE)
        {
            startToken = typedefTable.getHeader().m_methods;
            endToken = typedefTable.calculateEndMethodToken(*this);
        }

        // Test whether our token is inside the range
        if ((token >= startToken) && (token < endToken))
            return m_tables[TABLE_TYPEDEF_TABLE][i]->getToken();
    }

    // Cannot be found
    CHECK_FAIL();
}

RowTablesPtr MetadataTables::byTableID(enum TablesID tableID) const
{
    return m_tables[tableID];
}