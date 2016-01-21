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
#include "compiler/CompilerTrace.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerEngine.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/ObjectOpcodes.h"

bool RegisterEvaluatorOpcodes::getSignExtend32WithCheck(const ElementType& var)
{
    bool signExtend = false;

    if (var.isIntegerType())
    {
        signExtend = true;
        CHECK(var.getType() != ELEMENT_TYPE_I8);
    } else if (var.isUnsignedIntegerType())
    {
        CHECK(var.getType() != ELEMENT_TYPE_U8);
    } else if (var.isPointer())
    {
        // Pointer will be treated as unsigned numbers.
    }
    else
    {
        CHECK(var.isChar() || var.isBool() || var.isObject());
    }

    return signExtend;
}

void RegisterEvaluatorOpcodes::convertLocalOrArgToLocArgAddress(EmitContext& emitContext,
                                                                StackEntity& entity)
{
    if (entity.getType() == StackEntity::ENTITY_LOCAL)
    {
        entity.setType(StackEntity::ENTITY_LOCAL_ADDRESS);
    } else if (entity.getType() == StackEntity::ENTITY_ARGUMENT)
    {
        entity.setType(StackEntity::ENTITY_ARGUMENT_ADDRESS);
    } else if (entity.getType() == StackEntity::ENTITY_REGISTER_ADDRESS)
    {
        entity.setType(StackEntity::ENTITY_REGISTER);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR)
    {
        return;
    } else
    {
        CHECK_FAIL();
    }
}

