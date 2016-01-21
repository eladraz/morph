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

#ifndef __TBA_CLR_COMPILER_COMPILERINTERFACE_H
#define __TBA_CLR_COMPILER_COMPILERINTERFACE_H

/*
 * CompilerInterface.h
 *
 * Interface for compilation routines.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/setArray.h"
#include "xStl/data/smartptr.h"
#include "dismount/assembler/FirstPassBinary.h"
#include "dismount/assembler/AssemblerInterface.h"
#include "runnable/FrameworkMethods.h"

/* Data structure for register allocation data for every function */
class RegisterAllocationInfo
{
public:
    cSetArray m_acceptableSource;
    cSetArray m_acceptableDest;
    cSetArray m_modifiable;
    bool m_isDestAlsoSource;

    RegisterAllocationInfo(uint numberOfRegisters) :
        m_acceptableSource(numberOfRegisters, true, true),
        m_acceptableDest(numberOfRegisters, true, true),
        m_modifiable(numberOfRegisters, true, false),
        m_isDestAlsoSource(false)
    {};
};

// The reference countable object
typedef cSmartPtr<class CompilerInterface> CompilerInterfacePtr;

//Maximum size for constant in store const
#define STORE_CONST_MAX_SIZE (8)

/*
 * This struct defines all common compiler parameters which allow the user to tweak the compilation
 */
struct CompilerParameters
{
    // If this flag is set, exception handling is supported.
    // If this flag is not set, any exception-handling MSIL code will cause the compiler engine to fail
    bool m_bSupportExceptionHandling;

    // If this flag is set, compiler optimizations are enabled
    // If this flag is not set, the arch-compiler is invoked directly by the engine
    bool m_bEnableOptimizations;

    // If this flag is set, extra developer-only traces and output's will be generated (applicable to debug builds only)
    // If this flag is not set, only regular build traces are output
    bool m_bDeveloperVerbosity;
};

/*
 * This interface should be implemented by the different platforms.
 */
class CompilerInterface {
public:
    // You can inherit from me
    virtual ~CompilerInterface();

    /*
     * Constructor. Initialize the compiler and attach it to a framework
     * params - The compiler parameters to use
     * Framework calls constructor.
     */
    CompilerInterface(const FrameworkMethods& framework, const CompilerParameters& params);

    //////////////////////////////////////////////////////////////////////////
    // Generic method attributes & information

    /*
     * Change the size of the local variables.
     * NOTE: This variable can be changed until the function is sealed.
     *
     * localStackSize - The number of bytes for the initialize stack allocated
     *                  for locals
     */
    virtual void setLocalsSize(uint localStackSize) = 0;

    /*
     * Change the size of the arguments.
     *
     * This function must be called at the beginning of the method-compiler
     */
    virtual void setArgumentsSize(uint argsSize) = 0;

    /*
     * Moves the base pointer value from the current base stack register back to the default one
     */
    virtual void resetBaseStackRegister(const StackLocation& targetRegister) = 0;

    /*
     * Create new initialized first binary pass, setup for the current compiler
     * settings.
     */
    virtual const FirstPassBinaryPtr& getFirstPassPtr() const;
    virtual FirstPassBinaryPtr& getFirstPassPtr();

    /*
     * Return the method initialized free registers pool.
     */
    virtual const RegisterAllocationTable& getArchRegisters() const;

    //////////////////////////////////////////////////////////////////////////
    // Stack operations

    // The processor stack size
    enum StackSize {
        // The processor stack alignment stand on 32bit
        STACK_32 = 4,
        // The processor stack alignment stand on 64bit
        STACK_64 = 8
    };

    // The calling convention
    enum CallingConvention {
        STDCALL,
        C_DECL,
        FASTCALL,
    };

    /*
     * Return the current processor stack size. The stack-size must be aligned
     * into native machine pointer. The pointer must
     */
    virtual StackSize getStackSize() const = 0;

    /*
     * Return the thershold between short jump and long jumps
     */
    virtual uint getShortJumpLength() const = 0;

