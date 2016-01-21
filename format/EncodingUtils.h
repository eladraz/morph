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

#ifndef __TBA_CLR_FORMAT_ENCODINGUTILS_H
#define __TBA_CLR_FORMAT_ENCODINGUTILS_H

/*
 * EncodingUtils.h
 *
 * Provide a set of functions which helps to decode structs, integers and all
 * other Hanpatzot made by the CLR.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/basicIO.h"
#include "format/coreHeadersTypes.h"

/*
 * Provide a set of functions which helps to decode structs, integers and all
 * other Hanpatzot made by the CLR.
 */
class EncodingUtils {
public:
    /*
     * Read a compressed number. The maximum encodable integer is 29 bits long,
     * 0x1FFFFFFF. The compression algorithm used is as follows (bit 0 is the
     * least significant bit)
     *
     *  - If the value lies between 0 (0x00) and 127 (0x7F), inclusive, encode
     *    as a one-byte integer (bit #7 is clear, value held in bits #6 through
     *    #0)
     *  - If the value lies between 2^8 (0x80) and 2^14 – 1 (0x3FFF), inclusive,
     *    encode as a two-byte integer with bit #15 set, bit #14 clear (value
     *    held in bits #13 through #0)
     *  - Otherwise, encode as a 4-byte integer, with bit #31 set, bit #30 set,
     *    bit #29 clear (value held in bits #28 through #0)
     *  - A null string should be represented with the reserved single byte
     *    0xFF, and no following data
     *
     * Return the
     */
    static uint readCompressedNumber(basicInput& stream);

    /*
     * Return the higher 8 bits from a 32bit number. This number represent
     * the table ID.
     *
     * token - A number in the format: TTXXYYZZ, where TT is the table-index
     *
     * Return the TT part.
     */
    static uint getTokenTableIndex(mdToken token);

    /*
     * Return the row position of a table.
     *
     * IMPORTANT NOTE: The indexes are starting from 1 ID and not from 0!
     * This function will not return the offset of the table, but the offset in
     * the table + 1!
     *
     * See getTokenTableIndex
     */
    static uint getTokenPosition(mdToken token);

    /*
     * Build a new token from table ID and row position
     *
     * NOTE: The table row must be between 1 and above. However only assert will
     *       be made to check this role.
     *
     * See getTokenTableIndex
     */
    static mdToken buildToken(uint tableID, uint tableRow);

    /*
     * Return 0, This function return token which is invalid
     */
    // static mdToken getNilToken();
    #define getNilToken() ((mdToken)(0))

    //////////////////////////////////////////////////////////////////////////
    // Unpacking stored value

    /*
     * TypeDef/TypeRef tokenes are encoded as following:
     *    TypeRef  - 01 XXXXXX - Table ID
     *    TypeDef  - 02 YYYYYY - Table ID
     *    TypeSpec - 1B ZZZZZZ - Table ID
     *
     * - Encode the table that this token indexes as the least significant 2
     *   bits. The bit values to use are 0, 1 and 2, specifying the target table
     *   is the TypeDef, TypeRef or TypeSpec table, respectively.
     * - Shift the 3-byte row index (0x000012 in  this example) left by 2 bits
     *   and OR into the 2-bit encoding from step 1.
     *
     * For example:
     * a)  encoded = value for TypeRef table = 0x01 (from 1. above)
     * b)  encoded = ( 0x000012 << 2 ) |  0x01
     *     = 0x48 | 0x01
     *     = 0x49
     *
     * Return a complete token.  (01000012)
     */
    static mdToken unpackTypeDefOrRefToken(mdToken value);

    /*
     * Field/Param/Property hasConstant index token. The encoding is similar to
     * 'TypeDefOrRef', which means that 2 bits encode the table ID. but the ID's
     * 0, 1 and 2 specifying different target tables: Field, Param, Property
     * , respectively.
     *
     * Return a complete token.  (17000002)
     */
    static mdToken unpackHasConstantToken(mdToken value);

    /*
     * MethodDefOrRef read index into MethodDef or MemberRef token. The table ID
     * is encoded in 1 bit which indicate whether the table is MethodDef(0) or
     * MemberRef (1)
     */
    static mdToken unpackMethodDefOrRefToken(mdToken value);

    /*
     * TypeOrMethodDef read index into MethodDef or Typedef token. The table ID
     * is encoded in 1 bit which indicate whether the table is MethodDef(1) or
     * Typedef (0)
     */
    static mdToken unpackTypeOrMethodDefToken(mdToken value);

    /*
     * HasSemantics read index into Event or Property token. The table ID
     * is encoded in 1 bit which indicate whether the table is Event(0) or
     * Property (1)
     */
    static mdToken unpackHasSemanticsToken(mdToken value);

    /*
     * 5 bit encoding tag for almost all tables except CustomAttribute table
     */
    static mdToken unpackHasCustomAttributeToken(mdToken value);

    /*
     * 3 bit encoding tag for MethodDef or MemberRef
     * (TODO! Documentation also has Method notification!)
     */
    static mdToken unpackCustomAttributeTypeToken(mdToken value);

    /*
     * 2 bits encoding tag for TypeDef, MethodDef or Assembly
     */
    static mdToken unpackHasDeclSecurityToken(mdToken value);

    /*
     * 3 bits encoding tag for Module, ModuleRef, AssemblyRef or TypeRef
     */
    static mdToken unpackResolutionScopeToken(mdToken value);

    /*
     * 3 bits encoding tag for TypeRef, ModuleRef, Method or TypeSpec
     */
    static mdToken unpackMemberRefParentToken(mdToken value);
};

#endif // __TBA_CLR_FORMAT_ENCODINGUTILS_H