void RegisterEvaluatorOpcodes::evaluateInt32(EmitContext& emitContext,
                                             StackEntity& entity,
                                             bool shortUse,
                                             uint offset,
                                             bool shouldEvalToRegAddress)
{
    MethodRunnable& methodContext = emitContext.methodContext;
    MethodRuntimeBoundle& methodRuntime = emitContext.methodRuntime;
    MethodBlock& currentBlock = emitContext.currentBlock;
    GlobalContext& globalContext = methodContext.getApartment()->getObjects();

    // Test for pre-compiled token
    if (entity.getType() == StackEntity::ENTITY_REGISTER)
    {
        // ENTITY_REGISTER := a value stored in a register
        methodRuntime.m_compiler->addConst32(entity.getStackHolderObject()->getTemporaryObject(), offset); // If the offset is 0, nothing will happen
        return;
    }

    // TODO! Optimized this
    if (entity.getType() == StackEntity::ENTITY_REGISTER_ADDRESS)
    {
        // ENTITY_REGISTER_ADDRESS := an address stored in a register, of a memory value
        // The size of the token is in ElementType and cached (for converting purposes) as const value
        if (!shouldEvalToRegAddress)
        {
            methodRuntime.m_compiler->loadMemory(entity.getStackHolderObject()->getTemporaryObject(),
                                                entity.getStackHolderObject()->getTemporaryObject(),
                                                0,
                                                globalContext.getTypedefRepository().getTypeSize(entity.getElementType()),
                                                getSignExtend32WithCheck(entity.getElementType()));
        }
        // If the offset is 0, nothing will happen
        methodRuntime.m_compiler->addConst32(entity.getStackHolderObject()->getTemporaryObject(), offset);
        // Unmark as pointer
        if (!shouldEvalToRegAddress)
            entity.setType(StackEntity::ENTITY_REGISTER);
        return;
    }

    // TODO! Wrap this block with try-catch and in case of failure store old
    //       stack register
    // Allocate new register
    TemporaryStackHolderPtr ret(new TemporaryStackHolder(
                                    currentBlock,
                                    ELEMENT_TYPE_I4,
                                    CompilerInterface::STACK_32,
                                    TemporaryStackHolder::TEMP_ONLY_REGISTER));

    // If exception will thrown here then the entire function will failed.
    // There is no need to catch the following block and free 'ret'
    if (entity.getType() == StackEntity::ENTITY_ARGUMENT_ADDRESS)
    {
        // Get information from locals
        int argIndex = entity.getConst().getLocalOrArgValue();
        const ElementType& argType = methodRuntime.m_args.getArgumentStackVariableType(argIndex);
        uint pos = methodRuntime.m_args.getArgumentPosition(argIndex);
        uint size = globalContext.getTypedefRepository().getTypeSize(argType);

        // And load variable
        methodRuntime.m_compiler->load32addr(pos, size, offset, ret->getTemporaryObject(), true);

        // TODO! Mark type as pointer
        entity = StackEntity(StackEntity::ENTITY_REGISTER, argType);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL_ADDRESS)
    {
        // Get information from locals
        int localIndex = entity.getConst().getLocalOrArgValue();
        ElementType localType = methodRuntime.m_locals.getLocalStackVariableType(localIndex);
        uint pos = methodRuntime.m_locals.getLocalPosition(localIndex);
        uint size = globalContext.getTypedefRepository().getTypeSize(localType);

        // And load variable
        methodRuntime.m_compiler->load32addr(pos, size, offset,
                                            ret->getTemporaryObject(), false);
        //methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset);
        // Mark type as pointer
        localType.setPointerLevel(1);
        entity = StackEntity(StackEntity::ENTITY_REGISTER, localType);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL)
    {
        // Get information from locals
        mdToken localIndex = entity.getConst().getLocalOrArgValue();
        const ElementType& localType = methodRuntime.m_locals.getLocalStackVariableType(localIndex);
        uint pos = methodRuntime.m_locals.getLocalPosition(localIndex);
        uint size = globalContext.getTypedefRepository().getTypeSize(localType);

        bool signExtend = getSignExtend32WithCheck(localType);

        // TODO! Complexstruct
        if (size > (uint)methodRuntime.m_compiler->getStackSize())
        {
            CompilerTrace("Large complex structure with locals not supported yet" << endl);
            //do this so the local will be loaded as an address
            size = (uint)methodRuntime.m_compiler->getStackSize();
        }

        // And load variable
        methodRuntime.m_compiler->load32(pos, size,
                                        ret->getTemporaryObject(),
                                        signExtend, false, false);
        methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset); // If the offset is 0, nothing will happen

        entity = StackEntity(StackEntity::ENTITY_REGISTER, localType);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS)
    {
        // Just load the stack variable
        uint size = globalContext.getTypedefRepository().getTypeSize(entity.getElementType());
        bool signExtend = getSignExtend32WithCheck(entity.getElementType());

        methodRuntime.m_compiler->load32(entity.getStackHolderObject()->getTemporaryObject().u.reg,
                                        methodRuntime.m_compiler->getStackSize(),
                                        ret->getTemporaryObject(),
                                        false, false, true);
        if (!shouldEvalToRegAddress)
        {
            methodRuntime.m_compiler->loadMemory(ret->getTemporaryObject(),
                                                ret->getTemporaryObject(), 0,
                                                size,
                                                signExtend);
        }
        methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset);
        entity.setType(StackEntity::ENTITY_REGISTER);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK)
    {
        // Just load the stack variable
        uint size = globalContext.getTypedefRepository().getTypeSize(entity.getElementType());
        bool signExtend = getSignExtend32WithCheck(entity.getElementType());

        methodRuntime.m_compiler->load32(
                            entity.getStackHolderObject()->getTemporaryObject().u.reg,
                            size,
                            ret->getTemporaryObject(),
                            signExtend,
                            false,
                            true);
        methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset);
        entity.setType(StackEntity::ENTITY_REGISTER);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR)
    {
        uint size = globalContext.getTypedefRepository().getTypeSize(entity.getElementType());
        uint stackPosition = entity.getStackHolderObject()->getTemporaryObject().u.reg;
        // Just load the stack variable
        methodRuntime.m_compiler->load32addr(
                            stackPosition,
                            size,
                            offset,
                            ret->getTemporaryObject(),
                            false,
                            true);
        entity.setType(StackEntity::ENTITY_REGISTER);
        entity.setStackHolderObject(ret);
        // Fall to incobj reference
    } else if (entity.getType() == StackEntity::ENTITY_ARGUMENT)
    {
        // Get information from arguments
        int argIndex = entity.getConst().getLocalOrArgValue();
        const ElementType& argType = methodRuntime.m_args.getArgumentStackVariableType(argIndex);
        uint size = globalContext.getTypedefRepository().getTypeSize(argType);

        // For pointers/reference
        if (size == 0)
        {
            size = methodRuntime.m_args.getArgumentStackSize(argIndex);
        }
        uint pos = methodRuntime.m_args.getArgumentPosition(argIndex);
        bool signExtend = false;

        if (!argType.isObjectAndNotValueType())
        {
            signExtend = getSignExtend32WithCheck(argType);
            // TODO! ComplexStruct. Move as reference
            if (size > 4)
            {
                CompilerTrace("Large complex structure with arguments not supported yet" << endl);
                CHECK_FAIL();
            }
            // And load variable
            methodRuntime.m_compiler->load32(pos, size,
                                            ret->getTemporaryObject(),
                                            signExtend,
                                            true, false);
        } else
        {
            methodRuntime.m_compiler->load32(pos, methodRuntime.m_args.getArgumentStackSize(argIndex),
                                            ret->getTemporaryObject(),
                                            signExtend,
                                            true, false);
        }

        methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset); // If the offset is 0, nothing will happen
        entity = StackEntity(StackEntity::ENTITY_REGISTER, argType);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_CONST)
    {
        // TODO! Should be optimizied and depends on the operation.
        methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(),
                                           (uint32)entity.getConst().getConstValue());
        methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset);
        entity.setType(StackEntity::ENTITY_REGISTER);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_ADDRESS_VALUE)
    {
        // Add dependency
        const ElementType& type = entity.getElementType();
        uint size = globalContext.getTypedefRepository().getTypeSize(type);
        bool signExtend = getSignExtend32WithCheck(type);

        methodRuntime.m_compiler->load32(ret->getTemporaryObject(),
                                        size, signExtend,
                                        CallingConvention::serializeToken(entity.getConst().getTokenIndex()));
        entity = StackEntity(StackEntity::ENTITY_REGISTER, type);
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_ADDRESS_ADDRESS)
    {
        // Add dependency
        methodRuntime.m_compiler->load32addr(ret->getTemporaryObject(),
                                            CallingConvention::serializeToken(entity.getConst().getTokenIndex()));
        entity = StackEntity(StackEntity::ENTITY_REGISTER_ADDRESS, entity.getElementType());
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_ADDRESS_WITH_STRING_DATA)
    {
        // Add dependency
        methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(),
                                           StringRepository::serializeString(entity.getConst().getTokenIndex()));
        entity = StackEntity(StackEntity::ENTITY_REGISTER, entity.getElementType());
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_ADDRESS_WITH_DATA)
    {
        // Add dependency
        methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(),
                                            CallingConvention::serializeGlobalData(entity.getConst().getConstValue()));
        entity = StackEntity(StackEntity::ENTITY_REGISTER, entity.getElementType());
        entity.setStackHolderObject(ret);
    } else if (entity.getType() == StackEntity::ENTITY_TOKEN_ADDRESS)
    {
        // Add dependency
        TokenIndex token(entity.getConst().getTokenIndex());
        if (EncodingUtils::getTokenTableIndex(getTokenID(token)) == 0x70)
        {
            // Free register
            loadNewString(emitContext, globalContext, token, entity, ret);
            // The new string object might be a local-temp, reevaluate...
            evaluateInt32(emitContext, entity, true, 0); // , true -> TODO! IncObj Memory loss
        } else
        {
            methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(),
                                               CallingConvention::serializeToken(token));
            methodRuntime.m_compiler->addConst32(ret->getTemporaryObject(), offset);
            entity = StackEntity(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
            entity.setStackHolderObject(ret);
        }
    } else if (entity.getType() == StackEntity::ENTITY_METHOD_ADDRESS)
    {
        // Load method signature
        if (entity.getConst().getTokenIndex() == ElementType::UnresolvedTokenIndex)
        {
            // In case, evaluating a zero method token,
            // I dont want to serialize. because.
            methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(), 0);
        } else
        {
            /*
            MethodDefOrRefSignaturePtr signature =
                CallingConvention::readMethodSignature(*methodContext.getApartment()->getApt(entity.getConst().getTokenIndex()),
                                                       getTokenID(entity.getConst().getTokenIndex()));
             */
            // Add dependency
            methodRuntime.m_compiler->loadInt32(ret->getTemporaryObject(),
                                CallingConvention::serializedMethod(entity.getConst().getTokenIndex()));
        }
        entity = StackEntity(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
        entity.setStackHolderObject(ret);
    } else
    {
        // Unknown
        CompilerTrace("CompilerEngine::evaluateInt32(methodContext, ): ERROR! Unknown type!" << endl);
        CHECK_FAIL();
    }
}

