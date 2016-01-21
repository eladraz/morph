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

#ifndef __TBA_CLR_RUNNABLE_TYPESNAMEREPOSITORY_H
#define __TBA_CLR_RUNNABLE_TYPESNAMEREPOSITORY_H

/*
 * TypesNameRepository.h
 *
 * Contains cached database regarding class names
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "xStl/data/string.h"
#include "xStl/data/hash.h"
#include "xStl/stream/forkStream.h"
#include "format/coreHeadersTypes.h"
#include "format/MetadataTables.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/Apartment.h"

/*
 * Responsible for repository names for an apartment regarding all typedefs and
 * type-references.
 *
 * This module is thread-safe.
 */
class TypesNameRepository {
public:
    /*
     * From a typeref token, translate the apartment id and token id into a real typedef token
     */
    TokenIndex translateTyperefToTypedef(const TokenIndex& typeRefToken);

    /*
     * Return true if type-token represent '_namespace' and '_name' objects.
     *
     * NOTE: The comparison is case-sensitive.
     *
     * Throw exception if token is invalid
     */
    bool isTypeEquals(const TokenIndex& typeToken,
                      const cString& _namespace,
                      const cString& _name);

    /*
     * Return the namespace for a TypeDefOrRef
     *
     * Throw exception if token is invalid
     */
    const cString& getTypeName(const TokenIndex& typeToken) const;

    /*
     * Return the namespace for a TypeDefOrRef
     *
     * Throw exception if token is invalid
     */
    const cString& getTypeNamespace(const TokenIndex& typeToken) const;

    /*
     * Return the destructor method (Finialize() method) for a type
     * Return 0 to indicate that no destructor exist for this type
     */
    TokenIndex getTypeDestructor(const TokenIndex& typeToken);

    /*
     * From _namespace/_className and _methodName return the typedef token
     * of the method inside the current apartment.
     *
     * Throw exception ClrImportException if the method couldn't be found.
     */
    TokenIndex getMethodToken(const cString& _namespace,
                              const cString& _className,
                              const cString& _methodName,
                              const MethodDefOrRefSignature& methodSignature);

    /*
     * From _namespace/_className get the type token
     */
    TokenIndex getTypeToken(const cString& _namespace,
                            const cString& _className);

private:
    // Only the GlobalContext can instance this class
    friend class GlobalContext;

    // The main apartment
    ApartmentPtr m_mainApartment;

    /*
     * For each token (typedef), there is a 'TypeNameStruct' to dexcribes names/detors.
     */
    struct TypeNameStruct {
    public:
        // Default constructor.
        TypeNameStruct(const cString& namespaceString,
                       const cString& classnameString,
                       const TokenIndex& destructor);
        // Copy-constructor will auto-generated

        // Members
        cString m_namespaceString;
        cString m_classnameString;
        TokenIndex m_destructor;
    };

    /*
     * Private constructor. Only ApartmentObjects can instance this class
     */
    TypesNameRepository(const ApartmentPtr& mainApartment);

    /*
     * Checking new apartment for runtime functions
     *
     * apartment - The apartment object
     */
    void addApartment(const ApartmentPtr& apartment);

    /*
     * Append a new TypeDefOrRef class into the database
     *
     * NOTE: This function assumes the class is locked.
     */
    void appendTyperef(const TokenIndex& typeToken) const;

    /*
     * See getTypeToken
     * Assume m_lock is acquired.
     */
    TokenIndex lockGetTypeToken(const cString& _namespace,
                                const cString& _className);

    /*
     * Concat _namespace and _className with a *::* separator.
     * Used for unique key translation
     *
     * NOTE: This function assumes the class is locked.
     */
    static cString getUniqueClassNamespace(const cString& _namespace,
                                           const cString& _className);

    // The lock over the database
    mutable cXstlLockable m_lock;

    // The cached database names
    mutable cHash<TokenIndex, TypeNameStruct> m_types;
    // The cached namespace/class => typedef dictionary.
    mutable cHash<cString, TokenIndex> m_typesDictionary;

    // The connecting string between namespace and class names
    static const char gUniqueClassNamespaceConnectingString[];
};

#endif // __TBA_CLR_RUNNABLE_TYPESNAMEREPOSITORY_H
