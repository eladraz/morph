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
 * IA32CompilerInterface.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "dismount/proc/ia32/opcodeTable.h"
#include "dismount/assembler/AssemblingFactory.h"
#include "dismount/assembler/MangledNames.h"
#include "dismount/assembler/StackInterface.h"
#include "compiler/MethodBlock.h"
#include "compiler/processors/ia32/IA32CompilerInterface.h"


// Optimization
static const cString g_byteptrOnly("byte ptr ");
static const cString g_wordptrOnly("word ptr ");
static const cString g_dwordptrOnly("dword ptr ");

static const char g_open = '[';
static const char g_terminate = ']';

#define EAX (0)
#define EDI (1)
#define ESI (2)
#define EBX (3)
#define ECX (4)
#define EDX (5)

IA32CompilerInterface::IA32CompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params) : OptimizerOperationCompilerInterface(framework, params)
{
    // Generate new first-binary pass
    // Don't forget the negative size! This sign mark the register as temporary
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_EAX), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_EBX), RegisterEntry(Volatile, false, 1));  // NOT COMPLICANT WITH GCC and MS x86 __fastcall convention (EDX:ECX) since mul+shr uses (and free) this register
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_EDX), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_EDI), RegisterEntry(Volatile, false, 3));
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_ECX), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ia32dis::IA32_GP32_ESI), RegisterEntry(NonVolatile, false, 2));
    //setBaseStackRegister(getStackPointer());
    m_binary = FirstPassBinaryPtr(new FirstPassBinary(
                                       OpcodeSubsystems::DISASSEMBLER_INTEL_32,
                                       true));

    // This code apply only to 32 bit intel machines.
    m_assembler = AssemblingFactory::generateAssembler(m_binary,
                                       OpcodeSubsystems::DISASSEMBLER_INTEL_32);

    m_indexToRegister.append(EAX, getGPEncoding(ia32dis::IA32_GP32_EAX));
    m_indexToRegister.append(EBX, getGPEncoding(ia32dis::IA32_GP32_EBX));
    m_indexToRegister.append(EDX, getGPEncoding(ia32dis::IA32_GP32_EDX));
    m_indexToRegister.append(EDI, getGPEncoding(ia32dis::IA32_GP32_EDI));
    m_indexToRegister.append(ECX, getGPEncoding(ia32dis::IA32_GP32_ECX));
    m_indexToRegister.append(ESI, getGPEncoding(ia32dis::IA32_GP32_ESI));
}



void IA32CompilerInterface::setLocalsSize(uint localStackSize)
{
    m_binary->setStackBaseSize(localStackSize);
}

void IA32CompilerInterface::setArgumentsSize(uint argsSize)
{
    m_binary->setArgumentsSize(argsSize);
}

IA32CompilerInterface::StackSize IA32CompilerInterface::getStackSize() const
{
    return STACK_32;
}

uint IA32CompilerInterface::getShortJumpLength() const
{
    return 0x80;
}

StackLocation IA32CompilerInterface::getStackPointer() const
{
    return StackInterface::buildStackLocation(getGPEncoding(ia32dis::IA32_GP32_EBP), 0);
}

void IA32CompilerInterface::localloc(StackLocation destination,
                                     StackLocation size,
                                     bool isStackEmpty)
{
#if 1
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "sub esp, " << getRegister32(size) << endl;
    compiler << "mov " << getRegister32(destination) << ", esp" << endl;
#else
    // TODO! This call can be implemented by reducing the base size of the stack
    //       and copying all pending stack into the new location.
    //
    //       Although this method is the best, it's not generic, the following
    //       implementation will use a function call which will later be
    //       implemented by the framework.
    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;

        // Push the stack size argument
        compiler << "push " << getRegister32(size) << endl;

        saveVolatileRegisters(compiler, destination.u.reg, size.u.reg);
        compiler << "call $+66600666" << endl;
    }

    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(
                m_frameWork.getLocAllocMethodName(),
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_RELATIVE,
                0,
                false,
                -4);

    // Eax contains the address of the used buffer, move to destination
    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        // Move the result to the required register.
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "mov " << getRegister32(destination) << ", eax" << endl;
    }

    // TODO Mark a destructor function to the end of the function called.
#endif
}

void IA32CompilerInterface::generateMethodEpiProLogs(bool bForceSaveNonVolatiles /* = false */)
{
    // Check for allocated registers, and debug!
    RegToLocation locationMap;

    // Add prolog
    // Generate new first stream, and switch to it.
    MethodBlock* prolog = new MethodBlock(MethodBlock::BLOCK_PROLOG, *this);
    StackInterfacePtr pstack(prolog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_PROLOG, pstack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_PROLOG);
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "push ebp" << endl;
        compiler << "mov  ebp, esp" << endl;
        uint stackSize = m_binary->getStackSize();
        if (stackSize > 0)
            compiler << "sub  esp, 0x" << HEXDWORD(stackSize) << endl;

        saveNonVolatileRegisters(*ia32compiler, locationMap, true, bForceSaveNonVolatiles);
    }
    prolog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);

    // Conclude the method by adding a epilog
    // Generate last stream, and switch to it.
    MethodBlock* epilog = new MethodBlock(MethodBlock::BLOCK_RET, *this);
    StackInterfacePtr estack(epilog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_RET, estack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_RET);
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        // Restore the non-volatile registers touching
        saveNonVolatileRegisters(compiler, locationMap, false, bForceSaveNonVolatiles);



        compiler << "mov  esp, ebp" << endl;
        compiler << "pop  ebp" << endl;
        if (m_binary->isStdCall())
            compiler << "retn " << (int)m_binary->getArgumentsSize() << endl;
        else
            compiler << "ret" << endl;
    }
    epilog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

