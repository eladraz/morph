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

#ifndef __TBA_CLR_RUNNABLE_FRAMEWORKMETHODS_H
#define __TBA_CLR_RUNNABLE_FRAMEWORKMETHODS_H

/*
 * FrameworkMethods.h
 *
 * Getting framework methods token ids such as:
 *   memcpy
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "data/ElementType.h"
#include "format/MetadataTables.h"
#include "format/MSILStreams.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/Apartment.h"
#include "runnable/Stack.h"

class FrameworkMethods {
public:
    /*
     * Constructor
     */
    FrameworkMethods();

    /*
     * Checking new apartment for runtime functions
     *
     * apartment - The apartment object
     */
    void addApartment(const ApartmentPtr& apartment);

    /*
     * Return the token for "MethodXXX" memory management function
     */
    const TokenIndex& getMemcpy() const;
    const TokenIndex& getNewObj() const;

    /*
     * Return the token for "MethodXXX" garbage collection  function
     */
    const TokenIndex& getIncObj() const;
    const TokenIndex& getDecObj() const;
    const TokenIndex& getMemset() const;
    const TokenIndex& getObjSize() const;
    const TokenIndex& getNumElem() const;
    const TokenIndex& getVtblObj() const;
    const TokenIndex& getDecNoDestroyObj() const;
    const TokenIndex& getDestNoRef() const;

    /*
     * Return the token for "MethodXXX" exception stack function
     */
    const TokenIndex& getRegisterRoutine() const;
    const TokenIndex& getRegisterCatch() const;
    const TokenIndex& getRegisterFilter() const;
    const TokenIndex& getPopAndExec() const;
    const TokenIndex& getPop() const;
    const TokenIndex& getThrowException() const;
    const TokenIndex& getCurrentException() const;
    const TokenIndex& getCurrentStackPointerRet() const;

    /*
     * Return the token for "MethodXXX" virtual table functions
     */
    const TokenIndex& getIfaceLoc() const;
    const TokenIndex& isInstance() const;
    const TokenIndex& checkInstance() const;

    /*
     * Return the token for "MethodXXX" string functions
     */
    const TokenIndex& instanceString() const;
    const TokenIndex& instanceArray1() const;
    const TokenIndex& getArrayBuffer() const;
    const TokenIndex& getBin32Div() const;
    const TokenIndex& getBin32Rem() const;
    const TokenIndex& getBin32uDiv() const;
    const TokenIndex& getBin32uRem() const;

    /*
     * Return the token for 64bit binary functions
     */
    const TokenIndex& getBit64Add() const;
    const TokenIndex& getBit64Sub() const;
    const TokenIndex& getBit64Mul() const;
    const TokenIndex& getBit64ShiftLeft() const;
    const TokenIndex& getBit64ShiftRightUnsigned() const;
    const TokenIndex& getBit64ShiftRightSigned() const;
    const TokenIndex& getBit64Negate() const;
    const TokenIndex& getBit64BitwiseAnd() const;
    const TokenIndex& getBit64BitwiseOr() const;
    const TokenIndex& getBit64BitwiseXor() const;
    const TokenIndex& getBit64BitwiseNot() const;


    /*
     * Return a new signature for the framework description.
     * Return empty object if the function is not a framework function
     */
    MethodDefOrRefSignaturePtr getFrameworkSignature(mdToken token, const ResolverInterface& resolver) const;

    /*
     * Return string.ctor method signature
     */
    static MethodDefOrRefSignaturePtr getStringCtorSignature();

    /*
     * Return the name of a framework method, given its token.
     */
    cString getFrameworkMethodName(mdToken token) const;

    /*
     * Return whether all framework methods were resolved or not
     */
    bool isAllResolved() const;

    /*
     * Determine whether or not a specific method token is a framework method
     */
    bool isFrameworkMethod(TokenIndex methodToken) const;


    /*
     * A list of all frameworks functions
     */
    enum {
        // public unsafe static void memcpy(void* dv, void* sv, uint length)
        FRAMEWORK_INTERNAL_MEMCPY = 0xFFDEAD00,
        // public unsafe static void* garbageCollectorNewObject(void* vtbl, uint/ushort numberOfElements)
        FRAMEWORK_INTERNAL_NEWOBJ = 0xFFDEAD01,
        // public unsafe static void garbageCollectorIncreaseReference(void* buffer)
        FRAMEWORK_INTERNAL_INCREF = 0xFFDEAD02,
        // public unsafe static void garbageCollectorDecreaseReference(void* buffer)
        FRAMEWORK_INTERNAL_DECREF = 0xFFDEAD03,
        //public unsafe static void memset(void* p, byte value, uint length)
        FRAMEWORK_INTERNAL_MEMSET = 0xFFDEAD04,
        //public unsafe static uint garbageCollectorGetObjectSize(void* buffer)
        FRAMEWORK_INTERNAL_GETSIZE = 0xFFDEAD05,
        //public unsafe static void* garbageCollectorGetVTbl(void* buffer)
        FRAMEWORK_INTERNAL_GETVTBL = 0xFFDEAD06,
        //public unsafe static uint garbageCollectorGetNumberOfElements(void* buffer)
        FRAMEWORK_INTERNAL_GETNUM = 0xFFDEAD07,
        //public unsafe static void garbageCollectorDecreaseReferenceNoDestroy(void* buffer)
        FRAMEWORK_INTERNAL_NDECREF = 0xFFDEAD08,
        //internal unsafe static void garbageCollectorDestroyIfNoReference(void* buffer)
        FRAMEWORK_INTERNAL_DESTNOREF = 0xFFDEAD09,

        //public unsafe static void registerRoutine(uint type, void* handler, void* stackPointer)
        FRAMEWORK_INTERNAL_REGISTER_ROUTINE = 0xFFDEAD20,
        //public unsafe static void registerCatch(ushort exceptionRtti, void* handler, void* stackPointer)
        FRAMEWORK_INTERNAL_REGISTER_CATCH = 0xFFDEAD21,
        //public unsafe static void registerFilter(void* filter, void* handler, void* stackPointer)
        FRAMEWORK_INTERNAL_REGISTER_FILTER = 0xFFDEAD22,
        //public unsafe static void popAndExecute(uint expectedType, void* expectedHandler)
        FRAMEWORK_INTERNAL_POP_EXEC = 0xFFDEAD23,
        //public unsafe static void pop(uint expectedType, void* expectedHandler)
        FRAMEWORK_INTERNAL_POP = 0xFFDEAD24,
        //public unsafe static void throwException(void* exceptionObject)
        FRAMEWORK_INTERNAL_THROW = 0xFFDEAD25,
        //public static Morph.Exception getCurrentException()
        FRAMEWORK_INTERNAL_CURRENT_EXCEPTION = 0xFFDEAD26,
        //public static Morph.Exception getCurrentStackPointerRet()
        FRAMEWORK_INTERNAL_CURRENT_STACK_RET = 0xFFDEAD27,

        //private unsafe static uint virtualTableGetInterfaceLocation(void* parentvTbl, ushort rtti)
        FRAMEWORK_INTERNAL_GET_IFACE_LOC = 0xFFDEAD30,
        //private unsafe static bool virtualTableIsInstance(void* parentvTbl, ushort rtti)
        FRAMEWORK_INTERNAL_IS_INSTANCE = 0xFFDEAD31,
        //private unsafe static void virtualTableCheckIsInstance(void* parentvTbl, ushort rtti)
        FRAMEWORK_INTERNAL_CHECK_INSTANCE = 0xFFDEAD32,

        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        FRAMEWORK_INTERNAL_INSTANCE_STRING = 0xFFDEAD40,
        // static unsafe Array compilerInstanceNewArray1(void* vtbl, uint numberOfElements)
        FRAMEWORK_INTERNAL_INSTANCE_ARRAY_1 = 0xFFDEAD41,
        // static unsafe byte* compilerGetArrayOffset(Array arr)
        FRAMEWORK_INTERNAL_GET_ARRAY_OFFSET = 0xFFDEAD42,

        //public static int getBit32Div(int a, int b)
        FRAMEWORK_INTERNAL_BIT32_DIV = 0xFFCEAE00,
        //public static int getBit32Rem(int a, int b)
        FRAMEWORK_INTERNAL_BIT32_REM = 0xFFCEAE01,
        //public static uint getBit32uDiv(uint a, uint b)
        FRAMEWORK_INTERNAL_BIT32_UDIV = 0xFFCEAE02,
        //public static uint getBit32uRem(uint a, uint b)
        FRAMEWORK_INTERNAL_BIT32_UREM = 0xFFCEAE03,

        //public static void getBit64Add(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi)
        FRAMEWORK_INTERNAL_BIT64_ADD = 0xFFDEAE00,
        //public static void getBit64Sub(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi)
        FRAMEWORK_INTERNAL_BIT64_SUB = 0xFFDEAE01,
        //public static void getBit64Mul(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi)
        FRAMEWORK_INTERNAL_BIT64_MUL = 0xFFDEAE02,
        //public static void getBit64ShiftLeft(ref uint destLow, ref uint destHi, int n)
        FRAMEWORK_INTERNAL_BIT64_SHIFT_LEFT = 0xFFDEAE03,
        //public static void getBit64ShiftRightUnsigned(ref uint destLow, ref uint destHi, int n)
        FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_UNSIGNED = 0xFFDEAE04,
        //public static void getBit64ShiftRightSigned(ref uint destLow, ref uint destHi, int n
        FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_SIGNED = 0xFFDEAE05,
        //public static void bit64Negate(ref uint destLow, ref uint destHi)
        FRAMEWORK_INTERNAL_BIT64_NEGATE = 0xFFDEAE0A,
        //public static void getBit64BitwiseAnd(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi
        FRAMEWORK_INTERNAL_BIT64_BITWISE_AND = 0xFFDEAE06,
        //public static void getBit64BitwiseOr(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi)
        FRAMEWORK_INTERNAL_BIT64_BITWISE_OR = 0xFFDEAE07,
        //public static void getBit64BitwiseXor(ref uint destLow, ref uint destHi, uint sourceLow, uint sourceHi)
        FRAMEWORK_INTERNAL_BIT64_BITWISE_XOR = 0xFFDEAE08,
        //public static void getBit64BitwiseNot(ref uint destLow, ref uint destHi)
        FRAMEWORK_INTERNAL_BIT64_BITWISE_NOT = 0xFFDEAE09,
    };

    /*
     * A list of possible Exception Handling entries in the runtime stack
     */
    enum ExceptionRoutineType {
        // A method's cleanup routine for its locals
        EXCEPTION_ROUTINE_METHOD_CLEANUP = 1,
        // A finally clause handler
        EXCEPTION_ROUTINE_FINALLY_HANDLER,
        // A fault clause handler
        EXCEPTION_ROUTINE_FAULT_HANDLER,
        // A catch clause handler
        EXCEPTION_ROUTINE_CATCH_HANDLER,
        // A filter clause filter-function and handler
        EXCEPTION_ROUTINE_FILTER,
    };

    /*
     * Translates a .NET clause flags (from the method header) to a CLR runtime exception stack entry type
     */
    static ExceptionRoutineType GetExceptionRoutineType(uint clauseFlags);

