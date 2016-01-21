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
 * c32CCompilerInterface.cpp
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
#include "compiler/processors/c/32C.h"
#include "compiler/CallingConvention.h"

c32CCompilerInterface::cCFirstBinaryStream::cCFirstBinaryStream(FirstPassBinaryPtr& binary) :
    m_binary(*binary)
{
}

void c32CCompilerInterface::cCFirstBinaryStream::outputString(const cString& string)
{
    cSArray<char> a = string.getASCIIstring();
    m_binary.appendBuffer((const uint8*)a.getBuffer(), a.getSize() - 1); // Remove 0 terminate char
}

c32CCompilerInterface::c32CCompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params) :
    CompilerInterface(framework, params),
    m_currentArgs(0),
    m_maximumNumberOfArgs(0)
{
    // Generate new first-binary pass
    for (int i = 1; i <= NUMBER_OF_REGISTER; i++)
        m_archRegisters.append(-i, RegisterEntry(NonVolatile));
    //setBaseStackRegister(getStackPointer());

    m_binary = FirstPassBinaryPtr(new FirstPassBinary(
                                       OpcodeSubsystems::DISASSEMBLER_STRING,
                                       true));

    CHECK_MSG(!m_parameters.m_bSupportExceptionHandling, "32C compiler needs exception handling disabled");
}

void c32CCompilerInterface::setLocalsSize(uint localStackSize)
{
    m_binary->setStackBaseSize(localStackSize);
}

void c32CCompilerInterface::setArgumentsSize(uint argsSize)
{
    m_binary->setArgumentsSize(argsSize);
}

c32CCompilerInterface::StackSize c32CCompilerInterface::getStackSize() const
{
    return STACK_32;
}

uint c32CCompilerInterface::getShortJumpLength() const
{
    return 0x1000;
}

StackLocation c32CCompilerInterface::getStackPointer() const
{
    return StackInterface::buildStackLocation(REGISTER_BASE_POINTER, 0);
}

void c32CCompilerInterface::localloc(StackLocation destination,
                                     StackLocation size,
                                     bool isStackEmpty)
{
    CHECK_FAIL();
}

void c32CCompilerInterface::generateMethodEpiProLogs(bool bForceSaveNonVolatiles /* = false */)
{
    // Check for allocated registers, and debug!

    // Add prolog
    // Generate new first stream, and switch to it.

    MethodBlock* prolog = new MethodBlock(MethodBlock::BLOCK_PROLOG, *this);
    StackInterfacePtr estack(prolog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_PROLOG, estack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_PROLOG);
    if (true)
    {
        cCFirstBinaryStream compiler(m_binary);
        // Arguments list
        compiler << "(";
        uint32 argCount = (uint32)(m_binary->getArgumentsSize() + 3) / 4;
        uint32 j;
        for (j = 0; j < argCount; j++)
        {
            compiler << "int a" << j;
            if (j < (argCount - 1))
                compiler << ", ";
        }

        compiler << ")" << endl << "{" << endl;
        if (m_binary->getStackSize() > 0)
        {
            compiler << "unsigned char locals[0x" << HEXDWORD(m_binary->getStackSize() + 4) << "];"<< endl;
            compiler << "unsigned char* rbp = locals;" << endl;
        }
        compiler << "int ret = 0;" << endl;

        // Test for non-volatile registers touching
        const RegisterAllocationTable& touched = m_binary->getTouchedRegisters();
        cList<int> t = touched.keys();
        cList<int>::iterator i = t.begin();
        // Bug with the touch registers for return arguments
        if (i == t.end())
        {
            compiler << "int r" << cString(NUMBER_OF_REGISTER + 1) << ";";
        }

        for (;i != t.end(); ++i)
        {
            compiler << "int r" << -(*i-1) << ";";
        }
        for (j = 0; j < m_maximumNumberOfArgs; j++)
        {
            compiler << "int ap" << j << ";";
        }
        compiler << endl;
    }
    prolog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);

    // Conclude the method by adding a epilog
    // Generate last stream, and switch to it.
    MethodBlock* epilog = new MethodBlock(MethodBlock::BLOCK_RET, *this);
    StackInterfacePtr pstack(epilog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_RET, pstack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_RET);
    if (true)
    {
        // Add return value
        cCFirstBinaryStream compiler(m_binary);
        compiler << "return ret;" << endl;
        compiler << "}" << endl;
    }
    epilog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

