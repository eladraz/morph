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
 * MethodBlock.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/MethodBlock.h"
#include "compiler/StackEntity.h"
#include "compiler/CompilerTrace.h"
#include "compiler/CompilerInterface.h"
#include "compiler/EmitContext.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"

MethodBlock::MethodBlock(int firstBlockID,
                         CompilerInterface& compiler) :
    StackInterface(*compiler.getFirstPassPtr(), true, compiler.getStackSize()),
    m_compiler(compiler),
    m_isSeal(false),
    m_blockID(firstBlockID),
    m_stack(),
    m_registers(compiler.getArchRegisters()),
    m_registerSize(compiler.getStackSize()),
    m_exceptionStack()
{
    setBaseStackRegister(compiler.getStackPointer());
}

MethodBlock::MethodBlock(int blockID,
                         const MethodBlock& other) :
    StackInterface(other),
    m_compiler(other.m_compiler),
    m_isSeal(false),
    m_blockID(blockID),
    m_stack(other.m_stack),
    m_registers(other.m_registers),
    m_registerSize(other.m_registerSize),
    m_exceptionStack(other.m_exceptionStack)
{
    setBaseStackRegister(other.m_baseRegister);
}

MethodBlock::~MethodBlock()
{
    m_stack.clear();
}

uint MethodBlock::getStackType()
{
    return CLR_STACK;
}

RegisterAllocationTable& MethodBlock::getRegistersTable()
{
    return m_registers;
}

const RegisterAllocationTable& MethodBlock::getRegistersTable() const
{
    return m_registers;
}

uint MethodBlock::getRegisterSize()
{
    return m_registerSize;
}

StackLocation MethodBlock::replaceRegisterToStackVariable(StackLocation source)
{
    // Check that the source is really a register
    CHECK(source.u.reg < 0);

    CorElementType coreType;
    switch (m_registerSize)
    {
    case 1: coreType = ELEMENT_TYPE_U1; break;
    case 2: coreType = ELEMENT_TYPE_U2; break;
    case 4: coreType = ELEMENT_TYPE_U4; break;
    case 8: coreType = ELEMENT_TYPE_U8; break;
    default:
        // The machine register-size is invalid!
        CHECK_FAIL();
    }

    // Generate new stack holder
    TemporaryStackHolderPtr newDest(new TemporaryStackHolder(
            *this, coreType, m_registerSize,
            TemporaryStackHolder::TEMP_ONLY_STACK));


    // Scan all the register places and replace them with stack registers
    // NOTE: THIS CLASS IS NOT THREAD-SAFE. BE AWARE
    cList<StackEntity>& stackList = m_stack.getList();
    cList<StackEntity>::iterator i(stackList.begin());

    // DEAN: We replaced a register with a stack. But the stack value probably popped from
    //       the stack (for opcode implementation). Need to add "peek(-1)/peek(-2)" immediate
    //       access
    //CHECK(i != stackList.end());

    for (; i != stackList.end(); ++i)
    {
        TemporaryStackHolderPtr obj = (*i).getStackHolderObject();
        if ((!obj.isEmpty()) &&
            (((*i).getType() == StackEntity::ENTITY_REGISTER) ||
             ((*i).getType() == StackEntity::ENTITY_REGISTER_ADDRESS)))
        {
            if (obj->getTemporaryObject().u.reg == source.u.reg)
            {
                StackEntity newObject(*i);
                StackEntity::EntityType entityType = StackEntity::ENTITY_LOCAL_TEMP_STACK;
                if ((*i).getType() == StackEntity::ENTITY_REGISTER_ADDRESS)
                    entityType = StackEntity::ENTITY_LOCAL_TEMP_STACK_ADDRESS;

                newObject.setType(entityType);
                newObject.setStackHolderObject(newDest);
                // Just switch
                (*i) = newObject;
            }
        }
    }

    return newDest->getTemporaryObject();
}

void MethodBlock::fixStack(Stack& stack, MethodBlock& newBlock)
{
    // NOTE: THIS CLASS IS NOT THREAD-SAFE. BE AWARE
    // Change the stack
    cList<StackEntity>& stackList = stack.getList();
    cList<StackEntity>::iterator i(stackList.begin());
    cHash<uint32, TemporaryStackHolderPtr> m_allocated;
    for (; i != stackList.end(); ++i)
    {
        // Duplicate item
        TemporaryStackHolderPtr obj = (*i).getStackHolderObject();
        if (!obj.isEmpty())
        {
            uint32 stackLocation = obj->getTemporaryObject().raw;
            // And remove reference...
            // NOTE: Bug can be occurred if in the end of the block there are
            //       two reference into the same stack-location.
            if (m_allocated.hasKey(stackLocation))
            {
                // Use previous split
                (*i).setStackHolderObject(m_allocated[stackLocation]);
            } else
            {
                // Create new split. NOTICE THE REFERENCE!
                TemporaryStackHolderPtr newObject(
                    obj->duplicateOnNewInterface(newBlock));
                (*i).setStackHolderObject(newObject);
                m_allocated.append(stackLocation, newObject);
            }
        }
    }
}

