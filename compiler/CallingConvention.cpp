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
 * CallingConvention.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerEngine.h"
#include "compiler/CallingConvention.h"
#include "compiler/MethodCompiler.h"
#include "compiler/CompilerInterface.h"
#include "compiler/CompilerTrace.h"
#include "compiler/opcodes/RegisterEvaluatorOpcodes.h"
#include "compiler/opcodes/CompilerOpcodes.h"

const char CallingConvention::gCILMethodPrefix[] = "?CIL?STD?MTD?";
const char CallingConvention::gCILTokenPrefix[]  = "?CIL?STD?TKN?";
const char CallingConvention::gCILGlobalPrefix[] = "?CIL?STD?BSS?";
const char CallingConvention::gCILGlobalDataPrefix[] = "?CIL?STD?DATA?";

const char CallingConvention::gCallingConventionCustomAttributeName[] = "CallingConvention";
const char CallingConvention::gCallingArgumentName[] = "callingConvention";
const char CallingConvention::gCallingConventionStdcall[] = "stdcall";
const char CallingConvention::gCallingConventionCdecl[] = "cdecl";
const char CallingConvention::gCallingConventionFastcall[] = "fastcall";

MethodDefOrRefSignaturePtr CallingConvention::readMethodSignature(
                           const Apartment& apartment,
                           mdToken methodToken)
{
    uint methodTable = EncodingUtils::getTokenTableIndex(methodToken);
    mdToken signature = 0;
    MethodDefOrRefSignaturePtr methodSignature;
    if (methodTable == TABLE_METHOD_TABLE)
    {
        // Read method signature for arguments
        MethodTable& methodTable = (MethodTable&)(*apartment.getTables().
                                                  getTableByToken(methodToken));
        signature = methodTable.getHeader().m_signature;
    } else if (methodTable == TABLE_MEMBERREF_TABLE)
    {
        MemberRefTable& memberRefTable = (MemberRefTable&)(*apartment.getTables().
                                                  getTableByToken(methodToken));
        signature = memberRefTable.getHeader().m_signature;
    } else if (methodTable == TABLE_CLR_INTERNAL)
    {
        // TODO! Shouldn't get here
        methodSignature = apartment.getObjects().getFrameworkMethods().
                                         getFrameworkSignature(methodToken, apartment.getObjects().getTypedefRepository());
    } else if (methodTable == TABLE_CLR_CCTOR_WRAPPERS)
    {
        methodSignature = MethodDefOrRefSignaturePtr(new MethodDefOrRefSignature(
                                    ConstElements::gVoid,
                                    ElementsArrayType(),
                                    false,
                                    false,
                                    MethodDefOrRefSignature::CALLCONV_DEFAULT));
    }

    if ((methodTable == TABLE_METHOD_TABLE) ||
        (methodTable == TABLE_MEMBERREF_TABLE))
    {
        if (signature == 0)
            CHECK_FAIL();
        cForkStreamPtr blob = apartment.getStreams().getBlobStream()->fork();
        blob->seek(signature, basicInput::IO_SEEK_SET);
        methodSignature = MethodDefOrRefSignaturePtr(new MethodDefOrRefSignature(*blob,
                                                            apartment.getUniqueID(),
                                                            apartment.getObjects().getTypedefRepository()));
    }

    return methodSignature;
}

CompilerInterface::CallingConvention CallingConvention::mapCustomAttributeNametoMethodEnum(const cString& value)
{
    if (value.compare(gCallingConventionStdcall) == cString::EqualTo)
        return CompilerInterface::STDCALL;

    if (value.compare(gCallingConventionCdecl) == cString::EqualTo)
        return CompilerInterface::C_DECL;

    if (value.compare(gCallingConventionFastcall) == cString::EqualTo)
        return CompilerInterface::FASTCALL;

    CompilerTrace("CallingConvention! Error! cannot find calling convention of type " << value << endl);

    return CompilerInterface::STDCALL;
}

