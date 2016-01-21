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
 * ElementType.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "stdafx.h"
#include "data/ElementType.h"
#include "data/exceptions.h"
#include "runnable/ResolverInterface.h"

const TokenIndex ElementType::UnresolvedTokenIndex(-1, -1);

ElementType::ElementType(CorElementType type,
                         uint pointerLevel,
                         bool isReference,
                         bool isPinned,
                         bool isSingleArray,
                         const TokenIndex& classToken,
                         ElementType* genericClass,
                         const cArray<ElementType>* genericTypes) :
    m_type(type),
    m_pointerLevel(pointerLevel),
    m_isPinned(isPinned),
    m_isReference(isReference),
    m_classToken(classToken),
    m_isSingleArray(isSingleArray)
{
    #ifndef CLR_FLOAT_ENABLE
    // If the enviroment doesn't support floating-point operation. Each
    // occurence with floating point should raise exception.
    if ((type == ELEMENT_TYPE_R4) || (type == ELEMENT_TYPE_R8))
        XSTL_THROW(ClrFloatingPointEngineNotFound);
    #endif // CLR_FLOAT_ENABLE

    #ifndef CLR_I8_ENABLE
        if (type == ELEMENT_TYPE_U8)
            m_type = ELEMENT_TYPE_U4;
        if (type == ELEMENT_TYPE_I8)
            m_type = ELEMENT_TYPE_I4;
    #endif

    if (genericClass != NULL)
    {
        m_genericClass = cSmartPtr<ElementType>(new ElementType(*genericClass));
    }
    if (genericTypes != NULL)
    {
        m_genericTypes = *genericTypes;
    }

    CHECK(((type >= ELEMENT_TYPE_END) && (type < ELEMENT_TYPE_PTR)) ||
          (type == ELEMENT_TYPE_I) ||
          (type == ELEMENT_TYPE_U) ||
          (type == ELEMENT_TYPE_OBJECT) ||
          (type == ELEMENT_TYPE_VALUETYPE) ||
          (type == ELEMENT_TYPE_CLASS) ||
          (type == ELEMENT_TYPE_VAR) ||
          (type == ELEMENT_TYPE_SZARRAY) ||
          (type == ELEMENT_TYPE_GENERICINST));
}

CorElementType ElementType::getType() const
{
    return m_type;
}

uint ElementType::getTypeSize(CorElementType type)
{
    switch (type)
    {
    case ELEMENT_TYPE_BOOLEAN:
        return sizeof(bool);
    case ELEMENT_TYPE_CHAR:
        #ifdef CLR_UNICODE
            // TODO! For unicode support return 2;
            CHECK_FAIL();
        #else
            return 1;
        #endif
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
        return 1;
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
        return 2;
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
        return 4;
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
        return 8;
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
        // See TypedefRepository
        // Don't know size yet
        return 0;

    case ELEMENT_TYPE_END:
    case ELEMENT_TYPE_VOID:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_VALUETYPE:
    case ELEMENT_TYPE_GENERICINST:
        return 0;
    default:
        CHECK_FAIL();
    }
}

uint ElementType::getPointerLevel() const
{
    return m_pointerLevel;
}

void ElementType::setPointerLevel(uint level)
{
    m_pointerLevel = level;
}


bool ElementType::isPointer() const
{
    return m_pointerLevel > 0;
}

bool ElementType::isReference() const
{
    return m_isReference;
}

bool ElementType::isPinned() const
{
    return m_isPinned;
}


const TokenIndex& ElementType::getClassToken() const
{
    return m_classToken;
}

bool ElementType::isSingleDimensionArray() const
{
    return m_isSingleArray;
}

void ElementType::convertToArray()
{
    CHECK(!m_isSingleArray);

    m_isSingleArray = true;
}

void ElementType::unconvertFromArray()
{
    CHECK(m_isSingleArray);

    m_isSingleArray = false;
}

