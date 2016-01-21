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

#ifndef __TBA_CLR_DATA_ELEMENTTYPE_H
#define __TBA_CLR_DATA_ELEMENTTYPE_H

/*
 * ElementType.h
 *
 * The type of an element
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/enc/digest.h"
#include "xStl/data/array.h"
#include "xStl/data/string.h"
#include "xStl/data/smartptr.h"
#include "xStl/data/dualElement.h"
#include "xStl/except/exception.h"
#include "xStl/stream/basicIO.h"
#include "xStl/stream/stringerStream.h"
#include "format/coreHeadersTypes.h"

/*
 * Forward deceleration for output streams
 */

// m_a is the methodId m_b is the apartmentId
//typedef cDualElement<mdToken,mdToken> TokenIndex;

class TokenIndex: public cDualElement<mdToken,mdToken>
{
public:
    TokenIndex(void) : cDualElement<mdToken,mdToken>()
    {
    };

    TokenIndex(mdToken apartmentId, mdToken tokenId) : cDualElement<mdToken,mdToken>(apartmentId, tokenId)
    {
    };

    bool operator==(const TokenIndex& other) const
    {
        return (m_a == other.m_a) && (m_b == other.m_b);
    }
};

// Macro which helps managing the struct
#define getTokenID(x) ((x).m_a)
#define getApartmentID(x) ((x).m_b)

#define buildTokenIndex(apartmentId, tokenId) (TokenIndex(tokenId, apartmentId))
// Hashing function declarations
uint cHashFunction(const TokenIndex& index, uint range);

// Output trace on tokens
cString HEXTOKEN(const TokenIndex& tokenIndex);

// Forward declaration
class ResolverInterface;

/*
 * See CorElementType. Encode one of the ELEMENT_TYPE_XXX.
 * A type can be one of the following
 *
 * Author: Elad Raz <e@eladraz.com>
 */
class ElementType {
public:
    /*
     * The unresolved token index is used for objects that thier token is not unkown yet
     */
    static const TokenIndex UnresolvedTokenIndex;

    /*
     * Default constructor.
     * Generate a simple type of element.
     *
     * type - The end type of the element. One of the simplest following
     *        elements:
     *            ELEMENT_TYPE_VOID (0x01) until ELEMENT_TYPE_STRING (0x0E)
     *            including, ELEMENT_TYPE_I (0x18), ELEMENT_TYPE_U (0x19)
     *            For ELEMENT_TYPE_VALUETYPE (0x11), ELEMENT_TYPE_CLASS (0x12)
     *            the 'classToken' must also be valid.
     *        The reason why this argument has a default value is to allow
     *        array constructor to safely generate array of "ElementType".
     * pointerLevel  - The number of pointers levels
     * isReference   - Set to true if the type is a reference
     * isPinned      - Set to true if the type is pinned
     * isSingleArray - Set to true if the type is bound inside a single-sized
     *                 array
     * classToken - Typedef (or typeref before resolver) to the actual class
     *
     * For generic:
     *   genericClass - The class token.  The "stack" in "Stack<int>"
     *   genericTypes - The type of the generic instance: The "int" in "Stack<int>"
     *
     * Throw exception if the element type is one of the more complex typedef
     * (such as reference, constraint, array etc..) For this element-type the
     * class contains 'type properties'
     */
    ElementType(CorElementType type = ELEMENT_TYPE_VOID,
                uint pointerLevel   = 0,
                bool isReference    = false,
                bool isPinned       = false,
                bool isSingleArray  = false,
                const TokenIndex& classToken = UnresolvedTokenIndex,
                ElementType* genericClass = NULL,
                const cArray<ElementType>* genericTypes = NULL);

    /*
     * Copy-constructor and operator = will auto generated by the compiler
     */

    /*
     * Return the end type of the element.
     * Might be one of the following:
     *     ELEMENT_TYPE_VOID      (0x01) until ELEMENT_TYPE_STRING (0x0E)
     *     ELEMENT_TYPE_VALUETYPE (0x11)
     *     ELEMENT_TYPE_CLASS     (0x12)
     *     ELEMENT_TYPE_I         (0x18)
     *     ELEMENT_TYPE_U         (0x19)
     */
    CorElementType getType() const;

    /*
     * Return the raw-size needed to store 'type'.
     * Return 0 for unknown object size such as: I/U/Strings, Objects, Value-types and classes
     */
    static uint getTypeSize(CorElementType type);

    /*
     * Return true if there is at least one pointer
     */
    bool isPointer() const;
    bool isReference() const;
    bool isPinned() const;

    uint getPointerLevel() const;
    void setPointerLevel(uint level);

    /*
     * If type is ValueType or ClassType the function return the class token
     * key
     *
     * Throw exception if type don't encode any class-token
     */
    const TokenIndex& getClassToken() const;

    /*
     * Return true if the type is bounded by a single-dimension array
     */
    bool isSingleDimensionArray() const;

    // Operators

    /*
     * Convert the element into array of elements
     * Used to prevent element-duplication during operation 'newarr'
     *
     * Throw exception if the element was encoded as an array
     */
    void convertToArray();
    void unconvertFromArray();

