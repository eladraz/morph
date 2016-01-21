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

#ifndef __TBA_CLR_COMPILER_OPTIMIZERCOMPILERINTERFACE_H
#define __TBA_CLR_COMPILER_OPTIMIZERCOMPILERINTERFACE_H

/*
 * OptimizerCompilerInterface.h
 * This class is suppose to implement various optimization features for compilation.
 */

#include "xStl/data/smartptr.h"
#include "xStl/data/hash.h"

#include "compiler/CompilerInterface.h"
#include "compiler/OptimizerOperationCompilerInterface.h"

#include "dismount/assembler/FirstPassBinary.h"

#include "xStl/data/graph.h"

#include "compiler/CompilerTrace.h"
#include <stdio.h>


/* List to save block opcodes */
typedef cList<CompilerInterface::CompilerOperation> BlockOperationList;

class DummyRegistersMapping
{
public:
    class Entry
    {
    public:
        Entry() :
           m_isAlive(false),
           m_possibleRegisters(0, true, true),
           m_chosenRegister(-1)
        {};

        Entry(int numberOfRealRegisters) :
            m_isAlive(false),
            m_possibleRegisters(numberOfRealRegisters, true, true),
            m_chosenRegister(-1)
        {};

        Entry(const Entry& infoInEntry) :
            m_isAlive(infoInEntry.m_isAlive),
            m_possibleRegisters(infoInEntry.m_possibleRegisters),
            m_chosenRegister(infoInEntry.m_chosenRegister)
        {};

        void initEntry(uint numberOfRealRegisters)
        {
            m_possibleRegisters.changeSize(numberOfRealRegisters);
            for (uint i = 0  ; i < m_possibleRegisters.getLength() ; ++i)
                m_possibleRegisters.set(i);
            m_chosenRegister = -1;
            m_isAlive = false;
        }

        bool m_isAlive;
        cSetArray m_possibleRegisters;
        int m_chosenRegister;
    };

    class DummyRegisterEntry : public cArray<Entry>
    {
    public:
        DummyRegisterEntry() :
            m_stackLocation(StackInterface::buildStackLocation(0, 0)),
            m_historyChosenRegister(-1)
        {};

        void resize(int numberOfOperations, int numberOfRealRegisters)
        {
            int oldNumberOfOperations = getSize();
            changeSize(numberOfOperations);
            for (int i = oldNumberOfOperations ; i < numberOfOperations ; ++i)
                (*this)[i].initEntry(numberOfRealRegisters);
        };

        bool isActive(int index)
        {
            return (*this)[index].m_isAlive;
        };

        void setActive(int index, bool val)
        {
            (*this)[index].m_isAlive = val;
        };

        Entry& getLastEntry()
        {
            return (*this)[getSize()-1];
        };

        StackLocation m_stackLocation;
        int m_historyChosenRegister;
    };

    DummyRegistersMapping(int numberOfOperations, int numberOfRealRegisters):
        m_entries(),
        m_numberOfRealRegisters(numberOfRealRegisters),
        m_numberOfOperations(numberOfOperations)
        //m_dummyStackPointerRegister(-(NUMBER_OF_DUMMY_REGISTER+1)) // The first dummy register after the official list of free registers.
    {};

    void initialize()
    {
        cList<int> keys(m_entries.keys());
        cList<int>::iterator keysIter(keys.begin());

        m_numberOfOperations = 0;

        for (; keysIter != keys.end() ; ++keysIter)
        {
            // If there is NO history to keep, remove the key and value from hash
            if (m_entries[*keysIter].m_historyChosenRegister == -1)
            {
                m_entries.remove(*keysIter);
            }
            else // There is history. Only init the array operations.
            {
                m_entries[*keysIter].resize(m_numberOfOperations, m_numberOfRealRegisters);
                m_entries[*keysIter].m_stackLocation = StackInterface::buildStackLocation(0,0);
            }
        }
    }

