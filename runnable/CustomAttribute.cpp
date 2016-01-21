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

#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"

#include "runnable/CustomAttribute.h"
#include "runnable/StringReader.h"
#include "runnable/GlobalContext.h"
#include "runnable/ClrResolver.h"

#include "format/tables/MethodTable.h"
#include "format/tables/CustomAttributeTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/ParamTable.h"
#include "format/EncodingUtils.h"

#include "compiler/CallingConvention.h"

CustomAttribute::CustomAttribute()
{
}

CustomAttribute::CustomAttribute(const cString& name):
    m_name(name)
{
}

CustomAttribute::~CustomAttribute()
{
}

const cString& CustomAttribute::getName() const
{
    return m_name;
}

void CustomAttribute::getArguments(CustomAttributeArguments& outArguments) const
{
    outArguments = m_arguments;
}

CustomAttributeArgument& CustomAttribute::getArgument(const cString& name) const
{
    CustomAttributeArguments::iterator iterator = m_arguments.begin();
    CustomAttributeArguments::iterator endIterator = m_arguments.end();

    for (;iterator != endIterator; iterator++) {
        CustomAttributeArgument& argument = *(*iterator);
        if (name == argument.getName()) {
            return argument;
        }
    }

    XSTL_THROW(CustomAttribute::MissingArgument);
}

CustomAttributeArgumentPtr CustomAttribute::addArgument(const cString& name, const cString& value)
{
    CustomAttributeArgumentPtr ca(new CustomAttributeStringArgument(name, value));
    m_arguments.append(ca);
    return ca;
}

CustomAttributeArgumentPtr CustomAttribute::addArgument(const cString& name, const int value)
{
    CustomAttributeArgumentPtr ca(new CustomAttributeSignedIntegerArgument(name, value));
    m_arguments.append(ca);
    return ca;
}

CustomAttributeArgumentPtr CustomAttribute::addArgument(const cString& name, const unsigned int value)
{
    CustomAttributeArgumentPtr ca(new CustomAttributeUnsignedIntegerArgument(name, value));
    m_arguments.append(ca);
    return ca;
}

CustomAttributeArgumentPtr CustomAttribute::addArgument(const cString& name, const bool value)
{
    CustomAttributeArgumentPtr ca(new CustomAttributeBoolArgument(name, value));
    m_arguments.append(ca);
    return ca;
}

CustomAttributePtr CustomAttribute::factory(mdToken owner,
                                            const Apartment& apartment,
                                            const Apartment& caApartment,
                                            const CustomAttributeTable& customAttributeTable,
                                            const MethodTable& ctorMethodTable,
                                            const TypedefTable& ctorMethodTypeTable)
{
    // Create custom attribute object
    cString customAttributeName = StringReader::readStringName(apartment.getStreams().getStringsStream(), ctorMethodTypeTable.getName());
    CustomAttributePtr cAttribPtr = CustomAttributePtr(new CustomAttribute(customAttributeName));

    cForkStreamPtr blob = caApartment.getStreams().getBlobStream()->fork();
    blob->seek(customAttributeTable.getBlobIndex(), basicInput::IO_SEEK_SET);

    // Create CustomAtribbuteValues (a container class that helps parsing ca values).
    mdToken cavLength = EncodingUtils::readCompressedNumber(*blob);
    CustomAttributeValuesPtr customAttribValuesPtr = CustomAttributeValuesPtr(new CustomAttributeValues(blob, cavLength));

    // Read ctor signature to get the types of arguments.
    // Use paramToken to get more details.
    MethodDefOrRefSignaturePtr signature = CallingConvention::readMethodSignature(apartment, ctorMethodTable.getToken());
    ElementsArrayType::iterator iterator = signature->getParams().begin();
    ElementsArrayType::iterator endIterator = signature->getParams().end();
    mdToken paramToken = ctorMethodTable.getParamListIndex();

    // Go over ctor arguments
    for (;iterator != endIterator; iterator++) {
        ElementType elementType = *iterator;

        // Find param table
        TablePtr tablePtr = apartment.getTables().getTableByToken(paramToken);
        const ParamTable& paramTable = (const ParamTable&) *tablePtr;

        cString argName = StringReader::readStringName(apartment.getStreams().getStringsStream(), paramTable.getName());

        switch(elementType.getType())
        {
        case ELEMENT_TYPE_STRING:
            {
                mdToken strLength = EncodingUtils::readCompressedNumber(*blob);
                cBuffer buffer = cBuffer(strLength + 1);
                customAttribValuesPtr->getNextValue(buffer.getBuffer(), strLength);
                buffer[strLength] = '\0';
                cString value = cString((char *) buffer.getBuffer());
                cAttribPtr->addArgument(argName, value);
            }
            break;

        case ELEMENT_TYPE_I4:
            {
                int value;
                customAttribValuesPtr->getNextValue(&value, sizeof(value));
                cAttribPtr->addArgument(argName, value);
            }
            break;

        case ELEMENT_TYPE_U4:
            {
                unsigned int value;
                customAttribValuesPtr->getNextValue(&value, sizeof(value));
                cAttribPtr->addArgument(argName, value);
            }
            break;

        case ELEMENT_TYPE_BOOLEAN:
            {
                bool value;
                customAttribValuesPtr->getNextValue(&value, sizeof(value));
                cAttribPtr->addArgument(argName, value);
            }
            break;

        default:
            XSTL_THROW(ClrRuntimeException);
        }

        // Next parameter please.
        paramToken = EncodingUtils::buildToken(EncodingUtils::getTokenTableIndex(paramToken), EncodingUtils::getTokenPosition(paramToken) + 1);
    }

    return cAttribPtr;
}

