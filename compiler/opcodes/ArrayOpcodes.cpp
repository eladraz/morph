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
 * ArrayOpcodes.cpp
 *
  Author: Guy Geva
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerTrace.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerEngine.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/ArrayOpcodes.h"

void ArrayOpcodes::handleNewArray(EmitContext& emitContext,
                                  const ElementType& tokenType)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    // Number of elements is already on stack.

    // Push vtbl*
    StackEntity vtblEntitiy(StackEntity::ENTITY_TOKEN_ADDRESS, tokenType);
    vtblEntitiy.getConst().setTokenIndex(tokenType.getClassToken());
    stack.push(vtblEntitiy);

    // Call compilerInstanceNewArray()
    CallingConvention::call(emitContext,
                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().instanceArray1());

    // Change the type to Array: tokenType[]
    ElementType arrayType(tokenType);
    arrayType.convertToArray();
    stack.getArg(0).setElementType(arrayType);
}

uint ArrayOpcodes::arrayCalculateOffset(EmitContext& emitContext, ElementType& arrayInnerType, bool isLoad)
{
    /*
     *  Stack layout
     *
     *   +0 TOP:  Value (Store element)    +0 Index (Load element)
     *   +1       Index                    +1 Array
     *   +2       Array
     */

    Stack& stack = emitContext.currentBlock.getCurrentStack();
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    uint indexPos = INDEX_POS;
    uint arrayPos = ARRAY_POS;
    if (isLoad)
    {
        indexPos--;
        arrayPos--;
    }

    // Push the arguments compilerGetArrayOffset(Array arr, uint index, uint sizeofElements)
    StackEntity& arr = stack.getArg(arrayPos);
    StackEntity& index = stack.getArg(indexPos);

    // Calculate size of elements
    arrayInnerType = stack.getArg(arrayPos).getElementType();
    arrayInnerType.unconvertFromArray(); // Remove array attribute.
    uint sizeofElement = globalContext.getTypedefRepository().getTypeSize(arrayInnerType);

    stack.push(arr);     arr = StackEntity();
    stack.push(index);   index = StackEntity();

    StackEntity stackSizeofElement(StackEntity::ENTITY_CONST, ConstElements::gU);
    stackSizeofElement.getConst().setConstValue(sizeofElement);

    stack.push(stackSizeofElement);

    // Call compilerGetArrayOffset()
    CallingConvention::call(emitContext,
                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getArrayBuffer());

    // Now byte* pointer to start address is on the stack, change the type of the array
    arrayInnerType.setPointerLevel(arrayInnerType.getPointerLevel() + 1);
    stack.getArg(0).setElementType(arrayInnerType);

    return sizeofElement;
}

void ArrayOpcodes::handleStoreElement(EmitContext& emitContext)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    ElementType arrayInnerType;
    uint sizeofElement = arrayCalculateOffset(emitContext, arrayInnerType, false);

    // Save new object
    RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(VALUE_POS + CHANGE_OFFSET), stack.getArg(ARRAY_RET_POSITION), sizeofElement);

    stack.pop2null(); // array ret value
    stack.pop2null(); // value
    stack.pop2null(); // index
    stack.pop2null(); // array
}

void ArrayOpcodes::handleLoadElement(EmitContext& emitContext)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    ElementType arrayInnerType;
    uint sizeofElement = arrayCalculateOffset(emitContext, arrayInnerType, true);

    StackEntity ret(stack.peek());
    stack.pop2null(); // ret
    stack.pop2null(); // index
    stack.pop2null(); // array

    stack.push(ret);

    stack.getArg(0).setElementType(arrayInnerType);
    stack.getArg(0).setType(StackEntity::ENTITY_REGISTER);
    emitContext.methodRuntime.m_compiler->loadMemory(stack.peek().getStackHolderObject()->getTemporaryObject(),
                                                    stack.peek().getStackHolderObject()->getTemporaryObject(),
                                                    0,
                                                    sizeofElement,
                                                    RegisterEvaluatorOpcodes::getSignExtend32WithCheck(arrayInnerType));

}

void ArrayOpcodes::handleLoadAddressOfAnElement(EmitContext& emitContext,
                                                const ElementType& typeToken)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    ElementType arrayInnerType;
    uint sizeofElement = arrayCalculateOffset(emitContext, arrayInnerType, true);

    StackEntity ret(stack.peek());

    stack.pop2null(); // ret
    stack.pop2null(); // index
    stack.pop2null(); // array

    stack.push(ret);

    stack.getArg(0).setElementType(arrayInnerType);
    stack.getArg(0).setType(StackEntity::ENTITY_REGISTER);
}

void ArrayOpcodes::handleLoadLength(EmitContext& emitContext)
{
    CallingConvention::call(emitContext,
                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getNumElem(),
                            CallingConvention::ThisBelowParams);
}

////////////////////////////////////////////////////////////////////////////////


void ArrayOpcodes::ldind(EmitContext& emitContext, const ElementType& intType)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    CHECK(stack.getArg(0).getType() != StackEntity::ENTITY_CONST);
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0), true, 0, true);
    // Allocate new register
    TemporaryStackHolderPtr source = TemporaryStackHolderPtr(new TemporaryStackHolder(
                                emitContext.currentBlock,
                                intType.isUnsignedIntegerType() ? ELEMENT_TYPE_U : ELEMENT_TYPE_I,
                                emitContext.methodRuntime.m_compiler->getStackSize(),
                                TemporaryStackHolder::TEMP_ONLY_REGISTER));
    // Read value
    emitContext.methodRuntime.m_compiler->loadMemory(
                        stack.getArg(0).getStackHolderObject()->getTemporaryObject(),
                        source->getTemporaryObject(), 0,
                        emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().getTypeSize(intType),
                        RegisterEvaluatorOpcodes::getSignExtend32WithCheck(intType));
    stack.pop2null();

    // Put it back to stack
    StackEntity stackEntity = StackEntity(StackEntity::ENTITY_REGISTER, intType);
    stackEntity.setStackHolderObject(source);
    stack.push(stackEntity);
}

void ArrayOpcodes::stind(EmitContext& emitContext, const ElementType& intType)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    ElementType storeSize(intType);

    if (storeSize.isObjectAndNotValueType())
    {
        if (stack.getArg(1).getElementType().isObjectAndNotValueType())
        {
            // TODO! Incref/decref to the address stack.getArg(1)
            CompilerTrace("stind for object isn't implemented yet" << endl);
            CHECK_FAIL();
        } else
        {
            storeSize = stack.getArg(0).getElementType();
        }
    }

    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1), true); // address
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0), true); // value
    emitContext.methodRuntime.m_compiler->storeMemory(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),  // address
                                                     stack.getArg(0).getStackHolderObject()->getTemporaryObject(), 0,  // value
                                                     emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().getTypeSize(storeSize));
    stack.pop2null();
    stack.pop2null();
}
