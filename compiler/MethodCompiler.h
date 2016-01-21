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

#ifndef __TBA_CLR_COMPILER_METHODCOMPILER_H
#define __TBA_CLR_COMPILER_METHODCOMPILER_H

/*
 * MethodCompiler.h
 *
 * Module which passes on a single method and compile it (First pass).
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "format/coreHeadersTypes.h"
#include "format/signatures/LocalVarSignature.h"
#include "runnable/Apartment.h"
#include "runnable/MethodRunnable.h"
#include "compiler/StackEntity.h"
#include "compiler/CompilerInterface.h"
#include "compiler/CompilerFactory.h"
#include "compiler/LocalPositions.h"
#include "compiler/MethodRuntimeBoundle.h"
#include "compiler/opcodes/ExceptionOpcodes.h"
#include "executer/compiler/BinaryGetterInterface.h"
#include "executer/compiler/PrecompiledRepository.h"
#include "executer/compiler/CompilerNotifierInterface.h"
#include "dismount/assembler/SecondPassBinary.h"

// Forward deceleration
class CompilerEngine;
class CompilerEngineThread;

/*
 * The compiler output FirstPassBinary which includes the following:
 *    - A binary of the first-pass processor target
 *    - All related methods of which the current method execute
 *    - All places for relocation
 *    - All extern methods
 */
class MethodCompiler {
public:
    /*
     * Constructor. Setup the method compiler settings.
     *
     * compilerType - The compiler type. See CompilerFactory::CompilerType
     * apartment    - The cached apartment value
     * methodToken  - The method token to compile
     */
    MethodCompiler(CompilerFactory::CompilerType compilerType,
                   const CompilerParameters& compilerParameters,
                   const ApartmentPtr& apartment,
                   mdToken methodToken,
                   MethodRunnable& methodRunnable);

    /*
     * Compile the method.
     *
     * binaryRepository - Repository which can be used in order to store
     *                    utility functions for this method (like cleanup, etc)
     *
     * Return SecondPassBinary reperesenting the APARTMENT::METHOD given in the
     * constructor.
     *
     * Throw exception if the method is invalid or one of the tables inside the
     * apartment is incomplete (corrupted.)
     *
     * See FirstPassBinary
     */
    SecondPassBinaryPtr compile(CompilerEngineThread& engineThread);

    /*
     * Align a value according to the stack settings.
     * This method is used by CallingConvention class
     */
    static uint stackAlign(uint size,const CompilerInterface& compilerInterface);

    /*
     * Builds a .cctor wrapper function
     *
     * globalBool - .data variable that indicates if the function has been called or not
     * cctorMethod - Pointer to the .cctor function
     *
     * Return a compileld second pass binary
     */
    static SecondPassBinaryPtr compileCCTORWrapper(const Apartment& apartment,
                                                   CompilerFactory::CompilerType compilerType,
                                                   const CompilerParameters compilerParams,
                                                   const TokenIndex& globalBool, mdToken cctorMethod);

    /*
     *
     */
    static SecondPassBinaryPtr compileInstanceDestructor(const Apartment& apartment,
                                                         CompilerFactory::CompilerType compilerType,
                                                         const CompilerParameters compilerParams,
                                                         const TokenIndex& instanceDetorToken);

protected:
    // The engine should know about the following data-types
    friend class CompilerEngine;

private:
    /*
     * Each method has a "cleanup" methods which derefence objects
     * upon function exit or exception
     */
    void methodCreateCleanupFunc(ElementType& elementType,
                                 LocalPositions& locals,
                                 ArgumentsPositions& args,
                                 CompilerEngineThread& engineThread,
                                 const cString& methodName,
                                 SecondPassBinaryPtr secondPass);

    /*
     * Generate a token index for the cleanup function, if needed (or leave empty otherwise)
     */
    void generateCleanupTokenIndex(LocalPositions& locals, ArgumentsPositions& args, const cString& methodName);

    /**
     * Generate initialization block for all non-value locals (object references)
     */
    void methodInitObjectLocals(class LocalPositions& locals);

    /**
     * Register the cleanup function (if any) on the exception stack
     */
    void methodRegisterCleanupFunction(MethodRunnable& method, MethodRuntimeBoundle& boundle);

    /*
     * Take a list of all basic blocks, and write estimates condition according
     * to estimate number of blocks.
     */
    void methodEstimateEncoding(MethodRuntimeBoundle& boundle,
                                MethodHelper* pCurrentHandler = NULL);

    /*
     * Take the finish method and replace all block-dependency into fixed size
     */
    void methodRelocateBlocks();

    /*
     * Generate all member destructor function
     */
    static void createClassMemberDestructorContent(const TypedefRepository& repository,
                                                   CompilerInterface& _interface,
                                                   StackInterface& stack,
                                                   const cString& funcDecObj,
                                                   const TokenIndex& partentToken,
                                                   bool shouldReferenceBP);

    /*
     * Scan all locals and calculate the length of the total stack, in bytes
     *
     * stack  - The stack signature
     * locals - Will be filled with the local stack locations. The size of the
     *          locals will also change
     */
    uint calculateStackSize(const ElementsArrayType& locals,
                            LocalPositions& localsPos) const;

    /*
     * Test whether a jump to address can be encoded in it's short form.
     *
     * currentBlock   - The current block of which the condition should be
     *                  appended (At the end of the block, of-course)
     * jmpAddress     - The address to jump to
     * blocks         - All blocks in the system, used to calculate estimate
     *                  jumping length
     * methodBlocks   - The relations between each basic blocks
     *
     * Returns true if the jump can be done using relative addressing, returns
     * false otherwise
     */
    bool canUseShortRelativeAddress(int currentBlock,
                                    int jmpAddress,
                                    uint compilerInterfaceThershold,
                        const FirstPassBinary::BasicBlockList& blocks) const;

    /*
     * Takes the size of the 'block' and increase it by the maximum size that
     * the condition might takes
     */
    uint getMaxBlockSize(const MethodBlock& block,
                         const FirstPassBinary::BasicBlock& binary) const;

    /*
     * Compiles method blocks until there are no more
     * boundle - All context information about what is being compiled, as well as the block-stack to compile
     * pCurrentHandler - optional pointer to the current exception handler which is being compiled
     */
    void compileBlocks(MethodRuntimeBoundle& boundle, class MethodHelper* pCurrentHandler = NULL);

    // The apartment
    ApartmentPtr m_apartment;
    // The method inside the apartment to be compiler
    mdToken m_methodToken;
    // The cleanup routine for this method, if any
    TokenIndex m_cleanupIndex;
    // The compiler type
    CompilerFactory::CompilerType m_compilerType;
    // The compiler parameters
    const CompilerParameters& m_compilerParams;
    // The compiler interface to be in used
    CompilerInterfacePtr m_interface;
    // Method runnable context
    MethodRunnable& m_methodRunnable;

    // Exception Handling Helpers that need compilation too
    MethodHelperList m_helpers;

    static const char gCustomAttributeExportClass[];
};

// The compiler reference object
typedef cSmartPtr<MethodCompiler> MethodCompilerPtr;

#endif // __TBA_CLR_COMPILER_METHODCOMPILER_H
