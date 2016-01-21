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

#ifndef __TBA_CLR_FORMAT_TABLES_NESTEDCLASSTABLE_H
#define __TBA_CLR_FORMAT_TABLES_NESTEDCLASSTABLE_H

/*
 * NestedClassTable.h
 *
 * The NestedClass table records which Type definitions are nested within which
 * other Type definition.  In a typical high-level language, including ilasm,
 * the nested class is defined as lexically �inside� the text of its enclosing
 * Type
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataStream.h"
#include "format/tables/Table.h"

class NestedClassTable : public Table {
public:
    /*
     * Constructor. Reads a single 'NestedClassTable' table descriptor
     *
     * stream - Reference into s stream which start at the table position.
     *          The stream reference will be advance to the beginning of the
     *          next table
     * token  - A full qualified index ID for this table
     *
     * Throw exception if the first stage of table verification failed.
     */
    NestedClassTable(MetadataStream& stream,
                     mdToken token);

    /*
     * The header of the table as stored in the Clr...
     */
    struct Header {
        // TypeDef table index
        mdToken m_nestedClass;
        // TypeDef table index
        mdToken m_enclosingClass;
    };

    /*
     * Return the nested-class header
     */
    const Header& getHeader() const;


    // See Table::getIndex.  Return TABLE_NESTEDCLASS_TABLE
    virtual TablesID getIndex() const;
    // See Table::out.       Format the content of the NestedClassTable-header
    #ifdef TRACED_CLR
    virtual void out(cStringerStream& out,
                     StringsTableFormatter& formatter) const;
    // See Table::getTableName
    virtual cString getTableName() const;
    // See Table::getTableMSILname
    virtual cString getTableMSILname() const;
    #endif // TRACED_CLR

private:
    // The table's data
    Header m_header;
};

#endif // __TBA_CLR_FORMAT_TABLES_NESTEDCLASSTABLE_H