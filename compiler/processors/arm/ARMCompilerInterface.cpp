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

#include "compiler/stdafx.h"

#include "dismount/assembler/MangledNames.h"
#include "dismount/assembler/StackInterface.h"

#include "compiler/MethodBlock.h"
#include "compiler/CallingConvention.h"
#include "compiler/processors/arm/ARMCompilerInterface.h"
#include "compiler/StackEntity.h"
#include "runnable/FrameworkMethods.h"

char* gARMRegisters[16] = {
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "sp",
    "lr",
    "pc",
};

ARMCompilerInterface::ARMCompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params) :
    CompilerInterface(framework, params)
{
    m_archRegisters.append(getGPEncoding(ARM_GP32_R0), RegisterEntry(Volatile, false, 1));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R1), RegisterEntry(Volatile, false, 2));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R2), RegisterEntry(Volatile, false, 3));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R3), RegisterEntry(Volatile, false, 4));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R4), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R5), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R6), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R7), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R8), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R9), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_R10), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(ARM_GP32_FP), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(ARM_GP32_IP), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(ARM_GP32_SP), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(ARM_GP32_LR), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(ARM_GP32_PC), RegisterEntry(Platform, true));
    //setBaseStackRegister(getStackPointer());
    m_nonVolSize = 0;
    m_stackRef = 0;
    m_binary = FirstPassBinaryPtr(new FirstPassBinary(
                                      OpcodeSubsystems::DISASSEMBLER_ARM_32_LE,
                                      false));
}

uint8 ARMCompilerInterface::getBaseStackRegister(StackLocation baseRegister)
{
    if (baseRegister == StackInterface::NO_MEMORY)
        return ARM_GP32_SP;
    ASSERT(baseRegister.u.flags == 0);
    return getGPEncoding(baseRegister.u.reg);
}

void ARMCompilerInterface::setLocalsSize(uint localStackSize)
{
    m_binary->setStackBaseSize(localStackSize);
}

void ARMCompilerInterface::setArgumentsSize(uint argsSize)
{
    m_binary->setArgumentsSize(argsSize);
}

ARMCompilerInterface::StackSize ARMCompilerInterface::getStackSize() const
{
    return STACK_32;
}

uint ARMCompilerInterface::getShortJumpLength() const
{
    // 23 bit +/-
    return 0x7FFFFF;
}

StackLocation ARMCompilerInterface::getStackPointer() const
{
    return StackInterface::buildStackLocation(getGPEncoding(ARM_GP32_SP), 0);
}

const char* ARMCompilerInterface::getRegister32(StackLocation destination)
{
    return gARMRegisters[getGPEncoding(destination.u.reg)];
}

void ARMCompilerInterface::freeRegister32(int gpreg)
{
    StackLocation registerLocation = StackInterface::buildStackLocation(getGPEncoding(gpreg), 0);
    if (!m_binary->getCurrentStack()->isFreeTemporaryRegister(registerLocation))
    {
        // Allocate a stack based register
        StackLocation stackObject =
            m_binary->getCurrentStack()->replaceRegisterToStackVariable(
            registerLocation);
        store32(stackObject.u.reg, 4, registerLocation, false, true);
    }
}

void ARMCompilerInterface::storeConst(uint stackPosition,
                                      const uint8* bufferOffset,
                                      uint size,
                                      bool argumentStackLocation)
{
    StackLocation tempReg = m_binary->getCurrentStack()->
                            allocateTemporaryRegister();
    CHECK(tempReg != StackInterface::NO_MEMORY);

    loadInt32(tempReg, *((uint32*)bufferOffset));
    store32(stackPosition, size, tempReg, argumentStackLocation, false);

    m_binary->getCurrentStack()->freeTemporaryRegister(tempReg);
}

