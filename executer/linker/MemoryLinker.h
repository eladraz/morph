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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_MEMORYLINKER_H
#define __TBA_CLR_EXECUTER_RUNTIME_MEMORYLINKER_H

/*
 * MemoryLinker.h
 *
 * In-memory linker for COM-style format
 *
 * Author: Pavel Ferencz
 */
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/linker/LinkerInterface.h"

class MemoryLinker : public LinkerInterface
{
public:
    MemoryLinker(CompilerEngineThread& compilerEngineThread, ApartmentPtr apartment);

    /*
     * Scan a method and all-of it sub-method and resolve all connections.
     */
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
    // Disable copy construction
    MemoryLinker(const MemoryLinker& other);
    MemoryLinker& operator=(const MemoryLinker& other);

    /*
     * Recursive method which replace all dependencies.
     */
    void resolve(MethodStackObject& stack,
                 MethodResolvedObject& resolved);
    void resolve(const TokenIndex& methodIndex);

    /*
     * Allocate all memory needed for the sections
     */
    void allocate();

    /*
     * Copies all the functions that were previously processed using bind() to the
     * designated memory area.
     * A region of appropriate size with execute permissions is allocated for that purpose.
     */
    void relocate();


    // The strings
    cBuffer m_stringTable;
    // Static data
    cBuffer m_staticTable;
    cBuffer m_staticDataTable;
    // Virtual table
    cBuffer m_vtblBuffer;

    cHash<addressNumericValue, addressNumericValue> m_reloc;
    void *m_area;
    int  m_offset;
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_MEMORYLINKER_H