bool ElementType::operator == (const ElementType& other) const
{
    // Mini cast for objects
    if ((m_type == other.m_type) &&
        (m_classToken == other.m_classToken) &&
        (m_isSingleArray == other.m_isSingleArray) &&
        (m_pointerLevel == other.m_pointerLevel) &&
        (m_isPinned == other.m_isPinned) &&
        (m_isReference == other.m_isReference) &&
        (m_genericTypes.getSize() == other.m_genericTypes.getSize()))
    {
        if (!m_genericClass.isEmpty())
        {
            if (other.m_genericClass.isEmpty())
                return false;
            if (*m_genericClass != *other.m_genericClass)
                return false;
        }

        for (uint i = 0; i < m_genericTypes.getSize(); i++)
            if (m_genericTypes[i] != other.m_genericTypes[i])
                return false;

        return true;
    }

    return false;
}

bool ElementType::operator != (const ElementType& other) const
{
    return !(*this == other);
}

bool ElementType::isIntegerType() const
{
    if (isPointer())
        return false;

    return ((m_type == ELEMENT_TYPE_I) ||
            (m_type == ELEMENT_TYPE_I4) ||
            (m_type == ELEMENT_TYPE_I1) ||
            (m_type == ELEMENT_TYPE_I8) ||
            (m_type == ELEMENT_TYPE_I2));
}

bool ElementType::isUnsignedIntegerType() const
{
    if (isPointer())
        return false;

    return ((m_type == ELEMENT_TYPE_U) ||
            (m_type == ELEMENT_TYPE_U4) ||
            (m_type == ELEMENT_TYPE_U1) ||
            (m_type == ELEMENT_TYPE_U8) ||
            (m_type == ELEMENT_TYPE_U2));
}

bool ElementType::isFloat() const
{
    if (isPointer())
        return false;

    return ((m_type == ELEMENT_TYPE_R4) ||
            (m_type == ELEMENT_TYPE_R8));
}

bool ElementType::isBool() const
{
    if (isPointer())
        return false;
    return m_type == ELEMENT_TYPE_BOOLEAN;
}

bool ElementType::isString() const
{
    if (isPointer())
        return false;
    return m_type == ELEMENT_TYPE_STRING;
}

bool ElementType::isChar() const
{
    if (isPointer())
        return false;
    return m_type == ELEMENT_TYPE_CHAR;
}

bool ElementType::isVoid() const
{
    return (m_type == ELEMENT_TYPE_VOID) && (!isPointer()) && !m_isSingleArray;
}

bool ElementType::isObject(CorElementType type)
{
    return (type == ELEMENT_TYPE_OBJECT) ||
           (type == ELEMENT_TYPE_STRING) ||
           (type == ELEMENT_TYPE_CLASS) ||
           (type == ELEMENT_TYPE_VALUETYPE) ||
           (type == ELEMENT_TYPE_VAR) ||
           (type == ELEMENT_TYPE_GENERICINST);
}

bool ElementType::isObjectAndNotValueType() const
{
    if (isSingleDimensionArray())
        return true;

    if (isPointer())
        return false;

    return (m_type == ELEMENT_TYPE_OBJECT) ||
           (m_type == ELEMENT_TYPE_CLASS) ||
           (m_type == ELEMENT_TYPE_STRING);
    // TODO! Check inner types (!)
    // (type == ELEMENT_TYPE_VAR) ||
    // (type == ELEMENT_TYPE_GENERICINST);
}

bool ElementType::isObject() const
{
    if (isPointer())
        return false;

    return isObject(m_type);
}

const cArray<ElementType>& ElementType::getGenericTypes() const
{
    return m_genericTypes;
}
const ElementType& ElementType::getGenericClass() const
{
    return *m_genericClass;
}


