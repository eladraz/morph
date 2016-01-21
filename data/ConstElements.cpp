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
 * ConstElements.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "stdafx.h"
#include "data/ConstElements.h"

const ElementType ConstElements::gVoid(ELEMENT_TYPE_VOID,    0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gByte(ELEMENT_TYPE_U1,      0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gBytePtr(ELEMENT_TYPE_U1,   1, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gBool(ELEMENT_TYPE_BOOLEAN, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gChar(ELEMENT_TYPE_CHAR   , 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gCharPtr(ELEMENT_TYPE_CHAR, 1, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gI1(ELEMENT_TYPE_I1       , 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gI2(ELEMENT_TYPE_I2, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gI4(ELEMENT_TYPE_I4, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gU1(ELEMENT_TYPE_U1, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gU2(ELEMENT_TYPE_U2, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gU4(ELEMENT_TYPE_U4, 0, false, false, false, TokenIndex(-1, -1));

const ElementType ConstElements::gU4Ref(ELEMENT_TYPE_U4, 0, true, false, false, TokenIndex(-1, -1));

#ifdef CLR_I8_ENABLE
    const ElementType ConstElements::gU8(ELEMENT_TYPE_U8, 0, false, false, false, TokenIndex(-1, -1));
    const ElementType ConstElements::gI8(ELEMENT_TYPE_I8, 0, false, false, false, TokenIndex(-1, -1));
#else
    const ElementType ConstElements::gU8(ELEMENT_TYPE_U4, 0, false, false, false, TokenIndex(-1, -1));
    const ElementType ConstElements::gI8(ELEMENT_TYPE_I4, 0, false, false, false, TokenIndex(-1, -1));
#endif


#ifdef CLR_FLOAT_ENABLE
const ElementType ConstElements::gR4(ELEMENT_TYPE_R4, 0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gR8(ELEMENT_TYPE_R8, 0, false, false, false, TokenIndex(-1, -1));
#endif // CLR_FLOAT_ENABLE
const ElementType ConstElements::gI4Ptr(ELEMENT_TYPE_I4,       1, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gVoidPtr(ELEMENT_TYPE_VOID,   1, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gString(ELEMENT_TYPE_STRING,  0, false, false, false, TokenIndex(-1, -1));
const ElementType ConstElements::gVoidArray(ELEMENT_TYPE_VOID, 0, false, false, true, TokenIndex(-1, -1));
