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
 * CompilerFactory.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerFactory.h"
#include "compiler/OptimizerCompilerInterface.h"
#include "compiler/OptimizerOperationCompilerInterface.h"
#include "compiler/processors/arm/ARMCompilerInterface.h"
#include "compiler/processors/arm/THUMBCompilerInterface.h"
#include "compiler/processors/ia32/IA32CompilerInterface.h"
#include "compiler/processors/c/32C.h"


class IA32MemoryLayout : public MemoryLayoutInterface
{
    virtual uint pointerWidth() const
    {
        return sizeof(uint32);
    }


    virtual uint align(uint size) const
    {
        return size;
    }
};

class ARMMemoryLayout : public MemoryLayoutInterface
{
    virtual uint pointerWidth() const
    {
        return sizeof(uint32);
    }


    virtual uint align(uint size) const
    {
        return Alignment::alignUpToDword(size);
    }
};


CompilerInterfacePtr CompilerFactory::getCompiler(CompilerType type, const FrameworkMethods& framework, const CompilerParameters& params /* = CompilerInterface::defaultParameters */)
{
    CompilerInterface* pCompiler = NULL;
    switch(type)
    {
    case COMPILER_IA32:
        pCompiler = new IA32CompilerInterface(framework, params);
        break;
    case COMPILER_32C:
        pCompiler = new c32CCompilerInterface(framework, params);
        break;
    case COMPILER_ARM:
        pCompiler = new ARMCompilerInterface(framework, params);
        break;
    case COMPILER_THUMB:
        pCompiler = new THUMBCompilerInterface(framework, params);
        break;
    default:
        CHECK_FAIL();
    }

    if (params.m_bEnableOptimizations)
    {
        // Wrap the real arch-compiler with an optimizer compiler
        OptimizerOperationCompilerInterface* pOptimizee = dynamic_cast<OptimizerOperationCompilerInterface*>(pCompiler);

        // If the dynamic cast failed, then this arch does not support optimizations...
        if (pOptimizee == NULL)
            XSTL_THROW(ClrRuntimeException, XSTL_STRING("This architecture-compiler does not support optimizations"));

        return CompilerInterfacePtr(new OptimizerCompilerInterface(OptimizerOperationCompilerInterfacePtr(pOptimizee)));
    }

    // return the real arch-compiler
    return CompilerInterfacePtr(pCompiler);
}

MemoryLayoutInterfacePtr CompilerFactory::getMemoryLayout(CompilerType type)
{
    switch(type)
    {
    case COMPILER_IA32:
        return MemoryLayoutInterfacePtr(new IA32MemoryLayout());
    case COMPILER_32C:
        return MemoryLayoutInterfacePtr(new IA32MemoryLayout());
    case COMPILER_ARM:
        return MemoryLayoutInterfacePtr(new ARMMemoryLayout());
    case COMPILER_THUMB:
        return MemoryLayoutInterfacePtr(new ARMMemoryLayout());
    default:
        CHECK_FAIL();
        break;
    }
    return MemoryLayoutInterfacePtr(NULL);
}

