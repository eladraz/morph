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

#ifndef __TBA_CLR_COMPILER_METHODBLOCK_H
#define __TBA_CLR_COMPILER_METHODBLOCK_H

/*
 * MethodBlock.h
 *
 * Contains the information needed to compile a method basic block
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/orderedList.h"
#include "xStl/data/hash.h"
#include "xStl/data/setArray.h"
#include "dismount/assembler/FirstPassBinary.h"
#include "dismount/assembler/StackInterface.h"
#include "runnable/Stack.h"
#include "format/methodHeader.h"
#include "compiler/CompilerInterface.h"

// Forward declaration
struct EmitContext;

/*
 * Methods builds from many basic block. Each block is linked in a conditional
 * jump (always, greater, less or even return) into other basic blocks.
 * In order to compile a method you need to separate each basic block and then
 * to linked them together. This job is done by the FirstBinaryPass However the
 * method must interlinked with itself first.
 *
 * This class handle basic-block relation inside a method and provide a
 * mechanism to compile all methods blocks together and store instructions
 * pointers.
 *
 * Just to clarify a block is a set of instructions ends with conditional jump.
 *
 * NOTE: This class is not thread safe
 */
class MethodBlock : public StackInterface {
public:
    // Possible parts of an Exception clause
    enum ExceptionPart
    {
        ProtectedBlock,
        Filter,
        Handler,
        None,
    };

    // An entry in the exception stack
    class ExceptionEntry
    {
    public:
        /*
         * constructor. Initialize an entry based on a clause
         *
         * clase - The Exception clause from the method header
         */
        ExceptionEntry(const MethodHeader::ExceptionHandlingClause& clause, StackLocation origBaseStackRegister, CompilerInterface& origCompiler);
        /*
         * copy-constructor.
         */
        ExceptionEntry(const ExceptionEntry& other);

        /*
         * Returns the TokenIndex for the handler routine
         */
        const TokenIndex& getHandlerTokenIndex() const;
        /*
         * Sets the TokenIndex for the handler routine
         *
         * index - a generated TokenIndex for the handler routine
         */
        void setHandlerTokenIndex(const TokenIndex& index);

        /*
         * Returns the exception clause, just like the one in the method header
         */
        const MethodHeader::ExceptionHandlingClause& getClause() const;

        /*
         * Determine the relationship between an instruction (at the specified index) and this entry
         *
         * index - the index to test
         * returns any possible ExceptionPart value
         */
        ExceptionPart hitTest(uint index) const;

        /*
         * Returns a string suffix for the exception handler routine's name
         */
        cString nameSuffix() const;

        /*
         * Sets the current part of the cluase which is being compiled
         * part - the part being compiled
         */
        void setCurrentPart(ExceptionPart part);
        /*
         * Returns the current part of the cluase which is being compiled
         */
        ExceptionPart getCurrentPart() const;

        /*
         * Returns the original base stack register
         */
        const StackLocation& getOrigBaseStackRegister() const;

        /*
         * Returns the original compiler for the method
         */
        const CompilerInterface& getOrigCompiler() const;

    private:
        // The exception clause, including type, ranges, etc.
        MethodHeader::ExceptionHandlingClause m_clause;
        // The handler routine's TokenIndex
        TokenIndex  m_handlerTokenIndex;
        // The current part of the clause which is being compiled
        ExceptionPart m_currentPart;
        // Original compiler for the method (with the method's FPB and other state)
        CompilerInterface& m_origCompiler;
        // Original base stack register
        StackLocation m_origBaseStackRegister;
    };
    // A list of entries
    typedef cList<ExceptionEntry> ExceptionList;

    /*
     * First block constructor. Initialize a method block meta-data.
     *
     * firstBlockID      - The index for the FirstPassBinary::BasicBlock ID of
     *                     which content applies
     * compiler          - A compiler interface which is used for all other block parameters
     */
    MethodBlock(int firstBlockID,
                class CompilerInterface& compiler);

    /*
     * New method block constructor
     *
     * blockID  - The index for the FirstPassBinary::BasicBlock ID of
     *            which content applies
     * Stack - The initialized stack representation object
     * other    - The previous block to copy the information from
     */
    MethodBlock(int blockID,
                const MethodBlock& other);

    /*
     * Destructor. Make sure that the stack will be killed before the registers
     * table
     */
    ~MethodBlock();

    // The RTTI information for the CLR::compiler::MethodBlock stack repository
    enum { CLR_STACK = 0xC11134CA };

    // See StackInterface::getStackType
    virtual uint getStackType();
    // See StackInterface::getRegistersTable
    virtual RegisterAllocationTable& getRegistersTable();
    virtual const RegisterAllocationTable& getRegistersTable() const;
    // See StackInterface::getRegisterSize()
    virtual uint getRegisterSize();

    // See StackInterface::replaceRegisterToStackVariable
    // Change the Stack locations.
    // NOTE: THIS CLASS IS NOT THREAD-SAFE. BE AWARE
    virtual StackLocation replaceRegisterToStackVariable(StackLocation source);

    /*
     * Copy the current block into new block.
     *
     * NOTE: The stack reference objects are regenerated!
     *
     * newBlockID      - The new block ID
     * shouldRemoveTOS - Set to true to remove the top-os-stack.
     */
    MethodBlock* duplicate(EmitContext& emitContext,
                           int newBlockID,
                           bool shouldRemoveTOS = false);

    /*
     * Mark a new instruction at block position.
     *
     * address  - The address value, incremental value.
     * position - The start position of the new address
     *
     * Throw exception if the current block is sealed.
     * Throw exception if the address is not in linear raising order
     */
    void appendInstruction(int address, uint position);