// Generate instruction: mov [ebp + stackPosition], buffer
void IA32CompilerInterface::storeConst(uint stackPosition,
                                       const uint8* bufferOffset,
                                       uint  size,
                                       bool  argumentStackLocation)
{
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    // TODO! Add space for complex objects (3,5+ bytes)
    compiler << "mov " << getStackReference124(stackPosition, size,
                                               argumentStackLocation) << ", 0x";
    switch (size)
    {
    case 1: // 8 bit
        compiler << HEXBYTE(*bufferOffset) << endl;
        break;
    case 2: // 16 bit
        compiler << HEXWORD(*((const uint16*)bufferOffset)) << endl;
        break;
    case 4: // 32 bit
        compiler << HEXDWORD(*((const uint32*)bufferOffset)) << endl;
        break;
    }
}

void IA32CompilerInterface::store32(uint stackPosition,
                                    uint size,
                                    StackLocation source,
                                    bool argumentStackLocation,
                                    bool isTempStack)
{
    // Validate register
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    CHECK(size != 0);

    if ((size != 4) && (!isRegister8(source)))
    {
        freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        compiler << "mov eax, " << getRegister32(source) << endl;
        source.u.reg = getGPEncoding(ia32dis::IA32_GP32_EAX);
    }

    compiler << "mov ";
    if (isTempStack)
    {
        compiler << getTempStack32(stackPosition, size);
    } else
    {
        compiler << getStackReference124(stackPosition, size,
                                         argumentStackLocation);
    }
    compiler << ", ";

    switch (size)
    {
    case 1:
        compiler << getRegister8(source);
        break;
    case 2:
        compiler << getRegister16(source);
        break;
    case 4:
        compiler << getRegister32(source);
        break;
    }
    compiler << endl;
}

void IA32CompilerInterface::store32(StackLocation source,
                                    uint size,
                                    bool signExtend,
                                    const cString& dependencyName)
{
    // Validate register
    CHECK(isRegister32(source));
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;

        if (!isRegister8(source))
        {
            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
            compiler << "mov eax, " << getRegister32(source) << endl;
            source.u.reg = getGPEncoding(ia32dis::IA32_GP32_EAX);
        }

        compiler << "mov ";
        switch (size)
        {
        case 1: compiler << g_byteptrOnly; break;  // 8 bit
        case 2: compiler << g_wordptrOnly; break;  // 16 bit
        case 4: compiler << g_dwordptrOnly; break; // 32 bit
        default:
            CHECK_FAIL();
        }

        compiler << " [+66600666], ";

        switch (size)
        {
        case 1:
            compiler << getRegister8(source);
            break;
        case 2:
            compiler << getRegister16(source);
            break;
        case 4:
            compiler << getRegister32(source);
            break;
        }
        compiler << endl;
    }

    m_binary->getCurrentDependecies().addDependency(
        dependencyName,
        m_binary->getCurrentBlockData().getSize() - getStackSize(),
        getStackSize(),
        BinaryDependencies::DEP_ABSOLUTE,
        0,
        false,
        -4
        );
}

void IA32CompilerInterface::move32(StackLocation destination,
                                   StackLocation source,
                                   uint size,
                                   bool signExtend)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    CHECK(size != 0);
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    if (size == 4)
        compiler << "mov ";
    else if (signExtend)
    {
        compiler << "movsx ";
    } else
    {
        compiler << "movzx ";
    }
    compiler << getRegister32(destination) << ", ";
    compiler << getRegister32(source);

    compiler << endl;
}

// Generate instruction: mov/sx/zx reg32, [ebp + stackPosition]
void IA32CompilerInterface::load32(uint stackPosition,
                                   uint size,
                                   StackLocation destination,
                                   bool signExtend,
                                   bool argumentStackLocation,
                                   bool isTempStack)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(size != 0);
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    if (size == 4)
        compiler << "mov ";
    else if (signExtend)
    {
        compiler << "movsx ";
    } else
    {
        compiler << "movzx ";
    }
    compiler << getRegister32(destination) << ", ";

    if (!isTempStack)
    {
        compiler << getStackReference124(stackPosition, size,
                                         argumentStackLocation);
    } else
    {
        compiler << getTempStack32(stackPosition, size);
    }

    compiler << endl;
}

void IA32CompilerInterface::load32(StackLocation destination,
                                   uint size,
                                   bool signExtend,
                                   const cString& dependencyName)
{
    // Validate register
    CHECK(isRegister32(destination));

    if (1)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;

        if (size == 4)
        {
            compiler << "mov ";
        } else if (signExtend)
        {
            compiler << "movsx ";
        } else
        {
            compiler << "movzx ";
        }
        compiler << getRegister32(destination) << ", ";

        switch (size)
        {
        case 1: compiler << g_byteptrOnly;  break;  // 8 bit
        case 2: compiler << g_wordptrOnly;  break;  // 16 bit
        case 4: compiler << g_dwordptrOnly; break; // 32 bit
        default:
            CHECK_FAIL();
        }

        compiler << "[+66600666]" << endl;
    }

    m_binary->getCurrentDependecies().addDependency(
                dependencyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_ABSOLUTE,
                0,
                false,
                -4
                );
}

void IA32CompilerInterface::load32addr(uint stackPosition,
                                       uint size,
                                       uint offset,
                                       StackLocation destination,
                                       bool argumentStackLocation,
                                       bool isTempStack)
{
    // Validate register
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "lea " << getRegister32(destination) << ", ";

    compiler << g_open;
    compiler << getBaseStackRegister(getMethodBaseStackRegister());

    if (!argumentStackLocation)
    {
        // locals
        compiler << " - ";
        if (isTempStack)
        {
            stackPosition+= m_binary->getStackBaseSize() - StackInterface::LOCAL_STACK_START_VALUE;
        }
        //Add the size minus the offset into it
        stackPosition += Alignment::alignUpToDword(size) - offset;
    } else
    {
        // Argument
        // Add the EIP and EBP that are on the stack
        stackPosition+= 8 + offset;
        compiler << " + ";
    }

    // The -4 for the load32/store32 is not nessasery since we do that on "size"
    compiler << cString(stackPosition) << g_terminate << endl;
}

