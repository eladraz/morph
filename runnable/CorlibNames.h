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

#ifndef __TBA_CLR_RUNNABLE_CORLIB_CORLIBNAMES_H
#define __TBA_CLR_RUNNABLE_CORLIB_CORLIBNAMES_H

/*
 * CorlibNames.h
 *
 * Containing all names of the Corlib. Including namespace, objects names,
 * typedef names and method names.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"

class CorlibNames {
public:
    static bool isFinalizer(cString methodName);

    //////////////////////////////////////////////////////////////////////////
    // System namespace

    // Class separator
    static const char m_classSeparator[];

    // The "System" namespace.
    static const char m_systemNamespace[];
    // "Object"
    static const char m_classObject[];
    // "String"
    static const char m_classString[];
    // "Array"
    static const char m_classArray[];
    // "ValueType:
    static const char m_classValueType[];
    // "Exception"
    static const char m_classException[];

    // Methods

    // .Finalize method (~Destructor)
    static const char m_methodFinalizer[];
    // .cctor method
    static const char m_methodCctor[];
    // .Ctor method
    static const char m_methodCtor[];


    //////////////////////////////////////////////////////////////////////////
    // Morph namespace

    // The default corelib namespace: Morph
    static const char gCoreNamespace[];
};

#endif // __TBA_CLR_RUNNABLE_CORLIB_CORLIBNAMES_H
