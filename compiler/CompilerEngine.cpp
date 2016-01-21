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
* CompilerEngine.cpp
*
* Implementation file
*
* Author: Elad Raz <e@eladraz.com>
*/
#include "compiler/stdafx.h"

#include "compiler/CompilerEngine.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerTrace.h"

#include "compiler/opcodes/ArrayOpcodes.h"
#include "compiler/opcodes/Bin32Opcodes.h"
#include "compiler/opcodes/CompilerOpcodes.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/ExceptionOpcodes.h"

bool CompilerEngine::handleSplit(EmitContext& emitContext, basicInput& instructionCache)
{
    // Determine the next instruction's index
    uint instructionIndex = instructionCache.getPointer() -
        emitContext.methodContext.getMethodStreamStartAddress();

    // Check for block split, if this offset is referred-to by another instruction in the method
    if (emitContext.methodRuntime.m_blockSplit.isSet(instructionIndex) &&
        (emitContext.currentBlock.getBlockID() != instructionIndex))
    {
        // Add a duplicate block at the new offset, and terminate the current block
        return true;
    }

    // No split - just continue the current block
    return false;
}

bool CompilerEngine::step(EmitContext& emitContext,
    basicInput& instructionCache,
    uint initializedDummyPosition)
{
    // Get apartment
    MethodRunnable& methodContext = emitContext.methodContext;
    MethodRuntimeBoundle& methodRuntime = emitContext.methodRuntime;
    MethodBlock& currentBlock = emitContext.currentBlock;
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();
    Apartment& apartment = *methodContext.getApartment();
    uint apartmentId = apartment.getUniqueID();

    // Get MethodRuntimeBoundle
    CompilerInterface& compiler = *methodRuntime.m_compiler;
    const LocalPositions& locals = methodRuntime.m_locals;
    const ArgumentsPositions& args = methodRuntime.m_args;
    // Get current stack
    Stack& stack = currentBlock.getCurrentStack();

    // Check for previous compiled code
    uint32 instructionIndex = initializedDummyPosition + instructionCache.getPointer() -
        methodContext.getMethodStreamStartAddress();

    if (initializedDummyPosition == 0)
    {
        if (methodRuntime.m_methodSet.isSet(instructionIndex))
        {
            // Close block
            // TODO! Split blocks
            CHECK_FAIL();
            currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON,
                instructionIndex);
            return true;
        }

        // Mark the current instruction as handled
        methodRuntime.m_methodSet.set(instructionIndex);
        // Add new instruction and it's line transaction.
        currentBlock.appendInstruction(instructionIndex, methodRuntime.m_compiler->
            getFirstPassPtr()->getCurrentBlockData().getSize());
    }

    // Read the instruction
    uint8 instructionPrefix;
    instructionCache.streamReadUint8(instructionPrefix);

    // Set to false for opcode extension
    bool  shouldExecuteSwitch = true;
    int   offset;

    // Gneeral purpose variables:
    TokenIndex token;
    mdToken mdtoken;
    uint16 u16;
    uint8 u8;  uint32 u32, size;
    int8  i8;  int32  i32; int64 i64;
    uint32 opIndex = instructionIndex;
    bool mBool;

    ElementType    type;
    StackEntity varEntity1(StackEntity::ENTITY_LOCAL, ConstElements::gVoid);
    StackEntity varEntity2(StackEntity::ENTITY_LOCAL, ConstElements::gVoid);
    TemporaryStackHolderPtr destination;

    cForkStreamPtr newStream;

    // Parse the instruction
    if (instructionPrefix == 0xFE)
    {
        shouldExecuteSwitch = false;
        instructionCache.streamReadUint8(instructionPrefix);
        switch (instructionPrefix)
        {
        case 0x01: // ceq     compare equal
            CompilerTraceOpcode("ceq" << endl);
            BinaryOpcodes::compare(emitContext, BinaryOpcodes::CMP_EQUAL);
            break;

        case 0x02: // cgt - compare greater than
            CompilerTraceOpcode("cgt" << endl);
            BinaryOpcodes::compare(emitContext, BinaryOpcodes::CMP_GREATER_THEN);
            break;

        case 0x03: // cgt.un - compare greater than, unsigned or unoredered
            CompilerTraceOpcode("cgt.un" << endl);
            BinaryOpcodes::compare(emitContext, BinaryOpcodes::CMP_GREATER_THEN_UNSIGNED);
            break;

        case 0x04: // clt     - Compare less then
            CompilerTraceOpcode("clt" << endl);
            BinaryOpcodes::compare(emitContext, BinaryOpcodes::CMP_LESS_THEN);
            break;

        case 0x05: // clt.un  - Compare less then, unsigned or unoredered
            CompilerTraceOpcode("clt.un" << endl);
            BinaryOpcodes::compare(emitContext, BinaryOpcodes::CMP_LESS_THEN_UNSIGNED);
            break;

        case 0x06: // ldftn
            instructionCache.streamReadUint32(u32);
            CompilerTraceOpcode("ldftn " << HEXDWORD(u32) << endl);
            CHECK_FAIL();
            break;

        case 0x07: // ldvirtftn
            instructionCache.streamReadUint32(u32);
            CompilerTraceOpcode("ldftn " << HEXDWORD(u32) << endl);
            CHECK_FAIL();
            break;

        case 0x09:  // ldarg
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("ldarg  #" << opIndex << endl);

            // Push arg #index
            varEntity1 = StackEntity(StackEntity::ENTITY_ARGUMENT, args.getArgumentStackVariableType(opIndex));
            varEntity1.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity1);
            break;

        case 0x0A:  // ldarga
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("ldarga  #" << opIndex << endl);

            // Push arg #opIndex
            varEntity1 = StackEntity(StackEntity::ENTITY_ARGUMENT_ADDRESS, args.getArgumentStackVariableType(opIndex));
            varEntity1.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity1);
            break;

        case 0x0B:  // starg
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("starg  #" << opIndex << endl);
            // And store value
            varEntity2 = StackEntity(StackEntity::ENTITY_ARGUMENT, args.getArgumentStackVariableType(opIndex));
            varEntity2.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity2);
            RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));
            CompilerOpcodes::pop2null(emitContext);
            stack.pop2null();
            break;

        case 0x0C: // ldloc
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("ldloc  #" << opIndex << endl);

            // Push local #opIndex
            varEntity1 = StackEntity(StackEntity::ENTITY_LOCAL, locals.getLocalStackVariableType(opIndex));
            varEntity1.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity1);
            break;

        case 0x0D: // ldloca
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("ldloca  #" << opIndex << endl);

            // Push local #opIndex
            type = locals.getLocalStackVariableType(opIndex);
            type.setPointerLevel(1);
            varEntity1 = StackEntity(StackEntity::ENTITY_LOCAL_ADDRESS, type);
            varEntity1.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity1);
            break;

        case 0x0E: // stloc
            instructionCache.streamReadUint16(u16);
            opIndex = u16;
            CompilerTraceOpcode("stloc  #" << opIndex << endl);
            varEntity2 = StackEntity(StackEntity::ENTITY_LOCAL, locals.getLocalStackVariableType(opIndex));
            varEntity2.getConst().setLocalOrArgValue(opIndex);
            stack.push(varEntity2);
            // And store value
            RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));
            CompilerOpcodes::pop2null(emitContext);
            stack.pop2null();

            break;

        case 0x0F: // localloc
            CompilerTraceOpcode("localloc" << endl);
            // Check that the context is unsafe
            checkUnsafe(methodRuntime);

            // Allocate the result register
            destination = TemporaryStackHolderPtr(new
                TemporaryStackHolder(currentBlock,
                ELEMENT_TYPE_PTR,
                compiler.getStackSize(),
                TemporaryStackHolder::TEMP_ONLY_REGISTER));

            // Read the number of bytes that should be allocated from the stack
            RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));

            // Generate a call to the compiler for local memory allocation
            compiler.localloc(destination->getTemporaryObject(),
                              stack.getArg(0).getStackHolderObject()->getTemporaryObject(),
                              stack.isEmpty());

            // And push back the result to the stack
            stack.getArg(0) = StackEntity(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
            stack.getArg(0).setStackHolderObject(destination);
            break;
        case 0x15: // initobj
            instructionCache.streamReadUint32(u32);
            CompilerTraceOpcode("initobj " << HEXDWORD(u32) << endl);

            // Get type
            resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);

            // Zeroing the struct members
            // addrsess of struct is on the stack

            // Calculate the memory size for the struct
            size = globalContext.getTypedefRepository().getTypeSize(type);

            //push zero
            varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
            varEntity1.getConst().setConstValue(0);
            stack.push(varEntity1);

            // Push size
            varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
            varEntity1.getConst().setConstValue(size);
            stack.push(varEntity1);

            // Call memset
            CallingConvention::call(emitContext, globalContext.getFrameworkMethods().getMemset());
            break;

        case 0x16: // constrained
            instructionCache.streamReadUint32(u32);
            CompilerTraceOpcode(".constrained " << HEXDWORD(u32) << endl);
            // Pushing vtbl, mark callvirt of using this vtbl and not object
            resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
            varEntity1 = StackEntity(StackEntity::ENTITY_TOKEN_ADDRESS, type);
            varEntity1.getConst().setTokenIndex(type.getClassToken());
            stack.push(varEntity1);
            break;

        case 0x1C: // sizeof
            instructionCache.streamReadUint32(u32);
            CompilerTraceOpcode("sizeof " << HEXDWORD(u32) << endl);

            resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
            size = globalContext.getTypedefRepository().getTypeSize(type);

            // Push size as unsigned int32
            varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
            varEntity1.getConst().setConstValue(size);
            stack.push(varEntity1);
            break;
        default:
            // Not ready!
            CompilerTraceOpcode("Missing instruction: 0xFE " << HEXDWORD(instructionPrefix) << endl);
            XSTL_THROW(ClrIllegalInstruction);
        }
    }

    if (shouldExecuteSwitch) switch (instructionPrefix)
    {
        // See Common language infrastructor (CLI)
        //     Partition III - CIL instruction set
    case 0:
        // The nop opcode
        CompilerTraceOpcode("nop" << endl);
        break;

    case 1:
        // The break opcode
        CompilerTraceOpcode("break" << endl);
        // TODO! Add trap for debugger
        break;

    case 2: case 3: case 4: case 5: // ldarg 0-3. Load argument into stack
    case 0x0E:                      // ldarg.s (uint8)
        // TODO!
        // 0xFE, 0x09
        opIndex = translateIndexEncoding(instructionPrefix, 2);
        if (opIndex != MAX_UINT32)
            CompilerTraceOpcode("ldarg." << opIndex << endl);
        else
        {
            // Read unsigned8 for argument position
            instructionCache.streamReadUint8(u8);
            opIndex = u8;
            CompilerTraceOpcode("ldarg.s  #" << opIndex << endl);
        }
        // Push arg #index
        varEntity1 = StackEntity(StackEntity::ENTITY_ARGUMENT, args.getArgumentStackVariableType(opIndex));
        varEntity1.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity1);
        break;

    case 6: case 7: case 8: case 9: // ldloc 0-3. Load local into stack
    case 0x11:                      // ldloc.s (uint8)
        opIndex = translateIndexEncoding(instructionPrefix, 6);
        if (opIndex != MAX_UINT32)
            CompilerTraceOpcode("ldloc." << opIndex << endl);
        else
        {
            // Read unsigned8 for argument position
            instructionCache.streamReadUint8(u8);
            opIndex = u8;
            CompilerTraceOpcode("ldloc.s  #" << opIndex << endl);
        }

        // Push local #index
        varEntity1 = StackEntity(StackEntity::ENTITY_LOCAL, locals.getLocalStackVariableType(opIndex));
        varEntity1.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity1);
        break;

    case 0x0A: case 0x0B: case 0x0C: case 0x0D: // stloc 0-3. Store stack into locals
    case 0x13:                                  // stloc (uint8)
        // Get local index
        opIndex = translateIndexEncoding(instructionPrefix, 0x0A);
        if (opIndex != MAX_UINT32)
            CompilerTraceOpcode("stloc." << opIndex << endl);
        else
        {
            // Read unsigned8 for argument position
            instructionCache.streamReadUint8(u8);
            opIndex = u8;
            CompilerTraceOpcode("stloc.s  #" << opIndex << endl);
        }

        varEntity2 = StackEntity(StackEntity::ENTITY_LOCAL, locals.getLocalStackVariableType(opIndex));
        varEntity2.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity2);
        // And store value
        RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));
        CompilerOpcodes::pop2null(emitContext);
        stack.pop2null();
        break;

    case 0x0F:          // ldarga.s
        // Get argument index
        instructionCache.streamReadUint8(u8);
        opIndex = u8;
        CompilerTraceOpcode("ldarga.s  #" << opIndex << endl);

        // Push arg #index
        varEntity1 = StackEntity(StackEntity::ENTITY_ARGUMENT_ADDRESS, args.getArgumentStackVariableType(opIndex));
        varEntity1.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity1);
        break;

    case 0x10:              // starg (uint8)
        // Get argument index
        instructionCache.streamReadUint8(u8);
        opIndex = u8;
        CompilerTraceOpcode("starg.s  #" << opIndex << endl);

        // And store value
        varEntity2 = StackEntity(StackEntity::ENTITY_ARGUMENT, args.getArgumentStackVariableType(opIndex));
        varEntity2.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity2);
        RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));
        CompilerOpcodes::pop2null(emitContext);
        stack.pop2null();
        break;

    // TODO! Any change here should also result in a change to:
    // case 0xFE 0x0D   // ldloca
    case 0x12: // ldloca.s
        // Get argument index
        instructionCache.streamReadUint8(u8);
        opIndex = u8;
        CompilerTraceOpcode("ldloca.s  #" << opIndex << endl);

        // Get the local variable position
        type = locals.getLocalStackVariableType(opIndex);
        type.setPointerLevel(1);
        varEntity1 = StackEntity(StackEntity::ENTITY_LOCAL_ADDRESS, type);
        varEntity1.getConst().setLocalOrArgValue(opIndex);
        stack.push(varEntity1);
        break;

    case 0x14:  // ldnull
        CompilerTraceOpcode("ldnull" << endl);
        // push a null (TYPE_0 variable)
        varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gVoidPtr);
        varEntity1.getConst().setConstValue(0);
        stack.push(varEntity1);
        break;

    case 0x15: // ldc.i4.m1        Push -1
    case 0x16: // ldc.i4.0         Push 0
    case 0x17: // ldc.i4.1         Push 1
    case 0x18: // ldc.i4.2         Push 2
    case 0x19: case 0x1A: case 0x1B: case 0x1C:
    case 0x1D: // ...
    case 0x1E: // ldc.i4.8         Push 8

    case 0x1F: // ldc.i4.s <int8> as int32
    case 0x20: // ldc.i4   <int32> as int32
        if (instructionPrefix == 0x1F)
        {
            // Read and convert
            instructionCache.streamReadInt8(i8);
            CompilerTraceOpcode("ldc.i4.s 0x" << HEXBYTE(i8) << endl);
            i32 = i8;
        } else if (instructionPrefix == 0x20)
        {
            // Read and convert
            instructionCache.streamReadInt32(i32);
            CompilerTraceOpcode("ldc.i4." << HEXDWORD(i32) << endl);
        } else
        {
            // Get the push number from -1 to 8
            i32 = instructionPrefix - 0x16;
            CompilerTraceOpcode("ldc.i4." << i32 << endl);
        }

        // Use i32
        varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gI4);
        varEntity1.getConst().setConstValue(i32);
        stack.push(varEntity1);
        break;

    case 0x21: // ldc.i8 <int64> as int64
        CompilerTraceOpcode("ldc.i8" << endl);
        instructionCache.streamReadInt64(i64);
