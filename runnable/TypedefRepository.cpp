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
 * TypedefRepository.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/stream/traceStream.h"
#include "xStl/enc/digest/Crc16.h"
#include "xStl/enc/digest/Crc64.h"
#include "xStl/utils/algorithm.h"
#include "xStl/data/datastream.h"
#include "data/exceptions.h"
#include "data/ConstElements.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/Table.h"
#include "format/tables/FieldTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/InterfaceImplTable.h"
#include "format/tables/FieldTable.h"
#include "format/tables/TypeSpecTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/tables/MethodImplTable.h"
#include "format/signatures/FieldSig.h"
#include "runnable/RunnableTrace.h"
#include "runnable/ResolverInterface.h"
#include "runnable/TypedefRepository.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "runnable/CorlibNames.h"
#include "runnable/FrameworkMethods.h"
#include "runnable/ClrResolver.h"
#include "format/tables/ClassLayoutTable.h"

// Construct new empty virtual table
const TypedefRepository::VirtualTable TypedefRepository::m_emptyVirtualTable;
cCounter TypedefRepository::gRttiCounter(1); // Class always starts with 1, since 0 is no inheritance

TypedefRepository::TypedefRepository(const ApartmentPtr& apartment,
                                     const MemoryLayoutInterface& memoryLayoutInterface) :
    m_apartment(apartment),
    m_memoryLayout(memoryLayoutInterface),
    m_staticDBLength(0),
    m_tokenSystemObject(ElementType::UnresolvedTokenIndex),
    m_tokenSystemString(ElementType::UnresolvedTokenIndex),
    m_tokenSystemArray(ElementType::UnresolvedTokenIndex),
    m_tokenSystemValueType(ElementType::UnresolvedTokenIndex),
    m_tokenStringConstructor(ElementType::UnresolvedTokenIndex)
{
    addApartment(apartment);
}

void TypedefRepository::addApartment(const ApartmentPtr& apartment)
{
    // Check and see if we have System.Object (Morph.Object) or (System.String)
    m_tokenSystemObject =
        m_apartment->getObjects().getTypesNameRepository().getTypeToken(
            CorlibNames::gCoreNamespace, CorlibNames::m_classObject);
    m_tokenSystemValueType =
        m_apartment->getObjects().getTypesNameRepository().getTypeToken(
            CorlibNames::gCoreNamespace, CorlibNames::m_classValueType);
    m_tokenSystemString =
        m_apartment->getObjects().getTypesNameRepository().getTypeToken(
            CorlibNames::gCoreNamespace, CorlibNames::m_classString);
    m_tokenSystemArray =
        m_apartment->getObjects().getTypesNameRepository().getTypeToken(
            CorlibNames::gCoreNamespace, CorlibNames::m_classArray);
    m_tokenStringConstructor = m_apartment->getObjects().getTypesNameRepository().getMethodToken(
            CorlibNames::gCoreNamespace, CorlibNames::m_classString, CorlibNames::m_methodCtor, *FrameworkMethods::getStringCtorSignature());
}

void TypedefRepository::doneLoadingApartments()
{
    // Start by adding framework types
    repoApartment(m_apartment->getApartmentByID(getApartmentID(m_tokenSystemObject)));
    // Get the list of apartments and short by name
    cList<cString> apartments;
    m_apartment->getApartmentsNames(apartments);
    // Sort list
    boubbleSort(apartments.begin(), apartments.end());

    cList<cString>::iterator i = apartments.begin();
    for (; i != apartments.end(); ++i)
    {
        ApartmentPtr apt = m_apartment->getApartmentByName(*i);
        if (apt->getUniqueID() != getApartmentID(m_tokenSystemObject))
        {
            repoApartment(apt);
        }
    }
}

void TypedefRepository::repoApartment(const ApartmentPtr& apartment)
{
    uint typedefTablesSize = apartment->getTables().getNumberOfRows(TABLE_TYPEDEF_TABLE);
    for (uint i = 0; i < typedefTablesSize; i++)
    {
        // The typedef table
        mdToken tdmdToken = EncodingUtils::buildToken(TABLE_TYPEDEF_TABLE, i+1);
        TokenIndex tid = buildTokenIndex(apartment->getUniqueID(), tdmdToken);
        {
            cLock lock(m_lock);
            lockAppendTypedef(tid);
        }
    }
}

const TokenIndex& TypedefRepository::getStringConstructor() const
{
    return m_tokenStringConstructor;
}

