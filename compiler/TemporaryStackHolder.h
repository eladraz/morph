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

#ifndef __TBA_CLR_COMPILER_TEMPORARYSTACKHOLDER_H
#define __TBA_CLR_COMPILER_TEMPORARYSTACKHOLDER_H

/*
 * TemporaryStackHolder.cpp
 *
 * Contains a reference count object for temporary stack object holder.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "format/coreHeadersTypes.h"
#include "dismount/assembler/StackInterface.h"

// Forward deceleration
class TemporaryStackHolder;
// The reference countable object
typedef cSmartPtr<TemporaryStackHolder> TemporaryStackHolderPtr;

/*
 * Contain the reference information to store a temporary stack object. When
 * this class is no longer in use, the temporary object is automatically
 * destruct.
 *
 * See StackEntity for detailed information regarding the needs for this
 * struct.
 */
class TemporaryStackHolder {
public:
    // The different types for the stack allocation holding
    enum StackHolderAllocationRequest {
        // The allocated temporary register MUST be a register
        TEMP_ONLY_REGISTER,
        // The allocated temporary variable MUST be a stack based variable
        TEMP_ONLY_STACK,
        // The allocation temporary variable should be a register
        TEMP_PREFERRED_REGISTER,
        // The allocated temporary register MUST be a non-volatile register
        TEMP_NONVOLATILE_REGISTER,
        // The allocation temporary variable should be the accumulator register
        //      TEMP_PREFERRED_ACCUMULATOR
    };

    /*
     * Constructor. Allocate a temporary stack object.
     *
     * _interface  - The interface to allocated from
     * coreType    - The type of the register. For now the only possible values
     *               for core-type are:
     *                   ELEMENT_TYPE_I4, ELEMENT_TYPE_I2, ELEMENT_TYPE_I1
     *                   ELEMENT_TYPE_U4, ELEMENT_TYPE_U2, ELEMENT_TYPE_U1
     * size        - The size for the stack allocation
     * request     - The type of the requested stack type
     *
     * Throw exception if the stack holder cannot be allocated
     */
    TemporaryStackHolder(StackInterface& _interface,
                         CorElementType coreType,
                         uint size,
                         StackHolderAllocationRequest request);
    /*
     * Constructor. Allocate a stack object for a known StackLocation register
     *              Note: If this constructor is used, the register is not freed
     *                    when this object is desctructed!
     *
     * _interface    - The interface to allocated from
     * stackLocation - The register type of the requested stack type
     *
     * Throw exception if the stack holder cannot be allocated
     */
    TemporaryStackHolder(StackInterface& _interface,
                         const StackLocation& stackLocation);

    /*
     * Duplicate the current stack location on newly block interface, with only
     * one reference.
     *
     * _interface - The new block interface
     */
    TemporaryStackHolderPtr duplicateOnNewInterface(StackInterface& _interface);

    /*
     * Destructor. Free the temporary stack object
     */
    ~TemporaryStackHolder();

    /*
     * Return the temporary stack object pointer
     */
    const StackLocation& getTemporaryObject() const;
    StackLocation& getTemporaryObject();

    /*
     * Return the core type of the variable
     */
    CorElementType getCoreType() const;

private:
    // All possible held objects
    enum Type
    {
        AllocatedRegister,
        StackBuffer,
        DefinedRegister,
    };

    // Deny copy-constructor and operator =
    TemporaryStackHolder(const TemporaryStackHolder& other);
    TemporaryStackHolder& operator = (const TemporaryStackHolder& other);

    /*
     * Special constructor for 'duplicateOnNewInterface'
     */
    TemporaryStackHolder(StackLocation stackLocation,
                         Type type,
                         CorElementType coreType,
                         StackInterface& _interface);

    // The interface of which the temporary object was allocated
    StackInterface& m_interface;
    // The type of the register
    CorElementType m_coreType;
    // Type of object held - used for destruction/free
    Type m_type;
    // The stack location object
    StackLocation m_stackLocation;
};

#endif // __TBA_CLR_COMPILER_TEMPORARYSTACKHOLDER_H