void IA32CompilerInterface::load32addr(StackLocation destination,
                                       const cString& dependencyName)
{
    // Validate register
    CHECK(isRegister32(destination));
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "mov " << getRegister32(destination) << ", 0x66600666" << endl;
    }
    m_binary->getCurrentDependecies().addDependency(
                dependencyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_ABSOLUTE,
                0,
                false,
                -4);
}

void IA32CompilerInterface::loadInt32(StackLocation destination,
                                      uint32 value)
{
    // Validate register
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "mov " << getRegister32(destination) << ", 0x"
             << HEXDWORD(value) << endl;
}

void IA32CompilerInterface::loadInt32(StackLocation destination,
                                      const cString& dependancyName)
{
    loadInt32(destination, -4);
    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(
                dependancyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_ABSOLUTE,
                0,
                true);
}

void IA32CompilerInterface::assignRet32(StackLocation source)
{
    // Validate register
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    if (getGPEncoding(source.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        // TODO! Allocate and HOLDS! EAX register.
        compiler << "mov eax, " << getRegister32(source) << endl;
    }
}

void IA32CompilerInterface::pushArg32(StackLocation source)
{
    // Validate register
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "push " << getRegister32(source) << endl;
}

void IA32CompilerInterface::popArg32(StackLocation source)
{
    // Validate register
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "pop " << getRegister32(source) << endl;
}

void IA32CompilerInterface::call(const cString& dependancyName, uint)
{
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        saveVolatileRegisters();
        compiler << "call $+66600666" << endl;
    }

    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(
                dependancyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_RELATIVE,
                0,
                false,
                -4);
}

void IA32CompilerInterface::call(StackLocation address, uint)
{
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;
    saveVolatileRegisters();
    compiler << "call " << getRegister32(address) << endl;
}

void IA32CompilerInterface::call32(const cString& dependancyName,
                                   StackLocation destination, uint)
{
    bool shouldCopyEax = false;
    if (destination.u.reg != getGPEncoding(ia32dis::IA32_GP32_EAX))
    {
        //TODO: there needs to be t least one room in the stack so we can use free Eax
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        shouldCopyEax = true;
    }

    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        saveVolatileRegisters(getGPEncoding(ia32dis::IA32_GP32_EAX),
                              destination.u.reg);
        compiler << "call $+66600666" << endl;
    }

    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(
                dependancyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_RELATIVE,
                0,
                false,
                -4
                );

    if (shouldCopyEax)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "mov " << getRegister32(destination) << ", eax" << endl;
    }
}

void IA32CompilerInterface::call32(StackLocation address,
                                   StackLocation destination, uint)
{
    bool shouldCopyEax = false;
    if (destination.u.reg != getGPEncoding(ia32dis::IA32_GP32_EAX))
    {
        //TODO: there needs to be t least one room in the stack so we can use free Eax
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        shouldCopyEax = true;
    }

    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        saveVolatileRegisters(getGPEncoding(ia32dis::IA32_GP32_EAX),
                              destination.u.reg);
        compiler << "call " << getRegister32(address) << endl;
    }

    if (shouldCopyEax)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "mov " << getRegister32(destination) << ", eax" << endl;
    }
}

void IA32CompilerInterface::storeMemory(StackLocation destination,
                                        StackLocation value,
                                        uint offset,
                                        uint size)
{
    // The pointer must be a 32 bit register.
    CHECK(isRegister32(destination));
    // The pointer must be one of the following: EAX, EBX, ECX, EDX, ESI, EDI
    // All of this registers can also be used with disp8/disp32
    // EBP can be used only with disp8/disp32
    int destGPEncoding = getGPEncoding(destination.u.reg);
    // No other form of allocation is possible, see constructor.
    ASSERT((destGPEncoding == ia32dis::IA32_GP32_EAX) ||
           (destGPEncoding == ia32dis::IA32_GP32_EBX) ||
           (destGPEncoding == ia32dis::IA32_GP32_ECX) ||
           (destGPEncoding == ia32dis::IA32_GP32_EDX) ||
           (destGPEncoding == ia32dis::IA32_GP32_ESI) ||
           (destGPEncoding == ia32dis::IA32_GP32_EDI));

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    if (size != 4)
    {
        if (!isRegister8(value))
        {
            if (destGPEncoding == ia32dis::IA32_GP32_EAX)
            {
                freeRegister32(ia32dis::IA32_GP32_EDX, compiler);
                compiler << "mov edx, " << getRegister32(value) << endl;
                value.u.reg = getGPEncoding(ia32dis::IA32_GP32_EDX);
            } else
            {
                freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
                compiler << "mov eax, " << getRegister32(value) << endl;
                value.u.reg = getGPEncoding(ia32dis::IA32_GP32_EAX);
            }
        }
    }

    compiler << "mov ";
    switch (size)
    {
    case 1: compiler << "byte ptr"; break;
    case 2: compiler << "word ptr"; break;
    case 4: compiler << "dword ptr"; break;
    // 8 bytes transaction aren't available in this processor
    default: CHECK_FAIL();
    }

    compiler << " [" << getRegister32(destination);
    if (offset != 0)
        compiler << " + " << cString(offset);
    compiler << "], ";

    switch (size)
    {
    case 1:
        compiler << getRegister8(value);
        break;
    case 2:
        compiler << getRegister16(value);
        break;
    case 4:
        compiler << getRegister32(value);
        break;
    }
    compiler << endl;
}

