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
 * ExceptionOpcodes.cpp
 *
 * Implementation file
 *
 */

#include "compiler/stdafx.h"
#include "xStl/utils/algorithm.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerTrace.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/ExceptionOpcodes.h"
#include "compiler/opcodes/ObjectOpcodes.h"
#include "compiler/opcodes/CompilerOpcodes.h"

bool ExceptionOpcodes::implementLeave(EmitContext& emitContext, int instructionIndex, int targetIndex)
{
    if (emitContext.currentBlock.getExceptionsStack().isEmpty())
        // Treat this "leave" as a jump.
        return false;

    MethodBlock::ExceptionEntry& exception = *(emitContext.currentBlock.getExceptionsStack().end() - 1);
    if ((exception.getCurrentPart() == MethodBlock::ProtectedBlock) &&
        (exception.hitTest(instructionIndex) == MethodBlock::ProtectedBlock))
    {
        // Case 1: "leave" instruction inside a protected block
        // Create the new block for the target index
        MethodBlock* newBlock = emitContext.currentBlock.duplicate(emitContext, targetIndex);
        StackInterfacePtr newBlockPtr(newBlock);

        // Identify exactly which protected blocks we are leaving by this instruction
        MethodBlock::ExceptionList::iterator i = newBlock->getExceptionsStack().end() - 1;
        while ((*i).hitTest(targetIndex) != (*i).getCurrentPart())
        {
            // (*i) is an excepion clause that we are leaving now.

            // pop the handler from the registered handler stack by calling pop()
            // if it's a finally block - then execute too:

            if (emitContext.methodRuntime.m_compiler->getCompilerParameters().m_bSupportExceptionHandling)
            {
                TokenIndex routine;
                if ((*i).getClause().flags == MethodHeader::ClauseFinally)
                    routine = emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getPopAndExec();
                else
                    routine = emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getPop();
                CallingConvention::call(emitContext, routine);
            }
            else
            {
                // Only finally helpers are supported if exception handling is disabled. Others are ignored
                if ((*i).getClause().flags == MethodHeader::ClauseFinally)
                {
                    cString dependencyTokenName(CallingConvention::serializedMethod((*i).getHandlerTokenIndex()));
                    // Push the current base pointer
                    emitContext.methodRuntime.m_compiler->pushArg32(emitContext.currentBlock.getBaseStackRegister());
                    // And call the finally handler function directly
                    emitContext.methodRuntime.m_compiler->call(dependencyTokenName, 1);
                }
            }

            // pop this exception clause from the new block's exception stack
            i = newBlock->getExceptionsStack().remove(i);
            if (newBlock->getExceptionsStack().isEmpty())
                break;

            // i is the end() iterator now, so go back one place in the list to the previous clause
            i--;
        }

        // Generate a new block for the target index
        emitContext.methodRuntime.m_blockStack.add(newBlockPtr);
    }
    else if ((exception.getCurrentPart() == MethodBlock::Handler) &&
                (exception.hitTest(instructionIndex) == MethodBlock::Handler))
    {
        // Case 2: leave instruction inside a handler:

        // Must be in a catch/filter handler, and not anything else
        CHECK((exception.getClause().flags == MethodHeader::ClauseCatch) ||
                (exception.getClause().flags == MethodHeader::ClauseFilter));
        // If no exception handling, then this helper function should not be compiled at all.
        CHECK(emitContext.methodRuntime.m_compiler->getCompilerParameters().m_bSupportExceptionHandling);

        // Revert back to original base stack pointer register
        emitContext.methodRuntime.m_compiler->resetBaseStackRegister(exception.getOrigBaseStackRegister());

        if (exception.getOrigBaseStackRegister() != exception.getOrigCompiler().getStackPointer())
        {
            // Restore the stack frame pointer to the catch helper's parent frame

            // Call getCurrentExeption to Fill the register with a pointer to the object
            cString dependencyTokenName(CallingConvention::serializedMethod(
                emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getCurrentStackPointerRet()));
            emitContext.methodRuntime.m_compiler->call32(dependencyTokenName, exception.getOrigCompiler().getStackPointer(), 0);
        }

        // Jump to target, regardless of anything else.
        // Note: This creates a dependency which will break once the catch-handler FPB blocks are moved
        //       info the FPB of the helper method.
        emitContext.methodRuntime.m_compiler->jump(targetIndex);
    }
    else
    {
        // Case 3: leave instruction somewhere else in the clause?! in filter code?! a bug?!
        CHECK(FALSE);
    }

    // And jump to the leave target
    emitContext.currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON, targetIndex);
    return true;
}

