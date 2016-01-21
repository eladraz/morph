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

#ifndef __TBA_CLR_RUNNABLE_GLOBALCONTEXT_H
#define __TBA_CLR_RUNNABLE_GLOBALCONTEXT_H

/*
 * GlobalContext.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/counter.h"
#include "runnable/Apartment.h"
#include "runnable/ResolverInterface.h"
#include "runnable/GlobalContext.h"
#include "runnable/TypesNameRepository.h"
#include "runnable/TypedefRepository.h"
#include "runnable/FrameworkMethods.h"
#include "runnable/StringRepository.h"

class GlobalContext {
public:
    /*
     * [Singleton per main apartment] Return the types-name repository
     */
    TypesNameRepository& getTypesNameRepository();
    const TypesNameRepository& getTypesNameRepository() const;

    /*
     * [Singleton per main apartment] Return the TypedefRepository
     */
    TypedefRepository& getTypedefRepository();
    const TypedefRepository& getTypedefRepository() const;

    /*
     * [Singleton per main apartment] Return the FrameworkMethods
     */
    FrameworkMethods& getFrameworkMethods();
    const FrameworkMethods& getFrameworkMethods() const;

    /*
     * [Singleton per main apartment] See StringRepository
     */
    StringRepository& getStringRepository();
    const StringRepository& getStringRepository() const;

private:
    // Only apartment can instance and access this fields
    friend class Apartment;

    /*
     * Checking new apartment for runtime functions
     *
     * apartment - The apartment object
     */
    void addApartment(const ApartmentPtr& apartment);

    /*
     * Constructor.
     */
    GlobalContext();

    /*
     * Destructor. Free the allocated objects
     */
    ~GlobalContext();

    // See Apartment::init
    void init(const ApartmentPtr& mainApartment,
              const MemoryLayoutInterface& memoryLayout);

    // See Apartment::destroy
    void destroy();

    // The types name repository
    TypesNameRepository* m_typesNameRepository;
    // The typedef repository
    TypedefRepository* m_typedefRepository;
    // The framework functions
    FrameworkMethods* m_frameworkMethods;
    // String containers
    StringRepository* m_stringRepository;
    // Memory layout
    const MemoryLayoutInterface* m_memoryLayout;
};

#endif // __TBA_CLR_RUNNABLE_GLOBALCONTEXT_H