#ifdef CLR_I8_ENABLE
        varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gI8);
        varEntity1.getConst().setConst64Value(i64);
        stack.push(varEntity1);
#else
        varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gI4);
        varEntity1.getConst().setConstValue((int)i64);
        stack.push(varEntity1);
#endif
        break;

#ifdef CLR_FLOAT_ENABLE
    case 0x22: // ldc.r4 as native float
    case 0x23: // ldc.r8 as native float
        if (instructionPrefix == 0x22)
        {
            CompilerTraceOpcode("ldc.r4" << endl);
            ClrR4Float m_r4Float;
            instructionCache.pipeRead(m_r4Float, sizeof(m_r4Float));
            m_float = floatLoadFromR4(m_r4Float);
        } else
        {
            CompilerTraceOpcode("ldc.r8" << endl);
            ClrR8Float m_r8Float;
            instructionCache.pipeRead(m_r8Float, sizeof(m_r8Float));
            m_float = floatLoadFromR8(m_r8Float);
        }
        CHECK_FAIL();
        break;
#else
    case 0x22: // ldc.r4 float32 as float
    case 0x23: // ldc.r8 float64 as float
        CompilerTraceOpcode("ldc.r - Not implemented!"<< endl);
        XSTL_THROW(ClrIllegalInstruction);