CompilerInterface::CallingConvention
CallingConvention::getDesiredCallingMethod(mdToken methodToken,
                                           const Apartment& apartment,
                                           const CompilerInterface& compilerInterface
                                           /*MethodDefOrRefSignaturePtr methodSignature*/)
{
    // Default
    CompilerInterface::CallingConvention callingMethod = compilerInterface.getDefaultCallingConvention();

    // Old code, C# calling convention keyword must be default/others are not supported yet.
    // CHECK(methodSignature->getCallingConvention() == MethodDefOrRefSignature::CALLCONV_DEFAULT);

    // Retrieve "CallingConvention" custom attributes from method.
    CustomAttributes customAttributes;
    CustomAttribute::getAttributes(ApartmentPtr(&(Apartment&)apartment, SMARTPTR_DESTRUCT_NONE),
                                   methodToken,
                                   CallingConvention::gCallingConventionCustomAttributeName,
                                   customAttributes);

    if (0 != customAttributes.getSize()) {
        // Get the first one, it's enough for now.
        CustomAttributePtr customAttributePtr = customAttributes[0];
        try {
            callingMethod = CallingConvention::mapCustomAttributeNametoMethodEnum(customAttributePtr->getArgument(gCallingArgumentName).getStringValue());
        }
        catch(CustomAttribute::MissingArgument)
        {
            // Do nothing, revert to default.
        }
    }

    return callingMethod;
}

static cString getMethodName(Apartment& apartment, mdToken methodToken)
{
    mdToken table_id = EncodingUtils::getTokenTableIndex(methodToken);
    mdToken nameIndex = 0;

    if (TABLE_METHOD_TABLE == table_id) {
        const MethodTable& methodTable = (const MethodTable&) (*(apartment.getTables().getTableByToken(methodToken)));
        nameIndex = methodTable.getName();
    }
    else if (TABLE_MEMBERREF_TABLE == table_id) {
        const MemberRefTable& memberRefTable = (const MemberRefTable&) (*(apartment.getTables().getTableByToken(methodToken)));
        nameIndex = memberRefTable.getHeader().m_name;
    }
    else if (TABLE_CLR_CCTOR_WRAPPERS == table_id) {
        return cString("CLR CCTOR");
    }
    else if (TABLE_CLR_INTERNAL == table_id) {
        return cString("INTERNAL");//apartment.getObjects().getFrameworkMethods().getFrameworkMethodName(methodToken);
    }
    else if (TABLE_CLR_METHOD_HELPERS == table_id) {
        return cString("HELPER");
    }
    else {
        // Invalid table id
        ASSERT(0);
    }

    return StringReader::readStringName(*apartment.getStreams().getStringsStream()->fork(), nameIndex);
}

bool CallingConvention::compareElements(ResolverInterface& resolver,
                                        const ElementType& a,
                                        const StackEntity& b)
{
    // A value-type (for example, boxed value) can be passed into an object argument
    if (a.isObject() && (b.getElementType().getType() == ELEMENT_TYPE_VALUETYPE))
        return true;

    // If the types are of the same size, it can be passed
    if (resolver.getTypeSize(a) == (resolver.getTypeSize(b.getElementType())))
        return true;

    // A const can be passed as anything
    if (b.getType() == StackEntity::ENTITY_CONST)
        return true;

    // Anything can be passed as a pointer... "Cast to pointer. TODO!"
    if (a.isPointer())
        return true;

    // Check for implicit comparison
    if (!a.isObject() && !b.getElementType().isObject())
        return true;

    return false;
}

