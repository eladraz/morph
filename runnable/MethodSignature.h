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

#ifndef __TBA_CLR_COMPILER_METHODSIGNATURE_H
#define __TBA_CLR_COMPILER_METHODSIGNATURE_H


/*
 * MethodSignature.h
 *
 * Calculate a signature on a method, including locals and arguments
 * signature. Whenever a typedef/typeref has changed, the signature
 * will also result in a different value, hence recalculate will occured.
 *
 * Also different versions of the compiler (typedef repository, alignment etc)
 * will also yeld in new signature.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/sarray.h"
#include "xStl/enc/digest.h"
#include "xStl/enc/digest/Crc64.h"
#include "format/coreHeadersTypes.h"
#include "format/signatures/LocalVarSignature.h"
#include "runnable/Apartment.h"
#include "runnable/MethodRunnable.h"
#include "compiler/LocalPositions.h"

class MethodSignature
{
public:
    /*
     * Calculate a signature for a method, by method signature, object signature
     * locals signature, arguments signature and MSIL code
     */
    static cBuffer getMethodSignature(ApartmentPtr& mainApartment,
                                      MethodRunnable& methodRunnable,
                                      uint compilerType);
};

#endif // __TBA_CLR_COMPILER_METHODSIGNATURE_H

