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

#ifndef __TBA_CLR_COMPILER_OPCODES_BINARYOPCODES_H
#define __TBA_CLR_COMPILER_OPCODES_BINARYOPCODES_H

/*
 * Bin32Opcodes.h
 *
 * Implements binary and 32bit operations such add/mull/div, etc...
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "compiler/TemporaryStackHolder.h"
#include "compiler/StackEntity.h"
#include "compiler/EmitContext.h"

/*
 *
 */
class BinaryOpcodes
{
public:
    /*
     * Convert an integer or a floating point into one of the native core types.
     *
     * coreType - The core type of which the convention should be
     *
     * The function pop a variable from the stack and push a new converted
     * variable.
     */
    static void convert(EmitContext& emitContext,
                        CorElementType coreType);

    // List for all binary operations.
    enum BinaryOperation
    {
        // Arithmetics operations

        // Addition (+) operation
        BIN_ADD,
        // Substraction (-) operation
        BIN_SUB,
        // Multiple (*) operation
        BIN_MUL,
        // Divide (/) signed operation
        BIN_DIV,
        // Divide (/) unsigned operation
        BIN_DIV_UN,
        // Reminder (%) signed operation
        BIN_REM,
        // Reminder (%) unsigned operation
        BIN_REM_UN,

        // List of logical operations

        // Bitwise and (&) operation
        BIN_AND,
        // Bitwise or (|) operation
        BIN_OR,
        // eXclusive Or (^) operation
        BIN_XOR,
        // Shift left (<<) operation
        BIN_SHL,
        // Shift right (>>) operation
        BIN_SHR
    };

    /*
     * Perform binary operation over two int32 variables.
     */
    static void binary(EmitContext& emitContext,
                       BinaryOperation operation);

    // List for all comparison operations.
    enum ComparisonOperation {
        // Compare equal (== ceq)
        CMP_EQUAL,
        // Compare greater then (> cgt)
        CMP_GREATER_THEN,
        // Compare greater then for unsigned or unorder numbers (> cgt.un)
        CMP_GREATER_THEN_UNSIGNED,
        // Compare less then (< clt)
        CMP_LESS_THEN,
        // Compare less then for unsigned or unordered number (< clt.un)
        CMP_LESS_THEN_UNSIGNED
    };

    /*
     * Compare two variables from stack and push the return value into the
     * stack as 1 (true) or 0 (false).
     */
    static void compare(EmitContext& emitContext,
                        ComparisonOperation operation);
};

#endif // __TBA_CLR_COMPILER_OPCODES_BINARYOPCODES_H
