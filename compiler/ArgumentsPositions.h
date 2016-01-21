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

#ifndef __TBA_CLR_COMPILER_ARGUMENTSPOSITIONS_H
#define __TBA_CLR_COMPILER_ARGUMENTSPOSITIONS_H

/*
 * ArgumentsPositions.h
 *
 * Handle method arguments.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "data/ElementType.h"

// Forward deceleration
class MethodCompiler;

/*
 * Translate between argument to argument positions (+alignment) and sizes
 */
class ArgumentsPositions {
public:
    /*
     * Constructor. Generate new arguments template
     */
    ArgumentsPositions();

    /*
     * Return the size of all arguments
     */
    uint getTotalArgumentsSize() const;

    /*
     * Return the stack position of argument variable
     *
     * index - The index for the argument
     */
    uint getArgumentPosition(uint index) const;

    /*
     * Return the stack size of an argument
     *
     * index - The index for the argument
     */
    uint getArgumentStackSize(uint index) const;

    /*
     * Return the variable object. The variable object is used for managing the
     * apartment object and the different variable type.
     *
     * index - The index for the argument
     */
    const ElementType& getArgumentStackVariableType(uint index) const;

    /*
     * Return the amount of arguments
     */
    uint getCount() const;

    /*
     * Count the amount of object arguments
     */
    uint countObjects() const;

private:
    // Only CallingConvention can build a stack template
    friend class CallingConvention;

    // Deny copy-constructor and operator =
    ArgumentsPositions(const ArgumentsPositions& other);
    ArgumentsPositions& operator = (const ArgumentsPositions& other);

    // The array of arguments positions
    cSArray<uint> m_argumentPosition;
    // The array of arguments sizes
    cSArray<uint> m_argumentSizes;
    // The array of arguments types
    cArray<ElementType> m_argumentTypes;

    // the size of all arguments
    uint m_totalArgumentsSize;

    // NOTE: Each new member should affect the MethodCompiler and the
    //       MethodCompiler::calculateStackSize as well!
};

#endif // __TBA_CLR_COMPILER_ARGUMENTSPOSITIONS_H
