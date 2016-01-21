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

#ifndef __TBA_CLR_FORMAT_TABLES_TABLE_H
#define __TBA_CLR_FORMAT_TABLES_TABLE_H

/*
 * Table.h
 *
 * Interface for all tables parser.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/data/string.h"
#include "xStl/data/smartptr.h"
#include "xStl/stream/stringerStream.h"
#include "format/tables/TablesID.h"
#include "format/tables/StringsTableFormatter.h"

/*
 * This interface act as a simple chain-of-responsibilities for table loading
 * and parsing.
 */
class Table {
public:
    // You can inherit from me
    virtual ~Table();

    /*
     * Return the index ID of the table. According to this unique identifier
     * the table can
     */
    virtual TablesID getIndex() const = 0;

    /*
     * Return a full qualified token ID for this table
     */
    mdToken getToken() const;

    #ifdef TRACED_CLR
    /*
     * Format the content of the table as a human readable stream
     *
     * out       - The stream to format the information to
     * formatter - The component which translate id into strings
     *
     * NOTE: This function is not thread-safe
     */
    virtual void out(cStringerStream& out,
                     StringsTableFormatter& formatter) const = 0;

    /*
     * Return the table name
     */
    virtual cString getTableName() const = 0;

    /*
     * Return the assembly instruction which generate the table
     */
    virtual cString getTableMSILname() const = 0;
    #endif // TRACED_CLR

protected:
    /*
     * Protected constructor. Initialize the table-token.
     *
     * token -  A full qualified index ID for this table
     */
    Table(mdToken tableToken);

    // The token for the current table
    mdToken m_tableToken;
};

// The reference-countable object
typedef cSmartPtr<Table> TablePtr;
// An array of tables
typedef cArray<TablePtr> RowTablesPtr;

#endif // __TBA_CLR_FORMAT_TABLES_TABLE_H
