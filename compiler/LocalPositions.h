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

#ifndef __TBA_CLR_COMPILER_LOCALPOSITIONS_H
#define __TBA_CLR_COMPILER_LOCALPOSITIONS_H

/*
 * LocalPositions.h
 *
 * Handle method locals.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "data/ElementType.h"
#include "runnable/ResolverInterface.h"

// Forward deceleration
class MethodCompiler;

/*
 * Translate between local to local positions (+alignment) and sizes
 */
class LocalPositions {
public:
    /*
     * Constructor. Generate new locals template
     */
    LocalPositions();

    /*
     * Return the stack position of local variable
     *
     * index - The index for the local
     */
    uint getLocalPosition(uint index) const;

    /*
     * Return the variable object. The variable object is used for managing the
     * apartment object and the different variable type.
     *
     * index - The index for the local
     */
    const ElementType& getLocalStackVariableType(uint index) const;

    /*
     * Returns the total amount of locals
     */
    uint getSize() const;

    /*
     * Returns the amount of locals which are objects and not value types
     */
    uint countObjects(ResolverInterface& resolver, bool shouldCountFields) const;

    /*
     * Returns the index of the first local which is an object and not value type
     */
    uint firstObjectIndex() const;

private:
    // Only MethodCompiler can build a stack template
    friend class MethodCompiler;
    // Deny copy-constructor and operator =
    LocalPositions(const LocalPositions& other);
    LocalPositions& operator = (const LocalPositions& other);

    // The array of locals positions
    cSArray<uint> m_localPosition;
    // The array of locals types
    ElementsArrayType m_localTypes;

    // NOTE: Each new member should affect the MethodCompiler and the
    //       MethodCompiler::calculateStackSize as well!
};

#endif // __TBA_CLR_COMPILER_LOCALPOSITIONS_H
