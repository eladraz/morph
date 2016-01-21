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
#include "compiler/processors/arm/THUMBCompilerInterface.h"
#include "compiler/StackEntity.h"

/*
 * RAZI REMARKS:
 *
 * 1. I see issues with the stackPosition where there is a shift of 2 bits. Since the value we put is 8 bits. It's effectivly limts the stack positions to 8 bits (256 bytes).
 *    In the actual opcode we should shift the stackPosition in advanced
 * 2. While debugging I see that there is 4 bytes offset in the stack (e.g. var0 is actually => var4
 */

/////////////////////////////////////////////////////////////////////////////////
// Instruction set assembler

// Adding 16bit little endina to binary
#define appendUint16(u16) { m_binary->appendUint8(u16 & 0xFF); m_binary->appendUint8((u16 >> 8) & 0xFF);}
// Adding 16bit little endina to binary
#define appendUint32(u32) { m_binary->appendUint8(u32 & 0xFF); m_binary->appendUint8((u32 >> 8) & 0xFF); m_binary->appendUint8((u32 >> 16) & 0xFF); m_binary->appendUint8((u32 >> 24) & 0xFF);}


/*
 * Format2 ADD/SUB, Section 5.2 THUMB Instruction Set manaual
 *    immediate = 0/1, indiacte whether rn is a register or 3 bit immediate
 *    opcode_isSubOrAdd - 1 for sub, 0 for add
 */
#define FORMAT2_OP_ADD (0)
#define FORMAT2_OP_SUB (1)
#define format2(immediate, opcode_isSubOrAdd, rn_immediate3, rs, rd) { uint16 operand = \
            (3 << 11) | (immediate << 10) | (opcode_isSubOrAdd << 9) | (rn_immediate3 << 6) | (rs << 3) | rd;\
            appendUint16(operand);    \
            };

/*
 * Format3 MOV/CMP/ADD/SUB immediate, Section 5.3 THUMB Instruction Set manaual
 *    opcode - 0 - MOV
 *             1 - CMP
 *             2 - ADD
 *             3 - SUB
 */
#define FORMAT3_OP_MOV (0)
#define FORMAT3_OP_CMP (1)
#define FORMAT3_OP_ADD (2)
#define FORMAT3_OP_SUB (3)
#define format3(opcode, rd, immediate) { uint16 operand = \
            (1 << 13) | (opcode << 11) | (rd << 8) | (immediate);\
            appendUint16(operand);    \
            };
// ADDS/SUBS Rbase, #stackPosition
#define fixBasePointer(format3opcode, depType, stackPosition) \
            format3(format3opcode, getGPEncoding(getMethodBaseStackRegister().u.reg), ((uint)stackPosition & 0xFF));                \
            m_binary->getCurrentDependecies().addDependency(                                                    \
                                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, depType),        \
                                m_binary->getCurrentBlockData().getSize() - 2,                                  \
                                BinaryDependencies::DEP_8BIT,                                                   \
                                depType,                                                                        \
                                0,                                                                              \
                                true, 0, !isNativeBasePointer());                                               \

/*
 * Section 5.6 - PC relative load
 * LDR Rd, [PC + 10bit]
 */
#define format6(rd, immediate) { uint16 operand = \
            (9 << 11) | (rd << 8) | (immediate);\
            appendUint16(operand);    \
            };

#define loadBlock(Rd, newImmediateBlockId)                                      \
    format6(Rd, 0xFF);                                                          \
    m_binary->getCurrentDependecies().addExtraBlockDependency(                  \
                    MangledNames::getMangleBlock(newImmediateBlockId,           \
                    1,                                                          \
                    BinaryDependencies::DEP_RELATIVE),                          \
                    m_binary->getCurrentBlockData().getSize() - 2,              \
                    BinaryDependencies::DEP_8BIT,                               \
                    BinaryDependencies::DEP_RELATIVE,                           \
                    2,                                                          \
                    true);


/*
 * Format12 - Load address
 *
 * SP - 1 for SP as base, 0 for PC as base
 * offsetShift2 - Is a 8 bit value which indiate a number between 0..2^10 (2 lower buts will be added as 0)
 */
#define format12(sp, rd, offsetShift2) { uint16 operand =               \
            (0xA000) | (sp << 11) | (rd << 8) | ((uint)offsetShift2 & 0xFF);  \
            appendUint16(operand);                                      \
            };

// ADDS Rd, SP, #stackPosition
#define loadAddr(depType, rd, stackPosition) \
            format12(1, rd, stackPosition);                                                                     \
            m_binary->getCurrentDependecies().addDependency(                                                    \
                                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, depType),        \
                                m_binary->getCurrentBlockData().getSize() - 2,                                  \
                                BinaryDependencies::DEP_8BIT,                                                   \
                                depType,                                                                        \
                                2,                                                                              \
                                true, 0, !isNativeBasePointer());                                               \


/////////////////////////////////////////////////////////////////////////////////

