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

#ifndef __TBA_CLR_RUNNABLE_STRINGREADER_H
#define __TBA_CLR_RUNNABLE_STRINGREADER_H

/*
 * StringReader.h
 *
 * Read a string from the #String stream
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/memoryAccesserStream.h"
#include "format/coreHeadersTypes.h"

/*
 * Read a string from the #String stream
 */
class StringReader {
public:
    /*
     * Read a string from the #String stream
     *
     * stringStream - The #String index
     * index        - The position inside the #String stream of which the name
     *                is encoded
     *
     * NOTE: This function is thread safe! The implementation fork a new stream
     *       and seek into position
     *
     * Return the name of the string as encoded inside the #String table
     * Throw general exception if stream overflow occurred.
     */
    static cString readStringName(const cMemoryAccesserStreamPtr& stringStream,
                                  mdToken index);
    static cString readStringName(basicInput& stringStream,
                                  mdToken index);

private:
    // The maximum number of characters for entry in the #Strings.
    // Equal to MAX_PATH + some safity value
    enum { MAX_STRING = 270 };
};

#endif // __TBA_CLR_RUNNABLE_STRINGREADER_H