void CallingConvention::call(EmitContext& emitContext,
                             const TokenIndex& mid,
                             ThisObjectHandling thisHandling,
                             bool isVirtual,
                             bool skipPush)
{
    uint32 argSize = 0;
    MethodBlock& currentBlock = emitContext.currentBlock;

    // Resolve method
    TokenIndex methodToken = ClrResolver::resolve(emitContext.methodContext.getApartment(), mid);
    if (methodToken == ElementType::UnresolvedTokenIndex)
        methodToken = mid;

    mdToken mdMethodToken = getTokenID(methodToken);
    Apartment& apartment = *(emitContext.methodContext.getApartment()->getApt(methodToken));
    cString methodName = getMethodName(apartment, mdMethodToken);

    MethodDefOrRefSignaturePtr methodSignature = readMethodSignature(apartment, mdMethodToken);
    // Check for special method custom-attributes (Such as CPU operations)
    CustomAttributes customAttributes;
    CustomAttribute::getAttributes(emitContext.methodContext.getApartment()->getApt(methodToken),
                                   mdMethodToken,
                                   "CompilerOpcodes",
                                   customAttributes);

    if (0 != customAttributes.getSize())
    {
        CustomAttributePtr customAttributePtr = customAttributes[0];
        XSTL_TRY
        {

            if (!CompilerOpcodes::compilerOpcodes(emitContext, methodToken, customAttributePtr->getArgument("opcode").getStringValue(), *methodSignature))
            {
                return;
                }
        }
        XSTL_CATCH (CustomAttribute::MissingArgument)
            {
            }
    }

    if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == TABLE_MEMBERREF_TABLE)
    {
        CompilerTrace("CallingConvention::call() Error! Unimplemented function! " << HEXTOKEN(mid) <<
                      " - " << methodName << "()" << endl);
        CHECK_FAIL();
    }

    // Get method parent
    CompilerInterface& compiler = *emitContext.methodRuntime.m_compiler;
    CompilerTrace("\t\tCalling method:  " << ((isVirtual) ? "virtual " : cString()) << methodName << " " <<  HEXTOKEN(methodToken) << endl);

    // Start build method token
    ASSERT(!methodSignature.isEmpty());

    // Get calling method convention
    CompilerInterface::CallingConvention callingMethod = getDesiredCallingMethod(mdMethodToken,
                                                                                 apartment,
                                                                                 emitContext.methodRuntime.getCompiler());

    // TODO!  Store all pre-allocated super-free registers!
    Stack& stack = emitContext.currentBlock.getCurrentStack();

    // Check if current token is System.Object
    if (emitContext.methodContext.isSystemObject())
    {
        if (methodName.compare(CorlibNames::m_methodCtor) == cString::EqualTo)
        {
            // Skip all constructor calls from code in Morph.Object
            // pop 1 argument (this)
            stack.pop2null();
            CompilerTrace("\t\tIgnoring .ctor for System.Object recursive call..." << endl);
            return;
        }
    }

    //check return type
    const ElementType& retType = methodSignature->getReturnType();
    uint retSize = 0;
    StackEntity retSizeEntity;
    TemporaryStackHolderPtr newStackPositionHolder;
    TemporaryStackHolderPtr structRetHolder;

    StackEntity structRetEntity(StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR, retType);
    if (!retType.isVoid())
    {
        retSize = apartment.getObjects().getTypedefRepository().getTypeSize(retType);
        if(retSize > (uint)compiler.getStackSize())
        {
            //allocate on the stack a temporary the size of the struct
            structRetHolder = TemporaryStackHolderPtr(new TemporaryStackHolder(
                                    currentBlock,
                                    ELEMENT_TYPE_U1,
                                    retSize,
                                    TemporaryStackHolder::TEMP_ONLY_STACK));

            structRetEntity.setStackHolderObject(structRetHolder);

            //load the address
            RegisterEvaluatorOpcodes::evaluateInt32(emitContext, structRetEntity);

            //this will hold the address of the allocation on the stack of the struct returned
            compiler.pushArg32(structRetEntity.getStackHolderObject()->getTemporaryObject());

            argSize += 4;
        }

        CHECK(!retType.isSingleDimensionArray());
    }

    // Ignore "this" handling if there is no "this"
    if (!methodSignature->isHasThis())
        thisHandling = None;

    // If method needs a "this", then we have to know how to handle "this"
    // Otherwise, there is no handling of "this"
    CHECK(methodSignature->isHasThis() == (thisHandling != None));

    uint stackIndex = 0;

    StackEntity* thisObj = NULL;
    switch (thisHandling)
    {
    case ThisAboveParams:
    case ThisAboveParamsDup:
        // "this" object it at top of stack. get a reference to it now
        thisObj = &stack.getArg(stackIndex++);
        break;
    case ThisBelowParams:
        // "this" object is below the parameters. get a reference to it now
        thisObj = &stack.getArg(methodSignature->getParams().getSize());
        break;
    default:
        break;
    }

    if (!skipPush)
    {
        const ElementsArrayType& args = methodSignature->getParams();
        uint i = args.getSize();
        while (i > 0)
        {
            i--;
            // For each argument, get a reference to it
            // And remember to pop it from the stack eventually
            StackEntity& value = stack.getArg(stackIndex++);

            // Check that the type is equal.
            CHECK(compareElements(apartment.getObjects().getTypedefRepository(), args[i], value));

            // Push stack value/stack address or whatever...
            // For now assume all that are 32 bit devices.
            uint objectSize = apartment.getObjects().getTypedefRepository().getTypeSize(args[i]);
            if(objectSize > (uint)compiler.getStackSize())
            {
                objectSize = MethodCompiler::stackAlign(objectSize, compiler);
                //allocate space on the stack for the copied struct
                StackEntity structSizeEntity = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
                structSizeEntity.getConst().setConstValue(objectSize);
                RegisterEvaluatorOpcodes::evaluateInt32(emitContext, structSizeEntity);

                TemporaryStackHolderPtr newStackPositionHolder(new TemporaryStackHolder(
                                    currentBlock,
                                    ELEMENT_TYPE_I4,
                                    CompilerInterface::STACK_32,
                                    TemporaryStackHolder::TEMP_ONLY_REGISTER));

                compiler.localloc(newStackPositionHolder->getTemporaryObject(), structSizeEntity.getStackHolderObject()->getTemporaryObject(),false);

                //push dest address of new stack position
                StackEntity newStackPositionEntity(StackEntity::ENTITY_REGISTER, ConstElements::gVoidPtr);
                newStackPositionEntity.setStackHolderObject(newStackPositionHolder);
                stack.push(newStackPositionEntity);

                //push src address of struct
                value.AddressUpType();
                TemporaryStackHolderPtr tempRefernceStackHolder;
                if (value.getType() == StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR)
                {
                    // This is done in order to increase the refcount of the struct by 1
                    // We need to do this because the evaluateInt32 after us has an optimization that once the call has ended we can dereference
                    // the temp stack because the return register holds the value. but since under the case of ENTITY_LOCAL_TEMP_STACK_PTR it
                    // is pointing to the temp stack we don't want it to be freed, so we increase it so the decrease won't cause the free.
                    tempRefernceStackHolder = value.getStackHolderObject();
                }
                RegisterEvaluatorOpcodes::evaluateInt32(emitContext, value);
                stack.push(value);

                //push size of copy
                structSizeEntity = StackEntity(StackEntity::ENTITY_CONST, ConstElements::gU);
                structSizeEntity.getConst().setConstValue(objectSize);
                stack.push(structSizeEntity);

                // Add a "call" instruction to memcpy
                CallingConvention::call(emitContext,
                                    emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getMemcpy());

                argSize += objectSize;
            }
            else
            {
                RegisterEvaluatorOpcodes::evaluateInt32(emitContext, value);
                // And push as method argument
                compiler.pushArg32(value.getStackHolderObject()->getTemporaryObject());

                if (!emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().isFrameworkMethod(mid))
                {
                    // IncRef all object arguments, but not when calling framework methods!
                    // Note: No need to DestIfNoRef, because we just incref'ed. So the callee's cleanup will decref and destroy
                    //if (value.getElementType().isObject())
                    if (args[i].isObject())
                    {
                        TokenIndex incObj = emitContext.methodContext.getApartment()->getObjects().getFrameworkMethods().getIncObj();
                        cString sIncObjDepName(serializedMethod(incObj));
                        compiler.pushArg32(value.getStackHolderObject()->getTemporaryObject());
                        compiler.call(sIncObjDepName, 1);
                        // And revert the stack for cdecl
                        if (getDesiredCallingMethod(getTokenID(incObj),
                                                    *apartment.getApt(incObj),
                                                    emitContext.methodRuntime.getCompiler()) != CompilerInterface::STDCALL)
                        {
                            // Need to clear calling argument
                            compiler.revertStack(4);
                        }
                    }
                }

                value = StackEntity();
                argSize += 4;
            }
        }
    }

    // Push 'this' pointer for instance methods
    if (!retType.isVoid())
    {
        retSize = apartment.getObjects().getTypedefRepository().getTypeSize(retType);
    }

    if (methodSignature->isHasThis())
        {
        // Skip the "this" object, if it appears here
        // Note - we already have a reference to it.
        if (thisHandling == ThisBelowParams)
            stackIndex++;

        bool isConstrained = false;
        StackEntity* constrainedObj = NULL;
        if ((thisObj->getType() == StackEntity::ENTITY_TOKEN_ADDRESS) &&
            (EncodingUtils::getTokenTableIndex(getTokenID(thisObj->getConst().getTokenIndex())) != 0x70))
        {
            // .constrained was called (Otherwise it's string, which will re-eval)
            isConstrained = true;
            CHECK(thisHandling == ThisBelowParams);
            constrainedObj = thisObj;
            // In this case "this" appears twice on the stack - take the lower one instead
            // And remember to pop both of them from the stack eventually
            thisObj = &stack.getArg(stackIndex++);
        }

        RegisterEvaluatorOpcodes::evaluateInt32(emitContext, *thisObj);
        compiler.pushArg32(thisObj->getStackHolderObject()->getTemporaryObject());
        argSize += 4;

        if (isVirtual)
        {
            // Calculate the index of the method in the class's vtbl
            TokenIndex typedefParent = buildTokenIndex(getApartmentID(methodToken),
                                               apartment.getTables().getTypedefParent(mdMethodToken));
            GlobalContext& globalContext = emitContext.methodContext.getApartment()->getObjects();
            const ResolverInterface::VirtualTable& vtbl =
                globalContext.getTypedefRepository().getVirtualTable(typedefParent);
            int index = -1, j = 0;
            for (ResolverInterface::VirtualTable::iterator i = vtbl.begin();
                 i != vtbl.end();
                 i++, j++)
            {
                if (getVtblMethodIndexOriginal(*i) == methodToken)
                {
                    index = j;
                    break;
                }
            }

            // Invoke a call to index
            if (index >= 0)
            {
                // Determine the correct vtbl
                if (isConstrained)
                {
                    StackEntity constrainedVtbl = *constrainedObj;
                    RegisterEvaluatorOpcodes::evaluateInt32(emitContext, constrainedVtbl);
                    // Push the constrained vtbl on the stack
                    stack.push(constrainedVtbl);
                }
                else
                {
                    // Invoke getVtblObj in order to fetch this's vtbl
                    stack.push(*thisObj);
                    CallingConvention::call(emitContext,
                                            globalContext.getFrameworkMethods().getVtblObj());
                }
                StackEntity vtblReg;
                StackEntity offset(StackEntity::ENTITY_CONST, ConstElements::gVoid);

                // Call getIfaceLoc() to obtain the offset of the desired interface within the object's full vtbl
                if (globalContext.getTypedefRepository().isTypeInterface(typedefParent))
                {
                    // Duplicate parent vtbl
                    stack.push(stack.peek());
                    // Push interface vtbl
                    StackEntity vtblEntitiy(StackEntity::ENTITY_CONST, ConstElements::gU2);
                    vtblEntitiy.getConst().setConstValue(globalContext.getTypedefRepository().getRTTI(typedefParent));
                    stack.push(vtblEntitiy);
                    // And emit the call
                    CallingConvention::call(emitContext,
                                            globalContext.getFrameworkMethods().getIfaceLoc());
                    // Get the interface relative offset
                    offset = stack.getArg(0);
                    stack.pop2null(); // uint
                }

                vtblReg = stack.getArg(0);
                stack.pop2null(); // unsafe void*

                // make sure we are register, nothing will be done if vtblReg remains a register
                RegisterEvaluatorOpcodes::evaluateInt32(emitContext, vtblReg);
                if (offset.getType() != StackEntity::ENTITY_CONST)
                {
                    // Add interface offset to get the correct index within the vtbl
                    compiler.add32(vtblReg.getStackHolderObject()->getTemporaryObject(),
                                   offset.getStackHolderObject()->getTemporaryObject());
                }

                // Load the method pointer from the slot in the vtbl
                compiler.loadMemory(vtblReg.getStackHolderObject()->getTemporaryObject(),
                                    vtblReg.getStackHolderObject()->getTemporaryObject(),
                                    index * compiler.getStackSize(),
                                    compiler.getStackSize());

                // Pop all of the elements that we used from the stack (this, params, etc)
                stack.pop2null(stackIndex);
                thisObj = NULL;

                // Do the virtual call!
                if (retType.isVoid())
                {
                    compiler.call(vtblReg.getStackHolderObject()->getTemporaryObject(),
                                  argSize / 4); // TODO! 32bit call
                }
                else
                {
                    // Call, assign ret value to vtblReg, and push it
                    compiler.call32(vtblReg.getStackHolderObject()->getTemporaryObject(),
                                    vtblReg.getStackHolderObject()->getTemporaryObject(),
                                    argSize / 4); // TODO! 32bit call
                    stack.push(vtblReg);
                }

                // Clear stdcall arguments
                if (callingMethod != CompilerInterface::STDCALL)
                    compiler.revertStack(argSize);

                return;
            }
            else
            {
                CompilerTrace("\t\tWarning! virtual function couldn't be located in vtbl" << endl);
            }
        }
    }

    if ((thisObj != NULL) && (thisHandling == ThisAboveParamsDup))
        {
        // Create a temporary copy of "this"
        StackEntity copyOfThis(*thisObj);

        // Pop all of the elements that we used from the stack (this, params, etc)
        stack.pop2null(stackIndex);

        // push thisObj back onto the stack, so that after we return it can be used
        stack.push(copyOfThis);
        }
    else
    {
        // Pop all of the elements that we used from the stack (this, params, etc)
        stack.pop2null(stackIndex);
    }
    thisObj = NULL;

    // Get dependency name
    cString dependencyTokenName(serializedMethod(methodToken));

    // Calling the method itself, and setup stack for returned address
    if (retType.isVoid())
    {
        compiler.call(dependencyTokenName, argSize / 4);
    } else  if(retSize > (uint)compiler.getStackSize())
    {
        compiler.call(dependencyTokenName, argSize / 4);

                    //allocate on the stack a temporary the size of the struct
        ASSERT(!structRetHolder.isEmpty());

        //push ret struct to the stack
        StackEntity structRetStackEntity(StackEntity::ENTITY_LOCAL_TEMP_STACK_PTR, retType);
        structRetStackEntity.setStackHolderObject(structRetHolder);
        stack.push(structRetStackEntity);
    } else
    {
            // Allocate return variable.
         TemporaryStackHolderPtr ret(new TemporaryStackHolder(
                            emitContext.currentBlock,
                            retType.getType(),
                            CompilerInterface::STACK_32,
                            TemporaryStackHolder::TEMP_ONLY_REGISTER));

        // Call the method, and fill the return value variable
        compiler.call32(dependencyTokenName,
                        ret->getTemporaryObject(), argSize / 4);

        // push returned value back to stack
        StackEntity retValue(StackEntity::ENTITY_REGISTER, retType, true);
        retValue.setStackHolderObject(ret);
        stack.push(retValue);
    }

    // Clear the stack if we have cdecl calling convention
    if (callingMethod != CompilerInterface::STDCALL)
    {
        // Need to clear calling argument
        compiler.revertStack(argSize);
    }
}

