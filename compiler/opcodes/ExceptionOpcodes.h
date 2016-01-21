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

#ifndef __TBA_CLR_COMPILER_OPCODES_EXCEPTIONOPCODES_H
#define __TBA_CLR_COMPILER_OPCODES_EXCEPTIONOPCODES_H

/*
 * ObjectOpcodes.h
 *
 * Implements object operations such as, new-object, reference objects (both locals and args)
 *
 */
#include "xStl/types.h"
#include "compiler/TemporaryStackHolder.h"
#include "compiler/StackEntity.h"
#include "compiler/EmitContext.h"
#include "dismount/assembler/SecondPassBinary.h"

typedef cList<class MethodHelper> MethodHelperList;
typedef cSmartPtr<class MethodHelper> MethodHelperPtr;

/*
 *
 */
class ExceptionOpcodes
{
public:
    /*
     * Implement a leave instruction.
     * emitContext - The compilation context of the code
     * instructionIndex - The index of the leave instruction within the method
     * targetIndex - The index of the leave instruction's target
     *
     * returns true if handled, or false if this "leave" is not an exception-handling
     *         instruction, and should be treated as a "br" instead
     */
    static bool implementLeave(EmitContext& emitContext,
                               int instructionIndex,
                               int targetIndex);

    /*
     * Implement a endfinally/endfault instruction.
     * emitContext - The compilation context of the code
     * instructionIndex - The index of the instruction within the method
     */
    static void implementEndFinally(EmitContext& emitContext,
                                    uint instructionIndex);

    /*
     * Implement a throw instruction.
     * emitContext - The compilation context of the code
     * instructionIndex - The index of the instruction within the method
     */
    static void implementThrow(EmitContext& emitContext,
                               uint instructionIndex);

    /*
     * Detect entering a protected block (try), before compiling an instruction
     * emitContext - The compilation context of the code
     * instructionIndex - The index of the instruction which may be inside a protected block
     * helpers - A list of method helpers. Any new helpers will be added to this list.
     */
    static void enterProtectedBlock(EmitContext& emitContext,
                                    uint instructionIndex,
                                    MethodHelperList& helpers);

};

// A class that describes a method's helper function
class MethodHelper : public MethodBlock::ExceptionEntry
{
public:
    /*
    * Constructor for helpers which are exception clause handlers
    * entry - The exception entry for which this helper is the handler
    * parent - The parent of this helper. May be the method, or one of its helpers (in case of a nested try block)
    */
    MethodHelper(const MethodBlock::ExceptionEntry& entry, MethodHelper* parent = NULL);

public:
    // The compiler which is used to compile the helper (handler) code
    CompilerInterfacePtr m_compiler;
    // The boundle which is used to compile the helper (handler) code
    cSmartPtr<MethodRuntimeBoundle> m_boundle;
    // The register touched set for the helper code
    RegisterAllocationTable m_touched;
    // The second-pass binary for this helper
    SecondPassBinaryPtr m_secondPass;
    // The parent helper function for which this object is the helper
    // NULL if the parent function is the method
    MethodHelper* m_parent;
    // The base stack pointer of the method, which is used in the helper function
    StackLocation m_baseStackPointer;
    // The total temporary stack size
    int m_nTotalTempStack;
};

#endif // __TBA_CLR_COMPILER_OPCODES_OBJECTOPCODES_H
