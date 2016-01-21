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

#ifndef __TBA_CLR_RUNNABLE_METHODRUNNABLE_H
#define __TBA_CLR_RUNNABLE_METHODRUNNABLE_H

/*
 * MethodRunnable.h
 *
 * All run-time information needed for executing and compiling a method
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/list.h"
#include "xStl/data/smartptr.h"
#include "xStl/stream/forkStream.h"
#include "format/coreHeadersTypes.h"
#include "data/ElementType.h"
#include "format/methodHeader.h"
#include "format/tables/Table.h"
#include "format/tables/MethodTable.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/Apartment.h"

/*
 * Holds method runtime information.
 *
 * Usage: Call 'loadMethod' when ready
 *
 * NOTE: This class is not thread-safe.
 */
class MethodRunnable {
public:
    /*
     * Constructor. Don't do a thing.
     *
     * See loadMethod()
     */
    MethodRunnable(const ApartmentPtr& apartment);

    /*
     * Load a method data
     */
    void loadMethod(mdToken methodToken);

    /*
     * Return true if the implementation is missing. (e.g. extern method).
     * In that case only the method-header and method-signature is valid.
     */
    bool isEmptyMethod() const;

    /*
     * Return the current apartment
     */
    const ApartmentPtr& getApartment() const;
    ApartmentPtr& getApartment();

    /*
     * Return the stream pointer of the instruction-pointer of this function
     */
    cForkStreamPtr& getStreamPointer();

    /*
     * Return the method start address for 'stream-pointer()'. Using this value
     * user can invoke:
     *     getStreamPointer()->seek(getMethodStartAddress(), IO_SEEK_SET);
     * in order to return to the beginning of the method.
     *
     * NOTE: This method is the const version of
     *        getMethodHeader()->getMethodStreamStartAddress().
     * See MethodHeader::getMethodStreamStartAddress
     */
    uint getMethodStreamStartAddress() const;

    /*
     * Return the method-header
     */
    MethodHeader& getMethodHeader();

    /*
     * Returns the method signature
     *
     * Throw exception if the method is not fully loaded
     */
    const MethodDefOrRefSignature& getMethodSignature() const;

    /*
     * Return the locals variable of the function
     */
    ElementsArrayType& getLocals();

    /*
     * Return the methoddef or the typedef token index
     */
    const TokenIndex& getMethodToken() const;

    /*
     * Return true if the current method belongs to System.Object
     */
    bool isSystemObject() const;

    /*
     * Return method parent
     */
    const ElementType& getMethodTypeParent() const;

    /*
     * Return the full method name
     */
    const cString& getFullMethodName() const;

protected:
    // Deny copy-constructor and operator =
    MethodRunnable(const MethodRunnable& other);
    MethodRunnable& operator = (const MethodRunnable& other);

    // The function apartment
    ApartmentPtr m_apartment;

    // The method header
    cSmartPtr<MethodHeader> m_methodHeader;

    // Method name
    cString m_fullMethodName;

    // The method current pointer
    cForkStreamPtr m_streamPointer;

    // Method token
    TokenIndex m_methodToken;
    bool m_isSystemObject;
    ElementType m_methodParentType;

    // The method's signature
    MethodDefOrRefSignaturePtr m_methodSignature;

    // The locals
    ElementsArrayType m_locals;

    // Set to true if the method is not implemented (e.g. extern method)
    bool m_emptyImpl;
};

#endif // __TBA_CLR_RUNNABLE_METHODRUNNABLE_H