void ARMCompilerInterface::store32(uint stackPosition,
                                   uint size,
                                   StackLocation source,
                                   bool argumentStackLocation,
                                   bool isTempStack)
{
    if (argumentStackLocation)
    {
        stackPosition += getFrameStackRef() + 4;
        if (size != 2)
        {
            /*
             * STR Rn [SP, #stackPosition]; (12-bit)
             */
            m_binary->appendUint8(stackPosition & 0xFF);
            m_binary->appendUint8((getGPEncoding(source.u.reg) << 4) + ((stackPosition & 0xF00) >> 8));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, BinaryDependencies::DEP_STACK_ARG),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_12BIT,
                BinaryDependencies::DEP_STACK_ARG,
                0,
                true, 0, !isNativeBasePointer());
        } else
        {
            /*
             * STRH Rn [SP, #stackPosition]; (12-bit)
             */
            m_binary->appendUint8((0x0B << 4) + (stackPosition & 0x0F));
            m_binary->appendUint8((getGPEncoding(source.u.reg) << 4) + ((stackPosition & 0xF0) >> 4));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, BinaryDependencies::DEP_STACK_ARG),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_8BIT_4SPACE,
                BinaryDependencies::DEP_STACK_ARG,
                0,
                true, 0, !isNativeBasePointer());
        }
    } else
    {
        BinaryDependencies::DependencyType depType;
        if (isTempStack)
        {
            depType = BinaryDependencies::DEP_STACK_TEMP;
            stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
        }
        else
            depType = BinaryDependencies::DEP_STACK_LOCAL;
        stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size);

        if (size != 2)
        {
            /*
            * STR Rn [SP, #stackPosition]; (12-bit)
            */
            m_binary->appendUint8(stackPosition & 0xFF);
            m_binary->appendUint8((getGPEncoding(source.u.reg) << 4) + ((stackPosition & 0xF00) >> 8));
            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, depType),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_12BIT,
                depType,
                0,
                true, 0, !isNativeBasePointer());
        } else
        {
            /*
            * STR Rn [SP, #stackPosition]; (12-bit)
            */
            m_binary->appendUint8((0x0B << 4) + (stackPosition & 0xF));
            m_binary->appendUint8((getGPEncoding(source.u.reg) << 4) + ((stackPosition & 0xF0) >> 4));
            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, depType),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_8BIT_4SPACE,
                depType,
                0,
                true, 0, !isNativeBasePointer());
        }
    }

    switch(size)
    {
    case 1:
        m_binary->appendUint8(0xC0 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE5);

        break;
    case 2:
        m_binary->appendUint8(0xC0 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE1);

        break;
    case 4:
        m_binary->appendUint8(0x80 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE5);

        break;
    default:
        CHECK_FAIL();
    }
}

void ARMCompilerInterface::store32(StackLocation source,
                                   uint size,
                                   bool signExtend,
                                   const cString& dependencyName)
{
    int currentBlockID = m_binary->getCurrentBlockID();
    int newImmediateBlockId = m_binary->getCurrentDependecies().getExtraBlocks() + MethodBlock::BLOCK_EXTRA_DATA;

    MethodBlock* immBlock = new MethodBlock(newImmediateBlockId, *this);
    StackInterfacePtr pstack(immBlock);
    m_binary->createNewBlockWithoutChange(immBlock->getBlockID(), pstack);
    m_binary->changeBasicBlock(immBlock->getBlockID());

    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->getCurrentDependecies().addDependency(
        dependencyName,
        m_binary->getCurrentBlockData().getSize() - 4,
        BinaryDependencies::DEP_32BIT,
        BinaryDependencies::DEP_ABSOLUTE);

    immBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
    m_binary->changeBasicBlock(currentBlockID);

    StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

    /*
     * LDR rTemp, [PC, #offset];
     */
    m_binary->appendUint8(0xF8);
    m_binary->appendUint8((getGPEncoding(rTemp.u.reg) << 4) + 0x0F);
    m_binary->getCurrentDependecies().addExtraBlockDependency(
        MangledNames::getMangleBlock(newImmediateBlockId,
        2,
        BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 2,
        BinaryDependencies::DEP_12BIT,
        BinaryDependencies::DEP_RELATIVE,
        0,
        true);

    m_binary->appendUint8(0x9F);
    m_binary->appendUint8(0xE5);

    storeMemory(rTemp, source, 0, size);

    m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
}

void ARMCompilerInterface::move32(StackLocation destination,
                                    StackLocation source,
                                  uint size,
                                  bool signExtend)
{
}

void ARMCompilerInterface::load32(uint stackPosition,
                                  uint size,
                                  StackLocation destination,
                                  bool signExtend,
                                  bool argumentStackLocation,
                                  bool isTempStack)
{
    if (argumentStackLocation)
    {
        stackPosition += getFrameStackRef() + 4;

        /*
         * LDR Rd, [SP, #stackPosition]; (12bit)
         */
        if (size != 2)
        {
            m_binary->appendUint8(stackPosition & 0xFF);
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((stackPosition & 0xF00) >> 8));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, BinaryDependencies::DEP_STACK_ARG),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_12BIT,
                BinaryDependencies::DEP_STACK_ARG,
                0,
                true, 0, !isNativeBasePointer());
        } else
        {
            /*
             * LDRH Rd, [SP, #stackPosition]; (12bit)
             */
            m_binary->appendUint8((0x0B << 4) + (stackPosition & 0x0F));
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((stackPosition & 0xF0) >> 4));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, BinaryDependencies::DEP_STACK_ARG),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_8BIT_4SPACE,
                BinaryDependencies::DEP_STACK_ARG,
                0,
                true, 0, !isNativeBasePointer());
        }
    } else
    {
        BinaryDependencies::DependencyType depType;
        if (isTempStack)
        {
            depType = BinaryDependencies::DEP_STACK_TEMP;
            stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
        }
        else
            depType = BinaryDependencies::DEP_STACK_LOCAL;
        stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size);

        if (size != 2)
        {
            /*
             * LDR Rd, [SP, #stackPosition]; (12bit)
             */
            m_binary->appendUint8(stackPosition & 0xFF);
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((stackPosition & 0xF00) >> 8));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, depType),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_12BIT,
                depType,
                0,
                true, 0, !isNativeBasePointer());
        } else
        {
            /*
             * LDRH Rd, [SP, #stackPosition]; (12bit)
             */
            m_binary->appendUint8((0x0B << 4) + (stackPosition & 0x0F));
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((stackPosition & 0xF0) >> 4));

            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 2, depType),
                m_binary->getCurrentBlockData().getSize() - 2,
                BinaryDependencies::DEP_8BIT_4SPACE,
                depType,
                0,
                true, 0, !isNativeBasePointer());
        }
    }

    switch(size)
    {
    case 1:
        m_binary->appendUint8(0xD0 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE5);

        break;
    case 2:
        m_binary->appendUint8(0xD0 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE1);

        break;
    case 4:
        m_binary->appendUint8(0x90 + getBaseStackRegister(getMethodBaseStackRegister()));
        m_binary->appendUint8(0xE5);

        break;
    default:
        CHECK_FAIL();
    }
}