CorElementType ElementType::convertUnsignedToSigned(CorElementType unsignedType)
{
    // See CorElementType enum values for the following procedure
    uint ret = unsignedType - 1;

    if (((ret & 1) != 0) || // Must be odd
        (ret < ELEMENT_TYPE_I1) || // Must be in range
        (ret > ELEMENT_TYPE_U8))
        CHECK_FAIL();

    return (CorElementType)ret;
}

ElementType ElementType::readType(basicInput& stream, mdToken apartmentId)
{
    return privateReadType(stream, apartmentId);
}

ElementType ElementType::privateReadType(basicInput& stream,
                            mdToken apartmentId,
                            uint pointerLevel,
                            bool isReference,
                            bool isPinned,
                            const TokenIndex& paramClassToken,
                            bool paramIsSingleArray)
{
    uint8 type;
    stream.streamReadUint8(type);

    if (type == ELEMENT_TYPE_END)
        XSTL_THROW(NoElementException);

    if ((type < ELEMENT_TYPE_PTR) ||
        (type == ELEMENT_TYPE_I) ||
        (type == ELEMENT_TYPE_U) ||
        (type == ELEMENT_TYPE_OBJECT))
    {
        // Now we can instance the class
        return ElementType((CorElementType)type,
                           pointerLevel, isReference, isPinned, paramIsSingleArray, UnresolvedTokenIndex);
    }

    bool isRP = false;
    switch (type)
    {
    case ELEMENT_TYPE_PTR:
        pointerLevel++;
        isRP = true;
        break;

    case ELEMENT_TYPE_BYREF:
        isReference = true;
        isRP = true;
        break;

    case ELEMENT_TYPE_PINNED:
        isPinned = true;
        isRP = true;
        break;
    }

    if (isRP)
    {
        return privateReadType(stream, apartmentId, pointerLevel, isReference, isPinned, paramClassToken, paramIsSingleArray);
    }

    // Test for class element
    if ((type == ELEMENT_TYPE_VALUETYPE) ||
        (type == ELEMENT_TYPE_CLASS) ||
        (type == ELEMENT_TYPE_VAR))
    {
        // Read the class token
        mdToken classTokenId = EncodingUtils::unpackTypeDefOrRefToken(EncodingUtils::readCompressedNumber(stream));
        return ElementType((CorElementType)type,
                           pointerLevel, isReference, isPinned, paramIsSingleArray,
                           buildTokenIndex(apartmentId, classTokenId));
    }

    if (type == ELEMENT_TYPE_GENERICINST)
    {
        // Read the class type of the generic
        ElementType genericClass = privateReadType(stream, apartmentId);
        ElementsArrayType genericTypes;
        uint8 argCount;
        stream.streamReadUint8(argCount);
        genericTypes.changeSize(argCount);
        for (int i = 0; i < argCount; i++)
        {
            genericTypes[i] = privateReadType(stream, apartmentId, 0, false, false, UnresolvedTokenIndex, false);
        }
        return ElementType((CorElementType)type, pointerLevel, isReference, isPinned, paramIsSingleArray, paramClassToken,
                           &genericClass, &genericTypes);
    }

    if (type == ELEMENT_TYPE_SZARRAY)
    {
        CHECK(!paramIsSingleArray);
        return privateReadType(stream, apartmentId, pointerLevel, isReference, isPinned, paramClassToken, true);
    }

    /*
    if (type == ELEMENT_TYPE_ARRAY)
    {
       // TODO! Read the dimintion on the array and bounds
    }
    */

    // Unknown/ready type
    CHECK_FAIL();
}

void ElementType::assertTyperef(const TokenIndex& index)
{
    CHECK(index != UnresolvedTokenIndex); // Usually caused by System.Object or System.String which weren't initialized currectly
    CHECK(EncodingUtils::getTokenTableIndex(getTokenID(index)) != TABLE_TYPEREF_TABLE);
}

void ElementType::assertTyperef() const
{
    if (isObject() && (m_type != ELEMENT_TYPE_STRING))
    {
        assertTyperef(getClassToken());
    }
}