void CustomAttribute::getAttributes(const ApartmentPtr& apartment, mdToken methodToken, const cString& name, CustomAttributes& outAttributes)
{
    // Fetch all the custom attribute descriptors whose "parent" field points to method.
    RowTablesPtr customAttributeTables = apartment->getTables().byTableID(TABLE_CUSTOMATTRIBUTE_TABLE);
    RowTablesPtr::iterator iterator = customAttributeTables.begin();
    RowTablesPtr::iterator endIterator = customAttributeTables.end();
    for (;iterator != endIterator; iterator++) {

        // Get custom attribute table.
        const TablePtr tablePtr = *iterator;
        const CustomAttributeTable& caTable = (CustomAttributeTable &) (*tablePtr);

        if (methodToken != caTable.getParent())
        {
            // The "parent" field of custom attribute doesn't point to applied method.
            continue;
        }

        // Okay, parent matches.

        // The "type" field of custom attribute points to memberref ctor method.
        TokenIndex ctorToken = ClrResolver::resolve((ApartmentPtr&)apartment, buildTokenIndex(apartment->getUniqueID(), caTable.getType()));
        // No class exists
        if (ctorToken == ElementType::UnresolvedTokenIndex)
            continue;

        ApartmentPtr apt(((ApartmentPtr&)apartment)->getApartmentByID(getApartmentID(ctorToken)));
        TokenIndex typeToken = buildTokenIndex(apt->getUniqueID(),
                                               apt->getTables().getTypedefParent(getTokenID(ctorToken)));

        // Get name
        cString typeName = apt->getObjects().getTypesNameRepository().getTypeName(typeToken);

        if (typeName == name)
        {

            const TypedefTable& typedefTable = (const TypedefTable&)(*apt->getTables().getTableByToken(getTokenID(typeToken)));
            const MethodTable& ctorMethodTable = (const MethodTable&)(*apt->getTables().getTableByToken(getTokenID(ctorToken)));

        // Build CustomAttribute
            CustomAttributePtr attribute = CustomAttribute::factory(methodToken, *apt, *apartment,
                                                                    caTable, ctorMethodTable, typedefTable);
        outAttributes.append(attribute);
    }
}
}

void CustomAttribute::getAttributes(const ApartmentPtr& apartment, mdToken methodToken, CustomAttributes& outAttributes)
{
    return getAttributes(apartment, methodToken, "", outAttributes);
}

#ifdef TRACED_CLR

cStringerStream& operator << (cStringerStream& out, const CustomAttributePtr customAttributePtr)
{
    CustomAttributeArguments arguments;
    customAttributePtr->getArguments(arguments);
    out << "CustomAttribute(" << customAttributePtr->getName() << ", " <<  arguments  << ")";
    return out;
}

cStringerStream& operator << (cStringerStream& out, const CustomAttribute& customAttribute)
{
    CustomAttributeArguments arguments;
    customAttribute.getArguments(arguments);
    out << "CustomAttribute(" << customAttribute.getName() << ", " <<  arguments  << ")";
    return out;
}

#endif // TRACED_CLR

CustomAttributeValues::CustomAttributeValues(cForkStreamPtr streamPtr, mdToken length):
    m_stream(streamPtr),
    m_lengthLeft(length)
{
    testValidity();
}

void CustomAttributeValues::testValidity(void) const
{
    uint16 prolog;
    // XXX: FIX THIS.
    basicInput * bi = (basicInput *) m_stream.getPointer();
    bi->streamReadUint16(prolog);
    if (prolog != m_prologSignature) {
        XSTL_THROW(Invalid);
    }
}

void CustomAttributeValues::getNextValue(void * buffer, uint size)
{
    uint read = 0;
    basicInput * bi = (basicInput *) m_stream.getPointer();

    if (size > m_lengthLeft) {
        XSTL_THROW(OutOfBounds);
    }

    read = bi->pipeRead(buffer, size);
    if (read != size) {
        XSTL_THROW(EndOfStream);
    }

    m_lengthLeft -= read;
}
