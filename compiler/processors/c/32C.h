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

#ifndef __TBA_CLR_COMPILER_PROCESSORS_C_32C_H
#define __TBA_CLR_COMPILER_PROCESSORS_C_32C_H

/*
 * 32C.h
 *
 * MSIL to C (32 bit based) application
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/stringerStream.h"
#include "compiler/CompilerInterface.h"
#include "compiler/StackEntity.h"
#include "dismount/assembler/FirstPassBinary.h"

/*
 * A compiled application is going to be look like:
 *
 * Using "1000 int registers" (r0...r999)
 * No temporary registers.
 * All local variables will be implemented in a single stack variables
 * Arguments/push arguments will be:
 *
 *
 * Examples:
 *
 *    int fib(int a0)
 *    {
 *        unsigned char locals[LOCAL_SIZE];
 *        int r0, r1, r2, r3...rXXX;
 *        void* rbp = locals;
 *
 *        // if (arg0 < 2) return 1;
 *        r0 = a0;
 *        r2 = 1;
 *        r3 = 2;
 *        r1 = r0 < r3;
 *        if (r1) goto b53:
 *
 *        // fib(a - 1)
 *        r0-= r2;
 *           // push arg(r0)
 *           ca0 = r0;
 *           // call
 *           fib(ca0);
 *        ..
 *     b53:
 *        return r2;
 *    }
 *
 * Author: Elad Raz
 * 2012
 */
class c32CCompilerInterface : public CompilerInterface {
public:
    /*
     * Default constructor
     */
    c32CCompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params);

    // See CompilerInterface::setLocalsSize
    virtual void setLocalsSize(uint localStackSize);
    // See CompilerInterface::setArgumentsSize
    virtual void setArgumentsSize(uint argsSize);

    // See CompilerInterface::getStackSize() Return STACK_32
    virtual StackSize getStackSize() const;
    // See CompilerInterface::getShortJumpLength()
    virtual uint getShortJumpLength() const;

    // Overrides CompilerInterface::getStackPointer().
    virtual StackLocation getStackPointer() const;

    // Overrides CompilerInterface::resetBaseStackRegister().
    virtual void resetBaseStackRegister(const StackLocation& targetRegister);

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
    virtual void load32addr(StackLocation destination,
                            const cString& dependencyName);
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
                        StackLocation destination,
                        uint numberOfArguments);
    virtual void call32(StackLocation address,
                        StackLocation destination,
                        uint numberOfArguments);
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
    virtual void mul32h(StackLocation destlow,     StackLocation desthigh, StackLocation source, bool sign);

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

    /////////////////////////////////////////////////
    /// Get export names

    /*
     * Translate dependencyName to C function name
     */
    static cString getFunctionName(const TokenIndex& token);

    /*
     * Return the globals name
     */
    static cString getGlobalsName();

    /*
     * Convert size into a type or pointer
     * size - the size in bytes
     * ptr - true for pointer type, false for type
     */
    static cString getSizeCast(uint size, bool ptr);

protected:
    // See CompilerInterface::generateMethodEpiProLogs
    virtual void generateMethodEpiProLogs(bool bForceSaveNonVolatiles = false);

private:
    /*
     *
     */
    class cCFirstBinaryStream : public cStringStream
    {
    public:
        /*
         * Constructor. Get FirstBinaryPass and add instruction set into it.
         */
        cCFirstBinaryStream(FirstPassBinaryPtr& binary);

    protected:
        /*
         * Store the compilation output into the first binary stream
         */
        virtual void outputString(const cString& string);

    private:
        // Use only inside the class, assumes FirstPassBinary will always exists.
        FirstPassBinary& m_binary;
    };

    /*
     * Translates between GP register index and an encoded FirstBinaryPass value
     */
    enum {
            NUMBER_OF_REGISTER = 99,
            REGISTER_BASE_POINTER = NUMBER_OF_REGISTER+1
         };

    /*
     * Get the locations of arguments/locals
     */
    cString getStackReference124(uint stackPosition,
                                 uint size,
                                 bool argumentStackLocation,
                                 bool shouldReference=true);
    cString getTempStack32(uint stackPosition, bool derefAddr = false);

    cString getRegsiterName(StackLocation reg);

    /*
     * Invoke all calling arguments until m_currentArgs
     */
    void callArgs(cCFirstBinaryStream& compiler, uint numberOfArguments, bool isPrototype = false);

    /*
     * Translate dependencyName to C function name
     */
    cString getTokenName(const cString& dependencyName);

    // The number of push arguments
    uint m_currentArgs;
    // The maximum number of pushed arguments
    uint m_maximumNumberOfArgs;
};

#endif // __TBA_CLR_COMPILER_PROCESSORS_C_32C_H
