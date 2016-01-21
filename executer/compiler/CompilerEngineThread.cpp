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
 * CompilerEngineThread.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/os/thread.h"
#include "xStl/os/os.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "xStl/stream/fileStream.h"
#include "xStl/enc/digest/Crc64.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "compiler/MethodCompiler.h"
#include "runnable/Apartment.h"
#include "runnable/MethodSignature.h"
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/ExecuterTrace.h"
#include "runnable/GlobalContext.h"

CompilerEngineThread::CompilerEngineThread(
                        CompilerFactory::CompilerType compilerType,
                        const CompilerParameters& compilerParams,
                        const ApartmentPtr& mainApartemnt,
                        const cString& repositoryFilename) :
    m_compilerType(compilerType),
    m_compilerParams(compilerParams),
    m_main(mainApartemnt),
    m_repositoryFilename(repositoryFilename),
    m_precompiledRepository(mainApartemnt)
{
    // Read repository file
    XSTL_TRY
    {
        if (m_repositoryFilename.length() > 0)
        {
            ExecuterTrace("CompilerEngineThread: Reading repository file: " << m_repositoryFilename << endl);
            cFileStream repFile(m_repositoryFilename);
            m_precompiledRepository.deserialize(repFile);
        }
    }
    XSTL_CATCH_ALL
    {
        ExecuterTrace("CompilerEngineThread: Cannot read data to repository file: " <<
            m_repositoryFilename << endl);
    }
}

CompilerFactory::CompilerType CompilerEngineThread::getCompilerType() const
{
    return m_compilerType;
}

void CompilerEngineThread::addNotifier(CompilerNotifierInterface& notifier)
{
    m_notifiers.append(&notifier);
}