private:
    void InitMethod(uint index,
                    const char* namespaceName,
                    const char* className,
                    const char* methodName,
                    uint tokenID);

    enum {
        MEMCPY_INDEX = 0,
        NEWOBJ_INDEX,
        INCOBJ_INDEX,
        DECOBJ_INDEX,
        MEMSET_INDEX,
        GETSIZE_INDEX,
        GETNUM_INDEX,
        GETVTBL_INDEX,
        NDECOBJ_INDEX,
        DESTNOREF_INDEX,

        REGISTER_ROUTINE_INDEX,
        REGISTER_CATCH_INDEX,
        REGISTER_FILTER_INDEX,
        POP_EXEC_INDEX,
        POP_INDEX,
        THROW_INDEX,
        CURRENT_EXCEPTION_INDEX,
        CURRENT_STACK_RET_INDEX,

        GETIFACELOC_INDEX,
        ISINSTANCE_INDEX,
        CHECKINSTANCE_INDEX,

        INSTANCE_STRING_INDEX,
        INSTANCE_ARRAY1_INDEX,
        GET_ARRAY_BUFFER,

        BIN32_DIV,
        BIN32_REM,
        BIN32_UDIV,
        BIN32_UREM,

        TOTAL_METHODS,

        BIT64_ADD,
        BIT64_SUB,
        BIT64_MUL,
        BIT64_SHIFT_LEFT,
        BIT64_SHIFT_RIGHT_UNSIGNED,
        BIT64_SHIFT_RIGHT_SIGNED,
        BIT64_NEGATE,
        BIT64_BITWISE_AND,
        BIT64_BITWISE_OR,
        BIT64_BITWISE_XOR,
        BIT64_BITWISE_NOT,
    };
    // Store framework method defaults
    struct MethodDescription {
        const char* namespaceName;
        const char* className;
        const char* methodName;
        TokenIndex methodToken;
    };

    // The framework methods
    MethodDescription m_methods[TOTAL_METHODS];

    // Filter namespaces
    static const char gFrameworkNamespace[];
    static const char gFrameworkMemoryClassname[];
    static const char gFrameworkGarbageCollectorClassname[];
    static const char gFrameworkVirtualTableClassname[];
    static const char gFrameworkExceptionClassname[];
    static const char gFrameworkBinaryOperations[];
};

#endif // __TBA_CLR_RUNNABLE_FRAMEWORKMETHODS_H
