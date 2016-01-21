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
 * ArgumentsPositions.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/ArgumentsPositions.h"

ArgumentsPositions::ArgumentsPositions() :
    m_totalArgumentsSize(0)
{
}

uint ArgumentsPositions::getTotalArgumentsSize() const
{
    return m_totalArgumentsSize;
}

uint ArgumentsPositions::getArgumentPosition(uint index) const
{
    // Exception will be thrown if location is out of range
    return m_argumentPosition[index];
}

uint ArgumentsPositions::getArgumentStackSize(uint index) const
{
    // Exception will be thrown if location is out of range
    return m_argumentSizes[index];
}

const ElementType& ArgumentsPositions::getArgumentStackVariableType(
                                                               uint index) const
{
    // Exception will be thrown if location is out of range
    return m_argumentTypes[index];
}

uint ArgumentsPositions::countObjects() const
{
    uint count = 0;
    for (uint i = 0; i < m_argumentTypes.getSize(); i++)
    {
        if (m_argumentTypes[i].isObject())
            count++;
    }
    return count;
}

uint ArgumentsPositions::getCount() const
{
    ASSERT(m_argumentPosition.getSize() == m_argumentSizes.getSize());
    ASSERT(m_argumentTypes.getSize() == m_argumentSizes.getSize());
    return m_argumentTypes.getSize();
}
