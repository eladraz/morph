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

#ifndef __TBA_CLR_RUNNABLE_STRINGREPOSITORY_H
#define __TBA_CLR_RUNNABLE_STRINGREPOSITORY_H

/*
 * StringRepository.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "xStl/data/hash.h"
#include "xStl/data/counter.h"
#include "xStl/data/string.h"
#include "data/ElementType.h"
#include "runnable/ResolverInterface.h"
#include "runnable/MemoryLayoutInterface.h"
#include "runnable/Apartment.h"
#include "format/signatures/MethodDefOrRefSignature.h"

// Forward deceleration
class GlobalContext;


/*
 * Holds string repository as unicode/ascii strings with lengths
 */
class StringRepository
{
public:
    /*
     * Return the string repository
     */
    const cBuffer& getAsciiStringRepository() const;
    const cBuffer& getUnicodeStringRepository() const;

    /*
     * Get the length of a string.
     * Unicode strings is twice the size of ascii string
     *
     * String constructor doesn't get size in bytes
     */
    //uint getAsciiStringLength(const TokenIndex& stringToken);
    //uint getUnicodeStringLength(const TokenIndex& stringToken);
    uint getStringLength(const TokenIndex& stringToken);
    uint getStringOffset(const TokenIndex& stringToken);

    /*
     * From a dependancy, get the index inside the string buffer
     */
    bool deserializeStringA(const cString& dependency,
                            uint& stringOffset,
                            TokenIndex* tokenIndex = NULL);
    bool deserializeStringU(const cString& dependency,
                            uint& stringOffset,
                            TokenIndex* tokenIndex = NULL);

    /*
     * Translate char[] into a string table, with data convertion (to ascii)
     */
    cString serializeString(cForkStreamPtr& string, uint length, const TokenIndex& tokenInfo);

    /*
     * From a tokenIndex, get a mangaled name for the string repository
     */
    static cString serializeString(const TokenIndex& stringToken);

    /*
     * Get string token
     */
    static bool deserializeStringToken(const cString& name, TokenIndex& stringToken);

    /*
     * Return true if the string is a unicode string
     */
    // bool isStringUnicode(const TokenIndex& stringToken);

private:
    friend class GlobalContext;
    // Deny copy-constructore
    StringRepository(const StringRepository& other);

    /*
     * Construct new TypedefRepository object
     *
     * apartment - The apartment object
     */
    StringRepository(const ApartmentPtr& apartment,
                     const MemoryLayoutInterface& memoryLayoutInterface);


    /*
     * Parse a token, return ElementType::Unrsolved if string is invalid.
     */
    static TokenIndex getToken(const cString& dependency);

    /*
     * Add a new string into repository
     */
    void lockAppendString(const TokenIndex& stringToken);
    void lockAppendString(const cString& str, const TokenIndex& stringToken);

    // The apartment and memory layout
    mutable ApartmentPtr m_apartment;
    const MemoryLayoutInterface& m_memoryLayout;

    // The lockable object
    mutable cXstlLockable m_lock;

    // The string table, holds the it's offset. m_a = offset,  m_b = length
    #define stringRepoStringOffset(x) ((x).m_a)
    #define stringRepoStringLength(x) ((x).m_b)
    cHash<TokenIndex, cDualElement<uint,uint> > m_asciiStringTable;
    cHash<TokenIndex, cDualElement<uint,uint> > m_unicodeStringTable;


    // The string repository
    cBuffer m_asciiStringRepository;
    cBuffer m_unicodeStringRepository;

    // The serialized string prefix
    static const char gCILStringPrefix[];
};

#endif // __TBA_CLR_RUNNABLE_STRINGREPOSITORY_H