char* gTHUMBRegisters[16] = {
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

THUMBCompilerInterface::THUMBCompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params) :
    CompilerInterface(framework, params)
{
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R0), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R1), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R2), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R3), RegisterEntry(Volatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R4), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R5), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R6), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_R7), RegisterEntry(NonVolatile));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_SP), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_LR), RegisterEntry(Platform, true));
    m_archRegisters.append(getGPEncoding(THUMB_GP32_PC), RegisterEntry(Platform, true));
    //setBaseStackRegister(getStackPointer());
    m_nonVolSize = 0;
    m_stackRef = 0;
    m_binary = FirstPassBinaryPtr(new FirstPassBinary(
                                      OpcodeSubsystems::DISASSEMBLER_THUMB_LE,
                                      false));
}

uint8 THUMBCompilerInterface::getBaseStackRegister(StackLocation baseRegister)
{
    if (baseRegister == StackInterface::NO_MEMORY)
        return THUMB_GP32_SP;
    ASSERT(baseRegister.u.flags == 0);
    return getGPEncoding(baseRegister.u.reg);
}


void THUMBCompilerInterface::setLocalsSize(uint localStackSize)
{
    m_binary->setStackBaseSize(localStackSize);
}

void THUMBCompilerInterface::setArgumentsSize(uint argsSize)
{
    m_binary->setArgumentsSize(argsSize);
}

THUMBCompilerInterface::StackSize THUMBCompilerInterface::getStackSize() const
{
    return STACK_32;
}

uint THUMBCompilerInterface::getShortJumpLength() const
{
    // B<cond> is 8 bit.
    return 256;
}

StackLocation THUMBCompilerInterface::getStackPointer() const
{
    return StackInterface::buildStackLocation(getGPEncoding(THUMB_GP32_SP), 0);
}

const char* THUMBCompilerInterface::getRegister32(StackLocation destination)
{
    return gTHUMBRegisters[getGPEncoding(destination.u.reg)];
}

int THUMBCompilerInterface::getGPEncoding(int gpreg)
{
    return -(gpreg + GP32_BASE);
}

void THUMBCompilerInterface::freeRegister32(int gpreg)
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

void THUMBCompilerInterface::storeConst(uint stackPosition,
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

void THUMBCompilerInterface::store32(uint stackPosition,
                                     uint size,
                                     StackLocation source,
                                     bool argumentStackLocation,
                                     bool isTempStack)
{
    BinaryDependencies::DependencyType depType = BinaryDependencies::DEP_STACK_LOCAL;
    if (isTempStack)
    {
        depType = BinaryDependencies::DEP_STACK_TEMP;
        stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
    }


    if (argumentStackLocation)
    {
        ASSERT(!isTempStack);
        stackPosition += getFrameStackRef() + 4;
        depType = BinaryDependencies::DEP_STACK_ARG;
    } else
    {
        stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size);
    }

        if (getBaseStackRegister(getMethodBaseStackRegister()) == THUMB_GP32_SP)
    {
        // STR [SP, #stackPosition]
        m_binary->appendUint8(stackPosition);
        m_binary->getCurrentDependecies().addDependency(
                MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, depType),
                m_binary->getCurrentBlockData().getSize() - 1,
                BinaryDependencies::DEP_8BIT,
                depType,
                2,
                true, 0, !isNativeBasePointer());
        m_binary->appendUint8(0x90 + getGPEncoding(source.u.reg));
    } else
    {
        // ADDS Rbase, #stackPosition
        fixBasePointer(FORMAT3_OP_ADD, depType, stackPosition);

        storeMemory(getMethodBaseStackRegister(), source, 0, size);

        // SUBS Rbase, #stackPosition
        fixBasePointer(FORMAT3_OP_SUB, depType, stackPosition);
    }
}

void THUMBCompilerInterface::store32(StackLocation source,
                                   uint size,
                                   bool signExtend,
                                   const cString& dependencyName)
{
    MethodBlock* immBlock;
    int newImmediateBlockId;
    int currentBlockID = createExtraBlock(newImmediateBlockId, immBlock);

    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->getCurrentDependecies().addDependency(
                    dependencyName,
                    m_binary->getCurrentBlockData().getSize() - 4,
                    BinaryDependencies::DEP_32BIT,
                    BinaryDependencies::DEP_ABSOLUTE);

    exitExtraBlock(currentBlockID, immBlock);

    StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

    // LDR Rd, =value;
    loadBlock(getGPEncoding(rTemp.u.reg), newImmediateBlockId);

    storeMemory(rTemp, source, 0, size);

    m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
}

void THUMBCompilerInterface::move32(StackLocation destination,
                                      StackLocation source,
                                    uint size,
                                    bool signExtend)
{
    // TODO!!!
    CHECK_FAIL();
}