void ARMCompilerInterface::load32(StackLocation destination,
                                  uint size,
                                  bool signExtend,
                                  const cString& dependencyName)
{
    loadInt32(destination, dependencyName);

    loadMemory(destination, destination, 0, size, false);
}

void ARMCompilerInterface::load32addr(uint stackPosition,
                                      uint size,
                                      uint offset,
                                      StackLocation destination,
                                      bool argumentStackLocation,
                                      bool isTempStack)
{
    if ((stackPosition + offset) <= 255)
    {
        /*stackPosition *= -1;*/
        if (argumentStackLocation)
        {
            /*
             * ADD Rd, SP, #value (<=255)
             */
            stackPosition = getFrameStackRef() - offset + 4;
            m_binary->appendUint8(stackPosition);
            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, BinaryDependencies::DEP_STACK_ARG),
                m_binary->getCurrentBlockData().getSize() - 1,
                BinaryDependencies::DEP_8BIT,
                BinaryDependencies::DEP_STACK_ARG,
                0,
                true, 0, !isNativeBasePointer());
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
            m_binary->appendUint8((8 << 4) + getBaseStackRegister(getMethodBaseStackRegister()));
            m_binary->appendUint8(0xE2);
        } else
        {
            BinaryDependencies::DependencyType depType;
            stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size) + offset;
            if (isTempStack)
            {
                depType = BinaryDependencies::DEP_STACK_TEMP;
                stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
            }
            else
                depType = BinaryDependencies::DEP_STACK_LOCAL;
            /*
             * ADD Rd, SP, #value (<=255)
             */
            m_binary->appendUint8(stackPosition);
            m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, depType),
                m_binary->getCurrentBlockData().getSize() - 1,
                BinaryDependencies::DEP_8BIT,
                depType,
                0,
                true, 0, !isNativeBasePointer());
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
            m_binary->appendUint8((8 << 4) + getBaseStackRegister(getMethodBaseStackRegister()));
            m_binary->appendUint8(0xE2);
        }
    } else {
        // TODO
        CHECK_FAIL();
    }
    //load32(stackPosition + offset, size, destination, argumentStackLocation, isTempStack);
}

void ARMCompilerInterface::load32addr(StackLocation destination,
                                      const cString& dependencyName)
{
    loadInt32(destination, dependencyName);
}

// TODO: Move this function into xSTL
int ARMCompilerInterface::ntz(unsigned x)
{
    int n = 1;

    if (x == 0)
        return 32;

    if ((x & 0x0000FFFF) == 0)
    {
        n += 16;
        x >>= 16;
    }

    if ((x & 0x000000FF) == 0)
    {
        n += 8;
        x >>= 8;
    }

    if ((x & 0x0000000F) == 0)
    {
        n += 4;
        x >>= 4;
    }

    if ((x & 0x00000003) == 0)
    {
        n += 2;
        x >>= 2;
    }

    return n - (x & 1);
}

// TODO: Move this function into xSTL
int ARMCompilerInterface::nlz(unsigned x)
{
    int n = 1;

    if (x == 0)
        return 32;

    if ((x >> 16) == 0)
    {
        n += 16;
        x <<= 16;
    }

    if ((x >> 24) == 0)
    {
        n += 8;
        x <<= 8;
    }

    if ((x >> 28) == 0)
    {
        n += 4;
        x <<= 4;
    }

    if ((x >> 30) == 0)
    {
        n += 2;
        x <<= 2;
    }

    return n - (x >> 31);
}

