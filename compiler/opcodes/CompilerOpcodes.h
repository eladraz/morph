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

#ifndef __TBA_CLR_COMPILER_OPCODES_COMPILEROPCODES_H
#define __TBA_CLR_COMPILER_OPCODES_COMPILEROPCODES_H

/*
 * CompilerOpcodes.h
 *
 * Implements native compiler instruction. Called by "Calling Convention" whenever a
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
class CompilerOpcodes
{
public:
    /*
     * Called by class CallingConvention
     * Execute a commands given by custom attribute. For example:
     *          [clrcore.CompilerOpcodes("nop")]
     *          public static string convertToString(System.Object o)
     * Will cause the compiler to do "nothing".
     *
     * Return "true" is the original function should be called or not.
     */
    static bool compilerOpcodes(EmitContext& emitContext,
                                const TokenIndex& methodToken,
                                const cString& opcode,
                                const MethodDefOrRefSignature& signature);

    static void compilerReturn(EmitContext& emitContext);

    // Pop an entity from the evaluation stack
    // If this entity is an object, and it was returned, and the object has no references - then destroy it
    static void pop2null(EmitContext& emitContext);
};

#endif // __TBA_CLR_COMPILER_OPCODES_COMPILEROPCODES_H
