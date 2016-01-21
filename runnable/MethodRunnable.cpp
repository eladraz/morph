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
 * MethodRunnable.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/traceStream.h"
#include "data/exceptions.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/StandAloneSigTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/MethodTable.h"
#include "format/signatures/LocalVarSignature.h"
#include "runnable/MethodRunnable.h"
#include "runnable/RunnableTrace.h"
#include "compiler/CompilerEngine.h"

MethodRunnable::MethodRunnable(const ApartmentPtr& apartment) :
    m_apartment(apartment),
    m_emptyImpl(false),
    m_methodToken(ElementType::UnresolvedTokenIndex)
{
}

const TokenIndex& MethodRunnable::getMethodToken() const
{
    return m_methodToken;
}

bool MethodRunnable::isSystemObject() const
{
    return m_isSystemObject;
}

const cString& MethodRunnable::getFullMethodName() const
{
    return m_fullMethodName;
}

void MethodRunnable::loadMethod(mdToken methodToken)
{
    // Check that we aren't going to execute extern functions...
    CHECK(EncodingUtils::getTokenTableIndex(methodToken) == TABLE_METHOD_TABLE);
    m_methodToken = buildTokenIndex(m_apartment->getUniqueID(), methodToken);

    // Check system.obj
    CompilerEngine::resolveTypeToken(m_apartment->getTables().getTypedefParent(methodToken),
                                     m_apartment->getUniqueID(),
                                     m_apartment->getObjects().getTypedefRepository(),
                                     m_methodParentType);
    m_isSystemObject = m_methodParentType.getClassToken() == m_apartment->getObjects().getTypedefRepository().getSystemObject();


    // Start by reading method arguments and name
    MethodTable& methodTable = (MethodTable&)(*m_apartment->getTables().
                                                  getTableByToken(methodToken));

    cForkStreamPtr stringStream = m_apartment->getStreams().
                                        getStringsStream()->fork();
    cString methodName = StringReader::readStringName(*stringStream,
                                                methodTable.getHeader().m_name);
    // Get parent
    const TypedefTable::Header& typedefHeader = ((const TypedefTable&)(
        *m_apartment->getTables().getTableByToken(getTokenID(m_methodParentType.getClassToken())))).getHeader();
    cString _namespace = StringReader::readStringName(*stringStream,
                                                 typedefHeader.m_namespace);
    cString _name = StringReader::readStringName(*stringStream,
                                                 typedefHeader.m_name);

    m_fullMethodName = _namespace + "." + _name + "." + methodName;

    // (Re)Read the method signature
    cForkStreamPtr blob = m_apartment->getStreams().getBlobStream()->fork();
    blob->seek(methodTable.getHeader().m_signature, basicInput::IO_SEEK_SET);
    m_methodSignature = MethodDefOrRefSignaturePtr(new
                                MethodDefOrRefSignature(*blob,
                                                        m_apartment->getUniqueID(),
                                                        m_apartment->getObjects().getTypedefRepository()));

    // Check methods which are external and try to resolve/execute them
    if (methodTable.getHeader().m_rva == 0)
    {
        m_emptyImpl = true;
        return;
    }

    // Read the method header
    m_methodHeader = cSmartPtr<MethodHeader>(new MethodHeader(
                        m_apartment->getLayout()->
                               getVirtualStream(methodTable.getHeader().m_rva)));

    // Get the content of the function
    m_streamPointer = m_methodHeader->getFunction();
    CHECK(!m_streamPointer.isEmpty());

    // Read the local variables, if there any, and generate the stack frame
    // accordingly.
    XSTL_TRY
    {
        mdToken locals = m_methodHeader->getLocals();
        if (locals != MethodHeader::NO_LOCALS)
        {
            TablePtr localSignature = m_apartment->getTables().getTableByToken(locals);
            CHECK(!localSignature.isEmpty());
            CHECK(localSignature->getIndex() == TABLE_STANDALONGESIG_TABLE);
            StandAloneSigTable& sig = (StandAloneSigTable&)(*localSignature);
            // NOTE: Only this thread can access m_context!
            blob->seek(sig.getBlobIndex(), basicInput::IO_SEEK_SET);
            LocalVarSignature localsSignature(*blob, m_apartment->getUniqueID(),
                                              m_apartment->getObjects().getTypedefRepository());

            // Generate a list of locals.
            m_locals = localsSignature.getLocals();
        }
    }
    XSTL_CATCH_ALL
    {
        RunnableTrace("MethodRunnable::loadMethod() error while parsing locals for function: " << m_fullMethodName << endl);
        XSTL_RETHROW;
    }
}

bool MethodRunnable::isEmptyMethod() const
{
    return m_emptyImpl;
}

const ApartmentPtr& MethodRunnable::getApartment() const
{
    return m_apartment;
}

ApartmentPtr& MethodRunnable::getApartment()
{
    return m_apartment;
}

cForkStreamPtr& MethodRunnable::getStreamPointer()
{
    return m_streamPointer;
}

uint MethodRunnable::getMethodStreamStartAddress() const
{
    return m_methodHeader->getMethodStreamStartAddress();
}

MethodHeader& MethodRunnable::getMethodHeader()
{
    return *m_methodHeader;
}

const MethodDefOrRefSignature& MethodRunnable::getMethodSignature() const
{
    // Exception will be thrown if 'm_methodSignature' is null
    CHECK(!m_methodSignature.isEmpty())
    return *m_methodSignature;
}

ElementsArrayType& MethodRunnable::getLocals()
{
    return m_locals;
}

const ElementType& MethodRunnable::getMethodTypeParent() const
{
    return m_methodParentType;
}