void ARMCompilerInterface::loadInt32(StackLocation destination, uint32 value)
{
    int lead = nlz(value);
    int trail = ntz(value);
    trail -= trail & (~0xfffffffe);
    if ((32 - lead - trail) <= 8)
    {
        value >>= trail;
        // MOV Rd, #value;
        m_binary->appendUint8(value & 0xFF);
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((0x10 - (trail / 2)) & 0x0F));
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE3);
    } else
    {
        if ((32 - lead - trail) <= 16)
        {
            value >>= trail;
            // MOV Rd, (value & 0xFF00)
            m_binary->appendUint8(value >> 8);
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + (0x0C - ((trail / 2)) & 0x0F));
            m_binary->appendUint8(0xA0);
            m_binary->appendUint8(0xE3);

            // ORR Rd, (value & 0xFF)
            m_binary->appendUint8(value & 0xFF);
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + ((0xa0 - ((trail / 2)) & 0x0F)) & 0xFF);
            m_binary->appendUint8(0x80 + getGPEncoding(destination.u.reg));
            m_binary->appendUint8(0xE3);
        } else
        {
            int currentBlockID = m_binary->getCurrentBlockID();
            int newImmediateBlockId = m_binary->getCurrentDependecies().getExtraBlocks() + MethodBlock::BLOCK_EXTRA_DATA;

            MethodBlock* immBlock = new MethodBlock(newImmediateBlockId, *this);
            StackInterfacePtr pstack(immBlock);
            m_binary->createNewBlockWithoutChange(immBlock->getBlockID(), pstack);
            m_binary->changeBasicBlock(immBlock->getBlockID());

            m_binary->appendBuffer((uint8 *)&value, sizeof(value));
            immBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
            m_binary->changeBasicBlock(currentBlockID);

            uint stackPosition = -8 + getFrameStackRef();
            /*
             * LDR Rd, =value
             */
            m_binary->appendUint8(stackPosition); // TODO: The dependency should actually be 12 bit
            // Add dependency to the last byte
            m_binary->getCurrentDependecies().addExtraBlockDependency(
                MangledNames::getMangleBlock(newImmediateBlockId,
                1,
                BinaryDependencies::DEP_RELATIVE),
                m_binary->getCurrentBlockData().getSize() - 1,
                BinaryDependencies::DEP_12BIT,
                BinaryDependencies::DEP_RELATIVE,
                0,
                true);
            m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + 0x0F);
            m_binary->appendUint8(0x9F);
            m_binary->appendUint8(0xE5);
        }
    }
}


void ARMCompilerInterface::loadInt32(StackLocation destination,
                                     const cString& dependancyName)
{
    int currentBlockID = m_binary->getCurrentBlockID();
    int newImmediateBlockId = m_binary->getCurrentDependecies().getExtraBlocks() + MethodBlock::BLOCK_EXTRA_DATA;

    MethodBlock* immBlock = new MethodBlock(newImmediateBlockId, *this);
    StackInterfacePtr pstack(immBlock);
    m_binary->createNewBlockWithoutChange(immBlock->getBlockID(), pstack);
    m_binary->changeBasicBlock(immBlock->getBlockID());

    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->getCurrentDependecies().addDependency(
        dependancyName,
        m_binary->getCurrentBlockData().getSize() - 4,
        BinaryDependencies::DEP_32BIT,
        BinaryDependencies::DEP_ABSOLUTE);

    immBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
    m_binary->changeBasicBlock(currentBlockID);

    /*
     * LDR Rd, [PC, #offset];
     */
    m_binary->appendUint8(0xF8);
    m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + 0x0F);
    m_binary->getCurrentDependecies().addExtraBlockDependency(
        MangledNames::getMangleBlock(newImmediateBlockId,
        2,
        BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 2,
        BinaryDependencies::DEP_12BIT,
        BinaryDependencies::DEP_RELATIVE,
        0,
        true);
    m_binary->appendUint8(0x9F);
    m_binary->appendUint8(0xE5);
}


void ARMCompilerInterface::assignRet32(StackLocation source)
{
    if (getGPEncoding(source.u.reg) != ARM_GP32_R0)
    {
        /*
         * MOV R0, Rn;
         */
        m_binary->appendUint8(getGPEncoding(source.u.reg));
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE1);
    }
}

void ARMCompilerInterface::pushArg32(StackLocation source)
{
    StackLocation rTemp = source;
    if (getGPEncoding(source.u.reg) == ARM_GP32_SP)
    {
        rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

        /*
         * MOV rTemp, source;
         */
        m_binary->appendUint8(getGPEncoding(source.u.reg));
        m_binary->appendUint8((getGPEncoding(rTemp.u.reg) << 4));
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE1);
    }

    /*
     * PUSH {Rn};
     */
    m_binary->appendUint8(0x04);
    m_binary->appendUint8(getGPEncoding(rTemp.u.reg) << 4);
    m_binary->appendUint8(0x20 + ARM_GP32_SP);
    m_binary->appendUint8(0xE5);

    if (getGPEncoding(source.u.reg) == ARM_GP32_SP)
    {
        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
    m_stackRef += 4;
}

