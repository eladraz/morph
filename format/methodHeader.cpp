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
 * methodHeader.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "format/coreHeadersTypes.h"
#include "format/methodHeader.h"

MethodHeader::MethodHeader(const cForkStreamPtr& stream) :
    m_stream(stream->fork()),
    m_locals(NO_LOCALS),
    m_startAddress(0),
    m_initLocals(true)
{
    // Read the tiny format
    MethodHeader::TinyFormatHeader tinyHeader;
    // NOTE: The header is packed
    m_stream->pipeRead(&tinyHeader.m_packed, sizeof(tinyHeader.m_packed));
    m_startAddress = m_stream->getPointer();
    switch(tinyHeader.m_bits.flags)
    {
    case CorILMethod_TinyFormat:
        // The tiny format read
        m_length = tinyHeader.m_bits.codeSize;
        return;
    case CorILMethod_FatFormat:
        break;
    default:
        // Function format 1: Unknown.
        CHECK_FAIL();
    }

    // The function format is FAT format
    MethodHeader::FatFormatHeader fatHeader;
    memset(&fatHeader, 0, sizeof(fatHeader));
    // NOTE: 8-bits of the flags has already read.
    m_stream->pipeRead((uint8*)(&fatHeader)+1, sizeof(fatHeader)-1);
    fatHeader.flags |= tinyHeader.m_packed;

    // Fixup the code-size
    m_length = fatHeader.codeSize;
    m_locals = fatHeader.localVarSigTok;

    // m_stream is now points to function's body.
    m_startAddress = m_stream->getPointer();

    // Remember the "init locals" flag
    m_initLocals = ((fatHeader.flags & CorILMethod_InitLocals) != 0);

    // Test for exception-handling block
    if (fatHeader.flags & CorILMethod_MoreSects)
    {
        // Go to method data section.
        // This section bound to 4 bytes position.
        uint ehpos = m_startAddress + fatHeader.codeSize;
        ehpos = (ehpos + 3) & (~3);
        m_stream->seek(ehpos, basicInput::IO_SEEK_SET);

        // Read the section type
        uint8 sectionType;
        m_stream->streamReadUint8(sectionType);
        while ((sectionType & CorILMethod_Sect_EHTable) !=
                              CorILMethod_Sect_EHTable)
        {
            // Read unknown section
            CHECK_FAIL();
            m_stream->streamReadUint8(sectionType);
        }

        FatExceptionHandlingClause mainClause;
        if ((sectionType & CorILMethod_Sect_FatFormat) == 0)
        {
            // Read small exception-handling block

            // Size of the data for the block, including the header, say n*12+4.
            uint8 dataSize; uint16 padding;
            m_stream->streamReadUint8(dataSize);
            m_stream->streamReadUint16(padding);

            uint count = dataSize - 4;
            count = count / sizeof(MethodHeader::TinyExceptionHandlingClause);

            for (uint i = 0; i < count; i++)
            {
                MethodHeader::TinyExceptionHandlingClause tinyClause;
                m_stream->pipeRead(&tinyClause, sizeof(tinyClause));
                mainClause.flags = tinyClause.flags;
                mainClause.tryOffset = tinyClause.tryOffset;
                mainClause.tryLength = tinyClause.tryLength;
                mainClause.handlerOffset = tinyClause.handlerOffset;
                mainClause.handlerLength = tinyClause.handlerLength;
                mainClause.classTokenOrFilterOffset =
                    tinyClause.classTokenOrFilterOffset;

                // And append EH token
                m_exceptions.append(mainClause);
            }

        } else
        {
            // Size of the data for the block, including the header, say n*24+4.
            uint32 dataSize = 0; uint8 temp;
            m_stream->streamReadUint8(temp); dataSize = temp;
            m_stream->streamReadUint8(temp); dataSize+= (temp << 8);
            m_stream->streamReadUint8(temp); dataSize+= (temp << 16);

            uint count = dataSize - 4;
            count = count / sizeof(MethodHeader::FatExceptionHandlingClause);

            for (uint i = 0; i < count; i++)
            {
                m_stream->pipeRead(&mainClause, sizeof(mainClause));
                m_exceptions.append(mainClause);
            }
        }

        // Return to the function body
        m_stream->seek(m_startAddress, basicInput::IO_SEEK_SET);
    }

    boubbleSort(m_exceptions.begin(), m_exceptions.end());
}

const cForkStreamPtr& MethodHeader::getFunction() const
{
    return m_stream;
}

uint MethodHeader::getFunctionLength() const
{
    return m_length;
}

mdToken MethodHeader::getLocals() const
{
    return m_locals;
}

bool MethodHeader::isInitLocals() const
{
    return m_initLocals;
}

const MethodHeader::ExceptionHandlingClauseList&
                    MethodHeader::getExceptionsHandlers() const
{
    return m_exceptions;
}

uint MethodHeader::getMethodStreamStartAddress() const
{
    return m_startAddress;
}

bool MethodHeader::isOffsetInTryBlock(const ExceptionHandlingClause& block,
                                      uint offset)
{
    return (block.tryOffset <= offset) &&
           ((block.tryOffset + block.tryLength) <= offset);
}


MethodHeader::ExceptionHandlingClause::ExceptionHandlingClause()
{
}

// We wish to order so that the protected block that ends later will be first
bool MethodHeader::ExceptionHandlingClause::operator> (const ExceptionHandlingClause& other) const
{
    return (tryOffset + tryLength) < (other.tryOffset + other.tryLength);
}

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const MethodHeader& object)
{
    cBuffer functionContent;
    object.getFunction()->fork()->pipeRead(functionContent,
                                           object.getFunctionLength());
    out << "Length: " << HEXNUMBER(object.m_length) << endl;
    out << "Locals: " << HEXDWORD(object.m_locals) << endl;
    DATA dump(functionContent.begin(), functionContent.end());
    out << dump << endl;
    return out;
}
#endif // TRACED_CLR