    void resize(int numberOfOperations)
    {
        cList<int> keys(m_entries.keys());
        cList<int>::iterator keysIter(keys.begin());

        m_numberOfOperations = numberOfOperations;

        for (; keysIter != keys.end() ; ++keysIter)
            m_entries[*keysIter].resize(m_numberOfOperations, m_numberOfRealRegisters);
    };

    /*
     * Gets a register and an index of an operation.
     * Returns the info in the entry (possible register, chosen register etc)
     *    dummyRegister - the dummy register
     *  index - index of mapping info in the list
     */
    Entry& getAtRegister(int dummyRegister, int index)
    {
        CHECK(index <= m_numberOfOperations);

        return m_entries[dummyRegister][index];
    };

    /*
     * Gets a register.
     * Returns the info of the register (start, end and list of constraints).
     *    dummyRegister - the dummy register
     */
    DummyRegisterEntry& getAtRegister(int dummyRegister)
    {
        return m_entries[dummyRegister];
    };

    /*
     * For debugging
     */
    void printTimeline();

    void dontAllowAssignment(int chosenRegister, int index)
    {
        cList<int> dummyRegisters(m_entries.keys());
        cList<int>::iterator dummyRegistersIter(dummyRegisters.begin());

        for (dummyRegistersIter = dummyRegisters.begin();
             dummyRegistersIter!= dummyRegisters.end() ;
             ++dummyRegistersIter)
        {
            DummyRegistersMapping::DummyRegisterEntry &dummyRegisterEntry = m_entries[*dummyRegistersIter];
            // Ignore entries which correspond to registers which are not alive.
            if ((dummyRegisterEntry[index].m_chosenRegister != -1) ||
                (!dummyRegisterEntry.isActive(index)))
                continue;

            // Clear that possible register
            dummyRegisterEntry[index].m_possibleRegisters.clear(chosenRegister);

            // Make sure that the dummy register always has at least one possible real register to be mapped to.
            // If not, we are in a bad situation and cannot assign registers to dummy registers.
            CHECK((dummyRegisterEntry[index].m_possibleRegisters.first() <
                    dummyRegisterEntry[index].m_possibleRegisters.getLength()));
        }
    }

    /*
     * Assign registers to timeline
     */
    void assignRegistersMust(StackLocation baseRegister);
    void assignRegistersExtend();
    void assignRegistersFini();
    void assignRegistersSimple();
    void assignRegistersClever(StackLocation basePointer);
    bool assignRegistersVerify();
    int countSwaps();

    // Entries per dummy register
    cHash<int, DummyRegisterEntry> m_entries;
    // number of real registers in platform
    int m_numberOfRealRegisters;
    // number of operations in block
    int m_numberOfOperations;
    // stack register
    //int m_dummyStackPointerRegister;

};


class OptimizerCompilerInterface : public CompilerInterface
{
public:
    /*
     * Default constructor
     */
    OptimizerCompilerInterface(OptimizerOperationCompilerInterfacePtr _interface);

    // See CompilerInterface::setLocalsSize
    virtual void setLocalsSize(uint localStackSize);
    // See CompilerInterface::setArgumentsSize
    virtual void setArgumentsSize(uint argsSize);

    // See CompilerInterface::getStackSize() Return STACK_32
    virtual StackSize getStackSize() const;
    // See CompilerInterface::getShortJumpLength()
    virtual uint getShortJumpLength() const;


    virtual const FirstPassBinaryPtr& getFirstPassPtr() const;
    virtual FirstPassBinaryPtr& getFirstPassPtr();
    virtual const RegisterAllocationTable& getArchRegisters() const;

    // Overrides CompilerInterface::getStackPointer(). Returns esp
    virtual StackLocation getStackPointer() const;
    // Overrides CompilerInterface::resetBaseStackRegister(). Sets ebp
    virtual void resetBaseStackRegister(const StackLocation& targetRegister);
    // Overrides CompilerInterface::getAlignBuffer(). Retuns a buffer of ia32 NOPs
    virtual cBufferPtr getAlignBuffer() const;

    virtual CallingConvention getDefaultCallingConvention(void) const;

