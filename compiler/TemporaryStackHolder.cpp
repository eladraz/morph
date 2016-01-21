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
 * TemporaryStackHolder.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "compiler/stdafx.h"
#include "compiler/CompilerInterface.h"
#include "compiler/TemporaryStackHolder.h"

TemporaryStackHolder::TemporaryStackHolder(
                                StackInterface& _interface,
                                CorElementType coreType,
                                uint size,
                                StackHolderAllocationRequest request) :
    m_stackLocation(StackInterface::NO_MEMORY),
    m_coreType(coreType),
    m_interface(_interface),
    m_type(AllocatedRegister)
{
    //CHECK(size <= _interface.getRegisterSize());

    if (request == TEMP_NONVOLATILE_REGISTER)
    {
        m_type = AllocatedRegister;
        m_stackLocation = _interface.allocateTemporaryRegister(true);
        // Must succeed. If not, we cannot compile
        CHECK(m_stackLocation != StackInterface::NO_MEMORY);
        return;
    }

    if ((request == TEMP_ONLY_REGISTER) ||
        (request == TEMP_PREFERRED_REGISTER))
    {
        m_type = AllocatedRegister;
        // Try to allocate the buffer
        m_stackLocation = _interface.allocateTemporaryRegister();
        if (m_stackLocation != StackInterface::NO_MEMORY)
            return;
    }

    // For only register allocation, this method should fail.
    CHECK(request != TEMP_ONLY_REGISTER);

    // Only the following options left.
    ASSERT((request == TEMP_ONLY_STACK) ||
           (request == TEMP_PREFERRED_REGISTER));

    // Try to allocate the stack buffer
    m_type = StackBuffer;
    m_stackLocation = _interface.allocateTemporaryStackBuffer(size);

    // Exception might be thrown in the constructor!! Be aware on
    CHECK(m_stackLocation != StackInterface::NO_MEMORY);
}

TemporaryStackHolder::TemporaryStackHolder(
                                StackInterface& _interface,
                                const StackLocation& stackLocation) :
    m_coreType(ELEMENT_TYPE_PTR),
    m_interface(_interface),
    m_type(DefinedRegister),
    m_stackLocation(stackLocation)
{
    // The stackLocation must be a register.
    // Cannot use this constructor for anything else
    CHECK(stackLocation.u.flags == 0);
}

TemporaryStackHolder::TemporaryStackHolder(StackLocation stackLocation,
                                           Type type,
                                           CorElementType coreType,
                                           StackInterface& _interface) :
    m_interface(_interface),
    m_stackLocation(stackLocation),
    m_type(type),
    m_coreType(coreType)
{
    //CompilerTrace("    REG " << HEXDWORD(m_stackLocation) <<
    //          " DUPLICATED INTO DIFFERENT BLOCK." << endl);
}

TemporaryStackHolder::~TemporaryStackHolder()
{
    if (m_stackLocation != StackInterface::NO_MEMORY)
    {
        switch (m_type)
        {
        case AllocatedRegister:
            // CompilerTrace("    REG " << HEXDWORD(m_stackLocation) <<
            //           " freed." << endl);
            m_interface.freeTemporaryRegister(m_stackLocation);
            break;
        case StackBuffer:
            m_interface.freeTemporaryStackBuffer(m_stackLocation);
            break;
        case DefinedRegister:
            // Do nothing
            break;
        default:
            ASSERT(false);
        }
    }
}

TemporaryStackHolderPtr TemporaryStackHolder::duplicateOnNewInterface(
                                                    StackInterface& _interface)
{
    return TemporaryStackHolderPtr(new TemporaryStackHolder(m_stackLocation,
                                                            m_type,
                                                            m_coreType,
                                                            _interface));
}

const StackLocation& TemporaryStackHolder::getTemporaryObject() const
{
    ASSERT(m_stackLocation != StackInterface::NO_MEMORY);
    return m_stackLocation;
}

StackLocation& TemporaryStackHolder::getTemporaryObject()
{
    ASSERT(m_stackLocation != StackInterface::NO_MEMORY);
    return m_stackLocation;
}

CorElementType TemporaryStackHolder::getCoreType() const
{
    return m_coreType;
}
