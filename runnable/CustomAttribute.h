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

#ifndef __TBA_CLR_FORMAT_CUSTOM_ATTRIBUTE_H
#define __TBA_CLR_FORMAT_CUSTOM_ATTRIBUTE_H

#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/data/smartptr.h"
#include "xStl/stream/forkStream.h"

#include "data/exceptions.h"

#include "runnable/Apartment.h"
#include "runnable/CustomAttributeArgument.h"

#include "format/tables/MethodTable.h"
#include "format/tables/CustomAttributeTable.h"
#include "format/tables/TypedefTable.h"

class CustomAttribute;
class CustomAttributeValues;

typedef cSmartPtr<CustomAttribute> CustomAttributePtr;
typedef cSmartPtr<CustomAttributeValues>  CustomAttributeValuesPtr;
typedef cArray<CustomAttributePtr> CustomAttributes;

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out, const CustomAttribute& object);
cStringerStream& operator << (cStringerStream& out, const CustomAttributePtr customAttributePtr);
#endif

class CustomAttribute
{
public:
    CustomAttribute();
    CustomAttribute(const cString& name);

    virtual ~CustomAttribute();

    /*
     * Get name of custom attribute.
     */
    const cString& getName(void) const;

    /*
     * Returns argument by name.
     * if parameter is missing, a MissingArgument exception will be raised.
     */
    void getArguments(CustomAttributeArguments& outArguments) const;
    CustomAttributeArgument& getArgument(const cString& name) const;

    /*
     * Add argument to attribute.
     */
    CustomAttributeArgumentPtr addArgument(const cString& name, const cString& value);
    CustomAttributeArgumentPtr addArgument(const cString& name, const int value);
    CustomAttributeArgumentPtr addArgument(const cString& name, const unsigned int value);
    CustomAttributeArgumentPtr addArgument(const cString& name, const bool value);

    /*
     * Get custom attributes of method.
     * Returns a list of CustomAttributes.
     */
    static void getAttributes(const ApartmentPtr& apartment, mdToken methodToken, CustomAttributes& outAttributes);
    static void getAttributes(const ApartmentPtr& apartment, mdToken methodToken, const cString& name, CustomAttributes& attributes);

    /*
     * On a get argument request, MissingArgument is thrown
     * if argument criteria was not found.
     */
    class MissingArgument: public ClrRuntimeException
    {
    public:
        MissingArgument(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
    };

private:
    /*
     * CustomAttribute factory.
     */
    static CustomAttributePtr factory(mdToken owner,
                                      const Apartment& apartment,
                                      const Apartment& caApartment,
                                      const CustomAttributeTable& customAttributeTable,
                                      const MethodTable& ctorMethodTable,
                                      const TypedefTable& ctorMethodTypeTable);

    cString m_name;
    CustomAttributeArguments m_arguments;
};

class CustomAttributeValues
{
public:
    CustomAttributeValues(cForkStreamPtr streamPtr, mdToken length);
    void getNextValue(void * buffer, uint size);

    /*
     * On constructor, Invalid is thrown if CAV is invalid.
     */
    class Invalid: public ClrRuntimeException
    {
    public:
        Invalid(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};

    };

    /*
     * On getNextValue, throw EndOfStream if requesting more than a stream can offer.
     */
    class EndOfStream: public ClrRuntimeException
    {
    public:
        EndOfStream(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
    };

    /*
     * On getNextValue, throw OutOfBounds on attempt to read more than available.
     */
    class OutOfBounds: public ClrRuntimeException
    {
    public:
        OutOfBounds(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};

    };

private:
    const static uint16 m_prologSignature = 0x0001;
    cForkStreamPtr m_stream;
    uint32 m_lengthLeft;

    void testValidity(void) const;
};

#endif // __TBA_CLR_FORMAT_CUSTOM_ATTRIBUTE_H