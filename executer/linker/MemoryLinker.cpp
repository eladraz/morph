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
* MemoryLinker.cpp
*
* Implementation file
*/
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/data/endian.h"
#include "xStl/stream/traceStream.h"
#include "xStl/stream/ioStream.h"
#include "xStl/os/os.h"
#include "compiler/CallingConvention.h"
#include "executer/runtime/Executer.h"
#include "executer/linker/MemoryLinker.h"
#include "executer/ExecuterTrace.h"
#include "format/EncodingUtils.h"
#include "runnable/GlobalContext.h"
#include "runnable/StringRepository.h"
#ifdef _MSC_VER
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

#ifdef XSTL_LINUX
    #include <sys/mman.h>
    #include <errno.h>
#endif

MemoryLinker::MemoryLinker(CompilerEngineThread& compilerEngineThread,
    ApartmentPtr apartment) :
    LinkerInterface(compilerEngineThread, apartment)
{
    // Prepare the data-section
    cForkStreamPtr forked = m_apartment->getStreams().getUserStringsStream()->fork();
    forked->seek(0, basicInput::IO_SEEK_SET);
    forked->readAllStream(m_stringTable);

    m_area = NULL;
    m_offset = 0;
}

addressNumericValue MemoryLinker::bind(SecondPassBinary& pass)
{
    void *code = pass.getData().getBuffer();
    addressNumericValue addr = getNumeric(code);
    if (m_reloc.hasKey(addr)) {
        return m_reloc[addr];
    }
    else {
        addressNumericValue new_addr = getNumeric((char*)m_area + m_offset);
        m_reloc.append(addr, new_addr);
        m_offset += pass.getData().getSize();
        return new_addr;
    }
}

