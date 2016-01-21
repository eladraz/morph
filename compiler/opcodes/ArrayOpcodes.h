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

#ifndef __TBA_CLR_COMPILER_OPCODES_ARRAY_H
#define __TBA_CLR_COMPILER_OPCODES_ARRAY_H

/*
 * ArrayOpcodes.h
 *
 * Implements array operations
 *
 * Guy Geva
 *  2012
 */

#include "xStl/types.h"
#include "compiler/TemporaryStackHolder.h"
#include "compiler/StackEntity.h"
#include "compiler/EmitContext.h"


class ArrayOpcodes
{
public:
    static void handleNewArray(EmitContext& emitContext,
                               const ElementType& tokenType);

    static void handleStoreElement(EmitContext& emitContext);

    static void handleLoadElement(EmitContext& emitContext);

    static void handleLoadAddressOfAnElement(EmitContext& emitContext,
                                             const ElementType& typeToken);
    /*
     * Load length of array into stack
     * See ldlen.* instruction set
     *
     * emitContext      - Method context. See EmitContext
     */
    static void handleLoadLength(EmitContext& emitContext);


    /*
     * Load value indirect onto the stack,
     * See ldind.* instruction set
     *
     * emitContext      - Method context. See EmitContext
     * intType          - What type of memory dereference
     */
    static void ldind(EmitContext& emitContext, const ElementType& intType);

    /*
     * Store value indirect onto the stack,
     * See stind.* instruction set
     *
     * emitContext      - Method context. See EmitContext
     * intType          - What type of memory dereference
     */
    static void stind(EmitContext& emitContext, const ElementType& intType);

private:
    enum {
        VALUE_POS = 0,
        INDEX_POS = 1,
        CHANGE_OFFSET = 1,
        ARRAY_RET_POSITION = 0,
        ARRAY_POS = 2
    };
    /*
     * Stack presentation:
     *   TOP:  Value
     *         Index  -> Deleted
     *         Array  -> Convert to offset pointer
     *
     * Change Array = Array+Index*sizeof(type)
     *
     * Return sizeof(type)
     */
    static uint arrayCalculateOffset(EmitContext& emitContext, ElementType& arrayInnerType, bool isLoad);
};

#endif // __TBA_CLR_COMPILER_OPCODES_ARRAY_H
