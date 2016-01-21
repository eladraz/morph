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
 * CustomAttributeTable.cpp
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
#include "format/tables/CustomAttributeTable.h"

CustomAttributeTable::CustomAttributeTable(MetadataStream& stream,
                                           mdToken token) :
    Table(token)
{
    m_header.m_parent = EncodingUtils::unpackHasCustomAttributeToken(
                                    stream.readHasCustomAttributeToken());
    m_header.m_type = EncodingUtils::unpackCustomAttributeTypeToken(
                                    stream.readCustomAttributeTypeToken());
    m_header.m_value = stream.readBlobToken();
}

TablesID CustomAttributeTable::getIndex() const
{
    return TABLE_CUSTOMATTRIBUTE_TABLE;
}

#ifdef TRACED_CLR
void CustomAttributeTable::out(cStringerStream& out,
                        StringsTableFormatter& formatter) const
{
    out << "Parent:     " << HEXDWORD(m_header.m_parent) << endl;
    out << "Type:       " << HEXDWORD(m_header.m_type) << endl;
    out << "Value:      " << HEXDWORD(m_header.m_value) << endl;
}

cString CustomAttributeTable::getTableName() const
{
    return "CustomAttributeTable";
}

cString CustomAttributeTable::getTableMSILname() const
{
    return ".custom";
}
#endif // TRACED_CLR

mdToken CustomAttributeTable::getParent(void) const
{
    return m_header.m_parent;
}

mdToken CustomAttributeTable::getType(void) const
{
    return m_header.m_type;
}

mdToken CustomAttributeTable::getBlobIndex(void) const
{
    return m_header.m_value;
}

