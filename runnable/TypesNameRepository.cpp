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
 * TypesNameRepository.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/except/assert.h"
#include "xStl/stream/traceStream.h"
#include "data/exceptions.h"
#include "data/ElementType.h"
#include "format/EncodingUtils.h"
#include "format/MetadataTables.h"
#include "format/MSILStreams.h"
#include "format/tables/Table.h"
#include "format/tables/TablesID.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/ModuleTable.h"
//#include "format/tables/ModuleRefTable.h"
#include "format/tables/AssemblyRefTable.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/CorlibNames.h"
#include "runnable/TypesNameRepository.h"
#include "runnable/StringReader.h"
#include "runnable/GlobalContext.h"
#include "runnable/ClrResolver.h"
#include "runnable/RunnableTrace.h"

const char TypesNameRepository::gUniqueClassNamespaceConnectingString[] =
    "**::**";

TypesNameRepository::TypeNameStruct::TypeNameStruct(const cString& namespaceString,
                                                    const cString& classnameString,
                                                    const TokenIndex& destructor) :
    m_namespaceString(namespaceString),
    m_classnameString(classnameString),
    m_destructor(destructor)
{
}

TypesNameRepository::TypesNameRepository(const ApartmentPtr& apartment) :
    m_mainApartment(apartment)
{
    //m_tables(apartment->getTables()),
    //m_stringsStream(apartment->getStreams().getStringsStream()->fork()),
    //m_blobStream(apartment->getStreams().getBlobStream()->fork())

    addApartment(apartment);
}

void TypesNameRepository::addApartment(const ApartmentPtr& apartment)
{
    uint typedefTablesSize = apartment->getTables().getNumberOfRows(TABLE_TYPEDEF_TABLE);
    for (uint i = 0; i < typedefTablesSize; i++)
    {
        // The typedef table
        mdToken tdmdToken = EncodingUtils::buildToken(TABLE_TYPEDEF_TABLE, i+1);
        TokenIndex tid = buildTokenIndex(apartment->getUniqueID(), tdmdToken);
        {
            cLock lock(m_lock);
            if (!m_types.hasKey(tid))
                appendTyperef(tid);
        }
    }
}

TokenIndex TypesNameRepository::translateTyperefToTypedef(const TokenIndex& typerefToken)
{
    // TODO! Add MemberRef as well!
    if (EncodingUtils::getTokenTableIndex(getTokenID(typerefToken)) != TABLE_TYPEREF_TABLE)
        return typerefToken;

    // Start translation
    ApartmentPtr trefApt = m_mainApartment->getApt(typerefToken);
    const TablePtr& typeTablePtr = trefApt->getTables().getTableByToken(getTokenID(typerefToken));
    const TyperefTable& typeref = (const TyperefTable&)(*typeTablePtr);
    //mdToken resolutionScope = typeref.getHeader().m_resolutionScope;
    cForkStreamPtr trefStringStream = trefApt->getStreams().getStringsStream()->fork();

    // Try to translate with already exists repository
    cString trefName = StringReader::readStringName(*trefStringStream, typeref.getHeader().m_name);
    cString trefNamespace = StringReader::readStringName(*trefStringStream, typeref.getHeader().m_namespace);

    ClrResolver::translateMethodCoreNames(trefNamespace, trefName);

    return getTypeToken(trefNamespace, trefName);
}

bool TypesNameRepository::isTypeEquals(const TokenIndex& typeToken,
                                       const cString& _namespace,
                                       const cString& _name)
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);

    if (!m_types.hasKey(typeToken))
        appendTyperef(typeToken);

    const TypeNameStruct& ns = m_types[typeToken];
    return (ns.m_classnameString == _name) &&
           (ns.m_namespaceString == _namespace);
}

const cString& TypesNameRepository::getTypeName(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);

    if (!m_types.hasKey(typeToken))
        appendTyperef(typeToken);

    return m_types[typeToken].m_classnameString;
}

const cString& TypesNameRepository::getTypeNamespace(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);

    if (!m_types.hasKey(typeToken))
        appendTyperef(typeToken);

    return m_types[typeToken].m_namespaceString;
}

TokenIndex TypesNameRepository::getTypeDestructor(const TokenIndex& typeToken)
{
    ElementType::assertTyperef(typeToken);
    cLock lock(m_lock);

    if (!m_types.hasKey(typeToken))
        appendTyperef(typeToken);

    return m_types[typeToken].m_destructor;
}

TokenIndex TypesNameRepository::getTypeToken(const cString& _namespace,
                                             const cString& _className)
{
    cLock lock(m_lock);
    TokenIndex token = lockGetTypeToken(_namespace, _className);
    // Check whether class is found
    if (token == ElementType::UnresolvedTokenIndex)
    {
        RunnableTrace("Warning!! Missing class " << _namespace << "." << _className << endl);
    }

    return token;
}