    /*
     * Compare the types of two elements and return true if they are identical.
     */
    bool operator == (const ElementType& other) const;
    bool operator != (const ElementType& other) const;

    /*
     * Read encoded Element type from a stream
     *
     * stream - An input-stream which point to the position of the ElementType
     *          encoding.
     * apartmentId - The apartment Index
     *
     * Return the decoded element-type
     *
     * Throw exception if the element-type is invalid.
     *
     * Throw NoElementException, if the element has a ELEMENT_TYPE_END value.
     * NOTE: Most of the times when a function tries to read an element it's
     *       doesn't expect to find end-of-list type identifers, the method of
     *       throwing an exception will save if statement for each reading, only
     *       function which might encode list of element will have to wrap thier
     *       body with a 'try-catch' block
     */
    static ElementType readType(basicInput& stream, mdToken apartmentId);

    /*
     * Convert ELEMENT_TYPE_U1..ELEMENT_TYPE_U8 to thier coressponding integer
     * values: ELEMENT_TYPE_I1..ELEMENT_TYPE_I8
     *
     * Throw exception if unsignedType is not in the above range
     */
    static CorElementType convertUnsignedToSigned(CorElementType unsignedType);

    /*
     * Return true if the type of the element is one of the following:
     *      ELEMENT_TYPE_I1, ELEMENT_TYPE_I2, ELEMENT_TYPE_I4, ELEMENT_TYPE_I8
     *      ELEMENT_TYPE_I
     * Return false otherwise.
     *
     * NOTE: If the type is a pointer to that native type, the function will
     *       return false.
     */
    bool isIntegerType() const;

    /*
     * Return true if the type of the element is one of the following:
     *      ELEMENT_TYPE_U1, ELEMENT_TYPE_U2, ELEMENT_TYPE_U4, ELEMENT_TYPE_U8
     *      ELEMENT_TYPE_U
     * Return false otherwise.
     *
     * NOTE: If the type is a pointer to that native type, the function will
     *       return false.
     */
    bool isUnsignedIntegerType() const;

    /*
     * Return true if the type of the element is ELEMENT_TYPE_R4-ELEMENT_TYPE_R8,
     * ELEMENT_TYPE_STRING, ELEMENT_TYPE_CHAR or ELEMENT_TYPE_BOOL
     * Return false otherwise, or if the element is a pointer to a variable.
     */
    bool isBool() const;
    bool isChar() const;
    bool isFloat() const;
    bool isString() const;
    bool isVoid() const;   // Check real void (not void*, void[])

    /*
     * Return true if the type of the element is ELEMENT_TYPE_CLASS,
     * ELEMENT_TYPE_VALUETYPE or ELEMENT_TYPE_OBJECT
     * Return false otherwise, or if the element is a pointer to a variable.
     */
    bool isObject() const;
    static bool isObject(CorElementType type);
    bool isObjectAndNotValueType() const;

    /*
     * Return the generic elements
     */
    const cArray<ElementType>& getGenericTypes() const;
    const ElementType& getGenericClass() const;

    /*
     * See ElementType::readType
     */
    class NoElementException : public cException {
    public:
        NoElementException(char * file, uint32 line, const character* msg = NULL) : cException(file, line, msg) {};
    };

    /*
     * Should be called inorder to verify that the Element points to typedef table
     * (e.g. after transalation). See ReolverInterface::resolveTyperef()
     */
    void assertTyperef() const;
    static void assertTyperef(const TokenIndex& index);

    /*
     * Update hash according to element unique signature
     */
    void hashElement(cDigest& digest, const ResolverInterface& resolver) const;

private:
    // All alone am I
    friend class TypedefRepository;

    #ifdef TRACED_CLR
    friend cStringerStream& operator << (cStringerStream& out,
                                         const ElementType& object);
    #endif // TRACED_CLR

    /*
     * Private recursive implementation of 'readType'
     * See ElementType::readType
     */
    static ElementType privateReadType(basicInput& stream,
                                       mdToken apartmentId,
                                       uint pointerLevel = 0,
                                       bool isReference = false,
                                       bool isPinned = false,
                                       const TokenIndex& paramClassToken = UnresolvedTokenIndex,
                                       bool paramIsSingleArray = false);


    // The type of the element
    CorElementType m_type;
    // The different pointer properties
    uint m_pointerLevel;
    // Set to true for reference
    bool m_isReference;
    // Set to true for pinned objects
    bool m_isPinned;
    // The class/typedef-token
    TokenIndex m_classToken;
    // Set to true for single-dimension array
    bool m_isSingleArray;

    // Contains type of inner generic types
    // Example List<Node, Value> has 2 inner types class Node and class Value
    cArray<ElementType> m_genericTypes;
    cSmartPtr<ElementType> m_genericClass;
};

// An array of element's types
typedef cArray<ElementType> ElementsArrayType;

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out, const ElementType& object);
cStringerStream& operator << (cStringerStream& out, const TokenIndex& tokenIndex);
#endif // TRACED_CLR

#endif // __TBA_CLR_FORMAT_ELEMENTTYPE_H

