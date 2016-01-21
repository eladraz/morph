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
 * LocalVarSignature.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/stream/basicIO.h"
#include "data/ElementType.h"
#include "format/signatures/LocalVarSignature.h"
#include "format/EncodingUtils.h"

LocalVarSignature::LocalVarSignature(basicInput& stream, mdToken apartmentId, const ResolverInterface& resolverInterface)
{
    // Read the number of bytes this signature takes
    /* uint byteLength = */ EncodingUtils::readCompressedNumber(stream);

    uint8 signature;
    stream.streamReadUint8(signature);
    CHECK(signature == LOCAL_VAR_SIGNATURE);

    uint count = EncodingUtils::readCompressedNumber(stream);
    m_locals.changeSize(count);

    for (uint i = 0; i < count; i++)
    {
        m_locals[i] = ElementType::readType(stream, apartmentId);
        resolverInterface.resolveTyperef(m_locals[i]);
    }
}

LocalVarSignature::LocalVarSignature()
{
}

const ElementsArrayType& LocalVarSignature::getLocals() const
{
    return m_locals;
}

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const LocalVarSignature& object)
{
    uint32 count = object.getLocals().getSize();
    out << ".locals (";
    for (uint32 i = 0; i < count; i++)
    {
        out << object.getLocals()[i] << " V";
        out << i;
        if (i == (count - 1))
            out << ")";
        else
            out << ",";
        out << endl;
    }
    return out;
}
#endif // TRACED_CLR
