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
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/enc/digest/Crc64.h"
#include "executer/compiler/PrecompiledRepository.h"
#include "executer/ExecuterTrace.h"
#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"

const char PrecompiledRepository::gRepositoryHeader[] = "TBAMORPH\r\n";

PrecompiledRepository::PrecompiledRepository(const ApartmentPtr& mainApartment) :
    m_mainApartment(mainApartment)
{
}

SecondPassBinaryPtr
        PrecompiledRepository::getPrecompiledMethod(const cString& apartmentName,
                                                    const cBuffer& signature) const
{
    cLock lock(m_lock);
    // Look for the apartment object
    if (!m_apartments.hasKey(apartmentName))
        return SecondPassBinaryPtr();

    const ApartmentSignature& methods = m_apartments[apartmentName];
    if (!methods.hash.hasKey(signature))
        return SecondPassBinaryPtr();
    const MethodSignature& sig = methods.hash[signature];
    sig.time = cOS::getSystemTime();
    return sig.binary;
}

void PrecompiledRepository::appendPrecompiledMethod(const cString& apartmentName,
                                                    const cBuffer& signature,
                                                    const SecondPassBinaryPtr& compiledFunction)
{
    cLock lock(m_lock);
    // Look for the apartment object
    MethodSignature sig;
    sig.binary = compiledFunction;
    sig.time = cOS::getSystemTime();

    if (!m_apartments.hasKey(apartmentName))
    {
        ApartmentSignature methods;
        methods.hash.append(signature, sig);
        m_apartments.append(apartmentName, methods);
    } else
    {
        ApartmentSignature& methods(m_apartments[apartmentName]);
        if (!methods.hash.hasKey(signature))
        {
            methods.hash.append(signature, sig);
        } else
        {
            // Combine functions. TODO!
            // ExecuterTrace("Warning! Dual method with same signature has been detected!" << endl);
        }
    }
}

bool PrecompiledRepository::isValid() const
{
    return true;
}

void PrecompiledRepository::deserialize(basicInput& input)
{
    uint8 signatureLen;
    uint32 aptLen;
    char header[sizeof(gRepositoryHeader)];
    input.pipeRead(header, sizeof(header));
    CHECK(memcmp(header, gRepositoryHeader, sizeof(header)) == 0);
    // Read signature length (CRC64)
    input.streamReadUint8(signatureLen);
    CHECK(signatureLen == sizeof(uint64));
    input.streamReadUint32(aptLen);

    // Release old repository
    m_apartments = cHash<cString, ApartmentSignature>();

    for (uint32 i = 0; i < aptLen; i++)
    {
        // Read apartment name
        cString aptName(input.readUnicodeNullString());
        // Read unique generator value
        uint32 helperIndex;
        input.streamReadUint32(helperIndex);
        ApartmentPtr apt = m_mainApartment->getApartmentByName(aptName);
        if (!apt.isEmpty())
            apt->setMethodHelperRow(helperIndex);
        // Read methods length
        uint32 methodsLen;
        m_apartments.append(aptName, ApartmentSignature());
        input.streamReadUint32(methodsLen);
        ApartmentSignature& methods(m_apartments[aptName]);
        methods.apartmentHelperNumber = helperIndex;
        for (uint j = 0; j < methodsLen; j++)
        {
            cBuffer msig;
            MethodSignature data;
            input.pipeRead(msig, signatureLen);
            input.pipeRead(&data.time, sizeof(data.time));
            data.binary = SecondPassBinaryPtr(new SecondPassBinary(input));
            methods.hash.append(msig, data);
        }
    }
}

void PrecompiledRepository::serialize(basicOutput& output) const
{
    cList<cString> apartments;
    m_apartments.keys(apartments);
    cList<cString>::iterator i(apartments.begin());

    // Write function header
    output.pipeWrite(gRepositoryHeader, sizeof(gRepositoryHeader));
    // Write sizeof signature (CRC64)
    output.streamWriteUint8(sizeof(uint64));
    // Write number of apartments
    output.streamWriteUint32((uint32)apartments.length());

    for (; i != apartments.end(); ++i)
    {
        // Write apartment name
        output.writeUnicodeNullString(*i);
        // Write unique generator value
        const ApartmentSignature& methods(m_apartments[*i]);
        ApartmentPtr apt = m_mainApartment->getApartmentByName(*i);
        if (!apt.isEmpty())
            output.streamWriteUint32(apt->generateMethodHelperRow());
        else
            output.streamWriteUint32(methods.apartmentHelperNumber);

        // Start writing methods
        cList<cBuffer> signatures;
        methods.hash.keys(signatures);
        output.streamWriteUint32(signatures.length());
        cList<cBuffer>::iterator j(signatures.begin());
        for (; j != signatures.end(); ++j)
        {
            // Write signature
            output.pipeWrite(*j, sizeof(uint64));
            // Write second pass binary
            const MethodSignature& m = methods.hash[*j];
            // Write time
            output.pipeWrite(&m.time, sizeof(m.time));
            m.binary->serialize(output);
        }
    }
}
