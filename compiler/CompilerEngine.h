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

#ifndef __TBA_CLR_COMPILER_COMPILERENGINE_H
#define __TBA_CLR_COMPILER_COMPILERENGINE_H

/*
 * CompilerEngine.h
 *
 * The core of the compilation process. This module is a generic module, which
 * pass on the MSIL instruction set and dispatch the calls into unique compiler
 * modules.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/basicIO.h"
#include "xStl/stream/forkStream.h"
#include "compiler/TemporaryStackHolder.h"
#include "compiler/MethodRuntimeBoundle.h"
#include "compiler/MethodBlock.h"
#include "compiler/StackEntity.h"
#include "compiler/EmitContext.h"
#include "runnable/ResolverInterface.h"

// Forward decelerations
class CallingConvention;

/*
 * The core of the compilation process. This module is a generic module, which
 * pass on the MSIL instruction set and dispatch the calls into unique compiler
 * modules.
 *
 * NOTE: This module called by the MethodCompiler and designed to be expand into
 *       multi-threading enviroment.
 */
class CompilerEngine {
public:
    /*
     * Compile one MSIL instruction
     *
     * emitContext      - Method context. See EmitContext
     * instructionCache - Current block instruction cache
     *
     * NOTE: Argument which you mustn't use! (TODO! Refactor this code)
     * ================================================================
     * initializedDummyPosition   - Used in instruction reuse. The original
     *                              stream location. Used in pointers
     *                              calculation. If this number is different
     *                              then 0, then the engine assumes that
     *                              'instructionCache' is logical stream
     *
     * Return true if the current block is completed.
     * Return false if the current block is not ready yet and there are more
     * instruction to be compiled.
     */
    static bool step(EmitContext& emitContext,
                     basicInput& instructionCache,
                     uint initializedDummyPosition = 0);


    /*
     * Handle split condition, if this is a split point:
     *    - Terminate the current block with COND_NON
     *    - Generate a new block at next instruction with same stack
     *
     * emitContext      - Method context. See EmitContext
     * instructionCache - Current block instruction cache
     *
     * Return true if there is a split, so the current block is done.
     * Return false if the current block is not split and there are more
     *        instructions to be compiled.
     */
    static bool handleSplit(EmitContext& emitContext,
                     basicInput& instructionCache);

    /*
     * Jump into a basic block address
     *
     * emitContext      - Method context. See EmitContext
     * blockNumber      - The block to jump to (See Compiler MANGLED names)
     * shortAddress     - Set to true if the jump into address can be decoded as
     *                    short relative jump
     */
    static void jumpToAddress(EmitContext& emitContext,
                              int blockNumber,
                              bool shortAddress);

    /*
     * Jump into a basic block address with zero condition
     *
     * emitContext      - Method context. See EmitContext
     * blockNumber      - The block to jump to (See Compiler MANGLED names)
     * shortAddress     - Set to true if the jump into address can be decoded as
     *                    short relative jump
     *
     * Exception will be thrown if the stack is invalid.
     */
    static void simpleConditionalJump(EmitContext& emitContext,
                                      int blockNumber,
                                      bool shortAddress,
                                      bool isZero);

    /*
     * Make sure that a specific block terminates as the same state as a require
     * stack "customStack"
     *
     * emitContext      - Method context. See EmitContext
     * entitySource     - Temporary variable. (Not used)
     */
    static void fixStack(EmitContext& emitContext,
                         Stack& customStack);

    /*
     * From a 32 bit token, resolve the right element type
     *
     * outputType - The new resolved element type
     */
    static void resolveTypeToken(mdToken u32,
                                 uint apartmentId,
                                 ResolverInterface& resolver,
                                 ElementType& outputType);

private:
    // The calling convention can access compiling facilities.
    friend class CallingConvention;

    /*
     * Implements load field address using field token and object
     * Used for ldflda instruction as well as stfld
     */
    static void implementLoadField(EmitContext& emitContext,
                                   TokenIndex& field,
                                   StackEntity& object);

    /*
     * From element-type invoke a call to static initializer wrapper
     */
    static void emitStaticInitializer(EmitContext& emitContext, const ElementType& tokenType);

    /*
     * Some opcodes have several of addresses in order to encode numbers from
     * 0-3 depending on the opcodeBase. This routine returns 0-3 or MAX_UINT
     * if the opcode is not in the variable range
     *
     * opcode     - The opcode index
     * opcodeBase - The start index for the opcode optimized encoding
     */
    static uint translateIndexEncoding(uint8 opcode, uint8 opcodeBase);

    /*
     * Read an offset from the stream.
     *
     * stream    - The instruction stream
     * shortForm - Set to true in order to read 8 bit encoded offset, otherwise
     *             the function will read 32 bit offset.
     */
    static int readOffset(basicInput& stream, bool shortForm);

    // The command for generateInstructionStream
    enum ILasmInstruction {
        // FE 01 - Compare equal
        ILASM_CEQ,
        // FE 02 - Compare greater then
        ILASM_CGT,
        // FE 03 - Compare greater then unsigned or unorder
        ILASM_CGT_UN,
        // FE 04 - Compare less then
        ILASM_CLT,
        // FE 05 - Compare less then unsigned or unorder
        ILASM_CLT_UN,

        // 39/2C - Branch on false, null, or zero
        ILASM_BRFALSE,
        // 3A/2D - Branch on true, non-null, or non-zero
        ILASM_BRTRUE,
        // 38/2B - Branch always
        ILASM_BR,
    };

    /*
     * Generate new assembler stream containing a single instruction.
     *
     * instruction - The instruction to generate
     * immediate   - Hard-coded immediate in the right cases
     */
    static cForkStreamPtr generateInstructionStream(ILasmInstruction instruction,
                                                    int32 immediate);

    /*
     * Check that the current context allows unsafe execution.
     *
     * Throw ClrRuntimeException (Unsafe)
     */
    static void checkUnsafe(MethodRuntimeBoundle& methodRuntime);
};

#endif // __TBA_CLR_COMPILER_COMPILERENGINE_H
