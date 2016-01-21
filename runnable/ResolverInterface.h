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

#ifndef __TBA_CLR_RUNNABLE_RESOLVERINTERFACE_H
#define __TBA_CLR_RUNNABLE_RESOLVERINTERFACE_H

/*
 * ResolverInterface.h
 *
 * Translate typedef/classes/value-types and thier field into
 * storage object-size and field position inside the storage.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/list.h"
#include "xStl/data/hash.h"
#include "xStl/stream/forkStream.h"
#include "data/ElementType.h"
#include "runnable/MemoryLayoutInterface.h"

/*
 * API for Typedef/Typeref handler
 */
class ResolverInterface {
public:

    // The virtual table per token includes a list of:
    //   a = Original MethodIndex (original function)
    //   b = Override MethodIndex (The final function for the vtbl )
    typedef cList<cDualElement<TokenIndex, TokenIndex> > VirtualTable;
    // ParentDictonary. Stores parents and thier relative offset per typedef
    #ifdef XSTL_WINDOWS
        typedef cHash<typename TokenIndex, uint> ParentDictonary;
    #else
        typedef cHash<TokenIndex, uint> ParentDictonary;
    #endif

    #define getVtblMethodIndexOriginal(x) ((x).m_a)
    #define getVtblMethodIndexOverride(x) ((x).m_b)

    /*
     * Per field container
     */
    struct FieldRepositoryContainer {
        // Offset of the field from the beginning of the Object
        uint m_offset;
        // The type of the field
        ElementType m_type;
    };

    // For each field, store its position
    #ifdef XSTL_WINDOWS
	typedef cHash<typename TokenIndex, FieldRepositoryContainer> FieldsDictonary;
    #else
        typedef cHash<TokenIndex, FieldRepositoryContainer> FieldsDictonary;
    #endif


    // See scanAllFields
    class FieldEnumeratorCallbacker
    {
    public:
        /*
         * Default destructor
         */
        virtual ~FieldEnumeratorCallbacker();

        /*
         * Perform the callback. Per field
         */
        virtual void fieldCallback(void* context,
                                   const ResolverInterface& resolver,
                                   const TokenIndex& fieldContext,
                                   const ElementType& fieldElementType) = 0;
    };

///////////////////////////////////////
/// Functions

    // You can inherit from me
    virtual ~ResolverInterface();

    /*
     * Change a typeref according
     *
     * type - [in/out] Will be modified according to the
     */
    virtual void resolveTyperef(ElementType& type) const = 0;

    /*
     * Resolved external field token (=MemberRef)
     *
     * NOTE: parentToken must be resolved!
     */
    virtual void resolveFieldref(TokenIndex& fieldToken, const TokenIndex& parentType) const = 0;

    /*
     * Return the 'sizeof(typedef)' of a token. The token can be either a
     * class, value-type.
     * getTypeSize() - will return for objects and pointer only pointer-width
     *
     * typedefToken - The typedef token
     *
     * Throw exception if typeToken is not TABLE_TYPEDEF-REF_TABLE table.
     */
    virtual uint getTypeSize(const TokenIndex& typeToken) const = 0;

    /*
     * Calculate the size of an element from it's ElementType structure
     */
    virtual uint getTypeSize(const ElementType& typeToken) const = 0;

    /*
     * Return true if the typedef is an interface or not
     */
    virtual bool isTypeInterface(const TokenIndex& typeToken) const = 0;

    /*
     * Return true if the typedef needed member dereference
     */
    virtual bool isTypeShouldDref(const TokenIndex& typeToken) const = 0;

    /*
     * Return the layout of a virtual table
     */
    virtual const VirtualTable& getVirtualTable(const TokenIndex& typedefToken) const = 0;

    /*
     * Get all parents and thier relative offset within the class
     */
    virtual const ParentDictonary& getParentDirectory(const TokenIndex& typedefToken) const = 0;

    /*
     * Return the RTTI unique number per class, per compilation
     */
    virtual const uint getRTTI(const TokenIndex& typedefToken) const = 0;

    /*
     * Return list of all fields per a typedef
     */
    virtual const FieldsDictonary& getAllFields(const TokenIndex& parentToken) const = 0;

    /*
     * Provide a callback iterator over fields.
     */
    void scanAllFields(const TokenIndex& parentToken, FieldEnumeratorCallbacker& callback, void* argument) const;

    /*
     * Return the field's member storage position
     *
     * fieldToken - The field token
     * parentToken - The typedef token. Used to calculate the exact field
     *               location inside inheritance classes
     *
     * Throw exception if field-offset is corrupted.
     */
    virtual uint getFieldRelativePosition(const TokenIndex& fieldToken,
                                          const TokenIndex& parentToken) const = 0;

    /*
     * Return the type of the field.
     *
     * fieldToken - The field token
     * parentToken - The typedef token. Used to calculate the exact field
     *               location inside inheritance classes
     *
     * Throw exception if field-offset is corrupted.
     */
    virtual const ElementType& getFieldType(const TokenIndex& fieldToken,
                                            const TokenIndex& parentToken) const = 0;

    // Mark a field as static that it will not be addressed by "ldfld" and
    // the variable's members but by GlobalContext class
    enum { OFFSET_FOR_STATICS = MAX_UINT32 };

    /*
     * Return the offset from the beginning of the .data section of the
     * static field
     *
     * fieldToken  - The field token
     *
     * Throw exception if field-offset is corrupted, or if the field is non-static
     */
    virtual uint getStaticFieldOffset(const TokenIndex& fieldToken) const = 0;

    /*
     * Allocate 'size' bytes (uninitialized) from the .bss section
     */
    virtual TokenIndex allocateStatic(uint size) = 0;

    /*
     * Allocate 'size' bytes and fill it with data
     */
    virtual uint allocateDataSection(cForkStreamPtr& stream, uint size) = 0;
    virtual const cBuffer& getDataSection() = 0;

    /*
     * From a typedef return the .cctor function constructor
     */
    virtual TokenIndex getStaticInitializerMethod(const TokenIndex& typeToken) const = 0;

    /*
     * Return the entire memory use in the global buffer
     */
    virtual uint getStaticTotalLength() const = 0;

    /*
     * From class name and namespace gets new apartment context typedef
     *
     * _className - The class name
     * _namespace - The namespace
     *
     * Throw exception if the type couldn't be found
     */
    virtual TokenIndex getTypeToken(const cString& _namespace, const cString& _className) const = 0;

    /*
     * From a generic type element, constuct (or return already exist) index to TABLE_CLR_GEERICS_INSTANCES
     * which translate into a new repository binding the gerneric type
     *
     * type - A GENERIC_INSTANCE type
     *
     * Return an index to TABLE_CLR_GEERICS_INSTANCES which is already instanced
     */
    virtual TokenIndex getNewGenericInstanceToken(const ElementType& type) = 0;

    /*
     * Return true if a typedef is an object (inherits from System.Object)
     * Return false if a typedef is a ValueType (inherits from System.ValueType)
     */
    virtual bool isTypedefClass(const TokenIndex& typeToken) const = 0;

    /*
     * Calculate the signature of a type
     */
    virtual const cBuffer& getTypeHashSignature(const TokenIndex& typeToken) const = 0;
};

#endif // __TBA_CLR_RUNNABLE_RESOLVERINTERFACE_H

