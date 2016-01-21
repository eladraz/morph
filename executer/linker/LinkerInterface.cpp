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
* LinkerInterface.cpp
*
* Implementation file
*/
#include "executer/stdafx.h"
#include "executer/linker/LinkerInterface.h"

LinkerInterface::~LinkerInterface()
{
}

LinkerInterface::LinkerInterface(CompilerEngineThread& compilerEngineThread,
                                 ApartmentPtr apartment) :
    m_engine(compilerEngineThread),
    m_externalModuleResolver(apartment),
    m_apartment(apartment),
    m_vtblFilledSize(0),
    m_totalStaticFields(0)
{
}

addressNumericValue LinkerInterface::getVtblAddress(const TokenIndex& tokenIndex)
{
    if (m_vtbls.hasKey(tokenIndex))
        return m_vtbls[tokenIndex];
    return 0;
}

void LinkerInterface::setVtblAddress(const TokenIndex& tokenIndex, addressNumericValue address)
{
    m_vtbls.append(tokenIndex, address);
}

addressNumericValue LinkerInterface::getStaticAddress(const TokenIndex& fieldIndex)
{
    if (m_staticFieldsTable.hasKey(fieldIndex))
        return m_staticFieldsTable[fieldIndex];

    addressNumericValue ret = m_totalStaticFields;
    ResolverInterface& resolver = m_apartment->getObjects().getTypedefRepository();
    uint size = resolver.getTypeSize(resolver.getFieldType(fieldIndex, ElementType::UnresolvedTokenIndex));
    m_totalStaticFields+= size;
    m_staticFieldsTable.append(fieldIndex, ret);
    return ret;
}

uint LinkerInterface::getTotalAllocatedStaticBuffer() const
{
    return m_totalStaticFields;
}