    /*
     * Returns the default general-purpose stack pointer register
     * Note: the return value from this method should NOT change as
     * a result of a call to setBaseStackRegister()
     */
    virtual StackLocation getStackPointer() const = 0;

    /*
     * Return compiler choice for calling convention
     */
    virtual CallingConvention getDefaultCallingConvention(void) const;

    /*
     * Get the padding data for aligning methods in memory
     */
    virtual cBufferPtr getAlignBuffer() const;

    enum Operation {
        OPCODE_STORE_CONST, //0
        OPCODE_STORE_32, //1
        OPCODE_STORE_32_DEPENDENCY, //2
        OPCODE_MOVE_32, //3
        OPCODE_LOAD_32, //4
        OPCODE_LOAD_32_DEPENDENCY, //5
        OPCODE_LOAD_ADDR_32, //6
        OPCODE_LOAD_ADDR_32_DEPENDENCY, //7
        OPCODE_LOAD_INT_32, // 8
        OPCODE_LOAD_INT_32_DEPENDENCY, //9
        OPCODE_ASSIGN_RET_32, //10
        OPCODE_PUSH_ARG_32, //11
        OPCODE_POP_ARG_32, //12
        OPCODE_CALL, //13
        OPCODE_CALL_DEPENDENCY, //14
        OPCODE_CALL_32, //15
        OPCODE_CALL_32_DEPENDENCY, //16
        OPCODE_STORE_MEMORY, //17
        OPCODE_LOAD_MEMORY, //18
        OPCODE_CONV_32, //19
        OPCODE_NEG_32, //20
        OPCODE_NOT_32, //21
        OPCODE_ADD_32, //22
        OPCODE_SUB_32, //23
        OPCODE_MUL_32, //24
        OPCODE_DIV_32, //25
        OPCODE_REM_32, //26
        OPCODE_AND_32, //27
        OPCODE_XOR_32, //28
        OPCODE_SHR_32, //29
        OPCODE_SHL_32, //30
        OPCODE_OR_32, //31
        OPCODE_ADC_32, //32
        OPCODE_SBB_32, //33
        OPCODE_MUL_32H, //34
        OPCODE_ADD_CONST_32, //35
        OPCODE_JUMP, //36
        OPCODE_JUMP_SHORT, //37
        OPCODE_JUMP_COND, //38
        OPCODE_JUMP_COND_SHORT, //39
        OPCODE_CEQ_32, //40
        OPCODE_CGT_32, //41
        OPCODE_CLT_32,  //42
        OPCODE_LOCALLOC,  //43
        OPCODE_REVERT_STACK,  //44
        OPCODE_RESET_BASE_STACK_REGISTER, // 45
        OPCODE_SET_FRAME_POINTER
    };

    class CompilerOperation
    {
    public:
        CompilerOperation(
            CompilerInterface::Operation opcode,
            uint uval1,
            uint uval2,
            int val,
            uint size,
            StackLocation sloc1,
            StackLocation sloc2,
            bool cond1,
            bool cond2,
            bool cond3,
            const cString& dependencyName,
            const uint8* buf) :
                opcode(opcode),
                uval1(uval1), uval2(uval2), val(val), size(size),
                sloc1(sloc1), sloc2(sloc2),
                cond1(cond1), cond2(cond2), cond3(cond3),
                dependencyName(dependencyName)
            { if (buf != 0) {
                memcpy (buffer, buf, STORE_CONST_MAX_SIZE);
                }
            };

        CompilerInterface::Operation opcode;
        uint uval1;
        uint uval2;
        int val;
        uint size;
        StackLocation sloc1;
        StackLocation sloc2;
        bool cond1;
        bool cond2;
        bool cond3;
        cString dependencyName;
        uint8 buffer[STORE_CONST_MAX_SIZE];
    };

    class Register {

    };

