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

#include "../../tests.h"

#include "xStl/types.h"
#include "xStl/stream/fileStream.h"
#include "xStl/stream/memoryStream.h"
#include "runnable/CustomAttribute.h"

//#include "pe/ntDirCli.h"
//#include "runnable/ApartmentFactory.h"
//#include "runnable/FrameworkMethods.h"

class CustomAttributeValuesTests : public cTestObject
{
public:
    virtual void test();
    virtual cString getName() { return __FILE__; }

private:
    void create_custom_attrib_values_bad_marker(void);
    void create_custom_attrib_values_good_marker(void);
    void create_custom_attrib_values_read_inbound(void);
    void create_custom_attrib_values_read_out_of_bounds(void);
    void create_custom_attrib_values_read_end_of_stream(void);
    //void create_custom_attrib_values_specfic_name(void);
    //void create_custom_attrib_values_not_specfic_name(void);
};

// Instance test object
CustomAttributeValuesTests g_global;

void CustomAttributeValuesTests::create_custom_attrib_values_bad_marker(void)
{
    // Marker, string size, string "cdecl", unsigned int value 5
    uint8 data[] = {0x01, 0x05, 0x07, 0x73, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C, 0x05, 0x00, 0x00, 0x00};
    cBuffer buffer = cBuffer(data, sizeof(data));
    cMemoryStream stream = cMemoryStream(buffer);
    try {
        CustomAttributeValues caValues = CustomAttributeValues(stream.fork(), sizeof(data));
    }
    catch (CustomAttributeValues::Invalid& e)
    {
        e;
        return;
    }

    ASSERT(0);
}


void CustomAttributeValuesTests::create_custom_attrib_values_good_marker(void)
{
    // Marker, string size, string "cdecl", unsigned int value 5
    uint8 data[] = {0x01, 0x00, 0x07, 0x73, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C, 0x05, 0x00, 0x00, 0x00};
    cBuffer buffer = cBuffer(data, sizeof(data));
    cMemoryStream stream = cMemoryStream(buffer);
    CustomAttributeValues caValues = CustomAttributeValues(stream.fork(), sizeof(data));
}

void CustomAttributeValuesTests::create_custom_attrib_values_read_inbound(void)
{
    // Marker, string size, string "cdecl", unsigned int value 5
    uint8 data[] = {0x01, 0x00, 0x07, 0x73, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C, 0x05, 0x00, 0x00, 0x00};
    cBuffer buffer = cBuffer(data, sizeof(data));
    cMemoryStream stream = cMemoryStream(buffer);
    CustomAttributeValues caValues = CustomAttributeValues(stream.fork(), sizeof(data));

    uint8 __strBufferExpected[] = {0x07, 0x073, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C};
    cBuffer strBufferExpected = cBuffer(__strBufferExpected, sizeof(__strBufferExpected));
    cBuffer strBuffer = cBuffer(sizeof(__strBufferExpected));

    caValues.getNextValue(strBuffer.getBuffer(), sizeof(__strBufferExpected));
    ASSERT(strBuffer == strBufferExpected);

    uint32 valueExpected = 5;
    uint32 value;
    caValues.getNextValue(&value, sizeof(value));
    ASSERT(value == valueExpected);
}

void CustomAttributeValuesTests::create_custom_attrib_values_read_out_of_bounds(void)
{
    // Marker, string size, string "cdecl", unsigned int value 5
    uint8 data[] = {0x01, 0x00, 0x07, 0x73, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C, 0x05, 0x00, 0x00, 0x00};
    cBuffer buffer = cBuffer(data, sizeof(data));
    cMemoryStream stream = cMemoryStream(buffer);
    CustomAttributeValues caValues = CustomAttributeValues(stream.fork(), sizeof(data));

    uint8 __strBufferExpected[] = {0x07, 0x073, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C};
    cBuffer strBufferExpected = cBuffer(__strBufferExpected, sizeof(__strBufferExpected));
    cBuffer strBuffer = cBuffer(sizeof(__strBufferExpected));

    caValues.getNextValue(strBuffer.getBuffer(), sizeof(__strBufferExpected));
    ASSERT(strBuffer == strBufferExpected);

    try {
        caValues.getNextValue(strBuffer.getBuffer(), sizeof(__strBufferExpected));
    }
    catch (CustomAttributeValues::OutOfBounds) {
        return;
    }

    ASSERT(0);
}

void CustomAttributeValuesTests::create_custom_attrib_values_read_end_of_stream(void)
{
    // Marker, string size, string "cdecl", unsigned int value 5
    uint8 data[] = {0x01, 0x00, 0x07, 0x73, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C, 0x05, 0x00, 0x00, 0x00};
    uint8 eos_data[] = {0x01, 0x00, 0x07, 0x73, 0x74, 0x64};
    cBuffer buffer = cBuffer(eos_data, sizeof(eos_data));
    cMemoryStream stream = cMemoryStream(buffer);
    CustomAttributeValues caValues = CustomAttributeValues(stream.fork(), sizeof(data));

    uint8 __strBufferExpected[] = {0x07, 0x073, 0x74, 0x64, 0x63, 0x61, 0x6C, 0x6C};
    cBuffer strBufferExpected = cBuffer(__strBufferExpected, sizeof(__strBufferExpected));
    cBuffer strBuffer = cBuffer(sizeof(__strBufferExpected));
    try {
        caValues.getNextValue(strBuffer.getBuffer(), sizeof(__strBufferExpected));
    }
    catch (CustomAttributeValues::EndOfStream) {
        return;
    }

    ASSERT(0);
}

//void CustomAttributeValuesTests::create_custom_attrib_values_specfic_name(void)
//{
//    CustomAttributes customAttributes;
//    ApartmentPtr apartmentPtr = getApartment();
//    Apartment& apartment = *apartmentPtr;
//    mdToken methodToken = (mdToken) 0x06000004;
//
//    CustomAttribute::getAttributes(apartment, methodToken, cString("CallingConvention"), customAttributes);
//    CustomAttributes::iterator iterator = customAttributes.begin();
//    CustomAttributes::iterator end_iterator = customAttributes.end();
//
//    for (;iterator != end_iterator; iterator++) {
//        traceHigh(*iterator << endl);
//    }
//
//}
//
//void CustomAttributeValuesTests::create_custom_attrib_values_not_specfic_name(void)
//{
//    CustomAttributes customAttributes;
//    ApartmentPtr apartmentPtr = getApartment();
//    Apartment& apartment = *apartmentPtr;
//    mdToken methodToken = (mdToken) 0x06000004;
//
//    CustomAttribute::getAttributes(apartment, methodToken, customAttributes);
//    CustomAttributes::iterator iterator = customAttributes.begin();
//    CustomAttributes::iterator end_iterator = customAttributes.end();
//
//    for (;iterator != end_iterator; iterator++) {
//        traceHigh(*iterator << endl);
//    }
//
//}
//
void CustomAttributeValuesTests::test(void)
{
    create_custom_attrib_values_bad_marker();
    create_custom_attrib_values_good_marker();
    create_custom_attrib_values_read_inbound();
    create_custom_attrib_values_read_out_of_bounds();
    create_custom_attrib_values_read_end_of_stream();
    //create_custom_attrib_values_specfic_name();
    //create_custom_attrib_values_not_specfic_name();
}