TokenIndex CallingConvention::buildCCTOR(const TokenIndex& parentTypedef)
{
    return buildTokenIndex(getApartmentID(parentTypedef),
                                EncodingUtils::buildToken(TABLE_CLR_CCTOR_WRAPPERS,
                                     EncodingUtils::getTokenPosition(getTokenID(parentTypedef))));
}

cString CallingConvention::serializedMethod(const TokenIndex& token)
{
    // TODO! Apartment name!

    if ((getTokenID(token) == 0) || (getTokenID(token) == 0xCCCCCCCC))
    {
        CompilerTrace("serializedMethod: Adding wrong function modulation" << HEXTOKEN(token) << endl);
        CHECK_FAIL();
    }

    if (EncodingUtils::getTokenTableIndex(getTokenID(token)) == TABLE_MEMBERREF_TABLE)
    {
        CompilerTrace("serializedMethod: Warning serialized MethodRef (external) token: " << HEXTOKEN(token) << endl);
    }

    cString relocationTokenName(gCILMethodPrefix);
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getApartmentID(token));
    relocationTokenName+= '-';
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getTokenID(token));
    return relocationTokenName;
}

/*
cString CallingConvention::serializeGlobal(uint offset)
{
    cString relocationGlobalName(gCILGlobalPrefix);
    relocationGlobalName+= "0x";
    relocationGlobalName+= HEXDWORD(offset);
    return relocationGlobalName;
}
*/

