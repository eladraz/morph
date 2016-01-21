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

#ifndef __TBA_CLR_COMPILER_METHODRUNTIMEBOUNDLE_H
#define __TBA_CLR_COMPILER_METHODRUNTIMEBOUNDLE_H

/*
 * MethodRuntimeBoundle.h
 *
 * A collection of data-structs need for a method.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/setArray.h"
#include "runnable/MethodRunnable.h"
#include "format/MSILScanInterface.h"
#include "compiler/StackEntity.h"
#include "compiler/CompilerInterface.h"
#include "compiler/LocalPositions.h"
#include "compiler/ArgumentsPositions.h"
#include "compiler/MethodBlock.h"

/*
 * A collection of data-structs need for a method.
 * The only purpose of this class is to transfer method information into needed
 * methods
 */
class MethodRuntimeBoundle : public MSILScanInterface {
public:
    /*
     * Constructor. Initialize all members.
     * Generate new stack for the method start See DEFAULT_METHOD_START_ADDRESS
     *
     * compiler      - The compiler to be used
     * locals        - The locals position and values.
     * methodLength  - The length of the method, used in method tracing set
     *
     * NOTE: This method generate the default method block 0.
     */
    MethodRuntimeBoundle(CompilerInterfacePtr compiler,
                         const LocalPositions& locals,
                         const ArgumentsPositions& args,
                         const MethodHeader& methodHeader,
                         TokenIndex cleanupIndex);

    /*
     * Constructor. Create an object similar to 'other', but for use with a difference base register
     * This is useful for helper functions of methods
     */
    MethodRuntimeBoundle(const MethodRuntimeBoundle& other, uint handlerIndex);

    // Destructor. Make sure that the destcurtor order is safe
    virtual ~MethodRuntimeBoundle();

    /*
     * Add a method block to the block-stack by duplicating an existing one, or use its parameters to update an existing block
     */
    void AddMethodBlock(MethodBlock& block, struct EmitContext& emitContext, int newBlockID, bool bRemoveTOS = false);

    /*
     * Overrides MSILScanInterface::OnOffset()
     */
    virtual void OnOffset(uint instructionIndex);

    // The default basic block starting position. Stand on 0. This leaves the
    // method compiler to insert method prolog, exception-handling block and
    // other different components (Argument translation, switch blocks and more)
    enum { DEFAULT_BASIC_BLOCK_START = 0 };

    /*
     * Return the compiler interface
     */
    const CompilerInterface& getCompiler();

//////////////////////////////////////////////////////////////////////////
// Members. TODO! Change to private

    // The compiler interface
    CompilerInterfacePtr m_compiler;
    // The method's locals
    const LocalPositions& m_locals;
    // The method's arguments
    const ArgumentsPositions& m_args;

    ////////////////////////////////////////////////////////////
    // IMPORTANT NOTE! THE METHOD BLOCK STACK IS NOT THREAD SAFE
    // The method block stack
    MethodBlockOrderedList m_blockStack;
    // IMPORTANT NOTE! THE METHOD SET IS NOT THREAD SAFE
    // The method availability matrix
    cSetArray m_methodSet;

    // IMPORTANT NOTE! THE BLOCK SPLIT IS NOT THREAD SAFE
    // The method availability matrix
    cSetArray m_blockSplit;

    // Whether or not a try-catch clause has been handled while compiling code in this context
    bool m_bHasCatch;

    // To access the cleanupIndex from CompilerOpcodes::compilerReturn. Updated in methodEstimateEncoding.
    TokenIndex m_cleanupIndex;

private:
    // Disable copy-constructor and operator =
    MethodRuntimeBoundle(const MethodRuntimeBoundle& other);
    MethodRuntimeBoundle& operator = (const MethodRuntimeBoundle&);
};

#endif // __TBA_CLR_COMPILER_METHODRUNTIMEBOUNDLE_H