void ARMCompilerInterface::popArg32(StackLocation source)
{
    StackLocation rTemp = source;
    if (getGPEncoding(source.u.reg) == ARM_GP32_SP)
    {
        rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
    }

    /*
     * POP {rTemp};
     */
    m_binary->appendUint8(0x04);
    m_binary->appendUint8(getGPEncoding(rTemp.u.reg) << 4);
    m_binary->appendUint8(0x90 + ARM_GP32_SP);
    m_binary->appendUint8(0xE4);

    if (getGPEncoding(source.u.reg) == ARM_GP32_SP)
    {
        /*
         * MOV source, rTemp;
         */
        m_binary->appendUint8(getGPEncoding(rTemp.u.reg));
        m_binary->appendUint8((getGPEncoding(source.u.reg) << 4));
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE1);

        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
}

void ARMCompilerInterface::call(const cString& dependancyName, uint)
{
    saveVolatileRegisters();
    /*
     * BL    0xFFFF
     */
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xFF);
    // Add dependency to the last 3 bytes
    m_binary->getCurrentDependecies().addDependency(
        dependancyName,
        m_binary->getCurrentBlockData().getSize() - 3,
        BinaryDependencies::DEP_24BIT,
        BinaryDependencies::DEP_RELATIVE,
        2,
        true);
    m_binary->appendUint8(0xEB);
}

void ARMCompilerInterface::call(StackLocation address, uint)
{
    saveVolatileRegisters();

    // MOV LR, PC
    m_binary->appendUint8(0x0F);
    m_binary->appendUint8(0xE0);
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);

    // BX  R
    m_binary->appendUint8((0x1 << 4) + getGPEncoding(address.u.reg));
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0x2F);
    m_binary->appendUint8(0xE1);
}

void ARMCompilerInterface::call32(const cString& dependancyName,
                                  StackLocation destination, uint numberOfArguments)
{
    if (getGPEncoding(destination.u.reg) != ARM_GP32_R0)
    {
        // Free R0 from stack
        freeRegister32(ARM_GP32_R0);
    }

    saveVolatileRegisters(getGPEncoding(ARM_GP32_R0), getGPEncoding(destination.u.reg));
    /*
     * BL    0xFFFF
     */
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xFF);
    // Add dependency to the last 3 bytes
    m_binary->getCurrentDependecies().addDependency(
        dependancyName,
        m_binary->getCurrentBlockData().getSize() - 3,
        BinaryDependencies::DEP_24BIT,
        BinaryDependencies::DEP_RELATIVE,
        2,
        true);
    m_binary->appendUint8(0xEB);

    if (getGPEncoding(destination.u.reg) != ARM_GP32_R0)
    {
        /*
         * MOV Rn, R0;
         */
        m_binary->appendUint8(ARM_GP32_R0);
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE1);
    }
}

void ARMCompilerInterface::call32(StackLocation address, StackLocation destination, uint numberOfArguments)
{
    if (getGPEncoding(destination.u.reg) != ARM_GP32_R0)
    {
        // Free R0 from stack
        freeRegister32(ARM_GP32_R0);
    }

    saveVolatileRegisters(ARM_GP32_R0, getGPEncoding(destination.u.reg));
    // MOV LR, PC
    m_binary->appendUint8(0x0F);
    m_binary->appendUint8(0xE0);
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);

    // BX  R
    m_binary->appendUint8((0x1 << 4) + getGPEncoding(address.u.reg));
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0x2F);
    m_binary->appendUint8(0xE1);

    if (getGPEncoding(destination.u.reg) != ARM_GP32_R0)
    {
        /*
         * MOV Rn, R0;
         */
        m_binary->appendUint8(ARM_GP32_R0);
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xE1);
    }
}