    //////////////////////////////////////////////////////////////////////////
    // Stack operations

    // See CompilerInterface::storeConst
    virtual void storeConst(uint         stackPosition,
                            const uint8* bufferOffset,
                            uint         size,
                            bool         argumentStackLocation = false);
    // See CompilerInterface::store32
    virtual void store32(uint stackPosition,
                         uint size,
                         StackLocation source,
                         bool argumentStackLocation = false,
                         bool isTempStack = false);
    // See CompilerInterface::store32
    virtual void store32(StackLocation source,
                         uint size,
                         bool signExtend,
                         const cString& dependencyName);
    // See CompilerInterface::load32
    virtual void load32(uint stackPosition,
                        uint size,
                        StackLocation destination,
                        bool signExtend = false,
                        bool argumentStackLocation = false,
                        bool isTempStack = false);
    // See CompilerInterface::move32
    virtual void move32(StackLocation destination,
                        StackLocation source,
                        uint size,
                        bool signExtend);
    // See CompilerInterface::load32
    virtual void load32(StackLocation destination,
                        uint size,
                        bool signExtend,
                        const cString& dependencyName);
    // See CompilerInterface::load32addr
    virtual void load32addr(uint stackPosition,
                            uint size,
                            uint offset,
                            StackLocation destination,
                            bool argumentStackLocation = false,
                            bool isTempStack = false);
    virtual void load32addr(StackLocation destination, const cString& dependencyName);
    // See CompilerInterface::loadInt32
    virtual void loadInt32(StackLocation destination,
                           uint32 value);
    virtual void loadInt32(StackLocation destination,
                           const cString& dependancyName);
    // See CompilerInterface::assignRet32
    virtual void assignRet32(StackLocation source);
    // See CompilerInterface::pushArg32
    virtual void pushArg32(StackLocation source);
    // See CompilerInterface::popArg32
    virtual void popArg32(StackLocation source);
    // See CompilerInterface::call
    virtual void call(const cString& dependancyName, uint numberOfArguments);
    virtual void call(StackLocation address, uint numberOfArguments);
    // See CompilerInterface::call32
    virtual void call32(const cString& dependancyName,
                        StackLocation destination, uint numberOfArguments);
    virtual void call32(StackLocation address,
                        StackLocation destination, uint numberOfArguments);
    //////////////////////////////////////////////////////////////////////////
    // Memory handling

    // See CompilerInterface::storeMemory
    virtual void storeMemory(StackLocation destination, StackLocation value, uint offset,
                             uint size);
    //virtual void storeMemory(StackLocation destination, const uint8* bufferOffset,
    //                         uint size)
    // See CompilerInterface::loadMemory
    virtual void loadMemory(StackLocation destination, StackLocation value, uint offset,
                            uint size, bool signExtend = false);



    //////////////////////////////////////////////////////////////////////////
    // Unary operations

    // See CompilerInterface::conv32
    virtual void conv32(StackLocation destination,
                        uint size,
                        bool signExtend);
    virtual void neg32(StackLocation destination);
    virtual void not32(StackLocation destination);

    //////////////////////////////////////////////////////////////////////////
    // Binary operations

    // See CompilerInterface::add32 etc. etc.
    virtual void add32(StackLocation destination, StackLocation source);
    virtual void sub32(StackLocation destination, StackLocation source);
    virtual void mul32(StackLocation destination, StackLocation source);
    virtual void div32(StackLocation destination, StackLocation source, bool sign);
    virtual void rem32(StackLocation destination, StackLocation source, bool sign);
    virtual void and32(StackLocation destination, StackLocation source);
    virtual void xor32(StackLocation destination, StackLocation source);
    virtual void shr32(StackLocation destination, StackLocation source);
    virtual void shl32(StackLocation destination, StackLocation source);
    virtual void or32 (StackLocation destination, StackLocation source);