void THUMBCompilerInterface::load32(uint stackPosition,
                                  uint size,
                                  StackLocation destination,
                                  bool signExtend,
                                  bool argumentStackLocation,
                                  bool isTempStack)
{
    BinaryDependencies::DependencyType depType = BinaryDependencies::DEP_STACK_LOCAL;
    if (isTempStack)
    {
        depType = BinaryDependencies::DEP_STACK_TEMP;
        stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
    }

    if (argumentStackLocation)
    {
        ASSERT(!isTempStack);
        stackPosition += getFrameStackRef() + 4;
        depType = BinaryDependencies::DEP_STACK_ARG;
    } else
    {
        stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size);
    }

        if (getBaseStackRegister(getMethodBaseStackRegister()) == THUMB_GP32_SP)
    {
        // TODO! Format 11
        // LDR [SP, #stackPosition]
        m_binary->appendUint8(stackPosition);
        m_binary->getCurrentDependecies().addDependency(
            MangledNames::getMangleBlock(m_binary->getCurrentBlockID(), 1, depType),
            m_binary->getCurrentBlockData().getSize() - 1,
            BinaryDependencies::DEP_8BIT,
            depType,
            2,
            true, 0, !isNativeBasePointer());
        m_binary->appendUint8(0x98 + getGPEncoding(destination.u.reg));
    } else
    {
        // ADDS Rbase, #stackPosition
        fixBasePointer(FORMAT3_OP_ADD, depType, stackPosition);

            loadMemory(getMethodBaseStackRegister(), destination, 0, size);

        // When we commited LDR R0,[R0], there is no point to fix back the stack
        if (destination != getMethodBaseStackRegister())
        {
            // SUBS Rbase, #stackPosition
            fixBasePointer(FORMAT3_OP_SUB, depType, stackPosition);
        }
    }
}

void THUMBCompilerInterface::load32(StackLocation destination,
                                    uint size,
                                    bool signExtend,
                                    const cString& dependencyName)
{
    loadInt32(destination, dependencyName);
    loadMemory(destination, destination, 0, size, false);
}

void THUMBCompilerInterface::load32addr(uint stackPosition,
                                      uint size,
                                      uint offset,
                                      StackLocation destination,
                                      bool argumentStackLocation,
                                      bool isTempStack)
{
    BinaryDependencies::DependencyType depType = BinaryDependencies::DEP_STACK_LOCAL;
    if (isTempStack)
    {
        depType = BinaryDependencies::DEP_STACK_TEMP;
        stackPosition -= StackInterface::LOCAL_STACK_START_VALUE;
    }

    if (argumentStackLocation)
    {
        ASSERT(!isTempStack);
        stackPosition += getFrameStackRef() + 4;
        depType = BinaryDependencies::DEP_STACK_ARG;
    } else
    {
        stackPosition = getFrameStackRef() - stackPosition - Alignment::alignUpToDword(size);
    }

        if (getBaseStackRegister(getMethodBaseStackRegister()) == THUMB_GP32_SP)
    {
        // MOV Rd, SP, #stackPosition
        loadAddr(depType, getGPEncoding(destination.u.reg), stackPosition + offset);
    } else
    {
        // ADDS Rbase, #stackPosition
        fixBasePointer(FORMAT3_OP_ADD, depType, stackPosition + offset);

        mov(destination, getMethodBaseStackRegister());

        // SUBS Rbase, #stackPosition
        fixBasePointer(FORMAT3_OP_SUB, depType, stackPosition + offset);
    }
}

void THUMBCompilerInterface::load32addr(StackLocation destination,
                                        const cString& dependencyName)
{
    loadInt32(destination, dependencyName);
}

int THUMBCompilerInterface::createExtraBlock(int& newImmediateBlockId, MethodBlock*& immBlock)
{
    int currentBlockID = m_binary->getCurrentBlockID();
    newImmediateBlockId = m_binary->getCurrentDependecies().getExtraBlocks() + MethodBlock::BLOCK_EXTRA_DATA;
    immBlock = new MethodBlock(newImmediateBlockId, *this);

    StackInterfacePtr pstack(immBlock);
    m_binary->createNewBlockWithoutChange(immBlock->getBlockID(), pstack);
    m_binary->changeBasicBlock(immBlock->getBlockID());

    return currentBlockID;
}

void THUMBCompilerInterface::exitExtraBlock(int oldValue, MethodBlock* immBlock)
{
    immBlock->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
    m_binary->changeBasicBlock(oldValue);
}