#endif // CLR_FLOAT_ENABLE

        // 0x24 is undefined
    case 0x25: // dup
        // duplicates the top element of the stack.
        CompilerTraceOpcode("dup" << endl);
        ObjectOpcodes::duplicateStack(emitContext);
        break;

    case 0x26: // pop
        // Remove the top element of the stack
        CompilerTraceOpcode("pop" << endl);
        CompilerOpcodes::pop2null(emitContext);
        break;

        // TODO!
        // case 0x27: // jmp <token> - Exit current method and jump to specified method
        // case 0x29: // calli
    case 0x2A: // ret
        CompilerTraceOpcode("ret" << endl);

        CompilerOpcodes::compilerReturn(emitContext);

        // Conclude the current block with RET
        currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_RETURN,
            MethodBlock::BLOCK_RET,
            instructionIndex);
        return true;

    case 0x28: // call     <methodToken>
    case 0x6F: // callvirt <methodToken>
        mBool = (instructionPrefix == 0x6F); // isVirtual
        // Read the method token
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode(((mBool) ? "callvirt" : "call") << " " << "(" << HEXDWORD(u32) << ")" << endl);
        // Check stack and push arguments.
        // Call into implementation
        CallingConvention::call(emitContext, buildTokenIndex(apartmentId, u32), CallingConvention::ThisBelowParams, mBool);
        break;

    case 0x2B:  // br.s <int8>    - Branch always, continue execuation from
    case 0x38:  // br   <int32>     new method location.
        offset = readOffset(instructionCache,
            instructionPrefix == 0x2B);
        CompilerTraceOpcode("br" <<
            ((instructionPrefix == 0x2B) ? ".s" : "") << endl);

        // Generate new block for the jump instruction.
        if (initializedDummyPosition == 0)
        {
            opIndex = initializedDummyPosition + instructionCache.getPointer() -
                methodContext.getMethodStreamStartAddress();
        }
        opIndex += offset;
        methodRuntime.AddMethodBlock(currentBlock, emitContext, opIndex);

        // Just mark the current block as conditional true execution.
        // When the method is conclude all the internal relations will be
        // resolved
        currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_ALWAYS, opIndex, opIndex - offset);
        return true;

    case 0x2C: // brfalse.s   brnull.s    brzero.s      <int8>
    case 0x39: // brfalse     brnull      brzero        <int32>
        {
            offset = readOffset(instructionCache,
                instructionPrefix == 0x2C);
            CompilerTraceOpcode("brfalse" <<
                ((instructionPrefix == 0x2c) ? ".s" : "") << endl);

            // Check for zero/null/false

            // Generate two new blocks for both conditions.
            if (initializedDummyPosition == 0)
            {
                opIndex = initializedDummyPosition + instructionCache.getPointer() -
                    methodContext.getMethodStreamStartAddress();
            }
            // Remove the current entry from tos(), but keep it for the evaluation
            // process
            methodRuntime.AddMethodBlock(currentBlock, emitContext, opIndex + offset, true);
            methodRuntime.AddMethodBlock(currentBlock, emitContext, opIndex, true);

            // Just mark the current block as conditional true execution.
            // When the method is conclude all the internal relations will be
            // resolved
            currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_ZERO,
                opIndex + offset, opIndex);
        }
        return true;

    case 0x2D: // brtrue.s   brinst.s   <int8>
    case 0x3A: // brtrue     brinst     <int32>
        {
            offset = readOffset(instructionCache,
                instructionPrefix == 0x2D);
            CompilerTraceOpcode("brtrue" << ((instructionPrefix == 0x2D) ? ".s " : " ") << HEXDWORD(offset) << endl);

            // Check for non zero/non null/true

            // Generate two new blocks for both conditions.
            if (initializedDummyPosition == 0)
            {
                opIndex = initializedDummyPosition + instructionCache.getPointer() -
                    methodContext.getMethodStreamStartAddress();
            }
            // Remove the current entry from tos(), but keep it for the evaluation
            // process
            methodRuntime.AddMethodBlock(currentBlock, emitContext, opIndex + offset, true);
            methodRuntime.AddMethodBlock(currentBlock, emitContext, opIndex, true);

            // Just mark the current block as conditional true execution.
            // When the method is conclude all the internal relations will be
            // resolved
            currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON_ZERO, opIndex + offset, opIndex);
        }
        return true;

    case 0x2E:  // beq.s  <int8>        branch on equal
    case 0x3B:  // beq    <int32>
        CompilerTraceOpcode("beq" <<
            ((instructionPrefix == 0x2E) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            instructionPrefix == 0x2E);

        // Perform the ceq instruction
        step(emitContext,
            *generateInstructionStream(ILASM_CEQ, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brtrue with offset offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRTRUE, offset),
            instructionCache.getPointer());

    case 0x2F:  // bge.s <int8>     branch on greater than or equal to
    case 0x3C:  // bge   <int32>
    case 0x34:  // bge.un.s         unsigned or managed pointer comparison
    case 0x41:  // bge.un           our implementation unit them both
        CompilerTraceOpcode("bge" <<
            ((instructionPrefix == 0x34) ? ".un.s" : "") <<
            ((instructionPrefix == 0x41) ? ".un" : "") <<
            ((instructionPrefix == 0x2F) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            (instructionPrefix == 0x2F) ||
            (instructionPrefix == 0x34));

        // Set to true if unsigned comparison should be in order
        mBool = ((instructionPrefix == 0x41) || (instructionPrefix == 0x34));

        // Perform the clt instruction
        step(emitContext,
            *generateInstructionStream(mBool ? ILASM_CLT_UN : ILASM_CLT, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brfalse with offset offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRFALSE, offset),
            instructionCache.getPointer());

    case 0x30:  // bgt.s <int8>    branch on greater than
    case 0x3D:  // bgt   <int32>
    case 0x35:  // bgt.un.s         unsigned or managed pointer comparison
    case 0x42:  // bgt.un           our implementation unit them both
        CompilerTraceOpcode("bgt" <<
            ((instructionPrefix == 0x35) ? ".un.s" : "") <<
            ((instructionPrefix == 0x42) ? ".un" : "") <<
            ((instructionPrefix == 0x30) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            (instructionPrefix == 0x30) ||
            (instructionPrefix == 0x35));

        // Set to true if unsigned comparison should be in order
        mBool = ((instructionPrefix == 0x42) || (instructionPrefix == 0x35));

        // Perform the cgt instruction
        step(emitContext,
            *generateInstructionStream(mBool ? ILASM_CGT_UN : ILASM_CGT, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brtrue with offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRTRUE, offset),
            instructionCache.getPointer());

    case 0x31:  // ble.s  <int8>     branch on less than or equal to
    case 0x3E:  // ble    <int32>
    case 0x36:  // ble.un.s         unsigned or managed pointer comparison
    case 0x43:  // ble.un           our implementation unit them both
        CompilerTraceOpcode("ble" <<
            ((instructionPrefix == 0x36) ? ".un.s" : "") <<
            ((instructionPrefix == 0x43) ? ".un" : "") <<
            ((instructionPrefix == 0x31) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            (instructionPrefix == 0x31) ||
            (instructionPrefix == 0x36));

        // Set to true if unsigned comparison should be in order
        mBool = ((instructionPrefix == 0x43) || (instructionPrefix == 0x36));

        // Perform the cgt instruction
        step(emitContext,
            *generateInstructionStream(mBool ? ILASM_CGT_UN : ILASM_CGT, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brfalse with offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRFALSE, offset),
            instructionCache.getPointer());

    case 0x32: // blt.s  <int8>     branch on less than
    case 0x3F: // blt    <int32>
    case 0x37: // blt.un.s         unsigned or unordered comparison
    case 0x44: // blt.un           our implementation unit them both
        CompilerTraceOpcode("blt" <<
            ((instructionPrefix == 0x37) ? ".un.s" : "") <<
            ((instructionPrefix == 0x44) ? ".un" : "") <<
            ((instructionPrefix == 0x32) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            (instructionPrefix == 0x32) ||
            (instructionPrefix == 0x37));

        // Recursive on recursive is invalid!
        CHECK(initializedDummyPosition == 0);

        // Set to true if unsigned comparison should be in order
        mBool = ((instructionPrefix == 0x44) || (instructionPrefix == 0x37));

        // Perform the clt instruction
        step(emitContext,
            *generateInstructionStream(mBool ? ILASM_CLT_UN : ILASM_CLT, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brtrue with offset offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRTRUE, offset),
            instructionCache.getPointer());

    case 0x33:  // bne.un.s <int8>   branch on not equal or unordered
    case 0x40:  // bne.un   <int32>
        CompilerTraceOpcode("bne.un" <<
            ((instructionPrefix == 0x33) ? ".s" : "") << endl);
        offset = readOffset(instructionCache,
            instructionPrefix == 0x33);

        // Recursive on recursive is invalid!
        CHECK(initializedDummyPosition == 0);

        // Perform the ceq instruction
        step(emitContext,
            *generateInstructionStream(ILASM_CEQ, 0),
            methodContext.getMethodStreamStartAddress() + instructionIndex);

        // Call brfalse with offset offset and RETURN!
        return step(emitContext,
            *generateInstructionStream(ILASM_BRFALSE, offset),
            instructionCache.getPointer());

    case 0x45: // switch
        CompilerTraceOpcode("switch" << endl);
        // Read the instruction switch table
        // Pop the jump position
        instructionCache.streamReadUint32(u32);
        CHECK_FAIL();
        break;

    case 0x46: // ldind.i1  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.i1" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gI1);
        break;

    case 0x47: // ldind.u1  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.u1" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gU1);
        break;

    case 0x48: // ldind.i2  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.i2" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gI2);
        break;

    case 0x49: // ldind.u2  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.u2" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gU2);
        break;

    case 0x4A: // ldind.i4  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.i4" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gI4);
        break;

    case 0x4B: // ldind.u4  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.u4" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gU4);
        break;

    case 0x4C: // ldind.i8.u8  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.i8.u8" << endl);
#ifdef CLR_I8_ENABLE
        ArrayOpcodes::ldind(emitContext, ConstElements::gU8);
#else
        ArrayOpcodes::ldind(emitContext, ConstElements::gU4);
#endif
        break;

    case 0x4D: // ldind.i  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.i" << endl);
        ArrayOpcodes::ldind(emitContext, ConstElements::gI);
        break;

#ifdef CLR_FLOAT_ENABLE
    case 0x4E: // ldind.r4  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.r4" << endl);
        CHECK_FAIL();
        break;
    case 0x4F: // ldind.r8  Load value indirect onto the stack
        CompilerTraceOpcode("ldind.r8" << endl);
        CHECK_FAIL();
        break;
#endif // CLR_FLOAT_ENABLE

    case 0x50: // ldind.ref  Load value indirect onto the stack (object)
        CompilerTraceOpcode("ldind.ref" << endl);
        ArrayOpcodes::ldind(emitContext, ElementType(ELEMENT_TYPE_OBJECT));
        break;

    case 0x51: // stind.i1  Store value of type int8 into memory at address
        CompilerTraceOpcode("stind.ref" << endl);
        ArrayOpcodes::stind(emitContext, ElementType(ELEMENT_TYPE_OBJECT));
        break;

    case 0x52: // stind.i1  Store value of type int8 into memory at address
        CompilerTraceOpcode("stind.i1" << endl);
        ArrayOpcodes::stind(emitContext, ConstElements::gI1);
        break;

    case 0x53: // stind.i2  Store value of type int16 into memory at address
        CompilerTraceOpcode("stind.i2" << endl);
        ArrayOpcodes::stind(emitContext, ConstElements::gI2);
        break;

    case 0x54: // stind.i4  Store value of type int32 into memory at address
        CompilerTraceOpcode("stind.i4" << endl);
        ArrayOpcodes::stind(emitContext, ConstElements::gI4);
        break;

    case 0x55: // stind.i8  Store value of type int64 into memory at address
        CompilerTraceOpcode("stind.i8" << endl);
#ifdef CLR_I8_ENABLE
        ArrayOpcodes::stind(emitContext, ConstElements::gI8);
#else
        ArrayOpcodes::stind(emitContext, ConstElements::gI4);
#endif
        break;

#ifdef CLR_FLOAT_ENABLE
    case 0x56: // stind.r4  Store value of type float32 into memory at address
        CompilerTraceOpcode("stind.r4" << endl);
        CHECK_FAIL();
        break;
    case 0x57: // stind.r8  Store value of type float64 into memory at address
        CompilerTraceOpcode("stind.r8" << endl);
        CHECK_FAIL();
        break;
#endif // CLR_FLOAT_ENABLE

    case 0xDF:
        CompilerTraceOpcode("stind.i" << endl);
        ArrayOpcodes::stind(emitContext, ConstElements::gI);
        break;

    case 0x58: // add
        CompilerTraceOpcode("add" << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_ADD);
        break;

    case 0x59: // sub
        CompilerTraceOpcode("sub" << endl);

        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_SUB);
        break;

    case 0x5A: // mul
    case 0xD9: // mul.ovf.un
        CompilerTraceOpcode("mul" << ((instructionPrefix == 0xD9) ? ".ovf.un" : "") << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_MUL);
        break;

    case 0x5B: // div
    case 0x5C: // div.un -> Divide two unsigned. Implemented inside the binop
        CompilerTraceOpcode("div" << ((instructionPrefix == 0x5C) ? ".un" : "") << endl);
        BinaryOpcodes::binary(emitContext, (instructionPrefix == 0x5C) ? BinaryOpcodes::BIN_DIV_UN : BinaryOpcodes::BIN_DIV);
        break;

    case 0x5D: // rem
    case 0x5E: // rem.un  -> Divide two unsigned. Implemented inside the binop
        CompilerTraceOpcode("rem" << ((instructionPrefix == 0x5E) ? ".un" : "") << endl);
        BinaryOpcodes::binary(emitContext, (instructionPrefix == 0x5E) ? BinaryOpcodes::BIN_REM_UN : BinaryOpcodes::BIN_REM);
        break;

    case 0x5F: // and
        CompilerTraceOpcode("and" << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_AND);
        break;

    case 0x60: // or
        CompilerTraceOpcode("or" << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_OR);
        break;

    case 0x61: // xor
        CompilerTraceOpcode("xor" << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_XOR);
        break;

    case 0x62: // shl
        CompilerTraceOpcode("shl" << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_SHL);
        break;

    case 0x63: // shr
    case 0x64: // shr.un   See div.un, rem.un comment
        CompilerTraceOpcode("shr" << ((instructionPrefix == 0x64) ? ".un" : "") << endl);
        BinaryOpcodes::binary(emitContext, BinaryOpcodes::BIN_SHR);
        break;

    case 0x65: // neg - Minus number
        CompilerTraceOpcode("neg" << endl);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        methodRuntime.m_compiler->neg32(stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        break;

    case 0x66: // not - bitwise not
        CompilerTraceOpcode("not" << endl);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        methodRuntime.m_compiler->not32(stack.getArg(0).getStackHolderObject()->getTemporaryObject());
        break;

    case 0x67: // conv.i1
        CompilerTraceOpcode("conv.i1" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I1);
        break;
    case 0x68: // conv.i2
        CompilerTraceOpcode("conv.i2" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I2);
        break;
    case 0x69: // conv.i4
        CompilerTraceOpcode("conv.i4" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I4);
        break;
    case 0x6A: // conv.i8
        CompilerTraceOpcode("conv.i8" << endl);
#ifdef CLR_I8_ENABLE
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I8);
#else
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I4);
#endif
        break;

#ifdef CLR_FLOAT_ENABLE
    case 0x6B: // conv.r4
        CompilerTraceOpcode("conv.r4" << endl);
        CHECK_FAIL();
        break;
    case 0x6C: // conv.r8
        CompilerTraceOpcode("conv.r8" << endl);
        CHECK_FAIL();
        break;
    case 0x76: // conv.r.un
        CompilerTraceOpcode("conv.r.un" << endl);
        CHECK_FAIL();
        break;
#endif // CLR_FLOAT_ENABLE

    case 0x6D: // conv.u4
        CompilerTraceOpcode("conv.u4" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U4);
        break;

    case 0x6E: // conv.u8
        CompilerTraceOpcode("conv.u8" << endl);
#ifdef CLR_I8_ENABLE
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U8);
#else
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U4);
#endif
        break;

        // 0x6F is callvirt. See call

    case 0x70: // cpobj <token>
        // Read the token of the object
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("cpobj " << HEXDWORD(u32) << endl);
        // Invoke either memcpy or storeVar (for objects)
        CHECK_FAIL();
        break;

    case 0x71: // ldobj <token>
        CompilerTraceOpcode("ldobj" << endl);
        // Read the token of the object
        instructionCache.streamReadUint32(u32);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
        // Read the variable address
        ArrayOpcodes::ldind(emitContext, type);
        break;

    case 0x72: // ldstr <token>
        CompilerTraceOpcode("ldstr" << endl);
        // Read the token of the string
        instructionCache.streamReadUint32(u32);

        switch (EncodingUtils::getTokenTableIndex((mdToken)u32))
        {
        case 0x70:
            varEntity1 = StackEntity(StackEntity::ENTITY_TOKEN_ADDRESS, ConstElements::gString);
            varEntity1.getConst().setTokenIndex(buildTokenIndex(apartmentId, u32));
            stack.push(varEntity1);

            // The string itself
            //offset = EncodingUtils::getTokenPosition((mdToken)u32);
            //newStream = apartment.getStreams().getUserStringsStream()->fork();
            //newStream->seek(offset, basicInput::IO_SEEK_SET);
            break;
        default:
            // Unknown string encoding
            XSTL_THROW(ClrIllegalInstruction);
        }
        break;

    case 0x73: // newobj <T>
        // Read the token of the object's constructor
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("newobj " << HEXDWORD(u32) << endl);
        mdtoken = apartment.getTables().getTypedefParent(u32);
        resolveTypeToken(mdtoken, apartmentId, globalContext.getTypedefRepository(), type);

        // Call GC's newobj (a framework method) to allocate a buffer for the object.
        // Note that the new object pointer will be placed at the top of the evaluation stack upon return
        ObjectOpcodes::implementNewObj(emitContext, type);

        // Call the object constructor (stored at u32)
        // Note that we pass ThisAboveParamsDup, so the new object's pointer is copied back into the evaluation stack
        // after the call
        CallingConvention::call(emitContext, buildTokenIndex(apartmentId, u32), CallingConvention::ThisAboveParamsDup);
        break;

    case 0x74: // castclass <T>
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("castclass " << HEXDWORD(u32) << endl);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);

        if (!type.isObject())
        {
            // Casting into native type.
            if (!type.isPointer() && !type.isSingleDimensionArray())
            {
                CompilerTraceOpcode("Unsafe convert to native (not pointer or array(!). Use unbox..." << endl);
                CHECK_FAIL();
            }
            stack.getArg(0).setElementType(type);
        } else
        {
            // duplicate object for casting
            ObjectOpcodes::duplicateStack(emitContext);

            // call c# check type
            // Pop the object and replace it with it's vtbl
            CallingConvention::call(emitContext,
                                    globalContext.getFrameworkMethods().getVtblObj());
            // Push the RTTI
            varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU2);
            varEntity1.getConst().setConstValue(globalContext.getTypedefRepository().getRTTI(type.getClassToken()));
            stack.push(varEntity1);

            // And call checkInstance()
            CallingConvention::call(emitContext,
                                    globalContext.getFrameworkMethods().checkInstance());

            // And do the cast
            stack.getArg(0).setElementType(type);
        }
        break;

    case 0x75: // isinst <T>
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("isinst " << HEXDWORD(u32) << endl);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);

        // Push the RTTI
        varEntity1 = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU2);
        varEntity1.getConst().setConstValue(globalContext.getTypedefRepository().getRTTI(type.getClassToken()));
        stack.push(varEntity1);

        // And call isInstance
        CallingConvention::call(emitContext,
                                globalContext.getFrameworkMethods().isInstance());
        break;


    // conv.r.un 0x76

    case 0xA5: // unbox.any
    case 0x79: // unbox
        CompilerTraceOpcode("unbox" << ((instructionPrefix == 0xA5) ? ".any" : "") << endl);
        // Read the token
        instructionCache.streamReadUint32(u32);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0));
        stack.getArg(0).setType(StackEntity::ENTITY_REGISTER_ADDRESS);
        // Change the element size to the unbox form
        type = stack.getArg(0).getElementType();
        ClrResolver::resolveUnboxType(emitContext.methodContext.getApartment(), buildTokenIndex(apartmentId, u32), type);
        stack.getArg(0).setElementType(type);
        break;

    case 0x7a: // throw               Throw an exception
        CompilerTraceOpcode("throw" << endl);
        ExceptionOpcodes::implementThrow(emitContext, instructionIndex);
        return true;

    case 0x7B: // ldfld <fieldToken>  Load field of an object
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("ldfld " << HEXDWORD(u32) << endl);
        token = buildTokenIndex(apartmentId, u32);

        // Adding the structure offset to the stack TOS address object
        implementLoadField(emitContext, token, stack.getArg(0));
        break;

    case 0x7C: // ldflda <fieldToken>  Load field address
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("ldflda " << HEXDWORD(u32) << endl);
        token = buildTokenIndex(apartmentId, u32);

        // Adding the structure offset to the stack TOS address object
        implementLoadField(emitContext, token, stack.getArg(0));
        stack.getArg(0).setType(StackEntity::ENTITY_REGISTER);
        break;

    case 0x7D: // stfld <fieldToken>   Store into a field of an object
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("stfld " << HEXDWORD(u32) << endl);
        token = buildTokenIndex(apartmentId, u32);

        implementLoadField(emitContext, token, stack.getArg(1));
        RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(0), stack.getArg(1));
        stack.pop2null();
        CompilerOpcodes::pop2null(emitContext);
        break;


    case 0x7E: // ldsfld <fieldToken>   Load static field of a class
        CompilerTraceOpcode("ldsfld" << endl);
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        token = buildTokenIndex(apartmentId, u32);
        // Check to see whether the class .cctor should be called

        // Add a call jump (if !initialize) to .cctor if there is one
        resolveTypeToken(apartment.getTables().getTypedefParent(u32), apartmentId, globalContext.getTypedefRepository(), type);

        emitStaticInitializer(emitContext, type);

        // Get the position of the static field.
        globalContext.getTypedefRepository().resolveFieldref(token, type.getClassToken());

        // Read static field to registry
        varEntity1 = StackEntity(StackEntity::ENTITY_ADDRESS_VALUE,
            globalContext.getTypedefRepository().getFieldType(token, ElementType::UnresolvedTokenIndex));
        varEntity1.getConst().setTokenIndex(token);
        stack.push(varEntity1);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0), true, 0);
        break;


    case 0x7F: // ldsflda <fieldToken>   Load static field address
        CompilerTraceOpcode("ldsflda" << endl);
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        token = buildTokenIndex(apartmentId, u32);
        // Check to see whether the class .cctor should be called
        // Add a call jump (if !initialize) to .cctor if there is one
        resolveTypeToken(apartment.getTables().getTypedefParent(u32), apartmentId, globalContext.getTypedefRepository(), type);
        emitStaticInitializer(emitContext, type);

        // Get the position of the static field.
        globalContext.getTypedefRepository().resolveFieldref(token, type.getClassToken());

        // Read static field to registry
        varEntity1 = StackEntity(StackEntity::ENTITY_ADDRESS_ADDRESS,
            globalContext.getTypedefRepository().getFieldType(token, ElementType::UnresolvedTokenIndex));
        varEntity1.getConst().setTokenIndex(token);
        stack.push(varEntity1);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0), true, 0);
        break;

    case 0x80: // stsfld <fieldToken>   Store a static field of a class
        CompilerTraceOpcode("stsfld" << endl);
        // Read the field-id for the new array
        instructionCache.streamReadUint32(u32);
        token = buildTokenIndex(apartmentId, u32);

        // Check to see whether the class .cctor should be called
        // Add a call jump (if !initialize) to .cctor if there is one
        resolveTypeToken(apartment.getTables().getTypedefParent(u32), apartmentId, globalContext.getTypedefRepository(), type);
        emitStaticInitializer(emitContext, type);

        // Get the position of the static field.
        globalContext.getTypedefRepository().resolveFieldref(token, type.getClassToken());

        varEntity1 = StackEntity(StackEntity::ENTITY_ADDRESS_ADDRESS,
                                   globalContext.getTypedefRepository().getFieldType(token, ElementType::UnresolvedTokenIndex));
        varEntity1.getConst().setTokenIndex(token);
        stack.push(varEntity1);
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, stack.getArg(0), true, 0);

        // And store
        RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));

        CompilerOpcodes::pop2null(emitContext);
        stack.pop2null();
        break;

    case 0x81: // stobj <token>
        CompilerTraceOpcode("stobj" << endl);
        // Read the token of the object
        instructionCache.streamReadUint32(u32);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
        ArrayOpcodes::stind(emitContext, type);
        break;

    case 0x8C: // box <valueType>
        CompilerTraceOpcode("box" << endl);
        // Read the typedef or ref for the new array
        instructionCache.streamReadUint32(u32);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);

        // Pop the value from the stack and evaluate it

        // Boxed into u32
        ObjectOpcodes::implementNewObj(emitContext, type);

        // Implement memory reference.
        // stack.getArg(0) = The new box object
        // stack.getArg(1) = The value assignment

        RegisterEvaluatorOpcodes::storeVar(emitContext, stack.getArg(1), stack.getArg(0));

        varEntity1 = stack.getArg(0);
        stack.pop2null();
        stack.pop2null();
        // The new box object is a returned entity
        ASSERT(varEntity1.isReturned());
        varEntity1.setType(StackEntity::ENTITY_REGISTER);
        stack.push(varEntity1);
        break;

    case 0x8D: // newarr <token>
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("newarr (type: " << HEXDWORD(u32) << ")" << endl);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
        ArrayOpcodes::handleNewArray(emitContext, type);
        break;

    case 0x8E: // ldlen      Push the length of array on the stack
        CompilerTraceOpcode("ldlen " << endl);
        ArrayOpcodes::handleLoadLength(emitContext);
        break;

    case 0x8F: // ldelema <T> Load address of an element of an array
        CompilerTraceOpcode("ldelema" << endl);
        instructionCache.streamReadUint32(u32);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
        ArrayOpcodes::handleLoadAddressOfAnElement(emitContext, type);
        break;

    case 0x90: // ldelem.i1
    case 0x91: // ldelem.u1
    case 0x92: // ldelem.i2
    case 0x93: // ldelem.u2
    case 0x94: // ldelem.i4
    case 0x95: // ldelem.u4
    case 0x96: // ldelem.i8 -> Not implemented
    case 0x97: // ldelem.i
    case 0x98: // ldelem.r4 -> Not implemented
    case 0x99: // ldelem.r8 -> Not implemented
        CompilerTraceOpcode("ldelem" << endl);
        ArrayOpcodes::handleLoadElement(emitContext);
        break;

    case 0xA3: // ldelem - TODO!
    case 0xA4: // stelem - TODO!
        instructionCache.streamReadUint32(u32);
        resolveTypeToken(u32, apartmentId, globalContext.getTypedefRepository(), type);
        CompilerTraceOpcode("st/ldelem not implemmented yet for " << HEXTOKEN(type.getClassToken()) << endl);
        CHECK_FAIL();
        break;

    case 0x9b: // stelem.i
    case 0x9c: // stelem.i1
    case 0x9d: // stelem.i2
    case 0x9e: // stelem.i4
    case 0x9f: // stelem.i8 -> Not implemented

    case 0xA0: // stelem.r4 -> Not implemented
    case 0xA1: // stelem.r8 -> Not implemented
        CompilerTraceOpcode("stelem" << endl);
        ArrayOpcodes::handleStoreElement(emitContext);
        break;

    case 0x9A: // ldelem.ref   Load the element of type object, at index onto
        //              the top of the stack as an O
        CompilerTraceOpcode("ldelem.ref" << endl);
        // Reference is automaticly done
        ArrayOpcodes::handleLoadElement(emitContext);
        break;

    case 0xA2: // stelem.ref
        CompilerTraceOpcode("stelem.ref" << endl);
        // Reference is automaticly done
        ArrayOpcodes::handleStoreElement(emitContext);
        break;

        // TODO!!! From B3  conv.ovf.i1
        // TODO!!! From B3
        // TODO!!! From B3
        // TODO!!! From B3

    case 0xD0: // ldtoken
        instructionCache.streamReadUint32(u32);
        CompilerTraceOpcode("ldtoken " << HEXDWORD(u32) << endl);
        RegisterEvaluatorOpcodes::implementLoadToken(emitContext, u32);
        break;

    case 0xD1: // conv.u2
        CompilerTraceOpcode("conv.u2" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U2);
        break;
    case 0xD2: // conv.u1
        CompilerTraceOpcode("conv.u1" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U1);
        break;

    case 0xD3: // conv.i
        CompilerTraceOpcode("conv.i" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_I);
        break;

    case 0xDC: // endfinally / endfault
        CompilerTraceOpcode("endfinally" << endl);

        ExceptionOpcodes::implementEndFinally(emitContext, instructionIndex);
        return true;

    case 0xDD: // leave <int32>    Exit a protected region of code.
    case 0xDE: // leave <int8>     Exit a protected region of code, short form
        offset = readOffset(instructionCache, instructionPrefix == 0xDE);
        CompilerTraceOpcode("leave" <<
            ((instructionPrefix == 0xDE) ? ".s" : "") << endl);

        // Calculate the target index
        if (initializedDummyPosition == 0)
        {
            opIndex = initializedDummyPosition + instructionCache.getPointer() -
                methodContext.getMethodStreamStartAddress();
        }
        opIndex += offset;

        if (!ExceptionOpcodes::implementLeave(emitContext, instructionIndex, opIndex))
        {
            // Bad "leave" instruction - treat like "br"
            return step(emitContext,
                *generateInstructionStream(ILASM_BR, offset),
                instructionCache.getPointer());
        }
        return true;

    case 0xE0: // conv.u
        CompilerTraceOpcode("conv.u" << endl);
        BinaryOpcodes::convert(emitContext, ELEMENT_TYPE_U);
        break;

    default:
        CompilerTrace("Missing instruction: " << HEXDWORD(instructionPrefix) << endl);
        XSTL_THROW(ClrIllegalInstruction);
    }

    // Instruction compiled. Check for block merging
    if (initializedDummyPosition == 0)
    {
        opIndex = instructionCache.getPointer() -
            methodContext.getMethodStreamStartAddress();
        if (methodRuntime.m_methodSet.isSet(opIndex))
        {
            // Terminate block, merging should be in progress
            CompilerTrace("Part of block owned by another block, merge." << endl);
            currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON,
                opIndex, opIndex);
            return true;
        }
    }

    // Method is not fully complete
    return false;
}