    /*
     * Store a const value into the stack.
     *
     * stackPosition - The position inside the stack of which the object should
     *                 store
     * bufferOffset  - The data to store
     * size          - Number of bytes of 'bufferOffset'.
     *                 NOTE: Size can be only one of the following: 1,2,4,8 any
     *                       other value will cause in exception
     * argumentStackLocation - Set to true if the stack element is located at
     *                         the arguments pool
     *                         Set to false if the stack element is located at
     *                         the locals pool
     */
    virtual void storeConst(uint         stackPosition,
                            const uint8* bufferOffset,
                            uint         size,
                            bool argumentStackLocation = false) = 0;

    /*
     * Stores a 32bit register(s) value into the stack.
     *
     * stackPosition - The position inside the stack of which the object should
     *                 store
     * size          - Number of bytes of 'bufferOffset'.
     *                 NOTE: Size can be only one of the following: 1,2,4 any
     *                       other value will cause in exception
     * isTempStack  - Set to true if "stackPosition" is actually tempraryStack position
     */
    virtual void store32(uint stackPosition,
                         uint size,
                         StackLocation source,
                         bool argumentStackLocation = false,
                         bool isTempStack = false) = 0;
    /*
     * Store a 32bit register into a reference value with a dependency name
     *
     * destination   - The register(s) of which the value is stored.
     * size          - The size of the value in the stack.
     *                 0 indicates that this is a stack stored register.
     * signExtend    - Set to true if the stack variable is sign, and should be
     *                 converted as sign 32 bit value.
     */
    virtual void store32(StackLocation source,
                         uint size,
                         bool signExtend,
                         const cString& dependencyName) = 0;


    /*
     * Load sign/unsigned integer from stack into a register(s) as 32 bit value.
     *
     * stackPosition - The location in the stack
     * size          - The size of the value in the stack.
     *                 0 indicates that this is a stack stored register.
     * destination   - The register(s) of which the value is stored.
     * signExtend    - Set to true if the stack variable is sign, and should be
     *                 converted as sign 32 bit value.
     * argumentStackLocation - Set to true if the stack element is located at
     *                         the arguments pool
     *                         Set to false if the stack element is located at
     *                         the locals pool
     * isTempStack  - Set to true if "stackPosition" is actually tempraryStack position
     */
    virtual void load32(uint stackPosition,
                        uint size,
                        StackLocation destination,
                        bool signExtend = false,
                        bool argumentStackLocation = false,
                        bool isTempStack = false) = 0;

    /*
     * move a register to a register.
     *
     * source        - The register(s) of which the value is loaded.
     * destination   - The register(s) of which the value is stored.
     * size          - The size of the register.
     * signExtend    - Set to true if the register is sign, and should be
     *                 converted as sign 32 bit value.
     */
    virtual void move32(StackLocation destination,
                        StackLocation source,
                        uint size,
                        bool signExtend) = 0;

    /*
     * Load reference value to a 32bit register with a dependency name
     *
     * destination   - The register(s) of which the value is stored.
     * size          - The size of the value in the stack.
     *                 0 indicates that this is a stack stored register.
     * signExtend    - Set to true if the stack variable is sign, and should be
     *                 converted as sign 32 bit value.
     */
    virtual void load32(StackLocation destination,
                        uint size,
                        bool signExtend,
                        const cString& dependencyName) = 0;


    /*
     * Load a pointer of sign/unsigned integer from stack into a
     * register(s) as 32 bit value.
     *
     * stackPosition - The location in the stack
     * size          - The size of the value in the stack.
     *                 0 indicates that this is a stack stored register.
     * offset        - The position inside the struct to load the value to
     * destination   - The register(s) of which the value is stored.
     * argumentStackLocation - Set to true if the stack element is located at
     *                         the arguments pool
     *                         Set to false if the stack element is located at
     *                         the locals pool
     */
    virtual void load32addr(uint stackPosition,
                            uint size,
                            uint offset,
                            StackLocation destination,
                            bool argumentStackLocation = false,
                            bool isTempStack = false) = 0;
    virtual void load32addr(StackLocation destination, const cString& dependencyName) = 0;

    /*
     * Load sign/unsigned const integer into a register(s) as 32 bit integer
     * value
     *
     * destination   - The register(s) of which the value is stored.
     * value         - Un/Signed number
     */
    virtual void loadInt32(StackLocation destination,
                           uint32 value) = 0;
    virtual void loadInt32(StackLocation destination,
                           const cString& dependencyName) = 0;