void THUMBCompilerInterface::loadInt32(StackLocation destination, uint32 value)
{
    int lead = nlz(value);
    int trail = ntz(value);

    if (value <= 255)
    {
        // MOVS Rd, #value
        m_binary->appendUint8(value);
        m_binary->appendUint8(0x20 + getGPEncoding(destination.u.reg));
    } else {
        if ((32 - lead - trail) <= 8)
        {
            // MOVS Rd, #value
            m_binary->appendUint8(value >> trail);
            m_binary->appendUint8(0x20 + getGPEncoding(destination.u.reg));

            if (trail != 0)
            {
                // LSLS Rd, Rd, #trail
                m_binary->appendUint8(((trail << 6) & 0xFF) + (getGPEncoding(destination.u.reg) << 3) + (getGPEncoding(destination.u.reg)));
                m_binary->appendUint8((trail & 0x1C) >> 2);
            }
        } else
        {
            MethodBlock* immBlock;
            int newImmediateBlockId;
            int currentBlockID = createExtraBlock(newImmediateBlockId, immBlock);

            // TODO! Thumb big endian
            m_binary->appendUint8((value >>  0) & 0xFF);
            m_binary->appendUint8((value >>  8) & 0xFF);
            m_binary->appendUint8((value >> 16) & 0xFF);
            m_binary->appendUint8((value >> 24) & 0xFF);

            exitExtraBlock(currentBlockID, immBlock);

            // LDR Rd, =value;
            loadBlock(getGPEncoding(destination.u.reg), newImmediateBlockId);
        }
    }
}

void THUMBCompilerInterface::loadInt32(StackLocation destination,
                                       const cString& dependancyName)
{
    MethodBlock* immBlock;
    int newImmediateBlockId;
    int currentBlockID = createExtraBlock(newImmediateBlockId, immBlock);

    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0x00);
    m_binary->getCurrentDependecies().addDependency(
                                        dependancyName,
                                        m_binary->getCurrentBlockData().getSize() - 4,
                                        BinaryDependencies::DEP_32BIT,
                                        BinaryDependencies::DEP_ABSOLUTE);

    exitExtraBlock(currentBlockID, immBlock);

    // LDR Rd, =value;
    loadBlock(getGPEncoding(destination.u.reg), newImmediateBlockId);
}

void THUMBCompilerInterface::assignRet32(StackLocation source)
{
    if (getGPEncoding(source.u.reg) != THUMB_GP32_R0)
    {
        // MOV R0, Rn;
        mov(StackInterface::buildStackLocation(THUMB_GP32_R0, 0), source);
    }
}

void THUMBCompilerInterface::pushArg32(StackLocation source)
{
    StackLocation rTemp = source;
    if (getGPEncoding(source.u.reg) == THUMB_GP32_SP)
    {
        rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
        mov(rTemp, source);
    }
    if(getGPEncoding(source.u.reg) == THUMB_GP32_LR)
    {
        // PUSH {LR};
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xB5);
    } else
    {
        // PUSH {source};
        m_binary->appendUint8(0x01 << getGPEncoding(rTemp.u.reg));
        m_binary->appendUint8(0xB4);
    }

    if (getGPEncoding(source.u.reg) == THUMB_GP32_SP)
    {
        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
    m_stackRef += 4;
}

void THUMBCompilerInterface::popArg32(StackLocation source)
{
    StackLocation rTemp = source;
    if (getGPEncoding(source.u.reg) == THUMB_GP32_SP)
    {
        rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
    }
    if(getGPEncoding(source.u.reg) == THUMB_GP32_PC)
    {
        // POP {PC};
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xBD);
    } else {
        // POP {source};
        m_binary->appendUint8(0x01 << getGPEncoding(rTemp.u.reg));
        m_binary->appendUint8(0xBC);
    }

    if (getGPEncoding(source.u.reg) == THUMB_GP32_SP)
    {
        mov(source, rTemp);
        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
}

void THUMBCompilerInterface::mov(StackLocation destination, StackLocation source)
{
    int dst = getGPEncoding(destination.u.reg);
    int src = getGPEncoding(source.u.reg);

    if ((dst <= 7) && (src <=7))
    {
        m_binary->appendUint8((src << 3) + dst);
        m_binary->appendUint8(0x1C);
    } else {
        if ((dst <= 7) && (src > 7))
        {
            m_binary->appendUint8(0x40 + ((src - THUMB_GP32_R8) << 3) + dst);
        } else {
            if ((dst > 7) && (src <= 7))
            {
                m_binary->appendUint8(0x80 + ((src) << 3) + (dst - THUMB_GP32_R8));
            } else {
                // (dst > 7) && (src > 7)
                m_binary->appendUint8(0xC0 + ((src - THUMB_GP32_R8) << 3) + (dst - THUMB_GP32_R8));
            }
        }
        m_binary->appendUint8(0x46);
    }
}

void THUMBCompilerInterface::call(const cString& dependancyName, uint numberOfArguments)
{
    saveVolatileRegisters();
    // BL    0xFFFFFF
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xF7);
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);

    // Opcode f800f000, mask 22bit

    m_binary->getCurrentDependecies().addDependency(
        dependancyName,
        m_binary->getCurrentBlockData().getSize() - 4,
        BinaryDependencies::DEP_22BIT_2BYTES_LITTLE_ENDIAN,
        BinaryDependencies::DEP_RELATIVE,
        1,
        true);
}

