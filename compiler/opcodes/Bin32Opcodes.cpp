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
 * Bin32Opcodes.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerTrace.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerEngine.h"
#include "compiler/opcodes/Bin32Opcodes.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"

void Bin32Opcodes::convert32(EmitContext& emitContext,
                           CorElementType coreType)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    // Pop the value, and convert it into 32 bit integer
    // TODO! Check for different types.
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));

    // Calculate convertion information
    uint size = emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().getTypeSize(ElementType(coreType));
    ElementType convType(coreType);
    bool signExtend = false;
    if (convType.isIntegerType())
    {
        signExtend = true;
    } else
    {
        CHECK(convType.isUnsignedIntegerType());
    }

    // And convert the number into 32 bit integer
    emitContext.methodRuntime.m_compiler->conv32(stack.getArg(0).getStackHolderObject()->getTemporaryObject(),
                                                size, signExtend);
    stack.getArg(0).setElementType(convType);
}

void Bin32Opcodes::binary32(EmitContext& emitContext,
                            BinaryOpcodes::BinaryOperation operation)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    // COMPLEXSTRUCT
    // TODO!!! Check binary operation types
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1)); // destinationEntity
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0)); // sourceEntity

    // TODO! Check for source constance and translate them into optimization
    StackLocation source = stack.getArg(0).getStackHolderObject()->getTemporaryObject();
    StackLocation destination = stack.getArg(1).getStackHolderObject()->getTemporaryObject();
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;

    // Evaluate operation
    switch (operation)
    {
    case BinaryOpcodes::BIN_ADD: compiler.add32(destination, source); break;
    case BinaryOpcodes::BIN_SUB: compiler.sub32(destination, source); break;
    case BinaryOpcodes::BIN_MUL: compiler.mul32(destination, source); break;
    case BinaryOpcodes::BIN_DIV: compiler.div32(destination, source, true); break;
    case BinaryOpcodes::BIN_REM: compiler.rem32(destination, source, true); break;
    case BinaryOpcodes::BIN_AND: compiler.and32(destination, source); break;
    case BinaryOpcodes::BIN_SHL: compiler.shl32(destination, source); break;
    case BinaryOpcodes::BIN_SHR: compiler.shr32(destination, source); break;
    case BinaryOpcodes::BIN_XOR: compiler.xor32(destination, source); break;
    case BinaryOpcodes::BIN_OR:  compiler.or32 (destination, source); break;
    // Unsigned operations
    case BinaryOpcodes::BIN_DIV_UN: compiler.div32(destination, source, false); break;
    case BinaryOpcodes::BIN_REM_UN: compiler.rem32(destination, source, false); break;
    default:
        // Not ready yet
        CHECK_FAIL();
    }

    // Source will auto-destructed, since the reference will decrease

    // Add back into stack
    stack.pop2null();
}

void Bin32Opcodes::compare32(EmitContext& emitContext,
                             BinaryOpcodes::ComparisonOperation operation)
{
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    // COMPLEXSTRUCT
    // TODO!!! Check binary operation types
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(1)); // destinationEntity
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0)); // sourceEntity

    // TODO! Check for source constance and translate them into optimization
    StackLocation source = stack.getArg(0).getStackHolderObject()->getTemporaryObject();
    StackLocation destination = stack.getArg(1).getStackHolderObject()->getTemporaryObject();
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;

    // Evaluate operation
    switch (operation)
    {
    case BinaryOpcodes::CMP_EQUAL: compiler.ceq32(destination, source); break;
    case BinaryOpcodes::CMP_GREATER_THEN:
    case BinaryOpcodes::CMP_GREATER_THEN_UNSIGNED:
        compiler.cgt32(destination, source, (operation == BinaryOpcodes::CMP_GREATER_THEN));
        break;
    case BinaryOpcodes::CMP_LESS_THEN:
    case BinaryOpcodes::CMP_LESS_THEN_UNSIGNED:
        compiler.clt32(destination, source, (operation == BinaryOpcodes::CMP_LESS_THEN));
        break;
    default:
        // Not ready yet
        CHECK_FAIL();
    }

    // Source will auto-destructed, since the reference will decrease

    // Remove the source entity
    stack.pop2null();
    // Change the destination entity into a boolean
    stack.getArg(0).setElementType(ConstElements::gBool);
}