void ARMCompilerInterface::storeMemory(StackLocation destination,
                                       StackLocation value,
                                       uint offset,
                                       uint size)
{
    if (offset != 0)
    {
        addConst32(value, offset);
    }

    switch(size)
    {
    case 1:
        /*
         * STRB Rn, [Rd];
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
        m_binary->appendUint8(0xC0 + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0xE5);

        break;
    case 2:
        /*
         * STRH Rn, [Rd];
         */
        m_binary->appendUint8((0xB << 4));
        m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
        m_binary->appendUint8(0xC0 + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0xE1);

        break;
    case 4:
        /*
         * STR Rn, [Rd];
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
        m_binary->appendUint8(0x80 + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0xE5);

        break;
    default:
        CHECK_FAIL();
    }
}

void ARMCompilerInterface::loadMemory(StackLocation destination,
                                      StackLocation value,
                                      uint offset,
                                      uint size,
                                      bool signExtend)
{
    if (offset != 0)
    {
        addConst32(value, offset);
    }

    switch(size)
    {
        case 1:
            /*
             * LDRB Rn, [Rd];
             */
            m_binary->appendUint8(0x00);
            m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
            m_binary->appendUint8(0xD0 + getGPEncoding(destination.u.reg));
            m_binary->appendUint8(0xE5);

            break;
        case 2:
            /*
             * LDH Rn, [Rd];
             */
            m_binary->appendUint8(0xB0);
            m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
            m_binary->appendUint8(0xD0 + getGPEncoding(destination.u.reg));
            m_binary->appendUint8(0xE1);

            break;
        case 4:
            /*
             * LDR Rn, [Rd];
             */
            m_binary->appendUint8(0x00);
            m_binary->appendUint8(getGPEncoding(value.u.reg) << 4);
            m_binary->appendUint8(0x90 + getGPEncoding(destination.u.reg));
            m_binary->appendUint8(0xE5);

            break;
        default:
            CHECK_FAIL();
    }
}

void ARMCompilerInterface::conv32(StackLocation destination,
                                  uint size,
                                  bool signExtend)
{
    switch (size)
    {
    case 1:
        /*
         * AND Rd, Rd, #0xFF;
         */
        m_binary->appendUint8(0xFF);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0xE2);
        break;
    case 2:
        {
            /*
             * LDR R1, =0xFFFF;
             * AND R0, R0, R1;
             */
            StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
            loadInt32(rTemp, 0xFFFF);
            and32(destination, rTemp);
            m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
        }
        break;
    case 4:
        // Nothing to do
        break;
    default:
        CHECK_FAIL();
    }
}