//////////////////////////////////////////////////////////////////////////

void CompilerEngine::implementLoadField(EmitContext& emitContext,
    TokenIndex& field,
    StackEntity& object)
{
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();
    const ElementType& type = globalContext.getTypedefRepository().getFieldType(field, ElementType::UnresolvedTokenIndex);
    globalContext.getTypedefRepository().resolveFieldref(field, type.getClassToken());

    uint offset = globalContext.getTypedefRepository().getFieldRelativePosition(
        field,
        ElementType::UnresolvedTokenIndex);

    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, object, true, offset);

    // Return the object to the stack
    TemporaryStackHolderPtr destination = object.getStackHolderObject();
    object = StackEntity(StackEntity::ENTITY_REGISTER_ADDRESS, type);
    object.setStackHolderObject(destination);
}

//////////////////////////////////////////////////////////////////////////
// jumpToAddress

void CompilerEngine::jumpToAddress(EmitContext& emitContext,
    int blockNumber,
    bool shortAddress)
{
    if (shortAddress)
        emitContext.methodRuntime.m_compiler->jumpShort(blockNumber);
    else
        emitContext.methodRuntime.m_compiler->jump(blockNumber);
}

void CompilerEngine::simpleConditionalJump(EmitContext& emitContext,
    int blockNumber,
    bool shortAddress,
    bool isZero)
{
    // Pop top-of-stack, exception will be thrown if the stack is invalid.
    StackEntity compareObject(emitContext.currentBlock.getCurrentStack().peek());
    emitContext.currentBlock.getCurrentStack().pop2null();

    // Evaluate and compare to zero or non-zero
    // COMPEXSTRUCT
    // TODO! Can be int*, managed object, managed pointer, unmanaged
    //       pointer
    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, compareObject);
    const StackLocation& reg = compareObject.getStackHolderObject()->getTemporaryObject();

    if (!shortAddress)
    {
        // Long form
        emitContext.methodRuntime.m_compiler->jumpCond(reg, blockNumber, isZero);
    } else
    {
        // Short form
        emitContext.methodRuntime.m_compiler->jumpCondShort(reg, blockNumber, isZero);
    }
}

