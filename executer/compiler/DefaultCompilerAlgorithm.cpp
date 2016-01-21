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
 * DefaultCompilerAlgorithm.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/except/assert.h"
#include "xStl/stream/traceStream.h"
#include "format/EncodingUtils.h"
#include "compiler/CallingConvention.h"
#include "runnable/GlobalContext.h"
#include "executer/ExecuterTrace.h"
#include "executer/runtime/MethodBinder.h"
#include "executer/runtime/Executer.h"
#include "executer/compiler/DefaultCompilerAlgorithm.h"
#include "executer/linker/MemoryLinker.h"

DefaultCompilerAlgorithm::DefaultCompilerAlgorithm(const TokenIndex& mainMethod,
                                                   ApartmentPtr mainApartment,
                                                   CompilerEngineThread& engine,
                                                   const LinkerInterfacePtr& linker) :
    m_shouldExit(false),
    m_mainMethod(mainMethod),
    m_mainApartment(mainApartment),
    m_engine(engine),
    m_externalModuleResolver(mainApartment),
    m_linker(linker)
{
    // Start scanning functions and find: static .cctor for structure initializing
    // and framework implemented functions.

    // Store the first method into the list
    m_methodStack.push(mainMethod);
    // Make sure that the event is in it's reset mode.
    m_event.resetEvent();
}

bool DefaultCompilerAlgorithm::getNextMethod(TokenIndex& mid)
{
    // Check for previous death
    if (m_shouldExit)
    {
        // This shouldn't be happening.
        ExecuterTrace("DefaultCompilerAlgorithm: Called even if all functions has resolved" << endl);
        ASSERT(false);
        return false;
    }

    // TODO! Simple implementation
    cLock lock(m_lock);
    if (m_methodStack.isEmpty())
    {
        m_shouldExit = true;
        // Time to execute out main function!
        m_linker->resolveAndExecuteAllDependencies(m_mainMethod);

        return false;
    }

    mid = m_methodStack.getArg(0);
    m_methodStack.pop2null();
    return true;
}

bool DefaultCompilerAlgorithm::shouldExit() const
{
    return m_shouldExit;
}

void DefaultCompilerAlgorithm::onMethodCompiled(const TokenIndex& mid,
                                                SecondPassBinary& compiled, bool inCache)
{
    // NOTE! If the method is in the repository then it ALL of it's sub-methods
    //       are also in the repository. Change this method if methods are paged
    //       out from the pool
    if (!inCache)
    {
        // Try to find all sub-methods
        const BinaryDependencies::DependencyObjectList& dependencies = compiled.getDependencies().getList();
        BinaryDependencies::DependencyObjectList::iterator i = dependencies.begin();
        for (; i != dependencies.end(); ++i)
        {
            // Check for CIL methods links
            TokenIndex methodToken;
            if (CallingConvention::deserializeMethod((*i).m_name, methodToken))
            {
                m_methodStack.push(m_clrResolver.resolve(m_mainApartment, methodToken));
            } else if (CallingConvention::deserializeToken((*i).m_name, methodToken))
            {
                // Check for vtbl

                // #### Todo: This is only needed for vtbl entries that were actually used in the code!
                //            Any virtual method which was not called anywhere in the code, should be optimized away
                if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == TABLE_TYPEDEF_TABLE)
                {
                    const ResolverInterface::VirtualTable& vTbl =
                                m_mainApartment->getObjects().getTypedefRepository().getVirtualTable(methodToken);
                    ResolverInterface::VirtualTable::iterator j = vTbl.begin();
                    for (; j != vTbl.end(); j++)
                    {
                        m_methodStack.push(getVtblMethodIndexOverride(*j));
                    }
                }
            }
        }
    }
}


void DefaultCompilerAlgorithm::onCompilationFailed(const TokenIndex& mid)
{
    // Break event!
    m_shouldExit = true;
}
