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

#ifndef __TBA_FORMAT_SIGNATURES_FIELDSIG_H
#define __TBA_FORMAT_SIGNATURES_FIELDSIG_H

/*
 * FieldSig.h
 *
 * Parse the blob of type "FieldSig"
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/basicIO.h"
#include "data/ElementType.h"
#include "runnable/ResolverInterface.h"

/*
 * A FieldSig is indexed by the Field.Signature column, or by the MemberRef->
 * Signature column (in the case where it specifies a reference to a field, not
 * a method, of course).
 * The Signature captures the field’s definition. The field may be a static or
 * instance field in a class, or it may be a global variable.
 */
class FieldSig {
public:
    /*
     * Constructor. Read from the #Blob stream position by 'stream' the field
     * signature
     */
    FieldSig(basicInput& stream, mdToken apartmentId, const ResolverInterface& resolverInterface);

    // Copy-constructor and operator = will auto-generate by the compiler even
    // if they aren't nessasry...

    /*
     * Return the field's type
     */
    const ElementType& getType() const;

private:
    // No need of operator << for CLR_TRACED since the ElementType contains one

    // The blob signature of field-signature
    enum { FIELD_SIGNATURE = 6 };

    // The field type
    ElementType m_type;
};

#endif // __TBA_FORMAT_SIGNATURES_FIELDSIG_H
