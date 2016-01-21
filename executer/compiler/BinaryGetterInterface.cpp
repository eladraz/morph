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
 * BinaryGetterInterface.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "executer/compiler/BinaryGetterInterface.h"

BinaryGetterInterface::BinaryGetterInterface()
{
}

bool BinaryGetterInterface::isMethodExist(const TokenIndex& tokenIndex,
                                          SecondPassBinaryPtr* reference) const
{
    cLock lock(m_mutex);
    if (m_hash.hasKey(tokenIndex))
    {
        // Add references
        if (reference != NULL)
            *reference = m_hash[tokenIndex];
        return true;
    }
    return false;
}

const SecondPassBinaryPtr&
           BinaryGetterInterface::getSecondPassMethod(const TokenIndex& tokenIndex) const
{
    cLock lock(m_mutex);
    return m_hash[tokenIndex];
}

const MethodTransTable& BinaryGetterInterface::getMethodTransTable() const
{
    return m_hash;
}

void BinaryGetterInterface::addSecondPassMethod(const TokenIndex& tokenIndex,
                                                const SecondPassBinaryPtr& secondPass)
{
    cLock lock(m_mutex);
    m_hash.append(tokenIndex, secondPass);
}