void ARMCompilerInterface::neg32(StackLocation destination)
{
    // Convert to 2's complement
    not32(destination);

    /*
     * ADD Rd, #1;
     */
    m_binary->appendUint8(0x01);
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8((8 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE2);
}

void ARMCompilerInterface::not32(StackLocation destination)
{
    /*
     * MVN Rd, Rd;
     */
    m_binary->appendUint8(getGPEncoding(destination.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8(0xE0);
    m_binary->appendUint8(0xE1);
}

void ARMCompilerInterface::add32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * ADD Rd, Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8((8 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::sub32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * SUB Rd, Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8((4 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::mul32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * MUL Rd, Rn, Rd;
     */
    m_binary->appendUint8((0x9 << 4) + getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::div32(StackLocation destination,
                                 StackLocation source,
                                 bool sign)
{
    pushArg32(source);
    pushArg32(destination);
    if (sign)
        call32(::CallingConvention::serializedMethod(m_framework.getBin32Div()), destination, 2);
    else
        call32(::CallingConvention::serializedMethod(m_framework.getBin32uDiv()), destination, 2);
    revertStack(8);
}

void ARMCompilerInterface::rem32(StackLocation destination,
                                 StackLocation source,
                                 bool sign)
{
    pushArg32(source);
    pushArg32(destination);
    if (sign)
        call32(::CallingConvention::serializedMethod(m_framework.getBin32Rem()), destination, 2);
    else
        call32(::CallingConvention::serializedMethod(m_framework.getBin32uRem()), destination, 2);
    revertStack(8);
}

void ARMCompilerInterface::and32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * AND Rd, Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8(getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::xor32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * EOR Rd, Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8((2 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::shr32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * MOV Rd, Rd, LSR Rn;
     */
    m_binary->appendUint8((3 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + getGPEncoding(source.u.reg));
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);
}

void ARMCompilerInterface::shl32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * MOV Rd, Rd, LSL Rn;
     */
    m_binary->appendUint8((1 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4) + getGPEncoding(source.u.reg));
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);
}

void ARMCompilerInterface::or32(StackLocation destination,
                                StackLocation source)
{
    /*
     * ORR Rd, Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8((8 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE1);
}

void ARMCompilerInterface::addConst32(const StackLocation destination,
                                      int32 value)
{
    if (value == 0)
        return;

    if (value <= 255)
    {
        /*
         * ADD Rd, Rd, #value (<=255)
         */
        m_binary->appendUint8(value);
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
        m_binary->appendUint8((8 << 4) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0xE2);
    } else {
        StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
        StackLocation rDest = m_binary->getCurrentStack()->buildStackLocation(destination.u.reg, 0);
        loadInt32(rTemp, value);
        add32(rDest, rTemp);
        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
}

void ARMCompilerInterface::jump(int blockID)
{
    jumpShort(blockID);
}

void ARMCompilerInterface::jumpShort(int blockID)
{
    /*
     * B    blockID (24-bit)
     */
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xFF);
    //// Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(
        MangledNames::getMangleBlock(blockID, 3, BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 3,
        BinaryDependencies::DEP_24BIT,
        BinaryDependencies::DEP_RELATIVE,
        2,
        true);
    m_binary->appendUint8(0xEA);
}

void ARMCompilerInterface::jumpCond(StackLocation compare,
                                    int blockID,
                                    bool isZero)
{
    jumpCondShort(compare, blockID, isZero);
}

void ARMCompilerInterface::jumpCondShort(StackLocation compare,
                                         int blockID,
                                         bool isZero)
{
    /*
     * CMP    Rn, 0;
     */
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8((5 << 4) + getGPEncoding(compare.u.reg));
    m_binary->appendUint8(0xE3);

    /*
     * The address part of BEQ/BNE
     */
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xFF);
    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(MangledNames::getMangleBlock(blockID, 3, BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 3,
        BinaryDependencies::DEP_24BIT,
        BinaryDependencies::DEP_RELATIVE,
        2,
        true);
    if (isZero)
    {
        /*
         * BEQ    blockID
         */
        m_binary->appendUint8(0x0A);
    } else
    {
        /*
         * BNE    blockID
         */
        m_binary->appendUint8(0x1A);
    }
}

void ARMCompilerInterface::ceq32(StackLocation destination,
                                 StackLocation source)
{
    /*
     * CMP Rd, Rn
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(0x00);
    m_binary->appendUint8((5 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE1);

    /*
     * MOVEQ Rd, #1;
     */
    m_binary->appendUint8(0x01);
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0x03);

    /*
     * MOVNE Rd, #0;
     */
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0x13);

}

void ARMCompilerInterface::cgt32(StackLocation destination,
                                 StackLocation source,
                                 bool isSigned)
{
    /*
     * CMP Rn, Rd
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x50 + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE1);

    if (isSigned)
    {
        /*
         * MOVGT Rd, #1;
         */
        m_binary->appendUint8(0x01);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xC3);

        /*
         * MOVLE Rd, #0;
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xD3);
    } else
    {
        /*
         * MOVHI Rd, #1;
         */
        m_binary->appendUint8(0x01);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0x83);

        /*
         * MOVLS Rd, #0;
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0x93);
    }
}

void ARMCompilerInterface::clt32(StackLocation destination,
                                 StackLocation source,
                                 bool isSigned)
{
    /*
     * CMP Rn, Rd
     */
    m_binary->appendUint8(getGPEncoding(source.u.reg));
    m_binary->appendUint8(0x00);
    m_binary->appendUint8((5 << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE1);

    if (isSigned)
    {
        /*
         * MOVLT Rd, #1;
         */
        m_binary->appendUint8(0x01);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xB3);

        /*
         * MOVGE Rd, #0;
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0xA3);
    } else
    {
        /*
         * MOVLO Rd, #1;
         */
        m_binary->appendUint8(0x01);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0x33);

        /*
         * MOVHS Rd, #0;
         */
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(getGPEncoding(destination.u.reg) << 4);
        m_binary->appendUint8(0xA0);
        m_binary->appendUint8(0x23);
    }
}

void ARMCompilerInterface::localloc(StackLocation destination,
                                    StackLocation size,
                                    bool isStackEmpty)
{
    sub32(getMethodBaseStackRegister(), size);

    /*
     * MOV Rd, Rn; (MOV Rd, SP)
     */
    m_binary->appendUint8(getGPEncoding(getMethodBaseStackRegister().u.reg));
    m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);

}

void ARMCompilerInterface::generateMethodEpiProLogs(bool bForceSaveNonVolatiles /* = false */)
{
    // Check for allocated registers, and debug!

    // Add prolog
    // Generate new first stream, and switch to it.
    MethodBlock* prolog = new MethodBlock(MethodBlock::BLOCK_PROLOG, *this);
    StackInterfacePtr estack(prolog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_PROLOG, estack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_PROLOG);

    pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_LR), 0));
    m_stackRef -= 4;
    // Revert stackRef since this push is not a part of the function logic

    saveNonVolatileRegisters();

    /*
     * SUB SP, SP, #stackSize;
     */
    if (m_binary->getStackSize() != 0)
    {
        int x = m_binary->getStackSize();

        m_binary->appendUint8(m_binary->getStackSize());
        m_binary->appendUint8(0xD0);
        m_binary->appendUint8(0x4D);
        m_binary->appendUint8(0xE2);
    }
    m_stackRef += m_binary->getStackSize() - StackInterface::LOCAL_STACK_START_VALUE + 4;

    prolog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);

    // Conclude the method by adding an epilog
    // Generate last stream, and switch to it.
    MethodBlock* epilog = new MethodBlock(MethodBlock::BLOCK_RET, *this);
    StackInterfacePtr pstack(epilog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_RET, pstack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_RET);

    revertStack(m_binary->getStackSize());

    saveNonVolatileRegisters(false);
    popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_LR), 0));

    /*
     * BX LR;
     */
    m_binary->appendUint8(0x1E);
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0x2F);
    m_binary->appendUint8(0xE1);

    m_binary->setNonVolatilesSize(m_nonVolSize);

    epilog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