void TypedefRepository::resolveTyperef(ElementType& type) const
{
    if (type == ConstElements::gVoidArray)
    {
        type = ElementType(ELEMENT_TYPE_CLASS, 0, false, false, false, m_tokenSystemArray);
        return;
    }

    switch (type.m_type)
    {
        case ELEMENT_TYPE_OBJECT:
            type.m_classToken = m_tokenSystemObject;
            break;
        case ELEMENT_TYPE_STRING:
            type.m_classToken = m_tokenSystemString;
            break;
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE:
            // Check that the token is not empty and the
            ASSERT(type.m_classToken != ElementType::UnresolvedTokenIndex);
            // TODO! Move into function?
            if (EncodingUtils::getTokenTableIndex(getTokenID(type.m_classToken)) == TABLE_TYPESPEC_TABLE)
            {
                // Read the blob token
                ApartmentPtr apt = m_apartment->getApt(type.m_classToken);
                cForkStreamPtr blob = apt->getStreams().getBlobStream()->fork();
                const TypeSpecTable::Header& typeSpec = ((TypeSpecTable&)(*apt->getTables().getTableByToken(getTokenID(type.m_classToken)))).getHeader();
                blob->seek(typeSpec.m_signature + 1, basicInput::IO_SEEK_SET);
                type = ElementType::readType(*blob, 0);
                resolveTyperef(type);
                return;
            }
            type.m_classToken = m_apartment->getObjects().getTypesNameRepository().
                                    translateTyperefToTypedef(type.m_classToken);
            if (isTypedefClass(type.m_classToken))
                type.m_type = ELEMENT_TYPE_CLASS;
            else
                type.m_type = ELEMENT_TYPE_VALUETYPE;

            if (type.m_classToken == m_tokenSystemObject)
            {
                type.m_type = ELEMENT_TYPE_OBJECT;
            } else if (type.m_classToken == m_tokenSystemString)
            {
                type.m_type = ELEMENT_TYPE_STRING;
            }
            break;

        case ELEMENT_TYPE_VAR:
        case ELEMENT_TYPE_GENERICINST:
            // TODO!
            CHECK_FAIL();
        default:
            break;
    }
}

void TypedefRepository::resolveFieldref(TokenIndex& fieldToken, const TokenIndex& parentType) const
{
    // Read name and signature
    if (EncodingUtils::getTokenTableIndex(getTokenID(fieldToken)) == TABLE_FIELD_TABLE)
        return;
    CHECK(EncodingUtils::getTokenTableIndex(getTokenID(fieldToken)) == TABLE_MEMBERREF_TABLE);
    ElementType::assertTyperef(parentType);

    ApartmentPtr apt = m_apartment->getApt(fieldToken);
    const MemberRefTable::Header& memberRefTable = ((MemberRefTable&)(*apt->getTables().getTableByToken(getTokenID(fieldToken)))).getHeader();

    cForkStreamPtr blob = apt->getStreams().getBlobStream()->fork();
    blob->seek(memberRefTable.m_signature, basicInput::IO_SEEK_SET);
    FieldSig fieldSignature(*blob, apt->getUniqueID(), *this);
    cString fieldName(StringReader::readStringName(*apt->getStreams().getStringsStream()->fork(), memberRefTable.m_name));

    // Scan for field name and signature
    cLock lock(m_lock);
    lockCheckAppendTypedef(parentType);
    cList<TokenIndex> fields = m_types[parentType].m_fields.keys();
    cList<TokenIndex>::iterator i(fields.begin());
    for (; i != fields.end(); ++i)
    {
        // Read the field name and signature
        ApartmentPtr apt = m_apartment->getApt(*i);
        const MemberRefTable::Header& fieldHeader = ((MemberRefTable&)(*apt->getTables().getTableByToken(getTokenID(*i)))).getHeader();
        blob = apt->getStreams().getBlobStream()->fork();
        blob->seek(fieldHeader.m_signature, basicInput::IO_SEEK_SET);
        FieldSig newFieldSignature(*blob, apt->getUniqueID(), *this);
        cString newFieldName(StringReader::readStringName(*apt->getStreams().getStringsStream()->fork(), fieldHeader.m_name));

        if ((newFieldName == fieldName) &&
            (newFieldSignature.getType() == fieldSignature.getType()))
        {
            // Found
            fieldToken = *i;
            return;
        }
    }
}

const uint TypedefRepository::getRTTI(const TokenIndex& typedefToken) const
{
    ElementType::assertTyperef(typedefToken);
    cLock lock(m_lock);
    return lockGetRTTI(typedefToken);
}

const TypedefRepository::VirtualTable& TypedefRepository::getVirtualTable(
                                             const TokenIndex&  typedefToken) const
{
    ElementType::assertTyperef(typedefToken);
    cLock lock(m_lock);
    return lockGetVirtualTable(typedefToken);
}

const TypedefRepository::ParentDictonary& TypedefRepository::getParentDirectory(
                                            const TokenIndex& typedefToken) const
{
    ElementType::assertTyperef(typedefToken);
    cLock lock(m_lock);
    lockCheckAppendTypedef(typedefToken);
    return m_types[typedefToken].m_extends;
}

uint TypedefRepository::getTypeSize(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);
    return lockGetTypeSize(typeToken);
}

