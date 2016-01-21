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
 * MethodDefOrRefSignature.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/data/array.h"
#include "xStl/stream/basicIO.h"
#include "xStl/stream/stringerStream.h"
#include "data/ElementType.h"
#include "format/EncodingUtils.h"
#include "format/signatures/MethodDefOrRefSignature.h"

MethodDefOrRefSignature::MethodDefOrRefSignature(basicInput& stream, mdToken apartmentId, const ResolverInterface& resolverInterface) :
    m_hasThis(false),
    m_explicit(false),
    m_callingConvention(CALLCONV_DEFAULT)
{
    // Read the number of bytes this signature takes
    uint byteLength = EncodingUtils::readCompressedNumber(stream);

    bool stop = false;

    // Read header
    while (!stop)
    {
        uint8 data;
        stream.streamReadUint8(data);
        switch (data)
        {
        case HASTHIS: CHECK(!m_hasThis); m_hasThis = true;
            stop = true; // TODO! Documentation error?
            break;
        case EXPLICITTHIS: CHECK(!m_explicit); m_explicit = true; break;

        case CALLCONV_DEFAULT: stop = true; break;
        case CALLCONV_VARARG: m_callingConvention = CALLCONV_VARARG; stop = true; break;
        case UNMANAGED_CALLCONV_CDECL: m_callingConvention = UNMANAGED_CALLCONV_CDECL;  stop = true; break;
        case UNMANAGED_CALLCONV_STDCALL: m_callingConvention = UNMANAGED_CALLCONV_STDCALL;  stop = true; break;
        case UNMANAGED_CALLCONV_FASTCALL: m_callingConvention = UNMANAGED_CALLCONV_FASTCALL;  stop = true; break;
        case UNMANAGED_CALLCONV_THISCALL: m_callingConvention = UNMANAGED_CALLCONV_THISCALL;  stop = true; break;

        // MS proprity. Usuall come with IMAGE_CEE_CS_CALLCONV_PROPERTY for generic fields. Irrelevant
        case IMAGE_CEE_CS_CALLCONV_FIELD: m_callingConvention = IMAGE_CEE_CS_CALLCONV_FIELD;  stop = true; break;
        default:
            CHECK_FAIL();
        }
    }

    if (m_callingConvention == IMAGE_CEE_CS_CALLCONV_FIELD)
        return;

    uint paramCount = EncodingUtils::readCompressedNumber(stream);
    m_returnType = ElementType::readType(stream, apartmentId);
    resolverInterface.resolveTyperef(m_returnType);
    m_params.changeSize(paramCount);
    for (uint i = 0; i < paramCount; i++)
    {
        m_params[i] = ElementType::readType(stream, apartmentId);
        resolverInterface.resolveTyperef(m_params[i]);
    }
}

MethodDefOrRefSignature::MethodDefOrRefSignature(ElementType returnType,
                                                 ElementsArrayType params,
                                                 bool hasThis,
                                                 bool explicitThis,
                                                 CallingConvention callingConvention) :
    m_hasThis(hasThis),
    m_explicit(explicitThis),
    m_callingConvention(callingConvention),
    m_returnType(returnType),
    m_params(params)
{
}

const ElementType& MethodDefOrRefSignature::getReturnType() const
{
    return m_returnType;
}

const ElementsArrayType& MethodDefOrRefSignature::getParams() const
{
    return m_params;
}

bool MethodDefOrRefSignature::isHasThis() const
{
    return m_hasThis;
}

bool MethodDefOrRefSignature::isExplicitThis() const
{
    return m_explicit;
}

MethodDefOrRefSignature::CallingConvention
    MethodDefOrRefSignature::getCallingConvention() const
{
    return m_callingConvention;
}

bool MethodDefOrRefSignature::operator == (
                                    const MethodDefOrRefSignature& other) const
{
    bool ok = (m_hasThis == other.m_hasThis) &&
              (m_callingConvention == other.m_callingConvention) &&
              (m_params.getSize() == other.m_params.getSize()) &&
              (m_returnType == other.m_returnType);
    if (!ok)
        return false;

    for (uint i = 0; i < m_params.getSize(); i++)
    {
        if (m_params[i] != other.m_params[i])
            return false;
    }

    return true;
}

MethodDefOrRefSignature::MethodDefOrRefSignature(const ElementType& returnedType) :
    m_returnType(returnedType),
    m_hasThis(false),
    m_callingConvention(CALLCONV_DEFAULT)
{
}

void MethodDefOrRefSignature::hashSignature(cDigest& digest, const ResolverInterface& resolver) const
{
    m_returnType.hashElement(digest, resolver);
    digest.update(&m_callingConvention, sizeof(m_callingConvention));
    digest.update(&m_hasThis, sizeof(m_hasThis));
    uint count = getParams().getSize();
    for (uint i = 0; i < count; i++)
    {
        m_params[i].hashElement(digest, resolver);
    }
}

void MethodDefOrRefSignature::appendArgument(const ElementType& newParam)
{
    uint size = m_params.getSize();
    m_params.changeSize(size + 1);
    m_params[size] = newParam;
}

#ifdef TRACED_CLR
cStringerStream& operator << (cStringerStream& out,
                              const MethodDefOrRefSignature& object)
{
    if (object.isHasThis())
        out << "instance ";

    if (object.isExplicitThis())
        out << "explicit ";

    switch (object.getCallingConvention())
    {
    case MethodDefOrRefSignature::CALLCONV_DEFAULT: break;
    case MethodDefOrRefSignature::CALLCONV_VARARG: out << "vararg "; break;
    case MethodDefOrRefSignature::UNMANAGED_CALLCONV_CDECL: out << "cdecl "; break;
    case MethodDefOrRefSignature::UNMANAGED_CALLCONV_STDCALL: out << "stdcall "; break;
    case MethodDefOrRefSignature::UNMANAGED_CALLCONV_FASTCALL: out << "fastcall "; break;
    case MethodDefOrRefSignature::UNMANAGED_CALLCONV_THISCALL: out << "thiscall "; break;
    case MethodDefOrRefSignature::IMAGE_CEE_CS_CALLCONV_FIELD: out << ".field "; break;
    default:
        CHECK_FAIL();
    }

    out << object.getReturnType() << " (*)(";

    uint count = object.getParams().getSize();
    for (uint i = 0; i < count; i++)
    {
        out << object.getParams()[i];
        if (i < (count - 1))
            out << ", ";
    }
    out << ");";
    return out;
}
#endif // TRACED_CLR
