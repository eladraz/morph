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

#ifndef __TBA_CLR_EXECUTER_COMPILER_PRECOMPILEDREPOSITORY_H
#define __TBA_CLR_EXECUTER_COMPILER_PRECOMPILEDREPOSITORY_H

/*
 * PrecompiledRepository.h
 *
 * Contain a list of all functions that already been compiled
 *
 * Elad Raz
 * 2012
 */
#include "xStl/types.h"
#include "xStl/os/lockable.h"
#include "xStl/os/os.h"
#include "xStl/data/sarray.h"
#include "xStl/data/string.h"
#include "xStl/data/serializedObject.h"
#include "xStl/stream/basicIO.h"
#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"

#include "dismount/assembler/SecondPassBinary.h"

class PrecompiledRepository : public cSerializedObject {
public:
    /*
     * Constructor.
     */
    PrecompiledRepository(const ApartmentPtr& mainApartment);

    /*
     * From a signature and apartment name return a secondpass binary.
     * If there is a function in the repository. Update it's time and return the content of the binary
     * If there is no function return empty reference object
     *
     * NOTE: This is a thread safe function
     */
    SecondPassBinaryPtr getPrecompiledMethod(const cString& apartmentName, const cBuffer& signature) const;

    /*
     * Add a new precompiled header to the repository
     *
     * NOTE: This is a thread safe function
     */
    void appendPrecompiledMethod(const cString& apartmentName, const cBuffer& signature,
                                 const SecondPassBinaryPtr& compiledFunction);

    // See cSerializedObject::isValid
    virtual bool isValid() const;
    // See cSerializedObject::deserialize
    virtual void deserialize(basicInput& inputStream);
    // See cSerializedObject::serialize
    virtual void serialize(basicOutput& outputStream) const;

private:
    // Deny copy-constructor
    PrecompiledRepository(const PrecompiledRepository& other);

    // The lockable object
    mutable cXstlLockable m_lock;
    // Main apartment pointer
    mutable ApartmentPtr m_mainApartment;

    struct MethodSignature
    {
        // Last function update time. Use for aging.
        mutable cOSDef::systemTime time;
        // And data
        SecondPassBinaryPtr binary;
    };

    // Per apartment, signatures of methods
    typedef cHash<cBuffer, MethodSignature> ApartmentPrecompiledMethods;
    struct ApartmentSignature
    {
        ApartmentPrecompiledMethods hash;
        uint apartmentHelperNumber;
    };

    // Main header file
    cHash<cString, ApartmentSignature> m_apartments;

    // PrecompiledRepository file header
    static const char gRepositoryHeader[];
};

#endif // __TBA_CLR_EXECUTER_COMPILER_PRECOMPILEDREPOSITORY_H