uint TypedefRepository::getTypeSize(const ElementType& typeToken) const
{
    typeToken.assertTyperef();
    cLock lock(m_lock);
    return innerGetTypeSize(typeToken);
}

bool TypedefRepository::isTypeInterface(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);
    lockCheckAppendTypedef(typeToken);
    return m_types[typeToken].m_isInterface;
}

bool TypedefRepository::isTypeShouldDref(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);
    lockCheckAppendTypedef(typeToken);
    return m_types[typeToken].m_isSpecialCleanup;
}

uint TypedefRepository::getStaticFieldOffset(const TokenIndex& fieldToken) const
{
    return m_staticDB[fieldToken];
}

uint TypedefRepository::getStaticTotalLength() const
{
    return m_staticDBLength;
}

TokenIndex TypedefRepository::allocateStatic(uint size)
{
    // Build return token
    TokenIndex ret = buildTokenIndex(Apartment::HELPER_APARTMENT, m_apartment->generateMethodHelperRow());
    getTokenID(ret) = EncodingUtils::buildToken(TABLE_FIELD_TABLE, EncodingUtils::getTokenPosition(getTokenID(ret)));

    cLock lock(m_lock);
    // Add token to static
    size = m_memoryLayout.align(size);
    m_staticDB.append(ret, size);
    m_staticDBLength+= size;
    return ret;
}

uint TypedefRepository::allocateDataSection(cForkStreamPtr& stream, uint size)
{
    cLock lock(m_lock);
    uint ret = m_dataBuffer.getSize();
    stream->pipeRead(m_dataBuffer, size);
    return ret;
}

const cBuffer& TypedefRepository::getDataSection()
{
    return m_dataBuffer;
}

TokenIndex TypedefRepository::getStaticInitializerMethod(const TokenIndex& typeToken) const
{
    cLock lock(m_lock);
    lockCheckAppendTypedef(typeToken);
    return m_types[typeToken].m_staticInitializerMethod;
}

const TypedefRepository::FieldsDictonary& TypedefRepository::getAllFields(const TokenIndex& parentToken) const
{
    cLock lock(m_lock);
    lockCheckAppendTypedef(parentToken);

    // Exception might be thrown here. In this case the tables are corrupted.
    return m_types[parentToken].m_fields;
}

uint TypedefRepository::getFieldRelativePosition(const TokenIndex& fieldToken,
                                                 const TokenIndex& parentToken) const
{
    TokenIndex pt = resolveParentToken(fieldToken, parentToken);

    cLock lock(m_lock);
    lockCheckAppendTypedef(pt);

    // Exception might be thrown here. In this case the tables are corrupted.
    return m_types[pt].m_fields[fieldToken].m_offset;
}

const ElementType& TypedefRepository::getFieldType(const TokenIndex& fieldToken,
                                                   const TokenIndex& parentToken) const
{
    if (getApartmentID(fieldToken) == Apartment::HELPER_APARTMENT)
    {
        switch (m_staticDB[fieldToken])
        {
        case 1:
            return ConstElements::gByte;
        case 2:
            return ConstElements::gU2;
        case 4:
            return ConstElements::gU4;
        default:
            CHECK_FAIL();
        }
    } else
    {
        TokenIndex pt = resolveParentToken(fieldToken, parentToken);
        cLock lock(m_lock);
        lockCheckAppendTypedef(pt);
        // Exception might be thrown here. In this case the tables are corrupted.
        return m_types[pt].m_fields[fieldToken].m_type;
    }
}

TokenIndex TypedefRepository::getTypeToken(const cString& _namespace,
                                           const cString& _className) const
{
    return m_apartment->getObjects().getTypesNameRepository().getTypeToken(
                        _namespace,
                        _className);
}

 TokenIndex TypedefRepository::getNewGenericInstanceToken(const ElementType& type)
{
    // Check that the token is indeed generic value
    TokenIndex classToken = type.getClassToken();
    if (type.getType() != ELEMENT_TYPE_GENERICINST)
        return classToken;

    type.assertTyperef();
    classToken = type.getGenericClass().getClassToken();

    // Calculate hash. TODO! Switch to CRC32
    CRC16 crc;
    crc.update((const byte*)&classToken, sizeof(classToken));
    const cArray<ElementType>& genericVal = type.getGenericTypes();
    for (uint i = 0; i < genericVal.getSize(); i++)
    {
        crc.update(genericVal[i].getType());
        crc.update((const byte*)&genericVal[i].getClassToken(), sizeof(TokenIndex));
    }
    // Find the token
    TokenIndex newToken = buildTokenIndex(getApartmentID(classToken),
                                          EncodingUtils::buildToken(TABLE_CLR_GENERICS_INSTANCES, crc.getValue()));
    while (m_genericInstances.hasKey(newToken))
    {
        // Check the generic
        if (m_genericInstances[newToken] == type)
            break;
        crc.update(1);
        newToken = buildTokenIndex(getApartmentID(classToken),
                        EncodingUtils::buildToken(TABLE_CLR_GENERICS_INSTANCES, crc.getValue()));
    }

    // Return the value.
    m_genericInstances.append(newToken, type);
    return newToken;
}

