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
 * LocalPositions.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/LocalPositions.h"

LocalPositions::LocalPositions()
{
}

uint LocalPositions::getLocalPosition(uint index) const
{
    // Exception will be thrown if location is out of range
    return m_localPosition[index];
}

const ElementType& LocalPositions::getLocalStackVariableType(uint index) const
{
    // Exception will be thrown if location is out of range
    return m_localTypes[index];
}

class CountObjects : public ResolverInterface::FieldEnumeratorCallbacker
{
public:
    CountObjects() :
       m_count(0)
    {
    }

    virtual void fieldCallback(void*, const ResolverInterface&, const TokenIndex& fieldContext, const ElementType& fieldElementType)
    {
        if (fieldElementType.isObjectAndNotValueType())
            m_count++;
    }

    uint getCount()
    {
        return m_count;
    }

    void incCount()
    {
        m_count++;
    }

private:
    uint m_count;
};

uint LocalPositions::countObjects(ResolverInterface& resolver, bool shouldCountFields) const
{
    CountObjects o;
    for (uint uLocal = 0; uLocal < m_localPosition.getSize(); uLocal++)
    {
        if (getLocalStackVariableType(uLocal).isObjectAndNotValueType())
        {
            o.incCount();
            // NOTE: The else here is a special
        } else if (shouldCountFields && getLocalStackVariableType(uLocal).isObject())
        {
            resolver.scanAllFields(getLocalStackVariableType(uLocal).getClassToken(), o, NULL);
        }
    }
    return o.getCount();
}

uint LocalPositions::firstObjectIndex() const
{
    uint ret = 0;
    for (uint uLocal = 0; uLocal < m_localPosition.getSize(); uLocal++)
    {
        if (getLocalStackVariableType(uLocal).isObjectAndNotValueType())
            return uLocal;
    }
    // Bug! please don't call this method if countObjects() returns 0
    CHECK(false);
    return MAX_INT;
}

uint LocalPositions::getSize() const
{
    ASSERT(m_localPosition.getSize() == m_localTypes.getSize());
    return m_localPosition.getSize();
}
