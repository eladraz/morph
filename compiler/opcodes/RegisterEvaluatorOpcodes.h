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

#ifndef __TBA_CLR_COMPILER_OPCODES_REGISTEREVALUATOROPCODES_H
#define __TBA_CLR_COMPILER_OPCODES_REGISTEREVALUATOROPCODES_H

/*
 * RegisterEvaluatorOpcodes.h
 *
 * Implements register based and stack base operation.
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
class RegisterEvaluatorOpcodes
{
public:
    /*
     * Check that 'var' can be converted into 32 bit integer and returns wheter
     * sign extend should be in place.
     *
     * 'var' - The variable to be acquired.
     *
     * Return true if the variable is signed, and needed to be converted using
     * sign extend variable.
     */
    static bool getSignExtend32WithCheck(const ElementType& var);

    /*
     * Take a ENTITY_LOCAL or ENTITY_ARGUMENT and convert them into local/argument address
     * Use for getting to the memory directly
     */
    static void convertLocalOrArgToLocArgAddress(EmitContext& emitContext,
                                                 StackEntity& entity);

    /*
     * Evaluate 'entity' as a stack value as an integer 32 bit register.
     *
     * emitContext   - Method context. See EmitContext
     * entity        - [in/out] the object, will be changed to register or register address
     * shortUse      - Set to true to indicate that the value is temporary.
     * offset        - In memory address, used as a relative offset
     *
     * Return the register StackLocation
     */
    static void evaluateInt32(EmitContext& emitContext,
                              StackEntity& entity,
                              bool shortUse = true,
                              uint offset = 0,
                              bool shouldEvalToRegAddress = false);

    /*
     * Store a variable into a stack location
     *
     * emitContext   - Method context. See EmitContext
     * source        - The variable that contains the memory
     * destination   - The variable to store to. Can be LOCAL/ARGUMENT or REGISTER_ADDRESS (only)
     * operandSize   - [optional] Force size (0 means this argument is being ignored)
     */
    static void storeVar(EmitContext& emitContext,
                         StackEntity& source,
                         StackEntity& destination,
                         uint operandSize = 0);

    /*
     * Load from the stack the return value and jump into epilog.
     *
     * emitContext      - Method context. See EmitContext
     *
     * Throw exception if the stack doesn't equal to the return value
     */
    static void loadReturnedValue(EmitContext& emitContext);

    /*
     * Load FieldToken/MethodToken/Typedef token
     */
    static void implementLoadToken(EmitContext& emitContext, mdToken token);

private:
    /*
     * Calling newobj string
     */
    static void loadNewString(EmitContext& emitContext, GlobalContext& globalContext,
                              const TokenIndex& string,
                              StackEntity& destNewString,
                              TemporaryStackHolderPtr& newreg);
};


#endif // __TBA_CLR_COMPILER_OPCODES_REGISTEREVALUATOROPCODES_H
