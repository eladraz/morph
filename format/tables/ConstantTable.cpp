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
 * ConstantTable.cpp
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
#include "format/tables/ConstantTable.h"

ConstantTable::ConstantTable(MetadataStream& stream,
                             mdToken token) :
    Table(token)
{
    stream.streamReadUint16(m_header.m_type);

    // Read Field, Param or Property value
    m_header.m_parent = EncodingUtils::unpackHasConstantToken(
        stream.readHasConstantToken());

    m_header.m_value = stream.readBlobToken();
}

TablesID ConstantTable::getIndex() const
{
    return TABLE_CONSTANT_TABLE;
}

#ifdef TRACED_CLR
void ConstantTable::out(cStringerStream& out,
                         StringsTableFormatter& formatter) const
{
    out << "Type:       " << HEXWORD(m_header.m_type) << "    ";
    switch (m_header.m_type)
    {
    case ELEMENT_TYPE_BOOLEAN: out << "bool"; break;
    case ELEMENT_TYPE_CHAR:    out << "char"; break;
    case ELEMENT_TYPE_I1:      out << "int8"; break;
    case ELEMENT_TYPE_I2:      out << "int16"; break;
    case ELEMENT_TYPE_I4:      out << "int32"; break;
    case ELEMENT_TYPE_I8:      out << "int64"; break;

    case ELEMENT_TYPE_U1:      out << "uint8"; break;
    case ELEMENT_TYPE_U2:      out << "uint16"; break;
    case ELEMENT_TYPE_U4:      out << "uint32"; break;
    case ELEMENT_TYPE_U8:      out << "uint64"; break;

    case ELEMENT_TYPE_R4:      out << "float"; break;
    case ELEMENT_TYPE_R8:      out << "double"; break;
    case ELEMENT_TYPE_STRING:  out << "string"; break;
    case ELEMENT_TYPE_VOID:    out << "void"; break;
    case ELEMENT_TYPE_OBJECT:  out << "System.Object"; break;
    }
    out << endl;
    out << "Parent:     " << HEXDWORD(m_header.m_parent) << endl;
    out << "Value:      " << HEXDWORD(m_header.m_value) << endl;
}

cString ConstantTable::getTableName() const
{
    return "ConstantTable";
}

cString ConstantTable::getTableMSILname() const
{
    return ".const=";
}
#endif // TRACED_CLR