void MemoryLinker::allocate()
{
    m_offset = 0;

    MethodTransTable transTable = m_engine.getBinaryRepository().getMethodTransTable();
    const cList<TokenIndex>& transKeys(transTable.keys());
    cList<TokenIndex>::iterator i = transKeys.begin();
    for (;  i != transKeys.end(); ++i)
    {
        SecondPassBinaryPtr methodPtr(transTable[*i]);
        m_offset += methodPtr->getData().getSize();
    }

#ifdef XSTL_WINDOWS
    m_area = VirtualAlloc(0, m_offset, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#elif defined XSTL_LINUX

#ifdef XSTL_64BIT
    m_area = MAP_FAILED;
#else
    m_area = mmap(NULL, m_offset, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE  | MAP_ANONYMOUS, -1, 0);
#endif

    if (m_area == MAP_FAILED)
    {
        cout << "Cannot create executable code (errno): " << errno << endl;
        CHECK_FAIL();
    }
#else
    m_area = cOS::smallMemoryAllocation(m_offset);
    #error "Please add execution allocation right\n"
#endif

    // now that a new area has been allocated, we should start all over... :(
    m_offset = 0;
    m_reloc.removeAll();

    m_vtblBuffer.changeSize(m_apartment->getObjects().getTypedefRepository().
                                estimateVirtualTableMaximumSize(sizeof(addressNumericValue)));
}

void MemoryLinker::relocate()
{
    MethodTransTable transTable = m_engine.getBinaryRepository().getMethodTransTable();
    const cList<TokenIndex>& transKeys(transTable.keys());
    cList<TokenIndex>::iterator i = transKeys.begin();
    for (;  i != transKeys.end(); ++i)
    {
        SecondPassBinaryPtr methodPtr(transTable[*i]);
        void *code = methodPtr->getData().getBuffer();
        addressNumericValue addr = getNumeric(code);
        if (m_reloc.hasKey(addr)) {
            void *copy_to = getPtr(m_reloc[addr]);
            memcpy(copy_to, code, methodPtr->getData().getSize());
        }
    }
}

void MemoryLinker::resolveAndExecuteAllDependencies(TokenIndex& mainMethod)
{
    // Prepare the global table
    m_staticTable = cBuffer(m_apartment->getObjects().getTypedefRepository().getStaticTotalLength());
    m_staticDataTable = m_apartment->getObjects().getTypedefRepository().getDataSection();
    memset(m_staticTable.getBuffer(), 0, m_staticTable.getSize());

    SecondPassBinaryPtr mainMethodPtr(m_engine.getBinaryRepository().
                                        getSecondPassMethod(mainMethod));

    allocate();
    // Replacing all dependencies
    resolve(mainMethod);
    relocate();

    // And execute the main method
    XSTL_TRY
    {
        ExecuterTrace("Stage: executing " << HEXTOKEN(mainMethod) << endl);
        addressNumericValue addr = bind(*mainMethodPtr);

#ifdef _MSC_VER
#ifdef _DEBUG
        _CrtMemState cp;
        _CrtMemCheckpoint(&cp);
#endif
#endif
        int32 value = Executer::execute(addr);
#ifdef _MSC_VER
#ifdef _DEBUG
        _CrtMemDumpAllObjectsSince(&cp);
#endif
#endif
        ExecuterTrace("Stage: completed - " << HEXDWORD(value) << endl);
    }
    XSTL_CATCH_ALL
    {
        ExecuterTrace("EXCEPTION!" << endl);
    }
}

void MemoryLinker::resolve(const TokenIndex& methodIndex)
{
    MethodResolvedObject solved;
    MethodStackObject stack;
    stack.push(methodIndex);
    resolve(stack, solved);
}

/**
* Recursively processes methods' dependencies to replace symbols with actual
* addresses.
*/

void MemoryLinker::resolve(MethodStackObject& stack, MethodResolvedObject& resolved)
{
    cList<addressNumericValue> all_functions;

    // The breaking rule
    while (!stack.isEmpty())
    {
        // Get the binary
        TokenIndex nextMethod(stack.getArg(0));
        stack.pop2null();
        // Check for already resolved method
        if (resolved.hasKey(nextMethod))
            continue;


        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(nextMethod));
        SecondPassBinary& binary = *binaryPtr;
        addressNumericValue binaryAddress = bind(binary);

        // Scan the method and resolve it
        const BinaryDependencies::DependencyObjectList& dependencies = binary.getDependencies().getList();
        BinaryDependencies::DependencyObjectList::iterator i = dependencies.begin();

        // ExecuterResolveTrace("Resolving method: " << HEXDWORD(getTokenID(nextMethod)) << ":" << HEXDWORD(getApartmentID(nextMethod)) << endl);

        for (; i != dependencies.end(); ++i)
        {
            // Check for CIL methods links
            addressNumericValue addr = 0;
            BinaryDependencies::DependencyObject& object = *i;
            uint globalIndex;
            ResolverInterface& resolver = m_apartment->getObjects().getTypedefRepository();

            // ExecuterResolveTrace("\tTrying to resolve dependency: " << object.m_name << endl);

            TokenIndex methodToken;
            if (CallingConvention::deserializeMethod(object.m_name, methodToken))
            {
                addressNumericValue addr = 0;

                // Check for external method
                if (!m_externalModuleResolver.resolveExternalMethod(methodToken, addr))
                {
                    // Not external method
                    // Maybe clrResolver should be applied ?
                    methodToken = m_clrResolver.resolve(m_apartment, methodToken);

                    // . Compile the method
                    // Check if the method wasn't resolved yet.
                    if (!resolved.hasKey(methodToken))
                    {
                        // Add the method to the queue
                        stack.push(methodToken);
                    }


                    if (!m_engine.getBinaryRepository().isMethodExist(methodToken))
                    {
                        // Throw a nice exception
                        ExecuterResolveTrace("\tMETHOD NOT FOUND!" << HEXTOKEN(methodToken) << endl);
                        CHECK_FAIL();
                    }

                    // Resolve the address
                    SecondPassBinaryPtr methodSecondPtr(m_engine.getBinaryRepository().getSecondPassMethod(methodToken));
                    addr = bind(*methodSecondPtr);
                }
                else {
                    // ExecuterResolveTrace("\tExternal method detected." << endl);
                }

                // ExecuterResolveTrace("\tMethod binded to addr: " << HEXDWORD(addr) << endl);
                binary.resolveDependency(object, binaryAddress, addr);
                // ExecuterResolveTrace("\tMethod resolved" << endl);
            }
            else if (CallingConvention::deserializeGlobalData(object.m_name, globalIndex))
            {
                addressNumericValue addr = getNumeric(m_staticDataTable.getBuffer()) + globalIndex;
                // ExecuterResolveTrace("\tGlobal binded to addr: " << HEXDWORD(addr) << endl);
                binary.resolveDependency(object, binaryAddress, addr);
               //  ExecuterResolveTrace("\tGlobal resolved" << endl);
            }/*
            else if (CallingConvention::deserializeGlobal(object.m_name, globalIndex))
            {
                addressNumericValue addr = getNumeric(m_staticTable.getBuffer()) + globalIndex;
                // ExecuterResolveTrace("\tGlobal binded to addr: " << HEXDWORD(addr) << endl);
                binary.resolveDependency(object, binaryAddress, addr);
               //  ExecuterResolveTrace("\tGlobal resolved" << endl);
            }*/
            else if (CallingConvention::deserializeToken(object.m_name, methodToken))
            {
                mdToken methodID = getTokenID(methodToken);
                if (EncodingUtils::getTokenTableIndex(methodID) == TABLE_FIELD_TABLE)
                {
                    // Static field token
                    addressNumericValue addr = getNumeric(m_staticTable.getBuffer() + getStaticAddress(methodToken));
                    // ExecuterResolveTrace("\tGlobal binded to addr: " << HEXDWORD(addr) << endl);
                    binary.resolveDependency(object, binaryAddress, addr);
                    //  ExecuterResolveTrace("\tGlobal resolved" << endl);
                } else if (EncodingUtils::getTokenTableIndex(methodID) == 0x70)
                {
                    // Strings should get here
                    CHECK_FAIL();
                } else if (EncodingUtils::getTokenTableIndex(methodID) == TABLE_TYPEDEF_TABLE)
                {
                    // Adding virtual table pointer
                    addressNumericValue addr = getVtblAddress(methodToken);
                    if (addr == 0)
                    {
                        // Build new vtbl
                        addr = m_vtblFilledSize + getNumeric(m_vtblBuffer.getBuffer());
                        // Adding parents table
                        const ResolverInterface::ParentDictonary& parents = resolver.getParentDirectory(methodToken);
                        cList<TokenIndex> parentsId;
                        parents.keys(parentsId);
                        uint16 ancestorLength = 0;
                        uint16* l = (uint16*)getPtr(addr);

                        bool isLittleEndian =
                            #ifdef XSTL_LITTLE_ENDIAN
                                true;
                            #else
                                false;
                            #endif // XSTL_LITTLE_ENDIAN
                        for (cList<TokenIndex>::iterator j = parentsId.begin();
                             j != parentsId.end();
                             ++j, ancestorLength++)
                        {
                            // write the rtti
                            cEndian::writeUint16((uint8*)l, resolver.getRTTI(*j), isLittleEndian); l++;
                            // write offset
                            cEndian::writeUint16((uint8*)l, parents[*j], isLittleEndian); l++;
                            m_vtblFilledSize+= 4;
                        }
                        // Adding self rtti
                        ancestorLength++;
                        cEndian::writeUint16((uint8*)l, resolver.getRTTI(methodToken), isLittleEndian); l++;
                        // write offset
                        cEndian::writeUint16((uint8*)l, 0, isLittleEndian); l++;
                        m_vtblFilledSize+= 4;

                        // Adding size
                        cEndian::writeUint16((uint8*)l, ancestorLength, isLittleEndian); l++;
                        // Adding object-size
                        CHECK(resolver.getTypeSize(methodToken) < 0x10000);
                        cEndian::writeUint16((uint8*)l, (uint16)resolver.getTypeSize(methodToken), isLittleEndian); l++;
                        m_vtblFilledSize+= 4;

                        // Setting addr
                        addr = getNumeric(l);
                        setVtblAddress(methodToken, addr);
                        const ResolverInterface::VirtualTable& vTbl =
                                m_apartment->getObjects().getTypedefRepository().getVirtualTable(methodToken);
                        ResolverInterface::VirtualTable::iterator i = vTbl.begin();

                        addressNumericValue* ftbl = (addressNumericValue*)(l);

                        for (; i != vTbl.end(); i++)
                        {
                            TokenIndex func = getVtblMethodIndexOverride(*i);
                            stack.push(func);
                            SecondPassBinaryPtr spt = m_engine.getBinaryRepository().getSecondPassMethod(func);
                            addressNumericValue binaryAddress = bind(*spt);
                            // And write it flat to the vtbl
                            *ftbl = binaryAddress;
                            ftbl++;
                            m_vtblFilledSize += sizeof(addressNumericValue);
                        }
                    }

                    // Just bind
                    // ExecuterResolveTrace("\tVtbl binded to addr: " << HEXDWORD(addr) << endl);
                    binary.resolveDependency(object, binaryAddress, addr);
                    // ExecuterResolveTrace("\tToken resolved" << endl);
                } else if (EncodingUtils::getTokenTableIndex(methodID) == 0xFF) // TODO! Add enum/define
                {
                    // TODO!
                    CHECK_FAIL();
                } else
                {
                    ExecuterResolveTrace("\tDefaultCompilerAlgorithm::resolve(): ERROR! "
                        "Unknown token ID: " << HEXDWORD(methodID) << endl);
                    CHECK_FAIL();
                }
            }
#ifdef CLR_UNICODE
            } else if (m_apartment->getObjects().getStringRepository().deserializeStringA((*j).m_name, globalIndex))
            {
                // TODO! Add unicode support
                CHECK_FAIL();
            }
#else
            else if (m_apartment->getObjects().getStringRepository().deserializeStringA(object.m_name, globalIndex))
            {
                addr = getNumeric(m_apartment->getObjects().getStringRepository().getAsciiStringRepository().getBuffer() + globalIndex);
                // ExecuterResolveTrace("\tString binded to addr: " << HEXDWORD(addr) << endl);
                binary.resolveDependency(object, binaryAddress, addr);
                // ExecuterResolveTrace("\tToken resolved" << endl);
            }
#endif // CLR_UNICODE
            else
            {
                // ExecuterResolveTrace("\tCannot resolve: " << object.m_name << endl);
            }

            /*
            * Most of the dependency, such as basic-block and debug information
            * are custom.
            *
            else
            {
            ExecuterTrace("DefaultCompilerAlgorithm::resolve(): ERROR! "
            "Unknown dependency name!" << endl);
            CHECK_FAIL();
            }
            */
        }

        // The method was resolved OK.
        resolved.append(nextMethod, true);
    }
}
