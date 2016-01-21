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

#include "compiler/stdafx.h"
#include "compiler/CompilerTrace.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerEngine.h"
#include "compiler/opcodes/BinaryOpcodes.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/ArrayOpcodes.h"
#include "compiler/opcodes/CompilerOpcodes.h"

bool CompilerOpcodes::compilerOpcodes(EmitContext& emitContext,
                                      const TokenIndex& methodToken,
                                      const cString& opcode,
                                      const MethodDefOrRefSignature& methodSignature)
{
    Apartment& apartment = *(emitContext.methodContext.getApartment()->getApt(methodToken));
    MethodBlock& currentBlock = emitContext.currentBlock;
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;

    if (opcode.compare("getRTTI") == cString::EqualTo)
    {
        TokenIndex typedefParent = buildTokenIndex(getApartmentID(methodToken),
                                                   apartment.getTables().getTypedefParent(getTokenID(methodToken)));
        uint32 rttiObj = apartment.getObjects().getTypedefRepository().getRTTI(typedefParent);
        // Check that the function is static
        CompilerTrace("CallingConvention::call(): getRTTI - " << rttiObj << endl);
        // Function must be static
        CHECK(!methodSignature.isHasThis());

        // And push RTTI
        StackEntity rtti(StackEntity::ENTITY_CONST, ConstElements::gU2);
        rtti.getConst().setConstValue(rttiObj);
        currentBlock.getCurrentStack().push(rtti);
        return false;
    } else if (opcode.compare("nop") == cString::EqualTo)
    {
        // Do nothing. (Used for convertion, etc)
        CompilerTrace("CallingConvention::call(): Nop " << endl);
        return false;
    } else if (opcode.compare("storeVar") == cString::EqualTo)
    {
        CompilerTrace("CallingConvention::call(): storeVar" << endl);
        // Implement storeVar inside the compiler
        // Push size
        uint size = apartment.getObjects().getTypedefRepository().getTypeSize(
                        currentBlock.getCurrentStack().peek().getElementType());
        #ifndef CLR_UNICODE
        if (currentBlock.getCurrentStack().peek().getType() == StackEntity::ENTITY_ADDRESS_WITH_STRING_DATA)
        {
            // Change the push size to half
            size/= 2;
        }
        #endif CLR_UNICODE
        StackEntity sizeElement(StackEntity::ENTITY_CONST, ConstElements::gU4);
        sizeElement.getConst().setConstValue(size);
        currentBlock.getCurrentStack().push(sizeElement);
        CallingConvention::call(emitContext,
                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getMemcpy());
        return false;
    } else if (opcode.compare("add32_carry") == cString::EqualTo)
    {
        // public static uint add32ReturnCarry(out uint result, uint a1, uint a2)
        // Check signature
        CHECK(!methodSignature.isHasThis());
        CHECK(methodSignature.getParams().getSize() == 3);
        CHECK(methodSignature.getParams()[0] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[1] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[2] == ConstElements::gU4);
        CHECK(methodSignature.getReturnType() == ConstElements::gU4);

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));

        // a1+a2 -> store to register a1
        // Prepare return type (carry)
        TemporaryStackHolderPtr ret(new TemporaryStackHolder(
                                    currentBlock,
                                    ELEMENT_TYPE_I4,
                                    CompilerInterface::STACK_32,
                                    TemporaryStackHolder::TEMP_ONLY_REGISTER));
        compiler.xor32(ret->getTemporaryObject(), ret->getTemporaryObject());

        compiler.add32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        stack.pop2null();

        // Eval result argument address
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);

        compiler.adc32(ret->getTemporaryObject(), ret->getTemporaryObject());
        StackEntity carry(StackEntity::ENTITY_REGISTER, ConstElements::gU4);
        carry.setStackHolderObject(ret);
        stack.push(carry);
        return false;
    } else if (opcode.compare("sub32_carry") == cString::EqualTo)
    {
        // public static uint sub32WithBorrow(out uint result, uint a1, uint a2)
        // Check signature
        CHECK(!methodSignature.isHasThis());
        CHECK(methodSignature.getParams().getSize() == 3);
        CHECK(methodSignature.getParams()[0] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[1] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[2] == ConstElements::gU4);
        CHECK(methodSignature.getReturnType() == ConstElements::gU4);

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));

        // a1+a2 -> store to register a1
        // Prepare return type (carry)
        TemporaryStackHolderPtr ret(new TemporaryStackHolder(
                                    currentBlock,
                                    ELEMENT_TYPE_I4,
                                    CompilerInterface::STACK_32,
                                    TemporaryStackHolder::TEMP_ONLY_REGISTER));
        compiler.xor32(ret->getTemporaryObject(), ret->getTemporaryObject());

        compiler.sub32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        stack.pop2null();

        // Eval result argument address
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);

        compiler.adc32(ret->getTemporaryObject(), ret->getTemporaryObject());
        StackEntity carry(StackEntity::ENTITY_REGISTER, ConstElements::gU4);
        carry.setStackHolderObject(ret);
        stack.push(carry);
        return false;
    } else if (opcode.compare("adc32") == cString::EqualTo)
    {
        // public static void add32WithCarry(out uint resultHighWithCarry, out uint resultLow, uint high1, uint high2, uint low1, uint low2)
        // Check signature
        CHECK(!methodSignature.isHasThis());
        CHECK(methodSignature.getParams().getSize() == 6);
        CHECK(methodSignature.getParams()[0] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[1] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[2] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[3] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[4] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[5] == ConstElements::gU4);
        CHECK(methodSignature.getReturnType() == ConstElements::gVoid);

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));
        compiler.add32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        TemporaryStackHolderPtr low = stack.getArg(1).getStackHolderObject();
        stack.pop2null();
        stack.pop2null();

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));
        compiler.adc32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        TemporaryStackHolderPtr high = stack.getArg(1).getStackHolderObject();
        stack.pop2null();
        stack.pop2null();

        // Store low
        StackEntity object(StackEntity::ENTITY_REGISTER, ConstElements::gU4);
        object.setStackHolderObject(low);
        stack.push(object);
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);

        // Store high
        object.setStackHolderObject(high);
        stack.push(object);
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);
        return false;
    } else if (opcode.compare("sbb32") == cString::EqualTo)
    {
        // public static void sub32WithBorrow(out uint resultHighWithCarry, out uint resultLow, uint high1, uint high2, uint low1, uint low2)
        // Check signature
        CHECK(!methodSignature.isHasThis());
        CHECK(methodSignature.getParams().getSize() == 6);
        CHECK(methodSignature.getParams()[0] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[1] == ConstElements::gU4Ref);
        CHECK(methodSignature.getParams()[2] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[3] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[4] == ConstElements::gU4);
        CHECK(methodSignature.getParams()[5] == ConstElements::gU4);
        CHECK(methodSignature.getReturnType() == ConstElements::gVoid);

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));
        compiler.sub32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        TemporaryStackHolderPtr low = stack.getArg(1).getStackHolderObject();
        stack.pop2null();
        stack.pop2null();

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1));
        compiler.sbb32(stack.getArg(1).getStackHolderObject()->getTemporaryObject(),
                       stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        TemporaryStackHolderPtr high = stack.getArg(1).getStackHolderObject();
        stack.pop2null();
        stack.pop2null();

        // Store low
        StackEntity object(StackEntity::ENTITY_REGISTER, ConstElements::gU4);
        object.setStackHolderObject(low);
        stack.push(object);
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);

        // Store high
        object.setStackHolderObject(high);
        stack.push(object);
        ArrayOpcodes::stind(emitContext,ConstElements::gU4);
        return false;
    } else
    {
        CompilerTrace("CallingConvention::call(): Missing instruction! " <<
                        opcode << endl);
        return true;
    }

    // Missing return?!
    ASSERT(false);
}

