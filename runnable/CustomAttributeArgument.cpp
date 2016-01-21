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

#include "runnable/CustomAttributeArgument.h"

CustomAttributeArgument::CustomAttributeArgument(cString variableName, ElementType type):
    m_name(variableName),
    m_type(type)
{
}

const cString& CustomAttributeArgument::getName(void) const
{
    return m_name;
}

ElementType CustomAttributeArgument::getType(void) const
{
    return m_type;
}


CustomAttributeStringArgument::CustomAttributeStringArgument(cString name, cString value):
    CustomAttributeArgument(name, ElementType(ELEMENT_TYPE_STRING)),
    m_value(value)
{
}

const cString& CustomAttributeStringArgument::getStringValue(void) const
{
    return m_value;
}

CustomAttributeSignedIntegerArgument::CustomAttributeSignedIntegerArgument(cString name, int value):
    CustomAttributeArgument(name, ElementType(ELEMENT_TYPE_I4)),
    m_value(value)
{
}

int CustomAttributeSignedIntegerArgument::getSignedIntegerValue(void) const
{
    return m_value;
}

CustomAttributeUnsignedIntegerArgument::CustomAttributeUnsignedIntegerArgument(cString name, unsigned int value):
    CustomAttributeArgument(name, ElementType(ELEMENT_TYPE_U4)),
    m_value(value)
{
}

unsigned int CustomAttributeUnsignedIntegerArgument::getUnsignedIntegerValue(void) const
{
    return m_value;
}

CustomAttributeBoolArgument::CustomAttributeBoolArgument(cString name, bool value):
    CustomAttributeArgument(name, ElementType(ELEMENT_TYPE_BOOLEAN)),
    m_value(value)
{
}

bool CustomAttributeBoolArgument::getBoolValue(void) const
{
    return m_value;
}

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out, const CustomAttributeArgument& object)
{
    out << "CustomAttributeArgument(" << object.getName() << ", ";

    switch(object.getType().getType())
    {
    case ELEMENT_TYPE_STRING:
        out << object.getStringValue();
        break;
    case ELEMENT_TYPE_I4:
        out << object.getSignedIntegerValue();
        break;
    case ELEMENT_TYPE_U4:
        out << object.getUnsignedIntegerValue();
        break;
    case ELEMENT_TYPE_BOOLEAN:
        out << object.getBoolValue();
        break;
    }

    out << ")";

    return out;
}

cStringerStream& operator << (cStringerStream& out, const CustomAttributeArgumentPtr object)
{
    out << *object;
    return out;
}

#endif