void c32CCompilerInterface::storeConst(uint stackPosition,
                                       const uint8* bufferOffset,
                                       uint  size,
                                       bool  argumentStackLocation)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getStackReference124(stackPosition, size,
                                     argumentStackLocation) << " = 0x";
    switch (size)
    {
    case 1: // 8 bit
        compiler << HEXBYTE(*bufferOffset);
        break;
    case 2: // 16 bit
        compiler << HEXWORD(*((const uint16*)bufferOffset));
        break;
    case 4: // 32 bit
        compiler << HEXDWORD(*((const uint32*)bufferOffset));
        break;
    }

    compiler << ";" << endl;
}

void c32CCompilerInterface::store32(uint stackPosition,
                                    uint size,
                                    StackLocation source,
                                    bool argumentStackLocation,
                                    bool isTempStack)
{
    cCFirstBinaryStream compiler(m_binary);

    if (isTempStack)
    {
        // Temp stack
        compiler << getTempStack32(stackPosition);
    } else
    {
        compiler << getStackReference124(stackPosition, size,
                                         argumentStackLocation);
    }

    compiler << " = " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::store32(StackLocation source,
                                    uint size,
                                    bool signExtend,
                                    const cString& dependencyName)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << "*(" << getSizeCast(size, true) << "*)((unsigned char*)" << getTokenName(dependencyName) << ") = " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::move32(StackLocation destination,
                                    StackLocation source,
                                  uint size,
                                  bool signExtend)
{
}

void c32CCompilerInterface::load32(uint stackPosition,
                                   uint size,
                                   StackLocation destination,
                                   bool signExtend,
                                   bool argumentStackLocation,
                                   bool isTempStack)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = ";
    if (isTempStack)
    {
        // Temp stack
        compiler << getTempStack32(stackPosition);
    } else
    {
        compiler << getStackReference124(stackPosition, size,
                                         argumentStackLocation);
    }
    compiler << ";" << endl;
}

void c32CCompilerInterface::load32(StackLocation destination,
                                   uint size,
                                   bool signExtend,
                                   const cString& dependencyName)
{
    // Validate register
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << " = *(" << getSizeCast(size, true) << ")((unsigned char*)" << getTokenName(dependencyName) << ");" << endl;

    /*
    m_binary->getCurrentDependecies().addDependency(
                dependencyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_ABSOLUTE,
                0,
                false,
                -4
                );
    */
}

void c32CCompilerInterface::load32addr(uint stackPosition,
                                       uint size,
                                       uint offset,
                                       StackLocation destination,
                                       bool argumentStackLocation,
                                       bool isTempStack)
{
    cCFirstBinaryStream compiler(m_binary);

    // Check arguments
    if (argumentStackLocation)
    {
        compiler << getRegsiterName(destination) << " = (int)(&a" << (uint32)(stackPosition/4) << ");" << endl;
    } else
    {
        // TODO! << baseRegister;
        if(isTempStack)
        {
            compiler << getRegsiterName(destination) << " = (int)(" << getTempStack32(stackPosition, false) << ");" << endl;
        }
        else
        {
            compiler << getRegsiterName(destination) << " = (int)(" << getStackReference124(stackPosition,size,argumentStackLocation, false) << ");" << endl;
        }
    }
}

void c32CCompilerInterface::load32addr(StackLocation destination,
                                       const cString& dependencyName)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << " = (int)(&" << getTokenName(dependencyName) << ");" << endl;
}

void c32CCompilerInterface::loadInt32(StackLocation destination,
                                      uint32 value)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = " << value << ";" << endl;
}

void c32CCompilerInterface::loadInt32(StackLocation destination,
                                      const cString& dependancyName)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = " << getTokenName(dependancyName) << ";" << endl;