void RegisterEvaluatorOpcodes::storeVar(EmitContext& emitContext,
                                        StackEntity& source,
                                        StackEntity& destination,
                                        uint operandSize)
{
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;
    Stack& stack = emitContext.currentBlock.getCurrentStack();
    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    uint position = 0;
    uint size = 0;
    bool isArg = false, isRegister = false;
    int index;
    bool shouldDecReference = destination.getElementType().isObjectAndNotValueType() || source.getElementType().isObjectAndNotValueType();

    // Solve the array[offset] = null deref.
    if (destination.getElementType().getPointerLevel() > 0)
    {
        ElementType innerType(destination.getElementType());
        innerType.setPointerLevel(innerType.getPointerLevel() - 1);
        shouldDecReference |= innerType.isObjectAndNotValueType();
    }

    ElementType referenceType(destination.getElementType());
    // TODO!
    switch (destination.getType())
    {
    case StackEntity::ENTITY_REGISTER:
        if (referenceType.isPointer())
        {
            referenceType.setPointerLevel(referenceType.getPointerLevel() - 1);
        }
        break;
    }

    uint destinationSize = (operandSize != 0) ? operandSize : globalContext.getTypedefRepository().getTypeSize(referenceType);

    // Decrease old destination reference count
    if (shouldDecReference)
    {
        // Load destination into the stack
        //evaluateInt32(emitContext, destination, true, 0, true);
        ObjectOpcodes::duplicateStack(emitContext, destination);

        if (destination.getElementType().getPointerLevel() > 0)
        {
            // Check for memory refrence
            evaluateInt32(emitContext, stack.getArg(0));
            compiler.loadMemory(stack.peek().getStackHolderObject()->getTemporaryObject(),
                                stack.peek().getStackHolderObject()->getTemporaryObject(), 0,
                                destinationSize);
        }
        CallingConvention::call(emitContext, globalContext.getFrameworkMethods().getDecObj());
        // Due to decRef/eval-source, dest can be moved to local-temp register.
    }

    switch (destination.getType())
    {
    case StackEntity::ENTITY_LOCAL:
        isArg = false; isRegister = false;
        index = destination.getConst().getLocalOrArgValue();
        position = emitContext.methodRuntime.m_locals.getLocalPosition(index);
        size = globalContext.getTypedefRepository().getTypeSize(
                        emitContext.methodRuntime.m_locals.getLocalStackVariableType(index));
        break;

    case StackEntity::ENTITY_ARGUMENT:
        isArg = true; isRegister = false;
        index = destination.getConst().getLocalOrArgValue();
        position = emitContext.methodRuntime.m_args.getArgumentPosition(index);
        size = globalContext.getTypedefRepository().getTypeSize(
                        emitContext.methodRuntime.m_args.getArgumentStackVariableType(index));
        break;

    case StackEntity::ENTITY_LOCAL_TEMP_STACK:
        evaluateInt32(emitContext, destination, true);
        isRegister = true;
        break;

    case StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR:
        evaluateInt32(emitContext, destination, true);
        isRegister = true;
        break;

    case StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS:
        evaluateInt32(emitContext, destination, true, 0, true);
        isRegister = true;
        break;

    case StackEntity::ENTITY_REGISTER_ADDRESS:
    case StackEntity::ENTITY_REGISTER:
        isRegister = true;
        break;

    default:
        CHECK_FAIL();
    };

    switch (source.getType())
    {
    case StackEntity::ENTITY_CONST:
            // Check size
            ASSERT(destinationSize <= 4);
            // Store a local variable
            if (isRegister)
            {
                evaluateInt32(emitContext, source);
                compiler.storeMemory(destination.getStackHolderObject()->getTemporaryObject(),
                                     source.getStackHolderObject()->getTemporaryObject(),
                                     0,
                                     destinationSize);
            } else
            {
                int value = source.getConst().getConstValue();
                compiler.storeConst(position,
                                    (const uint8*)&value,
                                    destinationSize,
                                    isArg);
            }
        break;

    case StackEntity::ENTITY_REGISTER:
        // 64BITNOTE:
        // TODO! Stack variable can be also 64 bit value, please check the
        //       default machine size for more information
        // TODO! See case StackEntity::ENTITY_LOCAL. Duplicate code
        if (isRegister)
        {
            // or just mov?
            compiler.storeMemory(destination.getStackHolderObject()->getTemporaryObject(),
                                 source.getStackHolderObject()->getTemporaryObject(),
                                 0,
                                 destinationSize);
        } else
        {
            compiler.store32(position,
                             size,
                             source.getStackHolderObject()->getTemporaryObject(),
                             isArg, false);
        }

        // Increase reference
        if (shouldDecReference)
        {
            // Load destination into the stack
            ObjectOpcodes::duplicateStack(emitContext, source);
            CallingConvention::call(emitContext, globalContext.getFrameworkMethods().getIncObj());
            // Due to incRef/eval-source, dest can be moved to local-temp register.
        }
        break;
    case StackEntity::ENTITY_LOCAL:
    case StackEntity::ENTITY_TOKEN_ADDRESS:
    case StackEntity::ENTITY_ARGUMENT:
    case StackEntity::ENTITY_REGISTER_ADDRESS:
    case StackEntity::ENTITY_LOCAL_TEMP_STACK:
    case StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS:
    case StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR:

        // Check for COMPLEXSTRUCT
        if ((!isRegister && (size <= (uint)compiler.getStackSize())) ||
            (isRegister && (destinationSize <= (uint)compiler.getStackSize())))
        {
            // Increase reference
            if (shouldDecReference)
            {
                // Load destination into the stack
                //evaluateInt32(emitContext, source, true, 0, true);
                ObjectOpcodes::duplicateStack(emitContext, source);
                CallingConvention::call(emitContext, globalContext.getFrameworkMethods().getIncObj());
                // Due to incRef/eval-source, dest can be moved to local-temp register.
            }

            // Reeval source and detination
            evaluateInt32(emitContext, source, true, 0);

            if (isRegister)
            {
                evaluateInt32(emitContext, destination, true, 0, true);
                compiler.storeMemory(destination.getStackHolderObject()->getTemporaryObject(),
                                     source.getStackHolderObject()->getTemporaryObject(),
                                     0,
                                     destinationSize);
            } else
            {
                // Store. TODO! See case StackEntity::ENTITY_REGISTER. Duplicate code
                if ((source.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK) ||
                    (source.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS))
                    evaluateInt32(emitContext, source, true);

                compiler.store32(position,
                                 size,
                                 source.getStackHolderObject()->getTemporaryObject(),
                                 isArg, false);
            }
        } else
        {
            // Push the destination
            if (isRegister)
            {
                convertLocalOrArgToLocArgAddress(emitContext, destination);
                stack.push(destination);
            } else
            {
                TemporaryStackHolderPtr dest(new TemporaryStackHolder(emitContext.currentBlock,
                                                                      ELEMENT_TYPE_I4,
                                                                      CompilerInterface::STACK_32,
                                                                      TemporaryStackHolder::TEMP_ONLY_REGISTER));
                emitContext.methodRuntime.m_compiler->load32addr(position, size, 0,
                                                    dest->getTemporaryObject(),
                                                    isArg);

                StackEntity destination(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
                destination.setStackHolderObject(dest);
                stack.push(destination);
            }

            // Push the source. Convert into local-address
            convertLocalOrArgToLocArgAddress(emitContext, source);
            stack.push(source);

            // And push size
            StackEntity sizeEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
            sizeEntity.getConst().setConstValue(size);
            stack.push(sizeEntity);

            // Add a "call" instruction to memcpy
            CallingConvention::call(emitContext,
                                    emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getMemcpy());
        }
        break;

    default:
        // Not ready yet
        CHECK_FAIL();
    }
}

void RegisterEvaluatorOpcodes::loadReturnedValue(EmitContext& emitContext)
{
    // Check whether the method has a return code
    const MethodDefOrRefSignature& sig = emitContext.methodContext.getMethodSignature();
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    //TODOasdf return struct
    unsigned int size = emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().getTypeSize(sig.getReturnType());

    if (sig.getReturnType().isVoid())
    {
        // Check that the stack is empty
        // CLREXCEPT! TODO! Clr stack exception.
        CHECK(emitContext.currentBlock.getCurrentStack().isEmpty());
    }
    else if(size > (uint)compiler.getStackSize())
    {
        // Pop the only return value
        StackEntity value(stack.getArg(0));

        emitContext.currentBlock.getCurrentStack().pop2null();

        uint retStructIndex = 0;

        //get the argument which is the address of the buffer to copy into the
        StackEntity structRet(StackEntity::ENTITY_ARGUMENT, emitContext.methodRuntime.m_args.getArgumentStackVariableType(retStructIndex));
        structRet.getConst().setLocalOrArgValue(retStructIndex);
        stack.push(structRet);

        //push the src
        stack.push(value);
        stack.getArg(0).AddressUpType();

        //push the size
        StackEntity sizeEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
        sizeEntity.getConst().setConstValue(size);
        stack.push(sizeEntity);

        //call memcpy
        CallingConvention::call(emitContext, emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getMemcpy());
    }
    else
    {
        // Pop the only return value
        StackEntity value(emitContext.currentBlock.getCurrentStack().getArg(0));
        emitContext.currentBlock.getCurrentStack().pop2null();

        // CLREXCEPT! TODO! Clr stack exception.
        CHECK(emitContext.currentBlock.getCurrentStack().isEmpty());

        // COMPLEXSTRUCT
        // TODO! Check the return type for both casting and more.
        // For now assume 32 bit values integer
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, value);
        emitContext.methodRuntime.m_compiler->assignRet32(value.getStackHolderObject()->getTemporaryObject());
    }
}