void THUMBCompilerInterface::call(StackLocation address, uint numberOfArguments)
{
    saveVolatileRegisters();
    // BLX Rn
    format3(FORMAT3_OP_ADD, getGPEncoding(address.u.reg), 1); // Adding 1 to indicate we are brancing to THUMB function (Linker can't do it)
    m_binary->appendUint8(0x80 + (getGPEncoding(address.u.reg) << 3));
    m_binary->appendUint8(0x47);
}

void THUMBCompilerInterface::call32(const cString& dependancyName,
                                    StackLocation destination,
                                    uint numberOfArguments)
{
    if (getGPEncoding(destination.u.reg) != THUMB_GP32_R0)
    {
        // Free R0 from stack
        freeRegister32(THUMB_GP32_R0);
    }

    saveVolatileRegisters(getGPEncoding(THUMB_GP32_R0), getGPEncoding(destination.u.reg));

    // BL    0xFFFFFF
    m_binary->appendUint8(0xFF);
    m_binary->appendUint8(0xF7);
    m_binary->appendUint8(0xFE);
    m_binary->appendUint8(0xFF);
    m_binary->getCurrentDependecies().addDependency(
        dependancyName,
        m_binary->getCurrentBlockData().getSize() - 4,
        BinaryDependencies::DEP_22BIT_2BYTES_LITTLE_ENDIAN,
        BinaryDependencies::DEP_RELATIVE,
        1,
        true);

    if(destination.u.reg != getGPEncoding(THUMB_GP32_R0))
    {
        // MOV Rd, R0;
        mov(destination, StackInterface::buildStackLocation(THUMB_GP32_R0, 0));
    }
}

void THUMBCompilerInterface::call32(StackLocation address, StackLocation destination, uint numberOfArguments)
{
    if (getGPEncoding(destination.u.reg) != THUMB_GP32_R0)
    {
        // Free R0 from stack
        freeRegister32(THUMB_GP32_R0);
    }

    saveVolatileRegisters(THUMB_GP32_R0, getGPEncoding(destination.u.reg));

    // BLX Rn
    format3(FORMAT3_OP_ADD, getGPEncoding(address.u.reg), 1); // Adding 1 to indicate we are brancing to THUMB function (Linker can't do it)
    m_binary->appendUint8(0x80 + (getGPEncoding(address.u.reg) << 3));
    m_binary->appendUint8(0x47);

    if (getGPEncoding(destination.u.reg) != THUMB_GP32_R0)
    {
        // MOV Rd, R0;
        mov(destination, StackInterface::buildStackLocation(THUMB_GP32_R0, 0));
    }
}