void IA32CompilerInterface::loadMemory(StackLocation destination,
                                       StackLocation value,
                                       uint offset,
                                       uint size,
                                       bool signExtend)
{
    // The pointer must be a 32 bit register.
    CHECK(isRegister32(destination));
    // The pointer must be one of the following: EAX, EBX, ECX, EDX, ESI, EDI
    // All of this registers can also be used with disp8/disp32
    // EBP can be used only with disp8/disp32
    #ifdef _DEBUG
    int destGPEncoding = getGPEncoding(destination.u.reg);
    #endif // _DEBUG
    // No other form of allocation is possible, see constructor.
    ASSERT((destGPEncoding == ia32dis::IA32_GP32_EAX) ||
           (destGPEncoding == ia32dis::IA32_GP32_EBX) ||
           (destGPEncoding == ia32dis::IA32_GP32_ECX) ||
           (destGPEncoding == ia32dis::IA32_GP32_EDX) ||
           (destGPEncoding == ia32dis::IA32_GP32_ESI) ||
           (destGPEncoding == ia32dis::IA32_GP32_EDI));

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    if (size == 4)
        compiler << "mov ";
    else if (signExtend)
    {
        compiler << "movsx ";
    } else
    {
        compiler << "movzx ";
    }

    compiler << getRegister32(value);

    /*
    switch (size)
    {
    case 1:
        compiler << getRegister8(value);
        break;
    case 2:
        compiler << getRegister16(value);
        break;
    case 4:
        compiler << getRegister32(value);
        break;
    }
    */

    switch (size)
    {
    case 1: compiler << ", byte ptr ["; break;
    case 2: compiler << ", word ptr ["; break;
    case 4: compiler << ", dword ptr ["; break;
    // 8 bytes transaction aren't available in this processor
    default: CHECK_FAIL();
    }

    compiler << getRegister32(destination);

    if (offset != 0)
        compiler << " + " << cString(offset);

    compiler << "]" << endl;
}

void IA32CompilerInterface::conv32(StackLocation destination,
                                   uint size,
                                   bool signExtend)
{
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    switch (size)
    {
    case 1:
        compiler << "and " << getRegister32(destination) << ", " << "0xFF";
        break;
    case 2:
        compiler << "and " << getRegister32(destination) << ", " << "0xFFFF";
        break;
    case 4:
        // Nothing to do
        break;
    default:
        CHECK_FAIL();
    }
    compiler << endl;
}

void IA32CompilerInterface::neg32(StackLocation destination)
{
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "neg " << getRegister32(destination) << endl;
}

void IA32CompilerInterface::not32(StackLocation destination)
{
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "not " << getRegister32(destination) << endl;
}

void IA32CompilerInterface::add32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "add " << getRegister32(destination) << ", " <<
                          getRegister32(source) << endl;
}

void IA32CompilerInterface::sub32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "sub " << getRegister32(destination) << ", " <<
                          getRegister32(source) << endl;
}

void IA32CompilerInterface::mul32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    bool saveSourceInEAX = false;

    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        if (getGPEncoding(source.u.reg) == ia32dis::IA32_GP32_EAX)
        {
            compiler << "push eax" << endl;
            saveSourceInEAX = true;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
            compiler << "mov eax, " << getRegister32(destination) << endl;
        }
    }

    // Check for saving result (edx)
    freeRegister32(ia32dis::IA32_GP32_EDX, compiler);
    compiler << "imul " << (saveSourceInEAX ? getRegister32(destination) : getRegister32(source)) << endl;

    // Remove the memory pointer if exsisted.
    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        compiler << "mov " << getRegister32(destination) << ", eax" << endl;
    }

    // Return EAX register
    if (saveSourceInEAX)
    {
        compiler << "pop eax" << endl;
    }
}

void IA32CompilerInterface::internalDiv32(StackLocation destination,
                                          StackLocation source,
                                          bool sign, bool isDiv)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    // TODO! Optimize this code

    // Check whether source is eax or edx and copy it into new register
    StackLocation newSource = source;
    bool saveSource = false;
    bool isEcxNotEdi = false; // Switch between ecx or edi
    if ((getGPEncoding(source.u.reg) == ia32dis::IA32_GP32_EAX) ||
        (getGPEncoding(source.u.reg) == ia32dis::IA32_GP32_EDX))
    {
        saveSource = true;

        if (getGPEncoding(destination.u.reg) == ia32dis::IA32_GP32_ECX)
        {
            // Use EDI
            newSource.u.reg = getGPEncoding(ia32dis::IA32_GP32_EDI);
            compiler << "push edi" << endl
                     << "mov  edi, " << getRegister32(source) << endl;
        }
        else
        {
            // Use ECX
            newSource.u.reg = getGPEncoding(ia32dis::IA32_GP32_ECX);
            isEcxNotEdi = true;
            compiler << "push ecx" << endl
                     << "mov  ecx, " << getRegister32(source) << endl;
        }

    }

    // Move destination into EAX:EDX
    bool saveEax = false;
    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        compiler << "push eax" << endl;
        compiler << "mov  eax, " << getRegister32(destination) << endl;
        saveEax = true;
    }

    // Reset EDX
    bool shouldSaveEdx = false;
    if ((m_binary->getCurrentStack()->getRegistersTable().hasKey(getGPEncoding(ia32dis::IA32_GP32_EDX))) &&
        (!m_binary->getCurrentStack()->isFreeTemporaryRegister(
                        StackInterface::buildStackLocation(
                                    getGPEncoding(ia32dis::IA32_GP32_EDX), 0))))
    {
        shouldSaveEdx = true;
        compiler << "push edx" << endl;
    }

    // And perform divide for unsigned and signed information
    if (sign)
    {
        // Signed
        compiler << "cdq" << endl;
        compiler << "idiv ";
    } else
    {
        // Unsigned
        compiler << "xor  edx, edx" << endl;
        compiler << "div ";
    }
    compiler << getRegister32(newSource) << endl;

    if (isDiv)
    {
        // Save the divide result
        if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
        {
            compiler << "mov " << getRegister32(destination) << ", eax" << endl;
        }
    } else
    {
        // Save the reminder result
        if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EDX)
        {
            compiler << "mov " << getRegister32(destination) << ", edx" << endl;
        }
    }

    // Restore properties
    if (shouldSaveEdx)
    {
        // Restore EDX
        compiler << "pop edx" << endl;
    }

    if (saveEax)
        compiler << "pop eax" << endl; // Restore previous eax

    // Restore previous settings
    if (saveSource)
    {
        compiler << "mov " << getRegister32(source) << ", " << getRegister32(newSource) << endl;
        compiler << "pop " << getRegister32(newSource) << endl;
    }
}

