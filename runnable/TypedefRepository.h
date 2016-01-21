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

#ifndef __TBA_CLR_RUNNABLE_TYPEDEFREPOSITORY_H
#define __TBA_CLR_RUNNABLE_TYPEDEFREPOSITORY_H

/*
 * TypedefRepository.h
 *
 * Repository of typedef.
 * Taking in account inheritance and more.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/xstlLockable.h"
#include "xStl/data/hash.h"
#include "xStl/data/counter.h"
#include "data/ElementType.h"
#include "runnable/ResolverInterface.h"
#include "runnable/MemoryLayoutInterface.h"
#include "runnable/Apartment.h"

// Forward deceleration
class GlobalContext;

/*
 *
 * NOTE: This module must be constructed before the TypesNameRepository class
 * NOTE: This class is thread-safe
 */
class TypedefRepository : public ResolverInterface {
public:
    // See ResolverInterface::getTypeSize
    virtual void resolveTyperef(ElementType& type) const;
    // See ResolverInterface::resolveFieldref
    virtual void resolveFieldref(TokenIndex& fieldToken, const TokenIndex& parentType) const;
    // See ResolverInterface::getTypeSize
    virtual uint getTypeSize(const TokenIndex& typeToken) const;
    // See ResolverInterface::getTypeSize
    virtual uint getTypeSize(const ElementType& type) const;
    // See ResolverInterface::isTypeInterface
    virtual bool isTypeInterface(const TokenIndex& typeToken) const;
    // See ResolverInterface::isTypeShouldDref
    virtual bool isTypeShouldDref(const TokenIndex& typeToken) const;
    // See ResolverInterface::getVirtualTable
    virtual const ResolverInterface::VirtualTable&
            getVirtualTable(const TokenIndex& typedefToken) const;
    // See ResolverInterface::getParentDirectory
    virtual const ParentDictonary& getParentDirectory(const TokenIndex& typedefToken) const;
    // See ResolverInterface::getRTTI
    virtual const uint getRTTI(const TokenIndex& typedefToken) const;
    // See ResolverInterface::getAllFields
    virtual const FieldsDictonary& getAllFields(const TokenIndex& parentToken) const;
    // See ResolverInterface::getFieldRelativePosition
    virtual uint getFieldRelativePosition(const TokenIndex& fieldToken,
                                          const TokenIndex& parentToken) const;
    // See ResolverInterface::getFieldType
    const ElementType& getFieldType(const TokenIndex& fieldToken,
                                    const TokenIndex& parentToken) const;

    // See ResolverInterface::getStaticFieldOffset
    virtual uint getStaticFieldOffset(const TokenIndex& fieldToken) const;
    // See ResolverInterface::getStaticTotalLength
    virtual uint getStaticTotalLength() const;
    // See ResolverInterface::allocateStatic
    virtual TokenIndex allocateStatic(uint size);
    // See ResolverInterface::allocateDataSection
    virtual uint allocateDataSection(cForkStreamPtr& stream, uint size);
    virtual const cBuffer& getDataSection();
    // See ResolverInterface::getStaticInitializerMethod
    virtual TokenIndex getStaticInitializerMethod(const TokenIndex& typeToken) const;
    // See ResolverInterface::getTypeToken
    virtual TokenIndex getTypeToken(const cString& _namespace,
                                    const cString& _className) const;
    // See ResolverInterface::getNewGenericInstanceToken
    virtual TokenIndex getNewGenericInstanceToken(const ElementType& type);
    // See ResolverInterface::isTypedefClass
    virtual bool isTypedefClass(const TokenIndex& typeToken) const;
    // See ResolverInterface::getTypeHashSignature
    virtual const cBuffer& getTypeHashSignature(const TokenIndex& typeToken) const;

    /*
     * Virtual Table helper functions
     *
     * Scan all typedef and calulcate the virtual table sizes
     */
    uint estimateVirtualTableMaximumSize(uint callSize) const;

    /*
     * Return class known types
     */
    const TokenIndex& getSystemObject() const;

    /*
     * Return System.String.ctor() function
     */
    const TokenIndex& getStringConstructor() const;

    /*
     * In order to make sure the RTTI are generated the same per apartment.
     * Call this function when all apartments are loaded
     */
    void doneLoadingApartments();

protected:
    friend class GlobalContext;
    /*
     * Construct new TypedefRepository object
     *
     * apartment - The apartment object
     */
    TypedefRepository(const ApartmentPtr& apartment,
                      const MemoryLayoutInterface& memoryLayoutInterface);

    /*
     * Checking new apartment for runtime functions
     *
     * apartment - The apartment object
     */
    void addApartment(const ApartmentPtr& apartment);

    /*
     * Repository an apartment
     */
    void repoApartment(const ApartmentPtr& apartment);

    // The apartment
    mutable ApartmentPtr m_apartment;

    // The memory layout for object size calculation
    const MemoryLayoutInterface& m_memoryLayout;

private:

