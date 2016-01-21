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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_LINKERFACTORY_H
#define __TBA_CLR_EXECUTER_RUNTIME_LINKERFACTORY_H

/*
 * LinkerFactory.h
 *
 * Author: Pavel Ferencz
 */
#include "xStl/types.h"
#include "executer/linker/LinkerInterface.h"

/*
 * Generate linker object for new method for specific format
 */
class LinkerFactory {
public:
    // A list of all possible platforms
    enum LinkerType {
        // In-memory linker for 32-bit x86 based machines
        MEMORY_LINKER,
        // COFF format linker
        COFF_LINKER,
        // ELF format linker
        ELF_LINKER,
        // File (source) linker
        FILE_LINKER
    };

    /*
     * Build a new linker for a specific format
     *
     * type                    - The format type
     * compilerEngineThread -
     * apartment            -
     *
     * Return the linker interface
     */
    static LinkerInterfacePtr getLinker(LinkerType type,
                                        CompilerEngineThread& compilerEngineThread,
                                        ApartmentPtr apartment);
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_LINKERFACTORY_H