    /*
     * Assign 'source' into the returned value register. This function should
     * assum ethat the only stack variable is 'source' and it's must be
     * converted into the return value register/address.
     *
     * source - The returned value.
     *
     * NOTE: The return value will not be freed.
     */
    virtual void assignRet32(StackLocation source) = 0;

    /*
     * Push 32 bit method argument
     *
     * object - The argument to push for the next method
     */
    virtual void pushArg32(StackLocation source) = 0;

    /*
     * Pop entry from stack
     */
    virtual void popArg32(StackLocation source) = 0;

    /*
     * Write a 'call' instruction into a method which doesn't got return type.
     *
     * dependencyName - The method to invoke. Encoded as a remote dependency for
     *                  linker to resolve.
     * numberOfArguments - The number of arguments pushArg32 has been called
     */
    virtual void call(const cString& dependencyName, uint numberOfArguments) = 0;
    virtual void call(StackLocation address, uint numberOfArguments) = 0;

    /*
     * Write a 'call' instruction into a method which it's returns type is at
     * most 32 bit.
     *
     * dependencyName - The method to invoke. Encoded as a remote dependency for
     *                  linker to resolve.
     * destination    - Will be filled with the return value
     * numberOfArguments - The number of arguments pushArg32 has been called
     */
    virtual void call32(const cString& dependncyName,
                        StackLocation destination,
                        uint numberOfArguments) = 0;
    virtual void call32(StackLocation address,
                        StackLocation destination,
                        uint numberOfArguments) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Memory handling

    /*
     * Write 'Value' into memory location [destination].
     *    *((size*)destination + offset) = (size)value;
     *
     * destination    - The memory address to store the value to
     * value          - The register holding the written value
     * size           - The number of bytes used for this write transaction
     * bufferOffset   - Write const value to the memory
     */
    virtual void storeMemory(StackLocation destination,
                             StackLocation value,
                             uint offset,
                             uint size) = 0;
    /*
    virtual void storeMemory(StackLocation destination,
                             const uint8* bufferOffset,
                             uint size) = 0;
     */

    // Optimization note:
    //   Add the following operators:
    //      store(destination, offset, value) -> [destination + offset] = value;
    //      store(constdest,   value)         -> [constdest] = value;

    /*
     * Read memory 'destination' and places the result into 'value'
     *      (size)value = *((size*)destination + offset);
     *
     * destination    - The memory address to read the value from
     * value          - The register that will be filled with the memory value
     * offset         - Offset inside the buffer (in bytes)
     * size           - The number of bytes used for this read transaction
     */
    virtual void loadMemory(StackLocation destination,
                            StackLocation value,
                            uint offset,
                            uint size,
                            bool signExtend = false) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Unary operations

    /*
     * Convert a 32 bit register(s) into specific data value.
     *
     * destination - The register to convert. Used as result holder
     * size        - New format size.
     * signExtend  - Set to true if the sign bit should be consider as well.
     *
     * Examples: Converting int32 -1 value (0xFFFFFFFF) into size 2, sign extend
     *           will cause the following: 0xFFFF.
     */
    virtual void conv32(StackLocation destination,
                        uint size,
                        bool signExtend) = 0;

    /*
     * Net/Not a 32 bit register(s).
     *
     * destination - The register to convert. Used as result holder
     *
     * The difference between neg and not is:
     *  neg = -destination
     *  not = bit flip of destination
     */
    virtual void neg32(StackLocation destination) = 0;
    virtual void not32(StackLocation destination) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Binary operations.

