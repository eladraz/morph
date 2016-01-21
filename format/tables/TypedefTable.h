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

#ifndef __TBA_CLR_FORMAT_TABLES_TYPEDEFTABLE_H
#define __TBA_CLR_FORMAT_TABLES_TYPEDEFTABLE_H

/*
 * TypedefTable.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataStream.h"
#include "format/MetadataTables.h"
#include "format/tables/Table.h"

class TypedefTable : public Table {
public:
    /*
     * Constructor. Reads a single 'TypeDef' table descriptor
     *
     * stream - Reference into s stream which start at the table position.
     *          The stream reference will be advance to the beginning of the
     *          next table
     * token  - A full qualified index ID for this table
     *
     * Throw exception if the first stage of table verification failed.
     */
    TypedefTable(MetadataStream& stream,
                 mdToken token);

    /*
     * The header of the table as stored in the Clr...
     */
    struct Header {
        // See TypeAttributes
        uint32 m_flags;
        // Index into #Strings heap.
        mdToken m_name;
        // Index into #Strings heap.
        mdToken m_namespace;
        // Index into TypeDef, TypeRef or TypeSpec table. More precisely, a
        // TypeDefOrRef coded index.
        mdToken m_extends;
        // Index into FieldTable.
        mdToken m_fields;
        // Index into MethodTable.
        mdToken m_methods;
    };

    /*
     * Return the typedef header
     */
    const Header& getHeader() const;

    /*
     * Return the end-token of the typedef's methods list.
     * If this typedef token is the last row index of typedef tables, then the
     * function returns the last method token. Otherwise the function returns
     * the start method of the next typedef table in this row
     */
    mdToken calculateEndMethodToken(const MetadataTables& tables) const;

    /*
     * Return the end-token of the typedef's fields list.
     * If this typedef token is the last row index of typedef tables, then the
     * function returns the last field token. Otherwise the function returns
     * the start field of the next typedef table in this row
     */
    mdToken calculateEndFieldToken(const MetadataTables& tables) const;

    // See Table::getIndex.  Return TABLE_TYPEDEF_TABLE
    virtual TablesID getIndex() const;
    // See Table::out.       Format the content of the TypeDef-header
    #ifdef TRACED_CLR
    virtual void out(cStringerStream& out,
                     StringsTableFormatter& formatter) const;
    // See Table::getTableName
    virtual cString getTableName() const;
    // See Table::getTableMSILname
    virtual cString getTableMSILname() const;
    #endif // TRACED_CLR

    mdToken getName(void) const;
    mdToken getMethodList(void) const;

    // Copy from CorHdr.h
    enum TypeAttributes
    {
        // Use this mask to retrieve the type visibility information.
        tdVisibilityMask        =   0x00000007,
        // Class is not public scope.
        tdNotPublic             =   0x00000000,
        // Class is public scope.
        tdPublic                =   0x00000001,
        // Class is nested with public visibility.
        tdNestedPublic          =   0x00000002,
        // Class is nested with private visibility.
        tdNestedPrivate         =   0x00000003,
        // Class is nested with family visibility.
        tdNestedFamily          =   0x00000004,
        // Class is nested with assembly visibility.
        tdNestedAssembly        =   0x00000005,
        // Class is nested with family and assembly visibility.
        tdNestedFamANDAssem     =   0x00000006,
        // Class is nested with family or assembly visibility.
        tdNestedFamORAssem      =   0x00000007,

        // Use this mask to retrieve class layout information
        tdLayoutMask            =   0x00000018,
        // Class fields are auto-laid out
        tdAutoLayout            =   0x00000000,
        // Class fields are laid out sequentially
        tdSequentialLayout      =   0x00000008,
        // Layout is supplied explicitly
        tdExplicitLayout        =   0x00000010,
        // end layout mask

        // Use this mask to retrieve class semantics information.
        tdClassSemanticsMask    =   0x00000020,
        // Type is a class.
        tdClass                 =   0x00000000,
        // Type is an interface.
        tdInterface             =   0x00000020,
        // end semantics mask

        // Special semantics in addition to class semantics.
        // Class is abstract
        tdAbstract              =   0x00000080,
        // Class is concrete and may not be extended
        tdSealed                =   0x00000100,
        // Class name is special.  Name describes how.
        tdSpecialName           =   0x00000400,

        // Implementation attributes.
        // Class / interface is imported
        tdImport                =   0x00001000,
        // The class is Serializable.
        tdSerializable          =   0x00002000,

        // Use tdStringFormatMask to retrieve string information for native interop
        tdStringFormatMask      =   0x00030000,
        // LPTSTR is interpreted as ANSI in this class
        tdAnsiClass             =   0x00000000,
        // LPTSTR is interpreted as UNICODE
        tdUnicodeClass          =   0x00010000,
        // LPTSTR is interpreted automatically
        tdAutoClass             =   0x00020000,
        // end string format mask

        // Initialize the class any time before first static field access.
        tdBeforeFieldInit       =   0x00100000,

        // Flags reserved for runtime use.
        tdReservedMask          =   0x00040800,
        // Runtime should check name encoding.
        tdRTSpecialName         =   0x00000800,
        // Class has security associate with it.
        tdHasSecurity           =   0x00040000
    };

private:
    // The table's data
    Header m_header;
};

#endif // __TBA_CLR_FORMAT_TABLES_TYPEDEFTABLE_H
