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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_EXTERNALMODULERESOLVER_H
#define __TBA_CLR_EXECUTER_RUNTIME_EXTERNALMODULERESOLVER_H


/*
 * ExternalModuleResolver.h
 *
 * From a member-ref try to resolve an external module functions.
 * TODO! Add a factory so inheritage can be handled.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "runnable/Apartment.h"
#include "executer/runtime/RuntimeClasses/RuntimeClass.h"
#include "executer/runtime/ExternalModuleTable.h"
#include "format/signatures/MethodDefOrRefSignature.h"

class ExternalModuleResolver {
public:
    /*
     * Constructor. Initialize a new class
     */
    ExternalModuleResolver(const ApartmentPtr& apartment);

    /*
     * Check for external method implementation
     *
     * methodToken   - The method ID/Apartment
     * addr          - In case that the method is an external method. Will be filled
     *                 with the function address.
     * method        - Will be filled by the actual function entry. (Used for object file)
     *
     * Return true if the method is an external method.
     * Return false otherwise.
     */
    bool resolveExternalMethod(const TokenIndex& methodToken,
                               addressNumericValue& addr,
                               ExternalModuleFunctionEntry** method = NULL);

    /*
     * See resolveExternalMethod(). Called by resolveExternalMethod() after all
     * symbols where resolved
     *
     * importName      - The name of the import table
     * methodSignature - The method arguments and return address
     * method          - Will be filled with the actual import entry
     *
     * Return true if the method is an external method.
     * Return false otherwise.
     */
    virtual bool resolveExternalMethodName(const cString& importName,
                                           const MethodDefOrRefSignature& methodSignature,
                                           addressNumericValue& addr,
                                           ExternalModuleFunctionEntry** method);

protected:
    // The main apartment. Used for resolve methods
    ApartmentPtr m_apartment;
    // TODO! Switch to interface for additional information
    RuntimeClass m_runtimeClass;

    static const char gCustomAttributeImportClass[];
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_EXTERNALMODULERESOLVER_H