MethodBlock* MethodBlock::duplicate(EmitContext& emitContext,
                                    int newBlockID,
                                    bool shouldRemoveTOS)
{
    // Make sure that the evaulate stack has no locals
    uint floatCount = m_stack.getStackCount();
    for (uint i = (shouldRemoveTOS) ? 1 : 0; i < floatCount; i++)
    {
        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, m_stack.getArg(i), true, 0, true);
    }


    // Duplicate block
    MethodBlock* newBlock(new MethodBlock(newBlockID,
                                          *this));

    // Protect with reference countable object
    //StackInterfacePtr ref(newBlock);

    // Change the stack
    fixStack(newBlock->m_stack, *newBlock);

    // This will cause the last value to be removed from the NEW stack!
    if (shouldRemoveTOS)
        newBlock->m_stack.pop2null();

    // Just return the new object
    //return ref;
    return newBlock;
}

void MethodBlock::appendInstruction(int address, uint position)
{
    CHECK(!m_isSeal);
    // Check that the address is incremental.
    if (m_addressPositions.begin() != m_addressPositions.end())
    {
        CHECK((*(--m_addressPositions.end())).m_a < address);
    }
    // And just add address <-> position translator
    m_addressPositions.append(AddressTuple(address, position));
}

void MethodBlock::markAddress(cSetArray& setArray, bool shouldSet)
{
    for (cList<AddressTuple>::iterator i = m_addressPositions.begin();
         i != m_addressPositions.end();
         i++)
    {
        if (shouldSet)
            setArray.set((*i).m_a);
        else
            setArray.clear((*i).m_a);
    }
}

/*
 * Old code
 *
void MethodBlock::split(FirstPassBinary& pass,
                        FirstPassBinary::BasicBlock& currentBlock,
                        const StackInterfacePtr& newMethodStack,
                        MethodBlock& newMethodBlock)
{
    CHECK(m_isSeal);

    // Get split address
    uint splitAddress = newMethodBlock.getBlockID();
    cList<AddressTuple>::iterator i = m_addressPositions.begin();
    for (; i != m_addressPositions.end(); ++i)
    {
        if ((*i).m_a == splitAddress)
        {
            break;
        }
    }
    // Couldn't find the split address
    CHECK(i != m_addressPositions.end());

    // Move the data
    uint currentNewMethodLength = (*i).m_b;
    cBuffer newData(currentBlock.m_data.getBuffer() + currentNewMethodLength,
                    currentBlock.m_data.getSize() - currentNewMethodLength);
    currentBlock.m_data.changeSize(currentNewMethodLength);
    // Append the code information
    pass.createNewBlockWithoutChange(splitAddress, newMethodStack);
    pass.changeBasicBlock(splitAddress);
    pass.appendBuffer(newData);

    // Change the stack of the new method
    newMethodBlock.m_stack = m_stack;
    newMethodBlock.m_registers = m_registers;
    fixStack(newMethodBlock.m_stack, newMethodBlock);

    // Move address information
    for (; i != m_addressPositions.end(); )
    {
        newMethodBlock.appendInstruction((*i).m_a, (*i).m_b -
                                         currentNewMethodLength);
        cList<AddressTuple>::iterator temp = i;
        ++i;
        m_addressPositions.remove(temp);
    }

    // Append dependencies informations
    BinaryDependencies::DependencyObjectList::iterator j =
            currentBlock.m_dependecies.getList().begin();
    for (; j != currentBlock.m_dependecies.getList().end();)
    {
        // Check whether the dependency need to relocated
        if ((*j).m_position >= currentNewMethodLength)
        {
            // Add new dependecy
            pass.getCurrentDependecies().addDependency(
                                    (*j).m_name,
                                    (*j).m_position - currentNewMethodLength,
                                    (*j).m_length,
                                    (*j).m_type);
            // Remove dependency
            BinaryDependencies::DependencyObjectList::iterator temp(j);
             ++j;
            currentBlock.m_dependecies.getList().remove(temp);
        } else
             ++j;
    }

    // Seal up the new block with the current block condition
    newMethodBlock.terminateMethodBlock(m_type, m_conditionBlock);
    // Change the current block condition to default.
    m_type = MethodBlock::COND_NON;
    m_conditionBlock = 0;
}
*/

int MethodBlock::getBlockID() const
{
    return m_blockID;
}

Stack& MethodBlock::getCurrentStack()
{
    return m_stack;
}

