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

#ifndef __TBA_CLR_DATA_EXCEPTIONS_H
#define __TBA_CLR_DATA_EXCEPTIONS_H

/*
* exceptions.h
*
* List of all CLR runtime exceptions
*
* Author: Elad Raz <e@eladraz.com>
*/
#include "xStl/types.h"
#include "xStl/exceptions.h"

// Base exception class
class ClrRuntimeException : public cException
{
public:
    ClrRuntimeException(char * file, uint32 line, const character* msg = NULL) : cException(file, line, msg) {};
};

// Thrown when a given token for method/type/filed/const is mismatch or invalid
class ClrInvalidToken : public ClrRuntimeException
{
public:
    ClrInvalidToken(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Thrown when a given token for method is missing
class ClrMissingMethodException : public ClrInvalidToken
{
public:
    ClrMissingMethodException(char * file, uint32 line, const character* msg = NULL) : ClrInvalidToken(file, line, msg) {};
};

// Thrown when overflow occured
// See Engine
class ClrOverflowException : public ClrRuntimeException
{
public:
    ClrOverflowException(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Thrown when extern method or type was access and the runtime doesn't have
// information regarding that type.
// See class Resolver
class ClrImportException : public ClrRuntimeException
{
public:
    ClrImportException(char * file, uint32 line, const character* msg = NULL);
};

// The variable mismatch it's type. Invoked by Variable class when the access
// variable routine is invalid, or cast should be specificly invoked.
class ClrMismatchVariable : public ClrRuntimeException
{
public:
    ClrMismatchVariable(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Throw by the FunctionExecuter class to indicate that the given arguments
// type are not matched to the function description or the number of argument
// is incorrect
class ClrMismatchArguments : public ClrRuntimeException
{
public:
    ClrMismatchArguments(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Throw by the Engine to indicate that the opcode is invalid
class ClrIllegalInstruction : public ClrRuntimeException
{
public:
    ClrIllegalInstruction(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Throw when server doesn't support floating-point operations.
class ClrFloatingPointEngineNotFound : public ClrRuntimeException
{
public:
    ClrFloatingPointEngineNotFound(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Error at stack variable
class ClrStackError : public ClrRuntimeException
{
public:
    ClrStackError(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Thrown when invalid explicit or implicit cast occured.
class ClrInvalidCastException : public ClrRuntimeException
{
public:
    ClrInvalidCastException(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

//////////////////////////////////////////////////////////////////////////
// Real exceptions classes

// Dividing by zero exception
// Translated into: System.DivideByZeroException
class ClrDividingByZero : public ClrRuntimeException
{
public:
    ClrDividingByZero(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

// Thrown when accessing null-pointer object
// Translated into: System.NullReferenceException
class ClrNullReferenceException : public ClrRuntimeException
{
public:
    ClrNullReferenceException(char * file, uint32 line, const character* msg = NULL) : ClrRuntimeException(file, line, msg) {};
};

#endif // __TBA_CLR_DATA_EXCEPTIONS_H