//////////////////////////////////////////////////////////////////////////

void CompilerEngine::fixStack(EmitContext& emitContext,
    Stack& customStack)
{
    cList<StackEntity>::iterator currTos = emitContext.currentBlock.getCurrentStack().getTosPosition();
    cList<StackEntity>::iterator customTos = customStack.getTosPosition();

    while (true)
    {
        if ((currTos == emitContext.currentBlock.getCurrentStack().getList().end()) ||
            (customTos == customStack.getList().end()))
            break;

        // Check what is the difference.
        // TODO! Right now I'm only allowed to change the current block and the
        //       current stack, so some cases aren't handled.
        if ((*currTos).getType() == StackEntity::ENTITY_CONST)
        {
            // Switch the const to the other stack variable
            switch ((*customTos).getType())
            {
            case StackEntity::ENTITY_CONST:
                // TODO! Check that the consts are the same. If not switch to
                //       a register on both blocks.
                CHECK_FAIL();

            case StackEntity::ENTITY_LOCAL:
                // TODO! Check that the locals are the same. Otherwise add load
                //       variable to a register on both blocks.
                CHECK_FAIL();

            case StackEntity::ENTITY_REGISTER:
                {
                    // Allocate the same register as other stack
                    // Replace the stack pointer
                    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, *currTos);
                    // TODO! Check register to be the same
                    CHECK((*currTos).getStackHolderObject()->getTemporaryObject() ==
                        (*customTos).getStackHolderObject()->getTemporaryObject());
                }
                break;

            default:
                // TODO! Add case
                CHECK_FAIL();
            }
        } else if ((*currTos).getType() == StackEntity::ENTITY_REGISTER)
        {
            //if((*currTos).get
            if((*customTos).getType() == StackEntity::ENTITY_REGISTER)
            {
                //TODO: compare the m_stackLocation values
            }else
            {
                    CHECK_FAIL();
            }
        } else
        {
            // TODO! Add case
            CHECK_FAIL();
        }

        // Go to end element
        ++currTos;
        ++customTos;
    }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

