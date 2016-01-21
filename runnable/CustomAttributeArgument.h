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

#ifndef __TBA_CLR_RUNNABLE_CUSTOM_ATTRIBUTE_ARGUMENT_H
#define __TBA_CLR_RUNNABLE_CUSTOM_ATTRIBUTE_ARGUMENT_H

#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/data/smartptr.h"
#include "xStl/stream/stringerStream.h"

#include "data/ElementType.h"
#include "data/exceptions.h"

class CustomAttributeArgument
{
public:
    CustomAttributeArgument(cString name, ElementType type);
    const cString& getName(void) const;
    ElementType getType(void) const;

    class InvalidValue: public ClrRuntimeException
    {
    public:
        InvalidValue(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) { };
    };

    virtual const cString& getStringValue(void) const { XSTL_THROW(InvalidValue); };
    virtual int getSignedIntegerValue(void) const { XSTL_THROW(InvalidValue); };
    virtual unsigned int getUnsignedIntegerValue(void) const { XSTL_THROW(InvalidValue); };
    virtual bool getBoolValue(void) const { XSTL_THROW(InvalidValue); };

private:
    cString m_name;
    ElementType m_type;
};

class CustomAttributeStringArgument: public CustomAttributeArgument
{
public:
    CustomAttributeStringArgument(cString name, cString value);
    virtual const cString& getStringValue(void) const;

private:
    cString m_value;
};

class CustomAttributeSignedIntegerArgument: public CustomAttributeArgument
{
public:
    CustomAttributeSignedIntegerArgument(cString name, int value);
    virtual int getSignedIntegerValue(void) const;

private:
    int m_value;
};

class CustomAttributeUnsignedIntegerArgument: public CustomAttributeArgument
{
public:
    CustomAttributeUnsignedIntegerArgument(cString name, unsigned int value);
    virtual unsigned int getUnsignedIntegerValue(void) const;

private:
    unsigned int m_value;
};

class CustomAttributeBoolArgument: public CustomAttributeArgument
{
public:
    CustomAttributeBoolArgument(cString name, bool value);
    virtual bool getBoolValue(void) const;

private:
    bool m_value;
};

typedef cSmartPtr<CustomAttributeArgument> CustomAttributeArgumentPtr;
typedef cArray<CustomAttributeArgumentPtr> CustomAttributeArguments;

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out, const CustomAttributeArgument& object);
cStringerStream& operator << (cStringerStream& out, const CustomAttributeArgumentPtr customAttributePtr);
#endif

#endif // __TBA_CLR_RUNNABLE_CUSTOM_ATTRIBUTE_ARGUMENT_H