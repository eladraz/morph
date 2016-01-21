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

#ifndef __TBA_CLR_FORMAT_TABLES_FIELDTABLE_H
#define __TBA_CLR_FORMAT_TABLES_FIELDTABLE_H

/*
 * FieldTable.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataStream.h"
#include "format/tables/Table.h"

class FieldTable : public Table {
public:
    /*
     * Constructor. Reads a single 'Field' table descriptor
     *
     * stream - Reference into s stream which start at the table position.
     *          The stream reference will be advance to the beginning of the
     *          next table
     * token  - A full qualified index ID for this table
     *
     * Throw exception if the first stage of table verification failed.
     */
    FieldTable(MetadataStream& stream,
               mdToken token);

    /*
     * The header of the table as stored in the Clr...
     */
    struct Header {
        // See FieldAttributes
        uint16 m_flags;
        // Index into #Strings heap.
        mdToken m_name;
        // Index into Blob.
        mdToken m_signature;
    };

    /*
     * Return the field's header
     */
    const Header& getHeader() const;

    // See Table::getIndex.  Return TABLE_FIELD_TABLE
    virtual TablesID getIndex() const;
    // See Table::out.       Format the content of the Field-header
    #ifdef TRACED_CLR
    virtual void out(cStringerStream& out,
                     StringsTableFormatter& formatter) const;
    // See Table::getTableName
    virtual cString getTableName() const;
    // See Table::getTableMSILname
    virtual cString getTableMSILname() const;
    #endif // TRACED_CLR

    enum FieldAttributes
    {
        // member access mask - Use this mask to retrieve accessibility information.
        fdFieldAccessMask           =   0x0007,
        // Member not referenceable.
        fdPrivateScope              =   0x0000,
        // Accessible only by the parent type.
        fdPrivate                   =   0x0001,
        // Accessible by sub-types only in this Assembly.
        fdFamANDAssem               =   0x0002,
        // Accessibly by anyone in the Assembly.
        fdAssembly                  =   0x0003,
        // Accessible only by type and sub-types.
        fdFamily                    =   0x0004,
        // Accessibly by sub-types anywhere, plus anyone in assembly.
        fdFamORAssem                =   0x0005,
        // Accessibly by anyone who has visibility to this scope.
        fdPublic                    =   0x0006,
        // end member access mask

        // field contract attributes.
        // Defined on type, else per instance.
        fdStatic                    =   0x0010,
        // Field may only be initialized, not written to after init.
        fdInitOnly                  =   0x0020,
        // Value is compile time constant.
        fdLiteral                   =   0x0040,
        // Field does not have to be serialized when type is remoted.
        fdNotSerialized             =   0x0080,

        // field is special.  Name describes how.
        fdSpecialName               =   0x0200,

        // interop attributes
        // Implementation is forwarded through pinvoke.
        fdPinvokeImpl               =   0x2000,

        // Reserved flags for runtime use only.
        fdReservedMask              =   0x9500,
        // Runtime(metadata internal APIs) should check name encoding.
        fdRTSpecialName             =   0x0400,
        // Field has marshalling information.
        fdHasFieldMarshal           =   0x1000,
        // Field has default.
        fdHasDefault                =   0x8000,
        // Field has RVA.
        fdHasFieldRVA               =   0x0100
    };

private:
    // The table's data
    Header m_header;
};

#endif // __TBA_CLR_FORMAT_TABLES_FIELDTABLE_H
