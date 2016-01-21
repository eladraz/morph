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

#ifndef __TBA_CLR_COMPILER_StackENTITY_H
#define __TBA_CLR_COMPILER_StackENTITY_H

/*
 * StackEntity.h
 *
 * Contains a single stack representation entity
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "data/ElementType.h"
#include "compiler/TemporaryStackHolder.h"

/*
 * Stack entity is a 'link' into the content loaded into the stack.
 * The are number of MSIL instruction which load information. Here is a
 * list of these instruction and thier 'link' type:
 * ENTITY_CONST
 *    ldc   - Load a const 32 bit signed integer
 *    ldc   - Load a const 64 bit signed integer
 *    ldc   - Load a const IEEE4 floating point
 *    ldc   - Load a const IEEE8 floating point
 * ENTITY_LOCAL
 *    ldloc  - Load a local variable
 * ENTITY_LOCALADDRESS
 *    ldloca - Load a pointer into a local variable
 *
 *    ldarg   - Load an argument variable
 *    ldarga  - Load an argument pointer
 *    ldftn   - Load method pointer
 *
 *
 * However there are more then a couple of instructions which caused loading
 * value into the stack. These instruction takes exist stack variable and change
 * them. The result is stored at unique datastruct called 'TemporaryStackHolder'
 * See TemporaryStackHolder for more information. This value often accelerated
 * using CPU registers settings.
 *
 * For example: Instruction 'add' takes two numbers (either both 32 bit, 64 bit
 *              or float/pointer) and add these numbers. The engine will allocate
 *              temporary stack hodler, copy the first variable and add the
 *              second variable.
 */
class StackEntity {
public:
    /*
     * Different stack entities
     */
    enum EntityType {
        // The entity is register(s) The register size depend on machine size.
        // NOTE: These entities are free automatically using the
        //       TemporaryStackHolderPtr objects
        ENTITY_REGISTER,
        // The entity is register which points to an address
        ENTITY_REGISTER_ADDRESS,
        // The entity is a temporary stack buffer
        ENTITY_LOCAL_TEMP_STACK,
        // The entity is a temporary stack buffer pointer (=address)
        ENTITY_LOCAL_TEMP_STACK_ADDRESS,
        // The entity is a temporary stack buffer pointer to pass by addr
        ENTITY_LOCAL_TEMP_STACK_PTR,
        // The entity is a const. See ConstValues class
        ENTITY_CONST,
        // The entity is a local variable
        ENTITY_LOCAL,
        // The element is address for local variable
        ENTITY_LOCAL_ADDRESS,
        // The entity is argument to method
        ENTITY_ARGUMENT,
        // The entity is argument address
        ENTITY_ARGUMENT_ADDRESS,
        // The entity is a pointer into some data-section/fix memory address
        ENTITY_TOKEN_ADDRESS,
        // The entity is a pointer into method address
        ENTITY_METHOD_ADDRESS,
        // A fixed memory value
        ENTITY_ADDRESS_VALUE,
        // A fixed memory address
        ENTITY_ADDRESS_ADDRESS,
        // A fixed memory address
        ENTITY_ADDRESS_WITH_DATA,
        // A fixed memory translated into string
        ENTITY_ADDRESS_WITH_STRING_DATA
    };

    /*
     * Holds the CONST values. See StackEntity::setConstValue
     * So far we only support numbers since this is what we are using...
     */
    class ConstValue {
    public:
        /*
         * Default constructor
         */
        ConstValue(int constValue = 0);

        /*
         * Getter/Setter for the const int value
         */
        int getConstValue() const;
        void setConstValue(int constValue);
        int64 getConst64Value() const;
        void  setConst64Value(int64 constValue);

        /*
         * Getter/Setter for the local/arg value
         */
        int getLocalOrArgValue() const;
        void setLocalOrArgValue(int localOrArgValue);

        /*
         * Getter/Setter for the local/arg value
         */
        const TokenIndex& getTokenIndex() const;
        void setTokenIndex(const TokenIndex& tokenIndex);

    private:
        // The const values
        union {
            int        m_intValue;
            int        m_localOrArgIndex;
            int64      m_longValue;
        } m_u;
        // The token cannot be include in the uniun deince there is issue with the compiler
        TokenIndex m_token;
    };

    /*
     * Default constructor
     */
    StackEntity();

    /*
     * Constructor
     *
     * type                 - The type of the entity
     * localOrArgumentIndex - Extra information regarding the entity
     * var   - Variable used to encode ElementType/Data and more
     */
    StackEntity(EntityType type,
                const ElementType& elementType,
                bool isReturned = false);

    // Copy-constructor and operator = will auto-generated by the compiler

    /*
     * Return/set the type of the entity
     */
    EntityType getType() const;
    void setType(EntityType type);

    /*
     * Return the type of the
     */
    const ElementType& getElementType() const;
    void setElementType(const ElementType& newType);

    /*
     * Return the variable object
     */
    const ConstValue& getConst() const;
    ConstValue& getConst();

    bool isReturned() const;
    void setReturned(bool isReturned);

    /*
     * Return the temporary stack holder reference
     */
    const TemporaryStackHolderPtr& getStackHolderObject() const;
    TemporaryStackHolderPtr& getStackHolderObject();

    /*
     * Set the temporary stack holder reference
     */
    void setStackHolderObject(const TemporaryStackHolderPtr& newObject);

    /*
     * Changes the type of the stack entity to the corrosponding address type
     */
    void AddressUpType();

private:
    // The stack type object
    EntityType m_type;
    // The type type
    ElementType m_elementType;
    // Const value if there is one
    ConstValue m_const;
    // The temporary stack holder reference object
    TemporaryStackHolderPtr m_stackHolder;
    // True if it was returned from a call
    bool m_isReturned;
};

#endif // __TBA_CLR_COMPILER_StackENTITY_H
