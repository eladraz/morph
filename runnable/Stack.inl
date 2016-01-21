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
 * Stack.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */	
#include "xStl/types.h"
#include "data/exceptions.h"
#include "runnable/Stack.h"

template <class T, class Itr>
void StackInfrastructor<T, Itr>::pop2null(uint amount)
{
    if (amount == 0)
        return;
    if (isEmpty())
        XSTL_THROW(ClrStackError);
    for (uint i = 0; i < amount; i++)
    {
        Itr b(m_stack.begin());
        m_stack.remove(b);
    }
}

template <class T, class Itr>
void StackInfrastructor<T, Itr>::push(const T& var)
{
    m_stack.insert(var);
}

template <class T, class Itr>
const T& StackInfrastructor<T, Itr>::peek() const
{
    if (isEmpty())
        XSTL_THROW(ClrStackError);

    return *m_stack.begin();
}

template <class T, class Itr>
T& StackInfrastructor<T, Itr>::getArg(uint index)
{
	Itr j = m_stack.begin();
	
	while (index > 0)
	{
		if (j == m_stack.end())
			XSTL_THROW(ClrStackError);
		++j;
		--index;
	}

	return *j;
}

template <class T, class Itr>
T& StackInfrastructor<T, Itr>::tos()
{
    if (isEmpty())
        XSTL_THROW(ClrStackError);

    return *m_stack.begin();
}

template <class T, class Itr>
bool StackInfrastructor<T, Itr>::isEmpty() const
{
    return m_stack.begin() == m_stack.end();
}

template <class T, class Itr>
uint StackInfrastructor<T, Itr>::getStackCount() const
{
    return m_stack.length();
}

template <class T, class Itr>
cList<T>& StackInfrastructor<T, Itr>::getList()
{
    return m_stack;
}

template <class T, class Itr>
const Itr StackInfrastructor<T, Itr>::getTosPosition()
{
    return m_stack.begin();
}

template <class T, class Itr>
void StackInfrastructor<T, Itr>::revertStack(const Itr& pos)
{
    while (m_stack.begin() != pos)
    {
        if (isEmpty())
        {
            // Error with stack reverting
            CHECK_FAIL();
        }
        m_stack.remove(m_stack.begin());
    }
}

template <class T, class Itr>
void StackInfrastructor<T, Itr>::clear()
{
    m_stack.removeAll();
}
