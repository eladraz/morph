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

#ifndef __TBA_CLR_EXECUTER_BINARYGETTERINTERFACE_H
#define __TBA_CLR_EXECUTER_BINARYGETTERINTERFACE_H

/*
 * BinaryGetterInterface.h
 *
 * Provides a repository to second-pass compiled methods inside an apartment
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "xStl/data/hash.h"
#include "dismount/assembler/SecondPassBinary.h"
#include "format/coreHeadersTypes.h"
#include "data/ElementType.h"

typedef cHash<TokenIndex, SecondPassBinaryPtr> MethodTransTable;

/*
 * Return compiled method from cache
 *
 * NOTE: This class is thread-safe
 */
class BinaryGetterInterface {
public:
    // Default constructor
    BinaryGetterInterface();

    // Default virtual destructor. You can inherit from me
    virtual ~BinaryGetterInterface() {};

    /*
     * Get a second pass compiled method from the repository
     *
     * mid         - The method ID
     * apartmentId - The apartment ID
     *
     * Throw exception if there isn't any compiled method
     */
    virtual const SecondPassBinaryPtr& getSecondPassMethod(const TokenIndex& tokenIndex) const;

    /*
     * Get a hash of second pass compiled methods from the repository
     *
     * Throw exception if there isn't any compiled method
     */
    virtual const MethodTransTable& getMethodTransTable() const;

    /*
     * Return true if a method exist in the cache.
     * Return false otherwise
     *
     * mid          - The method ID
     * apartmentId  - The apartment ID
     * reference    - Used to retrieve the compiled method. Used for reference
     *                management (During the 'is' and 'get' method, the method
     *                might be revoked)
     */
    virtual bool isMethodExist(const TokenIndex& tokenIndex,
                               SecondPassBinaryPtr* reference = NULL) const;

    /*
     * Add a second pass binary into the map
     *
     * mid         - The method ID
     * apartmentId - The apartment ID
     * secondPass  - The method binary (upon it's dependencies)
     *
     * Throw exception if the binary already inside the tree
     */
    virtual void addSecondPassMethod(const TokenIndex& tokenIndex,
                                     const SecondPassBinaryPtr& secondPass);


private:
    // Deny copy-constructor
    BinaryGetterInterface(const BinaryGetterInterface& other);
    BinaryGetterInterface& operator = (const BinaryGetterInterface& other);

    // The translation table
    MethodTransTable m_hash;
    // The protection mutual
    mutable cXstlLockable m_mutex;
};

#endif // __TBA_CLR_EXECUTER_BINARYGETTERINTERFACE_H