TokenIndex TypesNameRepository::getMethodToken(const cString& _namespace,
                                               const cString& _className,
                                               const cString& _methodName,
                                               const MethodDefOrRefSignature& methodSignature)
{
    cLock lock(m_lock);

    // Get the typeref token
    TokenIndex classToken = lockGetTypeToken(_namespace, _className);
    // Check whether class exists
    if (classToken == ElementType::UnresolvedTokenIndex)
    {
        return classToken;
    }

    const ResolverInterface::ParentDictonary& parents = m_mainApartment->getObjects().getTypedefRepository().getParentDirectory(classToken);
    cList<TokenIndex> parentsId;
    parents.keys(parentsId);
    parentsId.insert(classToken);

    for (cList<TokenIndex>::iterator token = parentsId.begin(); token != parentsId.end(); ++token)
    {
        ApartmentPtr apartment = m_mainApartment->getApt(*token);
        const MetadataTables& tables = apartment->getTables();
        cForkStreamPtr stringStream = apartment->getStreams().getStringsStream()->fork();
        cForkStreamPtr blobStream = apartment->getStreams().getBlobStream()->fork();

        // Add optimization here
        // For now I'll scan all method until I'll find a match
        const TypedefTable& typedefHeader =
            (const TypedefTable&)(*tables.getTableByToken(getTokenID(*token)));

        mdToken startMethod = typedefHeader.getHeader().m_methods;
        mdToken endMethod = typedefHeader.calculateEndMethodToken(tables);
        for (mdToken i = startMethod; i < endMethod; i++)
        {
            const MethodTable& methodTable = (const MethodTable&)
                (*tables.getTableByToken(i));
            // Read method name
            cString methodName = StringReader::readStringName(*stringStream,
                                                    methodTable.getHeader().m_name);
            if (methodName == _methodName)
            {
                // Read signature
                blobStream->seek(methodTable.getHeader().m_signature,
                                 basicInput::IO_SEEK_SET);
                MethodDefOrRefSignature sig(*blobStream, getApartmentID(*token), apartment->getObjects().getTypedefRepository());

                // TODO! The next comparison also validate calling-convention.
                //       please note
                if (sig == methodSignature)
                {
                    // We have a match
                    return buildTokenIndex(getApartmentID(*token), i);
                }
            }
        }
    }

    // Couldn't find method
    return ElementType::UnresolvedTokenIndex;
}

void TypesNameRepository::appendTyperef(const TokenIndex& typeToken) const
{
    ElementType::assertTyperef(typeToken);

    ApartmentPtr apartment = m_mainApartment->getApt(typeToken);
    const Table& table = *apartment->getTables().getTableByToken(getTokenID(typeToken));
    cForkStreamPtr stringStream = apartment->getStreams().getStringsStream()->fork();

    mdToken namespaceIndex;
    mdToken nameIndex;
    CHECK(table.getIndex() != TABLE_TYPEREF_TABLE);

    const TypedefTable& thisClassHeader = (const TypedefTable&)table;
    namespaceIndex = thisClassHeader.getHeader().m_namespace;
    nameIndex = thisClassHeader.getHeader().m_name;

    // Start scanning methods
    mdToken startMethod = thisClassHeader.getHeader().m_methods;
    mdToken endMethod = thisClassHeader.calculateEndMethodToken(apartment->getTables());
    mdToken finializer = 0;

    for (mdToken i = startMethod; i < endMethod; i++)
    {
        const MethodTable& methodTable = (const MethodTable&)
                                            (*apartment->getTables().getTableByToken(i));
        if ((methodTable.getHeader().m_flags & MethodTable::mdVirtual) != 0)
            if (StringReader::readStringName(*stringStream,
                                                methodTable.getHeader().m_name)
                == CorlibNames::m_methodFinalizer)
            {
                // Test that there aren't any finializer
                ASSERT(finializer == 0);
                finializer = i;

                #ifndef _DEBUG
                    // Release mode will ended the search here.
                    break;
                #endif
            }
    }

    // Read the names
    cString namespaceString =
        StringReader::readStringName(*stringStream, namespaceIndex);
    cString classNameString =
        StringReader::readStringName(*stringStream, nameIndex);

    // RunnableTrace("Adding typeref: " << namespaceString << "." << classNameString << endl);
    cString ucname = getUniqueClassNamespace(namespaceString, classNameString);
    if (!m_typesDictionary.hasKey(ucname))
    {
        // Usually module typedef tokens are duplicated
        m_typesDictionary.append(ucname,
                                 typeToken);
    }

    m_types.append(typeToken, TypeNameStruct(namespaceString,
                                             classNameString,
                                             (finializer == 0) ? ElementType::UnresolvedTokenIndex :
                                                                 buildTokenIndex(getApartmentID(typeToken), finializer)));

}

TokenIndex TypesNameRepository::lockGetTypeToken(const cString& _namespace,
                                                 const cString& _className)
{
    // Get token from namespace and class-name
    cString signature = getUniqueClassNamespace(_namespace, _className);
    TokenIndex token = ElementType::UnresolvedTokenIndex;
    if (m_typesDictionary.hasKey(signature))
    {
        token = m_typesDictionary[signature];
    }

    return token;
}

cString TypesNameRepository::getUniqueClassNamespace(const cString& _namespace,
                                                     const cString& _className)
{
    cString ret(_namespace);
    ret+= gUniqueClassNamespaceConnectingString;
    ret+= _className;
    return ret;
}

