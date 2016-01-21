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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_DEFAULTCOMPILERALGORITHM_H
#define __TBA_CLR_EXECUTER_RUNTIME_DEFAULTCOMPILERALGORITHM_H

/*
* DefaultCompilerAlgorithm.h
*
* The default runtime algorithm.
*
* Author: Elad Raz <e@eladraz.com>
*/
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "format/coreHeadersTypes.h"
#include "runnable/Stack.h"
#include "runnable/ClrResolver.h"
#include "executer/MethodIndex.h"
#include "executer/compiler/ScanningAlgorithmInterface.h"
#include "executer/compiler/CompilerNotifierInterface.h"
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/linker/LinkerInterface.h"
/*
 * The runtime algorithm start compiling methods and execute them by resolving
 * inner connection. Managing pre-ahead queue and more.
 *
 * NOTE: This class is thread-safe
 */
class DefaultCompilerAlgorithm : public ScanningAlgorithmInterface,
                                 public CompilerNotifierInterface {
public:
    /*
    * Constructor. Start scanning the methods according to runtime needs by
    * creating the application main thread.
    *
    * mainMethodID    - The first method to be executed
    * mainApartmentID - The apartment ID of the first method
    * mainApartment   - The apartment handler
    * engine          - The compilation engine. Used to bind to all methods.
    *
    * TODO! Add initialized stack arguments
    */
    DefaultCompilerAlgorithm(const TokenIndex& mainMethod,
                             ApartmentPtr mainApartment,
                             CompilerEngineThread& engine,
                             const LinkerInterfacePtr& linker);

    // See ScanningAlgorithmInterface::getNextMethod
    virtual bool getNextMethod(TokenIndex& mid);

    // See ScanningAlgorithmInterface::shouldExit
    virtual bool shouldExit() const;

    // See CompilerNotifierInterface::onMethodCompiled
    virtual void onMethodCompiled(const TokenIndex& mid, SecondPassBinary& compiled, bool inCache);

    // See CompilerNotifierInterface::onCompilationFailed
    virtual void onCompilationFailed(const TokenIndex& mid);

private:
    // The queue of pre-ahead methods protection
    mutable cXstlLockable m_lock;

    // The stack of all need-to-be compiled methods.
    MethodStackObject m_methodStack;

    // Mark to true
    volatile bool m_shouldExit;

    // The main routine. Execute only if there are no methods left
    TokenIndex m_mainMethod;
    ApartmentPtr m_mainApartment;

    // The binary engine.
    CompilerEngineThread& m_engine;

    // The external method resolver. TODO! Get from factory
    ExternalModuleResolver m_externalModuleResolver;
    ClrResolver m_clrResolver;

    // The linker
    LinkerInterfacePtr m_linker;
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_DEFAULTCOMPILERALGORITHM_H