void RegisterEvaluatorOpcodes::loadNewString(EmitContext& emitContext,
                                             GlobalContext& globalContext,
                                             const TokenIndex& string,
                                             StackEntity& destNewString,
                                             TemporaryStackHolderPtr& newreg)
{
    Stack& stack(emitContext.currentBlock.getCurrentStack());

    // Push size
    StackEntity sizeEntity(StackEntity::ENTITY_CONST, ConstElements::gBool);
    sizeEntity.getConst().setConstValue(globalContext.getStringRepository().getStringLength(string));
    stack.push(sizeEntity);

    // Push string
    emitContext.methodRuntime.m_compiler->loadInt32(newreg->getTemporaryObject(),
                                                   StringRepository::serializeString(string));
    destNewString = StackEntity(StackEntity::ENTITY_REGISTER, ConstElements::gBytePtr); // TODO! Ascii/Unicode
    destNewString.setStackHolderObject(newreg);
    stack.push(destNewString);
    newreg = TemporaryStackHolderPtr();
    destNewString = StackEntity();

    CallingConvention::call(emitContext,
                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().instanceString());
    destNewString = stack.getArg(0);
    stack.pop2null();
}

void RegisterEvaluatorOpcodes::implementLoadToken(EmitContext& emitContext, mdToken token)
{
    Apartment& apartment = *emitContext.methodContext.getApartment();
    Stack& stack(emitContext.currentBlock.getCurrentStack());
    const MetadataTables& tables = apartment.getTables();
    TablePtr table(tables.getTableByToken(token));

    switch (EncodingUtils::getTokenTableIndex(token))
    {
    case TABLE_FIELD_TABLE:
        {
            const FieldTable::Header& fheader = ((FieldTable&)(*(table))).getHeader();
            if (!(fheader.m_flags & FieldTable::fdHasFieldRVA))
            {
                CompilerTrace("ldtoken: Loading field token that has no RVA" << endl);
                CHECK_FAIL();
            } // else
            // Read FieldRVA and field signature for the type of the field.
            RowTablesPtr fieldsRvas = tables.byTableID(TABLE_FIELDRVA_TABLE);
            for (uint i = 0; i < fieldsRvas.getSize(); i++)
            {
                const FieldRVATable::Header& rvaHeader = ((FieldRVATable&)(*fieldsRvas[i])).getHeader();
                if (rvaHeader.m_field == token)
                {
                    // Change the System.Array class to byte*
                    StackEntity zero(StackEntity::ENTITY_CONST, ConstElements::gU);
                    zero.getConst().setConstValue(0);
                    // Stack map before call:
                    //   Class System.Array
                    //   Offset = 0
                    //   Element Size = 0
                    stack.push(zero);
                    stack.push(zero);
                    // Call compilerGetArrayOffset()
                    CallingConvention::call(emitContext,
                                            emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getArrayBuffer());
                    // Stack after call:
                    //   byte* System.Array.m_buffer

                    // Get RVA locations
                    cForkStreamPtr data = apartment.getLayout()->
                        getVirtualStream(rvaHeader.m_fieldRva);
                    // Get type size
                    TokenIndex fieldToken(buildTokenIndex(apartment.getUniqueID(), token));
                    ElementType fType = apartment.getObjects().getTypedefRepository().getFieldType(fieldToken, ElementType::UnresolvedTokenIndex);
                    uint size = apartment.getObjects().getTypedefRepository().getTypeSize(fType);
                    // Huristic
                    //
                    // newarr [mscorlib]System.Char
                    // dup
                    // ldtoken $$method0x6000016-1
                    // call void [mscorlib]System.Runtime.CompilerServices.RuntimeHelpers::InitializeArray(class [mscorlib]System.Array, value class [mscorlib]System.RuntimeFieldHandle)
                    //
                    //
                    ElementType storeType(stack.getArg(1).getElementType());
                    ClrResolver::resolveUnboxType(emitContext.methodContext.getApartment(), storeType);
                    if (storeType.getType() == ELEMENT_TYPE_CHAR)
                    {
                        // Use String repository instead (for Unicode to ASCII convertion)
                        apartment.getObjects().getStringRepository().serializeString(data, size, fieldToken);
                        StackEntity entity(StackEntity::ENTITY_ADDRESS_WITH_STRING_DATA, fType);
                        entity.getConst().setTokenIndex(fieldToken);
                        stack.push(entity);
                        return;
                    } else
                    {
                        uint offset = apartment.getObjects().getTypedefRepository().allocateDataSection(data, size);
                        StackEntity entity(StackEntity::ENTITY_ADDRESS_WITH_DATA, fType);
                        entity.getConst().setConstValue(offset);
                        stack.push(entity);
                        return;
                    }
                }
            }
            CompilerTrace("ldtoken: Cannot find FieldRVA for token " << HEXDWORD(token) << endl);
            CHECK_FAIL();
        }
        break;

    // Need to add Method/ref/spec tokens, Typedef/ref tokens
    default:
        CompilerTrace("ldtoken: Unimplemented token type" << HEXDWORD(token) << endl);
        CHECK_FAIL();
    }
}