    /*
     * Perform binary operation between two stack registers. The result will
     * uses the 32 bit value and round-rip the value into 32 bit boundaries. The
     * result stores back at "destination" register
     *
     * The result can be translated as:
     *    destination = destination (+,-,*,/,%,&,|,^,<<,>>) source
     *
     * Operations:
     *    add32 - Adding (+)
     *    sub32 - Substraction (-)
     *    mul32 - Multiply (*)
     *    div32 - Divide (/)
     *    rem32 - Reminder (%)
     *    and32 - Bitwise and (&)
     *    or32  - Bitwise or (|)
     *    xor32 - Bitwise eXclusive or (^)
     *    shl32 - Bitwise shift to left (>>)
     *    shr32 - Bitwise shift to right (<<)
     *
     * destination - The 'a' variable. Used as result holder
     * source      - The 'b' variable.
     * sign        - Set to true if the operation should consider the two
     *               integers as signed. False otherwise
     *
     * OPTIMIZATION NOTE: Some platforms uses can extend ALU operation by using
     *                    source which lays on the stack. This optimization is
     *                    not yet implemented.
     */
    virtual void add32(StackLocation destination, StackLocation source) = 0;
    virtual void sub32(StackLocation destination, StackLocation source) = 0;
    virtual void mul32(StackLocation destination, StackLocation source) = 0;
    virtual void div32(StackLocation destination, StackLocation source, bool sign) = 0;
    virtual void rem32(StackLocation destination, StackLocation source, bool sign) = 0;
    virtual void and32(StackLocation destination, StackLocation source) = 0;
    virtual void xor32(StackLocation destination, StackLocation source) = 0;
    virtual void shr32(StackLocation destination, StackLocation source) = 0;
    virtual void shl32(StackLocation destination, StackLocation source) = 0;
    virtual void or32 (StackLocation destination, StackLocation source) = 0;

    /*
     * Advance opcodes
     *
     * adc32   - Adding with carry (+c+). Adding a number with the carry remains in the previous operation
     * sbb32   - Sub with barrow (-c-)
     * mul32hs - mul32 signned and export the higher 33:64 bit into a register (desthigh:destlow = destlow*source)
     */
    virtual void adc32 (StackLocation destination, StackLocation source) = 0;
    virtual void sbb32 (StackLocation destination, StackLocation source) = 0;
    /* ILANK: Ignoring this method for optimization at first step. FIX later */
    virtual void mul32h(StackLocation destlow, StackLocation desthigh, StackLocation source, bool sign) = 0;

    /*
     * Perform add operation between stack register(s) and const integer 32. The
     * integer can be either sign or unsigned. The result is stored at the
     * "destination" register.
     *
     * The result can be translated as:
     *    destination = destination + value
     *
     * destination  - The register to be add to
     * value        - The value to be added
     *
     * OPTIMIZATION NOTE: For some platforms, if 'value' is 1 INC instruction is
     *                    used instead.
     */
    virtual void addConst32(const StackLocation destination,
                            int32 value) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Conditional execution

    // Jump always

    /*
     * Generate jump into 'blockID' in normal form (long) or with the Short
     * postfix to write the small form (-128...127 bytes long)
     *
     * blockID - The block to jump to. Mangled and add into the dependency-tree
     */
    virtual void jump(int blockID) = 0;
    virtual void jumpShort(int blockID) = 0;

    // Jump on zero or non-zero

    /*
     * Generate condition jump into 'blockID' in normal form (long)  or with the
     * Short postfix to write the small form (-128...127 bytes long.
     * The condition are register is zero or register is not zero.
     *
     * compare - The register to compare to (zero or non-zero)
     * blockID - The block to jump to. Mangled and add into the dependency-tree
     * isZero  - Set to true if the condition set to true when 'compare' is zero
     *           Set to false if condution should be taken for any non-zero
     *           values
     */
    virtual void jumpCond(StackLocation compare, int blockID, bool isZero) = 0;
    virtual void jumpCondShort(StackLocation compare, int blockID, bool isZero) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Comparison methods

