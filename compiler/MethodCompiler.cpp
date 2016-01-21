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
 * MethodCompiler.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"

#include "dismount/assembler/DependencyException.h"

#include "compiler/MethodCompiler.h"
#include "compiler/CompilerEngine.h"
#include "compiler/CompilerException.h"
#include "compiler/CompilerFactory.h"
#include "compiler/MethodBlock.h"
#include "compiler/MethodRuntimeBoundle.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerTrace.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"

#include "executer/compiler/CompilerEngineThread.h"

// TODO! Move to configuration
const char MethodCompiler::gCustomAttributeExportClass[] = "Export";

MethodCompiler::MethodCompiler(CompilerFactory::CompilerType compilerType,
                               const CompilerParameters& compilerParameters,
                               const ApartmentPtr& apartment,
                               mdToken methodToken,
                               MethodRunnable& methodRunnable) :
    m_apartment(apartment),
    m_methodToken(methodToken),
    m_cleanupIndex(ElementType::UnresolvedTokenIndex),
    m_compilerType(compilerType),
    m_compilerParams(compilerParameters),
    m_methodRunnable(methodRunnable)
{
}

void MethodCompiler::methodInitObjectLocals(class LocalPositions& locals)
{
    // If there are any locals which are objects (not value types), then
    // we need to add a block to initialize all local objects to NULL
    uint uObjectLocalsAmount = locals.countObjects(m_apartment->getObjects().getTypedefRepository(), false);

    // Don't even generate the block if all locals are non-objects
    if (uObjectLocalsAmount == 0)
        return;

    // Compiling locals initialization
    MethodBlock* pInitBlock = new MethodBlock(MethodBlock::BLOCK_INIT_OBJECTS,
                                    *m_interface->getInnerCompilerInterface());
    StackInterfacePtr stack(pInitBlock);

    m_interface->getFirstPassPtr()->createNewBlockWithoutChange(
        MethodBlock::BLOCK_INIT_OBJECTS, stack);
    m_interface->getFirstPassPtr()->changeBasicBlock(
        MethodBlock::BLOCK_INIT_OBJECTS);

    // Use storeConst if we have just one object local.
    // Use a register and store32 if we have more than one object local.
    if (uObjectLocalsAmount == 1)
    {
        // Store a const NULL to the object local
        uint uLocalIndex = locals.firstObjectIndex();
        CHECK(locals.getLocalStackVariableType(uLocalIndex).
                                    isObjectAndNotValueType());

        uint uSize = m_apartment->getObjects().getTypedefRepository().
                getTypeSize(locals.getLocalStackVariableType(uLocalIndex));
        cSmartPtr<uint8> zero(new uint8[uSize]);
        memset(zero.getPointer(), 0, uSize);
        m_interface->storeConst(locals.getLocalPosition(uLocalIndex),
                                zero, uSize);
    }
    else
    {
        // Reserve a register for the NULL value
        TemporaryStackHolderPtr nullReg(new TemporaryStackHolder(
                                        *stack,
                                        ELEMENT_TYPE_I4,
                                        CompilerInterface::STACK_32,
                                        TemporaryStackHolder::TEMP_ONLY_REGISTER));
        // Put NULL on the register
        m_interface->loadInt32(nullReg->getTemporaryObject(), 0);
        // And store this NULL to all object locals
        for (uint uLocal = 0; uLocal < locals.getSize(); uLocal++)
        {
            if (locals.getLocalStackVariableType(uLocal).isObjectAndNotValueType())
            {
                m_interface->store32(locals.getLocalPosition(uLocal),
                                        m_apartment->getObjects().getTypedefRepository().
                                        getTypeSize(locals.getLocalStackVariableType(uLocal)),
                                        nullReg->getTemporaryObject());
            }
        }
    }
    pInitBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

class FieldDRef: public ResolverInterface::FieldEnumeratorCallbacker
{
public:
    FieldDRef(CompilerInterface& _interface, StackInterface& stack, const cString& funcDecObj, uint position, bool bArgument, const TokenIndex& partentToken) :
        m_interface(_interface),
        m_stack(stack),
        m_funcDecObj(funcDecObj),
        m_position(position),
        m_bArgument(bArgument),
        m_partentToken(partentToken)
    {
    }

    virtual void fieldCallback(void*, const ResolverInterface& resolver,
                               const TokenIndex& fieldContext, const ElementType& fieldElementType)
    {
        if (fieldElementType.isObjectAndNotValueType())
        {
            // Load field address and invoke a call to decobj
            {
                // Load the object local pointer using the real stack base
                TemporaryStackHolder localAddr(m_stack, ELEMENT_TYPE_PTR, m_interface.getStackSize(), TemporaryStackHolder::TEMP_ONLY_REGISTER);
                uint offset = resolver.getFieldRelativePosition(fieldContext, m_partentToken);
                m_interface.load32(m_position + offset, m_interface.getStackSize(), localAddr.getTemporaryObject(), false, m_bArgument, false);

                // Push the object local pointer as the argument to DecRef
                m_interface.pushArg32(localAddr.getTemporaryObject());
                // Free the object local register, so it is not saved on stack (if volatile)
            }

            // Call DecRef for this local
            m_interface.call(m_funcDecObj, 1);

            if (m_interface.getDefaultCallingConvention() == CompilerInterface::C_DECL)
            {
                // Revert the pushArg for decRef
                m_interface.revertStack(m_interface.getStackSize());
            }
        }
    }

private:
    CompilerInterface& m_interface;
    StackInterface& m_stack;
    const cString& m_funcDecObj;
    uint m_position;
    bool m_bArgument;
    const TokenIndex& m_partentToken;
    TemporaryStackHolderPtr m_framePtr;
};

class MemberObjDref : public ResolverInterface::FieldEnumeratorCallbacker
{
public:
    MemberObjDref(CompilerInterface& _interface, StackInterface& stack, const cString& funcDecObj,
                  const TokenIndex& partentToken, bool shouldReferenceBP) :
        m_interface(_interface),
        m_stack(stack),
        m_funcDecObj(funcDecObj),
        m_partentToken(partentToken),
        m_shouldReferenceBP(shouldReferenceBP)
    {
    }

    virtual void fieldCallback(void*,
                               const ResolverInterface& resolver,
                               const TokenIndex& fieldContext,
                               const ElementType& fieldElementType)
    {
        if (fieldElementType.isObjectAndNotValueType())
        {
            uint offset = resolver.getFieldRelativePosition(fieldContext, m_partentToken);
            if (offset == ResolverInterface::OFFSET_FOR_STATICS)
                return;

            // Load field address and invoke a call to decobj
            {
                // Load method's real base stack pointer from the first argument
                // Load the object 1st argument
                TemporaryStackHolder localAddr(m_stack, ELEMENT_TYPE_PTR, m_interface.getStackSize(), TemporaryStackHolder::TEMP_ONLY_REGISTER);
                if (!m_shouldReferenceBP)
                {
                    m_interface.load32(0, m_interface.getStackSize(), localAddr.getTemporaryObject(), false, true);

                StackLocation oldBasePointer = m_interface.getFirstPassPtr()->getCurrentStack()->setBaseStackRegister(localAddr.getTemporaryObject());
                m_interface.load32(0, m_interface.getStackSize(), localAddr.getTemporaryObject(), false, true, false);
                m_interface.getFirstPassPtr()->getCurrentStack()->setBaseStackRegister(oldBasePointer);
            } else
            {
                // Load base pointer from first argument
                m_interface.load32(0, m_interface.getStackSize(), localAddr.getTemporaryObject(), false, true);
            }

                // Refernce member = [this+offset]
                m_interface.loadMemory(localAddr.getTemporaryObject(), localAddr.getTemporaryObject(), offset, m_interface.getStackSize());

                // Push the object local pointer as the argument to DecRef
                m_interface.pushArg32(localAddr.getTemporaryObject());
                // Free the registers, so they are not saved on stack if volatile
            }

            // Call DecRef for this local
            m_interface.call(m_funcDecObj, 1);

            if (m_interface.getDefaultCallingConvention() == CompilerInterface::C_DECL)
            {
                // Revert the pushArg for decRef
                m_interface.revertStack(m_interface.getStackSize());
            }
        }
    }
private:
    CompilerInterface& m_interface;
    StackInterface& m_stack;
    const cString& m_funcDecObj;
    const TokenIndex& m_partentToken;
    bool m_shouldReferenceBP;
};

void MethodCompiler::createClassMemberDestructorContent(const TypedefRepository& repository,
                                                        CompilerInterface& _interface,
                                                        StackInterface& stack,
                                                        const cString& funcDecObj,
                                                        const TokenIndex& partentToken,
                                                        bool shouldReferenceBP)
{
    MemberObjDref _enum(_interface, stack, funcDecObj, partentToken, shouldReferenceBP);
    repository.scanAllFields(partentToken, _enum, NULL);
}

void MethodCompiler::generateCleanupTokenIndex(LocalPositions& locals, ArgumentsPositions& args, const cString& methodName)
{
    // If there are object locals, then there is need of a cleanup routine to deref them
    bool shouldCleanupLocals = (locals.countObjects(m_apartment->getObjects().getTypedefRepository(), true) > 0);
    if (!m_apartment->getObjects().getFrameworkMethods().isFrameworkMethod(m_methodRunnable.getMethodToken()))
        shouldCleanupLocals |= (args.countObjects() > 0);

    // If this is a descructor and the class has special cleanup, then there is need of a cleanup routine
    bool shouldCleanupClass = CorlibNames::isFinalizer(methodName) &&
        m_apartment->getObjects().getTypedefRepository().isTypeShouldDref(m_methodRunnable.getMethodTypeParent().getClassToken());

    // Do we need any cleanup at all?
    if (!shouldCleanupLocals && !shouldCleanupClass)
        return;

    mdToken cleanupToken = EncodingUtils::buildToken(TABLE_CLR_METHOD_HELPERS,
        m_apartment->generateMethodHelperRow());
    m_cleanupIndex = buildTokenIndex(m_apartment->getUniqueID(), cleanupToken);
}

void MethodCompiler::methodCreateCleanupFunc(ElementType& elementType,
                                             LocalPositions& locals,
                                             ArgumentsPositions& args,
                                             CompilerEngineThread& engineThread,
                                             const cString& methodName,
                                             SecondPassBinaryPtr secondPass)
{
    // If there are object locals, then there is need of a cleanup routine to deref them
    bool shouldCleanupLocals = (locals.countObjects(m_apartment->getObjects().getTypedefRepository(), true) > 0);
    if (!m_apartment->getObjects().getFrameworkMethods().isFrameworkMethod(m_methodRunnable.getMethodToken()))
        shouldCleanupLocals |= (args.countObjects() > 0);

    // If this is a descructor and the class has special cleanup, then there is need of a cleanup routine
    bool shouldCleanupClass = CorlibNames::isFinalizer(m_methodRunnable.getFullMethodName()) &&
        m_apartment->getObjects().getTypedefRepository().isTypeShouldDref(m_methodRunnable.getMethodTypeParent().getClassToken());

    // Do we need any cleanup at all?
    if (!shouldCleanupLocals && !shouldCleanupClass)
    {
        ASSERT(m_cleanupIndex == ElementType::UnresolvedTokenIndex);
        return;
    }
    ASSERT(m_cleanupIndex != ElementType::UnresolvedTokenIndex);

    // Generate cleanup function for this method
    // No locals, 1 argument the size of a pointer
    CompilerInterfacePtr compilerInterface  = CompilerFactory::getCompiler(m_compilerType, m_apartment->getObjects().getFrameworkMethods(), m_compilerParams);
    compilerInterface->setLocalsSize(0);
    compilerInterface->setArgumentsSize(compilerInterface->getStackSize());

    // Create new stack
    StackInterfacePtr stack(new MethodBlock(0, *compilerInterface->getInnerCompilerInterface()));

    // We use just one block (0) in the cleanup routine
    compilerInterface->getFirstPassPtr()->createNewBlockWithoutChange(0, stack);
    compilerInterface->getFirstPassPtr()->changeBasicBlock(0);

    // Get a dependency-name for the dec-ref function
    cString sDecRefDependency(CallingConvention::serializedMethod(m_apartment->getObjects().getFrameworkMethods().getDecObj()));

    if (shouldCleanupLocals)
    {
        // Load method's real base stack pointer from the first argument
        TemporaryStackHolder framePtr(*stack, ELEMENT_TYPE_PTR, compilerInterface->getStackSize(), TemporaryStackHolder::TEMP_NONVOLATILE_REGISTER);
        compilerInterface->load32(0, compilerInterface->getStackSize(), framePtr.getTemporaryObject(), false, true);

        StackLocation oldBasePointer = compilerInterface->getFirstPassPtr()->getCurrentStack()->setBaseStackRegister(framePtr.getTemporaryObject());

        for (uint i = 0; i < locals.getSize(); ++i)
        {
            if (locals.getLocalStackVariableType(i).isObjectAndNotValueType())
            {
                // Dereference the object:
                {
                    // Load the object local pointer using the real stack base
                    TemporaryStackHolder localAddr(*stack, ELEMENT_TYPE_PTR, compilerInterface->getStackSize(), TemporaryStackHolder::TEMP_ONLY_REGISTER);
                    compilerInterface->load32(locals.getLocalPosition(i), compilerInterface->getStackSize(), localAddr.getTemporaryObject(), false, false, false);

                    // Push the object local pointer as the argument to DecRef
                    compilerInterface->pushArg32(localAddr.getTemporaryObject());
                    // Free the object local register, so it is not saved on stack (if volatile)
                }

                // Call DecRef for this local
                compilerInterface->call(sDecRefDependency, 1);

                if (compilerInterface->getDefaultCallingConvention() == CompilerInterface::C_DECL)
                {
                    // Revert the pushArg for decRef
                    compilerInterface->revertStack(compilerInterface->getStackSize());
                }
            } else if (locals.getLocalStackVariableType(i).isObject())
            {
                // Start enumerator on all fields for object
                FieldDRef _enum(*compilerInterface, *stack, sDecRefDependency,
                                locals.getLocalPosition(i), false, locals.getLocalStackVariableType(i).getClassToken());
                m_apartment->getObjects().getTypedefRepository().scanAllFields(locals.getLocalStackVariableType(i).getClassToken(), _enum, NULL);
            }
        }
        if (!m_apartment->getObjects().getFrameworkMethods().isFrameworkMethod(m_methodRunnable.getMethodToken()))
        {
            for (uint i = 0; i < args.getCount(); ++i)
            {
                // Argument 0 is "this". Don't dec-ref it
                if (m_methodRunnable.getMethodSignature().isHasThis() && (i == 0))
                    continue;

                if (args.getArgumentStackVariableType(i).isObject())
                {
                    // Dereference the object:
                    {
                        // Load the object argument pointer using the real stack base
                        TemporaryStackHolder localAddr(*stack, ELEMENT_TYPE_PTR, compilerInterface->getStackSize(), TemporaryStackHolder::TEMP_ONLY_REGISTER);
                        ASSERT(args.getArgumentStackSize(i) == compilerInterface->getStackSize());
                        compilerInterface->load32(args.getArgumentPosition(i), compilerInterface->getStackSize(), localAddr.getTemporaryObject(), false, true, false);

                        // Push the object local pointer as the argument to DecRef
                        compilerInterface->pushArg32(localAddr.getTemporaryObject());
                        // Free the object local register, so it is not saved on stack (if volatile)
                    }

                    // Call DecRef for this local
                    compilerInterface->call(sDecRefDependency, 1);

                    if (compilerInterface->getDefaultCallingConvention() == CompilerInterface::C_DECL)
                    {
                        // Revert the pushArg for decRef
                        compilerInterface->revertStack(compilerInterface->getStackSize());
                    }
                } else if (args.getArgumentStackVariableType(i).isObject())
                {
                    // Start enumerator on all fields for object
                    FieldDRef _enum(*compilerInterface, *stack, sDecRefDependency,
                                    args.getArgumentPosition(i), true, args.getArgumentStackVariableType(i).getClassToken());
                    m_apartment->getObjects().getTypedefRepository().scanAllFields(args.getArgumentStackVariableType(i).getClassToken(), _enum, NULL);
                }
            }
        }

        compilerInterface->getFirstPassPtr()->getCurrentStack()->setBaseStackRegister(oldBasePointer);
    }

    if (shouldCleanupClass)
    {
        createClassMemberDestructorContent(m_apartment->getObjects().getTypedefRepository(),
                                           *compilerInterface,
                                           *stack,
                                           sDecRefDependency,
                                           m_methodRunnable.getMethodTypeParent().getClassToken(),
                                           false);
    }

    // Seal the method.
    // Second pass method linker, and return
    compilerInterface->generateMethodEpiProLogs();
    compilerInterface->getFirstPassPtr()->seal();

    // Add the method itself to the list
    SecondPassBinary::ResolveHelperList list;
    list.append(SecondPassBinary::ResolveHelper(secondPass, CallingConvention::serializedMethod(buildTokenIndex(m_apartment->getUniqueID(), m_methodToken)), m_interface->getFirstPassPtr()));

    // Generate second-pass binary for this function
    SecondPassBinaryPtr pass = SecondPassBinaryPtr(new SecondPassBinary(
        *compilerInterface->getFirstPassPtr(), MethodBlock::BLOCK_RET,
        compilerInterface->getStackSize(), compilerInterface->getAlignBuffer(), &list));

    // Give a name to the cleanup function
    pass->getDebugInformation().setMethodName(m_methodRunnable.getFullMethodName() + "_CleanUp");

    // Todo: Better wrap this code in a public function in CompilerEngineThread
    //       and re-use it everywhere
    // Ilan: please look at the precompiled-repository. We can use signature to unite functions
    // Add the function to the repository with a unique token
    // Calculate signature and notify repositories
    CRC64 crc64;
    mdToken token(getTokenID(m_cleanupIndex));
    crc64.update(&token, sizeof(token));
    engineThread.onMethodCompiled(m_apartment->getApartmentName(), crc64.digest(), m_cleanupIndex, pass, false);
}

SecondPassBinaryPtr MethodCompiler::compileInstanceDestructor(const Apartment& apartment,
                                                              CompilerFactory::CompilerType compilerType,
                                                              const CompilerParameters compilerParams,
                                                              const TokenIndex& instanceDetorToken)
{
    CompilerInterfacePtr compilerInterface  = CompilerFactory::getCompiler(compilerType, apartment.getObjects().getFrameworkMethods(), compilerParams);
    compilerInterface->setLocalsSize(0);
    compilerInterface->setArgumentsSize(compilerInterface->getStackSize());

    // Create new stack
    StackInterfacePtr stack(new MethodBlock(0, *compilerInterface->getInnerCompilerInterface()));

    // We use just one block (0) in the cleanup routine
    compilerInterface->getFirstPassPtr()->createNewBlockWithoutChange(0, stack);
    compilerInterface->getFirstPassPtr()->changeBasicBlock(0);
    cString dependencyTokenName(CallingConvention::serializedMethod(apartment.getObjects().getFrameworkMethods().getDecObj()));

    TokenIndex classToken = instanceDetorToken;
    getTokenID(classToken) = EncodingUtils::buildToken(TABLE_TYPEDEF_TABLE,
                                        EncodingUtils::getTokenPosition(getTokenID(classToken)));

    createClassMemberDestructorContent(apartment.getObjects().getTypedefRepository(),
                                        *compilerInterface,
                                        *stack,
                                        dependencyTokenName,
                                        classToken,
                                        true);

    compilerInterface->generateMethodEpiProLogs();
    compilerInterface->getFirstPassPtr()->seal();

    SecondPassBinaryPtr pass = SecondPassBinaryPtr(new SecondPassBinary(
                            *compilerInterface->getFirstPassPtr(), MethodBlock::BLOCK_RET,
                            compilerInterface->getStackSize(), compilerInterface->getAlignBuffer()));

    cString FinializeName = apartment.getObjects().getTypesNameRepository().getTypeNamespace(classToken);
    FinializeName+= ".";
    FinializeName+= apartment.getObjects().getTypesNameRepository().getTypeName(classToken);
    FinializeName+= ".Finalize_Autogerated";
    pass->getDebugInformation().setMethodName(FinializeName);
    return pass;
}

void MethodCompiler::methodRegisterCleanupFunction(MethodRunnable& method, MethodRuntimeBoundle& boundle)
{
    // If there is no cleanup function, then there's nothing to register
    // and the method itself becomes transparent to exceptions
    if (m_cleanupIndex == ElementType::UnresolvedTokenIndex)
        return;

    // If not supporting exception handling, then the cleaup function will be called directly.
    // So no need to mess with the exception stack
    if (!m_compilerParams.m_bSupportExceptionHandling)
        return;

    // Generate the block with a clean stack
    MethodBlock* pInitBlock = new MethodBlock(MethodBlock::BLOCK_REG_CLEANUP, *m_interface->getInnerCompilerInterface());
    StackInterfacePtr stack(pInitBlock);
    m_interface->getFirstPassPtr()->createNewBlockWithoutChange(
        MethodBlock::BLOCK_REG_CLEANUP, stack);
    m_interface->getFirstPassPtr()->changeBasicBlock(
        MethodBlock::BLOCK_REG_CLEANUP);

    // Create an EmitContext for this block
    EmitContext emitContext(method, boundle, *pInitBlock);

    // Param1 is the type - method cleanup
    StackEntity type(StackEntity::ENTITY_CONST, ConstElements::gU4);
    type.getConst().setConstValue(FrameworkMethods::EXCEPTION_ROUTINE_METHOD_CLEANUP);
    pInitBlock->getCurrentStack().push(type);

    // Param2 is the address of the cleanup function. store in a stack entity
    StackEntity cleanupFunctionAddress(StackEntity::ENTITY_METHOD_ADDRESS, ConstElements::gVoidPtr);
    cleanupFunctionAddress.getConst().setTokenIndex(m_cleanupIndex);
    pInitBlock->getCurrentStack().push(cleanupFunctionAddress);

    // Param3 is the stack pointer. Load it onto a stack entity
    StackEntity stackPointer(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
    stackPointer.setStackHolderObject(
        TemporaryStackHolderPtr(new TemporaryStackHolder(
            *stack, pInitBlock->getBaseStackRegister())));
    pInitBlock->getCurrentStack().push(stackPointer);

    // Call clrRegisterRoutine
    CallingConvention::call(emitContext,
                            m_apartment->getObjects().getFrameworkMethods().getRegisterRoutine());

    pInitBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

void MethodCompiler::compileBlocks(MethodRuntimeBoundle& boundle, MethodHelper* pCurrentHandler)
{
    boundle.m_compiler->enableOptimizations();

    while (!boundle.m_blockStack.isEmpty())
    {
        // Get the next block to be compile...
        StackInterfacePtr nextBlockPtr(boundle.m_blockStack.first());
        MethodBlock& nextBlock((MethodBlock&)*nextBlockPtr);
        boundle.m_blockStack.pop2null();

        CompilerTrace("Compiling next block: " << HEXDWORD(nextBlock.getBlockID()) << endl);
#ifdef _DEBUG
                // A nice spot to start debugging a specific function
                if (nextBlock.getBlockID() == 0x00000079)
                {
                    int x = 0;
                    x++;
                    //cOS::debuggerBreak();
                }
#endif
        // Drop precompiled blocks
        if (boundle.m_methodSet.isSet(nextBlock.getBlockID()))
        {
            // Get the previous block (See the -1 later)
            FirstPassBinary& methodPass = *boundle.m_compiler->getFirstPassPtr();
            FirstPassBinary::BasicBlockList::iterator oldpos = methodPass.
                getNearestBlock(methodPass.getBlocksList(),
                                nextBlock.getBlockID(),
                                false);
            if ((oldpos != methodPass.getBlocksList().end()) &&
                ((*oldpos).m_blockNumber == nextBlock.getBlockID()))
            {
                // Blocks equals
                CompilerTrace("Block was already compiled...ignoring." << endl);
                // TODO! CHECK FOR REGISTERS COMPATIBILITY!!!
                continue;
            }
            else
            {
                // Should never need to recompile!
                CHECK(FALSE);
            }
        }

        // Seek into method address
        cForkStreamPtr stream = m_methodRunnable.getStreamPointer()->fork();
        stream->seek(nextBlock.getBlockID() +
                     m_methodRunnable.getMethodStreamStartAddress(),
                     basicInput::IO_SEEK_SET);
        // Create new binary pass with method address.
        // This code cannot be failed, since the instruction is yet to be
        // compiled
        boundle.m_compiler->getFirstPassPtr()->createNewBlockWithoutChange(
                                            nextBlock.getBlockID(),
                                            nextBlockPtr);
        boundle.m_compiler->getFirstPassPtr()->changeBasicBlock(nextBlock.getBlockID());

        EmitContext emitContext(m_methodRunnable, boundle, nextBlock, pCurrentHandler);

        XSTL_TRY
        {
            // Compile current block instruction
            while (true)
            {
                uint index = stream->getPointer() - m_methodRunnable.getMethodStreamStartAddress();

                // Handle block-split condition, before entering protected block
                if (CompilerEngine::handleSplit(emitContext, *stream))
                {
                    emitContext.methodRuntime.m_blockStack.add(StackInterfacePtr(emitContext.currentBlock.duplicate(emitContext, index)));

                    // Flush the optimizer cache
                    emitContext.methodRuntime.m_compiler->renderBlock();

                    emitContext.currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON, index);
                    break;
                }

                // Detect entering a protected block
                ExceptionOpcodes::enterProtectedBlock(emitContext, index, m_helpers);

                if (CompilerEngine::step(emitContext, *stream))
                {
                    // Flush the optimizer cache
                    emitContext.methodRuntime.m_compiler->renderBlock();
                    break;
                }
            }
            // Basic block was terminate. Make sure that all block variables are registers...

        }
        XSTL_CATCH(cException& e)
        {
            CompilerTrace("Caught unhandled exception while stepping..." << endl);
            XSTL_THROW(CompilerException, m_methodRunnable.getFullMethodName(), &e);
        }

        XSTL_CATCH_ALL
        {
            CompilerTrace("Caught unhandled exception while stepping..." << endl);
            XSTL_THROW(CompilerException, m_methodRunnable.getFullMethodName(), NULL);
        }
    }

    boundle.m_compiler->disableOptimizations();
}

SecondPassBinaryPtr MethodCompiler::compile(CompilerEngineThread& engineThread)
{
    // Forget members which were set for a previous method
    m_cleanupIndex = ElementType::UnresolvedTokenIndex;

    // Generate a compiler and configure the apartment for compilation
    m_interface = CompilerFactory::getCompiler(m_compilerType, m_apartment->getObjects().getFrameworkMethods(), m_compilerParams);
    m_interface->getFirstPassPtr()->setStdCall(CallingConvention::getDesiredCallingMethod(m_methodToken, *m_apartment, *m_interface)
        == CompilerInterface::STDCALL);

    // Trace the executed code
    CompilerTrace("Entering " << m_methodRunnable.getFullMethodName() << endl);


    // Check for extern method
    if (m_methodRunnable.isEmptyMethod())
    {
        CompilerTrace("Function " << HEXDWORD(m_methodToken) << " is interface decleration, return NULL code..." << endl);
        return SecondPassBinaryPtr(new SecondPassBinary(*m_interface->getFirstPassPtr(), SecondPassBinary::NO_BLOCKS_ALIGN,
                                    m_interface->getStackSize(), m_interface->getAlignBuffer()));
    }

    // Start compiling. Generate new binary pass

    // Calculate the stack size and positions
    LocalPositions locals;
    uint stackSize = calculateStackSize(m_methodRunnable.getLocals(),
                                        locals);

    // Check for this variable
    ElementType thisElementType(ConstElements::gVoid);
    if (m_methodRunnable.getMethodSignature().isHasThis())
    {
        // Get method typedef
        mdToken typedefToken = m_apartment->getTables().getTypedefParent(m_methodToken);
        TokenIndex typedefIndex = buildTokenIndex(m_apartment->getUniqueID(), typedefToken);
        m_apartment->getObjects().getTypesNameRepository().translateTyperefToTypedef(typedefIndex);

        bool isClass = m_apartment->getObjects().getTypedefRepository().isTypedefClass(typedefIndex);
        thisElementType = ElementType(ELEMENT_TYPE_CLASS,     // (isClass ? ELEMENT_TYPE_OBJECT : ELEMENT_TYPE_VALUETYPE),
                                      0, false, false, false, // (isClass ? ElementType::PT_NORMAL : ElementType::PT_POINTER),
                                      typedefIndex);
    }

    // Calculate the argument sizes
    ArgumentsPositions args;
    CallingConvention::calculateArgumentsSizes(*m_interface,
                                               m_apartment,
                                               *this,
                                               m_methodRunnable.getMethodSignature(),
                                               args,
                                               thisElementType);

    // Create a dependency name for the cleanup function of this method
    generateCleanupTokenIndex(locals, args, m_methodRunnable.getFullMethodName());

    // Prepare for this method
    m_interface->setLocalsSize(stackSize);
    m_interface->setArgumentsSize(args.getTotalArgumentsSize());

    // Compiling method using the engine...
    MethodRuntimeBoundle boundle(m_interface, locals, args,
                                 m_methodRunnable.getMethodHeader(), m_cleanupIndex);

    {
        // Scan and locate all block-split points in the MSIL before starting to compile
        cForkStreamPtr stream = m_methodRunnable.getStreamPointer()->fork();
        stream->seek(m_methodRunnable.getMethodStreamStartAddress(), basicInput::IO_SEEK_SET);
        cBuffer methodData;
        stream->pipeRead(methodData, m_methodRunnable.getMethodHeader().getFunctionLength());
        // Start parsing MSIL opcodes and check for dependencies inside the code
        const uint8* msil = methodData.getBuffer();
        const uint size = methodData.getSize();

        boundle.m_blockSplit.changeSize(size);
        boundle.m_blockSplit.resetArray();
        boundle.scanMSIL(msil, size);
    }

    // Generate object-locals initialization block (set objects to null)
    if (m_methodRunnable.getMethodHeader().isInitLocals())
        methodInitObjectLocals(locals);

    // Register the cleanup function (if any) on the exception stack
    methodRegisterCleanupFunction(m_methodRunnable, boundle);

    // Compile all code blocks
    compileBlocks(boundle);

    // Compile helpers as well
    for (MethodHelperList::iterator iHelper = m_helpers.begin(); iHelper != m_helpers.end(); iHelper++)
    {
        MethodHelper& helper = (*iHelper);
        if (!m_interface->getCompilerParameters().m_bSupportExceptionHandling)
        {
            // If no exception handling, then only compile finally handler helpers
            if (helper.getClause().flags != MethodHeader::ClauseFinally)
                continue;
        }

        CompilerTrace("Compiling handler: " << m_methodRunnable.getFullMethodName() << helper.nameSuffix() << endl);

        // Create and prepare a new compiler for the helper function's contents
        helper.m_compiler = CompilerFactory::getCompiler(m_compilerType, m_apartment->getObjects().getFrameworkMethods(), m_compilerParams);
        helper.m_compiler->enableOptimizations();
        m_interface->getFirstPassPtr()->setStdCall(CallingConvention::getDesiredCallingMethod(m_methodToken, *m_apartment, *m_interface)
            == CompilerInterface::STDCALL);
        helper.m_compiler->setLocalsSize(0);

        // the signature is:
        //   void helper(void* baseStackPointer)
        helper.m_compiler->setArgumentsSize(m_interface->getStackSize());

        // Create block for getting the method stack base
        MethodBlock* pBlockInit = new MethodBlock(MethodBlock::BLOCK_INIT_OBJECTS, *helper.m_compiler/*->getInnerCompilerInterface()*/);
        StackInterfacePtr stackInit(pBlockInit);
        helper.m_compiler->getFirstPassPtr()->createNewBlockWithoutChange(MethodBlock::BLOCK_INIT_OBJECTS, stackInit);
        helper.m_compiler->getFirstPassPtr()->changeBasicBlock(MethodBlock::BLOCK_INIT_OBJECTS);

        // Load method's real base stack pointer from the first argument, into a non-volatile register
        // Allocate a nonvolatile register
        TemporaryStackHolder baseStackPointer(*stackInit, ELEMENT_TYPE_PTR, helper.m_compiler->getStackSize(), TemporaryStackHolder::TEMP_NONVOLATILE_REGISTER);
        // Remember this nonvolatile register, so it can be used in methodEstimateEncoding
        helper.m_baseStackPointer = baseStackPointer.getTemporaryObject();

        // Load the first argument into this register
        helper.m_compiler->load32(0, helper.m_compiler->getStackSize(), helper.m_baseStackPointer, false, true, false);

        // Retrieve the exception object, so the code can use it
        switch (helper.getClause().flags)
        {
        case MethodHeader::ClauseCatch:
        case MethodHeader::ClauseFilter:
            // Allocate a register
            TemporaryStackHolderPtr ret(new TemporaryStackHolder(
                                    *pBlockInit,
                                    ELEMENT_TYPE_I,
                                    CompilerInterface::STACK_32,
                                    TemporaryStackHolder::TEMP_ONLY_REGISTER));

            // Call getCurrentExeption to Fill the register with a pointer to the object
            cString dependencyTokenName(CallingConvention::serializedMethod(m_apartment->getObjects().getFrameworkMethods().getCurrentException()));
            helper.m_compiler->call32(dependencyTokenName, ret->getTemporaryObject(), 0);

            // And push this object onto the evaulation stack, so the code can use it
            StackEntity retValue(StackEntity::ENTITY_REGISTER, ElementType(ELEMENT_TYPE_CLASS, 0, false, false, false, m_apartment->getObjects().getTypedefRepository().getTypeToken(CorlibNames::gCoreNamespace, CorlibNames::m_classException)));
            retValue.setStackHolderObject(ret);
            pBlockInit->getCurrentStack().push(retValue);
            break;
        }

        pBlockInit->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
        helper.m_compiler->renderBlock();
        helper.m_compiler->disableOptimizations();

        // Create a boundle for compiling the helper's contents
        helper.m_boundle = cSmartPtr<MethodRuntimeBoundle>(new MethodRuntimeBoundle(boundle, helper.getClause().handlerOffset));
        // Fetch the first block
        MethodBlock* firstBlock = (MethodBlock*)(StackInterface*)helper.m_boundle->m_blockStack.first();
        firstBlock->getCurrentStack() = pBlockInit->getCurrentStack();
        // Let the compiler know that this is code inside the helper
        firstBlock->getExceptionsStack().append(helper);
        // Copy register allocation table (so that the current exeption is remembered)
        firstBlock->getRegistersTable() = pBlockInit->getRegistersTable();
        // Forcefully reserve the base stack pointer register
        firstBlock->getRegistersTable()[helper.m_baseStackPointer.u.reg].m_bAllocated = true;
        // Set the base stack register
        firstBlock->setBaseStackRegister(helper.m_baseStackPointer);

        // And compile the code into the function, as if we're still in the method
        RegisterAllocationTable oldTouched = m_interface->getFirstPassPtr()->getTouchedRegisters();
        m_interface->getFirstPassPtr()->getTouchedRegisters().removeAll();
        m_interface->getFirstPassPtr()->touch(helper.m_baseStackPointer.u.reg);

        compileBlocks(*helper.m_boundle, &helper);

        // Restore the method's compiler state
        helper.m_touched = m_interface->getFirstPassPtr()->getTouchedRegisters();
        m_interface->getFirstPassPtr()->getTouchedRegisters() = oldTouched;
    }

    // Generate method header and closer
    m_interface->generateMethodEpiProLogs(boundle.m_bHasCatch);

    // link and finish the helpers
    for (MethodHelperList::iterator iHelper = m_helpers.begin(); iHelper != m_helpers.end(); iHelper++)
    {
        MethodHelper& helper = (*iHelper);
        if (!m_interface->getCompilerParameters().m_bSupportExceptionHandling)
        {
            // If no exception handling, then only compile finally handler helpers
            if (helper.getClause().flags != MethodHeader::ClauseFinally)
                continue;
        }

        // Move all compiled basic blocks to our helper's FPB
        m_interface->getFirstPassPtr()->moveBasicBlocks(helper.m_boundle->m_methodSet, *helper.m_compiler->getFirstPassPtr());
        // Copy the touched registers as well
        helper.m_compiler->getFirstPassPtr()->getTouchedRegisters() = helper.m_touched;

        // From now on, we use the helper's compiler, and not the method's
        helper.m_boundle->m_compiler = helper.m_compiler;

        // Second pass linker for helper function
        helper.m_compiler->generateMethodEpiProLogs(helper.m_boundle->m_bHasCatch);

        // Run the FP method linker, with the correct base pointer
        helper.m_boundle->m_compiler = CompilerInterfacePtr(helper.m_boundle->m_compiler, SMARTPTR_DESTRUCT_NONE);
        methodEstimateEncoding(*helper.m_boundle, &helper);

        // Seal the helper
        helper.m_compiler->getFirstPassPtr()->seal();
    }

    // First pass method linker
    methodEstimateEncoding(boundle);

    // Seal the method
    m_interface->getFirstPassPtr()->seal();

    // Return the first binary pass
    // Second pass method linker, and return
    SecondPassBinaryPtr ret(NULL);
    try
    {
        ret = SecondPassBinaryPtr(new SecondPassBinary(*m_interface->getFirstPassPtr(), MethodBlock::BLOCK_RET,
                                                       m_interface->getStackSize(), m_interface->getAlignBuffer()));
    }
    catch (DependencyException& de)
    {
        XSTL_THROW(CompilerException, m_methodRunnable.getFullMethodName(), &de);
    }
    catch (...)
    {
        XSTL_THROW(CompilerException, m_methodRunnable.getFullMethodName(), NULL);
    }

    // Add debug information
    ret->getDebugInformation().setMethodName(m_methodRunnable.getFullMethodName());

    // Compile the cleanup routine for this method
    methodCreateCleanupFunc(thisElementType, locals, args, engineThread, m_methodRunnable.getFullMethodName(), ret);

    for (MethodHelperList::iterator iHelper = m_helpers.begin(); iHelper != m_helpers.end(); iHelper++)
    {
        MethodHelper& helper = (*iHelper);
        if (!m_interface->getCompilerParameters().m_bSupportExceptionHandling)
        {
            // If no exception handling, then only compile finally handler helpers
            if (helper.getClause().flags != MethodHeader::ClauseFinally)
                continue;
        }

        {
            SecondPassBinary::ResolveHelperList list;
            MethodHelper* parentHelper = helper.m_parent;
            while (parentHelper != NULL)
            {
                cString sParentName = CallingConvention::serializedMethod(parentHelper->getHandlerTokenIndex());
                list.append(SecondPassBinary::ResolveHelper(parentHelper->m_secondPass, sParentName, helper.m_compiler->getFirstPassPtr()));
                parentHelper = parentHelper->m_parent;
            }

            // Add the method itself to the list
            list.append(SecondPassBinary::ResolveHelper(ret, CallingConvention::serializedMethod(buildTokenIndex(m_apartment->getUniqueID(), m_methodToken)), m_interface->getFirstPassPtr()));

            // Generate second-pass binary for this helper function
            helper.m_secondPass = SecondPassBinaryPtr(new SecondPassBinary(
                *helper.m_compiler->getFirstPassPtr(), MethodBlock::BLOCK_RET,
                helper.m_compiler->getStackSize(), helper.m_compiler->getAlignBuffer(),
                &list));
        }

        // Give a name to the cleanup function
        helper.m_secondPass->getDebugInformation().setMethodName(m_methodRunnable.getFullMethodName() + helper.nameSuffix());

        // Ilan: please look at the precompiled-repository. We can use signature to unite functions
        // Add the function to the repository with a unique token

        // Calculate signature and notify repositories
        CRC64 crc64;
        mdToken token(getTokenID(helper.getHandlerTokenIndex()));
        crc64.update(&token, sizeof(token));
        engineThread.onMethodCompiled(m_apartment->getApartmentName(), crc64.digest(), helper.getHandlerTokenIndex(), helper.m_secondPass, false);
    }

    // Check for export name
    CustomAttributes customAttributes;
    CustomAttribute::getAttributes(m_apartment,
                                   m_methodToken,
                                   gCustomAttributeExportClass,
                                   customAttributes);

    if (0 != customAttributes.getSize())
    {
        CustomAttributePtr customAttributePtr = customAttributes[0];
        XSTL_TRY
        {
            ret->getDebugInformation().setExportName(customAttributePtr->getArgument("exportName").getStringValue());
        }
        XSTL_CATCH (CustomAttribute::MissingArgument)
        {
            // Do nothing, revert to default.
        }
    }

    return ret;
}

void MethodCompiler::methodEstimateEncoding(MethodRuntimeBoundle& boundle,
                                            MethodHelper* pCurrentHandler)
{
    boundle.m_compiler->enableOptimizations();

    // Linked all internal basic blocks
    const FirstPassBinary::BasicBlockList& blocks(boundle.getCompiler().getFirstPassPtr()
                                                             ->getBlocksList());
    FirstPassBinary::BasicBlockList::iterator blockItr(blocks.begin());

    for (; blockItr != blocks.end(); ++blockItr)
    {
        // For each block, check it's condition break
        MethodBlock& block((MethodBlock&)(*(*blockItr).m_stack));
        // Prepare for information addition.
        boundle.m_compiler->getFirstPassPtr()->changeBasicBlock(block.getBlockID());

        EmitContext emitContext(m_methodRunnable, boundle, block, pCurrentHandler);

        bool shouldUseShortAddress = false;
        switch (block.getConditionalCase())
        {
        // No relocation is required. Drop by default
        case MethodBlock::COND_NON: break;

        case MethodBlock::COND_ALWAYS:
            // Check the condition address
            shouldUseShortAddress = canUseShortRelativeAddress(
                                                block.getBlockID(),
                                                block.getConditionBlock(),
                                                boundle.m_compiler->getShortJumpLength(),
                                                blocks);
            // And add jump
            CompilerEngine::jumpToAddress(emitContext,
                                          block.getConditionBlock(),
                                          shouldUseShortAddress);
            break;

        case MethodBlock::COND_ZERO:
        case MethodBlock::COND_NON_ZERO:
            // Check the next conditions
            shouldUseShortAddress = canUseShortRelativeAddress(
                                                block.getBlockID(),
                                                block.getConditionBlock(),
                                                boundle.m_compiler->getShortJumpLength(),
                                                blocks);

            // Try to encode test address
            CompilerEngine::simpleConditionalJump(emitContext,
                          block.getConditionBlock(),
                          shouldUseShortAddress,
                          block.getConditionalCase() == MethodBlock::COND_ZERO);
            break;

        case MethodBlock::COND_RETURN:

            // Optimization, check if the next block is return block
            if ((*(blockItr + 1)).m_blockNumber != MethodBlock::BLOCK_RET)
            {
                // Jump to last block
                CompilerEngine::jumpToAddress(emitContext,
                                            MethodBlock::BLOCK_RET,
                                            shouldUseShortAddress);
            }
            break;

        default:
            // Please append new block relations.
            if (!block.isConditionFinialized())
            {
                CHECK_FAIL();
            } else
            {
                ASSERT(false);
            }
        }

        // Mark the block as complete
        block.finalizeCondition();
        boundle.m_compiler->renderBlock();
    }
    boundle.m_compiler->disableOptimizations();
}

uint MethodCompiler::calculateStackSize(const ElementsArrayType& locals,
                                        LocalPositions& localsPos) const
{
    uint stackSize = 0;
    uint count = locals.getSize();
    localsPos.m_localPosition.changeSize(count, false);
    localsPos.m_localTypes.changeSize(count, false);


    // For each local
    for (uint i = 0; i < count; ++i)
    {
        localsPos.m_localTypes[i] = locals[i];
        localsPos.m_localPosition[i] = stackSize;

        // TODO! Improve this code and move it into separate block

        uint elemSize = m_apartment->getObjects().getTypedefRepository().getTypeSize(locals[i]);

        // If this check fails, then you must have forgotten a case
        CHECK(elemSize > 0);
        elemSize = stackAlign(elemSize, *m_interface);
        stackSize+= elemSize;
    }

    return stackSize;
}

uint MethodCompiler::stackAlign(uint size, const CompilerInterface& compilerInterface)
{
    switch (compilerInterface.getStackSize())
    {
    case CompilerInterface::STACK_32:
        return Alignment::alignUpToDword(size);
    case CompilerInterface::STACK_64:
        return Alignment::alignUpToQword(size);
    default:
        // Cannot calculate alignment
        ASSERT(false);
        CHECK_FAIL();
    }
}

bool MethodCompiler::canUseShortRelativeAddress(int currentBlock,
                                                int jmpAddress,
                                                uint compilerInterfaceThershold,
                         const FirstPassBinary::BasicBlockList& blocks) const
{
    // Get the estimate|MAX| size of 'jmpAddress' and 'currentAddress' by
    // scanning all blocks
    uint jmpAddressPosition = 0;
    uint currentAddressPosition = 0;
    bool jmpAddressCalculated = false;
    bool currentAddressCalculated = false;

    FirstPassBinary::BasicBlockList::iterator i(blocks.begin());
    for (; i != blocks.end(); ++i)
    {
        int blockNumber = (*i).m_blockNumber;

        MethodBlock& block((MethodBlock&)(*(*i).m_stack));


        uint currentBlockSize = getMaxBlockSize(block, *i);

        if (blockNumber == jmpAddress)
        {
            jmpAddressCalculated = true;
            // Check whether we can stop scanning
            if (currentAddressCalculated) break;
        }
        if (blockNumber == currentBlock)
        {
            // Append location details
            currentAddressPosition+= currentBlockSize;
            // This is the location for the current address
            currentAddressCalculated = true;
            if (jmpAddressCalculated) break;
        }

        // Increase sizes
        if (!jmpAddressCalculated)
            jmpAddressPosition+= currentBlockSize;
        if (!currentAddressCalculated)
            currentAddressPosition+= currentBlockSize;
    }

    uint distance = t_abs(((int)currentAddressPosition) -
                          ((int)jmpAddressPosition));
    // TODO! Add CompilerInterface getShortJumpSize()
    return distance < compilerInterfaceThershold;
}

uint MethodCompiler::getMaxBlockSize(const MethodBlock& block,
                               const FirstPassBinary::BasicBlock& binary) const
{
    uint ret = binary.m_data.getSize();
    switch (block.getConditionalCase())
    {
    // Default condition doesn't have any additional information
    case MethodBlock::COND_NON: break;

    case MethodBlock::COND_RETURN:
    case MethodBlock::COND_ALWAYS:
    case MethodBlock::COND_NON_ZERO:
    case MethodBlock::COND_ZERO:
        // Max jmp instruction for all CPU is 8 bytes
        ret+= 8;
        break;
    default:
        if (!block.isConditionFinialized())
        {
            // TODO! Append condition length estimation!
            ASSERT(false);
        }
        // Else, the block already being handled.
    }

    return ret;
}

SecondPassBinaryPtr MethodCompiler::compileCCTORWrapper(const Apartment& apartment,
                                                        CompilerFactory::CompilerType compilerType,
                                                        const CompilerParameters compilerParams,
                                                        const TokenIndex& globalBool, mdToken cctorMethod)
{
    CompilerInterfacePtr compilerInterface = CompilerFactory::getCompiler(compilerType, apartment.getObjects().getFrameworkMethods(), compilerParams);
    // No locals, arguments
    compilerInterface->setLocalsSize(0);
    compilerInterface->setArgumentsSize(0);

    // Create new stack
    StackInterfacePtr tempStack(new MethodBlock(0, *compilerInterface->getInnerCompilerInterface()));

    compilerInterface->getFirstPassPtr()->createNewBlockWithoutChange(0, tempStack);
    compilerInterface->getFirstPassPtr()->changeBasicBlock(0);

    // Compile block #0
    //
    // if (!gIsCctorCalled)
    // {
    //
    StackLocation r0 = tempStack->allocateTemporaryRegister();
    StackLocation r1 = tempStack->allocateTemporaryRegister();

    // Check if the value is 1, if so, then the constructor is initialized and called
    // so jump o block #2 which is the ret
    compilerInterface->loadInt32(r1, 1);
    compilerInterface->load32(r0, 1, false, CallingConvention::serializeToken(globalBool));
    compilerInterface->ceq32(r0, r1);
    compilerInterface->jumpCondShort(r0, 2, false); // jmp to block #2 if r0==1

    // Compile block #2
    //    gIsCctorCalled = 1
    //    call .cctor()
    // }
    compilerInterface->getFirstPassPtr()->createNewBlockWithoutChange(1, tempStack);
    compilerInterface->getFirstPassPtr()->changeBasicBlock(1);
    compilerInterface->store32(r1, 1, false, CallingConvention::serializeToken(globalBool));
    tempStack->freeTemporaryRegister(r0);
    tempStack->freeTemporaryRegister(r1);

    // TODO!
    // CallingConvention::call(emitContext, buildTokenIndex(apartment.getUniqueID(), cctorMethod));

    MethodDefOrRefSignature methodSignature(ConstElements::gVoid,
                                            ElementsArrayType(),
                                            false,
                                            false,
                                            MethodDefOrRefSignature::CALLCONV_DEFAULT);
    cString dependencyTokenName(CallingConvention::serializedMethod(buildTokenIndex(apartment.getUniqueID(), cctorMethod)));
    compilerInterface->call(dependencyTokenName, 0);

    // No need to revertStack on cdecl since the cctor has no arguments

    // Compile block #3: ret
    compilerInterface->getFirstPassPtr()->createNewBlockWithoutChange(2, tempStack);
    compilerInterface->getFirstPassPtr()->changeBasicBlock(2);


    // Seal the method.
    // Second pass method linker, and return
    compilerInterface->generateMethodEpiProLogs();
    compilerInterface->getFirstPassPtr()->seal();

    SecondPassBinaryPtr ret(new SecondPassBinary(*compilerInterface->getFirstPassPtr(), MethodBlock::BLOCK_RET,
        compilerInterface->getStackSize(), compilerInterface->getAlignBuffer()));
    ret->getDebugInformation().setMethodName(cString("cctorWrapper") + HEXDWORD(cctorMethod));
    return ret;
}

