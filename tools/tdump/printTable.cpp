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
 * printTable.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "data/ElementType.h"
#include "format/EncodingUtils.h"
#include "format/methodHeader.h"
#include "format/tables/Table.h"
#include "format/tables/TablesID.h"
#include "format/tables/GenericParamTable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/MethodSpecTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/tables/TypeSpecTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/FieldTable.h"
#include "format/signatures/FieldSig.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/GlobalContext.h"
#include "printTable.h"

#ifdef _DEBUG

// This file can compile only in debug mode

void printTable(Resolver& resolver,
                cMarginStringerStream& output,
                StringsTableFormatter& formatter,
                MSILStreams& streams,
                const TablePtr& table,
                CilFormatLayout& cilFormatLayout,
                MetadataTables& tables,
                bool shouldSeperate)
{
    // Print the table header prefix
    if (shouldSeperate)
        output << "==== " << HEXDWORD(table->getToken()) << " =========  "
               << table->getTableName() << "==== "
               << table->getTableMSILname() << endl;
    else
        output << HEXDWORD(table->getToken()) << "            "
               << table->getTableName() << "     "
               << table->getTableMSILname() << endl;

    // For all tables show the default content formatter
    table->out(output, formatter);


    switch (table->getIndex())
    {
    case TABLE_TYPESPEC_TABLE:
        {
            const TypeSpecTable::Header& typeSpec = ((TypeSpecTable&)(*table)).getHeader();
            cForkStreamPtr blob = streams.getBlobStream()->fork();
            blob->seek(typeSpec.m_signature + 1, basicInput::IO_SEEK_SET);
            ElementType temp = ElementType::readType(*blob, 0);
            output << temp << endl;
            break;
        }
    case TABLE_METHODSPEC_TABLE:
        {
            const MethodSpecTable::Header& metdhoSpec = ((MethodSpecTable&)(*table)).getHeader();
            cForkStreamPtr blob = streams.getBlobStream()->fork();
            blob->seek(metdhoSpec.m_instantiation, basicInput::IO_SEEK_SET);
            cBuffer temp;
            blob->pipeRead(temp, 0x30);
            DATA dump(temp.begin(), temp.end());
            output << dump << endl;
            break;
        }
    case TABLE_TYPEREF_TABLE:
        {
            const TyperefTable::Header& typeRefTable =
                ((TyperefTable&)(*table)).getHeader();

            output << "Resolution scope: " << endl;
            output.setMargin(output.getMargin() + 4);
            printTable(resolver, output, formatter, streams,
                       tables.getTableByToken(typeRefTable.m_resolutionScope),
                       cilFormatLayout, tables, false);
            output.setMargin(output.getMargin() - 4);
        }
        break;

    case TABLE_MEMBERREF_TABLE:
        {
            const MemberRefTable::Header& memberRefTable =
                ((MemberRefTable&)(*table)).getHeader();

            // Read the method signature
            cMemoryAccesserStreamPtr blob = streams.getBlobStream();
            blob->seek(memberRefTable.m_signature, basicInput::IO_SEEK_SET);
            MethodDefOrRefSignature methodSignature(*blob, 0, resolver);
            output << methodSignature << endl;

            output << "Class parsing: " << endl;
            output.setMargin(output.getMargin() + 4);
            printTable(resolver, output, formatter, streams,
                       tables.getTableByToken(memberRefTable.m_class),
                       cilFormatLayout, tables, false);
            output.setMargin(output.getMargin() - 4);
        }
        break;

    case TABLE_FIELD_TABLE:
        {
            // Get the field header
            const FieldTable::Header& fieldHeader =
                ((FieldTable&)(*table)).getHeader();

            // Read the signature
            cMemoryAccesserStreamPtr blob = streams.getBlobStream();
            blob->seek(fieldHeader.m_signature, basicInput::IO_SEEK_SET);
            FieldSig fieldType(*blob, 0, resolver);
            output << "typedef: " << fieldType.getType() << endl;
        }
        break;

    case TABLE_METHOD_TABLE:
        {
            // Get the method header
            const MethodTable& methodTable = (const MethodTable&)(*table);
            const MethodTable::Header& methodHeader = methodTable.getHeader();

            // Read the method signature
            cMemoryAccesserStreamPtr blob = streams.getBlobStream();
            blob->seek(methodHeader.m_signature, basicInput::IO_SEEK_SET);
            MethodDefOrRefSignature methodSignature(*blob, 0, resolver);
            output << methodSignature << endl;

            // Parse the method's param
            mdToken startParam = methodHeader.m_params;
            mdToken endParam = methodTable.calculateEndParamToken(tables);

            output << "Params:" << endl;
            for (mdToken params = startParam; params < endParam; params++)
            {
                output.setMargin(output.getMargin() + 4);
                printTable(resolver, output, formatter, streams,
                           tables.getTableByToken(params),
                           cilFormatLayout, tables, false);
                output.setMargin(output.getMargin() - 4);
            }

            // Try to parse the method
            if (methodHeader.m_rva != 0)
            {
                // NOTE: The function can be abstract!
                cForkStreamPtr methodStream =
                    cilFormatLayout.getVirtualStream(methodHeader.m_rva);

                MethodHeader method(methodStream);
                output << method;
            }
        }
        break;

    case TABLE_TYPEDEF_TABLE:
        {
            // Get the method header
            const TypedefTable& typeDefTable = ((TypedefTable&)(*table));
            const TypedefTable::Header& typedefHeader = typeDefTable.getHeader();

            // Print out all filleds
            mdToken startField = typedefHeader.m_fields;
            mdToken endField = typeDefTable.calculateEndFieldToken(tables);
            output << "Fields:" << endl;
            for (mdToken field = startField; field < endField; field++)
            {
                output.setMargin(output.getMargin() + 4);
                printTable(resolver, output, formatter, streams,
                           tables.getTableByToken(field),
                           cilFormatLayout, tables, false);
                output.setMargin(output.getMargin() - 4);
            }

            // Print out all methods
            mdToken startMethod = typedefHeader.m_methods;
            mdToken endMethod = typeDefTable.calculateEndMethodToken(tables);
            output << "Methods:" << endl;
            for (mdToken method = startMethod; method < endMethod; method++)
            {
                output.setMargin(output.getMargin() + 4);
                printTable(resolver, output, formatter, streams,
                           tables.getTableByToken(method),
                           cilFormatLayout, tables, false);
                output.setMargin(output.getMargin() - 4);
            }
        }
        break;
    default:
        // for now, don't print any table...
        break;
    }

    // Print postfix seperator
    output << endl;
}

#endif // _DEBUG