void ElementType::hashElement(cDigest& digest, const ResolverInterface& resolver) const
{
    digest.update(&m_type,          sizeof(m_type));
    digest.update(&m_pointerLevel,  sizeof(m_pointerLevel));
    digest.update(&m_isReference,   sizeof(m_isReference));
    digest.update(&m_isPinned,      sizeof(m_isPinned));
    digest.update(&m_isPinned,      sizeof(m_isPinned));
    digest.update(&m_isSingleArray, sizeof(m_isSingleArray));
    // TODO! Generic
    if (isObject(m_type))
    {
        resolver.getTypeHashSignature(m_classToken);
    }
}

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const ElementType& object)
{
    if (object.m_isReference)
        out << "ref ";

    switch (object.getType())
    {
    case ELEMENT_TYPE_VOID:    out << "void"; break;
    case ELEMENT_TYPE_BOOLEAN: out << "bool"; break;
    case ELEMENT_TYPE_CHAR:    out << "char"; break;
    case ELEMENT_TYPE_I1:      out << "int8"; break;
    case ELEMENT_TYPE_U1:      out << "uint8"; break;
    case ELEMENT_TYPE_I2:      out << "int16"; break;
    case ELEMENT_TYPE_U2:      out << "uint16"; break;
    case ELEMENT_TYPE_I4:      out << "int32"; break;
    case ELEMENT_TYPE_U4:      out << "uint32"; break;
    case ELEMENT_TYPE_I8:      out << "int64"; break;
    case ELEMENT_TYPE_U8:      out << "uint64"; break;
    case ELEMENT_TYPE_R4:      out << "float"; break;
    case ELEMENT_TYPE_R8:      out << "double"; break;
    case ELEMENT_TYPE_STRING:  out << "string"; break;
    case ELEMENT_TYPE_I:       out << "int"; break;
    case ELEMENT_TYPE_U:       out << "uint"; break;
    case ELEMENT_TYPE_OBJECT:  out << "object"; break;
    case ELEMENT_TYPE_CLASS:   out << "class"; break;
    case ELEMENT_TYPE_VAR:     out << "var"; break;
    case ELEMENT_TYPE_VALUETYPE:   out << "value class"; break;
    case ELEMENT_TYPE_GENERICINST: out << "generic instance"; break;
    default:
        CHECK_FAIL();
    }

    for (uint i = 0; i < object.m_pointerLevel; i++)
    {
        out << "*";
    }

    if ((object.getType() == ELEMENT_TYPE_CLASS) ||
        (object.getType() == ELEMENT_TYPE_VALUETYPE) ||
        (object.getType() == ELEMENT_TYPE_VAR))
    {
        // Print the class type
        out << HEXTOKEN(object.getClassToken());
    }

    if (object.getType() == ELEMENT_TYPE_GENERICINST)
    {
        out << " " << HEXTOKEN(object.getGenericClass().getClassToken()) << " <";
        for (uint i = 0; i < object.getGenericTypes().getSize(); i++)
        {
            out << object.getGenericTypes()[i];
            if (i < object.getGenericTypes().getSize() - 1)
                out << ", ";
        }
        out << ">";
    }

    if (object.isSingleDimensionArray())
        out << "[]";

    return out;
}

cStringerStream& operator << (cStringerStream& out, const TokenIndex& tokenIndex)
{
    out << getApartmentID(tokenIndex) << ":" << HEXDWORD(getTokenID(tokenIndex));
    return out;
}

#endif // TRACED_CLR

uint cHashFunction(const TokenIndex& index, uint range)
{
    return ((getApartmentID(index) << 16) + getTokenID(index)) % range;
}

cString HEXTOKEN(const TokenIndex& tokenIndex)
{
    return cString(getApartmentID(tokenIndex)) + ":" + HEXDWORD(getTokenID(tokenIndex));
}