void ARMCompilerInterface::revertStack(uint32 size)
{
    if (size > 0)
    {
        m_stackRef -= size;
        /*
         * ADD SP, SP, #stackSize;
         */
        m_binary->appendUint8(size);
        m_binary->appendUint8(ARM_GP32_SP << 4);
        m_binary->appendUint8((0x8 << 4) + ARM_GP32_SP);
        m_binary->appendUint8(0xE2);
    }
}

void ARMCompilerInterface::saveNonVolatileRegisters(bool shouldPush)
{
    const RegisterAllocationTable& touched = m_binary->getTouchedRegisters();
    // NOTE: The order between push and pop should be reverse!!!
    if (shouldPush)
    {
        // These pushes are not a part of the function logic,
        // hence the m_stackRef should be decreased by 4 (pushArg32 increases it)
        if (touched.hasKey(getGPEncoding(ARM_GP32_R5)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R5), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R6)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R6), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R7)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R7), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R8)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R8), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R9)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R9), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R10)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R10), 0));
            m_stackRef -= 4;

            m_nonVolSize += 4;
        }
    } else
    {
        if (touched.hasKey(getGPEncoding(ARM_GP32_R10)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R10), 0));
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R9)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R9), 0));
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R8)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R8), 0));
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R7)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R7), 0));
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R6)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R6), 0));
        }

        if (touched.hasKey(getGPEncoding(ARM_GP32_R5)))
        {
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(ARM_GP32_R5), 0));
        }
    }
}

void ARMCompilerInterface::saveVolatileRegisters(int saveRegister1,
                                                 int saveRegister2,
                                                 int saveRegister3,
                                                 int saveRegister4,
                                                 int saveRegister5)
{
    // Examine all known registers
    cList<int> regs = m_archRegisters.keys();
    for (cList<int>::iterator i = regs.begin(); i != regs.end(); i++)
    {
        // Is this register volatile?
        if (m_archRegisters[*i].m_eType != Volatile)
            continue;

        // Do we skip this register?
        if ((*i == saveRegister1) || (*i == saveRegister2) || (*i == saveRegister3))
            continue;

        // If it's used, replace it with a stack temporary
        freeRegister32(getGPEncoding(*i));
    }
}

bool ARMCompilerInterface::isNativeBasePointer() const
{
    return getMethodBaseStackRegister() == getStackPointer();
}

void ARMCompilerInterface::setFramePointer(StackLocation destination)
{
    ASSERT(destination != getMethodBaseStackRegister());

    mov(destination, getMethodBaseStackRegister());

    // If it's SP, then take stackRef into account
    if (getMethodBaseStackRegister() == getStackPointer())
    {
        if (m_stackRef != 0)
        {
            // SUB SP, #stackRef
            m_binary->appendUint8(m_stackRef);
            m_binary->appendUint8(0xD0);
            m_binary->appendUint8(0x4D);
            m_binary->appendUint8(0xE2);
        }
    }
}

uint ARMCompilerInterface::getFrameStackRef()
{
    if (getMethodBaseStackRegister() == getStackPointer())
    {
        return m_stackRef;
    }
    return 0;
}

void ARMCompilerInterface::resetBaseStackRegister(const StackLocation& targetRegister)
{
    if (getMethodBaseStackRegister() == targetRegister)
        return;

    /*
     * MOV Rd, Rn;
     */
    m_binary->appendUint8(getGPEncoding(getMethodBaseStackRegister().u.reg));
    m_binary->appendUint8((getGPEncoding(targetRegister.u.reg) << 4));
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);

    // And forget that we ever used a different register
    setMethodBaseStackRegister(getStackPointer());
}

void ARMCompilerInterface::adc32(StackLocation destination, StackLocation source)
{
    /*
     * ADC Rd, Rn;
     */
    m_binary->appendUint8((getGPEncoding(source.u.reg)));
    m_binary->appendUint8((getGPEncoding(destination.u.reg) << 4));
    m_binary->appendUint8((0xA << 4) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0xE0);
}

void ARMCompilerInterface::sbb32(StackLocation destination, StackLocation source)
{
    /*
     * SBB Rd, Rn;
     */
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
}

void ARMCompilerInterface::mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign)
{

}

void ARMCompilerInterface::mov(StackLocation destination, StackLocation source)
{
    int dst = getGPEncoding(destination.u.reg);
    int src = getGPEncoding(source.u.reg);

    m_binary->appendUint8(src);
    m_binary->appendUint8(dst << 4);
    m_binary->appendUint8(0xA0);
    m_binary->appendUint8(0xE1);
}