void THUMBCompilerInterface::storeMemory(StackLocation destination,
                                         StackLocation value,
                                         uint offset,
                                         uint size)
{
    if (offset != 0)
    {
        addConst32(value, offset);
    }

    switch (size)
    {
    case 1:
        // STRB Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x70);
        break;
    case 2:
        // STRH Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x80);
        break;
    case 4:
        // STR Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x60);
        break;
    default:
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::loadMemory(StackLocation destination,
                                        StackLocation value,
                                        uint offset,
                                        uint size,
                                        bool signExtend)
{
    if (offset != 0)
    {
        addConst32(value, offset);
    }

    switch (size)
    {
    case 1:
        // LDRB Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x78);
        break;
    case 2:
        // LDRH Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x88);
        break;
    case 4:
        // LDR Rn, [Rd];
        m_binary->appendUint8((getGPEncoding(destination.u.reg) << 3) + getGPEncoding(value.u.reg));
        m_binary->appendUint8(0x68);
        break;
    default:
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::conv32(StackLocation destination,
                                    uint size,
                                    bool signExtend)
{
    StackLocation rTemp;

    if ((size == 1) || (size == 2))
    {
        rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();
        /*
         * LDR R1, =0xFF / 0xFFFF;
         * AND R0, R0, R1;
         */
        switch (size)
        {
        case 1:loadInt32(rTemp, 0xFFFF); break;
        case 2: loadInt32(rTemp, 0xFFFF); break;
        }
        and32(destination, rTemp);
        m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
    }
}

void THUMBCompilerInterface::neg32(StackLocation destination)
{
    // Convert to 2's complement
    not32(destination);
    addConst32(destination, 1);
}

void THUMBCompilerInterface::not32(StackLocation destination)
{
    // MVN Rd, Rd;
    m_binary->appendUint8((0x0C << 4) + (getGPEncoding(destination.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x43);
}

void THUMBCompilerInterface::add32(StackLocation destination,
                                   StackLocation source)
{
    // ADD Rd, Rd, Rs  (=rn)
    format2(0, FORMAT2_OP_ADD, getGPEncoding(source.u.reg),getGPEncoding(destination.u.reg),getGPEncoding(destination.u.reg));

    /**
     * Don't try to understand this, simply don't...
     */
    //m_binary->appendUint8(((getGPEncoding(source.u.reg) & 0x3) << 6) + (getGPEncoding(destination.u.reg) << 3) + getGPEncoding(destination.u.reg));
    //m_binary->appendUint8(0x18 + ((getGPEncoding(source.u.reg) & 0x4) >> 2));
}

void THUMBCompilerInterface::sub32(StackLocation destination,
                                   StackLocation source)
{
    // SUB Rd, Rd, Rs  (=rn)
    format2(0, FORMAT2_OP_SUB, getGPEncoding(source.u.reg),getGPEncoding(destination.u.reg),getGPEncoding(destination.u.reg));

    /**
     * Don't try to understand this, simply don't...
    // SUB Rd, Rn;
    m_binary->appendUint8(((getGPEncoding(source.u.reg) & 0x3) << 6) + (getGPEncoding(destination.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x1A + ((getGPEncoding(source.u.reg) & 0x4) >> 2));
     */
}

void THUMBCompilerInterface::mul32(StackLocation destination,
                                   StackLocation source)
{
    // MUL Rd, Rn;
    m_binary->appendUint8((0x04 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x43);
}

void THUMBCompilerInterface::div32(StackLocation destination,
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

void THUMBCompilerInterface::rem32(StackLocation destination,
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

void THUMBCompilerInterface::and32(StackLocation destination,
                                   StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        // AND Rd, Rn;
        m_binary->appendUint8((getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x40);
    } else
    {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::xor32(StackLocation destination,
                                   StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        // EOR Rd, Rn;
        m_binary->appendUint8((0x04 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x40);
    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::shr32(StackLocation destination,
                                   StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        // LSR Rd, Rn;
        m_binary->appendUint8((0x0C << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x40);
    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::shl32(StackLocation destination,
                                   StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        // LSL Rd, Rn;
        m_binary->appendUint8((0x08 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x40);
    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::or32(StackLocation destination,
                                  StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        // ORR Rd, Rn;
        m_binary->appendUint8((getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x43);
    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::adc32 (StackLocation destination, StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {
        m_binary->appendUint8(0x40 + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
        m_binary->appendUint8(0x41);
    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::sbb32 (StackLocation destination, StackLocation source)
{
    if ((getGPEncoding(destination.u.reg) <= THUMB_GP32_R7) && (getGPEncoding(source.u.reg) <= THUMB_GP32_R7))
    {

    } else {
        // UNSUPPORTED in THUMB
        CHECK_FAIL();
    }
}

void THUMBCompilerInterface::mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign)
{
    CHECK_MSG(FALSE, "mul32h not supported yet");
}

void THUMBCompilerInterface::addConst32(const StackLocation destination, int32 value)
{
    if (value != 0)
    {
        switch (getGPEncoding(destination.u.reg))
        {
        case THUMB_GP32_SP:
            m_binary->appendUint8(value / 4);
            m_binary->appendUint8(0xB0);
            break;
        default:
            m_binary->appendUint8(value);
            m_binary->appendUint8((0x03 << 4) + getGPEncoding(destination.u.reg));
            break;
        }
    }
}

void THUMBCompilerInterface::jump(int blockID)
{
    // TODO! Thumb2 instruction-set, what should we do with thumb-1?

    // Thumb-2 instruction set, section 3.3.6
    m_binary->appendUint8(0xFF);  // offset (MSB 8 bit)
    m_binary->appendUint8(0xF7);  // 1111 0 (MSB 3 bit offset)
    m_binary->appendUint8(0xFE);  // offset (LSB lower 8 bits)
    m_binary->appendUint8(0xBF);  // 1011 1 (LSB 3 bit offset)  TODO documents says that bits are 1 0 J1 1 J2, where J1 and J2 are bits 18:19, (duplicate)
    m_binary->getCurrentDependecies().addDependency(
        MangledNames::getMangleBlock(blockID, 2, BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 4,
        BinaryDependencies::DEP_22BIT_2BYTES_LITTLE_ENDIAN,
        BinaryDependencies::DEP_RELATIVE,
        1,
        true);
}

void THUMBCompilerInterface::jumpShort(int blockID)
{
    // B    blockID (15-bit)
    // See 5.18. Todo! Move to macro
    m_binary->appendUint8(0xFE);  // offset (lower 8 bit)
    m_binary->appendUint8(0xE7);  // 1110 0 (3 bit offset)
    //// Add dependency to the last 2 bytes
    m_binary->getCurrentDependecies().addDependency(
        MangledNames::getMangleBlock(blockID, 2, BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 2,
        BinaryDependencies::DEP_11BIT,
        BinaryDependencies::DEP_RELATIVE,
        1,
        true);
}

void THUMBCompilerInterface::jumpCond(StackLocation compare,
                                      int blockID,
                                      bool isZero)
{
    // TODO! Thumb2 instruction-set, what should we do with thumb-1?

    // CMP    Rn, #0;
    format3(FORMAT3_OP_CMP, getGPEncoding(compare.u.reg), 0);


    // Operand High
    {
        uint16 operand = (0xF << 12) |               // opcode
                         (1 << 10) |                 // signned
                         ((isZero ? 0 : 1) << 6) |   // cond 0 - BEQ, 1 = BNE
                         (0x3F);                     // offset [12:17]
        appendUint16(operand);
    }

    // Operand low
    {
        uint16 operand = (2 << 14) |                 // opcode
                         (5 << 11) |                 // J1 0 J2
                         (0x7FE);                    // offset LSB [1:11]
        appendUint16(operand);
    }

    m_binary->getCurrentDependecies().addDependency(
            MangledNames::getMangleBlock(blockID, 2, BinaryDependencies::DEP_RELATIVE),
            m_binary->getCurrentBlockData().getSize() - 4,
            BinaryDependencies::DEP_19BIT_2BYTES_LITTLE_ENDIAN,
            BinaryDependencies::DEP_RELATIVE,
            1,
            true);
}

void THUMBCompilerInterface::jumpCondShort(StackLocation compare,
                                           int blockID,
                                           bool isZero)
{
    // CMP    Rn, #0;
    format3(FORMAT3_OP_CMP, getGPEncoding(compare.u.reg), 0);

    // The address part of BEQ/BNE
    // TODO! Format 5.16
    m_binary->appendUint8(0xFD);
    // Add dependency to the last byte
    m_binary->getCurrentDependecies().addDependency(MangledNames::getMangleBlock(blockID, 1, BinaryDependencies::DEP_RELATIVE),
        m_binary->getCurrentBlockData().getSize() - 1,
        BinaryDependencies::DEP_8BIT,
        BinaryDependencies::DEP_RELATIVE,
        1, // Divide by 2
        true);
    if (isZero)
    {
        // BEQ    blockID
        m_binary->appendUint8(0xD0);
    } else
    {
        // BNE    blockID
        m_binary->appendUint8(0xD1);
    }
}

void THUMBCompilerInterface::ceq32(StackLocation destination,
                                   StackLocation source)
{
    StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

    // LDR rTemp, =1;
    loadInt32(rTemp, 1);

    // CMP Rd, Rn;
    m_binary->appendUint8((0x08 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x42);

    // BEQ #1;
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0xD0);

    // LDR Rd, =0;
    loadInt32(rTemp, 0);

    mov(destination, rTemp);

    m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
}

void THUMBCompilerInterface::cgt32(StackLocation destination,
                                   StackLocation source,
                                   bool isSigned)
{
    StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

    // LDR Rd, =1;
    loadInt32(rTemp, 1);

    // CMP Rd, Rn;
    m_binary->appendUint8((0x08 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x42);

    if (isSigned)
    {
        // BGT #1;
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xDC);
    } else
    {
        // BHI #1;
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xD8);
    }

    // LDR rTemp, =0;
    loadInt32(rTemp, 0);

    mov(destination, rTemp);

    m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
}

void THUMBCompilerInterface::clt32(StackLocation destination,
                                   StackLocation source,
                                   bool isSigned)
{
    StackLocation rTemp = m_binary->getCurrentStack()->allocateTemporaryRegister();

    // LDR Rd, =1;
    loadInt32(rTemp, 1);

    // CMP Rd, Rn;
    m_binary->appendUint8((0x08 << 4) + (getGPEncoding(source.u.reg) << 3) + getGPEncoding(destination.u.reg));
    m_binary->appendUint8(0x42);

    if (isSigned)
    {
        // BLT #1;
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xDB);
    } else
    {
        // BLO #1;
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0xD3);
    }

    // LDR Rd, =0;
    loadInt32(rTemp, 0);

    mov(destination, rTemp);

    m_binary->getCurrentStack()->freeTemporaryRegister(rTemp);
}

void THUMBCompilerInterface::localloc(StackLocation destination,
                                      StackLocation size,
                                      bool isStackEmpty)
{
    // Allocate #size bytes onthe stack
    sub32(getMethodBaseStackRegister(), size);
    mov(destination, getMethodBaseStackRegister());
}

void THUMBCompilerInterface::revertStack(uint32 size)
{
    m_stackRef -= size;
    addConst32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_SP), 0), size);
}

void THUMBCompilerInterface::saveNonVolatileRegisters(bool bSave)
{
    const RegisterAllocationTable& touched = m_binary->getTouchedRegisters();

    // Examine all known registers
    cList<int> regs = m_archRegisters.keys();
    cList<int>::iterator i = regs.begin();

    if (!bSave)
    {
        i = regs.end();
        --i;
    }

    while (1)
    {
        if (bSave && (i == regs.end()))
            break;

        int reg = *i;
        // Is this register non-volatile?
        if ((m_archRegisters[reg].m_eType == NonVolatile) &&
            (touched.hasKey(reg)))
        {
            StackLocation registerLocation = StackInterface::buildStackLocation(reg, 0);
            if (bSave)
            {
                pushArg32(registerLocation);
                m_stackRef -= 4;
                m_nonVolSize += 4;
            }
            else
            {
                popArg32(registerLocation);
            }
        }

        if (bSave)
        {
            ++i;
        } else
        {
            if (i == regs.begin())
                break;
            --i;
        }
    }

///////////////////////
#if 0
    const RegisterAllocationTable& touched = m_binary->getTouchedRegisters();

    // NOTE: The order between push and pop should be reverse!!!
    if (shouldPush)
    {
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R3)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R3), 0));
            m_stackRef -= 4;
            m_nonVolSize += 4;
        }
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R4)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R4), 0));
            m_stackRef -= 4;
            m_nonVolSize += 4;
        }
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R5)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R5), 0));
            m_stackRef -= 4;
            m_nonVolSize += 4;
        }
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R6)))
        {
            pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R6), 0));
            m_stackRef -= 4;
            m_nonVolSize += 4;
        }
    } else
    {
        // POP. NOTE: REVERSE ORDER!
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R6)))
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R6), 0));
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R5)))
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R5), 0));
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R4)))
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R4), 0));
        if (touched.hasKey(getGPEncoding(THUMB_GP32_R3)))
            popArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_R3), 0));
    }
