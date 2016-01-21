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
 * OptimizerCompilerInterface.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "xStl/stream/ioStream.h"
#include "compiler/OptimizerCompilerInterface.h"
#include "compiler/CompilerTrace.h"
#include "compiler/TemporaryStackHolder.h"

OptimizerCompilerInterface::OptimizerCompilerInterface(OptimizerOperationCompilerInterfacePtr _interface):
    m_interface(_interface),
    CompilerInterface(_interface->m_framework, _interface->m_parameters),
    m_dummyRegistersMapping(0, _interface->getNumberOfRegisters())
{
    cList<int> registers(m_interface->getArchRegisters().keys());
    cList<int>::iterator registerIter(registers.begin());

    // The volatile dummy registers.
    for (int i = START_VOLATILE_REGISTERS; i > END_VOLATILE_REGISTERS ; --i)
    {
        m_archRegisters.append(i, RegisterEntry(Volatile));
    }
    // The non-volatile dummy registers.
    for (int i = START_NON_VOLATILE_REGISTERS; i > END_NON_VOLATILE_REGISTERS ; --i)
    {
        m_archRegisters.append(i, RegisterEntry(NonVolatile));
    }
}

void OptimizerCompilerInterface::setLocalsSize(uint localStackSize)
{
    m_interface->setLocalsSize(localStackSize);
}

void OptimizerCompilerInterface::setArgumentsSize(uint argsSize)
{
    m_interface->setArgumentsSize(argsSize);
}

CompilerInterface::StackSize OptimizerCompilerInterface::getStackSize() const
{
    return m_interface->getStackSize();
}

uint OptimizerCompilerInterface::getShortJumpLength() const
{
    return m_interface->getShortJumpLength();
}

const FirstPassBinaryPtr& OptimizerCompilerInterface::getFirstPassPtr() const
{
    return m_interface->getFirstPassPtr();
}

FirstPassBinaryPtr& OptimizerCompilerInterface::getFirstPassPtr()
{
    return m_interface->getFirstPassPtr();
}

const RegisterAllocationTable& OptimizerCompilerInterface::getArchRegisters() const
{
    return m_archRegisters;
}

StackLocation OptimizerCompilerInterface::getStackPointer() const
{
    /*if (!isOptimizerOn())
    {
        return m_interface->getStackPointer();
    }*/
    return StackInterface::buildStackLocation(INITIAL_BASE_STACK_REGISTER, 0);
}

