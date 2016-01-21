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

/*
 * ObjectOpcodes.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerTrace.h"
#include "compiler/CallingConvention.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"

void ObjectOpcodes::implementNewObj(EmitContext& emitContext,
                                    ElementType& objectType)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    // Push vtbl
    StackEntity vtblEntitiy(StackEntity::ENTITY_TOKEN_ADDRESS, ConstElements::gVoidPtr);
    vtblEntitiy.getConst().setTokenIndex(objectType.getClassToken());
    stack.push(vtblEntitiy);

    // Allocate gc buffer
    CallingConvention::call(emitContext,
                            globalContext.getFrameworkMethods().getNewObj());
    // Change return type of object type
    ElementType ptrType(objectType);
    //ptrType.setPointerLevel(ptrType.getPointerLevel()+1);
    stack.getArg(0).setElementType(ptrType);
}

void ObjectOpcodes::duplicateStack(EmitContext& emitContext, StackEntity& source, bool shouldEvalLocals)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    if ((source.getType() != StackEntity::ENTITY_REGISTER) &&
        (source.getType() != StackEntity::ENTITY_REGISTER_ADDRESS))
    {
        // Make new register only if not address
        if ((source.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK) ||
            (source.getType() == StackEntity::ENTITY_TOKEN_ADDRESS) ||
            ((shouldEvalLocals) && (source.getType() == StackEntity::ENTITY_LOCAL)) ||
            ((shouldEvalLocals) && (source.getType() == StackEntity::ENTITY_ARGUMENT)))
        {
            RegisterEvaluatorOpcodes::evaluateInt32(emitContext, source);
            // And fault to register duplication
        } else
        {
            stack.push(source);
            // It's a duplicate, not returned
            stack.getArg(0).setReturned(false);
            return;
        }
    }

    // And duplicate register
    // If the compiler is optimizer, allocate a real register and do a move.
    TemporaryStackHolder::StackHolderAllocationRequest eRequest = TemporaryStackHolder::TEMP_ONLY_STACK;
    StackEntity::EntityType eEntityType = (source.getType() == StackEntity::ENTITY_REGISTER_ADDRESS) ? StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS : StackEntity::ENTITY_LOCAL_TEMP_STACK;

    if (emitContext.methodRuntime.m_compiler->isOptimizerCompiler())
    {
        eRequest = TemporaryStackHolder::TEMP_ONLY_REGISTER;
        eEntityType = (source.getType() == StackEntity::ENTITY_REGISTER_ADDRESS) ? StackEntity::ENTITY_REGISTER_ADDRESS : StackEntity::ENTITY_REGISTER;
    }

    // Save variable on stack
    StackEntity dupStack(eEntityType, source.getElementType(), source.isReturned());

    TemporaryStackHolderPtr newDest(new TemporaryStackHolder(
                            emitContext.currentBlock,
                            ELEMENT_TYPE_I4,
                            CompilerInterface::STACK_32,
                            eRequest));

    // Implement the MOV
    if (newDest->getTemporaryObject().u.flags == STACK_LOCATION_FLAGS_LOCAL)
    {
        emitContext.methodRuntime.m_compiler->store32(newDest->getTemporaryObject().u.reg,
                                                 emitContext.methodRuntime.m_compiler->getStackSize(),
                                                 source.getStackHolderObject()->getTemporaryObject(),
                                                 false,
                                                 true);
    }
    else
    {
        emitContext.methodRuntime.m_compiler->move32(newDest->getTemporaryObject(),
                                                     source.getStackHolderObject()->getTemporaryObject(),
                                                     emitContext.methodRuntime.m_compiler->getStackSize(),
                                                     false);
    }

    dupStack.setStackHolderObject(newDest);
    stack.push(source);
    source = dupStack;
    // The new duplicate was not returned
    stack.getArg(0).setReturned(false);
}

void ObjectOpcodes::duplicateStack(EmitContext& emitContext)
{
    duplicateStack(emitContext, emitContext.currentBlock.getCurrentStack().getArg(0), true);
}