#endif
}

void THUMBCompilerInterface::saveVolatileRegisters(int saveRegister1,
                                                    int saveRegister2,
                                                    int saveRegister3)
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

void THUMBCompilerInterface::align()
{
    const FirstPassBinary::BasicBlockList& blocks = m_binary->getBlocksList();
    // And combine blocks
    FirstPassBinary::BasicBlockList::iterator i = blocks.begin();
    uint32 methodSize = 0;
    for (; i != blocks.end(); ++i)
    {
        methodSize += (*i).m_data.getSize();
    }

    if ((methodSize % 4) != 0)
    {
        // ZEROS;
        m_binary->appendUint8(0x00);
        m_binary->appendUint8(0x00);
    }
}

void THUMBCompilerInterface::generateMethodEpiProLogs(bool bForceSaveNonVolatiles /* = false */)
{
    // Check for allocated registers, and debug!

    // Add prolog
    // Generate new first stream, and switch to it.
    MethodBlock* prolog = new MethodBlock(MethodBlock::BLOCK_PROLOG, *this);
    StackInterfacePtr estack(prolog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_PROLOG, estack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_PROLOG);

    // PUSH {LR};
    pushArg32(m_binary->getCurrentStack()->buildStackLocation(getGPEncoding(THUMB_GP32_LR), 0));
    m_stackRef -= 4;

    // Test for non-volatile registers touching
    saveNonVolatileRegisters(true);

    // SUB SP, #stackSize;
    if (m_binary->getStackSize() != 0)
    {
        m_binary->appendUint8((0x80) + (m_binary->getStackSize() / 4));
        m_binary->appendUint8(0xB0);
    }

    m_stackRef += m_binary->getStackSize() - StackInterface::LOCAL_STACK_START_VALUE + 4;

    prolog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);

    // Conclude the method by adding a epilog
    // Generate last stream, and switch to it.
    MethodBlock* epilog = new MethodBlock(MethodBlock::BLOCK_RET, *this);
    StackInterfacePtr pstack(epilog);

    m_binary->createNewBlockWithoutChange(MethodBlock::BLOCK_RET, pstack);
    m_binary->changeBasicBlock(MethodBlock::BLOCK_RET);

    revertStack(m_binary->getStackSize());
    // Restore the non-volatile registers touching
    saveNonVolatileRegisters(false);

    // POP {PC};
    m_binary->appendUint8(0x00);
    m_binary->appendUint8(0xBD);

    m_binary->setNonVolatilesSize(m_nonVolSize);

    epilog->terminateMethodBlock(NULL, MethodBlock::COND_NON, 0);
}

