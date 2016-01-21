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
 * ApartmentFactory.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/traceStream.h"
#include "format/coreHeadersTypes.h"
#include "format/CilFormatLayout.h"
#include "format/pe/CilPeLayout.h"
#include "runnable/ApartmentFactory.h"
#include "runnable/RunnableTrace.h"
#include "pe/dosheader.h"
#include "pe/ntheader.h"
#include "pe/ntPrivateDirectory.h"
#include "pe/ntDirCli.h"

ApartmentPtr ApartmentFactory::createApartment(cNtHeaderPtr& inputStream,
                                               const cNtDirCli& cliDirectory,
                                               const MemoryLayoutInterface& memoryLayout,
                                               ApartmentPtr& mainApartment)
{
    // Generate the layout object
    CilFormatLayoutPtr layoutPtr(new CilPeLayout(inputStream));

    // Read the entire CLI header
    cNtPrivateDirectory metaData(*inputStream,
                                 cliDirectory.getCoreHeader().MetaData);
    // Prepare the apartment
    ApartmentPtr apartment(new Apartment(layoutPtr,
                                         metaData.getData(),
                                         metaData.getDirectory(),
                                         cliDirectory.getCoreHeader().EntryPointToken,
                                         mainApartment));
    // Init memory
    apartment->init(apartment, memoryLayout);
    RunnableTrace("Created apartment: " << *apartment << endl);

    // Insert apartment if needed
    if (!mainApartment.isEmpty())
    {
        mainApartment->insertModule(apartment);
    }

    // And return newly constructed apartment
    return apartment;
}

cNtHeaderPtr ApartmentFactory::loadEXEFile(basicInput& stream)
{
    // Read the PE
    cDosHeader dosFile(stream, false);

    uint32 id;
    stream.seek(dosFile.e_lfanew, basicInput::IO_SEEK_SET);
    stream.pipeRead(&id ,sizeof(id));
    CHECK(id == IMAGE_NT_SIGNATURE);
    stream.seek(dosFile.e_lfanew, basicInput::IO_SEEK_SET);

    return cNtHeaderPtr(new cNtHeader(stream));
}