cString CallingConvention::serializeGlobalData(uint offset)
{
    cString relocationGlobalName(gCILGlobalDataPrefix);
    relocationGlobalName+= "0x";
    relocationGlobalName+= HEXDWORD(offset);
    return relocationGlobalName;
}

cString CallingConvention::serializeToken(const TokenIndex& token)
{
    if (getTokenID(token) == 0xCCCCCCCC)
    {
        CompilerTrace("serializeToken: Adding wrong function modulation" << HEXTOKEN(token) << endl);
        CHECK_FAIL();
    }

    // TODO! Apartment name!
    cString relocationTokenName(gCILTokenPrefix);
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getApartmentID(token));
    relocationTokenName+= '-';
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getTokenID(token));
    return relocationTokenName;
}

bool CallingConvention::deserializeMethod(const cString& dependency,
                                          TokenIndex& token)
{
    token = ElementType::UnresolvedTokenIndex;
    uint len = arraysize(gCILMethodPrefix) - 1;
    if (dependency.left(len) != gCILMethodPrefix)
        return false;

    cSArray<char> ascii = dependency.part(len, MAX_UINT32).getASCIIstring();
    Parser parser(ascii.getBuffer(), ascii.getBuffer(), ascii.getSize() - 1, 0);
    getApartmentID(token) = parser.readCUnsignedInteger();
    CHECK(parser.readChar() == '-');
    getTokenID(token) = parser.readCUnsignedInteger();
    return true;
}