void THUMBCompilerInterface::resetBaseStackRegister(const StackLocation& targetRegister)
{
    if (getMethodBaseStackRegister() == targetRegister)
        return;

    mov(targetRegister, getMethodBaseStackRegister());

    // And forget that we ever used a different register
    setMethodBaseStackRegister(getStackPointer());
}

void THUMBCompilerInterface::setFramePointer(StackLocation destination)
{
    ASSERT(destination != getMethodBaseStackRegister());

    mov(destination, getMethodBaseStackRegister());

    // If it's SP, then take stackRef into account
    if (getMethodBaseStackRegister() == getStackPointer())
    {
        if (m_stackRef != 0)
        {
            // SUB SP, #stackRef
            m_binary->appendUint8(0x80 + (m_stackRef / 4));
            m_binary->appendUint8(0xB0);
        }
    }
}

bool THUMBCompilerInterface::isNativeBasePointer() const
{
    return (getMethodBaseStackRegister() == getStackPointer());
}

uint THUMBCompilerInterface::getFrameStackRef()
{
    if (getMethodBaseStackRegister() == getStackPointer())
    {
        return m_stackRef;
    }
    return 0;
}
// TODO: Move this function into xSTL
int THUMBCompilerInterface::ntz(unsigned x)
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
int THUMBCompilerInterface::nlz(unsigned x)
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