void IA32CompilerInterface::div32(StackLocation destination,
                                  StackLocation source,
                                  bool sign)
{
    internalDiv32(destination, source, sign, true);
}

void IA32CompilerInterface::rem32(StackLocation destination,
                                  StackLocation source,
                                  bool sign)
{
    internalDiv32(destination, source, sign, false);
}

void IA32CompilerInterface::and32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "and " << getRegister32(destination) << ", " <<
                          getRegister32(source) << endl;
}

void IA32CompilerInterface::xor32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "xor " << getRegister32(destination) << ", " <<
                          getRegister32(source) << endl;
}

void IA32CompilerInterface::shr32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    bool shouldMovDestinationEcx = false;

    if (source.u.reg != getGPEncoding(ia32dis::IA32_GP32_ECX))
    {
        if (destination.u.reg == getGPEncoding(ia32dis::IA32_GP32_ECX))
        {
            // ECX is the destination (should be the rotator)
            shouldMovDestinationEcx = true;
            compiler << "xchg " << getRegister32(source) << ", " << getRegister32(destination) << endl;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_ECX, compiler);
            compiler << "mov ecx, " << getRegister32(source) << endl;
        }
    }

    if (shouldMovDestinationEcx)
    {
        compiler << "shr " << getRegister32(source) << ", cl" << endl;
        compiler << "xchg " << getRegister32(source) << ", " << getRegister32(destination) << endl;
    } else
    {
        compiler << "shr " << getRegister32(destination) << ", cl" << endl;
    }
}

void IA32CompilerInterface::shl32(StackLocation destination,
                                  StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    bool shouldMovDestinationEcx = false;
    if (source.u.reg != getGPEncoding(ia32dis::IA32_GP32_ECX))
    {
        if (destination.u.reg == getGPEncoding(ia32dis::IA32_GP32_ECX))
        {
            // ECX is the destination (should be the rotator)
            shouldMovDestinationEcx = true;
            compiler << "xchg " << getRegister32(source) << ", " << getRegister32(destination) << endl;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_ECX, compiler);
            compiler << "mov ecx, " << getRegister32(source) << endl;
        }
    }

    if (shouldMovDestinationEcx)
    {
        compiler << "shl " << getRegister32(source) << ", cl" << endl;
        compiler << "xchg " << getRegister32(source) << ", " << getRegister32(destination) << endl;
    } else
    {
        compiler << "shl " << getRegister32(destination) << ", cl" << endl;
    }
}

void IA32CompilerInterface::or32(StackLocation destination,
                                 StackLocation source)
{
    // Validate register
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "or " << getRegister32(destination) << ", " <<
                         getRegister32(source) << endl;
}

void IA32CompilerInterface::adc32 (StackLocation destination, StackLocation source)
{
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "adc " << getRegister32(destination) << ", " <<
                         getRegister32(source) << endl;
}

void IA32CompilerInterface::sbb32 (StackLocation destination, StackLocation source)
{
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "sbb " << getRegister32(destination) << ", " <<
                          getRegister32(source) << endl;
}

void IA32CompilerInterface::mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign)
{
    // Validate register
    /*
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    bool saveSourceInEAX = false;

    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        if (getGPEncoding(source.u.reg) == ia32dis::IA32_GP32_EAX)
        {
            compiler << "push eax" << endl;
            saveSourceInEAX = true;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
            compiler << "mov eax, " << getRegister32(destination) << endl;
        }
    }

    // Check for saving result (edx)
    freeRegister32(ia32dis::IA32_GP32_EDX, compiler);
    compiler << "imul " << (saveSourceInEAX ? getRegister32(destination) : getRegister32(source)) << endl;

    // Remove the memory pointer if exsisted.
    if (getGPEncoding(destination.u.reg) != ia32dis::IA32_GP32_EAX)
    {
        compiler << "mov " << getRegister32(destination) << ", eax" << endl;
    }

    // Return EAX register
    if (saveSourceInEAX)
    {
        compiler << "pop eax" << endl;
    }
    */
}

void IA32CompilerInterface::addConst32(const StackLocation destination,
                                       int32 value)
{
    if (value == 0)
        return;
    // Validate register
    CHECK(isRegister32(destination));
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "add " << getRegister32(destination) << ", 0x" <<
                          HEXDWORD(value) << endl;
}

void IA32CompilerInterface::jump(int blockID)
{
    // Compile the instruction
    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "jmp $+66600666" << endl;
    }
    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(
                    MangledNames::getMangleBlock(blockID,
                                            getStackSize(),
                                            BinaryDependencies::DEP_RELATIVE),
                    m_binary->getCurrentBlockData().getSize() - getStackSize(),
                    getStackSize(),
                    BinaryDependencies::DEP_RELATIVE,
                    0,
                    false,
                    -4);
}

void IA32CompilerInterface::jumpShort(int blockID)
{
    // Compile the instruction
    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        compiler << "jmp $-1" << endl;
    }
    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(
                    MangledNames::getMangleBlock(blockID,
                                            BinaryDependencies::DEP_8BIT,
                                            BinaryDependencies::DEP_RELATIVE),
                    m_binary->getCurrentBlockData().getSize()-1,
                    BinaryDependencies::DEP_8BIT,
                    BinaryDependencies::DEP_RELATIVE,
                    0,
                    true);
}

void IA32CompilerInterface::jumpCond(StackLocation compare,
                                     int blockID, bool isZero)
{
    // Validate register
    CHECK(isRegister32(compare));
    // Compile the instructions
    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        // I don't know of a better way
        compiler << "cmp " << getRegister32(compare) << ", 0" << endl;
        if (isZero)
            compiler << "jz $+66600666" << endl;
        else
            compiler << "jnz $+66600666" << endl;
    }

    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(
                    MangledNames::getMangleBlock(blockID, getStackSize(), BinaryDependencies::DEP_RELATIVE),
                    m_binary->getCurrentBlockData().getSize() - getStackSize(),
                    getStackSize(),
                    BinaryDependencies::DEP_RELATIVE,
                    0,
                    false,
                    -4);
}