void CompilerEngineThread::run(ScanningAlgorithmInterface& scanAlgorithm)
{
    bool shouldExit = false;
    ApartmentPtr apartmentPtr;

    // Trying to get a method from the scanning algorithm
    addressNumericValue currentThread =  getNumeric((void*)(cThread::getCurrentThreadHandle()));
    ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread) << "] " "Entry point called." << endl);

    while (!shouldExit)
    {
        TokenIndex mid;
        if (!scanAlgorithm.getNextMethod(mid))
        {
            // Error getting method
            shouldExit = scanAlgorithm.shouldExit();
            if (!shouldExit)
            {
                // Wait for an event
                ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                    << "] Waiting for event." << endl);
                scanAlgorithm.getSleepEvent().wait();
                ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                    << "] Wakeup!." << endl);
            } else
            {
                // Finish executing
                ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                    << "] Finish." << endl);
                // Save the repository
                if (m_repositoryFilename.length() > 0)
                {
                    XSTL_TRY
                    {
                        cFileStream repFile(m_repositoryFilename, cFile::CREATE | cFile::WRITE);
                        m_precompiledRepository.serialize(repFile);
                    }
                    XSTL_CATCH_ALL
                    {
                        ExecuterTrace("CompilerEngineThread: Cannot save data to repository file: " <<
                            m_repositoryFilename << endl);
                    }
                }
                shouldExit = true;
            }
        }
        else
        {
            SecondPassBinaryPtr pass;

            // Check for recompiled cached method
            if (m_binaryRepository.isMethodExist(mid, &pass))
            {
                //ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                //              << "] Method " << HEXTOKEN(mid) << " is in repository." << endl);
                // Notify that the method was completed
                notifyOnCompiled(mid, *pass, true);
            }
            else
            {
#ifdef _DEBUG
                // A nice spot to start debugging a specific function
                if ((getApartmentID(mid) == 2) && (getTokenID(mid) == 0x6000124))
                {
                    mid = mid;
                    //cOS::debuggerBreak();
                }
#endif

                if (EncodingUtils::getTokenTableIndex(getTokenID(mid)) == TABLE_CLR_METHOD_INSTANCE_DETOR)
                {
                    pass = MethodCompiler::compileInstanceDestructor(*m_main->getApt(mid),
                                                                     m_compilerType, m_compilerParams, mid);
                    m_binaryRepository.addSecondPassMethod(mid, pass);
                    notifyOnCompiled(mid, *pass, false);
                    ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                        << "] Instance destructor DONE - " << HEXTOKEN(mid) << endl);
                } else if (EncodingUtils::getTokenTableIndex(getTokenID(mid)) == TABLE_CLR_CCTOR_WRAPPERS)
                {
                    // Generate method wrapper
                    mdToken wrapper = EncodingUtils::buildToken(TABLE_TYPEDEF_TABLE,
                        EncodingUtils::getTokenPosition(getTokenID(mid)));
                    TokenIndex cctorToken = buildTokenIndex(getApartmentID(mid), wrapper);
                    cctorToken = m_main->getObjects().getTypedefRepository().getStaticInitializerMethod(cctorToken);
                    // Increase static size by a boolean
                    TokenIndex booleanAddress = m_main->getObjects().getTypedefRepository().allocateStatic(1);
                    // And compile
                    pass = MethodCompiler::compileCCTORWrapper(*m_main->getApt(mid),
                        m_compilerType, m_compilerParams, booleanAddress, getTokenID(cctorToken));
                    m_binaryRepository.addSecondPassMethod(mid, pass);
                    notifyOnCompiled(mid, *pass, false);
                    ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                        << "] Static Wrapper DONE - " << HEXTOKEN(mid) << endl);
                } else if ((EncodingUtils::getTokenTableIndex(getTokenID(mid)) == TABLE_MEMBERREF_TABLE) ||
                    EncodingUtils::getTokenTableIndex(getTokenID(mid)) == TABLE_CLR_INTERNAL)
                {
                    if (getApartmentID(mid) != -1) {
                        ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                            << "] is memberref/internal, skipping. " << HEXTOKEN(mid) << endl);

                        ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                            << "] DONE - " << HEXTOKEN(mid) << endl);
                    }
                    // The method belongs to an external module
                    // TODO! Check for module loading...
                    //notifyOnCompiled(mid, apartmentId, *pass, false);
                }
                else if (EncodingUtils::getTokenTableIndex(getTokenID(mid)) == TABLE_CLR_METHOD_HELPERS)
                {
                    // The cleanup and other helper functions must also be in the precompiled repository, calculate simple signature
                    ApartmentPtr apt(m_main->getApt(mid));
                    CRC64 crc64;
                    mdToken token(getTokenID(mid));
                    crc64.update(&token, sizeof(token));
                    cBuffer signature(crc64.digest());
                    pass = m_precompiledRepository.getPrecompiledMethod(apt->getApartmentName(),
                        signature);

                    // If this helper function was not compiled - then this is a bug
                    CHECK(!pass.isEmpty());

                    onMethodCompiled(apt->getApartmentName(), signature, mid, pass, false);
                }
                else
                {
                    // Prepare runnable object
                    ApartmentPtr apt(m_main->getApt(mid));
                    MethodRunnable method(apt);
                    method.loadMethod(getTokenID(mid));

                    // Calculate signature and check old runnable signatures
                    cBuffer signature = MethodSignature::getMethodSignature(apt, method, (uint)m_compilerType);

                    // Check for signature
                    pass = m_precompiledRepository.getPrecompiledMethod(apt->getApartmentName(),
                        signature);
                    if (!pass.isEmpty())
                    {
                        // ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                        //     << "] Method  " << HEXTOKEN(mid) << " in precompiled repository" << endl);
                    }
                    else
                    {
                        // There is no function in the precompiled header, compile
                        ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                            << "] Compiling method " << HEXTOKEN(mid) << endl);

                        XSTL_TRY
                        {
                            // Prepare Method compiler
                            MethodCompiler compiler(m_compilerType, m_compilerParams, apt, getTokenID(mid), method);
                            // And compile
                            pass = compiler.compile(*this);
                            ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                                << "] DONE - " << HEXTOKEN(mid) << endl);
                        }
                        XSTL_CATCH_ALL
                        {
                            ExecuterTrace("CompilerEngineThread [" << HEXADDRESS(currentThread)
                                << "] ERROR! Failed!  " << HEXTOKEN(mid) << endl);
                            // Throw exception? Transfer into virtual mode?
                            // Just notify the algorithm

                            notifyOnCompilationFalied(mid);
                            XSTL_RETHROW;
                        }
                    }
                    // Compilation done
                    if (!pass.isEmpty()) {
                        onMethodCompiled(apt->getApartmentName(), signature, mid, pass, false);
                    }
                }
            }
            // Reference is removed.
        }
    }
}

void CompilerEngineThread::onMethodCompiled(const cString& aptName,
    const cBuffer& signature,
    const TokenIndex& mid,
    const SecondPassBinaryPtr& compiled,
    bool inCache)
{
    m_binaryRepository.addSecondPassMethod(mid, compiled);
    m_precompiledRepository.appendPrecompiledMethod(aptName,
                                                    signature,
                                                    compiled);
    notifyOnCompiled(mid, (SecondPassBinary&)(*compiled), inCache);
}

void onCompilationFailed(const TokenIndex& mid)
{
}

const BinaryGetterInterface& CompilerEngineThread::getBinaryRepository() const
{
    return m_binaryRepository;
}

void CompilerEngineThread::notifyOnCompiled(const TokenIndex& mid,
    SecondPassBinary& compiled,
    bool inCache)
{
    CompilerNotifierInterfaceList::iterator i = m_notifiers.begin();
    for (; i != m_notifiers.end(); ++i)
        (*i)->onMethodCompiled(mid, compiled,inCache);
}

void CompilerEngineThread::notifyOnCompilationFalied(const TokenIndex& mid)
{
    CompilerNotifierInterfaceList::iterator i = m_notifiers.begin();
    for (; i != m_notifiers.end(); ++i)
        (*i)->onCompilationFailed(mid);
}
