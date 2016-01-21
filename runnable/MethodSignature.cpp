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

#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/os/osrand.h"
#include "xStl/enc/random/prf.h"
#include "xStl/enc/digest/Crc64.h"
#include "xStl/stream/traceStream.h"
#include "data/exceptions.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringReader.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/StandAloneSigTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/TypeSpecTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/FieldTable.h"
#include "format/tables/FieldRVATable.h"
#include "format/signatures/LocalVarSignature.h"
#include "format/MSILScanInterface.h"
#include "runnable/MethodRunnable.h"
#include "runnable/RunnableTrace.h"
#include "runnable/MethodSignature.h"
#include "compiler/CompilerEngine.h"
#include "compiler/CallingConvention.h"

class MethodScanAndSignDependencies : public MSILScanInterface
{
public:
    MethodScanAndSignDependencies(ApartmentPtr& mainApartment, uint apartmentId, cDigest& digest);
    virtual void OnToken(mdToken token, bool bLdToken = false);

protected:
    ApartmentPtr& m_mainApartment;
    uint m_apartmentId;
    cDigest& m_digest;
    static void hashToken(ApartmentPtr& mainApartment,
                                uint apartmentId,
                                mdToken token,
                                cDigest& digest);
};

MethodScanAndSignDependencies::MethodScanAndSignDependencies(ApartmentPtr& mainApartment, uint apartmentId, cDigest& digest) :
    m_mainApartment(mainApartment),
    m_apartmentId(apartmentId),
    m_digest(digest)
{
}

void MethodScanAndSignDependencies::OnToken(mdToken token, bool bLdToken)
{
    if (bLdToken)
    {
        // Add random, so the function will recompile every time
        cBuffer random;
        cOSRand::generateRandom(8, random);
        cOSDef::systemTime now = cOS::getSystemTime();
        m_digest.updateStream(random);
        m_digest.update(&now, sizeof(now));
    }
    else
        hashToken(m_mainApartment, m_apartmentId, token, m_digest);
}

cBuffer MethodSignature::getMethodSignature(ApartmentPtr& mainApartment,
                                            MethodRunnable& methodRunnable,
                                            uint compilerType)
{
    CRC64 crc64;
    // Update compiler type
    crc64.update(&compilerType, sizeof(compilerType));
    // Update method signature
    ResolverInterface& resolver = mainApartment->getObjects().getTypedefRepository();
    methodRunnable.getMethodSignature().hashSignature(crc64, resolver);

    // Update token... unfortunately
    crc64.update(&methodRunnable.getMethodToken(), sizeof(TokenIndex));

    // Update locals
    ElementsArrayType& locals = methodRunnable.getLocals();
    for (uint i = 0; i < locals.getSize(); i++)
        locals[i].hashElement(crc64, resolver);

    // Update method data (Only for non-interface methods)
    if (!methodRunnable.isEmptyMethod())
    {
        cForkStreamPtr stream = methodRunnable.getStreamPointer()->fork();
        stream->seek(methodRunnable.getMethodStreamStartAddress(),
                     basicInput::IO_SEEK_SET);
        cBuffer methodData;
        stream->pipeRead(methodData, methodRunnable.getMethodHeader().getFunctionLength());
        crc64.updateStream(methodData);
        // Start parsing MSIL opcodes and check for dependencies inside the code
        const uint8* msil = methodData.getBuffer();
        const uint size = methodData.getSize();

        MethodScanAndSignDependencies scanner(mainApartment, getApartmentID(methodRunnable.getMethodToken()), crc64);
        scanner.scanMSIL(msil, size);
    }

    return crc64.digest();
}

void MethodScanAndSignDependencies::hashToken(ApartmentPtr& mainApartment,
                                uint apartmentId,
                                mdToken token,
                                cDigest& digest)
{
    TokenIndex t = buildTokenIndex(apartmentId, token);
    Apartment& apartment = *mainApartment->getApt(t);
    //const MetadataTables& tables = apartment.getTables();
    ResolverInterface& resolver = apartment.getObjects().getTypedefRepository();
    // Cast according to type of token
    switch (EncodingUtils::getTokenTableIndex(token))
    {
    case TABLE_TYPEREF_TABLE:
    case TABLE_TYPEDEF_TABLE:
    case TABLE_TYPESPEC_TABLE:
        {
            ElementType type;
            CompilerEngine::resolveTypeToken(token, apartmentId, resolver, type);
            type.hashElement(digest, resolver);
        }
        break;

    case TABLE_MEMBERREF_TABLE:
    case TABLE_METHOD_TABLE:
        {
            MethodDefOrRefSignaturePtr methodSignature = CallingConvention::readMethodSignature(apartment, token);
            methodSignature->hashSignature(digest, resolver);
        }
        break;
/*
    case TABLE_FIELD_TABLE: // Field RVA
        {
            TablePtr table(tables.getTableByToken(token));
            const FieldTable::Header& fheader = ((FieldTable&)(*(table))).getHeader();
            if (!(fheader.m_flags & FieldTable::fdHasFieldRVA))
            {
                RunnableTrace("MethodSignature::hashToken(): Loading field token that has no RVA" << endl);
                CHECK_FAIL();
            } // else
            // Read FieldRVA and field signature for the type of the field.
            RowTablesPtr fieldsRvas = tables.byTableID(TABLE_FIELDRVA_TABLE);
            uint i = 0;
            for (; i < fieldsRvas.getSize(); i++)
            {
                const FieldRVATable::Header& rvaHeader = ((FieldRVATable&)(*fieldsRvas[i])).getHeader();
                if (rvaHeader.m_field == token)
                {
                    // Get RVA locations
                    cForkStreamPtr data = apartment.getLayout()->
                        getVirtualStream(rvaHeader.m_fieldRva);
                    // Get type size
                    TokenIndex fieldToken(buildTokenIndex(apartment.getUniqueID(), token));
                    ElementType fType = resolver.getFieldType(fieldToken, ElementType::UnresolvedTokenIndex);
                    uint size = resolver.getTypeSize(fType);

                    cBuffer buffer;
                    data->pipeRead(buffer, size);
                    digest.updateStream(buffer);
                    break;
                }
            }

            if (i == fieldsRvas.getSize())
            {
                RunnableTrace("MethodSignature::hashToken(): Cannot find FieldRVA for token " << HEXDWORD(token) << endl);
                CHECK_FAIL();
            }
        }
        break;
*/
    case 0x70: // string
        // Add string and get length
        digest.update(mainApartment->getObjects().getStringRepository().getAsciiStringRepository().getBuffer() +
                      mainApartment->getObjects().getStringRepository().getStringOffset(t),
                      mainApartment->getObjects().getStringRepository().getStringLength(t));
        break;

    default:
        RunnableTrace("MethodSignature::hashToken() - Warning! unknown token! " << HEXDWORD(token) << endl);
    }
}
