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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_RUNTIME_H
#define __TBA_CLR_EXECUTER_RUNTIME_RUNTIME_H


/*
 * Runtime.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

#if defined(XSTL_WINDOWS)
    #pragma warning(push)
    #pragma warning(disable:4200)
#elif defined (XSTL_LINUX)
    #define __cdecl __attribute__((cdecl))
#endif

class Runtime {
public:
    /*
     * Print a string/character on the debug console
     */
    static void __cdecl DebugCh(unichar ch);
    static void __cdecl AssertFalse();

    /*
     * OS/platform independed APIs
     */
    static int __cdecl getTickCount();

    /*
     * Memory handling
     */
    static void* __cdecl allocate(uint size);
    static void  __cdecl free(void* mem);

    /*
     * Unsafe operations
     */
    // Helper methods for using function pointers
    static void  __cdecl callFunction(void* function, void* param);
    static void __cdecl unhandledException(void* exceptionObject);
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_RUNTIME_H