MethodBlock::ConditionalType MethodBlock::getConditionalCase() const
{
    CHECK(m_isSeal);
    return m_type;
}

int MethodBlock::getConditionBlock() const
{
    CHECK(m_isSeal);
    return m_conditionBlock;
}

void MethodBlock::terminateMethodBlock(EmitContext* emitContext, ConditionalType type, int conditionBlock, int terminateAt)
{
    ASSERT(!m_isSeal);

    // Evaluate stack
    if ((emitContext != NULL) && (type == COND_NON))
    {
        uint floatCount = m_stack.getStackCount();
        for (uint i = 0; i < floatCount; i++)
        {
            RegisterEvaluatorOpcodes::evaluateInt32(*emitContext, m_stack.getArg(i), true, 0, true);
        }
    }

    m_type = type;
    m_conditionBlock = conditionBlock;
    m_isSeal = true;

    // CompilerTrace("End of block. Return to: " << HEXDWORD(terminateAt) << " " << "Cond block at: " << HEXDWORD(m_conditionBlock) << endl);
}

void MethodBlock::finalizeCondition()
{
    ASSERT(m_isSeal);
    m_conditionBlock|= COND_HANDLED_BIT;
}

bool MethodBlock::isConditionFinialized() const
{
    ASSERT(m_isSeal);
    return (m_conditionBlock & COND_HANDLED_BIT) != 0;
}

MethodBlock::ExceptionList& MethodBlock::getExceptionsStack()
{
    return m_exceptionStack;
}

MethodBlock::ExceptionEntry::ExceptionEntry(const ExceptionEntry& other) :
    m_clause(other.m_clause),
    m_handlerTokenIndex(other.m_handlerTokenIndex),
    m_currentPart(other.m_currentPart),
    m_origCompiler(other.m_origCompiler),
    m_origBaseStackRegister(other.m_origBaseStackRegister)
{
}

MethodBlock::ExceptionEntry::ExceptionEntry(const MethodHeader::ExceptionHandlingClause& clause, StackLocation origBaseStackRegister, CompilerInterface& origCompiler) :
    m_clause(clause),
    m_handlerTokenIndex(ElementType::UnresolvedTokenIndex),
    m_currentPart(None),
    m_origCompiler(origCompiler),
    m_origBaseStackRegister(origBaseStackRegister)
{
}

const TokenIndex& MethodBlock::ExceptionEntry::getHandlerTokenIndex() const
{
    return m_handlerTokenIndex;
}

void MethodBlock::ExceptionEntry::setHandlerTokenIndex(const TokenIndex& index)
{
    m_handlerTokenIndex = index;
}

const MethodHeader::ExceptionHandlingClause& MethodBlock::ExceptionEntry::getClause() const
{
    return m_clause;
}

MethodBlock::ExceptionPart MethodBlock::ExceptionEntry::hitTest(uint index) const
{
    if ((index >= m_clause.tryOffset) &&
        (index < m_clause.tryOffset + m_clause.tryLength))
        return ProtectedBlock;
    if ((index >= m_clause.handlerOffset) &&
        (index < m_clause.handlerOffset + m_clause.handlerLength))
        return Handler;
    return None;
}

cString MethodBlock::ExceptionEntry::nameSuffix() const
{
    cStringStream str;
    switch (m_clause.flags)
    {
    case MethodHeader::ClauseCatch:
        str << "_CatchHandler_";
        break;
    case MethodHeader::ClauseFilter:
        str <<"_FilterHandler_";
        break;
    case MethodHeader::ClauseFinally:
        str << "_FinallyHandler_";
        break;
    case MethodHeader::ClauseFault:
        str << "_FaultHandler_";
        break;
    }
    str << HEXDWORD(m_handlerTokenIndex.m_a & 0xFFFFFF);
    return str.getData();
}

void MethodBlock::ExceptionEntry::setCurrentPart(ExceptionPart part)
{
    m_currentPart = part;
}

MethodBlock::ExceptionPart MethodBlock::ExceptionEntry::getCurrentPart() const
{
    return m_currentPart;
}

const CompilerInterface& MethodBlock::ExceptionEntry::getOrigCompiler() const
{
    return m_origCompiler;
}

const StackLocation& MethodBlock::ExceptionEntry::getOrigBaseStackRegister() const
{
    return m_origBaseStackRegister;
}

bool operator< (const StackInterfacePtr& one, const StackInterfacePtr& two)
{
    const MethodBlock* blockOne = ((const MethodBlock*)one.getPointer());
    const MethodBlock* blockTwo = ((const MethodBlock*)two.getPointer());
    // Order by block IDs
    return blockOne->getBlockID() < blockTwo->getBlockID();
}


bool MethodBlock::isOptimizerCompiler() const
{
    if (m_compiler.isOptimizerCompiler())
    {
        return true;
    }
    return false;
}
