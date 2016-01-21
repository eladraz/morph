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

#ifndef __TBA_CLR_COMPILER_PROCESSORS_IA32_IA32COMPILERINTERFACE_H
#define __TBA_CLR_COMPILER_PROCESSORS_IA32_IA32COMPILERINTERFACE_H

/*
 * IA32CompilerInterface.h
 *
 * Native Compiler for x86 machine.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "compiler/OptimizerOperationCompilerInterface.h"
#include "compiler/StackEntity.h"
#include "dismount/assembler/AssemblerInterface.h"
#include "dismount/assembler/FirstPassBinary.h"
#include "dismount/proc/ia32/IA32eInstructionSet.h"

/*
 * Here is a simple view of the x86 method stack:
 *
 *   FFFFFFFF    +-+-+-+-+-+-+-+-+--++-+
 *  Upper mem    |  Argument  (n)      |
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               |  Argument  (n-1)    |
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               ~       ...           ~
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               |  Argument  (1)      |
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               |  Argument (0/this)  |
 *               +-+-+-+-+-+-+-+-+-+-+-+   <- [ebp+4+index]
 *   Init        |  x86 previous EIP   |
 *   State  -->  +-+-+-+-+-+-+-+-+-+-+-+   <- EBP
 *               | Saved EBP           |
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               |                     |
 *               ~      Locals         ~   <- [ebp-4-index]
 *               |                     |
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *               |    Normal stack     |
 *
 * When a function called there are several of ways to manager register usage,
 * such standard methods (fastcall, stdcall, cdecl).
 *
 * I decided a calling-conventions equals to the ia64 standard which is similar
 * to the fast-call conventions:
 *
 *    Volatile registers: eax, ebx, edx, edi
 *    Saved registers:    ecx, ebp, esp, esi
 *
 *    See saveNonVolatileRegisters()
 *    See saveVolatileRegisters()
 *
 *
 * Arguments passed through stack (not enough registers for fast arguments
 * passing)
 *
 * Returned value are stored in (according to type):
 *   8 bit, 16 bit, 32 bit into eax.
 *   64 bit                     eax:edx.
 */
class IA32CompilerInterface : public OptimizerOperationCompilerInterface {
public:
    /*
     * Default constructor
     */
    IA32CompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params);

    // See CompilerInterface::setLocalsSize
    virtual void setLocalsSize(uint localStackSize);
    // See CompilerInterface::setArgumentsSize
    virtual void setArgumentsSize(uint argsSize);

    // See CompilerInterface::getStackSize() Return STACK_32
    virtual StackSize getStackSize() const;
    // See CompilerInterface::getShortJumpLength()
    virtual uint getShortJumpLength() const;

    // Overrides CompilerInterface::getStackPointer(). Returns esp
    virtual StackLocation getStackPointer() const;
    // Overrides CompilerInterface::resetBaseStackRegister(). Sets ebp
    virtual void resetBaseStackRegister(const StackLocation& targetRegister);
    // Overrides CompilerInterface::getAlignBuffer(). Retuns a buffer of ia32 NOPs
    virtual cBufferPtr getAlignBuffer() const;
    // Overrides CompilerInterface:::setFramePointer(). "mov" from ebp to destination
    virtual void setFramePointer(StackLocation destination);

    virtual RegisterAllocationInfo getOperationRegisterAllocationInfo(CompilerInterface::CompilerOperation& operation);

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
    // See CompilerInterface::move32
    virtual void move32(StackLocation destination,
                          StackLocation source,
                        uint size,
                        bool signExtend);
    // See CompilerInterface::load32
    virtual void load32(uint stackPosition,
                        uint size,
                        StackLocation destination,
                        bool signExtend = false,
                        bool argumentStackLocation = false,
                        bool isTempStack = false);
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
    // Internal implementation of div32 and rem32. The algorithm is the same
    // the return register is the different
    void internalDiv32(StackLocation destination, StackLocation source,
                       bool sign, bool isDiv);

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

protected:
    // See CompilerInterface::generateMethodEpiProLogs
    virtual void generateMethodEpiProLogs(bool bForceSaveNonVolatiles = false);

private:
    typedef cHash<int, StackLocation> RegToLocation;

    /*
     * This method make sure that a specific register is free to use. If not
     * the method will allocate a temporary stack based variable and store the
     * value of the register inside the new allocated block.
     *
     * gpreg - The register to free
     *
     * For example: calling this method with argument (ia32dis::IA32_GP32_EAX)
     *              will make sure that if this register is occupy it will be
     *              copied into a stack variable.
     */
    void freeRegister32(int gpreg, cStringerStream& compiler);

    /*
     * Returns the name of the base register for locals/arguments
     */
    static const char* getBaseStackRegister(StackLocation baseRegister);

    /*
     * Return a string reference into stack variable of sizes 1, 2, 4.
     *  For size 1 the method will return "byte ptr [ebp+stackPosition]"
     *  For size 2 the method will return "word ptr [ebp+stackPosition]"
     *  For size 4 the method will return "dword ptr [ebp+stackPosition]"
     *
     * All other sizes will cause an exception to be thrown
     */
    cString getStackReference124(uint stackPosition,
                                 uint size,
                                 bool argumentStackLocation);

    /*
     * Return true if 'destination' is a 32 bit register.
     * Return false otherwise
     */
    static bool isRegister32(StackLocation destination);

    /*
     * Return true if 'destination' has an 8 bit portion.
     * Return false otherwise
     */
    static bool isRegister8(StackLocation destination);

    /*
     * Return a string referring into a temporary stack register at the size of
     * the local register size (32 bit)
     */
    cString getTempStack32(uint stackPosition, uint size);

    /*
     * Return the name of a 32/16/8 bit register according to the register index
     */
    static const char* getRegister32(StackLocation destination);
    static const char* getRegister16(StackLocation destination);
    static const char* getRegister8(StackLocation destination);

    /*
     * Save the non-volatile registers which MUST save across method calls:
     *    ecx, esi.         NOTE: The ebp, esp are not allocated...
     *
     * compiler   - The compiler to save the instruction to.
     * bSave      - Set to true in order to save the touched registers
     *              Set to false in order to restore the touched registers
     * bAlways    - Set to true to save nonvolatile registers even if not touched
     *              false for default behavior (e.g. only if touched)
     */
    void saveNonVolatileRegisters(cStringerStream& compiler,
                                  RegToLocation& locationMap,
                                  bool bSave = true,
                                  bool bAlways = false);

    /*
     * Push all volatile registers before calling other methods
     *
     * compiler - The code to be output
     * saveRegisterX - Register which is occupied for the result.
     *                 NOTE: These value should be transfer in GP encoding
     */
    void saveVolatileRegisters(int saveRegister1 = 0,
                               int saveRegister2 = 0,
                               int saveRegister3 = 0);


    // Function specific information
    //       NOTE: All of the following members can be implemented as the
    //             FirstBinary private data. This should be useful when encoding
    //             will be in use.

};

#endif // __TBA_CLR_COMPILER_PROCESSORS_IA32_IA32COMPILERINTERFACE_H
