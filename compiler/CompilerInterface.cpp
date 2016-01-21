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
 * CompilerInterface.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerInterface.h"

const CompilerParameters CompilerInterface::defaultParameters = {
    true, //m_bSupportExceptionHandling
    false, //m_bEnableOptimizations
    false, //m_bDeveloperVerbosity
};

CompilerInterface::~CompilerInterface()
{
}

CompilerInterface::CompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params) : m_parameters(params), m_framework(framework), m_optimizationRefCount(0)
{
}

void CompilerInterface::setFramePointer(StackLocation destination)
{

}

void CompilerInterface::disableOptimizations()
{
    if (m_optimizationRefCount > 0)
        m_optimizationRefCount--;
    else
        m_optimizationRefCount = 0;

}

void CompilerInterface::enableOptimizations()
{
    m_optimizationRefCount++;
}

bool CompilerInterface::isOptimizerOn() const
{
    return m_optimizationRefCount > 0;
}

CompilerInterface::CallingConvention CompilerInterface::getDefaultCallingConvention(void) const
{
    bool isStdCall = getFirstPassPtr()->isDefaultStdCall();
    if (true == isStdCall) {
        return STDCALL;
    }

    return C_DECL;
}

const FirstPassBinaryPtr& CompilerInterface::getFirstPassPtr() const
{
    return m_binary;
}

FirstPassBinaryPtr& CompilerInterface::getFirstPassPtr()
{
    return m_binary;
}

const RegisterAllocationTable& CompilerInterface::getArchRegisters() const
{
    return m_archRegisters;
}

cBufferPtr CompilerInterface::getAlignBuffer() const
{
    return cBufferPtr(NULL);
}

const CompilerParameters& CompilerInterface::getCompilerParameters() const
{
    return m_parameters;
}

void CompilerInterface::renderBlock()
{
    // The default action is not to do anything when a blcok is sealed.
}

CompilerInterface* CompilerInterface::getInnerCompilerInterface()
{
    return this;
}

uint CompilerInterface::getNumberOfRegisters()
{
    return m_archRegisters.keys().length();
}

int CompilerInterface::getGPEncoding(int gpreg)
{
    return -(gpreg + GP32_BASE);
}

bool CompilerInterface::isOptimizerCompiler() const
{
    return false;
}

StackLocation CompilerInterface::getMethodBaseStackRegister() const
{
    return m_binary->getCurrentStack()->getBaseStackRegister();
}

void CompilerInterface::setMethodBaseStackRegister(StackLocation stackLocation)
{
    m_binary->getCurrentStack()->setBaseStackRegister(stackLocation);
}