/*
    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(
                dependancyName,
                m_binary->getCurrentBlockData().getSize() - getStackSize(),
                getStackSize(),
                BinaryDependencies::DEP_ABSOLUTE,
                0,
                true);
 */
}

void c32CCompilerInterface::assignRet32(StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << "ret = " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::pushArg32(StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    // Use a local int, named ap#
    compiler << "ap" << (uint32)m_currentArgs << " = " << getRegsiterName(source) << ";" << endl;
    // And remember that we need to use it
    m_currentArgs++;
    if (m_maximumNumberOfArgs < m_currentArgs)
        m_maximumNumberOfArgs = m_currentArgs;
}

void c32CCompilerInterface::popArg32(StackLocation source)
{
    // Never pop anything in C32
    CHECK_FAIL();
}

void c32CCompilerInterface::callArgs(cCFirstBinaryStream& compiler, uint numberOfArguments, bool isPrototype)
{
    for (uint i = 0; i < numberOfArguments; i++)
    {
        if (!isPrototype)
            compiler << "ap" << (uint32)(m_currentArgs - i - 1);
        else
            compiler << "int";
        if (i != numberOfArguments - 1)
            compiler << ", ";
    }
    if (!isPrototype)
        m_currentArgs-= numberOfArguments;
}

void c32CCompilerInterface::call(const cString& dependancyName, uint numberOfArguments)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getTokenName(dependancyName) << "(";
    callArgs(compiler, numberOfArguments);
    compiler << ");" << endl;

    // Add dependency to the last 4 bytes
    m_binary->getCurrentDependecies().addDependency(dependancyName,m_binary->getCurrentBlockData().getSize() - getStackSize(), getStackSize(),BinaryDependencies::DEP_RELATIVE);
}

void c32CCompilerInterface::call(StackLocation address, uint numberOfArguments)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << "((int (*)(";
    callArgs(compiler, numberOfArguments, true);
    compiler << ")) (" << getRegsiterName(address) << "))(";
    callArgs(compiler, numberOfArguments);
    compiler << ");" << endl;
}

void c32CCompilerInterface::call32(const cString& dependancyName,
                                   StackLocation destination, uint numberOfArguments)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = " << getTokenName(dependancyName) << "(";
    callArgs(compiler, numberOfArguments);
    compiler << ");" << endl;
    m_binary->getCurrentDependecies().addDependency(dependancyName,m_binary->getCurrentBlockData().getSize() - getStackSize(), getStackSize(),BinaryDependencies::DEP_RELATIVE);
}

