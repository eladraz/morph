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

#ifndef __TBA_CLR_RUNNABLE_APARTMENT_H
#define __TBA_CLR_RUNNABLE_APARTMENT_H

/*
 * Apartment.h
 *
 * Apartment for all modules/globals/streams and tables of which a stand-alone
 * application should required.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "xStl/data/hash.h"
#include "xStl/data/array.h"
#include "xStl/data/counter.h"
#include "xStl/data/string.h"
#include "data/ElementType.h"
#include "format/metadataHeader.h"
#include "format/CilFormatLayout.h"
#include "format/MetadataTables.h"
#include "format/MSILStreams.h"
#include "runnable/MemoryLayoutInterface.h"

// Forward deceleration
class Apartment;
class GlobalContext;
class FrameworkMethods;
// The reference-object pointer
typedef cSmartPtr<Apartment> ApartmentPtr;
typedef cHash<uint, ApartmentPtr> ApartmentHash;

cStringerStream& operator << (cStringerStream& out, const Apartment& apartment);

/*
 * Use this following class as a global context for:
 *     - Metadata
 *     - Tables
 *     - Streams
 *
 * This class is thread-safe upon all it's members. And can be shared between
 * several of threads
 */
class Apartment {
public:
    /*
     * Default constructor. Construct all necessary objects for a file-layout.
     *
     * layout - The layout for the file
     * metadata - The ~ stream
     * directory - The directory information for the ~ stream
     */
    Apartment(const CilFormatLayoutPtr& layout,
              const cMemoryAccesserStreamPtr& metadata,
              const IMAGE_DATA_DIRECTORY& directory,
              const DWORD entryPointToken,
              const ApartmentPtr& mainApartment);

    // Free memory
    ~Apartment();

    /*
     * Insert extern module into the current module space\
     *
     * The link is in one-direction. The current module will be able to use the
     * 'module' functions
     *
     * Return true if the module was resolved and the current module does uses
     * the external module
     * Return false otherwise
     *
     * NOTE: This function is thread-safe
     */
    bool insertModule(ApartmentPtr& module);

    /*
     * Return the list of all modules
     */
    const ApartmentHash& getModules(void);

    /*
     * Return the name of the apartment
     */
    const cString& getApartmentName() const;

    /*
     * Return an apartment according to module name
     */
    ApartmentPtr getApartmentByName(const cString& name);

    /*
     * Fill up the list of apartments
     */
    void getApartmentsNames(cList<cString>& apts);

    /*
     * Returns an apartment according to it's unique-id given in the
     * 'getUniqueID()'.
     * If the uniqueId equal this apartment unique ID then the function return
     * new non-destructible reference into this object
     *
     * NOTE: This function is thread-safe
     */
    ApartmentPtr getApartmentByID(uint uniqueId) const;

    /*
     * From TokenIndex return the apartment-id
     */
    ApartmentPtr getApt(const TokenIndex& token) const;

    /*
     * Generate a circle smart-pointer reference from this apartment and the
     * main's object.
     *
     * globalContext - See GlobalContext
     *
     * See 'destroy'
     */
    void init(const cSmartPtr<Apartment>& thisApartment,
              const MemoryLayoutInterface& memoryLayoutInterface);

    /*
     * Garbage collection.
     * Remove the rest of dual-links created by the 'init()' function and allow
     * getting the rest of the memory
     */
    void destroy();

    // Information retrieving

    /*
     * Return the main module.
     *
     * NOTE: The return object is a temporary and cannot be saved for a long time
     */
    ApartmentPtr getMainApartment();

    /*
     * Return the file's tables
     */
    const MetadataTables& getTables() const;

    /*
     * Return the file's streams
     */
    const MSILStreams& getStreams() const;

    const TokenIndex getEntryPointToken() const;
    /*
     * Return the execution global resolver module
     */
    GlobalContext& getObjects();
    const GlobalContext& getObjects() const;

    /*
     * Return the layout object
     */
    CilFormatLayoutPtr& getLayout();

    /*
     * Return the apartment (current runtime package) ID.
     * Used by the compiler to distinguish different apartments.
     */
    uint getUniqueID() const;

    enum ApartmentID { HELPER_APARTMENT = MAX_UINT32 };

    /*
     * Generate a new token for a compiler-generated method helper function
     */
    uint generateMethodHelperRow() const;

    /*
     * Change beginning method token
     */
    void setMethodHelperRow(uint initIndex);

private:
    // The DLL/EXE name as inside the Assembly table
    cString m_name;
    // The file layout
    CilFormatLayoutPtr m_layout;
    // The meta-data header for the apartment
    MetadataHeader m_metaHeader;
    // The tables of the file
    MetadataTables m_tables;
    // The streams of the file
    MSILStreams m_streams;
    // The objects
    GlobalContext* m_objects;

    // Other apartment
    mutable cXstlLockable m_lock;
    ApartmentHash m_externModules;
    // Pointer to the main object
    ApartmentPtr m_mainApartment;

    // The number of unrelated modules which are being imported brutally.
    uint m_unrelatedExternalModules;

    TokenIndex m_entryPointToken;

    // The unique apartment ID
    uint m_id;
    // The helper number generator
    mutable cCounter m_helperGenerator;

    // The unique ID generator.
    static cCounter gIdGenerator;
};

#endif // __TBA_CLR_RUNNABLE_APARTMENT_H

