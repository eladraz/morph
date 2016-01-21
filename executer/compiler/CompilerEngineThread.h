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

#ifndef __TBA_CLR_EXECUTER_COMPILERENGINETHREAD_H
#define __TBA_CLR_EXECUTER_COMPILERENGINETHREAD_H

/*
 * CompilerEngineThread.h
 *
 * The engine thread scan methods according to a given algorithm, compile them
 * and store them on the BinaryGetterInterface queue.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "compiler/CompilerFactory.h"
#include "executer/compiler/BinaryGetterInterface.h"
#include "executer/compiler/CompilerNotifierInterface.h"
#include "executer/compiler/ScanningAlgorithmInterface.h"
#include "runnable/Apartment.h"
#include "executer/compiler/PrecompiledRepository.h"

/*
 * Compiling method at user request.
 *
 * NOTE: This class is not thread safe and must be called within a single thread
 *       context
 */
class CompilerEngineThread {
public:
    /*
     * Initialized the scanning algorithm
     *
     * compilerType  - The type of the compiler
     * mainApartemnt - The main apartment for the compiled process. All modules
     *                 should be inserted into this apartment
     */
    CompilerEngineThread(CompilerFactory::CompilerType compilerType,
                         const CompilerParameters& compilerParams,
                         const ApartmentPtr& mainApartemnt,
                         const cString& repositoryFilename);

    /*
     * Called in order to add a notifier object
     *
     * notifier - The object to receive the notification to.
     */
    void addNotifier(CompilerNotifierInterface& notifier);

    /*
     * Start compiling methods and handle them (CompilerNotifierInterfacePtr)
     * according to the scan algorithm.
     *
     * scanAlgorithm - The scanning algorithm. See ScanningAlgorithmInterface
     *
     * NOTE: This method may throw compilation exceptions/runtime exceptions
     *       and more.
     */
    void run(ScanningAlgorithmInterface& scanAlgorithm);

    /*
     * Return the binary repository
     */
    const BinaryGetterInterface& getBinaryRepository() const;

    /*
     * Return the current compiler type
     */
    CompilerFactory::CompilerType getCompilerType() const;

    /*
     * Adding method compilation notification
     */
    void onMethodCompiled(const cString& aptName,
                          const cBuffer& signature,
                          const TokenIndex& mid,
                          const SecondPassBinaryPtr& compiled,
                          bool inCache);

private:
    // Deny copy-constructor and operator =
    CompilerEngineThread(const CompilerEngineThread& other);
    CompilerEngineThread& operator = (const CompilerEngineThread& other);

    /*
     * Called in order to notify all instances that a method was compiled.
     *
     * See CompilerNotifierInterface::onMethodCompiled
     */
    void notifyOnCompiled(const TokenIndex& mid,
                          SecondPassBinary& compiled,
                          bool inCache);

    /*
     * Called in order to notify all instances that a method was failed to be
     * compiled.
     */
    void notifyOnCompilationFalied(const TokenIndex& mid);

    // The list of all notifiers
    CompilerNotifierInterfaceList m_notifiers;
    // The binary repository
    BinaryGetterInterface m_binaryRepository;
    // The precompiled repository
    PrecompiledRepository m_precompiledRepository;
    // The compiler type
    CompilerFactory::CompilerType m_compilerType;
    // The compiler parameters
    const CompilerParameters& m_compilerParams;
    // The main apartment
    ApartmentPtr m_main;
    // Repository filename
    cString m_repositoryFilename;
};

#endif // __TBA_CLR_EXECUTER_COMPILERENGINETHREAD_H
