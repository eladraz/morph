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
#include "runnable/CustomAttribute.h"

class CustomAttributeTests : public cTestObject
{
public:
    virtual void test();
    virtual cString getName() { return __FILE__; }

private:
    void create_custom_attrib_with_args(void);
};

// Instance test object
CustomAttributeTests g_global;

void CustomAttributeTests::create_custom_attrib_with_args(void)
{
    cString caName = "CallingConvention";
    CustomAttribute cattrib = CustomAttribute(caName);
    ASSERT(caName == cattrib.getName());

    cString caStringName =  "my_string";
    cString caStringValue = "This is my string";
    CustomAttributeArgumentPtr caStringPtr = cattrib.addArgument(caStringName, caStringValue);
    ASSERT(caStringValue == caStringPtr->getStringValue());

    cString caSignedIntegerName =  "my_signed_int";
    int caSignedIntegerValue = -5;
    CustomAttributeArgumentPtr caSignedIntegerPtr = cattrib.addArgument(caSignedIntegerName, caSignedIntegerValue);
    ASSERT(caSignedIntegerValue == caSignedIntegerPtr->getSignedIntegerValue());

    cString caUnsignedIntegerName =  "my_unsigned_int";
    unsigned int caUnsignedIntegerValue = 5;
    CustomAttributeArgumentPtr caUnsignedIntegerPtr = cattrib.addArgument(caUnsignedIntegerName, caUnsignedIntegerValue);
    ASSERT(caUnsignedIntegerValue == caUnsignedIntegerPtr->getUnsignedIntegerValue());

    cString caBoolName =  "my_bool";
    bool caBoolValue = true;
    CustomAttributeArgumentPtr caBoolPtr = cattrib.addArgument(caBoolName, caBoolValue);
    ASSERT(caBoolValue == caBoolPtr->getBoolValue());

    TESTS_LOG(cattrib << endl);
}

void CustomAttributeTests::test(void)
{
    create_custom_attrib_with_args();
}