void IA32CompilerInterface::jumpCondShort(StackLocation compare,
                                          int blockID, bool isZero)
{
    // Validate register
    CHECK(isRegister32(compare));
    // Compile the instructions
    if (true)
    {
        cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
        cStringerStream& compiler = *ia32compiler;
        // I don't know of a better way
        compiler << "cmp " << getRegister32(compare) << ", 0" << endl;
        if (isZero)
            compiler << "jz $-1" << endl;
        else
            compiler << "jnz $-1" << endl;
    }
    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(
                    MangledNames::getMangleBlock(blockID,
                                            BinaryDependencies::DEP_8BIT,
                                            BinaryDependencies::DEP_RELATIVE),
                    m_binary->getCurrentBlockData().getSize()-1,
                    BinaryDependencies::DEP_8BIT,
                    BinaryDependencies::DEP_RELATIVE,
                    0,
                    true);
}

void IA32CompilerInterface::ceq32(StackLocation destination,
                                  StackLocation source)
{
    // Validate registers
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "cmp " << getRegister32(destination) << ", "
             << getRegister32(source) << endl;

    bool shouldRestoreSourceEAX = false;
    if (!isRegister8(destination))
    {
        if (source.u.reg == getGPEncoding(ia32dis::IA32_GP32_EAX))
        {
            shouldRestoreSourceEAX = true;
            compiler << "push eax" << endl;
        } else
        {

            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        }
        compiler << "setz al" << endl;
        compiler << "movsx " << getRegister32(destination) << ", al" << endl;
        if (shouldRestoreSourceEAX)
        {
            compiler << "pop eax" << endl;
        }
    } else
    {
        compiler << "setz " << getRegister8(destination) << endl;
        compiler << "movsx " << getRegister32(destination) << ", "
                 << getRegister8(destination) << endl;
    }

}

void IA32CompilerInterface::cgt32(StackLocation destination,
                                  StackLocation source, bool isSigned)
{
    // Validate registers
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "cmp " << getRegister32(destination) << ", "
             << getRegister32(source) << endl;
    bool shouldRestoreSourceEAX = false;
    if (!isRegister8(destination))
    {
        // TODO! Handle this case
        if (source.u.reg == getGPEncoding(ia32dis::IA32_GP32_EAX))
        {
            shouldRestoreSourceEAX = true;
            compiler << "push eax" << endl;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        }
    }

    if (isSigned)
    {
        // Signed
        compiler << "setg ";
    } else
    {
        // Unsigned
        compiler << "seta ";
    }

    if (!isRegister8(destination))
    {
        compiler << "al" << endl;
        compiler << "movsx " << getRegister32(destination) << ", al" << endl;
    } else
    {
        compiler << getRegister8(destination) << endl;
        compiler << "movsx " << getRegister32(destination) << ", "
                 << getRegister8(destination) << endl;
        if (shouldRestoreSourceEAX)
        {
            compiler << "pop eax" << endl;
        }
    }
}

