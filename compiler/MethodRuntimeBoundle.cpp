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
 * MethodRuntimeBoundle.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/MethodRuntimeBoundle.h"
#include "compiler/EmitContext.h"

MethodRuntimeBoundle::MethodRuntimeBoundle(CompilerInterfacePtr compiler,
                                           const LocalPositions& locals,
                                           const ArgumentsPositions& args,
                                           const MethodHeader& methodHeader,
                                           TokenIndex cleanupIndex) :
    m_compiler(compiler),
    m_locals(locals),
    m_args(args),
    m_methodSet(methodHeader.getFunctionLength()),
    m_bHasCatch(false),
    m_cleanupIndex(cleanupIndex)
{
    // Initialize first block
    m_blockStack.add(StackInterfacePtr(new MethodBlock(DEFAULT_BASIC_BLOCK_START, *m_compiler)));
}

MethodRuntimeBoundle::MethodRuntimeBoundle(const MethodRuntimeBoundle& other, uint handlerIndex) :
    m_compiler(other.m_compiler),
    m_locals(other.m_locals),
    m_args(other.m_args),
    m_methodSet(other.m_methodSet.getLength()),
    m_bHasCatch(false),
    m_blockSplit(other.m_blockSplit),
    m_cleanupIndex(other.m_cleanupIndex)
{
    // Initialize first block - at handler's initial index
    m_blockStack.add(StackInterfacePtr(new MethodBlock(handlerIndex, *m_compiler)));
}

MethodRuntimeBoundle::~MethodRuntimeBoundle()
{
}

const CompilerInterface& MethodRuntimeBoundle::getCompiler()
{
    return *m_compiler;
}

void MethodRuntimeBoundle::OnOffset(uint instructionIndex)
{
    m_blockSplit.set(instructionIndex);
}

void MethodRuntimeBoundle::AddMethodBlock(MethodBlock& block, EmitContext& emitContext, int newBlockID, bool bRemoveTOS /* = false */)
{
    // Look for an existing block
    for (MethodBlockOrderedList::iterator i = m_blockStack.begin(); i != m_blockStack.end(); i++)
    {
        MethodBlock* pBlock = (MethodBlock*)(*i).getPointer();
        if (pBlock->getBlockID() == newBlockID)
        {
            // Block exists already. Only update it
            // TODO: Also combine the temp-stack contents!
            pBlock->UpdateTotalTempStack(block.getTotalTemporaryStackSize());
            return;
        }
    }

    // Block does not exist. Add it
    m_blockStack.add(StackInterfacePtr(block.duplicate(emitContext, newBlockID, bRemoveTOS)));
}