uint CompilerEngine::translateIndexEncoding(uint8 opcode, uint8 opcodeBase)
{
    if ((opcode >= opcodeBase) &&
        (opcode < (opcodeBase + 4)))
    {
        return opcode - opcodeBase;
    }

    return MAX_UINT32;
}

int CompilerEngine::readOffset(basicInput& stream, bool shortForm)
{
    if (shortForm)
    {
        int8 offset;
        stream.streamReadInt8(offset);
        return offset;
    } else
    {
        int32 offset;
        stream.streamReadInt32(offset);
        return offset;
    }
}

cForkStreamPtr CompilerEngine::generateInstructionStream(
    ILasmInstruction instruction,
    int32 immediate)
{
    static const uint8 gCeq[]     = {0xFE, 0x01};
    static const uint8 gCgt[]     = {0xFE, 0x02};
    static const uint8 gCgtUn[]   = {0xFE, 0x03};
    static const uint8 gClt[]     = {0xFE, 0x04};
    static const uint8 gCltUn[]   = {0xFE, 0x05};
    static const uint8 gBrfalse[] = {0x39};
    static const uint8 gBrtrue[]  = {0x3A};
    static const uint8 gBr[]      = {0x38};

    cForkStreamPtr stream(new cMemoryStream());
    switch (instruction)
    {
    case ILASM_CEQ: stream->pipeWrite(gCeq, arraysize(gCeq)); break;
    case ILASM_CGT: stream->pipeWrite(gCgt, arraysize(gCgt)); break;
    case ILASM_CLT: stream->pipeWrite(gClt, arraysize(gClt)); break;
    case ILASM_CGT_UN: stream->pipeWrite(gCgtUn, arraysize(gCgtUn)); break;
    case ILASM_CLT_UN: stream->pipeWrite(gCltUn, arraysize(gCltUn)); break;

    case ILASM_BRFALSE:
        stream->pipeWrite(gBrfalse, arraysize(gBrfalse));
        stream->streamWriteInt32(immediate);
        break;
    case ILASM_BRTRUE:
        stream->pipeWrite(gBrtrue, arraysize(gBrtrue));
        stream->streamWriteInt32(immediate);
        break;
    case ILASM_BR:
        stream->pipeWrite(gBr, arraysize(gBr));
        stream->streamWriteInt32(immediate);
        break;
    default:
        // Unknown opcode
        CompilerTrace("generateInstructionStream: Unknown instruction!" << endl);
        CHECK_FAIL();
    }

    stream->seek(0, basicInput::IO_SEEK_SET);
    return stream;
}

void CompilerEngine::checkUnsafe(MethodRuntimeBoundle& methodRuntime)
{
    // TODO! Empty function. Implement the method...
}

void CompilerEngine::resolveTypeToken(mdToken u32,
    uint apartmentId,
    ResolverInterface& resolver,
    ElementType& outputType)
{
    outputType = ElementType(ELEMENT_TYPE_CLASS,
                             0, false, false, false,
        buildTokenIndex(apartmentId, u32));
    resolver.resolveTyperef(outputType);
}

void CompilerEngine::emitStaticInitializer(EmitContext& emitContext,
    const ElementType& tokenType)
{
    if (emitContext.methodContext.getApartment()->getObjects().
        getTypedefRepository().getStaticInitializerMethod(tokenType.getClassToken()) != ElementType::UnresolvedTokenIndex)
    {
        CallingConvention::call(emitContext, CallingConvention::buildCCTOR(tokenType.getClassToken()));
    }
}