void IA32CompilerInterface::clt32(StackLocation destination,
                                  StackLocation source, bool isSigned)
{
    // Validate registers
    CHECK(isRegister32(destination));
    CHECK(isRegister32(source));

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    compiler << "cmp " << getRegister32(destination) << ", "
             << getRegister32(source) << endl;

    bool shouldRestoreSourceEAX = false;
    if (!isRegister8(destination))
    {
        if (source.u.reg == getGPEncoding(ia32dis::IA32_GP32_EAX))
        {
            shouldRestoreSourceEAX = true;
            compiler << "push eax" << endl;
        } else
        {
            freeRegister32(ia32dis::IA32_GP32_EAX, compiler);
        }
    }

    if (isSigned)
    {
        // Signed
        compiler << "setl ";
    } else
    {
        // Unsigned
        compiler << "setb ";
    }

    if (!isRegister8(destination))
    {
        compiler << "al" << endl;
        compiler << "movsx " << getRegister32(destination) << ", al" << endl;
        if (shouldRestoreSourceEAX)
        {
            compiler << "pop eax" << endl;
        }
    } else
    {
        compiler << getRegister8(destination) << endl;
        compiler << "movsx " << getRegister32(destination) << ", "
                 << getRegister8(destination) << endl;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
//     Helper routines.

void IA32CompilerInterface::freeRegister32(int gpreg, cStringerStream& compiler)
{
    StackLocation registerLocation = StackInterface::buildStackLocation(
                                                getGPEncoding(gpreg), 0);
    ASSERT(isRegister32(registerLocation));

    if((m_binary->getCurrentStack()->getRegistersTable().hasKey(getGPEncoding(gpreg))) &&
       (!m_binary->getCurrentStack()->isFreeTemporaryRegister(registerLocation)))
    {
        // Allocate a stack based register
        StackLocation stackObject =
                m_binary->getCurrentStack()->replaceRegisterToStackVariable(
                                                              registerLocation);
        ASSERT(!isRegister32(stackObject));

        compiler << "mov " << getTempStack32(stackObject.u.reg, 4) << ", "
                 << getRegister32(registerLocation) << endl;

        // The register is now free
    }
}

bool IA32CompilerInterface::isRegister32(StackLocation destination)
{
    if (destination.u.reg >= 0) return false;
    if (getGPEncoding(destination.u.reg) <= ia32dis::IA32_GP32_EDI) return true;
    return false;
}

bool IA32CompilerInterface::isRegister8(StackLocation destination)
{
    uint reg = getGPEncoding(destination.u.reg);
    if ((reg >= ia32dis::IA32_GP32_EAX) &&
        (reg <= ia32dis::IA32_GP32_EBX))
        return true;
    return false;
}

const char* IA32CompilerInterface::getBaseStackRegister(StackLocation baseRegister)
{
    if (baseRegister == StackInterface::NO_MEMORY)
        return "ebp";
    ASSERT(baseRegister.u.flags == 0);
    return getRegister32(baseRegister);
}

const char* IA32CompilerInterface::getRegister32(StackLocation destination)
{
    ASSERT(isRegister32(destination));
    return ia32dis::gIa32Registers[getGPEncoding(destination.u.reg)].m_name;
}

const char* IA32CompilerInterface::getRegister16(StackLocation destination)
{
    ASSERT(isRegister32(destination));

    return ia32dis::gIa16Registers[getGPEncoding(destination.u.reg)].m_name;
}

const char* IA32CompilerInterface::getRegister8(StackLocation destination)
{
    ASSERT(isRegister32(destination));
    uint reg = getGPEncoding(destination.u.reg);
    if (reg == ia32dis::IA32_GP32_EAX)
        return ia32dis::gIa8Registers[ia32dis::IA32_GP8_AL].m_name;
    if (reg == ia32dis::IA32_GP32_EBX)
        return ia32dis::gIa8Registers[ia32dis::IA32_GP8_BL].m_name;
    if (reg == ia32dis::IA32_GP32_ECX)
        return ia32dis::gIa8Registers[ia32dis::IA32_GP8_CL].m_name;
    if (reg == ia32dis::IA32_GP32_EDX)
        return ia32dis::gIa8Registers[ia32dis::IA32_GP8_DL].m_name;

    CHECK_FAIL();
}

cString IA32CompilerInterface::getStackReference124(uint stackPosition,
                                                    uint size,
                                                    bool argumentStackLocation)
{
    cString ret;
    switch (size)
    {
    case 1: ret = g_byteptrOnly; break;  // 8 bit
    case 2: ret = g_wordptrOnly; break;  // 16 bit
    case 4: ret = g_dwordptrOnly; break; // 32 bit
    default:
        CHECK_FAIL();
    }

    ret += g_open;
    ret += getBaseStackRegister(getMethodBaseStackRegister());

    // See also load32addr
    if (!argumentStackLocation)
        ret+= " - ";
    else
    {
        // Add the EIP
        stackPosition+= 4;
        ret+= " + ";
    }

    ret+= cString(stackPosition + 4);
    ret+= g_terminate;
    return ret;
}

cString IA32CompilerInterface::getTempStack32(uint stackPosition, uint size)
{
    cString ret;
    switch (size)
    {
    case 1: ret = g_byteptrOnly; break;  // 8 bit
    case 2: ret = g_wordptrOnly; break;  // 16 bit
    case 4: ret = g_dwordptrOnly; break; // 32 bit
    default:
        CHECK_FAIL();
    }

    ret += g_open;
    ret += getBaseStackRegister(getMethodBaseStackRegister());
    ret+= " - ";
    ret+= cString(stackPosition + 4 + m_binary->getStackBaseSize()
                  - StackInterface::LOCAL_STACK_START_VALUE);
    ret+= g_terminate;
    return ret;
}

void IA32CompilerInterface::saveNonVolatileRegisters(cStringerStream& compiler,
                                                     RegToLocation& locationMap,
                                                     bool bSave, bool bAlways)
{
    // Start with the maximal temp stack size of all blocks
    uint pos = m_binary->getStackSize();

    const RegisterAllocationTable& touched = m_binary->getTouchedRegisters();

    // Examine all known registers
    cList<int> regs = m_archRegisters.keys();
    for (cList<int>::iterator i = regs.begin(); i != regs.end(); i++)
    {
        int reg = *i;
        // Is this register non-volatile?
        if (m_archRegisters[reg].m_eType != NonVolatile)
            continue;

        // Does it need saving/restoring
        if (!bAlways && !touched.hasKey(reg))
            continue;

        StackLocation registerLocation = StackInterface::buildStackLocation(reg, 0);
        ASSERT(isRegister32(registerLocation));

        if (bSave)
        {
            // Allocate a location for it on the stack (not temporary!)
            StackLocation location;
            location.u.flags = STACK_LOCATION_FLAGS_LOCAL;
            location.u.reg = pos - m_binary->getStackBaseSize() + StackInterface::LOCAL_STACK_START_VALUE;

            locationMap.append(reg, location);

            ASSERT(!isRegister32(locationMap[reg]));
            // Push the register - this essentially allocates the position on the stack
            compiler << "push " << getRegister32(registerLocation) << endl;
        }
        else
        {
            // Restore from the stack to the register
            compiler << "mov " << getRegister32(registerLocation) << ", "
                        << getTempStack32(locationMap[reg].u.reg, 4) << endl;
        }
        pos += getStackSize();
    }
}

void IA32CompilerInterface::saveVolatileRegisters(int saveRegister1,
                                                  int saveRegister2,
                                                  int saveRegister3)
{
    // Examine all known registers
    cList<int> regs = m_archRegisters.keys();
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    for (cList<int>::iterator i = regs.begin(); i != regs.end(); i++)
    {
        // Is this register volatile?
        if (m_archRegisters[*i].m_eType != Volatile)
            continue;

        // Do we skip this register?
        if ((*i == saveRegister1) || (*i == saveRegister2) || (*i == saveRegister3))
            continue;

        // If it's used, replace it with a stack temporary
        freeRegister32(getGPEncoding(*i), compiler);
    }
}

void IA32CompilerInterface::revertStack(uint32 size)
{
    if (size == 0)
        return;

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;
    compiler << "add esp, 0x" << HEXDWORD(size) << endl;
}

void IA32CompilerInterface::setFramePointer(StackLocation destination)
{
    if (destination == getMethodBaseStackRegister())
        return;

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;
    compiler << "mov " << getRegister32(destination) << ", " << getRegister32(getMethodBaseStackRegister()) << endl;
}

void IA32CompilerInterface::resetBaseStackRegister(const StackLocation& targetRegister)
{
    // Nothing to do if it's already the default register
    if (getMethodBaseStackRegister() == targetRegister)
        return;

    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;

    // Set EBP (or otherwise the target's base register) to its real value
    compiler << "mov " << getRegister32(targetRegister) << ", " << getRegister32(getMethodBaseStackRegister()) << endl;

    // And forget that we ever used a different register
    setMethodBaseStackRegister(getStackPointer());
}

cBufferPtr IA32CompilerInterface::getAlignBuffer() const
{
    static cBuffer nops((const uint8*)"\x90\x90\x90\x90", 4);
    return cBufferPtr(&nops, SMARTPTR_DESTRUCT_NONE);
}

RegisterAllocationInfo IA32CompilerInterface::getOperationRegisterAllocationInfo(CompilerInterface::CompilerOperation& operation)
{
    RegisterAllocationInfo registerAllocationInfo(getNumberOfRegisters());

    switch(operation.opcode) {
        case CompilerInterface::OPCODE_STORE_CONST:
        {
            break;
        }
        case CompilerInterface::OPCODE_STORE_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_STORE_32_DEPENDENCY:
        {
            registerAllocationInfo.m_acceptableSource.resetArray();
            registerAllocationInfo.m_acceptableSource.set(EAX);
            break;
        }
        case CompilerInterface::OPCODE_MOVE_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_32_DEPENDENCY:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_ADDR_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_ADDR_32_DEPENDENCY:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_INT_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_INT_32_DEPENDENCY:
        {
            break;
        }
        case CompilerInterface::OPCODE_ASSIGN_RET_32:
        {
            registerAllocationInfo.m_acceptableSource.resetArray();
            registerAllocationInfo.m_acceptableSource.set(EAX);
            break;
        }

        case CompilerInterface::OPCODE_PUSH_ARG_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_POP_ARG_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_CALL:
        {
            // Volatile registers should be saved before call
            registerAllocationInfo.m_modifiable.set(EAX);
            registerAllocationInfo.m_modifiable.set(EBX);
            registerAllocationInfo.m_modifiable.set(EDX);
            registerAllocationInfo.m_modifiable.set(EDI);
            break;
        }
        case CompilerInterface::OPCODE_CALL_DEPENDENCY:
        {
            // Volatile registers should be saved before call
            registerAllocationInfo.m_modifiable.set(EAX);
            registerAllocationInfo.m_modifiable.set(EBX);
            registerAllocationInfo.m_modifiable.set(EDX);
            registerAllocationInfo.m_modifiable.set(EDI);
            break;
        }
        case CompilerInterface::OPCODE_CALL_32:
        {
            registerAllocationInfo.m_acceptableDest.resetArray();
            registerAllocationInfo.m_acceptableDest.set(EAX);
            // Volatile registers should be saved before call
            registerAllocationInfo.m_modifiable.set(EBX);
            registerAllocationInfo.m_modifiable.set(EDX);
            registerAllocationInfo.m_modifiable.set(EDI);
            break;
        }
        case CompilerInterface::OPCODE_CALL_32_DEPENDENCY:
        {
            registerAllocationInfo.m_acceptableDest.resetArray();
            registerAllocationInfo.m_acceptableDest.set(EAX);
            // Volatile registers should be saved before call
            registerAllocationInfo.m_modifiable.set(EBX);
            registerAllocationInfo.m_modifiable.set(EDX);
            registerAllocationInfo.m_modifiable.set(EDI);
            break;
        }
        case CompilerInterface::OPCODE_STORE_MEMORY:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOAD_MEMORY:
        {
            break;
        }
        case CompilerInterface::OPCODE_CONV_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_NEG_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_NOT_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_ADD_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_SUB_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_MUL_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            registerAllocationInfo.m_acceptableDest.resetArray();
            registerAllocationInfo.m_acceptableDest.set(EAX);
            registerAllocationInfo.m_acceptableSource.clear(EDX);
            registerAllocationInfo.m_modifiable.set(EDX);
            break;
        }
        case CompilerInterface::OPCODE_DIV_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            registerAllocationInfo.m_acceptableSource.clear(EAX);
            registerAllocationInfo.m_acceptableSource.clear(EDX);
            registerAllocationInfo.m_acceptableDest.resetArray();
            registerAllocationInfo.m_acceptableDest.set(EAX);
            registerAllocationInfo.m_modifiable.set(EDX);
            break;
        }
        case CompilerInterface::OPCODE_REM_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            registerAllocationInfo.m_acceptableSource.clear(EAX);
            registerAllocationInfo.m_acceptableSource.clear(EDX);
            registerAllocationInfo.m_acceptableDest.resetArray();
            registerAllocationInfo.m_acceptableDest.set(EDX);
            registerAllocationInfo.m_modifiable.set(EAX);
            break;
        }
        case CompilerInterface::OPCODE_AND_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_XOR_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_SHR_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            registerAllocationInfo.m_acceptableSource.resetArray();
            registerAllocationInfo.m_acceptableSource.set(ECX);
            break;
        }
        case CompilerInterface::OPCODE_SHL_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            registerAllocationInfo.m_acceptableSource.resetArray();
            registerAllocationInfo.m_acceptableSource.set(ECX);
            break;
        }
        case CompilerInterface::OPCODE_OR_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_ADC_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_SBB_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_MUL_32H:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_ADD_CONST_32:
        {
            registerAllocationInfo.m_isDestAlsoSource = true;
            break;
        }
        case CompilerInterface::OPCODE_JUMP:
        {
            break;
        }
        case CompilerInterface::OPCODE_JUMP_SHORT:
        {
            break;
        }
        case CompilerInterface::OPCODE_JUMP_COND:
        {
            break;
        }
        case CompilerInterface::OPCODE_JUMP_COND_SHORT:
        {
            break;
        }
        case CompilerInterface::OPCODE_CEQ_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_CGT_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_CLT_32:
        {
            break;
        }
        case CompilerInterface::OPCODE_LOCALLOC:
        {
            break;
        }
        case CompilerInterface::OPCODE_REVERT_STACK:
        {
            break;
        }
        case CompilerInterface::OPCODE_RESET_BASE_STACK_REGISTER:
        {
            break;
        }
        case CompilerInterface::OPCODE_SET_FRAME_POINTER:
        {
            break;
        }
    }

    return registerAllocationInfo;
}