void c32CCompilerInterface::call32(StackLocation address,
                                   StackLocation destination, uint numberOfArguments)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = ((int (*)(";
    callArgs(compiler, numberOfArguments, true);
    compiler << "))(" << getRegsiterName(address) << "))(";
    callArgs(compiler, numberOfArguments);
    compiler << ");" << endl;
}

void c32CCompilerInterface::storeMemory(StackLocation destination,
                                        StackLocation value,
                                        uint offset,
                                        uint size)
{
    cCFirstBinaryStream compiler(m_binary);
    //  *((size*)destination + offset) = (size)value;
    compiler << "*(" << getSizeCast(size, true) << ")((unsigned char*)" << getRegsiterName(destination) << " + " << (uint32)offset << ") = ("
             << getSizeCast(size, false) << ")" << getRegsiterName(value) << ";" << endl;
}

void c32CCompilerInterface::loadMemory(StackLocation destination,
                                       StackLocation value,
                                       uint offset,
                                       uint size,
                                       bool signExtend)
{
    cCFirstBinaryStream compiler(m_binary);
    //  *((size*)destination + offset) = (size)value;
    compiler << getRegsiterName(value) << " = (int)(*(" << getSizeCast(size, true) << ")((unsigned char*)" << getRegsiterName(destination) << " + "
             << (uint32)offset << "));" << endl;
}

void c32CCompilerInterface::conv32(StackLocation destination,
                                   uint size,
                                   bool signExtend)
{
    cCFirstBinaryStream compiler(m_binary);

    switch (size)
    {
    case 1:
        compiler << getRegsiterName(destination) << "&= 0xFF;" << endl;
        break;
    case 2:
        compiler << getRegsiterName(destination) << "&= 0xFFFF;" << endl;
        break;
    }
}

void c32CCompilerInterface::neg32(StackLocation destination)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = -" << getRegsiterName(destination) << ";" << endl;
}

void c32CCompilerInterface::not32(StackLocation destination)
{
    cCFirstBinaryStream compiler(m_binary);

    compiler << getRegsiterName(destination) << " = !" << getRegsiterName(destination) << ";" << endl;
}

void c32CCompilerInterface::add32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "+= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::sub32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "-= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::mul32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "*= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::div32(StackLocation destination,
                                  StackLocation source,
                                  bool sign)
{
    cCFirstBinaryStream compiler(m_binary);

    // TODO! Add sign
    compiler << getRegsiterName(destination) << "/= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::rem32(StackLocation destination,
                                  StackLocation source,
                                  bool sign)
{
    cCFirstBinaryStream compiler(m_binary);

    // TODO! Add sign
    compiler << getRegsiterName(destination) << "%= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::and32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "&= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::xor32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "^= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::shr32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << ">>= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::shl32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "<<= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::or32(StackLocation destination,
                                 StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "|= " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::adc32 (StackLocation destination, StackLocation source)
{
    CHECK_MSG(FALSE, "adc32 not supported yet");
}

void c32CCompilerInterface::sbb32 (StackLocation destination, StackLocation source)
{
    CHECK_MSG(FALSE, "sbb32 not supported yet");
}

void c32CCompilerInterface::mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign)
{
    CHECK_MSG(FALSE, "mul32h not supported yet");
}

void c32CCompilerInterface::addConst32(const StackLocation destination,
                                       int32 value)
{
    if (value == 0)
        return;
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << "+= " << value << ";" << endl;
}


void c32CCompilerInterface::jump(int blockID)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << "goto cblk" << blockID << ";" << endl;
}

void c32CCompilerInterface::jumpShort(int blockID)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << "goto cblk" << blockID << ";" << endl;
}

void c32CCompilerInterface::jumpCond(StackLocation compare,
                                     int blockID, bool isZero)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << "if (" << getRegsiterName(compare) << ((isZero) ? " == " : " != ") << "0) goto cblk" << blockID << ";" << endl;
}

void c32CCompilerInterface::jumpCondShort(StackLocation compare,
                                          int blockID, bool isZero)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << "if (" << getRegsiterName(compare) << ((isZero) ? " == " : " != ") << "0) goto cblk" << blockID << ";" << endl;
}

void c32CCompilerInterface::ceq32(StackLocation destination,
                                  StackLocation source)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << " = " << getRegsiterName(destination) << " == " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::cgt32(StackLocation destination,
                                  StackLocation source, bool isSigned)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << " = " << getRegsiterName(destination) << " > " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::clt32(StackLocation destination,
                                  StackLocation source, bool isSigned)
{
    cCFirstBinaryStream compiler(m_binary);
    compiler << getRegsiterName(destination) << " = " << getRegsiterName(destination) << " < " << getRegsiterName(source) << ";" << endl;
}

