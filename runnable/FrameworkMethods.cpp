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
* FrameworkMethods.cpp
*
* Implementation file
*
* Author: Elad Raz <e@eladraz.com>
*/
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/except/assert.h"
#include "xStl/except/trace.h"
#include "data/ElementType.h"
#include "data/ConstElements.h"
#include "format/MetadataTables.h"
#include "format/methodHeader.h"
#include "format/EncodingUtils.h"
#include "format/tables/MethodTable.h"
#include "format/tables/TypedefTable.h"
#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "runnable/FrameworkMethods.h"
#include "runnable/CorlibNames.h"
#include "runnable/RunnableTrace.h"

const char FrameworkMethods::gFrameworkNamespace[] = "clrcore";
const char FrameworkMethods::gFrameworkMemoryClassname[] = "Memory";
const char FrameworkMethods::gFrameworkGarbageCollectorClassname[] = "GarbageCollector";
const char FrameworkMethods::gFrameworkVirtualTableClassname[] = "VirtualTable";
const char FrameworkMethods::gFrameworkExceptionClassname[] = "ExceptionHandling";
const char FrameworkMethods::gFrameworkBinaryOperations[] = "BinaryOperations";

FrameworkMethods::FrameworkMethods()
{
    InitMethod(MEMCPY_INDEX, gFrameworkNamespace, gFrameworkMemoryClassname, "memcpy", FRAMEWORK_INTERNAL_MEMCPY);
    InitMethod(MEMSET_INDEX, gFrameworkNamespace, gFrameworkMemoryClassname, "memset", FRAMEWORK_INTERNAL_MEMSET);

    InitMethod(NEWOBJ_INDEX,  gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorNewObject", FRAMEWORK_INTERNAL_NEWOBJ);
    InitMethod(INCOBJ_INDEX,  gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorIncreaseReference", FRAMEWORK_INTERNAL_INCREF);
    InitMethod(DECOBJ_INDEX,  gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorDecreaseReference", FRAMEWORK_INTERNAL_DECREF);
    InitMethod(GETSIZE_INDEX, gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorGetObjectSize", FRAMEWORK_INTERNAL_GETSIZE);
    InitMethod(GETVTBL_INDEX, gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorGetVTbl", FRAMEWORK_INTERNAL_GETVTBL);
    InitMethod(NDECOBJ_INDEX, gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorDecreaseReferenceNoDestroy", FRAMEWORK_INTERNAL_NDECREF);
    InitMethod(DESTNOREF_INDEX, gFrameworkNamespace, gFrameworkGarbageCollectorClassname, "garbageCollectorDestroyIfNoReference", FRAMEWORK_INTERNAL_DESTNOREF);

    InitMethod(GETNUM_INDEX,  CorlibNames::gCoreNamespace, CorlibNames::m_classArray, "get_Length", FRAMEWORK_INTERNAL_GETNUM);


    InitMethod(REGISTER_ROUTINE_INDEX, gFrameworkNamespace, gFrameworkExceptionClassname, "registerRoutine", FRAMEWORK_INTERNAL_REGISTER_ROUTINE);
    InitMethod(REGISTER_CATCH_INDEX,   gFrameworkNamespace, gFrameworkExceptionClassname, "registerCatch", FRAMEWORK_INTERNAL_REGISTER_CATCH);
    InitMethod(REGISTER_FILTER_INDEX,  gFrameworkNamespace, gFrameworkExceptionClassname, "registerFilter", FRAMEWORK_INTERNAL_REGISTER_FILTER);
    InitMethod(POP_EXEC_INDEX,         gFrameworkNamespace, gFrameworkExceptionClassname, "popAndExecute", FRAMEWORK_INTERNAL_POP_EXEC);
    InitMethod(POP_INDEX,              gFrameworkNamespace, gFrameworkExceptionClassname, "pop", FRAMEWORK_INTERNAL_POP);
    InitMethod(THROW_INDEX,            gFrameworkNamespace, gFrameworkExceptionClassname, "throwException", FRAMEWORK_INTERNAL_THROW);
    InitMethod(CURRENT_EXCEPTION_INDEX,gFrameworkNamespace, gFrameworkExceptionClassname, "getCurrentException", FRAMEWORK_INTERNAL_CURRENT_EXCEPTION);
    InitMethod(CURRENT_STACK_RET_INDEX,gFrameworkNamespace, gFrameworkExceptionClassname, "getStackPointerRet", FRAMEWORK_INTERNAL_CURRENT_STACK_RET);

    InitMethod(GETIFACELOC_INDEX,   gFrameworkNamespace, gFrameworkVirtualTableClassname, "virtualTableGetInterfaceLocation", FRAMEWORK_INTERNAL_GET_IFACE_LOC);
    InitMethod(ISINSTANCE_INDEX,    gFrameworkNamespace, gFrameworkVirtualTableClassname, "virtualTableIsInstance", FRAMEWORK_INTERNAL_IS_INSTANCE);
    InitMethod(CHECKINSTANCE_INDEX, gFrameworkNamespace, gFrameworkVirtualTableClassname, "virtualTableCheckIsInstance", FRAMEWORK_INTERNAL_CHECK_INSTANCE);

    InitMethod(INSTANCE_STRING_INDEX, CorlibNames::gCoreNamespace, CorlibNames::m_classString, "compilerInstanceNewString", FRAMEWORK_INTERNAL_INSTANCE_STRING);
    InitMethod(INSTANCE_ARRAY1_INDEX, CorlibNames::gCoreNamespace, CorlibNames::m_classArray,  "compilerInstanceNewArray1", FRAMEWORK_INTERNAL_INSTANCE_ARRAY_1);
    InitMethod(GET_ARRAY_BUFFER,      CorlibNames::gCoreNamespace, CorlibNames::m_classArray,  "compilerGetArrayOffset",    FRAMEWORK_INTERNAL_GET_ARRAY_OFFSET);

    InitMethod(BIN32_DIV,  gFrameworkNamespace, gFrameworkBinaryOperations, "bin32div", FRAMEWORK_INTERNAL_BIT32_DIV);
    InitMethod(BIN32_REM,  gFrameworkNamespace, gFrameworkBinaryOperations, "bin32rem", FRAMEWORK_INTERNAL_BIT32_REM);
    InitMethod(BIN32_UDIV, gFrameworkNamespace, gFrameworkBinaryOperations, "bin32udiv", FRAMEWORK_INTERNAL_BIT32_UDIV);
    InitMethod(BIN32_UREM, gFrameworkNamespace, gFrameworkBinaryOperations, "bin32urem", FRAMEWORK_INTERNAL_BIT32_UREM);

    /*
    InitMethod(BIT64_ADD, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64Add", FRAMEWORK_INTERNAL_BIT64_ADD);
    InitMethod(BIT64_SUB, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64Sub", FRAMEWORK_INTERNAL_BIT64_SUB);
    InitMethod(BIT64_MUL, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64Mul", FRAMEWORK_INTERNAL_BIT64_MUL);
    InitMethod(BIT64_SHIFT_LEFT, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64ShiftLeft", FRAMEWORK_INTERNAL_BIT64_SHIFT_LEFT);
    InitMethod(BIT64_SHIFT_RIGHT_UNSIGNED, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64ShiftRightUnsigned", FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_UNSIGNED);
    InitMethod(BIT64_SHIFT_RIGHT_SIGNED, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64ShiftRightSigned", FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_SIGNED);
    InitMethod(BIT64_BITWISE_AND, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64BitwiseAnd", FRAMEWORK_INTERNAL_BIT64_BITWISE_AND);
    InitMethod(BIT64_BITWISE_OR, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64BitwiseOr", FRAMEWORK_INTERNAL_BIT64_BITWISE_OR);
    InitMethod(BIT64_BITWISE_XOR, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64BitwiseXor", FRAMEWORK_INTERNAL_BIT64_BITWISE_XOR);
    InitMethod(BIT64_BITWISE_NOT, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64BitwiseNot", FRAMEWORK_INTERNAL_BIT64_BITWISE_NOT);
    InitMethod(BIT64_NEGATE, gFrameworkNamespace, gFrameworkBinaryOperations, "bit64Negate", FRAMEWORK_INTERNAL_BIT64_NEGATE);
    */
}

void FrameworkMethods::InitMethod(uint index,
                                  const char* namespaceName,
                                  const char* className,
                                  const char* methodName, uint tokenID)
{
    CHECK(index < TOTAL_METHODS);
    m_methods[index].namespaceName = namespaceName;
    m_methods[index].className = className;
    m_methods[index].methodName = methodName;
    m_methods[index].methodToken = buildTokenIndex(MAX_UINT32, tokenID);
}

cString FrameworkMethods::getFrameworkMethodName(mdToken token) const
{
    for (uint index = 0; index < TOTAL_METHODS; index++)
    {
        if (m_methods[index].methodToken.m_a == token)
            return cString(m_methods[index].className) + "::" + m_methods[index].methodName;
    }
    ASSERT(false);
    return "Unknown internal framework method";
}

bool FrameworkMethods::isAllResolved() const
{
    bool ret = true;
    for (int x = 0; x < TOTAL_METHODS; x++)
        if (getApartmentID(m_methods[x].methodToken) == MAX_UINT32)
        {
            RunnableTrace("Missing function " << m_methods[x].namespaceName << "." <<
                          m_methods[x].className << "." << m_methods[x].methodName << "()" << endl);
            ret = false;
        }
    return ret;
}

void FrameworkMethods::addApartment(const ApartmentPtr& _apartment)
{
    // Start checking module functions
    ApartmentPtr apartment = _apartment;
    for (int x = 0; x < TOTAL_METHODS; x++)
    {
        // Try to resolved unnamed methods
        if (getApartmentID(m_methods[x].methodToken) == MAX_UINT32)
        {
            TokenIndex mid = apartment->getObjects().getTypesNameRepository().getMethodToken(
                                m_methods[x].namespaceName,
                                m_methods[x].className,
                                m_methods[x].methodName,
                                *getFrameworkSignature(getTokenID(m_methods[x].methodToken),
                                                       apartment->getObjects().getTypedefRepository()));
            if (mid != ElementType::UnresolvedTokenIndex)
            {
                m_methods[x].methodToken = mid;
            }
        }
    }
}

const TokenIndex& FrameworkMethods::getMemcpy() const
{
    return m_methods[MEMCPY_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getMemset() const
{
    return m_methods[MEMSET_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getNewObj() const
{
    return m_methods[NEWOBJ_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getIncObj() const
{
    return m_methods[INCOBJ_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getDecObj() const
{
    return m_methods[DECOBJ_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getDecNoDestroyObj() const
{
    return m_methods[NDECOBJ_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getDestNoRef() const
{
    return m_methods[DESTNOREF_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getObjSize() const
{
    return m_methods[GETSIZE_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getNumElem() const
{
    return m_methods[GETNUM_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getVtblObj() const
{
    return m_methods[GETVTBL_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getRegisterRoutine() const
{
    return m_methods[REGISTER_ROUTINE_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getRegisterCatch() const
{
    return m_methods[REGISTER_CATCH_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getRegisterFilter() const
{
    return m_methods[REGISTER_FILTER_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getPopAndExec() const
{
    return m_methods[POP_EXEC_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getPop() const
{
    return m_methods[POP_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getThrowException() const
{
    return m_methods[THROW_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getCurrentException() const
{
    return m_methods[CURRENT_EXCEPTION_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getCurrentStackPointerRet() const
{
    return m_methods[CURRENT_STACK_RET_INDEX].methodToken;
}


const TokenIndex& FrameworkMethods::getIfaceLoc() const
{
    return m_methods[GETIFACELOC_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::isInstance() const
{
    return m_methods[ISINSTANCE_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::checkInstance() const
{
    return m_methods[CHECKINSTANCE_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::instanceString() const
{
    return m_methods[INSTANCE_STRING_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::instanceArray1() const
{
    return m_methods[INSTANCE_ARRAY1_INDEX].methodToken;
}

const TokenIndex& FrameworkMethods::getArrayBuffer() const
{
    return m_methods[GET_ARRAY_BUFFER].methodToken;
}

const TokenIndex& FrameworkMethods::getBin32Div() const
{
    return m_methods[BIN32_DIV].methodToken;
}

const TokenIndex& FrameworkMethods::getBin32Rem() const
{
    return m_methods[BIN32_REM].methodToken;
}

const TokenIndex& FrameworkMethods::getBin32uDiv() const
{
    return m_methods[BIN32_UDIV].methodToken;
}

const TokenIndex& FrameworkMethods::getBin32uRem() const
{
    return m_methods[BIN32_UREM].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64Add() const
{
    return m_methods[BIT64_ADD].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64Sub() const
{
    return m_methods[BIT64_SUB].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64Mul() const
{
    return m_methods[BIT64_MUL].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64ShiftLeft() const
{
    return m_methods[BIT64_SHIFT_LEFT].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64ShiftRightUnsigned() const
{
    return m_methods[BIT64_SHIFT_RIGHT_UNSIGNED].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64ShiftRightSigned() const
{
    return m_methods[BIT64_SHIFT_RIGHT_SIGNED].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64Negate() const
{
    return m_methods[BIT64_NEGATE].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64BitwiseAnd() const
{
    return m_methods[BIT64_BITWISE_AND].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64BitwiseOr() const
{
    return m_methods[BIT64_BITWISE_OR].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64BitwiseXor() const
{
    return m_methods[BIT64_BITWISE_XOR].methodToken;
}

const TokenIndex& FrameworkMethods::getBit64BitwiseNot() const
{
    return m_methods[BIT64_BITWISE_NOT].methodToken;
}

MethodDefOrRefSignaturePtr FrameworkMethods::getFrameworkSignature(
                                                            mdToken token,
                                                            const ResolverInterface& resolver) const
{
    ElementType returnType;
    ElementsArrayType args;

    TokenIndex index;
    bool hasThis = false;

    switch (token)
    {
    case FRAMEWORK_INTERNAL_MEMCPY:
        // void memcpy(void*, void*, uint)
        args.changeSize(3);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gVoidPtr;
        args[2] = ConstElements::gU4;
        // returnType is already void
        break;
    case FRAMEWORK_INTERNAL_MEMSET:
        // void memset(void*, uint)
        args.changeSize(3);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gU1;
        args[2] = ConstElements::gU;
        // returnType is already void
        break;

    case FRAMEWORK_INTERNAL_NEWOBJ:
        // void* garbageCollectorNewObject(void* vtbl)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        returnType = ConstElements::gVoidPtr;
        break;

    case FRAMEWORK_INTERNAL_INCREF:
        // void garbageCollectorIncreaseReference(void* buffer)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        // returnType is already void
        break;
    case FRAMEWORK_INTERNAL_DECREF:
        // void garbageCollectorDecreaseReference(void* buffer)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        // returnType is already void
        break;
    case FRAMEWORK_INTERNAL_NDECREF:
        // void garbageCollectorDecreaseReferenceNoDestroy(void* buffer)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        // returnType is already void
        break;
    case FRAMEWORK_INTERNAL_DESTNOREF:
        // void garbageCollectorDestroyIfNoReference(void* buffer)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        // returnType is already void
        break;
    case FRAMEWORK_INTERNAL_GETSIZE:
        // uint garbageCollectorGetObjectSize(void* buffer)
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        returnType = ConstElements::gU;
        break;
    case FRAMEWORK_INTERNAL_GETNUM:
        // int Array.Length
        hasThis = true;
        args.changeSize(0);
        returnType = ConstElements::gI;
        break;
    case FRAMEWORK_INTERNAL_GETVTBL:
        // void* garbageCollectorGetVTbl(void* buffer)
        returnType = ConstElements::gVoidPtr;
        args.changeSize(1);
        args[0] = ConstElements::gVoidPtr;
        break;
    case FRAMEWORK_INTERNAL_REGISTER_ROUTINE:
        // void registerRoutine(uint type, void* handler, void* stackPointer)
        args.changeSize(3);
        args[0] = ConstElements::gU4;
        args[1] = ConstElements::gVoidPtr;
        args[2] = ConstElements::gVoidPtr;
        break;
    case FRAMEWORK_INTERNAL_REGISTER_CATCH:
        // void registerCatch(ushort exceptionRtti, void* handler, void* stackPointer, void* stackPointerRet)
        args.changeSize(4);
        args[0] = ConstElements::gU2;
        args[1] = ConstElements::gVoidPtr;
        args[2] = ConstElements::gVoidPtr;
        args[3] = ConstElements::gVoidPtr;
        break;
    case FRAMEWORK_INTERNAL_REGISTER_FILTER:
        // void registerFilter(void* filter, void* handler, void* stackPointer, void* stackPointerRet)
        args.changeSize(4);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gVoidPtr;
        args[2] = ConstElements::gVoidPtr;
        args[3] = ConstElements::gVoidPtr;
        break;
    case FRAMEWORK_INTERNAL_POP_EXEC:
        // void popAndExecute()
        args.changeSize(0);
        break;
    case FRAMEWORK_INTERNAL_POP:
        // void pop()
        args.changeSize(0);
        break;
    case FRAMEWORK_INTERNAL_THROW:
        // void throwException(Morph.Exception exceptionObject)
        args.changeSize(1);
        args[0] = ElementType(ELEMENT_TYPE_CLASS, 0, false, false, false, resolver.getTypeToken(CorlibNames::gCoreNamespace, CorlibNames::m_classException));
        break;
    case FRAMEWORK_INTERNAL_CURRENT_EXCEPTION:
        //public static Morph.Exception getCurrentException()
        args.changeSize(0);
        returnType = ElementType(ELEMENT_TYPE_CLASS, 0, false, false, false, resolver.getTypeToken(CorlibNames::gCoreNamespace, CorlibNames::m_classException));
        break;
    case FRAMEWORK_INTERNAL_CURRENT_STACK_RET:
        //public static void* getStackPointerRet()
        args.changeSize(0);
        returnType = ConstElements::gVoidPtr;
        break;

    case FRAMEWORK_INTERNAL_GET_IFACE_LOC:
        // uint virtualTableGetInterfaceLocation(void* parentvTbl, ushort childRtti)
        args.changeSize(2);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gU2;
        returnType = ConstElements::gU;
        break;

    case FRAMEWORK_INTERNAL_IS_INSTANCE:
        // bool virtualTableIsInstance(void* parentvTbl, ushort childRtti)
        args.changeSize(2);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gU2;
        returnType = ConstElements::gVoidPtr;
        break;

    case FRAMEWORK_INTERNAL_CHECK_INSTANCE:
        // unsafe static void virtualTableCheckIsInstance(void* parentvTbl, ushort childRtti)
        args.changeSize(2);
        args[0] = ConstElements::gVoidPtr;
        args[1] = ConstElements::gU2;
        break;

    case FRAMEWORK_INTERNAL_INSTANCE_STRING:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU;
        args[1] = ConstElements::gBytePtr;
        returnType = ConstElements::gString;
        resolver.resolveTyperef(returnType);
        break;

    case FRAMEWORK_INTERNAL_INSTANCE_ARRAY_1:
        // static unsafe Array compilerInstanceNewArray1(void* vtbl, uint numberOfElements)
        args.changeSize(2);
        args[0] = ConstElements::gU;
        args[1] = ConstElements::gVoidPtr;
        returnType = ConstElements::gVoidArray;
        resolver.resolveTyperef(returnType);
        break;

    case FRAMEWORK_INTERNAL_GET_ARRAY_OFFSET:
        // static unsafe byte* compilerGetArrayOffset(Array arr, uint index, uint sizeofElements)
        args.changeSize(3);
        args[0] = ConstElements::gVoidArray;
        args[1] = ConstElements::gU;
        args[2] = ConstElements::gU;
        returnType = ConstElements::gBytePtr;
        resolver.resolveTyperef(args[0]);
        break;

    case FRAMEWORK_INTERNAL_BIT32_DIV:
        // public static int bin32div(int a, int b)
        args.changeSize(2);
        args[0] = ConstElements::gI;
        args[1] = ConstElements::gI;
        returnType = ConstElements::gI;
        break;

    case FRAMEWORK_INTERNAL_BIT32_REM:
        // public static int bin32rem(int a, int b)
        args.changeSize(2);
        args[0] = ConstElements::gI;
        args[1] = ConstElements::gI;
        returnType = ConstElements::gI;
        break;

    case FRAMEWORK_INTERNAL_BIT32_UREM:
        // public static uint bin32urem(uint a, uint b)
        args.changeSize(2);
        args[0] = ConstElements::gU;
        args[1] = ConstElements::gU;
        returnType = ConstElements::gU;
        break;

    case FRAMEWORK_INTERNAL_BIT32_UDIV:
        // public static uint bin32div(uint a, uint b)
        args.changeSize(2);
        args[0] = ConstElements::gU;
        args[1] = ConstElements::gU;
        returnType = ConstElements::gU;
        break;

    //*******/
     case FRAMEWORK_INTERNAL_BIT64_ADD:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        args[3] = ConstElements::gU4;
        break;

    case FRAMEWORK_INTERNAL_BIT64_SUB:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        args[3] = ConstElements::gU4;
        break;
     case FRAMEWORK_INTERNAL_BIT64_SHIFT_LEFT:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_UNSIGNED:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_SHIFT_RIGHT_SIGNED:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_NEGATE:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        break;
    case FRAMEWORK_INTERNAL_BIT64_BITWISE_AND:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        args[3] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_BITWISE_OR:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        args[3] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_BITWISE_XOR:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        args[2] = ConstElements::gU4;
        args[3] = ConstElements::gU4;
        break;
    case FRAMEWORK_INTERNAL_BIT64_BITWISE_NOT:
        // static Morph.String compilerInstanceNewString(uint size, tchar* source)
        args.changeSize(2);
        args[0] = ConstElements::gU4Ref;
        args[1] = ConstElements::gU4Ref;
        break;
    default:
        return MethodDefOrRefSignaturePtr();
    }

    return MethodDefOrRefSignaturePtr(new MethodDefOrRefSignature(
                                returnType,
                                args,
                                hasThis,
                                false,
                                MethodDefOrRefSignature::CALLCONV_DEFAULT));
}

MethodDefOrRefSignaturePtr FrameworkMethods::getStringCtorSignature()
{
    ElementsArrayType args;

    args.changeSize(3);
    args[0] = ConstElements::gU;
    args[1] = ConstElements::gBytePtr; // // TODO! Ascii/Unicode
    args[2] = ConstElements::gBool;

    return MethodDefOrRefSignaturePtr(new MethodDefOrRefSignature(
                                ConstElements::gVoid, //returnType
                                args,
                                true,
                                false,
                                MethodDefOrRefSignature::CALLCONV_DEFAULT));

}

FrameworkMethods::ExceptionRoutineType FrameworkMethods::GetExceptionRoutineType(uint clauseFlags)
{
    switch (clauseFlags)
    {
    case MethodHeader::ClauseCatch:
        return FrameworkMethods::EXCEPTION_ROUTINE_CATCH_HANDLER;
    case MethodHeader::ClauseFilter:
        return FrameworkMethods::EXCEPTION_ROUTINE_FILTER;
    case MethodHeader::ClauseFinally:
        return FrameworkMethods::EXCEPTION_ROUTINE_FINALLY_HANDLER;
    case MethodHeader::ClauseFault:
        return FrameworkMethods::EXCEPTION_ROUTINE_FAULT_HANDLER;
    }
    // Where did this flag come from? Anyway, we don't support it.
    CHECK(false);
}

bool FrameworkMethods::isFrameworkMethod(TokenIndex methodToken) const
{
    for (int x = 0; x < TOTAL_METHODS; x++)
    {
        ASSERT(getApartmentID(m_methods[x].methodToken) != MAX_UINT32);

        if (m_methods[x].methodToken == methodToken)
            return true;
    }
    return false;
}
