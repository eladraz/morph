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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_LINKERINTERFACE_H
#define __TBA_CLR_EXECUTER_RUNTIME_LINKERINTERFACE_H

/*
* LinkerInterface.h
*
* Interface for linking routines
*/
#include "xStl/types.h"
#include "xStl/data/hash.h"
#include "xStl/data/smartptr.h"
#include "format/coreHeadersTypes.h"
#include "data/ElementType.h"
#include "runnable/Stack.h"
#include "runnable/ResolverInterface.h"
#include "runnable/TypedefRepository.h"
#include "runnable/ClrResolver.h"
#include "executer/MethodIndex.h"
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/runtime/ExternalModuleResolver.h"
#include "dismount/assembler/SecondPassBinary.h"

// Hash function for MethodIndex
uint simpleHashFunction(const TokenIndex& index, uint range);

// Typedef for all methods which already resolved.
typedef cHash<TokenIndex, bool> MethodResolvedObject;

// Typedef for stack of objects
typedef StackInfrastructor<TokenIndex, cList<TokenIndex>::iterator> MethodStackObject;

typedef cHash<TokenIndex, addressNumericValue> RelocationHash;

/*
* This interface should implement different
* linkers e.g. COFF, COM etc.
*/
class LinkerInterface
{
public:
    // C'tor
    LinkerInterface(CompilerEngineThread& compilerEngineThread, ApartmentPtr apartment);

    // You can inherit from me
    virtual ~LinkerInterface();

    /*
     * Scan a method and all-of it sub-method and resolve all connections.
     */
    virtual void resolveAndExecuteAllDependencies(TokenIndex& mainMethod) = 0;

protected:

    /*
     * Bind into a method and fills all the dependencies needed in order to
     * execute the method.
     *
     * Return a numeric value which can be transfer into the 'ExecuteInterface'
     */
    virtual addressNumericValue bind(SecondPassBinary& pass) = 0;


    /*
     * From a token index, return an already exists address for a vtbl.
     * If the vtbl address hasn't been set yet (See setVtblAddress), a 0 will be returned.
     */
    addressNumericValue getVtblAddress(const TokenIndex& tokenIndex);

    /*
     * Change a vtbl address (depenends on linker implementation)
     *
     * tokenIndex - The TypeDef token
     * address - The binded address for the vtbl
     */
    void setVtblAddress(const TokenIndex& tokenIndex, addressNumericValue address);

    /*
     * Add a static into the used global hash table and return it's position from global
     * start section
     */
    addressNumericValue getStaticAddress(const TokenIndex& fieldIndex);

    /*
     * Return the total size of the global (.bss)
     */
    uint getTotalAllocatedStaticBuffer() const;

    // The binary engine.
    CompilerEngineThread& m_engine;

    // The external method resolver. TODO! Get from factory
    ExternalModuleResolver m_externalModuleResolver;
    ClrResolver m_clrResolver;

    // The apartment
    ApartmentPtr m_apartment;
    // Offset inside the vtbl (filled so far)
    uint m_vtblFilledSize;
private:
    // Disable assignment and copy construction
    LinkerInterface(const LinkerInterface& other);
    LinkerInterface& operator=(const LinkerInterface& other);

    // Repository for vtbl addresses
    RelocationHash m_vtbls;
    // Resolver for all static fields allocated so far
    RelocationHash m_staticFieldsTable;
    // Contain the entire size of the static table so far
    uint m_totalStaticFields;
};

// The reference-counted object
typedef cSmartPtr<LinkerInterface> LinkerInterfacePtr;

#endif // __TBA_CLR_EXECUTER_RUNTIME_LINKERINTERFACE_H