bool TypedefRepository::isTypedefClass(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);

    cLock lock(m_lock);
    lockCheckAppendTypedef(typeToken);

    // Check the typedef
    return !m_types[typeToken].m_extends.hasKey(m_tokenSystemValueType);
}

const cBuffer& TypedefRepository::getTypeHashSignature(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);

    cLock lock(m_lock);
    lockCheckAppendTypedef(typeToken);

    // And return value
    return m_types[typeToken].m_hashSignature;
}

uint TypedefRepository::estimateVirtualTableMaximumSize(uint callSize) const
{
    cLock lock(m_lock);
    cList<TokenIndex> types;
    m_types.keys(types);

    uint estimatedSize = 0;
    for (cList<TokenIndex>::iterator i = types.begin(); i != types.end(); i++)
    {
        // Adding vtbl
        estimatedSize += m_types[*i].m_virtualTable.length() * callSize;
        // Adding parntes, and number of total parents
        estimatedSize += (m_types[*i].m_extends.keys().length() + 2) * callSize;
    }

    return estimatedSize;
}

const TokenIndex& TypedefRepository::getSystemObject() const
{
    return m_tokenSystemObject;
}

//////////////////////////////////////////////////////////////////////////
// private methods


const uint TypedefRepository::lockGetRTTI(const TokenIndex&  typedefToken) const
{
    ElementType::assertTyperef(typedefToken);

    lockCheckAppendTypedef(typedefToken);
    return m_types[typedefToken].m_rtti;
}

const TypedefRepository::VirtualTable&
             TypedefRepository::lockGetVirtualTable(const TokenIndex& typedefToken) const
{
    ElementType::assertTyperef(typedefToken);

    lockCheckAppendTypedef(typedefToken);
    return m_types[typedefToken].m_virtualTable;
}

uint TypedefRepository::lockGetTypeSize(const TokenIndex& typedefToken) const
{
    ElementType::assertTyperef(typedefToken);

    lockCheckAppendTypedef(typedefToken);
    return m_memoryLayout.align(m_types[typedefToken].m_typedefSize);
}

void TypedefRepository::appendParents(TypedefRepository::ParentDictonary& dest,
                                      const TypedefRepository::ParentDictonary& source,
                                      uint& interfaceOffset) const
{
    // Get all interface methods
    cList<TokenIndex> keys;
    source.keys(keys);

    cList<TokenIndex>::iterator i = keys.begin();
    for (; i!= keys.end(); ++i)
    {
        TokenIndex parent(*i);
        if (!dest.hasKey(parent))
        {
            dest.append(parent, source[parent] + interfaceOffset);
            // And increase offset by the vtbl size
            // interfaceOffset += lockGetVirtualTable(parent).length();
        }
    }
}

void TypedefRepository::mergeInterfaceVirtualTables(TypedefRepository::VirtualTable& dest,
                                                    const TypedefRepository::VirtualTable& source)
{
    // Get all interface methods
    VirtualTable::iterator i = source.begin();

    for (; i != source.end(); ++i)
    {
        bool bSkip = false;
        TokenIndex virtualMethod(getVtblMethodIndexOriginal(*i));
        // For each virtual-table entry, check whether it exist in the
        // destination
        // Check for previous implemenation.
        VirtualTable::iterator j = dest.begin();
        for (; j != dest.end(); ++j)
        {
            if (virtualMethod == getVtblMethodIndexOriginal(*j))
            {
                // This method is already in the virtual table.
                // The source here is always an interface, so we prefer the existing method!
                bSkip = true;
                break;
            }
        }

        if (bSkip)
            continue;

        // Append to destination. If this fails, then we might have a problem
        dest.append(cDualElement<TokenIndex, TokenIndex>(virtualMethod, virtualMethod));
    }
}

//////////////////////////////////////////////////////////////////////////
/////////////
///////
// The main method.
//
// Calculate all typedef parameter
//
//

void TypedefRepository::lockCheckAppendTypedef(const TokenIndex& typedefToken) const
{
    if (!m_types.hasKey(typedefToken))
        lockAppendTypedef(typedefToken);
}


