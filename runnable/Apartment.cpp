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
 * Apartment.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/os/xstlLockable.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/AssemblyTable.h"
#include "format/tables/AssemblyRefTable.h"
#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "runnable/RunnableTrace.h"


// Static deceleration
cCounter Apartment::gIdGenerator(0);

Apartment::Apartment(const CilFormatLayoutPtr& layout,
                     const cMemoryAccesserStreamPtr& metadata,
                     const IMAGE_DATA_DIRECTORY& directory,
                     const DWORD entryPointToken,
                     const ApartmentPtr& mainApartment) :
    m_layout(layout),
    m_metaHeader(metadata, directory),
    // Hopefully the compiler will not ignore the order registered here
    m_tables(m_metaHeader),
    m_streams(m_metaHeader),
    m_objects(NULL),
    m_mainApartment(mainApartment),
    m_unrelatedExternalModules(0),
    m_id(gIdGenerator.increase()),
    m_helperGenerator(0)
{
    m_entryPointToken = buildTokenIndex(m_id, entryPointToken);

    // Read the name of the assembly
    TablePtr asmTablePtr = m_tables.getTableByToken(EncodingUtils::buildToken(TABLE_ASSEMBLY_TABLE, 1)); // 0x20000001
    CHECK(!asmTablePtr.isEmpty()); // Error module don't have Assembly module (MUST)
    const AssemblyTable::Header& asmTable = ((AssemblyTable&)(*asmTablePtr)).getHeader();
    // Read the name from the string
    m_name = StringReader::readStringName(m_streams.getStringsStream(), asmTable.m_name);

    RunnableTrace("Apartment MODULE " << m_id << ": "<< m_name << endl);
}

Apartment::~Apartment()
{
    if (m_mainApartment.isEmpty())
        delete m_objects;
}

ApartmentPtr Apartment::getApt(const TokenIndex& token) const
{
    return getApartmentByID(getApartmentID(token));
}

bool Apartment::insertModule(ApartmentPtr& module)
{
    // insertModule is only for main apartment!
    CHECK(m_mainApartment.isEmpty());

    {
        // The entire class should be thread-safe include this method
        cLock lock(m_lock);
        // Mark the module as extern
        // The reference-count is increased, preventing sudden module death
        m_externModules.append(module->getUniqueID(), module);
    }

    // Adding new module to typedef/names/framework methods repositories
    m_objects->addApartment(module);

    return false;
}

ApartmentPtr Apartment::getApartmentByName(const cString& name)
{
    if (!m_mainApartment.isEmpty())
        return m_mainApartment->getApartmentByName(name);

    // Return main module
    if (name == m_name)
        return ApartmentPtr(this, SMARTPTR_DESTRUCT_NONE);

    // Scan external modules
    // TODO! Optimize here?
    cList<uint> modules = m_externModules.keys();
    for (cList<uint>::iterator i = modules.begin(); i != modules.end(); i++)
    {
        const ApartmentPtr& apt(m_externModules[*i]);
        if (apt->m_name == name)
            return apt;
    }

    // Name not found!
    return ApartmentPtr();
}

void Apartment::getApartmentsNames(cList<cString>& apts)
{
    apts.removeAll();
    cList<uint> modules = m_externModules.keys();
    for (cList<uint>::iterator i = modules.begin(); i != modules.end(); i++)
    {
        apts.append(m_externModules[*i]->m_name);
    }
}

const ApartmentHash& Apartment::getModules()
{
    return m_externModules;
}

const cString& Apartment::getApartmentName() const
{
    return m_name;
}

ApartmentPtr Apartment::getApartmentByID(uint uniqueId) const
{
    // Check for current module
    if (m_id == uniqueId)
    {
        return ApartmentPtr((Apartment*)this, SMARTPTR_DESTRUCT_NONE);
    }
    cLock lock(m_lock);
    return m_externModules[uniqueId];
}

void Apartment::init(const ApartmentPtr& thisApartment,
                     const MemoryLayoutInterface& memoryLayoutInterface)
{
    if (m_mainApartment.isEmpty())
    {
        m_objects = new GlobalContext();
        m_objects->init(thisApartment, memoryLayoutInterface);
    } else
    {
        m_objects = m_mainApartment->m_objects;
    }

}

void Apartment::destroy()
{
    m_objects->destroy();
}

ApartmentPtr Apartment::getMainApartment()
{
    if (m_mainApartment.isEmpty())
        return ApartmentPtr(this, SMARTPTR_DESTRUCT_NONE);

    return m_mainApartment;
}


GlobalContext& Apartment::getObjects()
{
    return *m_objects;
}

const GlobalContext& Apartment::getObjects() const
{
    return *m_objects;
}

const MetadataTables& Apartment::getTables() const
{
    return m_tables;
}

const MSILStreams& Apartment::getStreams() const
{
    return m_streams;
}

CilFormatLayoutPtr& Apartment::getLayout()
{
    return m_layout;
}

uint Apartment::getUniqueID() const
{
    return m_id;
}

uint Apartment::generateMethodHelperRow() const
{
    return m_helperGenerator.increase();
}

void Apartment::setMethodHelperRow(uint initIndex)
{
    m_helperGenerator.setValue(initIndex);
}

const TokenIndex Apartment::getEntryPointToken() const
{
    return m_entryPointToken;
}

cStringerStream& operator << (cStringerStream& out, const Apartment& apartment)
{
    out << "Apartment(";
    out << "id = " << (uint32)apartment.getUniqueID();
    out <<  ")";
    return out;
}
