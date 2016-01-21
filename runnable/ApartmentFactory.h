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

#ifndef __TBA_CLR_RUNNABLE_APARTMENTFACTORY_H
#define __TBA_CLR_RUNNABLE_APARTMENTFACTORY_H

/*
 * ApartmentFactory.h
 *
 * Load modules and generate apartment object
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/basicIO.h"
#include "xStl/stream/memoryStream.h"
#include "runnable/Apartment.h"
#include "pe/ntheader.h"
#include "pe/ntDirCli.h"
#include "runnable/MemoryLayoutInterface.h"

/*
 * Some helper routines for loading modules
 *
 * Usage:
 *       cFileStream peFileStream("HelloWorld.exe");
 *       cNtHeaderPtr ntFile(ApartmentFactory::loadEXEFile(peFileStream));
 *       cNtDirCli cliDirectory(*ntFile);
 *       ApartmentPtr mainApartment(ApartmentFactory::createApartment(
 *                                       ntFile,
 *                                       cliDirectory));
 *
 * As simple as that!
 */
class ApartmentFactory {
public:
    /*
     * From a nt-header image (file/stream/memory) generate new apartment object
     *
     * mainObjects - Global context for all apartments
     *
     * The apartment is initialized.
     * NOTE: Don't forget to call destroy when the apartment is about to be
     *       destroied.
     */
    static ApartmentPtr createApartment(cNtHeaderPtr& inputStream,
                                        const cNtDirCli& cliDirectory,
                                        const MemoryLayoutInterface& memoryLayout,
                                        ApartmentPtr& mainApartment);

    /*
     * Load EXE file from the file-system and generate new nt-header object
     *
     * Throw exception if the fail-format is invalid
     */
    static cNtHeaderPtr loadEXEFile(basicInput& stream);

    // TODO!

    // static cNtHeaderPtr loadEXEFromMemoryUnmapped();
    // static cNtHeaderPtr attachToEXE();
};

#endif // __TBA_CLR_RUNNABLE_APARTMENTFACTORY_H

