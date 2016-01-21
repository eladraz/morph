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

#include "data/ConstElements.h"
#include "runnable/ClrResolver.h"
#include "format/tables/MethodTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/MemberRefTable.h"
#include "runnable/StringReader.h"
#include "format/EncodingUtils.h"
#include "executer/ExecuterTrace.h"
#include "runnable/GlobalContext.h"
#include "runnable/CorlibNames.h"
#include "runnable/RunnableTrace.h"


static void get_indexes(ApartmentPtr apartment,
                        mdToken method_id,
                        mdToken& method_name_index,
                        mdToken& method_classname_index,
                        mdToken& method_namespace_index,
                        mdToken& method_signature_index
                        )
{
    unsigned int table_id  = EncodingUtils::getTokenTableIndex(method_id);

    const MemberRefTable& table = (const MemberRefTable&) (*apartment->getTables().getTableByToken(method_id));
    const mdToken typedefToken = apartment->getTables().getTypedefParent(table.getToken());
    const TypedefTable::Header& typedef_header = ((const TypedefTable&)(*apartment->getTables().getTableByToken(typedefToken))).getHeader();

    method_name_index = table.getHeader().m_name;
    method_signature_index = table.getHeader().m_signature;

    method_classname_index = typedef_header.m_name;
    method_namespace_index = typedef_header.m_namespace;
}

void ClrResolver::translateMethodCoreNames(cString& namespaceName,
                                           cString& /*className*/)
{
    uint pos = namespaceName.find(".");
    if (namespaceName.left(pos).compare(CorlibNames::m_systemNamespace) == cString::EqualTo)
    {
        namespaceName = cString(CorlibNames::gCoreNamespace) + namespaceName.right(namespaceName.length() - pos);
    }
}

void ClrResolver::readMethodSignatureAndNames(ApartmentPtr& apartment,
                                              const TokenIndex& methodToken,
                                              cString& methodName,
                                              cString& methodClassname,
                                              cString& methodNamespace,
                                              MethodDefOrRefSignaturePtr& methodSignature)
{
    mdToken method_id = getTokenID(methodToken);
    mdToken apartment_id = getApartmentID(methodToken);

    apartment = apartment->getApartmentByID(apartment_id);
    /////

    mdToken method_name_index = 0;
    mdToken method_classname_index = 0;
    mdToken method_namespace_index = 0;
    mdToken methodSignatureIndex = 0;

    // Get indexes to strings & signature
    get_indexes(apartment, method_id,
                method_name_index, method_classname_index, method_namespace_index, methodSignatureIndex);

    // Read strings.
    cForkStreamPtr stringStream = apartment->getStreams().getStringsStream()->fork();
    methodName = StringReader::readStringName(*stringStream, method_name_index);
    methodClassname = StringReader::readStringName(*stringStream, method_classname_index);
    methodNamespace = StringReader::readStringName(*stringStream, method_namespace_index);

    // Read signature.
    cForkStreamPtr blobStream = apartment->getStreams().getBlobStream()->fork();
    blobStream->seek(methodSignatureIndex, basicInput::IO_SEEK_SET);
    XSTL_TRY
    {
        methodSignature = MethodDefOrRefSignaturePtr(new MethodDefOrRefSignature(*blobStream, apartment->getUniqueID(),
                                                                                 apartment->getObjects().getTypedefRepository()));
    }
    XSTL_CATCH_ALL
    {
        RunnableTrace("Error reading signature for method: " << methodNamespace+"."+methodClassname+"."+methodName   << endl);
        XSTL_RETHROW;
    }

    // Request to "System" namespace is redirected to "Morph".
    // Our DLL (netcore) implements the Morph namespace.
    translateMethodCoreNames(methodNamespace, methodClassname);

}

TokenIndex ClrResolver::resolve(ApartmentPtr& apartment, const TokenIndex& methodToken)
{
    if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) != TABLE_MEMBERREF_TABLE)
        return methodToken;

    cString methodName;
    cString methodClassname;
    cString methodNamespace;
    MethodDefOrRefSignaturePtr methodSignature;

    readMethodSignatureAndNames(apartment, methodToken, methodName, methodClassname, methodNamespace, methodSignature);

    return apartment->getObjects().getTypesNameRepository().getMethodToken(methodNamespace, methodClassname, methodName, *methodSignature);
}

void ClrResolver::resolveUnboxType(ApartmentPtr& apartment, const TokenIndex& typeToken, ElementType& unboxType)
{
    TokenIndex tt = apartment->getObjects().getTypesNameRepository().translateTyperefToTypedef(typeToken);
    cString typeName = apartment->getObjects().getTypesNameRepository().getTypeName(tt);
    cString typeNamespace = apartment->getObjects().getTypesNameRepository().getTypeNamespace(tt);

    translateMethodCoreNames(typeNamespace, typeName);

    if (typeNamespace == CorlibNames::gCoreNamespace)
    {
        // Todo! Add to CorlibNames
        if (typeName == "Byte")
            unboxType = ConstElements::gByte;
        else if (typeName == "Char")
            unboxType = ConstElements::gChar;
        else if (typeName == "Int32")
            unboxType = ConstElements::gI4;
        else if (typeName == "UInt32")
            unboxType = ConstElements::gU4;
        else if (typeName == "Boolean")
            unboxType = ConstElements::gBool;
        else if (typeName == "Int16")
            unboxType = ConstElements::gI2;
        else if (typeName == "UInt16")
            unboxType = ConstElements::gU2;
        else if (typeName == "SByte")
            unboxType = ConstElements::gI1;
        else if (typeName == "Byte")
            unboxType = ConstElements::gU1;
    }
}

void ClrResolver::resolveUnboxType(ApartmentPtr& apartment, ElementType& type)
{
    if (!type.isObject())
        return;

    ElementType newType;
    resolveUnboxType(apartment, type.getClassToken(), newType);
    if (!newType.isVoid())
    {
        type = ElementType(newType.getType(), type.isPointer(), type.isReference(), type.isPinned(), type.isSingleDimensionArray());
    }
}
