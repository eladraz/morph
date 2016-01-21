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
 * EncodingUtils.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/assert.h"
#include "xStl/except/trace.h"
#include "format/metadataStream.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"

uint EncodingUtils::readCompressedNumber(basicInput& stream)
{
    // The number is encode in MSB format since the most-significant bit used
    // to determine the encoding.
    uint8 firstByte;
    stream.streamReadUint8(firstByte);
    // If the high-bit is clear than the value is 7-bit value.
    if ((firstByte & 0x80) == 0)
        return firstByte;

    // Read the second byte
    uint8 secondByte;
    stream.streamReadUint8(secondByte);

    // Otherwise test whether this value is encode in 2 bytes or 4 bytes...
    if ((firstByte & 0x40) == 0)
    {
        // Bit 14 is clear. value held in bits 0..13   (Mask: 0x3FFF)
        return (((firstByte & 0x3F) << 8) | secondByte);
    }

    // Bit 9 must be cleared
    CHECK((firstByte & 0x20) == 0);

    uint8 thirdByte;
    stream.streamReadUint8(thirdByte);
    uint8 fourthByte;
    stream.streamReadUint8(fourthByte);

    // 28 bit number
    return ((firstByte & 0x1F) << 24) |
            (secondByte << 16) |
            (thirdByte << 8) |
            fourthByte;
}

uint EncodingUtils::getTokenTableIndex(mdToken token)
{
    return (token >> 24) & 0xFF;
}

uint EncodingUtils::getTokenPosition(mdToken token)
{
    return token & 0xFFFFFF;
}

mdToken EncodingUtils::buildToken(uint tableID, uint tableRow)
{
    ASSERT(tableID < 0xFF);
    ASSERT(tableRow > 0);
    ASSERT(tableRow < 0xFFFFFF);

    return ((tableID & 0xFF) << 24) | tableRow;
}

// mdToken EncodingUtils::getNilToken()
// {
//     return 0;
// }

//////////////////////////////////////////////////////////////////////////
// Unpacking stored value

mdToken EncodingUtils::unpackTypeDefOrRefToken(mdToken value)
{
    // The lower 2 bits are the table ID
    uint tableID = value & 3;
    uint tablePos = value >> 2;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_TYPEDEF_TABLE; break;
    case 1: tableID = TABLE_TYPEREF_TABLE; break;
    case 2: tableID = TABLE_TYPESPEC_TABLE; break; // TODO!
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackHasConstantToken(mdToken value)
{
    // The lower 2 bits are the table ID
    uint tableID = value & 3;
    uint tablePos = value >> 2;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_FIELD_TABLE; break;
    case 1: tableID = TABLE_PARAM_TABLE; break;
    case 2: tableID = TABLE_PROPERTY_TABLE; CHECK_FAIL(); break; // TODO!
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackMethodDefOrRefToken(mdToken value)
{
    // The lower bit is the table ID
    uint tableID = value & 1;
    uint tablePos = value >> 1;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_METHOD_TABLE; break;
    case 1: tableID = TABLE_MEMBERREF_TABLE; break;
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackTypeOrMethodDefToken(mdToken value)
{
    // The lower bit is the table ID
    uint tableID = value & 1;
    uint tablePos = value >> 1;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_TYPEDEF_TABLE; break;
    case 1: tableID = TABLE_METHOD_TABLE; break;
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackHasSemanticsToken(mdToken value)
{
    // The lower bit is the table ID
    uint tableID = value & 1;
    uint tablePos = value >> 1;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_EVENT_TABLE; break;
    case 1: tableID = TABLE_PROPERTY_TABLE; break;
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackHasCustomAttributeToken(mdToken value)
{
    // The lower 5 bits are the table ID
    uint tableID = value & 0x1F;
    uint tablePos = value >> 5;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0:  tableID = TABLE_METHOD_TABLE; break;
    case 1:  tableID = TABLE_FIELD_TABLE; break;
    case 2:  tableID = TABLE_TYPEREF_TABLE; break;
    case 3:  tableID = TABLE_TYPEDEF_TABLE; break;
    case 4:  tableID = TABLE_PARAM_TABLE; break;
    case 5:  tableID = TABLE_INTERFACEIMPL_TABLE; break;
    case 6:  tableID = TABLE_MEMBERREF_TABLE; break;
    case 7:  tableID = TABLE_MODULE_TABLE; break;
    case 8:  tableID = TABLE_DECLSECURITY_TABLE; break;
    case 9:  tableID = TABLE_PROPERTY_TABLE; break;
    case 10: tableID = TABLE_EVENT_TABLE; break;
    case 11: tableID = TABLE_STANDALONGESIG_TABLE; break;
    case 12: tableID = TABLE_MODULEREF_TABLE; break;
    case 13: tableID = TABLE_TYPESPEC_TABLE; break;
    case 14: tableID = TABLE_ASSEMBLY_TABLE; break;
    case 15: tableID = TABLE_ASSEMBLYREF_TABLE; break;
    case 16: tableID = TABLE_FILE_TABLE; break;
    case 17: tableID = TABLE_EXPORTTYPE_TABLE; break;
    case 18: tableID = TABLE_MANIFESTRESOURCE_TABLE; break;
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackCustomAttributeTypeToken(mdToken value)
{
    // The lower 3 bits are the table ID
    uint tableID = value & 7;
    uint tablePos = value >> 3;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 2: tableID = TABLE_METHOD_TABLE; break;
    case 3: tableID = TABLE_MEMBERREF_TABLE; break;
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackHasDeclSecurityToken(mdToken value)
{
    // The lower 2 bits are the table ID
    uint tableID = value & 3;
    uint tablePos = value >> 2;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_TYPEDEF_TABLE; break;
    case 1: tableID = TABLE_METHOD_TABLE; break;
    case 2: tableID = TABLE_ASSEMBLY_TABLE; break;
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackResolutionScopeToken(mdToken value)
{
    // TODO!!!

    // The lower 3 bits are the table ID
    uint tableID = value & 3;
    uint tablePos = value >> 2;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 0: tableID = TABLE_MODULE_TABLE; break;
    case 1: tableID = TABLE_MODULEREF_TABLE; break;
    case 2: tableID = TABLE_ASSEMBLYREF_TABLE; break;
    case 3: tableID = TABLE_TYPEREF_TABLE; break;
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}

mdToken EncodingUtils::unpackMemberRefParentToken(mdToken value)
{
    // The lower 3 bits are the table ID
    uint tableID = value & 7;
    uint tablePos = value >> 3;

    // Check that the table-position is valid
    CHECK(tablePos <= 0xFFFFFF);

    switch (tableID)
    {
    case 1: tableID = TABLE_TYPEREF_TABLE; break;
    case 2: tableID = TABLE_MODULEREF_TABLE; break;
    case 3: tableID = TABLE_METHOD_TABLE; break;
    case 4: tableID = TABLE_TYPESPEC_TABLE; break;
    default: CHECK_FAIL();
    }

    return (tableID << 24) | tablePos;
}
