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

#ifndef __TBA_CLR_COMPILER_OPCODES_OBJECTOPCODES_H
#define __TBA_CLR_COMPILER_OPCODES_OBJECTOPCODES_H

/*
 * ObjectOpcodes.h
 *
 * Implements object operations such as, new-object, reference objects (both locals and args)
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
class ObjectOpcodes
{
public:
    /*
     * Insert a code for calling to gcNewObj() function for a specific 'objectId'
     * This function DOESN'T invoke constructur or any other memory manipulation.
     *
     * emitContext  - Method context. See EmitContext
     * objectType   - The token for the object to create
     *
     * The current stack will have a register on top which points to the object head
     */
    static void implementNewObj(EmitContext& emitContext,
                                ElementType& objectType);

    /*
     * Duplicate stack. Use smart duplicant for register duplication
     */
    static void duplicateStack(EmitContext& emitContext);
    static void duplicateStack(EmitContext& emitContext, StackEntity& source, bool shouldEvalLocals = false);
};

#endif // __TBA_CLR_COMPILER_OPCODES_OBJECTOPCODES_H
