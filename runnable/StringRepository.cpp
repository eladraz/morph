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
 * StringRepository.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/os/lock.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"
#include "xStl/parser/parser.h"
#include "xStl/enc/digest/Crc16.h"
#include "data/exceptions.h"
#include "data/ElementType.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/Table.h"
#include "runnable/RunnableTrace.h"
#include "runnable/ResolverInterface.h"
#include "runnable/TypedefRepository.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "runnable/CorlibNames.h"
#include "runnable/StringRepository.h"

#define PAGE_SIZE 4*1024

const char StringRepository::gCILStringPrefix[] = "?CIL?STD?STRING?";


StringRepository::StringRepository(const ApartmentPtr& apartment,
                                   const MemoryLayoutInterface& memoryLayoutInterface) :
    m_apartment(apartment),
    m_memoryLayout(memoryLayoutInterface),
    m_asciiStringRepository((uint)0, PAGE_SIZE),
    m_unicodeStringRepository((uint)0, sizeof(unichar) * PAGE_SIZE)
{
}

const cBuffer& StringRepository::getAsciiStringRepository() const
{
    return m_asciiStringRepository;
}

const cBuffer& StringRepository::getUnicodeStringRepository() const
{
    return m_unicodeStringRepository;
}

/*
uint StringRepository::getAsciiStringLength(const TokenIndex& stringToken)
{
    cLock lock(m_lock);
    lockAppendString(stringToken);
    return stringRepoStringLength(m_asciiStringTable[stringToken]);
}

uint StringRepository::getUnicodeStringLength(const TokenIndex& stringToken)
{
    cLock lock(m_lock);
    lockAppendString(stringToken);
    return stringRepoStringLength(m_unicodeStringTable[stringToken]);
}
*/
uint StringRepository::getStringLength(const TokenIndex& stringToken)
{
    cLock lock(m_lock);
    lockAppendString(stringToken);
    return stringRepoStringLength(m_asciiStringTable[stringToken]);
}

uint StringRepository::getStringOffset(const TokenIndex& stringToken)
{
    cLock lock(m_lock);
    lockAppendString(stringToken);
    return stringRepoStringOffset(m_asciiStringTable[stringToken]);
}

cString StringRepository::serializeString(const TokenIndex& stringToken)
{
    cString relocationTokenName(gCILStringPrefix);
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getApartmentID(stringToken));
    relocationTokenName+= '-';
    relocationTokenName+= "0x";
    relocationTokenName+= HEXDWORD(getTokenID(stringToken));
    return relocationTokenName;
}

cString StringRepository::serializeString(cForkStreamPtr& string, uint length, const TokenIndex& stringToken)
{
    if (!m_asciiStringTable.hasKey(stringToken))
    {
        cLock lock(m_lock);
        cString str(string->readFixedSizeString(length / 2, CLR_FORMAT_WCHAR_SIZE));
        lockAppendString(str, stringToken);
    }

    return serializeString(stringToken);
}

bool StringRepository::deserializeStringToken(const cString& name, TokenIndex& stringToken)
{
    stringToken = getToken(name);
    return stringToken != ElementType::UnresolvedTokenIndex;
}

TokenIndex StringRepository::getToken(const cString& dependency)
{
    uint len = arraysize(gCILStringPrefix) - 1;
    if (dependency.left(len) != gCILStringPrefix)
        return ElementType::UnresolvedTokenIndex;

    TokenIndex stringToken = ElementType::UnresolvedTokenIndex;
    cSArray<char> ascii = dependency.part(len, MAX_UINT32).getASCIIstring();
    Parser parser(ascii.getBuffer(), ascii.getBuffer(), ascii.getSize() - 1, 0);
    getApartmentID(stringToken) = parser.readCUnsignedInteger();
    CHECK(parser.readChar() == '-');
    getTokenID(stringToken) = parser.readCUnsignedInteger();
    return stringToken;
}

bool StringRepository::deserializeStringA(const cString& dependency,
                                          uint& stringOffset,
                                          TokenIndex* tokenIndex)
{
    TokenIndex stringToken = getToken(dependency);
    if (stringToken == ElementType::UnresolvedTokenIndex)
        return false;

    if (tokenIndex)
        *tokenIndex = stringToken;

    cLock lock(m_lock);
    lockAppendString(stringToken);
    stringOffset = stringRepoStringOffset(m_asciiStringTable[stringToken]);
    return true;
}


bool StringRepository::deserializeStringU(const cString& dependency,
                                          uint& stringOffset,
                                          TokenIndex* tokenIndex)
{
    TokenIndex stringToken = getToken(dependency);
    if (stringToken == ElementType::UnresolvedTokenIndex)
        return false;

    if (tokenIndex)
        *tokenIndex = stringToken;

    cLock lock(m_lock);
    lockAppendString(stringToken);
    stringOffset = stringRepoStringOffset(m_unicodeStringTable[stringToken]);
    return true;
}

void StringRepository::lockAppendString(const TokenIndex& stringToken)
{
    if (m_asciiStringTable.hasKey(stringToken))
        return;

    // Read string
    cForkStreamPtr stream = m_apartment->getApartmentByID(getApartmentID(stringToken))->getStreams().getUserStringsStream()->fork();
    stream->seek(EncodingUtils::getTokenPosition(getTokenID(stringToken)), basicInput::IO_SEEK_SET);
    uint length = EncodingUtils::readCompressedNumber(*stream);
    cString str(stream->readFixedSizeString((length - 1) / 2, CLR_FORMAT_WCHAR_SIZE));

    lockAppendString(str, stringToken);
}

void StringRepository::lockAppendString(const cString& str, const TokenIndex& stringToken)
{
    // Get length of string.
    uint alength = str.length();
    uint ulength = alength * sizeof(unichar);
    uint apos = m_asciiStringRepository.getSize();
    uint upos = m_unicodeStringRepository.getSize();

    cSArray<unichar> uarr = str.getUnicodeString();
    cSArray<char>    aarr = str.getASCIIstring();


    // Append and remove the null-terminate string
    m_asciiStringRepository.changeSize(apos + m_memoryLayout.align(alength));       // Align!
    m_unicodeStringRepository.changeSize(upos + m_memoryLayout.align(ulength));     // Align!
    cOS::memcpy(m_asciiStringRepository.getBuffer() + apos, aarr.getBuffer(), alength);
    cOS::memcpy(m_unicodeStringRepository.getBuffer() + upos, uarr.getBuffer(), ulength);

    m_asciiStringTable.append(stringToken, cDualElement<uint,uint>(apos, alength));
    m_unicodeStringTable.append(stringToken, cDualElement<uint,uint>(upos, ulength));
}
