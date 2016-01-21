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

#ifndef __TBA_CLR_FORMAT_METADATATABLES_H
#define __TBA_CLR_FORMAT_METADATATABLES_H

/*
 * MetadataTables.h
 *
 * Contain a cache/partial cache (Depends on memory settings) of all CLR tables
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "format/metadataHeader.h"
#include "format/metadataStream.h"
#include "format/tables/Table.h"

/*
 * This module opens a metadata stream and reads all information contains in the
 * file header. That include critical information data such as tables.
 * The module manage the memory in best fit manner according to memory avaliable
 * and optimization level. The information can be either cached or can be
 * re-mapped according to the method access.
 *
 * The usage of this module is quite simple:
 *   1. Construct a new MetadataTables from MSIL file.
 *   2. Use the 'getTableByToken' in order to access tables from it.
 *
 * NOTE: For now the entire tables are cached.
 *
 * NOTE: This module is thread-safe and can be access from a multiple threads
 *       or ThreadContext
 */
class MetadataTables {
public:
    /*
     * Constructor. Use the Metadata PE header for reading and storing the
     * tables.
     *
     * Throw exception if the meta-data stream is corrupted.
     */
    MetadataTables(MetadataHeader& metaHeader);

    /*
     * Return a reference count object into the table by it's token ID
     * The token ID must be in the following format:
     *    The high 8 bit shall be an index into table ID
     *    The lower 24 bit shall be a row index into the tables list
     *    0       8                       32
     *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *    | <ID>  |  <INDEX>              |
     *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * See EncodingUtils::getTokenTableIndex
     * See EncodingUtils::getTokenPosition
     *
     * IMPORTANT NOTE: The indexes are starting from 1 ID and not from 0!
     *
     * Throw exception if the table ID is invalid or the index into the table
     * row does not exist.
     */
    const TablePtr& getTableByToken(mdToken token) const;

    /*
     * Return the number of rows for a table
     */
    uint getNumberOfRows(uint indexID) const;

    /*
     * According to a token, get it's typedef parent class.
     * token can be one of the following tables:
     *      FieldTable
     *      MethodTable
     *
     * Throw exception if parent table cannot be found
     */
    mdToken getTypedefParent(mdToken token) const;
    RowTablesPtr byTableID(enum TablesID tableID) const;

private:
    // Deny copy-constructor and operator =
    MetadataTables(const MetadataTables& other);
    MetadataTables& operator = (const MetadataTables& other);

    // All of the following members will be protect by the lockable m_lock
    // TODO!
    // cXstlLockable m_lock;

    // The cache stream header. Protected by m_lock
    MetadataStream m_metaStream;
    // The list of all tables. Protected by m_lock
    RowTablesPtr m_tables[MetadataStream::NUMBER_OF_TABLES];
};

#endif // __TBA_CLR_FORMAT_METADATATABLES_H