    /*
     * Copy the address tables into set array
     *
     * shouldSet - Set to true if the address should be "set".
     *             Set to false if the address should be "clear"
     */
    void markAddress(cSetArray& setArray, bool shouldSet = true);

    /*
     * Split a sealed blocks into another.
     *
     * (Sealed block is a block which the 'terminateMethodBlock' method was
     * called for it, e.g. the current block should be sealed)
     *
     * This function takes the data from the current block, starting from
     * 'newMethodBlock.getBlockID()', including code, addresses and dependencies
     * generate new FirstPass block and transferring the information to it.
     *
     * This function changes the end-condition of this block into 'COND_NON' and
     * sealing the new block with the current block condition status.
     *
     * pass           - The current method pass
     * currentBlock   - The current block data. Variable for optimization.
     * newMethodBlock - The new block to generate and move information to.
     *
     * Throw exception if the current block is not sealed.
     */

    /*
     * Depracted code
     *
    void split(FirstPassBinary& pass,
               FirstPassBinary::BasicBlock& currentBlock,
               const StackInterfacePtr& newMethodStack,
               MethodBlock& newMethodBlock);
     */

    /*
     * Returns the index of FirstPassBinary::BasicBlock ID value
     */
    int getBlockID() const;

    /*
     * Return the current block representation stack.
     *
     * NOTE: THIS STACK IS NOT THREAD SAFE
     *       But this stack is unique for the current block, allowing compiler
     *       threads optimizations
     */
    Stack& getCurrentStack();

    //////////////////////////////////////////////////////////////////////////
    // The conditional jump
    enum ConditionalType {
        // Merge with another block or just terminate without any more
        // information (e.g. exception was thrown)
        COND_NON        = 0,
        // Jump always.   (.NET instruction br.s)
        COND_ALWAYS     = 1,
        // Jump if last value is zero (false, equals)
        COND_ZERO       = 2,
        // Jump if last value is non zero (true, non-null, not equal)
        COND_NON_ZERO   = 3,
        // Terminate the method, go to return block  (.NET instruction ret)
        COND_RETURN     = 4,

        // TODO! Add exception handling blocks

        // Appended to the condition bit to determine that the current block was
        // handled, and the terminate block was already added.
        COND_HANDLED_BIT = 0x8000
    };

    // First block and last block IDs
    enum {
        // ID for prolog block
        BLOCK_PROLOG = (~MAX_INT32),
        // ID for initialize (uninitaliez) locals object
        BLOCK_INIT_OBJECTS,
        // ID for register method cleanup handler
        BLOCK_REG_CLEANUP,

        // ID for just-before-return block
        BLOCK_BEFORE_RET = (1<<24),
        // ID for return block
        BLOCK_RET,
        // Extra function data
        BLOCK_EXTRA_DATA
    };

    /*
     * Returns the condition of this block
     *
     * Throw exception if this method block is not sealed.
     */
    ConditionalType getConditionalCase() const;

    /*
     * Returns the next block if the condition is true
     *
     * NOTE: For always and return condition the next block will be the same as
     *       getNotConditionBlock()
     *
     * Throw exception if this method block is not sealed.
     */
    int getConditionBlock() const;

    /*
     * Mark the end of the current block and seal it.
     *
     * type              - The type of the condition
     * conditionBlock    - The next block if the condition is true
     *
     * NOTE: Assertion will be thrown if the method-block was already sealed.
     */
    void terminateMethodBlock(EmitContext* emitContext, ConditionalType type,
                              int conditionBlock, int terminateAt = -1);

    /*
     * Changes the COND_HANDLED_BIT to indicate that the current block was
     * handled by the MethodCompiler (Useful to determine block size).
     *
     * Throw exception if this method block is not sealed.
     */
    void finalizeCondition();

    /*
     * Return true if the COND_HANDLED_BIT was set in the condition case.
     * See finalizeCondition()
     *
     * Throw exception if this method block is not sealed.
     */
    bool isConditionFinialized() const;

    /*
     * Return the exception-stack
     */
    ExceptionList& getExceptionsStack();

    /*
     * Re-implementation of method from StackInterface::shouldFreeRegisters
     */
    virtual bool isOptimizerCompiler() const;

private:
    // Deny copy-constructor and operator =
    MethodBlock(const MethodBlock& other);
    MethodBlock& operator = (const MethodBlock& other);

    /*
     * Fix new stack of new block by duplicating all of it's registers into the
     * newly generated context
     *
     * stack    - The stack to be fixed
     * newBlock - The newly generated stack
     *
     * NOTE: This method is not thread safe
     */
    void fixStack(Stack& stack, MethodBlock& newBlock);

    // Set to true if the terminateMethodBlock() was already called.
    bool m_isSeal;
    // The condition type
    ConditionalType m_type;
    // The next block if the condition is true
    int m_conditionBlock;

    // The current block ID
    int m_blockID;
    // The current block initialized stack
    Stack m_stack;
    // The allocated and free registers for this block
    RegisterAllocationTable m_registers;
    // The size of each register
    uint m_registerSize;

    // The exception-stack
    ExceptionList m_exceptionStack;

    // Method addresses. Stored a pair of 'int address' and 'uint position'
    typedef cDualElement<int, uint> AddressTuple;
    cList<AddressTuple> m_addressPositions;

    CompilerInterface& m_compiler;
};

// The management stack
typedef StackInfrastructor<StackInterfacePtr, cList<StackInterfacePtr>::iterator>
                                                            MethodBlockStack;
// An ordered list of blocks
typedef cOrderedList<StackInterfacePtr> MethodBlockOrderedList;

// This operator determines the order of blocks in the ordered list
bool operator< (const StackInterfacePtr& one, const StackInterfacePtr& two);

#endif // __TBA_CLR_COMPILER_METHODBLOCK_H
