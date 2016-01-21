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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_FILELINKER_H
#define __TBA_CLR_EXECUTER_RUNTIME_FILELINKER_H

/*
 * FileLinker.h
 *
 * Link into source file (such as C/Javascript) file
 *
 * Author: Pavel Ferencz
 */
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/linker/LinkerInterface.h"
#include "dismount/assembler/BinaryDependencies.h"

class FileLinker : public LinkerInterface
{
public:
    // Constructor
    FileLinker(CompilerEngineThread& compilerEngineThread, ApartmentPtr apartment);

    // See LinkerInterface::resolveAndExecuteAllDependencies
    virtual void resolveAndExecuteAllDependencies(TokenIndex& mainMethod);

protected:
    /*
     * Bind into a method and fills all the dependencies needed in order to
     * execute the method.
     *
     * Return a numeric value which can be transfer into the 'ExecuteInterface'
     */
    virtual addressNumericValue bind(SecondPassBinary& pass);

private:
    /*
     * Encode a string to a file
     */
    void writeString(basicIO& out, const cString& string);

    /*
     * Return a function
     *
     * shouldAddBody - Set to true if adding all body,
     *                 false if only the header
     */
    void writeFuntionHeaderAndBody(basicIO& out,
                                   const TokenIndex& function,
                                   const SecondPassBinary& bin,
                                   bool shouldAddBody);

    /*
     * Create a new vtbl for token
     */
    void writeVTable(basicIO& out,
                     const TokenIndex& token);

    /*
     * Convert cdecl/stdcall/c++ names into C names
     */
    cString demangleName(const cString& name);

    // Disable copy construction
    FileLinker(const FileLinker& other);
    FileLinker& operator=(const FileLinker& other);
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_FILELINKER_H
