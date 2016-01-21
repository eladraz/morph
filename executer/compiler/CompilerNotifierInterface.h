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

#ifndef __TBA_CLR_EXECUTER_COMPILERNOTIFIERINTERFACE_H
#define __TBA_CLR_EXECUTER_COMPILERNOTIFIERINTERFACE_H

/*
 * CompilerNotifierInterface.h
 *
 * Interface for compiler notification events.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "dismount/assembler/SecondPassBinary.h"
#include "format/coreHeadersTypes.h"

/*
 * Interface for compiler notification events.
 * See ScanningAlgorithmInterface for vivid example.
 *
 * The most usage notifications are implementation of pre-scan algorithms and
 * method executing routines.
 */
class CompilerNotifierInterface {
public:
    // You can inherit from me
    virtual ~CompilerNotifierInterface() {};

    /*
     * Called when a method finish it's compilation process.
     *
     * mid         - The method id
     * apartmentId - The apartment index.
     * compiled    - The compiled method
     * inCache     - Set to true to notify that the method was already in cache
     *
     * NOTE: The context of this method is within the thread that executed the
     *       'CompilerEngineThread'. Hense, the context is synamic.
     */
    virtual void onMethodCompiled(const TokenIndex& mid,
                                  SecondPassBinary& compiled,
                                  bool inCache) = 0;

    /*
     * Called when the compiler failed to compile a method. Used for error
     * handling
     *
     * mid         - The method id
     * apartmentId - The apartment index.
     *
     * NOTE: The context of this method is within the thread that executed the
     *       'CompilerEngineThread'. Hense, the context is synamic.
     */
    virtual void onCompilationFailed(const TokenIndex& mid) = 0;
};

// The reference countable object
typedef cSmartPtr<CompilerNotifierInterface> CompilerNotifierInterfacePtr;
typedef cList<CompilerNotifierInterface*> CompilerNotifierInterfaceList;

#endif // __TBA_CLR_EXECUTER_COMPILERNOTIFIERINTERFACE_H
