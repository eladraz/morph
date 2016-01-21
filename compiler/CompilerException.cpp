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

#include "compiler/stdafx.h"
#include "xStl/types.h"
#include "xStl/stream/ioStream.h"
#include "compiler/CompilerException.h"
#include "dismount/assembler/DependencyException.h"

static const lpString COMPILER_EXCEPTION = XSTL_STRING("Compiler exception");

CompilerException::CompilerException(char * file, uint32 line,
                                     const cString& methodName,
                                     const cException* innerException) :
    cException(file, line, NULL, 0),
    m_methodName(methodName),
    m_innerException(NULL)
{
    if (innerException != NULL)
    {
        m_innerException = innerException->clone();
    }
}

CompilerException::~CompilerException()
{
    if (m_innerException != NULL)
        delete m_innerException;
}

const character* CompilerException::getMessage()
{
    return COMPILER_EXCEPTION;
}

void CompilerException::print()
{
    cException::print();

    cString faultString("Exception when compiling function: ");
    faultString+= m_methodName;
    faultString+= endl;

    TRACE(TRACE_VERY_HIGH, faultString);
    cerr << faultString.getBuffer();

    if (m_innerException != NULL)
    {
        m_innerException->print();
    }
}

