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

#ifndef __TBA_CLR_FORMAT_CILFORMATLAYOUT_H
#define __TBA_CLR_FORMAT_CILFORMATLAYOUT_H

/*
 * CilFormatLayout.h
 *
 * An interface for CIL file-format layout
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "xStl/stream/forkStream.h"

/*
 * An interface for CIL file-format layout
 */
class CilFormatLayout {
public:
    // Default destructor. You can inherit from me
    virtual ~CilFormatLayout() {};

    // See CilFormatLayout::getVirtualStream
    enum { DEFAULT_END_ADDRESS = 0xFFFFFFFF };

    /*
     * Return a forkable stream to the beginning of a virtual address 'startRVA'
     * the stream ends at 'endRVA'
     *
     * stratRVA - The relative strat address in the file/memory
     * endRVA   - The relative end address in the file/memory. If this value is
     *            DEFAULT_END_ADDRESS, the stream can access the entire file.
     *
     * Throw exception is endRVA <= startRVA
     */
    virtual cForkStreamPtr getVirtualStream(uint startRVA,
                                            uint endRVA = DEFAULT_END_ADDRESS) = 0;
};

// The reference object counter
typedef cSmartPtr<CilFormatLayout> CilFormatLayoutPtr;

#endif // __TBA_CLR_FORMAT_CILFORMATLAYOUT_H
