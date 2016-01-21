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
 * CorlibNames.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "runnable/CorlibNames.h"

//////////////////////////////////////////////////////////////////////////
// System namespace

// The namespace
const char CorlibNames::m_classSeparator[] = ".";
const char CorlibNames::m_systemNamespace[] = "System";
const char CorlibNames::m_classObject[] = "Object";
const char CorlibNames::m_classString[] = "String";
const char CorlibNames::m_classArray[] = "Array";
const char CorlibNames::m_classValueType[] = "ValueType";
const char CorlibNames::m_classException[] = "Exception";

const char CorlibNames::m_methodFinalizer[] = "Finalize";
const char CorlibNames::m_methodCctor[] = ".cctor";
const char CorlibNames::m_methodCtor[] = ".ctor";

// Morph namespace

const char CorlibNames::gCoreNamespace[] = "Morph";

bool CorlibNames::isFinalizer(cString methodName)
{

    int pos = methodName.rfind(".");
    cString basename = methodName.right(methodName.length() - pos - 1);
    if (basename.compare(CorlibNames::m_methodFinalizer) == cString::EqualTo) {
        return true;
    }

    return false;
}
