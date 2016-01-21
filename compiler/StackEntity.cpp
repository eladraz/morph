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
 * StackEntity.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/StackEntity.h"

StackEntity::StackEntity()
{
}

StackEntity::StackEntity(EntityType type,
                         const ElementType& elementType,
                         bool isReturned) :
    m_type(type),
    m_elementType(elementType),
    m_isReturned(isReturned)
{
}

StackEntity::EntityType StackEntity::getType() const
{
    return m_type;
}

void StackEntity::setType(EntityType type)
{
    m_type = type;
}

const ElementType& StackEntity::getElementType() const
{
    return m_elementType;
}

void StackEntity::setElementType(const ElementType& newType)
{
    m_elementType = newType;
}

const StackEntity::ConstValue& StackEntity::getConst() const
{
    return m_const;
}

StackEntity::ConstValue& StackEntity::getConst()
{
    return m_const;
}

const TemporaryStackHolderPtr& StackEntity::getStackHolderObject() const
{
    return m_stackHolder;
}

TemporaryStackHolderPtr& StackEntity::getStackHolderObject()
{
    return m_stackHolder;
}

void StackEntity::setStackHolderObject(const TemporaryStackHolderPtr& newObject)
{
    m_stackHolder = newObject;
}

void StackEntity::AddressUpType()
{
    switch(m_type)
    {
    case ENTITY_REGISTER_ADDRESS:
        m_type = ENTITY_REGISTER;
        break;
    case ENTITY_LOCAL:
        m_type = ENTITY_LOCAL_ADDRESS;
        break;
    case ENTITY_ARGUMENT:
        m_type = ENTITY_ARGUMENT_ADDRESS;
        break;
    case ENTITY_LOCAL_TEMP_STACK:
        m_type = ENTITY_LOCAL_TEMP_STACK_ADDRESS;
        break;
    default:
        return;
    }
}

///////////////////////////////////////////////

StackEntity::ConstValue::ConstValue(int constValue)
{
    m_u.m_intValue = constValue;
}

int StackEntity::ConstValue::getConstValue() const
{
    return m_u.m_intValue;
}

int64 StackEntity::ConstValue::getConst64Value() const
{
    return m_u.m_longValue;
}

void StackEntity::ConstValue::setConst64Value(int64 constValue)
{
    m_u.m_longValue = constValue;
}


void StackEntity::ConstValue::setConstValue(int constValue)
{
    m_u.m_intValue = constValue;
}

int StackEntity::ConstValue::getLocalOrArgValue() const
{
    return m_u.m_localOrArgIndex;
}

void StackEntity::ConstValue::setLocalOrArgValue(int localOrArgValue)
{
    m_u.m_localOrArgIndex = localOrArgValue;
}

const TokenIndex& StackEntity::ConstValue::getTokenIndex() const
{
    return m_token;
}

void StackEntity::ConstValue::setTokenIndex(const TokenIndex& tokenIndex)
{
    m_token = tokenIndex;
}

bool StackEntity::isReturned() const
{
    return m_isReturned;
}

void StackEntity::setReturned(bool isReturned)
{
    m_isReturned = isReturned;
}