    virtual void adc32 (StackLocation destination, StackLocation source);
    virtual void sbb32 (StackLocation destination, StackLocation source);
    virtual void mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign);

    // See CompilerInterface::addConst32
    virtual void addConst32(const StackLocation destination,
                            int32 value);

    //////////////////////////////////////////////////////////////////////////
    // Conditional execution

    // See CompilerInterface::jump.s
    virtual void jump(int blockID);
    virtual void jumpShort(int blockID);


    // See CompilerInterface::jump.s.XXX
    virtual void jumpCond(StackLocation compare, int blockID, bool isZero);
    virtual void jumpCondShort(StackLocation compare, int blockID, bool isZero);

    //////////////////////////////////////////////////////////////////////////
    // Comparison methods

    // See CompilerInterface::cXX32
    virtual void ceq32(StackLocation destination, StackLocation source);
    virtual void cgt32(StackLocation destination, StackLocation source, bool isSigned);
    virtual void clt32(StackLocation destination, StackLocation source, bool isSigned);

    //////////////////////////////////////////////////////////////////////////
    // Memory handling

    // See CompilerInterface::localloc
    // TODO! Refactor this function. Create a base class and implement this
    //       functionally. NOTE: This method touches the generateMethodEpiProLogs
    //       method
    virtual void localloc(StackLocation destination, StackLocation size,
                          bool isStackEmpty);

    // See CompilerInterface::revertStack
    virtual void revertStack(uint32 size);

    // See CompilerInterface::setFramePointer
    virtual void setFramePointer(StackLocation destination);

    // See CompilerInterface::sealFirstPassBinary
    virtual void renderBlock();

    // See CompilerInterface::getCompilerParameters
    virtual const CompilerParameters& getCompilerParameters() const;

    virtual CompilerInterface* getInnerCompilerInterface();

protected:
    // See CompilerInterface::generateMethodEpiProLogs
    virtual void generateMethodEpiProLogs(bool bForceSaveNonVolatiles = false);

    /* Updated the start and end indexes according to the live registers at this moment */
    void udpateRegisterStartEndIndexes();

    /* Updated the register mapping according to the operation */
    void updateDummyRegistersMapping(CompilerInterface::CompilerOperation opcode, int operationIndex);

    OptimizerOperationCompilerInterfacePtr m_interface;
    // Block operations
    BlockOperationList m_blockOperations;
    // A data structure to hold which register is alive at which point.
    DummyRegistersMapping m_dummyRegistersMapping;

    uint m_offsetTempStack;

    /*
     * See CompielrInterface::isOptimizerCompiler
     */
    virtual bool isOptimizerCompiler() const;

private:
    enum {
        // Order MATTERS!!! Default allocation is from smaller number (Volatile).
        START_NON_VOLATILE_REGISTERS = -1,
        END_NON_VOLATILE_REGISTERS = -32,

        START_VOLATILE_REGISTERS = -32,
        END_VOLATILE_REGISTERS = -64,

          // Dummy register for base stack pointer (fix in m_indexToRegister.append(-(64 + 1), getGPEncoding(ia32dis::IA32_GP32_EBP));)
        INITIAL_BASE_STACK_REGISTER = -64,
    };

    void mapRegisters(StackLocation basePointer);

    int updateRegisterByMapping(BlockOperationList::iterator operationsPosition, int operationIndex, int dummyRegister);
    void backupRegisters(BlockOperationList::iterator operationsPosition, int operationIndex);

    void handleModifications(BlockOperationList::iterator operationsPosition, int operationIndex);
    void doOperation(CompilerInterface::CompilerOperation &operation);

    void updateTouchedRegisters();

    void saveHistoryForNextBlock();

    StackLocation storeRegisterToStackLocation(BlockOperationList::iterator operationsPosition, int realRegister);
    void loadRegisterFromStackLocation(BlockOperationList::iterator operationsPosition, StackLocation stackLocation, int realRegister);
    void switchRegisters(BlockOperationList::iterator operationsPosition, int srcRealRegister, int dstRealRegister);

    // Optimization operations
    void removeUnnecessaryLoad();
    void removeUnnecessaryMove();
};

#endif // __TBA_CLR_COMPILER_OPTIMIZERCOMPILERINTERFACE_H