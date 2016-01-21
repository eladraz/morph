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

#ifndef __TBA_CLR_DATA_CONSTELEMENTS_H
#define __TBA_CLR_DATA_CONSTELEMENTS_H

/*
 * ConstElements.h
 *
 * Define of global const elements which simplify memory requirements and
 * application speed.
 * NOTE: This elements only required for the engine.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "data/ElementType.h"

/*
 * See prolog
 */
class ConstElements {
public:
    // void
    static const ElementType gVoid;
    // byte
    static const ElementType gByte;
    // byte*
    static const ElementType gBytePtr;
    // character
    static const ElementType gChar;
    // character*
    static const ElementType gCharPtr;
    // bool
    static const ElementType gBool;
    // int8
    static const ElementType gI1;
    // int16
    static const ElementType gI2;
    // int32
    static const ElementType gI4;
    // int64
    static const ElementType gI8;
    // uint8
    static const ElementType gU1;
    // uint16
    static const ElementType gU2;
    // uint32
    static const ElementType gU4;
    static const ElementType gU4Ref;
    // uint64
    static const ElementType gU8;

    #ifdef CLR_FLOAT_ENABLE
    // float
    static const ElementType gR4;
    // double
    static const ElementType gR8;
    #endif // CLR_FLOAT_ENABLE

    // Native signed/unsigned integers
    // See also ElementType::translateNativeSize
    #define gI gI4
    #define gU gU4

    // int32*
    static const ElementType gI4Ptr;
    // void*
    static const ElementType gVoidPtr;
    // string
    static const ElementType gString;
    // void[] -> Used for generic Array class
    static const ElementType gVoidArray;
    // string[]
    static const ElementType gStringArray;
};

#endif // __TBA_CLR_DATA_CONSTELEMENTS_H