bool CallingConvention::deserializeToken(const cString& dependency,
                                         TokenIndex& token)
{
    token = ElementType::UnresolvedTokenIndex;
    uint len = arraysize(gCILTokenPrefix) - 1;
    if (dependency.left(len) != gCILTokenPrefix)
        return false;

    cSArray<char> ascii = dependency.part(len, MAX_UINT32).getASCIIstring();
    Parser parser(ascii.getBuffer(), ascii.getBuffer(), ascii.getSize() - 1, 0);
    getApartmentID(token) = parser.readCUnsignedInteger();
    CHECK(parser.readChar() == '-');
    getTokenID(token) = parser.readCUnsignedInteger();
    return true;
}

/*
bool CallingConvention::deserializeGlobal(const cString& dependency,
                                          uint& offset)
{
    offset = 0;
    uint len = arraysize(gCILGlobalPrefix) - 1;
    if (dependency.left(len) != gCILGlobalPrefix)
        return false;

    cSArray<char> ascii = dependency.part(len, MAX_UINT32).getASCIIstring();
    Parser parser(ascii.getBuffer(), ascii.getBuffer(), ascii.getSize() - 1, 0);
    offset = parser.readCUnsignedInteger();
    return true;
}
*/
bool CallingConvention::deserializeGlobalData(const cString& dependency,
                                              uint& offset)
{
    offset = 0;
    uint len = arraysize(gCILGlobalDataPrefix) - 1;
    if (dependency.left(len) != gCILGlobalDataPrefix)
        return false;

    cSArray<char> ascii = dependency.part(len, MAX_UINT32).getASCIIstring();
    Parser parser(ascii.getBuffer(), ascii.getBuffer(), ascii.getSize() - 1, 0);
    offset = parser.readCUnsignedInteger();
    return true;
}