void c32CCompilerInterface::revertStack(uint32 size)
{
    /*
     * No need
     *
    cStringerStreamPtr ia32compiler = m_assembler->getAssembler();
    cStringerStream& compiler = *ia32compiler;
    compiler << "add esp, 0x" << HEXDWORD(size) << endl;
    */
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
//     Helper routines.


cString c32CCompilerInterface::getStackReference124(uint stackPosition,
                                                    uint size,
                                                    bool argumentStackLocation,
                                                    bool shouldReference)
{
    // TODO! REGISTER_BASE_POINTER
    cString ret;
    if (argumentStackLocation)
    {
        ret = "(";
        ret+= getSizeCast(size, false);
        ret+= ")a";
        ret+= cString(stackPosition / 4);
    } else
    {
        if (shouldReference)
            ret = "*";

        ret+="((";
        ret+= getSizeCast(size, true);
        ret+= ")(";
        if (getMethodBaseStackRegister() == StackInterface::NO_MEMORY)
            ret+= "rbp";
        else
        {
            ret+= getRegsiterName(getMethodBaseStackRegister());
        }
        ret+= " + ";
        ret+= cString(stackPosition);
        ret+= "))";
    }
    return ret;
}

cString c32CCompilerInterface::getTempStack32(uint stackPosition, bool derefAddr)
{
    cString ret("");
    if(derefAddr)
        ret += "*";
    ret+= "((";
    ret+= getSizeCast(4, derefAddr);
    ret+= ")(";
    ret+= "rbp + ";
    ret+= cString(stackPosition + m_binary->getStackBaseSize());
    ret+= "))";
    return ret;
}

cString c32CCompilerInterface::getRegsiterName(StackLocation reg)
{
    if (reg == getStackPointer())
    {
        return "rbp";
    } else
    {
        cString ret = "r";
        ret+= cString(-(reg.u.reg - 1));
        return ret;
    }
}

cString c32CCompilerInterface::getFunctionName(const TokenIndex& token)
{
    cString ret = "func";
    ret+= HEXDWORD(token.m_b);
    ret+= HEXDWORD(token.m_a);
    return ret;
}

cString c32CCompilerInterface::getGlobalsName()
{
    return "g32CGlobals";
}

cString c32CCompilerInterface::getSizeCast(uint size, bool ptr)
{
    cString ret;
    switch (size)
    {
    case 1:
        ret = "unsigned char";
        break;
    case 2:
        ret = "unsigned short";
        break;
    case 4:
        ret = "unsigned int";
        break;
    default:
        // 8 bytes transaction aren't available in this processor
        if (!ptr)
            CHECK_FAIL();
        // pointer to struct? nevermind what kind of pointer, as long as it is a pointer.
        return "unsigned char*";
    }
    if (ptr)
        ret += "*";
    return ret;
}

cString c32CCompilerInterface::getTokenName(const cString& dependencyName)
{
    TokenIndex t;
    if (::CallingConvention::deserializeMethod(dependencyName, t))
    {
        return getFunctionName(t);
    }


    uint offset;
    if (::CallingConvention::deserializeGlobalData(dependencyName, offset))
    {
        cString ret(getGlobalsName());
        ret+= " + 0x";
        ret+= HEXDWORD(offset);
        return ret;
    }

    if (::CallingConvention::deserializeToken(dependencyName, t))
    {
        cString ret;
        // Static members are called glblXXX, class vtables are called vtblXXX
        if (EncodingUtils::getTokenTableIndex(getTokenID(t)) == TABLE_FIELD_TABLE)
            ret += "glbl";
        else
            ret += "vtbl";

        ret+= HEXDWORD(t.m_b);
        ret+= HEXDWORD(t.m_a);
        m_binary->getCurrentDependecies().addDependency(
                    dependencyName,
                    m_binary->getCurrentBlockData().getSize() - getStackSize(),
                    getStackSize(),
                    BinaryDependencies::DEP_ABSOLUTE,
                    0,
                    true);

        return ret;
    }


    if (StringRepository::deserializeStringToken(dependencyName, t))
    {
        cString ret = "str";
        ret+= HEXDWORD(t.m_b);
        ret+= HEXDWORD(t.m_a);
        m_binary->getCurrentDependecies().addDependency(
                    dependencyName,
                    m_binary->getCurrentBlockData().getSize() - getStackSize(),
                    getStackSize(),
                    BinaryDependencies::DEP_ABSOLUTE,
                    0,
                    true);
        return ret;
    }

    CHECK_FAIL();
}

void c32CCompilerInterface::resetBaseStackRegister(const StackLocation& targetRegister)
{
    // Nothing to do if it's already the default register
    if (getMethodBaseStackRegister() == targetRegister)
        return;

    cCFirstBinaryStream compiler(m_binary);

    // Set the target's base register to its real value
    compiler << getRegsiterName(targetRegister) << " = " << getRegsiterName(getMethodBaseStackRegister()) << ";" << endl;

    // And forget that we ever used a different register
    setMethodBaseStackRegister(getStackPointer());
}