void CompilerOpcodes::compilerReturn(EmitContext& emitContext)
{
    // Will be set to true in order to dref temporary reference object
    bool shouldDrefNotDestory = false;

    // Check whether the return value is an object
    // If so, increase reference
    if (!emitContext.currentBlock.getCurrentStack().isEmpty() &&
        emitContext.currentBlock.getCurrentStack().peek().getElementType().isObjectAndNotValueType())
    {
        // Call inc obj
        ObjectOpcodes::duplicateStack(emitContext, emitContext.currentBlock.getCurrentStack().getArg(0));
        CallingConvention::call(emitContext, emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getIncObj());
        shouldDrefNotDestory = true;
    }

    // Call cleanup routine, and pass the stack pointer as argument

    if (emitContext.methodRuntime.m_cleanupIndex != ElementType::UnresolvedTokenIndex)
    {

        if (emitContext.methodRuntime.m_compiler->getCompilerParameters().m_bSupportExceptionHandling)
        {
            // pop the cleanup routine from the exception stack, and execute it
            // Call clr's PopAndExec
            CallingConvention::call(emitContext, emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getPopAndExec());
        }
        else
        {
            cString dependencyTokenName(CallingConvention::serializedMethod(emitContext.methodRuntime.m_cleanupIndex));

            // Push the current base pointer
            emitContext.methodRuntime.m_compiler->pushArg32(emitContext.currentBlock.getBaseStackRegister());
            // And call the cleanup function directly
            emitContext.methodRuntime.m_compiler->call(dependencyTokenName, 1);
        }
    }

    if (shouldDrefNotDestory)
    {
        // Dref,
        ObjectOpcodes::duplicateStack(emitContext, emitContext.currentBlock.getCurrentStack().getArg(0));
        CallingConvention::call(emitContext, emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getDecNoDestroyObj());
    }

    // Load return value to the stack or register before jumping to the final "ret"
    RegisterEvaluatorOpcodes::loadReturnedValue(emitContext);
}

void CompilerOpcodes::pop2null(EmitContext& emitContext)
{
    StackEntity& entity = emitContext.currentBlock.getCurrentStack().getArg(0);
    if (entity.isReturned() && entity.getElementType().isObjectAndNotValueType())
        CallingConvention::call(emitContext, emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getDestNoRef());
    else
        emitContext.currentBlock.getCurrentStack().pop2null();
}