void CallingConvention::calculateArgumentsSizes(const CompilerInterface& compiler,
                                                ApartmentPtr& apartment,
                                                const MethodCompiler& methodCompiler,
                                                const MethodDefOrRefSignature& signature,
                                                ArgumentsPositions& args,
                                                ElementType& thisType)
{
    const ElementsArrayType& types = signature.getParams();
    uint count = types.getSize();
    uint startRealArgs = 0;
    uint retSize = apartment->getObjects().getTypedefRepository().getTypeSize(signature.getReturnType());

    if (signature.isHasThis())
    {
        count++;
        startRealArgs = 1;
    }


    if (retSize > (uint)compiler.getStackSize())
    {
        ASSERT(!signature.isHasThis()); //what to do in this case?
        count++;
        startRealArgs = 1;
    }

    args.m_argumentPosition.changeSize(count);
    args.m_argumentSizes.changeSize(count);
    args.m_argumentTypes.changeSize(count);

    uint argSize = 0;
    if (signature.isHasThis())
    {
        args.m_argumentPosition[0] = 0;
        argSize = args.m_argumentSizes[0] = compiler.getStackSize();
        args.m_argumentTypes[0] = thisType;
    }
    if (retSize > (uint)compiler.getStackSize())
    {
        ASSERT(!signature.isHasThis()); //what to do in this case?
        args.m_argumentPosition[0] = 0;
        argSize = args.m_argumentSizes[0] = compiler.getStackSize();
        args.m_argumentTypes[0] = signature.getReturnType();
        args.m_argumentTypes[0].setPointerLevel(args.m_argumentTypes[0].getPointerLevel()+1);
    }


    for (uint i = startRealArgs; i < count; i++)
    {
        const ElementType& arg(types[i - startRealArgs]);
        uint objectSize = apartment->getObjects().getTypedefRepository().getTypeSize(arg);
        if (arg.isObjectAndNotValueType())
        {
            // Class object. Not ready yet. Need to increase reference?!
            objectSize = compiler.getStackSize();
        }
        //else if (objectSize > (uint)compiler.getStackSize())
        //{
        //    // Structure containing members - is passed as a pointer
        //    // (see call())
        //    objectSize = compiler.getStackSize();
        //}
        else
        {
            // For POINTER/REF/BY VAL/...
            if (types[i- startRealArgs].isPointer())
            {
                objectSize = compiler.getStackSize();
            }
            else {
                // For normal length
                objectSize = methodCompiler.stackAlign(objectSize, compiler);
            }
        }
        args.m_argumentTypes[i] = arg;
        args.m_argumentPosition[i] = argSize;
        args.m_argumentSizes[i] = objectSize;
        argSize += objectSize;
    }

    args.m_totalArgumentsSize = argSize;
}