void OptimizerCompilerInterface::setFramePointer(StackLocation destination)
{
    if (!isOptimizerOn()) {
        m_interface->setFramePointer(destination);
        return;
    }

        CompilerInterface::CompilerOperation opcode(
        OPCODE_SET_FRAME_POINTER,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::resetBaseStackRegister(const StackLocation& targetRegister)
{
    if (!isOptimizerOn()) {
        m_interface->resetBaseStackRegister(targetRegister);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_RESET_BASE_STACK_REGISTER,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        targetRegister,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

cBufferPtr OptimizerCompilerInterface::getAlignBuffer() const
{
    return m_interface->getAlignBuffer();
}

CompilerInterface::CallingConvention OptimizerCompilerInterface::getDefaultCallingConvention() const
{
    return m_interface->getDefaultCallingConvention();
}

void OptimizerCompilerInterface::udpateRegisterStartEndIndexes()
{
    int operationIndex = m_blockOperations.length()-1;

    cList<int> registers(m_interface->m_binary->getCurrentStack()->getRegistersTable().keys());
    cList<int>::iterator registersIterator(registers.begin());
    cList<int> allocatedRegisters;

    cList<int> dummyRegisters(m_interface->m_binary->getCurrentStack()->getRegistersTable().keys());
    cList<int>::iterator dummyRegistersIterator(dummyRegisters.begin());

    // Alive registers list
    for (registersIterator = registers.begin();
         registersIterator != registers.end() ;
         ++registersIterator)
    {
        if (m_interface->m_binary->getCurrentStack()->getRegistersTable()[(*registersIterator)].m_bAllocated)
            allocatedRegisters.append(*registersIterator);
    }

    // Make room for one more entry in the timeline
    m_dummyRegistersMapping.resize(m_blockOperations.length());

    // Update start and end time of registers according to the alive registers.
    for (dummyRegistersIterator = dummyRegisters.begin();
         dummyRegistersIterator != dummyRegisters.end();
         ++dummyRegistersIterator)
    {
        int dummyRegister = *dummyRegistersIterator;

        // If is not alive yet but in alive list, update start time
        if (allocatedRegisters.isIn(dummyRegister))
        {
            if (!m_dummyRegistersMapping.m_entries.hasKey(dummyRegister))
            {
                m_dummyRegistersMapping.m_entries.append(dummyRegister, DummyRegistersMapping::DummyRegisterEntry());
                // Make room for one more entry in the timeline
                m_dummyRegistersMapping.resize(m_blockOperations.length());
            }
            // Set this register to active in this entry
            m_dummyRegistersMapping.getAtRegister(dummyRegister).setActive(operationIndex, true);
            // Update the possible registers according to if it is volatile or non-volatile
            for (int i = 0; i < m_dummyRegistersMapping.m_numberOfRealRegisters; ++i)
            {
                if (dummyRegister <= START_VOLATILE_REGISTERS &&
                    dummyRegister > END_VOLATILE_REGISTERS) // Volatile
                {
                    // If Volatile, we can also return a non-volatile. No restrictions in this case.
                    //if (m_interface->getArchRegisters()[m_interface->m_indexToRegister[i]].m_eType != Volatile)
                        //m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex).m_possibleRegisters.clear(i);
                }
                else if (dummyRegister <= START_NON_VOLATILE_REGISTERS &&
                         dummyRegister > END_NON_VOLATILE_REGISTERS) // NonVolatile
                {
                    if (m_interface->getArchRegisters()[m_interface->m_indexToRegister[i]].m_eType != NonVolatile)
                        m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex).m_possibleRegisters.clear(i);
                }
                else
                    ASSERT(0);
            }
        }
    }
}

void OptimizerCompilerInterface::storeConst(uint         stackPosition,
                                            const uint8* bufferOffset,
                                            uint         size,
                                            bool         argumentStackLocation)
{
    if (!isOptimizerOn()) {
        m_interface->storeConst(stackPosition, bufferOffset, size, argumentStackLocation);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_STORE_CONST,
        stackPosition,
        0,
        0,
        size,
        StackInterface::EMPTY,
        StackInterface::EMPTY,
        argumentStackLocation,
        0,
        0,
        cString(0),
        bufferOffset);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::store32(uint stackPosition,
                                         uint size,
                                         StackLocation source,
                                         bool argumentStackLocation,
                                         bool isTempStack)
{
    if (!isOptimizerOn()) {
        m_interface->store32(stackPosition, size, source, argumentStackLocation, isTempStack);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_STORE_32,
        stackPosition,
        0,
        0,
        size,
        source,
        StackInterface::EMPTY,
        argumentStackLocation,
        isTempStack,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::store32(StackLocation source,
                                         uint size,
                                         bool signExtend,
                                         const cString& dependencyName)
{
    if (!isOptimizerOn()) {
        m_interface->store32(source, size, signExtend, dependencyName);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_STORE_32_DEPENDENCY,
        0,
        0,
        0,
        size,
        source,
        StackInterface::EMPTY,
        signExtend,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::move32(StackLocation destination,
                                        StackLocation source,
                                        uint size,
                                        bool signExtend)
{
    if (!isOptimizerOn()) {
        m_interface->move32(destination, source, size, signExtend);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_MOVE_32,
        0,
        0,
        0,
        size,
        source,
        destination,
        signExtend,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::load32(uint stackPosition,
                                        uint size,
                                        StackLocation destination,
                                        bool signExtend,
                                        bool argumentStackLocation,
                                        bool isTempStack)
{
    if (!isOptimizerOn()) {
        m_interface->load32(stackPosition, size, destination, signExtend, argumentStackLocation, isTempStack);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_32,
        stackPosition,
        0,
        0,
        size,
        StackInterface::EMPTY,
        destination,
        signExtend,
        argumentStackLocation,
        isTempStack,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::load32(StackLocation destination,
                                        uint size,
                                        bool signExtend,
                                        const cString& dependencyName)
{
    if (!isOptimizerOn()) {
        m_interface->load32(destination, size, signExtend, dependencyName);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_32_DEPENDENCY,
        0,
        0,
        0,
        size,
        StackInterface::EMPTY,
        destination,
        signExtend,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}


void OptimizerCompilerInterface::load32addr(uint stackPosition,
                                            uint size,
                                            uint offset,
                                            StackLocation destination,
                                            bool argumentStackLocation,
                                            bool isTempStack)
{
    if (!isOptimizerOn()) {
        m_interface->load32addr(stackPosition, size, offset, destination, argumentStackLocation);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_ADDR_32,
        stackPosition,
        offset,
        0,
        size,
        StackInterface::EMPTY,
        destination,
        argumentStackLocation,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::load32addr(StackLocation destination, const cString& dependencyName)
{
    if (!isOptimizerOn()) {
        m_interface->load32addr(destination, dependencyName);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_ADDR_32_DEPENDENCY,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::loadInt32(StackLocation destination,
                                           uint32 value)
{
    if (!isOptimizerOn()) {
        m_interface->loadInt32(destination, value);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_INT_32,
        value,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::loadInt32(StackLocation destination,
                                           const cString& dependencyName)
{
    if (!isOptimizerOn()) {
        m_interface->loadInt32(destination, dependencyName);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_INT_32_DEPENDENCY,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::assignRet32(StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->assignRet32(source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_ASSIGN_RET_32,
        0,
        0,
        0,
        0,
        source,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::pushArg32(StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->pushArg32(source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_PUSH_ARG_32,
        0,
        0,
        0,
        0,
        source,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::popArg32(StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->popArg32(source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_POP_ARG_32,
        0,
        0,
        0,
        0,
        source,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::call(const cString& dependencyName, uint numberOfArguments)
{
    if (!isOptimizerOn()) {
        m_interface->call(dependencyName, numberOfArguments);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CALL_DEPENDENCY,
        numberOfArguments,
        0,
        0,
        0,
        StackInterface::EMPTY,
        StackInterface::EMPTY,
        0,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::call(StackLocation address, uint numberOfArguments)
{
    if (!isOptimizerOn()) {
        m_interface->call(address, numberOfArguments);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CALL,
        numberOfArguments,
        0,
        0,
        0,
        address,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::call32(const cString& dependencyName,
                                        StackLocation destination, uint numberOfArguments)
{
    if (!isOptimizerOn()) {
        m_interface->call32(dependencyName, destination, numberOfArguments);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CALL_32_DEPENDENCY,
        numberOfArguments,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        dependencyName,
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::call32(StackLocation address,
                                        StackLocation destination, uint numberOfArguments)
{
    if (!isOptimizerOn()) {
        m_interface->call32(address, destination, numberOfArguments);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CALL_32,
        numberOfArguments,
        0,
        0,
        0,
        address,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::storeMemory(StackLocation destination, StackLocation value, uint offset,
                                             uint size)
{
    if (!isOptimizerOn()) {
        m_interface->storeMemory(destination, value, offset, size);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_STORE_MEMORY,
        offset,
        0,
        0,
        size,
        value,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::loadMemory(StackLocation destination, StackLocation value, uint offset,
                                            uint size, bool signExtend)
{
    if (!isOptimizerOn()) {
        m_interface->loadMemory(destination, value, offset, size, signExtend);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_MEMORY,
        offset,
        0,
        0,
        size,
        value,
        destination,
        signExtend,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::conv32(StackLocation destination,
                                        uint size,
                                        bool signExtend)
{
    if (!isOptimizerOn()) {
        m_interface->conv32(destination, size, signExtend);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CONV_32,
        0,
        0,
        0,
        size,
        StackInterface::EMPTY,
        destination,
        signExtend,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::neg32(StackLocation destination)
{
    if (!isOptimizerOn()) {
        m_interface->neg32(destination);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_NEG_32,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::not32(StackLocation destination)
{
    if (!isOptimizerOn()) {
        m_interface->not32(destination);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_NOT_32,
        0,
        0,
        0,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}
void OptimizerCompilerInterface::add32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->add32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_ADD_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::sub32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->sub32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_SUB_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}
void OptimizerCompilerInterface::mul32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->mul32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_MUL_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::div32(StackLocation destination, StackLocation source, bool sign)
{
    if (!isOptimizerOn()) {
        m_interface->div32(destination, source, sign);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_DIV_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        sign,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::rem32(StackLocation destination, StackLocation source, bool sign)
{
    if (!isOptimizerOn()) {
        m_interface->rem32(destination, source, sign);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_REM_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        sign,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::and32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->and32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_AND_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::xor32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->xor32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_XOR_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::shr32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->shr32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_SHR_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::shl32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->shl32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_SHL_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::or32 (StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->or32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_OR_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::adc32 (StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->adc32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_ADC_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::sbb32 (StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->sbb32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_SBB_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign)
{
    /* NOT IMPLEMENTED YET!!! */

    if (!isOptimizerOn()) {
        m_interface->mul32h(destlow, desthigh, source, sign);
        return;
    }

    /*
    udpateRegisterStartEndIndexes();
    CompilerInterface::CompilerOperation opcode(
        OPCODE_MUL_32H,
        0,
        0,
        0,
        source,
        destlow,
        desthigh,
        sign,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);*/
}

void OptimizerCompilerInterface::addConst32(const StackLocation destination,
                                            int32 value)
{
    if (!isOptimizerOn()) {
        m_interface->addConst32(destination, value);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_ADD_CONST_32,
        0,
        0,
        value,
        0,
        StackInterface::EMPTY,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::jump(int blockID)
{
    if (!isOptimizerOn()) {
        m_interface->jump(blockID);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_JUMP,
        0,
        0,
        blockID,
        0,
        StackInterface::EMPTY,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::jumpShort(int blockID)
{
    if (!isOptimizerOn()) {
        m_interface->jumpShort(blockID);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_JUMP_SHORT,
        0,
        0,
        blockID,
        0,
        StackInterface::EMPTY,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::jumpCond(StackLocation compare, int blockID, bool isZero)
{
    if (!isOptimizerOn()) {
        m_interface->jumpCond(compare, blockID, isZero);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_JUMP_COND,
        0,
        0,
        blockID,
        0,
        compare,
        StackInterface::EMPTY,
        isZero,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::jumpCondShort(StackLocation compare, int blockID, bool isZero)
{
    if (!isOptimizerOn()) {
        m_interface->jumpCondShort(compare, blockID, isZero);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_JUMP_COND_SHORT,
        0,
        0,
        blockID,
        0,
        compare,
        StackInterface::EMPTY,
        isZero,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}


void OptimizerCompilerInterface::ceq32(StackLocation destination, StackLocation source)
{
    if (!isOptimizerOn()) {
        m_interface->ceq32(destination, source);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CEQ_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::cgt32(StackLocation destination, StackLocation source, bool isSigned)
{
    if (!isOptimizerOn()) {
        m_interface->cgt32(destination, source, isSigned);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CGT_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        isSigned,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

void OptimizerCompilerInterface::clt32(StackLocation destination, StackLocation source, bool isSigned)
{
    if (!isOptimizerOn()) {
        m_interface->clt32(destination, source, isSigned);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_CLT_32,
        0,
        0,
        0,
        0,
        source,
        destination,
        isSigned,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}


void OptimizerCompilerInterface::localloc(StackLocation destination, StackLocation size,
                                          bool isStackEmpty)
{
    if (!isOptimizerOn()) {
        m_interface->localloc(destination, size, isStackEmpty);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOCALLOC,
        0,
        0,
        0,
        0,
        size,
        destination,
        isStackEmpty,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}


void OptimizerCompilerInterface::revertStack(uint32 size)
{
    if (!isOptimizerOn()) {
        m_interface->revertStack(size);
        return;
    }

    CompilerInterface::CompilerOperation opcode(
        OPCODE_REVERT_STACK,
        0,
        0,
        0,
        size,
        StackInterface::EMPTY,
        StackInterface::EMPTY,
        0,
        0,
        0,
        cString(0),
        0);

    m_blockOperations.append(opcode);

    udpateRegisterStartEndIndexes();
}

const CompilerParameters& OptimizerCompilerInterface::getCompilerParameters() const
{
    return m_interface->getCompilerParameters();
}

void OptimizerCompilerInterface::generateMethodEpiProLogs(bool bForceSaveNonVolatiles)
{
    m_interface->generateMethodEpiProLogs(bForceSaveNonVolatiles);
}

void OptimizerCompilerInterface::mapRegisters(StackLocation basePointer)
{
    m_dummyRegistersMapping.assignRegistersClever(basePointer);
    CHECK(m_dummyRegistersMapping.assignRegistersVerify());
#ifdef _DEBUG
    if (m_parameters.m_bDeveloperVerbosity)
    {
        int numberOfSwaps = m_dummyRegistersMapping.countSwaps();
        static int total = 0;
        static int counter = 0;

        m_dummyRegistersMapping.printTimeline();

        if (numberOfSwaps > 0)
        {
            counter++;
            total += numberOfSwaps;
            cout << counter << ") Number of Swaps is " << m_dummyRegistersMapping.countSwaps() << endl;
            cout << "Total is " << total << endl;
        }
    }
#endif
}

void OptimizerCompilerInterface::switchRegisters(BlockOperationList::iterator operationsPosition, int srcRealRegister, int dstRealRegister)
{
    CompilerInterface::CompilerOperation opcode(
        OPCODE_MOVE_32,
        0,
        0,
        0,
        4,
        StackInterface::buildStackLocation(srcRealRegister,0),
        StackInterface::buildStackLocation(dstRealRegister,0),
        false,
        true,
        0,
        cString(0),
        0);
    m_blockOperations.insert(operationsPosition, opcode);
    //doOperation(opcode);
}

StackLocation OptimizerCompilerInterface::storeRegisterToStackLocation(BlockOperationList::iterator operationsPosition, int realRegister)
{
    StackLocation tmpStackPosition = m_interface->getFirstPassPtr()->getCurrentStack()->allocateTemporaryStackBuffer(
        m_interface->getFirstPassPtr()->getCurrentStack()->getRegisterSize());

    // Add the new operation to the list of operations in the correct position.
    CompilerInterface::CompilerOperation opcode(
        OPCODE_STORE_32,
        tmpStackPosition.u.reg,
        0,
        0,
        4,
        StackInterface::buildStackLocation(realRegister, 0),
        StackInterface::EMPTY,
        false,
        true,
        false,
        cString(0),
        0);
    m_blockOperations.insert(operationsPosition, opcode);
    //doOperation(opcode);
    return tmpStackPosition;
}

void OptimizerCompilerInterface::loadRegisterFromStackLocation(BlockOperationList::iterator operationsPosition, StackLocation stackPosition, int realRegister)
{
    CompilerInterface::CompilerOperation opcode(
        OPCODE_LOAD_32,
        stackPosition.u.reg,
        0,
        0,
        4,
        StackInterface::EMPTY,
        StackInterface::buildStackLocation(realRegister, 0),
        false,
        false,
        true,
        cString(0),
        0);
    m_blockOperations.insert(operationsPosition, opcode);
    //doOperation(opcode);

    m_interface->getFirstPassPtr()->getCurrentStack()->freeTemporaryStackBuffer(stackPosition);
}

void OptimizerCompilerInterface::updateDummyRegistersMapping(CompilerInterface::CompilerOperation opcode, int operationIndex)
{
    // If the register is valid and not the stack register, update.
    // If the register is the stack register, there is a 1-to-1 mapping, so there is no need to update restrictions
    if ((opcode.sloc1.u.reg != 0) &&
        (opcode.sloc1 != getStackPointer()))
    {
        m_dummyRegistersMapping.getAtRegister(opcode.sloc1.u.reg, operationIndex).m_possibleRegisters &=
            m_interface->getOperationRegisterAllocationInfo(opcode).m_acceptableSource;
    }

    // If the register is valid and not the stack register, update.
    // If the register is the stack register, there is a 1-to-1 mapping, so there is no need to update restrictions
    if ((opcode.sloc2.u.reg != 0) &&
        (opcode.sloc2 != getStackPointer()))
    {
        m_dummyRegistersMapping.getAtRegister(opcode.sloc2.u.reg, operationIndex).m_possibleRegisters &=
            m_interface->getOperationRegisterAllocationInfo(opcode).m_acceptableDest;
    }
}

void OptimizerCompilerInterface::backupRegisters(BlockOperationList::iterator operationsPosition, int operationIndex)
{
    cList<int> dummyRegisters(m_dummyRegistersMapping.m_entries.keys());
    cList<int>::iterator dummyRegistersIterator(dummyRegisters.begin());
    StackLocation stackLocation;

    cList<int> dummyRegistersInner(m_dummyRegistersMapping.m_entries.keys());
    cList<int>::iterator dummyRegistersInnerIterator(dummyRegistersInner.begin());

    for (; dummyRegistersIterator != dummyRegisters.end() ; ++dummyRegistersIterator)
    {
        int dummyRegister = *dummyRegistersIterator;
        int currentMappingOfDummyRegister = -1;
        int previousMappingOfDummyRegister = -1;

        // If register is not alive in current operation or already on stack from previous operation, ignore it.
        if ((!(m_dummyRegistersMapping.getAtRegister(dummyRegister).isActive(operationIndex))) ||
            (m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation != StackInterface::EMPTY))
            continue;

        // Optimization: Dont touch the destination of the operation (it is going to be overwritten anyway)
        if (((*operationsPosition).sloc2.u.reg == dummyRegister) &&
            (!m_interface->getOperationRegisterAllocationInfo(*operationsPosition).m_isDestAlsoSource))
            continue;

        // Get current mapping
        currentMappingOfDummyRegister = m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex).m_chosenRegister];

        // Check if there is history
        if (m_dummyRegistersMapping.getAtRegister(dummyRegister).m_historyChosenRegister != -1)
        {
            // The previous mapping is from the history
            previousMappingOfDummyRegister = m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(dummyRegister).m_historyChosenRegister];
            // Mark that there is no more history
            m_dummyRegistersMapping.getAtRegister(dummyRegister).m_historyChosenRegister = -1;
        }
        else // No history
        {
            // If this is the first operation or register was not alive in previous operation, there is nothing to do
            if ((operationIndex == 0) ||
                (!(m_dummyRegistersMapping.getAtRegister(dummyRegister).isActive(operationIndex-1))))
                continue;

            // take previous mapping from previous operation.
            previousMappingOfDummyRegister = m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex-1).m_chosenRegister];
        }
        // If mapping changed from previous operation, save the previous mapping on the temp-stack.
        if (currentMappingOfDummyRegister != previousMappingOfDummyRegister)
        {
            /*// Check if the destination is used by another register in the previous operation. If not we can optimize by doing a move.
            bool isUsed = false;
            for (dummyRegistersInnerIterator = dummyRegistersInner.begin();
                 dummyRegistersInnerIterator != dummyRegistersInner.end() ;
                 ++dummyRegistersInnerIterator)
            {
                int dummyRegisterInner = *dummyRegistersInnerIterator;
                // If other register is alive at previous time
                if (m_dummyRegistersMapping.getAtRegister(dummyRegisterInner, operationIndex-1).m_isAlive)
                    // If alive,
                    if (currentMappingOfDummyRegister ==
                        m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(dummyRegisterInner, operationIndex-1).m_chosenRegister])
                    {
                        isUsed = true;
                        break;
                    }
            }
            // If the destination is in use, do a store to tempStack
            if (isUsed)
            {*/
            stackLocation = storeRegisterToStackLocation(operationsPosition, previousMappingOfDummyRegister);
            m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation = stackLocation;
            /*}
            // If the destination is not used, just do a mov. Notice this is not the first operation
            else
            {
                switchRegisters(operationsPosition, previousMappingOfDummyRegister, currentMappingOfDummyRegister);
                m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation = StackInterface::EMPTY;
            }*/
        }
    }
}

int OptimizerCompilerInterface::updateRegisterByMapping(BlockOperationList::iterator operationsPosition, int operationIndex,  int dummyRegister)
{
    int realRegister = 0;

    // Do the update only if the dummy register is not empty (points at a real register)
    if (0 == dummyRegister)
        goto Exit;

    // If we get the base stack register, just return it
    // TODO: We assume dummyregisters don't collide with the real registers in numbers
    // In the future we can make this a special dummy register and check that dummyRegister == dummyStackRegister
    if (dummyRegister == INITIAL_BASE_STACK_REGISTER)
    {
        realRegister = m_interface->getMethodBaseStackRegister().u.reg;
        goto Exit;
    }

    realRegister = m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex).m_chosenRegister];

    if (m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation != StackInterface::EMPTY)
    {
        loadRegisterFromStackLocation(operationsPosition, m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation, realRegister);
        // Empty the stackLocation
        m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation = StackInterface::EMPTY;
    }

Exit:
    return realRegister;
}

void OptimizerCompilerInterface::handleModifications(BlockOperationList::iterator operationsPosition, int operationIndex)
{
    const cSetArray& modifiableRegisters = m_interface->getOperationRegisterAllocationInfo(*operationsPosition).m_modifiable;
    StackLocation stackLocation;

    cList<int> dummyRegisters(m_dummyRegistersMapping.m_entries.keys());
    cList<int>::iterator dummyRegistersIterator(dummyRegisters.begin());

    for (uint i = 0 ; i < modifiableRegisters.getLength() ; ++i)
    {
        // If the register is not modifiable by the operation, ignore it
        if (!modifiableRegisters.isSet(i))
            continue;

        for (dummyRegistersIterator = dummyRegisters.begin();
             dummyRegistersIterator != dummyRegisters.end() ;
             ++dummyRegistersIterator)
        {
            int dummyRegister = *dummyRegistersIterator;
            // If active and mathes the modifiable and NOT already on stack, save it.
            if (m_dummyRegistersMapping.getAtRegister(dummyRegister).isActive(operationIndex) &&
                m_dummyRegistersMapping.getAtRegister(dummyRegister, operationIndex).m_chosenRegister == i &&
                m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation == StackInterface::EMPTY)
            {
                stackLocation = storeRegisterToStackLocation(operationsPosition, m_interface->m_indexToRegister[i]);
                m_dummyRegistersMapping.getAtRegister(dummyRegister).m_stackLocation = stackLocation;
            }
        }
    }
}

void OptimizerCompilerInterface::doOperation(CompilerInterface::CompilerOperation &operation)
{
    switch(operation.opcode) {
    case OPCODE_STORE_CONST:
        m_interface->storeConst(operation.uval1, operation.buffer, operation.size, operation.cond1);
        break;
    case OPCODE_STORE_32:
        m_interface->store32(operation.uval1, operation.size, operation.sloc1, operation.cond1, operation.cond2);
        break;
    case OPCODE_STORE_32_DEPENDENCY:
        m_interface->store32(operation.sloc1, operation.size, operation.cond1, operation.dependencyName);
        break;
    case OPCODE_MOVE_32:
        m_interface->move32(operation.sloc2, operation.sloc1, operation.size, operation.cond1);
        break;
    case OPCODE_LOAD_32:
        m_interface->load32(operation.uval1, operation.size, operation.sloc2, operation.cond1, operation.cond2, operation.cond3);
        break;
    case OPCODE_LOAD_32_DEPENDENCY:
        m_interface->load32(operation.sloc2, operation.size, operation.cond1, operation.dependencyName);
        break;
    case OPCODE_LOAD_ADDR_32:
        m_interface->load32addr(operation.uval1, operation.size, operation.uval2, operation.sloc2, operation.cond1);
        break;
    case OPCODE_LOAD_ADDR_32_DEPENDENCY:
        m_interface->load32addr(operation.sloc2, operation.dependencyName);
        break;
    case OPCODE_LOAD_INT_32:
        m_interface->loadInt32(operation.sloc2, operation.uval1);
        break;
    case OPCODE_LOAD_INT_32_DEPENDENCY:
        m_interface->loadInt32(operation.sloc2, operation.dependencyName);
        break;
    case OPCODE_ASSIGN_RET_32:
        m_interface->assignRet32(operation.sloc1);
        break;

    case OPCODE_PUSH_ARG_32:
        m_interface->pushArg32(operation.sloc1);
        break;
    case OPCODE_POP_ARG_32:
        m_interface->popArg32(operation.sloc1);
        break;
    case OPCODE_CALL:
        m_interface->call(operation.sloc1, operation.uval1);
        break;
    case OPCODE_CALL_DEPENDENCY:
        m_interface->call(operation.dependencyName, operation.uval1);
        break;
    case OPCODE_CALL_32:
        m_interface->call32(operation.sloc1, operation.sloc2, operation.uval1);
        break;
    case OPCODE_CALL_32_DEPENDENCY:
        m_interface->call32(operation.dependencyName, operation.sloc2, operation.uval1);
        break;
    case OPCODE_STORE_MEMORY:
        m_interface->storeMemory(operation.sloc2, operation.sloc1, operation.uval1, operation.size);
        break;
    case OPCODE_LOAD_MEMORY:
        m_interface->loadMemory(operation.sloc2, operation.sloc1, operation.uval1, operation.size, operation.cond1);
        break;
    case OPCODE_CONV_32:
        m_interface->conv32(operation.sloc2, operation.size, operation.cond1);
        break;
    case OPCODE_NEG_32:
        m_interface->neg32(operation.sloc2);
        break;
    case OPCODE_NOT_32:
        m_interface->not32(operation.sloc2);
        break;
    case OPCODE_ADD_32:
        m_interface->add32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_SUB_32:
        m_interface->sub32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_MUL_32:
        m_interface->mul32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_DIV_32:
        m_interface->div32(operation.sloc2, operation.sloc1, operation.cond1);
        break;
    case OPCODE_REM_32:
        m_interface->rem32(operation.sloc2, operation.sloc1, operation.cond1);
        break;
    case OPCODE_AND_32:
        m_interface->and32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_XOR_32:
        m_interface->xor32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_SHR_32:
        m_interface->shr32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_SHL_32:
        m_interface->shl32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_OR_32:
        m_interface->or32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_ADC_32:
        m_interface->adc32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_SBB_32:
        m_interface->sbb32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_MUL_32H:
        ASSERT(0); //Not implemented
        break;
    case OPCODE_ADD_CONST_32:
        m_interface->addConst32(operation.sloc2, operation.val);
        break;
    case OPCODE_JUMP:
        m_interface->jump(operation.val);
        break;
    case OPCODE_JUMP_SHORT:
        m_interface->jumpShort(operation.val);
        break;
    case OPCODE_JUMP_COND:
        m_interface->jumpCond(operation.sloc1, operation.val, operation.cond1);
        break;
    case OPCODE_JUMP_COND_SHORT:
        m_interface->jumpCondShort(operation.sloc1, operation.val, operation.cond1);
        break;
    case OPCODE_CEQ_32:
        m_interface->ceq32(operation.sloc2, operation.sloc1);
        break;
    case OPCODE_CGT_32:
        m_interface->cgt32(operation.sloc2, operation.sloc1, operation.cond1);
        break;
    case OPCODE_CLT_32:
        m_interface->clt32(operation.sloc2, operation.sloc1, operation.cond1);
        break;
    case OPCODE_LOCALLOC:
        m_interface->localloc(operation.sloc2, operation.sloc1, operation.cond1);
        break;
    case OPCODE_REVERT_STACK:
        m_interface->revertStack(operation.size);
        break;
    case OPCODE_SET_FRAME_POINTER:
        m_interface->setFramePointer(operation.sloc2);
        break;
    case OPCODE_RESET_BASE_STACK_REGISTER:
        m_interface->resetBaseStackRegister(operation.sloc2);
        break;
    default:
        ASSERT(0);
        break;
    }
}

void OptimizerCompilerInterface::removeUnnecessaryLoad()
{
    BlockOperationList::iterator firstOperation(m_blockOperations.begin());
    BlockOperationList::iterator secondOperation(m_blockOperations.begin());
    ++secondOperation;

    for (; secondOperation != m_blockOperations.end();
         ++firstOperation, ++secondOperation)
    {
        CompilerInterface::CompilerOperation& operation1 = *firstOperation;
        CompilerInterface::CompilerOperation& operation2 = *secondOperation;
        if (operation1.opcode != OPCODE_STORE_32 ||
            operation2.opcode != OPCODE_LOAD_32)
            continue;

        if (operation1.uval1 != operation2.uval1 || // stackPosition
            operation1.cond1 != operation2.cond2 || // ArgumentStackLocation
            operation1.cond2 != operation2.cond3 || // isTempStack
            operation1.sloc1 != operation2.sloc2 ) // source != destination
        {
            continue;
        }

        secondOperation = m_blockOperations.remove(secondOperation);
    }
}

void OptimizerCompilerInterface::removeUnnecessaryMove()
{
    BlockOperationList::iterator firstOperation(m_blockOperations.begin());
    BlockOperationList::iterator secondOperation(m_blockOperations.begin());
    ++secondOperation;

    for (; secondOperation != m_blockOperations.end();
         ++firstOperation, ++secondOperation)
    {
        CompilerInterface::CompilerOperation& operation1 = *firstOperation;
        CompilerInterface::CompilerOperation& operation2 = *secondOperation;
        if (operation1.opcode != OPCODE_LOAD_32 ||
            operation2.opcode != OPCODE_LOAD_32)
            continue;

        if (operation1.sloc1 != operation2.sloc2 ) // source != destination
        {
            continue;
        }

        secondOperation = m_blockOperations.remove(secondOperation);
    }
}

void OptimizerCompilerInterface::renderBlock()
{
    BlockOperationList::iterator current_operation(m_blockOperations.begin());
    int operationIndex = 0;

    if (m_blockOperations.isEmpty())
        return;

    // Update possible registers for every register
    operationIndex = 0;
    for (current_operation = m_blockOperations.begin();
         current_operation != m_blockOperations.end();
         ++current_operation, ++operationIndex)
    {
        CompilerInterface::CompilerOperation& operation = *current_operation;
        updateDummyRegistersMapping(operation, operationIndex);
    }

    // Translate dummy base pointer register to real register
    StackLocation oldDummyBaseRegister = m_interface->m_binary->getCurrentStack()->getBaseStackRegister();
    if (oldDummyBaseRegister.u.reg == INITIAL_BASE_STACK_REGISTER)
    {
        mapRegisters(StackInterface::EMPTY);
        m_interface->m_binary->getCurrentStack()->setBaseStackRegister(m_interface->getStackPointer());
    }
    else
    {
        mapRegisters(oldDummyBaseRegister);
        m_interface->m_binary->getCurrentStack()->setBaseStackRegister(
            StackInterface::buildStackLocation(m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(oldDummyBaseRegister.u.reg, 0).m_chosenRegister], 0));
    }

    // Get the correct register from dummy register, along the way add handling of collisions in assignments
    operationIndex = 0;
    for (current_operation = m_blockOperations.begin();
         current_operation != m_blockOperations.end();
         ++current_operation, ++operationIndex)
    {
        CompilerInterface::CompilerOperation& operation = *current_operation;
        int srcDummyRegister = operation.sloc1.u.reg;
        int dstDummyRegister = operation.sloc2.u.reg;

        // Store what needed on tempstack
        backupRegisters(current_operation, operationIndex);

        // Add handling of registers that have a chance to change.
        handleModifications(current_operation, operationIndex);

        // Map the real register instead of the dummy
        operation.sloc1.u.reg = updateRegisterByMapping(current_operation, operationIndex, srcDummyRegister);
        // If the source and dest are the same, don't do anything to get the dest
        if (srcDummyRegister == dstDummyRegister)
            operation.sloc2.u.reg = operation.sloc1.u.reg;
        else
            operation.sloc2.u.reg = updateRegisterByMapping(current_operation, operationIndex, dstDummyRegister);
        //doOperation(operation);
    }

    removeUnnecessaryLoad();
    removeUnnecessaryMove();

    // Run every command and hope for the best.
    for (current_operation = m_blockOperations.begin();
         current_operation != m_blockOperations.end();
         ++current_operation)
    {
        CompilerInterface::CompilerOperation& operation = *current_operation;
        doOperation(operation);
    }

    // After sealing the block, clean the list.
    m_blockOperations.removeAll();

    // Update touched registers
    updateTouchedRegisters();

    // Initialize the mapping in the registerAllocationTable and save history for next block
    saveHistoryForNextBlock();

    // Initialize m_dummyRegistersMapping
    m_dummyRegistersMapping.initialize();

    m_interface->m_binary->getCurrentStack()->setBaseStackRegister(oldDummyBaseRegister);
}

void OptimizerCompilerInterface::updateTouchedRegisters()
{
    cList<int> touchedRegisters(m_interface->m_binary->getTouchedRegisters().keys());
    cList<int>::iterator touchedRegistersIterator(touchedRegisters.begin());

    for (touchedRegistersIterator = touchedRegisters.begin();
         touchedRegistersIterator != touchedRegisters.end() ;
         ++touchedRegistersIterator)
    {
        int touchedRegister = *touchedRegistersIterator;
        if (m_dummyRegistersMapping.m_entries.hasKey(touchedRegister))
        {
            for (int i = 0 ; i < m_dummyRegistersMapping.m_numberOfOperations ; ++i)
            {
                if (m_dummyRegistersMapping.getAtRegister(touchedRegister).isActive(i))
                {
                    m_interface->m_binary->untouch(touchedRegister);
                    m_interface->m_binary->touch(m_interface->m_indexToRegister[m_dummyRegistersMapping.getAtRegister(touchedRegister, i).m_chosenRegister]);
                }
            }
        }
    }
}

void OptimizerCompilerInterface::saveHistoryForNextBlock()
{
    cList<int> dummyRegisters(m_dummyRegistersMapping.m_entries.keys());
    cList<int>::iterator dummyRegistersIterator(dummyRegisters.begin());

    cList<int> registers(m_interface->m_binary->getCurrentStack()->getRegistersTable().keys());
    cList<int>::iterator registersIterator(registers.begin());
    cList<int> allocatedRegisters;

    // Retrieve alive registers list
    for (registersIterator = registers.begin();
         registersIterator != registers.end() ;
         ++registersIterator)
    {
        if (m_interface->m_binary->getCurrentStack()->getRegistersTable()[(*registersIterator)].m_bAllocated)
            allocatedRegisters.append(*registersIterator);
    }

    for (dummyRegistersIterator = dummyRegisters.begin();
         dummyRegistersIterator != dummyRegisters.end();
         ++dummyRegistersIterator)
    {
        int dummyRegister = *dummyRegistersIterator;
        // If the dummy register is in the alive list, it should be saved for the next run.
        if (allocatedRegisters.isIn(dummyRegister))
        {
            int lastChosenRegister = m_dummyRegistersMapping.getAtRegister(dummyRegister).getLastEntry().m_chosenRegister;
            // Save the last mapping of the dummy register for the next block.
            m_dummyRegistersMapping.getAtRegister(dummyRegister).m_historyChosenRegister = lastChosenRegister;
        }
    }
}

CompilerInterface* OptimizerCompilerInterface::getInnerCompilerInterface()
{
    return m_interface;
}

bool OptimizerCompilerInterface::isOptimizerCompiler() const
{
    return true;
}

void DummyRegistersMapping::assignRegistersSimple()
{
    /*cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

    assignRegistersMust();
    assignRegistersFini();    */
}

int DummyRegistersMapping::countSwaps()
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

    uint count = 0;

    for (dummyRegistersIter = dummyRegisters.begin();
                dummyRegistersIter != dummyRegisters.end() ;
                ++dummyRegistersIter)
    {
        DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
        for (int index = 0 ; index < m_numberOfOperations-1 ; ++index)
        {
            DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
            DummyRegistersMapping::Entry &entry = dummyRegisterEntry[index];
            DummyRegistersMapping::Entry &nextEntry = dummyRegisterEntry[index + 1];

            if ((!dummyRegisterEntry.isActive(index)) ||
                (!dummyRegisterEntry.isActive(index + 1)))
                continue;

            if (nextEntry.m_chosenRegister != entry.m_chosenRegister)
            {
                count++;
            }
        }
    }
    return count;
}

void DummyRegistersMapping::assignRegistersMust(StackLocation baseRegister)
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());
    bool foundSingle = false;

    for (int index = 0 ; index < m_numberOfOperations ; ++index)
    {
        do
        {
            foundSingle = false;
            for (dummyRegistersIter = dummyRegisters.begin();
                 dummyRegistersIter != dummyRegisters.end() ;
                 ++dummyRegistersIter)
            {
                DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
                DummyRegistersMapping::Entry &entry = dummyRegisterEntry[index];
                // Ignore entries which correspond to registers which are not alive.
                if ((entry.m_chosenRegister != -1) ||
                    (!dummyRegisterEntry.isActive(index)))
                    continue;

                // If there is a register that must be something in a particular index,
                // set it and clear that bit from the other registers in that time.
                if (entry.m_possibleRegisters.isSetInOnePlace())
                {
                    foundSingle = true;

                    uint chosenReg = entry.m_possibleRegisters.first();
                    // Set the color of the entry to be that.
                    entry.m_chosenRegister = chosenReg;
                    // clear this color from the other registers at that point.
                    dontAllowAssignment(chosenReg, index);
                }
            }
        } while (foundSingle);
    }

    if (baseRegister != StackInterface::EMPTY)
    {
        bool isAvailable = true;
        for (int reg = 0 ; reg < m_numberOfRealRegisters ; ++reg)
        {
            isAvailable = true;
            for (int index = 0 ; index < m_numberOfOperations ; ++index)
            {
                // If register is alive and reg is not allowed there
                if ((getAtRegister(baseRegister.u.reg, index).m_isAlive) &&
                    (!getAtRegister(baseRegister.u.reg, index).m_possibleRegisters.isSet(reg)))
                    isAvailable = false;
            }
            if (isAvailable)
            {
                // If we found a register for the basepointer, disallow it for everybody else and
                // Also set it to be constant (swaps).
                for (int index = 0 ; index < m_numberOfOperations ; ++index)
                {
                    getAtRegister(baseRegister.u.reg, index).m_chosenRegister = reg;
                    dontAllowAssignment(reg, index);
                }
                break;
            }
        }
    }
}

void DummyRegistersMapping::assignRegistersExtend()
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());
    bool extended = false;

    do
    {
        extended = false;
        for (dummyRegistersIter = dummyRegisters.begin();
              dummyRegistersIter != dummyRegisters.end() ;
             ++dummyRegistersIter)
        {
            DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
            for (int index = 1 ; index < m_numberOfOperations-1 ; ++index)
            {
                if ((!dummyRegisterEntry.isActive(index)) ||
                    (!dummyRegisterEntry.isActive(index + 1)))
                    continue;

                if ((dummyRegisterEntry[index].m_chosenRegister != -1) &&
                    (dummyRegisterEntry[index + 1].m_chosenRegister == -1))
                {
                    if (dummyRegisterEntry[index + 1].m_possibleRegisters.isSet(dummyRegisterEntry[index].m_chosenRegister))
                    {
                        extended = true;
                        dummyRegisterEntry[index + 1].m_chosenRegister = dummyRegisterEntry[index].m_chosenRegister;
                        dontAllowAssignment(dummyRegisterEntry[index + 1].m_chosenRegister, index + 1);
                    }
                }

                if ((dummyRegisterEntry[index].m_chosenRegister == -1) &&
                    (dummyRegisterEntry[index + 1].m_chosenRegister != -1))
                {
                    if (dummyRegisterEntry[index].m_possibleRegisters.isSet(dummyRegisterEntry[index + 1].m_chosenRegister))
                    {
                        extended = true;
                        dummyRegisterEntry[index].m_chosenRegister = dummyRegisterEntry[index + 1].m_chosenRegister;
                        dontAllowAssignment(dummyRegisterEntry[index].m_chosenRegister, index);
                    }
                }
            }
        }
    } while (extended);
}

void DummyRegistersMapping::assignRegistersFini()
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

    for (int index = 0 ; index < m_numberOfOperations ; ++index)
    {
        for (dummyRegistersIter = dummyRegisters.begin();
                dummyRegistersIter != dummyRegisters.end() ;
                ++dummyRegistersIter)
        {
            DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
            if (!dummyRegisterEntry.isActive(index))
                continue;

            // Ignore entries which correspond to registers which are not alive.
            if ((dummyRegisterEntry[index].m_chosenRegister != -1) ||
                (!dummyRegisterEntry.isActive(index)))
                    continue;

            uint chosenReg = -1;
            bool found = false;

            if (index >= 1)
                if (dummyRegisterEntry[index-1].m_chosenRegister != -1)
                    if (dummyRegisterEntry[index].m_possibleRegisters.isSet(dummyRegisterEntry[index-1].m_chosenRegister))
                    {
                        chosenReg = dummyRegisterEntry[index-1].m_chosenRegister;
                        found = true;
                    }
            if (!found)
                if (index < m_numberOfOperations - 1)
                    if (dummyRegisterEntry[index+1].m_chosenRegister != -1)
                        if (dummyRegisterEntry[index].m_possibleRegisters.isSet(dummyRegisterEntry[index+1].m_chosenRegister))
                        {
                            chosenReg = dummyRegisterEntry[index+1].m_chosenRegister;
                            found = true;
                        }
            if (!found)
            {
                chosenReg = dummyRegisterEntry[index].m_possibleRegisters.first();
            }

            dummyRegisterEntry[index].m_chosenRegister = chosenReg;
            dontAllowAssignment(chosenReg, index);
        }
    }
}

void DummyRegistersMapping::assignRegistersClever(StackLocation basePointer)
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());
    bool foundSingle = false;

    assignRegistersMust(basePointer);
    assignRegistersExtend();
    assignRegistersFini();
}

bool DummyRegistersMapping::assignRegistersVerify()
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

    for (int index = 0 ; index < m_numberOfOperations ; ++index)
    {
        for (dummyRegistersIter = dummyRegisters.begin();
                dummyRegistersIter != dummyRegisters.end() ;
                ++dummyRegistersIter)
        {
            DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
            if (!dummyRegisterEntry.isActive(index))
                continue;
            if (dummyRegisterEntry[index].m_chosenRegister == -1)
                return false;
        }
    }
    return true;
}


void DummyRegistersMapping::printTimeline()
{
    cList<int> dummyRegisters(m_entries.keys());
    cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

    for (dummyRegistersIter = dummyRegisters.begin();
            dummyRegistersIter != dummyRegisters.end() ;
            ++dummyRegistersIter)
    {
        DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
        cout << "Register Number: " << HEXDWORD(*dummyRegistersIter) << ", ";
    }
    cout << endl;

    cout << "Number of Operations: " << m_numberOfOperations << endl;
    cout << "Number of Real Registers: " << m_numberOfRealRegisters << endl;

    for (dummyRegistersIter = dummyRegisters.begin();
         dummyRegistersIter != dummyRegisters.end() ;
         ++dummyRegistersIter)
    {
        DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
        cout << "Register Number: " << HEXDWORD(*dummyRegistersIter);
        cout << "StackLocation "<< HEXDWORD(dummyRegisterEntry.m_stackLocation.raw);

        for (int index = 0 ; index < m_numberOfOperations ; ++index)
        {
            if (!dummyRegisterEntry.isActive(index))
                continue;

            DummyRegistersMapping::Entry &entry = dummyRegisterEntry[index];
            cout << "Index " << index << endl;
            cout << "    PossibleRegisters ";
            for (uint i = 0 ; i < entry.m_possibleRegisters.getLength() ; i++)
            {
                if (entry.m_possibleRegisters.isSet(i))
                    cout << i << ' ';
            }
            cout << endl;
            cout << "    ChosenRegister " << entry.m_chosenRegister << endl;
        }
    }
}
