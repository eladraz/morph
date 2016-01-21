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

#ifndef __TBA_CLR_COMPILER_CALLINGCONVENTION_H
#define __TBA_CLR_COMPILER_CALLINGCONVENTION_H

/*
 * CallingConvention.h
 *
 * Implement functions call for different convention types.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "format/coreHeadersTypes.h"
#include "format/signatures/MethodDefOrRefSignature.h"
#include "runnable/MethodRunnable.h"
#include "compiler/TemporaryStackHolder.h"
#include "compiler/MethodRuntimeBoundle.h"
#include "compiler/MethodBlock.h"
#include "compiler/ArgumentsPositions.h"
#include "compiler/CompilerInterface.h"
#include "compiler/EmitContext.h"

// Forward deceleration
class MethodCompiler;

/*
 * See Arguments for more information
 */
class CallingConvention {
public:
    enum ThisObjectHandling
    {
        None,
        ThisBelowParams, // "this" was pushed before the parameres
        ThisAboveParams, // "this" was pushed after the parameters, and is at the top of stack
        ThisAboveParamsDup, // "this" was pushed after the parameters, and the call should duplicate it on the stack (below the return value)
    };

    /*
     * Implement a call instruction into new method as new method calling
     * convention, according to the current compiler settings.
     *
     * NOTE: This function also uses as a thunk for build-in methods.
     *       SEE BUILD-IN METHOD SUPPORT
     *
     * emitContext - The context of the current method. Note: The method's arguments
     *               should be pushed onto the stack one by one, Left-to-right.
     * methodToken - The jump method ID.
     *                    The method can be either:
     *                    METHODDEF - Execute method from the current namespace
     *                    METHODREF - Execute external method
     * isVirtual     - Set to true if the call should be virtual
     * isThisOnTop - Set to true in order to indicate that 'this' pointer is
     *               on top-of-stack. Used for 'newobj' instruction and some
     *               other method of calling-conventions.
     *               Throw exception if the function is static one.
     * skipPush - Use in incobj/decobj api where pushing the arguments is done in the
     *            calle code
     * duplicateCtor - Set to true if the current function is .ctor function. In this case
     *            the .ctor function will push the object into the stack as a return
     *            value
     *
     * Throw exception if the stack is corrupted or another error that may
     * occurred
     */
    static void call(EmitContext& emitContext,
                     const TokenIndex& methodToken,
                     ThisObjectHandling thisHandling = None,
                     bool isVirtual = false,
                     bool skipPush = false);

    /*
     * Helper function. From apartment and method-token, get the method signature
     *
     * methodContext - The context of the current method
     * methodRuntime - Compiler runtime data
     * methodID      - Method ID (METHODDEF/METHODREF)
     *
     * Return MethodDefOrRefSignaturePtr.
     * Incase of an error return null object
     */
    static MethodDefOrRefSignaturePtr readMethodSignature(const Apartment& apartment,
                                                          mdToken methodToken);


    /*
     * From a typedef token, build a CCTOR wrapper name
     */
    static TokenIndex buildCCTOR(const TokenIndex& parentTypedef);

    /*
     * Encode a method token as a dependency, later to be used as a reference
     * for external method (See SecondPassBinary)
     *
     * token - The method token
     *
     * TODO! Add additional information for making the method stand-bye compiled
     *       For example, encode signature.
     *
     * Return the encoded string
     */
    static cString serializedMethod(const TokenIndex& token);

    /*
     * Encode a global variable
     *
     * offset - The offset of the variable in the .data section
     */
    //static cString serializeGlobal(uint offset);
    static cString serializeGlobalData(uint offset);

    /*
     * Encode a token: Data section object/string object, later to be used as a
     * reference for external method (See SecondPassBinary)
     *
     * apartment - The apartment which the token is related to.
     * methodId - The method token
     *
     * Return the encoded string
     */
    static cString serializeToken(const TokenIndex& token);

    /*
     * Tries to deserialized a dependency for encoded method.
     *
     * dependency  - The dependency to deserialized
     * apartmentID - Will be filled with the apartment ID
     * methodID    - Will be filled with the method ID to execute
     *
     * TODO! Add additional information for making the method stand-bye compiled
     * apartmentName - Will be filled with the name of the apartment
     * typeNamespace - Will be filled with the type namespace
     * typename      - Will be filled with the type name
     * methodName    - Will be filled with the method name
     * signature     - Will be filled with the method signature
     *
     * Return true if the dependency is a CIL method token.
     * Return false otherwise and nullify all output parameters.
     */
    static bool deserializeMethod(const cString& dependency,
                                  TokenIndex& token);

    /*
     * Tries to deserialized a dependency for encoded token.
     *
     * dependency  - The dependency to deserialized
     * apartmentID - Will be filled with the apartment ID
     * token       - Will be filled by the actual token
     *
     * Return true if the dependency is a CIL token.
     * Return false otherwise and nullify all output parameters.
     */
    static bool deserializeToken(const cString& dependency,
                                 TokenIndex& token);

    /*
     * Tries to deserialized a dependency for encoded .data variable
     *
     * dependency  - The dependency to deserialized
     * offset - Will be filled with the variable offset inside the global memory
     *
     * Return true if the dependency is a CIL token.
     * Return false otherwise and nullify all output parameters.
     */
    //static bool deserializeGlobal(const cString& dependency,
    //                              uint& offset);
    static bool deserializeGlobalData(const cString& dependency,
                                      uint& offset);

    /*
     * From an apartment/method calculate the desired calling convention
     */
    static CompilerInterface::CallingConvention
           getDesiredCallingMethod(mdToken methodToken,
                                   const Apartment& apartment,
                                   const CompilerInterface& compilerInterface
                                   /*MethodDefOrRefSignaturePtr methodSignature*/);


private:
    // Only MethodCompiler can safely calculate arguments positions
    friend class MethodCompiler;

    /*
     * From a method signature calculate the position and length of arguments.
     * See ArgumentsPositions.
     *
     * compiler  - The compiler settings
     * signature - The signature for the method
     * args      - Will be filled with the method arguments positions array
     */
    static void calculateArgumentsSizes(const CompilerInterface& compiler,
                                        ApartmentPtr& apartment,
                                        const MethodCompiler& methodCompiler,
                                        const MethodDefOrRefSignature& signature,
                                        ArgumentsPositions& args,
                                        ElementType& thisType);

    /*
     * Compare 2 elements for arguments
     */
    static bool compareElements(ResolverInterface& resolver,
                                const ElementType& a,
                                const StackEntity& b);

    // CIL method prefix
    static const char gCILMethodPrefix[];
    // CIL token prefix
    static const char gCILTokenPrefix[];
    // CIL global prefix
    static const char gCILGlobalPrefix[];
    static const char gCILGlobalDataPrefix[];

    static const char gCallingConventionCustomAttributeName[];
    static const char gCallingArgumentName[];
    static const char gCallingConventionStdcall[];
    static const char gCallingConventionCdecl[];
    static const char gCallingConventionFastcall[];

    // Map between custom attribute value to method convention enum.
    static CompilerInterface::CallingConvention
                 mapCustomAttributeNametoMethodEnum(const cString& value);
};

#endif // __TBA_CLR_COMPILER_CALLINGCONVENTION_H

