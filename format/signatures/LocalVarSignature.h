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

#ifndef __TBA_CLR_FORMAT_SIGNATURES_LOCALVARSIGNATURE_H
#define __TBA_CLR_FORMAT_SIGNATURES_LOCALVARSIGNATURE_H

/*
 * LocalVarSignature.h
 *
 * Parse the blob of type "LocalVarSig"
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/utils/dumpMemory.h"
#include "xStl/except/trace.h"
#include "xStl/stream/stringerStream.h"
#include "data/ElementType.h"
#include "runnable/ResolverInterface.h"

/*
 * Forward deceleration for output streams
 */
#ifdef TRACED_CLR
class LocalVarSignature;
cStringerStream& operator << (cStringerStream& out,
                              const LocalVarSignature& object);
#endif // TRACED_CLR


/*
 * A LocalVarSig is indexed by the StandAloneSig.Signature column. It captures
 * the type of all the local variables in a method.
 */
class LocalVarSignature {
public:
    /*
     * Constructor. Read the blob section:
     *
     * stream - The stream into the blob section which points to the beginning
     *          of the signature
     */
    LocalVarSignature(basicInput& stream,
                      mdToken apartmentId,
                      const ResolverInterface& resolverInterface);

    /*
     * Default constructor. Generate empty local list
     */
    LocalVarSignature();

    // Copy-constructor and operator = will auto generated by the compiler

    /*
     * Return the array of all locals this signature represents
     */
    const ElementsArrayType& getLocals() const;

private:
    // Everyone need a friend
    #ifdef TRACED_CLR
    friend cStringerStream& operator << (cStringerStream& out,
                                         const LocalVarSignature& object);
    #endif

    // The signature byte which encode at the beginning of the blob
    enum { LOCAL_VAR_SIGNATURE = 0x07 };

    // A list of all local variables
    ElementsArrayType m_locals;
};

#endif // __TBA_CLR_FORMAT_SIGNATURES_LOCALVARSIGNATURE_H