    /*
     * Perform binary comparison operation between two stack registers. The
     * result will be stored in 'destinaion'. For unsigned operations the
     * 'isSigned' argument must be set to false.
     *
     * The result can be translated as:
     *    destination = destination (==,!=,<,>,<=,>=) source
     * Where 1 means true and 0 means false.
     *
     * Operations:
     *    ceq32 - Compare equals (==)
     *    cgt32 - Compare greater then (>)
     *    clt32 - Compare less then (<)
     *
     * destination - The 'a' variable. Used as result holder
     * source      - The 'b' variable.
     * isSigned    - Set to true if the operation reffering to unsigned numbers
     *
     * OPTIMIZATION NOTE: Store the result of the operation in the processor's
     *                    flag register and expand the jumpCondXXX to take in
     *                    account for that special case (And uses special
     *                    condition).
     *                    Problem may occurred if the flag result will be used
     *                    later on, and overlapped by another instruction.
     */
    virtual void ceq32(StackLocation destination, StackLocation source) = 0;
    virtual void cgt32(StackLocation destination, StackLocation source, bool isSigned) = 0;
    virtual void clt32(StackLocation destination, StackLocation source, bool isSigned) = 0;

    //////////////////////////////////////////////////////////////////////////
    // Memory handling

    /*
     * Allocate dynamic memory from the function dynamic space.
     *
     * destination  - Will be filled with the pointer to the buffer.
     *                NOTE: This value must be a register
     * size         - The size of the memory to be allocated
     * isStackEmpty - Set to true if the current stack is empty
     *
     * See Page 76 of the .NET Partition 3 (CIL) - localloc instruction for
     * implementation details.
     */
    virtual void localloc(StackLocation destination, StackLocation size,
                          bool isStackEmpty) = 0;

    /*
     * Free `size` bytes from the stack.
     * Should be called after return from a cdecl call
     *
     * size - number of bytes to free from the stack
     */
    virtual void revertStack(uint32 size) = 0;

    /*
     * Returns the current compiler parameters for this compiler.
     */
    virtual const CompilerParameters& getCompilerParameters() const;

    /* Seal */
    virtual void renderBlock();

    virtual void setFramePointer(StackLocation destination);

    virtual void enableOptimizations();
    virtual void disableOptimizations();
    virtual bool isOptimizerOn() const;

    /*
     * Well-defined defaults for all compiled parameters
     */
    static const CompilerParameters defaultParameters;

    /*
     * Translates between GP register index and an encoded FirstBinaryPass value
     */
    enum { GP32_BASE = 0x100 };
    static int getGPEncoding(int gpreg);

    /*
     * Returns true if the compiler is optimizer and false otherwise.
     */
    virtual bool isOptimizerCompiler() const;

    /* Get the inner compiler used.
     * In case of optimizer, it returns the inner compiler
     * In case no optimizer, return itself.
     */
    virtual CompilerInterface* getInnerCompilerInterface();

    /*
     * Return the amount of registers in the specific platform
     */
    virtual uint getNumberOfRegisters();

    /*
     * Returns the base stack register in the current block
     */
    StackLocation getMethodBaseStackRegister() const;

    /*
     * Sets the base stack register in the current block
     */
    void setMethodBaseStackRegister(StackLocation stackLocation);

private:
    // Everyone need a friend
    friend class MethodCompiler;
    friend class OptimizerCompilerInterface;
    friend class OptimizerOperationCompilerInterface;

    /*
     * Conclude the method. Add prolog and epilog to the method according to
     * stack-size and exception information.
     *
     * bForceSaveNonVolatiles - true to force saving all nonvolatile registers, even
     *      if not touched. false to save only touched nonvolatiles.
     *
     * Note: Forcing nonvolatiles is used in exception handling mechanism where
     * non-volatiles might not be restored normally, if returning from a catch-
     * handler helper function.
     */
    virtual void generateMethodEpiProLogs(bool bForceSaveNonVolatiles = false) = 0;

protected:
    // The first binary pass
    FirstPassBinaryPtr m_binary;
    // The assembler interface
    AssemblerInterfacePtr m_assembler;
    // The list of all architecture registers
    RegisterAllocationTable m_archRegisters;
    // Framework methods
    FrameworkMethods m_framework;
    // Compiler Parameters
    CompilerParameters m_parameters;
    // Optimization related
    uint m_optimizationRefCount;
    // giving an index to every register in the m_freeRegister table
    cHash<int,int> m_indexToRegister;
};

#endif // __TBA_CLR_COMPILER_COMPILERINTERFACE_H