void TypedefRepository::lockAppendTypedef(const TokenIndex& _typedefToken) const
{
    // We already worked on that typedef
    TokenIndex typedefToken = _typedefToken;
    if (m_types.hasKey(typedefToken))
        return;

    // Checking for generic instance. TODO!
    if (EncodingUtils::getTokenTableIndex(getTokenID(typedefToken)) == TABLE_CLR_GENERICS_INSTANCES)
    {
        typedefToken = m_genericInstances[typedefToken].getGenericClass().getClassToken();
    }

    //////////////////////////////////////////////////////////////////////////
    // Get typedef tokens
    uint apartmentId = getApartmentID(typedefToken);
    const ApartmentPtr& apartment = m_apartment->getApartmentByID(apartmentId);
    cForkStreamPtr stringStream = apartment->getStreams().getStringsStream()->fork();
    cForkStreamPtr blobStream = apartment->getStreams().getBlobStream()->fork();

    TablePtr tablePtr(apartment->getTables().getTableByToken(getTokenID(typedefToken)));
    ASSERT(tablePtr->getIndex() == TABLE_TYPEDEF_TABLE);
    const TypedefTable& typedefTable = (const TypedefTable&)(*tablePtr);

    cString className = StringReader::readStringName(*stringStream, typedefTable.getHeader().m_name);
    cString namespaceName = StringReader::readStringName(*stringStream, typedefTable.getHeader().m_namespace);

    //////////////////////////////////////////////////////////////////////////
    // Add all extends to the typedef hash.
    // Check whether or not this class extends something else, or this class is
    // just an interface
    TypedefRepositoryContainer newType;
    newType.m_isCompleted = false;
    newType.m_isSpecialCleanup = false;
    newType.m_typedefSize = 0;
    newType.m_isInterface = false;
    newType.m_staticInitializerMethod = ElementType::UnresolvedTokenIndex;
    bool isSystemObject = (typedefToken == m_tokenSystemObject);

    if (EncodingUtils::getTokenPosition(typedefTable.getHeader().m_extends) ==
        getNilToken())
    {
        newType.m_isInterface = true;
        // Assert whether this class is an interface
        if (typedefTable.getHeader().m_flags == 0)
        {
            // Not sure why, maybe it's a "reserved" future typedef for the compiler
            return;
        }
        ASSERT((typedefTable.getHeader().m_flags & tdInterface) != 0);
    }

    // The newly node
    CRC64 crc64;
    uint typedefSize = 0;

    // Lock adding counter for RTTI information
    newType.m_rtti = gRttiCounter.increase();
    // Adding RTTI
    crc64.update(&newType.m_rtti, sizeof(newType.m_rtti));
    crc64.update(className.getBuffer(), className.length());
    crc64.update(namespaceName.getBuffer(), namespaceName.length());

    RunnableTrace("TypedefRepository: RTTI " << HEXWORD(newType.m_rtti) << " is " << HEXTOKEN(typedefToken) << " " << namespaceName << "." << className << endl);

    // Only for non-interfaces class/structs
    // Only for our System.Object don't include the real System.Object
    if ((!newType.m_isInterface) && (!isSystemObject))
    {
        TokenIndex extendIndex = m_apartment->getObjects().getTypesNameRepository().
                                    translateTyperefToTypedef(buildTokenIndex(apartmentId, typedefTable.getHeader().m_extends));

        lockAppendTypedef(extendIndex);
        // Update parent hash
        crc64.updateStream(m_types[extendIndex].m_hashSignature);
        newType.m_extends = m_types[extendIndex].m_extends;
        // Add current parent as prime father
        newType.m_extends.append(extendIndex, 0);

        newType.m_virtualTable = lockGetVirtualTable(extendIndex);
        newType.m_fields = m_types[extendIndex].m_fields;

        typedefSize = lockGetTypeSize(extendIndex);
    }

    //////////////////////////////////////////////////////////////////////////
    // Get all interfaces and resolve thier methods
    uint interfacesTableSize =
            apartment->getTables().getNumberOfRows(TABLE_INTERFACEIMPL_TABLE);
    for (uint i = 0; i < interfacesTableSize; i++)
    {
        const InterfaceImplTable::Header& iitable = ((const InterfaceImplTable&)(*
            apartment->getTables().getTableByToken(
                EncodingUtils::buildToken(TABLE_INTERFACEIMPL_TABLE,
                                          i + 1)))).getHeader();

        // Only interested in interfaces that this class inherits
        if (iitable.m_class != getTokenID(typedefToken))
            continue;

        // Read parent class and convert to typedef
        TokenIndex iiToken = m_apartment->getObjects().getTypesNameRepository().
                            translateTyperefToTypedef(buildTokenIndex(apartmentId, iitable.m_interface));

        // Check that we hasn't define the interface yet
        if (newType.m_extends.hasKey(iiToken))
            continue;

        lockAppendTypedef(iiToken);

        // The offset for new interface(s)
        uint interfaceOffset = newType.m_virtualTable.length();

        // Add all other interfaces. TODO! Flat model.
        // Update parent hash
        crc64.updateStream(m_types[iiToken].m_hashSignature);
        appendParents(newType.m_extends, m_types[iiToken].m_extends, interfaceOffset);

        // Mark myself
        newType.m_extends.append(iiToken, interfaceOffset);

        /////////////////////////////

        // Add interface virtual table
        const VirtualTable& interfaceVtbl = lockGetVirtualTable(iiToken);
        mergeInterfaceVirtualTables(newType.m_virtualTable, interfaceVtbl);

        /////////////////////////////

        // Fields: Mark starting of current fields offset
        const FieldsDictonary& iifields = m_types[iiToken].m_fields;
        cList<TokenIndex> fields;
        iifields.keys(fields);
        cList<TokenIndex>::iterator x = fields.begin();
        for (; x != fields.end(); ++x)
        {
            TokenIndex fieldToken = *x;
            FieldRepositoryContainer offsetType(iifields[fieldToken]);
            // Change the offset
            setFieldOffset(typedefSize, offsetType);
            // Append new field
            offsetType.m_type.hashElement(crc64, *this);
            newType.m_fields.append(fieldToken, offsetType);
            // Mark special cleanning
            if (offsetType.m_type.isObject())
            {
                newType.m_isSpecialCleanup |= m_types[offsetType.m_type.getClassToken()].m_isSpecialCleanup;
            }
        }
    }

    // Add current type to prevent recursive enterance.
    m_types.append(typedefToken, newType);


    //////////////////////////////////////////////////////////////////////////
    // We need to check if this is a Sequential or Explicit Layout
    // mdToken layoutParentToken = 0;
    uint totalLayoutSize = 0;

    if ((typedefTable.getHeader().m_flags & TypedefTable::tdExplicitLayout) ||
        (typedefTable.getHeader().m_flags & TypedefTable::tdSequentialLayout) )
    {
        for (uint i = 1; i <= apartment->getTables().getNumberOfRows(TABLE_CLASSLAYOUT_TABLE); i++)
        {
            const TablePtr& ptr = apartment->getTables().getTableByToken(EncodingUtils::buildToken(TABLE_CLASSLAYOUT_TABLE, i));
            const ClassLayoutTable& classLayout((const ClassLayoutTable&)(*ptr));
            ASSERT(EncodingUtils::getTokenTableIndex(getTokenID(typedefToken))==TABLE_TYPEDEF_TABLE);
            if(classLayout.getHeader().m_parent == EncodingUtils::getTokenPosition(getTokenID(typedefToken)))
            {
                totalLayoutSize = classLayout.getHeader().m_classSize;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Scan all fields for static members
    bool classDetorNeeded = false;
    mdToken startField = typedefTable.getHeader().m_fields;
    mdToken endField = typedefTable.calculateEndFieldToken(
                                                apartment->getTables());

    for (mdToken i = startField; i < endField; i++)
    {
        // Read the field signature
        const TablePtr& ptr = apartment->getTables().getTableByToken(i);
        // if this is a fixed size buffer we already added it's size from the class layout table
        // if (apartment->getTables().getTypedefParent(i) == layoutParentToken)
        //        continue;
        // CLR file-format errors. Scaned already by the GlobalContext
        // This is nearly an assertion.
        ASSERT(ptr->getIndex() == TABLE_FIELD_TABLE);
        // Prepare offset
        const FieldTable& fieldTable((const FieldTable&)(*ptr));
        blobStream->seek(fieldTable.getHeader().m_signature, basicInput::IO_SEEK_SET);
        // Translate ElementType and remove generic variable (if any)
        FieldSig fieldType(*blobStream, apartmentId, *this);
        FieldRepositoryContainer offsetType;
        offsetType.m_offset = 0;
        offsetType.m_type = getGenericRealElementType(fieldType.getType(), _typedefToken);
        offsetType.m_type.hashElement(crc64, *this);
        if ((fieldTable.getHeader().m_flags & FieldTable::fdStatic) == 0)
        {
            // The field is not static and must be append as an offset and size
            // Change the offset
            setFieldOffset(typedefSize, offsetType);
            // Append new field
            newType.m_fields.append(buildTokenIndex(apartmentId, i), offsetType);
            // Mark special cleanning
            if (offsetType.m_type.isObject())
            {
                if ((offsetType.m_type.isObjectAndNotValueType()) ||
                    (m_types[offsetType.m_type.getClassToken()].m_isSpecialCleanup))
                {
                    classDetorNeeded = newType.m_isSpecialCleanup = true;
                }
            }
        } else
        {
            // Write that field-id 'i' is actually a static member
            offsetType.m_offset = OFFSET_FOR_STATICS;
            newType.m_fields.append(buildTokenIndex(apartmentId, i), offsetType);
            // Add the length of the variable for the global directory
            uint size = innerGetTypeSize(offsetType.m_type);
            m_staticDB.append(buildTokenIndex(apartmentId, i), size);
            m_staticDBLength += size;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Scan all methods and append them into the virtual table
    mdToken startMethod = typedefTable.getHeader().m_methods;
    mdToken endMethod =
        typedefTable.calculateEndMethodToken(apartment->getTables());
    uint endOldVTblItr = newType.m_virtualTable.length(); // Don't scan newly inserted elements

    // For each method check whether it should be add into the virtual table
    for (mdToken currentMethod = startMethod;
        currentMethod != endMethod;
        currentMethod++)
    {
        // Read method name
        const MethodTable::Header& thisMethod = ((const MethodTable&)*
            apartment->getTables().getTableByToken(currentMethod)).getHeader();
        cString methodName = StringReader::readStringName(*stringStream,
                                                            thisMethod.m_name);
        blobStream->seek(thisMethod.m_signature, basicInput::IO_SEEK_SET);

        XSTL_TRY
        {
            MethodDefOrRefSignature methodSignature(*blobStream,
                                                    apartmentId,
                                                    *this);

            // Commented the ifdef by guyg in order for RELEASE version to pass tests.
            // Test for special initializer
            if (((thisMethod.m_flags & MethodTable::mdStatic) != 0) &&
                ((thisMethod.m_flags & MethodTable::mdSpecialName) != 0))
            {
                if (methodName.compare(CorlibNames::m_methodCctor) == cString::EqualTo)
                {
                    newType.m_staticInitializerMethod = buildTokenIndex(apartmentId, currentMethod);
                }
            }

            TokenIndex newvtblmid = buildTokenIndex(apartmentId, currentMethod);
            if ((thisMethod.m_flags & MethodTable::mdVirtual) != 0)
            {
                // Scan for previous method
                VirtualTable::iterator i = newType.m_virtualTable.begin();
                uint indexVtbl = 0;

                cList<cString> mNames = methodName.split(".");
                const cString& methodRealName = *(--mNames.end());

                // Hash only virtual functions. Order is important
                // methodSignature.hashSignature(crc64, *this);
                crc64.update(&currentMethod, sizeof(currentMethod));

                for (; (indexVtbl < endOldVTblItr) && (i != newType.m_virtualTable.end()); ++i, ++indexVtbl)
                {
                    // Compare functions names and signature
                    TokenIndex& orgIndex = getVtblMethodIndexOriginal(*i);

                    // Read other signature and name
                    ApartmentPtr orgApt = m_apartment->getApt(orgIndex);
                    const MethodTable::Header& otherMethod = ((const MethodTable&)*
                        orgApt->getTables().getTableByToken(getTokenID(orgIndex))).getHeader();
                    // Read name and signature of org method
                    cString orgName = StringReader::readStringName(*orgApt->getStreams().getStringsStream()->fork(),
                                                                    otherMethod.m_name);
                    // Assert that the first method in the vtbl is indeed the destructor
                    if ((!newType.m_isInterface) &&
                        (indexVtbl == 0) && (orgName.compare(CorlibNames::m_methodFinalizer) != cString::EqualTo))
                    {
                        RunnableTrace("Error! Finialize (detor) function is not located at first index! Instead function is " <<
                                        namespaceName << "." << className << "." << orgName << endl);
                        CHECK_FAIL(); // Throw error message
                    }
                    cForkStreamPtr orgBlobStream(orgApt->getStreams().getBlobStream()->fork());
                    orgBlobStream->seek(otherMethod.m_signature, basicInput::IO_SEEK_SET);
                    MethodDefOrRefSignature orgSignature(*orgBlobStream,
                                                            getApartmentID(orgIndex),
                                                            *this);

                    // Check if this method is compatible with the original
                    if ((methodRealName == orgName) && (orgSignature == methodSignature))
                    {
                        // Apply the override! This method will be placed in the vtbl instead of the original.
                        getVtblMethodIndexOverride(*i) = newvtblmid;
                        continue;
                    }
                }
                // Also add a new entry to the table
                // Todo: Support "newslot" here?
                newType.m_virtualTable.append(cDualElement<TokenIndex, TokenIndex>(newvtblmid,newvtblmid));
            }
        }
        XSTL_CATCH_ALL
        {
            RunnableTrace("Error while reading signature for method: " << namespaceName << "." << className << "." << methodName << endl);
            XSTL_RETHROW;
        }
    }

    // Override previous overrides using MethodImpl entries
    // See if there are MethodImpl entries for this class and apply them
    uint methodImplCount = apartment->getTables().getNumberOfRows(TABLE_METHODIMPL_TABLE);
    for (uint mi = 0; mi < methodImplCount; mi++)
    {
        MethodImplTable& methodImpl = (MethodImplTable&)(*apartment->getTables().getTableByToken(EncodingUtils::buildToken(TABLE_METHODIMPL_TABLE, mi+1)));
        if (methodImpl.getHeader().m_class == getTokenID(typedefToken))
        {
            // Found a MethodImpl entry for this class

            // Translate MemberRef of other apartments, if needed, into proper MethodDef TokenIndex
            ApartmentPtr aptPtr(apartment);
            TokenIndex baseMethod = ClrResolver::resolve(aptPtr, buildTokenIndex(apartmentId, methodImpl.getHeader().m_methodDeclaration));
            TokenIndex overrideMethod = ClrResolver::resolve(aptPtr, buildTokenIndex(apartmentId, methodImpl.getHeader().m_methodBody));

            // Verify that we're not left with a method ref
            CHECK(EncodingUtils::getTokenTableIndex(getTokenID(baseMethod)) == TABLE_METHOD_TABLE);
            CHECK(EncodingUtils::getTokenTableIndex(getTokenID(overrideMethod)) == TABLE_METHOD_TABLE);

            // Make sure override method is one of our methods
            CHECK(getTokenID(overrideMethod) >= startMethod);
            CHECK(getTokenID(overrideMethod) < endMethod);

            // Read method name
            const MethodTable::Header& thisMethod = ((const MethodTable&)*
                apartment->getTables().getTableByToken(getTokenID(overrideMethod))).getHeader();
            cString methodName = StringReader::readStringName(*stringStream, thisMethod.m_name);

            // Replace this virtual table entry
            VirtualTable::iterator i = newType.m_virtualTable.begin();
            for (; i != newType.m_virtualTable.end(); i++)
            {
                if (getVtblMethodIndexOriginal(*i) == baseMethod)
                {
                    getVtblMethodIndexOverride(*i) = overrideMethod;
                    break;
                }
            }

            // Make sure this entry was applied correctly!
            CHECK_MSG(i != newType.m_virtualTable.end(), cString("MethodImpl entry for ") + namespaceName + "." + className + "." + methodName + "() could not be applied to the virtual table");
        }
    }

    // Check if we need to add special destructor
    if (classDetorNeeded)
    {
        cDualElement<TokenIndex, TokenIndex>& detorFunc = *newType.m_virtualTable.begin();
        mdToken oFunc = getTokenID(getVtblMethodIndexOverride(detorFunc));
        if (!((startMethod <= oFunc) && (oFunc <= endMethod)))
        {
            // Overwrite into special detor function
            getVtblMethodIndexOverride(detorFunc) = buildTokenIndex(getApartmentID(typedefToken),
                                            EncodingUtils::buildToken(TABLE_CLR_METHOD_INSTANCE_DETOR,
                                                EncodingUtils::getTokenPosition(getTokenID(typedefToken))));
        }
    }

    // ClassLayout Padding
    if (totalLayoutSize)
        typedefSize = totalLayoutSize;

    newType.m_typedefSize = typedefSize;
    newType.m_isCompleted = true;
    crc64.update(&typedefSize, sizeof(typedefSize));
    // And calculate hash
    newType.m_hashSignature = crc64.digest();

    // And append!
    m_types[typedefToken] = newType;
}

uint TypedefRepository::innerGetTypeSize(const ElementType& type) const
{
    uint size = ElementType::getTypeSize(type.getType());
    if ((type.isPointer() || type.isReference()) ||
        (type.isSingleDimensionArray()))
    {
        return m_memoryLayout.pointerWidth();
    } else if (size == 0)
    {
        switch (type.getType())
        {
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_GENERICINST:
                return m_memoryLayout.pointerWidth();
            case ELEMENT_TYPE_VOID:
                return 0;
            case ELEMENT_TYPE_VALUETYPE:
                return lockGetTypeSize(type.getClassToken());
            default:
                CHECK_FAIL();
        }
    }

    return size;
}

ElementType TypedefRepository::getGenericRealElementType(const ElementType& elementType,
                                                         const TokenIndex& genericValue) const
{
    // TODO! I'm sure there are many cases that I have missed!
    if (elementType.getType() == ELEMENT_TYPE_VAR)
    {
        /*
        if (EncodingUtils::getTokenPosition(elementType.getClassToken()) == 0)
        {
            // TODO! Bad implementation. Need to check array, pointer, class, and dual instances
            return m_genericInstances[genericValue].getGenericTypes()[0];
        }
        */
    }
    return elementType;
}

void TypedefRepository::setFieldOffset(uint& typedefSize,
                                       FieldRepositoryContainer& offsetType) const
{
    offsetType.m_offset = typedefSize;
    typedefSize+= m_memoryLayout.align(innerGetTypeSize(offsetType.m_type));
}

TokenIndex TypedefRepository::resolveParentToken(const TokenIndex& fieldToken,
                                                 const TokenIndex& parentToken) const
{
    TokenIndex pt = parentToken;
    if (pt == ElementType::UnresolvedTokenIndex)
    {
        pt = buildTokenIndex(getApartmentID(fieldToken),
                                m_apartment->getApt(fieldToken)->getTables().
                                getTypedefParent(getTokenID(fieldToken)));
        m_apartment->getObjects().getTypesNameRepository().
                                    translateTyperefToTypedef(pt);
    }
    return pt;
}
