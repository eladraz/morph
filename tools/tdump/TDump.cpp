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

#include <stdlib.h>
#include "xStl/types.h"
#include "xStl/os/virtualMemoryAccesser.h"
#include "xStl/os/threadUnsafeMemoryAccesser.h"
#include "xStl/data/char.h"
#include "xStl/data/datastream.h"
#include "xStl/utils/dumpMemory.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/stream/ioStream.h"
#include "xStl/stream/fileStream.h"
#include "xStl/stream/memoryAccesserStream.h"
#include "xStl/stream/marginStringerStream.h"
#include "pe/peFile.h"
#include "pe/ntheader.h"
#include "pe/ntDirCli.h"
#include "pe/dosheader.h"
#include "pe/ntPrivateDirectory.h"
#include "format/methodHeader.h"
#include "format/metadataHeader.h"
#include "format/metadataStream.h"
#include "format/MetadataTables.h"
#include "format/EncodingUtils.h"
#include "format/pe/CilPeLayout.h"
#include "runnable/ApartmentFactory.h"
#include "executer/linker/LinkerFactory.h"
#include "printTable.h"
#include "resolver.h"
#include "DefaultStringsTableFormatter.h"

/*
 * The main entry point. Captures all unexpected exceptions and make sure
 * that the application will notify the programmer.
 *
 * Invoke a call to the following modules:
 *   1.
 *
 * Author: Elad Raz <e@eladraz.com>
 */

int main(int argc, const char** argv)
{
    XSTL_TRY
    {
        // Check the arguments
        if (argc < 2)
        {
            cout << "Usage: testClr <.NET PE file-name>" << endl;
            return RC_ERROR;
        }

        // Open the file for read-only
        cFileStream peFileStream(argv[1]);

        // Print all tables.
        uint tableID = 0xFFFFFFFF;
        if (argc == 3)
            tableID = atoi(argv[2]);

        cNtHeaderPtr ntFile;
        XSTL_TRY
        {
            // Create nt-header
            ntFile = ApartmentFactory::loadEXEFile(peFileStream);
        } XSTL_CATCH_ALL
        {
            cout << "File " << argv[2] << ": INVALID FORMAT" << endl;
            return RC_ERROR;
        }

        // Read the entire CLI header
        // TODO! See clr/format/pe
        CilPeLayout layout(ntFile);
        cNtDirCli cliDirectory(*ntFile);
        #ifdef PE_TRACE
        cout << cliDirectory << endl;
        #endif
        cNtPrivateDirectory metaData(*ntFile, cliDirectory.getCoreHeader().MetaData);

        CompilerFactory::CompilerType compilerType = CompilerFactory::COMPILER_IA32;
//        LinkerFactory::LinkerType linkerType = LinkerFactory::MEMORY_LINKER;
        MemoryLayoutInterfacePtr memoryLayout = CompilerFactory::getMemoryLayout(compilerType);
        ApartmentPtr tempApt(NULL, SMARTPTR_DESTRUCT_NONE);
        ApartmentPtr mainApartment(ApartmentFactory::createApartment(ntFile, cliDirectory, *memoryLayout, tempApt));
        // First read the layout header
        MetadataHeader metaHeader(metaData.getData(), metaData.getDirectory());

#ifdef _DEBUG
        cout << metaHeader << endl;
#endif

        // Prepare output stream
        cMarginStringerStream mout(cout);

        // Read the #Strings, #Blob and the #GUID
        MSILStreams msilStreams(metaHeader);
        // Read tables
        MetadataTables tables(metaHeader); // The global objects

        // Print out #blob sections
        cMemoryAccesserStreamPtr blob = msilStreams.getBlobStream();
        uint blobLength = blob->getMemoryAccesserEndAddress() -
                          blob->getMemoryAccesserStartAddress();
        cBuffer blobData;
        blob->seek(0, basicInput::IO_SEEK_SET);
        blob->pipeRead(blobData, blobLength);
        cout << "#Blob" << endl;
        cout << "=====" << endl;
        DATA data(blobData.begin(), blobData.end());
        cout << data << endl;

        // Print out #String sections
        cout << "#US" << endl;
        cout << "===" << endl;
        cMemoryAccesserStreamPtr strings = msilStreams.getUserStringsStream();
        uint stringsLength = strings->getMemoryAccesserEndAddress() -
                             strings->getMemoryAccesserStartAddress();
        cBuffer stringsData;
        strings->seek(0, basicInput::IO_SEEK_SET);
        strings->pipeRead(stringsData, stringsLength);
        DATA strData(stringsData.begin(), stringsData.end());
        cout << strData << endl;

        for (uint j = 0; j < MetadataStream::NUMBER_OF_TABLES; j++)
            for (uint i = 0; i < tables.getNumberOfRows(j); i++)
        {
            // Get the table
            TablePtr table = tables.getTableByToken(
                                            EncodingUtils::buildToken(j, i + 1));

            // Test wheter we should print the table or not.
            // NOTE: This code is build for optimization
            bool shouldPrintTable =
                (tableID == j) || (tableID == 0xFFFFFFFF);

            // If we should print the table just go to the next table
            if (!shouldPrintTable)
                continue;

            // Read the functions table
#ifdef _DEBUG
            DefaultStringsTableFormatter formatter(msilStreams);
            Resolver resolver(mainApartment, *memoryLayout);
            printTable(resolver, mout, formatter, msilStreams, table, layout, tables);
#endif
        }

    return RC_OK;
    }
    XSTL_CATCH(cException& e)
    {
        // Print the exception
        e.print();
        return RC_ERROR;
    }
    XSTL_CATCH_ALL
    {
        TRACE(TRACE_VERY_HIGH,
                XSTL_STRING("Unknwon exceptions caught at main()..."));
        return RC_ERROR;
    }
}
