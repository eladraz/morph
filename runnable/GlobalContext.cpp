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

/*
 * GlobalContext.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/assert.h"
#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"

GlobalContext::GlobalContext() :
    m_typesNameRepository(NULL),
    m_typedefRepository(NULL),
    m_frameworkMethods(NULL),
    m_stringRepository(NULL),
    m_memoryLayout(NULL)
{
}

GlobalContext::~GlobalContext()
{
    destroy();
}

void GlobalContext::addApartment(const ApartmentPtr& apartment)
{
    m_typesNameRepository->addApartment(apartment);
    m_typedefRepository->addApartment(apartment);
    m_frameworkMethods->addApartment(apartment);
    // m_stringRepository->addApartment(apartment);
}

void GlobalContext::destroy()
{
    delete m_typesNameRepository; m_typesNameRepository = NULL;
    delete m_typedefRepository;   m_typedefRepository = NULL;
    delete m_frameworkMethods;    m_frameworkMethods = NULL;
    delete m_stringRepository;    m_stringRepository = NULL;
}

void GlobalContext::init(const ApartmentPtr& mainApartment,
                         const MemoryLayoutInterface& memoryLayout)
{
    m_memoryLayout = &memoryLayout;
    ASSERT(m_stringRepository == NULL);
    m_stringRepository = new StringRepository(mainApartment, memoryLayout);
    ASSERT(m_typesNameRepository == NULL);
    m_typesNameRepository = new TypesNameRepository(mainApartment);
    // NOTE: The TypedefRepository constructed must be after the
    //       TypesNameRepository constructed. Please keep this order
    ASSERT(m_typedefRepository == NULL);
    m_typedefRepository = new TypedefRepository(mainApartment, memoryLayout);
    ASSERT(m_frameworkMethods == NULL);
    m_frameworkMethods = new FrameworkMethods();
    m_frameworkMethods->addApartment(mainApartment);
}


TypesNameRepository& GlobalContext::getTypesNameRepository()
{
    ASSERT(m_typesNameRepository != NULL);
    return *m_typesNameRepository;
}

TypedefRepository& GlobalContext::getTypedefRepository()
{
    ASSERT(m_typedefRepository != NULL);
    return *m_typedefRepository;
}

FrameworkMethods& GlobalContext::getFrameworkMethods()
{
    ASSERT(m_frameworkMethods != NULL);
    return *m_frameworkMethods;
}

StringRepository& GlobalContext::getStringRepository()
{
    ASSERT(m_stringRepository != NULL);
    return *m_stringRepository;
}

const TypesNameRepository& GlobalContext::getTypesNameRepository() const
{
    ASSERT(m_typesNameRepository != NULL);
    return *m_typesNameRepository;
}

const TypedefRepository& GlobalContext::getTypedefRepository() const
{
    ASSERT(m_typedefRepository != NULL);
    return *m_typedefRepository;
}

const FrameworkMethods& GlobalContext::getFrameworkMethods() const
{
    ASSERT(m_frameworkMethods != NULL);
    return *m_frameworkMethods;
}

const StringRepository& GlobalContext::getStringRepository() const
{
    ASSERT(m_stringRepository != NULL);
    return *m_stringRepository;
}