void ExceptionOpcodes::implementEndFinally(EmitContext& emitContext, uint instructionIndex)
{
    // Must appear inside an exception handler block
    CHECK(emitContext.pCurrentHelper != NULL);

    // Must be in a finally/fault block, and nowhere else
    CHECK((emitContext.pCurrentHelper->getClause().flags == MethodHeader::ClauseFinally) ||
            (emitContext.pCurrentHelper->getClause().flags == MethodHeader::ClauseFault));

    // Treat this like a "ret" from the handler helper function
    emitContext.currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_RETURN,
        MethodBlock::BLOCK_RET,
        instructionIndex);
}

void ExceptionOpcodes::implementThrow(EmitContext& emitContext, uint instructionIndex)
{
    if (!emitContext.methodRuntime.getCompiler().getCompilerParameters().m_bSupportExceptionHandling)
    {
        // Pop the exception object
        CompilerOpcodes::pop2null(emitContext);
        // Any following code should not be executed, but it will
        emitContext.currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON, instructionIndex);
        return;
    }

    GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();

    // The exception object is at the top of the evaluation stack already
    // Process the exception by calling throwException()
    TokenIndex routine = globalContext.getFrameworkMethods().getThrowException();
    CallingConvention::call(emitContext, routine);

    // Any following code will not be executed
    emitContext.currentBlock.terminateMethodBlock(&emitContext, MethodBlock::COND_NON, instructionIndex);
}

