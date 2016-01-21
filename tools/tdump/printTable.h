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

#ifndef __TBA_CLR_DUMP_PRINTTABLE_H
#define __TBA_CLR_DUMP_PRINTTABLE_H

#include "xStl/types.h"
#include "xStl/stream/marginStringerStream.h"
#include "format/CilFormatLayout.h"
#include "format/MetadataTables.h"
#include "format/MSILStreams.h"
#include "format/tables/Table.h"
#include "format/tables/StringsTableFormatter.h"
#include "runnable/Apartment.h"
#include "resolver.h"

/*
 * Prints a single table with all it's special componented including sub-tables
 * to the screen
 *
 * Author: Elad Raz <e@eladraz.com>
 */
void printTable(Resolver& resolver,
                cMarginStringerStream& output,
                StringsTableFormatter& formatter,
                MSILStreams& streams,
                const TablePtr& table,
                CilFormatLayout& cilFormatLayout,
                MetadataTables& tables,
                bool shouldSeperate = true);

#endif // __TBA_CLR_DUMP_PRINTTABLE_H