    // See getRTII()       This method is the locked version of the old method
    const uint lockGetRTTI(const TokenIndex& typedefToken) const;
    // See getVirtualTable() This method is the locked version of the old method
    const VirtualTable& lockGetVirtualTable(const TokenIndex& typedefToken) const;
    // See getTypedefSize().
    uint lockGetTypeSize(const TokenIndex& typeToken) const;

    /*
     * Add a typedef token into the repository.
     *
     * NOTE: This method is recursive
     */
    void lockAppendTypedef(const TokenIndex& typedefToken) const;
    void lockCheckAppendTypedef(const TokenIndex& typedefToken) const;

    /*
     * Inline helper function, resolve parent for field token if the parent is unknown
     */
    TokenIndex resolveParentToken(const TokenIndex& fieldToken,
                                  const TokenIndex& parentToken) const;

    /*
     * Return the size of an element-type.
     * If the element is a value-type then the inner size of the element will be
     * increase to the size of the typedef.
     *
     * type - The type
     *
     * Throw exception if recursive call are made. (E.g. shouldProtectClass is
     * false and the typedef token cannot be found in the hash)
     */
    uint innerGetTypeSize(const ElementType& type) const;

    /*
     * From an element type, remove all generic instances (nil token) and fill with
     * the actual data type. If the element type is a normal element, return the actual
     * input.
     *
     * elementType - The input element. All nil token will be replace at the actual
     *               generic information
     * genericValue - Pointer to TABLE_CLR_GENERICS_INSTANCES (internal constructed)
     *                See getNewGenericInstanceToken()
     *
     * Return the filled ElementType
     */
    ElementType getGenericRealElementType(const ElementType& elementType,
                                          const TokenIndex& genericValue) const;

    /*
     * Append all parents into the new destination
     */
    void appendParents(ParentDictonary& dest, const ParentDictonary& source, uint& interfaceOffset) const;

    /*
     * Merge interface vtbl into parent
     */
    static void mergeInterfaceVirtualTables(VirtualTable& dest, const VirtualTable& source);

    // The lockable object
    mutable cXstlLockable m_lock;

    // Translate typedef and class-type
    // OPT// cHash<mdToken, ClassType> m_classesTypes;

    // The System.Object class token
    TokenIndex m_tokenSystemObject;
    // See System.Object class token
    TokenIndex m_tokenSystemValueType;
    // The System.String class token (if any)
    TokenIndex m_tokenSystemString;
    // The System.Array class token (if any)
    TokenIndex m_tokenSystemArray;
    // private String(uint size, tchar* source, bool shouldCopyToHeap)
    TokenIndex m_tokenStringConstructor;

    /*
     * Contains all information needed for a typedef
     */
    struct TypedefRepositoryContainer {
        // RTTI per token.
        uint m_rtti;
        // The virtual-table
        ResolverInterface::VirtualTable m_virtualTable;
        // Store the class relations
        ParentDictonary m_extends;
        // Static initalizer
        TokenIndex m_staticInitializerMethod;
        // The total length of the structure.
        uint m_typedefSize;
        // List of all fields.
        FieldsDictonary m_fields;
        // Set to true if the typedef is an interface
        bool m_isInterface;
        // Set to true if the type could be completed
        bool m_isCompleted;
        // Set to true if the typedef needs special cleanning (ValueType with object members or
        // Class with members objects)
        bool m_isSpecialCleanup;
        // Unique hash table for the object
        cBuffer m_hashSignature;
    };

    #ifdef XSTL_WINDOWS
		// Typedef virtual-table repository layout
        mutable cHash<typename TokenIndex, TypedefRepositoryContainer> m_types;
		// New generic instances (new tokens) vs Generic elements
		cHash<typename TokenIndex, ElementType> m_genericInstances;
    #else
		// Typedef virtual-table repository layout
        mutable cHash<TokenIndex, TypedefRepositoryContainer> m_types;
		// New generic instances (new tokens) vs Generic elements
		cHash<TokenIndex, ElementType> m_genericInstances;
    #endif

    // Global database, stores token and size of each token
    mutable cHash<TokenIndex, uint> m_staticDB;
    mutable uint m_staticDBLength;

    // The data buffer (.data)
    cBuffer m_dataBuffer;

    /*
     * From a type encoded in "offsetType" calculate it's member-size/storage-
     * size and append this value into the right position
     */
    void setFieldOffset(uint& typedefSize,
                        FieldRepositoryContainer& offsetType) const;


    /*
     * From a typeref token get new typedef and resolution scope
     *
     * typerefToken    - The typeref to be translated into typedef
     * resolutionScope - Will be field with the new token resolution scope
     *
     * Return the new apartment typedef object
     * Throw exception if the typeref is invalid
     */
    mdToken translateTyperefToken(mdToken typerefToken,
                                  mdToken& resolutionScope) const;


    //////////////////////////////////////////////////////////////////////////
    // Statics
    // Empty constructed virtual table
    static const VirtualTable m_emptyVirtualTable;

    // RTTI generator
    static cCounter gRttiCounter;
};

#endif // __TBA_CLR_RUNNABLE_TYPEDEFREPOSITORY_H