void ExceptionOpcodes::enterProtectedBlock(EmitContext& emitContext, uint index, MethodHelperList& helpers)
{
    // Search all method's clauses for ones that starts on this index
    const MethodHeader::ExceptionHandlingClauseList& exceptions = emitContext.methodContext.getMethodHeader().getExceptionsHandlers();
    for (MethodHeader::ExceptionHandlingClauseList::iterator i = exceptions.begin(); i != exceptions.end(); i++)
    {
        const MethodHeader::ExceptionHandlingClause& clause = (*i);
        // Are we just entering this protected block?
        if (index == clause.tryOffset)
        {
            CompilerTrace("Entering protected block at " << HEXWORD(index) << endl);
            MethodBlock::ExceptionEntry exceptionEntry(clause, emitContext.currentBlock.getBaseStackRegister(), *emitContext.methodRuntime.m_compiler);

            // Generate a unique token for the helper handler function
            mdToken handlerToken = EncodingUtils::buildToken(TABLE_CLR_METHOD_HELPERS,
                                                             emitContext.methodContext.getApartment()->generateMethodHelperRow());
            exceptionEntry.setHandlerTokenIndex(buildTokenIndex(emitContext.methodContext.getApartment()->getUniqueID(), handlerToken));

            if (emitContext.methodRuntime.getCompiler().getCompilerParameters().m_bSupportExceptionHandling)
            {
                if (clause.flags == MethodHeader::ClauseFilter)
                {
                    // Not implemented yet
                    ASSERT(FALSE);
                }

                if (clause.flags == MethodHeader::ClauseCatch)
                {
                    // Push another block - after the whole clause
                    // TODO: Instead, push another block at the location where the catch handler's "leave" instruction points
                    uint afterTry = clause.tryOffset + clause.tryLength;
                    uint afterHandler = clause.handlerOffset + clause.handlerLength;

                    MethodBlock* newBlock = emitContext.currentBlock.duplicate(emitContext, t_max(afterTry, afterHandler));
                    emitContext.methodRuntime.m_blockStack.add(StackInterfacePtr(newBlock));

                    // Determine the caught type's RTTI
                    ElementType catchType = ElementType(ELEMENT_TYPE_CLASS, 0, false, false, false,
                        buildTokenIndex(emitContext.methodContext.getApartment()->getUniqueID(), clause.classTokenOrFilterOffset));
                    emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().resolveTyperef(catchType);
                    uint rtti = emitContext.methodContext.getApartment()->getObjects().getTypedefRepository().getRTTI(catchType.getClassToken());

                    // Param1 for RegisterCatch is the RTTI of the class being caught
                    StackEntity type(StackEntity::ENTITY_CONST, ConstElements::gU2);
                    type.getConst().setConstValue(rtti);
                    emitContext.currentBlock.getCurrentStack().push(type);

                    emitContext.methodRuntime.m_bHasCatch = true;
                }
                else
                {
                    // Param1 for registerRoutine is the type
                    StackEntity type(StackEntity::ENTITY_CONST, ConstElements::gU4);
                    type.getConst().setConstValue(FrameworkMethods::GetExceptionRoutineType(clause.flags));
                    emitContext.currentBlock.getCurrentStack().push(type);
                }

                // Param2 is the address of the cleanup function. store in a stack entity
                StackEntity cleanupFunctionAddress(StackEntity::ENTITY_METHOD_ADDRESS, ConstElements::gVoidPtr);
                cleanupFunctionAddress.getConst().setTokenIndex(exceptionEntry.getHandlerTokenIndex());
                emitContext.currentBlock.getCurrentStack().push(cleanupFunctionAddress);

                // Param3 is the stack frame pointer. Load it onto a stack entity
                TemporaryStackHolderPtr framePointer(new TemporaryStackHolder(emitContext.currentBlock, ELEMENT_TYPE_PTR, emitContext.methodRuntime.m_compiler->getStackSize(), TemporaryStackHolder::TEMP_ONLY_REGISTER));
                emitContext.methodRuntime.m_compiler->setFramePointer(framePointer->getTemporaryObject());
                StackEntity framePointerEntity(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
                framePointerEntity.setStackHolderObject(framePointer);
                emitContext.currentBlock.getCurrentStack().push(framePointerEntity);

                TokenIndex routine;
                if (clause.flags == MethodHeader::ClauseCatch)
                {
                    // Param4 for registerCatch is the stack-frame pointer (EBP) of the current function
                    StackEntity framePointer(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
                    framePointer.setStackHolderObject(
                        TemporaryStackHolderPtr(new TemporaryStackHolder(
                            emitContext.currentBlock, emitContext.methodRuntime.getCompiler().getStackPointer())));
                    emitContext.currentBlock.getCurrentStack().push(framePointer);

                    routine = emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getRegisterCatch();
                }
                else
                    routine = emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getRegisterRoutine();

                // Call clrRegisterRoutine
                CallingConvention::call(emitContext, routine);
            }

            // Remember this protected block on the exception stack for now
            exceptionEntry.setCurrentPart(MethodBlock::ProtectedBlock);
            emitContext.currentBlock.getExceptionsStack().append(exceptionEntry);

            // And remember this protected block's handler for compilation later
            exceptionEntry.setCurrentPart(MethodBlock::Handler);
            helpers.append(MethodHelper(exceptionEntry, emitContext.pCurrentHelper));
        }
    }
}

MethodHelper::MethodHelper(const MethodBlock::ExceptionEntry& entry, MethodHelper* parent) :
    MethodBlock::ExceptionEntry(entry),
    m_parent(parent),
    m_nTotalTempStack(0)
{
}
