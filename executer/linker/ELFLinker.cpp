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
 * ELFLinker.cpp
 *
 * Implementation file
 */
#include "executer/stdafx.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerFactory.h"
#include "executer/runtime/Executer.h"
#include "executer/linker/ELFLinker.h"
#include "executer/ExecuterTrace.h"
#include "format/EncodingUtils.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/fileStream.h"
#include "xStl/stream/traceStream.h"

const cString ELFLinker::m_sectionBssString(".bss");
const cString ELFLinker::m_sectionDataString(".data");
const cString ELFLinker::m_sectionRDataString(".rdata");
const cString ELFLinker::m_sectionVtblString(".vtbl");
const cString ELFLinker::m_sectionTextString(".text");
const cString ELFLinker::m_sectionImportString(".import");

// 8kb of code per memory allocation
#define PAGE_SIZE (8192)

// Marco that use for 16 bit to 32 bit alignment
#define alignTo16() { if (m_alignment == 4) { cEndian::writeUint16((uint8*)l, 0, isLittleEndian); l++; m_vtblFilledSize+=2; } }

cString ELFLinker::GlobalObject::buildExportString(cHash<uint, cString> functionNames)
{
    if ((m_sectionTarget == SECTION_TEXT) && (functionNames.hasKey(m_globalIndex)))
    {
        return functionNames[m_globalIndex];
    }

    cString ret;
    switch (m_sectionTarget)
    {
        case SECTION_BSS:   ret = m_sectionBssString;  break;
        case SECTION_DATA:  ret = m_sectionDataString; break;
        case SECTION_RDATA: ret = m_sectionRDataString; break;
        case SECTION_TEXT:  ret = m_sectionTextString; break;
        case SECTION_VTBL:  ret = m_sectionVtblString; break;
    }
    ret[0] = 'g';
    ret += HEXDWORD(m_globalIndex);
    return ret;
}

ELFLinker::ELFLinker(CompilerEngineThread& compilerEngineThread,
                     ApartmentPtr apartment) :
    LinkerInterface(compilerEngineThread, apartment),
    m_elfObj(),
    m_isThumb(FALSE),
    m_secText((uint)0, PAGE_SIZE),
    m_vtblBuffer((uint)0, PAGE_SIZE),
    m_codeSize(0),
    m_importLength(0)
{
    switch (compilerEngineThread.getCompilerType())
    {
    case CompilerFactory::COMPILER_THUMB:
        m_isThumb = TRUE;
    case CompilerFactory::COMPILER_ARM:
        m_globalRelocationType = R_ARM_ABS32;
        m_elfObj.setArch(EM_ARM);
        m_pointerSize = 4;
        m_alignment = 4;
        break;
    case CompilerFactory::COMPILER_IA32:
        m_globalRelocationType = R_386_32;
        m_elfObj.setArch(EM_386);
        m_pointerSize = 4;
        m_alignment = 0;
        break;
    default:
        CHECK_FAIL();
        break;
    }
}

uint ELFLinker::getImportRelocationType(BinaryDependencies::DependencyLength length)
{
    if ((m_engine.getCompilerType() == CompilerFactory::COMPILER_IA32))
        return R_386_PC32;
    CHECK((m_engine.getCompilerType() == CompilerFactory::COMPILER_ARM) ||
          (m_engine.getCompilerType() == CompilerFactory::COMPILER_THUMB));
    switch (length)
    {
    case BinaryDependencies::DEP_11BIT: return R_ARM_PC13;
    case BinaryDependencies::DEP_24BIT: return R_ARM_PC24;
    case BinaryDependencies::DEP_22BIT_2BYTES_LITTLE_ENDIAN: return R_ARM_THM_PC22;
    }
    // TODO! Pavel!
    CHECK_FAIL();
}

addressNumericValue ELFLinker::bind(SecondPassBinary& pass)
{
    // Returns direct address
    return getNumeric(pass.getData().getBuffer());
}

const cString& ELFLinker::getSectionName(ELFLinker::SectionType sectionType)
{
    switch (sectionType)
    {
        case SECTION_BSS:
            return m_sectionBssString;
        case SECTION_DATA:
            return m_sectionDataString;
        case SECTION_RDATA:
            return m_sectionRDataString;
        case SECTION_TEXT:
            return m_sectionTextString;
        case SECTION_VTBL:
            return m_sectionVtblString;
    }
    CHECK_FAIL();
}

void ELFLinker::registerSections()
{

    // Initate .bss
    m_secRdata = cBuffer(getTotalAllocatedStaticBuffer());
    memset(m_secRdata.getBuffer(), 0, m_secRdata.getSize());

    // Adds the sections.
    m_elfObj.addSection(SHT_PROGBITS, m_sectionTextString, m_secText, SHF_ALLOC | SHF_EXECINSTR);
    m_elfObj.addSymbol(m_sectionTextString,
        cElfSymbol::SYMTYPE_SECTION,
        m_sectionTextString,
        0,
        0,
        false);

    if (m_secRdata.getSize() != 0)
    {
        m_elfObj.addSection(SHT_NOBITS, m_sectionBssString, m_secRdata, SHF_ALLOC | SHF_WRITE);
        m_elfObj.addSymbol(m_sectionBssString,
            cElfSymbol::SYMTYPE_SECTION,
            m_sectionBssString,
            0,
            0,
            false);
    }

    if (m_apartment->getObjects().getStringRepository().getAsciiStringRepository().getSize() != 0)
    {
        m_elfObj.addSection(SHT_PROGBITS, m_sectionDataString, m_apartment->getObjects().getStringRepository().getAsciiStringRepository(), SHF_ALLOC | SHF_WRITE);
        m_elfObj.addSymbol(m_sectionDataString,
            cElfSymbol::SYMTYPE_SECTION,
            m_sectionDataString,
            0,
            0,
            false);
    }

    if (m_apartment->getObjects().getTypedefRepository().getDataSection().getSize() != 0)
    {
        m_elfObj.addSection(SHT_PROGBITS, m_sectionRDataString, m_apartment->getObjects().getTypedefRepository().getDataSection(), SHF_ALLOC | SHF_WRITE);
        m_elfObj.addSymbol(m_sectionRDataString,
            cElfSymbol::SYMTYPE_SECTION,
            m_sectionRDataString,
            0,
            0,
            false);
    }

    if (m_vtblFilledSize > 0)
    {
        m_vtblBuffer.changeSize(m_vtblFilledSize);
        m_elfObj.addSection(SHT_PROGBITS, m_sectionVtblString, m_vtblBuffer, SHF_ALLOC | SHF_WRITE);
        m_elfObj.addSymbol(m_sectionVtblString,
            cElfSymbol::SYMTYPE_SECTION,
            m_sectionVtblString,
            0,
            0,
            false);
    }
}

void ELFLinker::registerLocalFunctions(RelocationHash& relocHash)
{
    cList<TokenIndex> relocFuncs(relocHash.keys());
    cList<TokenIndex>::iterator i(relocFuncs.begin());

    /* We need to add Thumb Symbol before thumb functions.
       if we will support both Arm & Thumb functions in the future,
       we will need to add $a before decalering Arm Functions.
       and then split registers or something .. */
    if ( m_isThumb ) {
        m_elfObj.addSymbol(cString("$t"),
            cElfSymbol::SYMTYPE_FUNC,
            m_sectionTextString,
            0,
            2,
            false);
    }

    for (; i != relocFuncs.end(); ++i)
    {
        TokenIndex method = *i;
        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(method));
        if ((binaryPtr->getDebugInformation().getExportName().length() == 0) &&
            (binaryPtr->getDebugInformation().getMethodName().length() > 0))
        {
            cString rname = cString("f") + binaryPtr->getDebugInformation().getMethodName() + HEXWORD(relocHash[method]);

            m_elfObj.addSymbol(rname,
                cElfSymbol::SYMTYPE_FUNC,
                m_sectionTextString,
                relocHash[method],
                binaryPtr->getData().getSize(),
                false);
        }
    }

}

void ELFLinker::registerGlobals()
{
    GlobalObjectList::iterator curGlob = m_globals.begin();
    for (; curGlob != m_globals.end() ; ++curGlob)
    {
        m_elfObj.useSymbol(getSectionName((SectionType)(*curGlob).m_sectionTarget),
            getSectionName((SectionType)(*curGlob).m_sectionSource),
            m_globalRelocationType,
            (*curGlob).m_dependancyPosition);
    }
}

void ELFLinker::registerExportedFunctions(RelocationHash& relocHash)
{
    cList<TokenIndex>::iterator i(m_exports.begin());

    for (; i != m_exports.end(); i++)
    {
        TokenIndex entry(*i);
        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(entry));
        cString rname = binaryPtr->getDebugInformation().getExportName();

        m_elfObj.addSymbol(rname,
            cElfSymbol::SYMTYPE_FUNC,
            m_sectionTextString,
            relocHash[entry],
            binaryPtr->getData().getSize(),
            true);
    }
}

void ELFLinker::registerImports()
{
    cList<ImportObject>::iterator curImport(m_imports.begin());
    for (; curImport != m_imports.end(); ++curImport)
    {
        // Build name
        cString importName((*curImport).m_entry->m_importName);

        if (false == m_elfObj.hasSymbol(importName))
        {
            m_elfObj.addSymbol(importName,
                cElfSymbol::SYMTYPE_EXTERN,
                "",
                0,//m_importPositions[getNumeric((*curImport).m_entry)],
                0,//m_pointerSize,
                true);
        }

        m_elfObj.useSymbol(importName,
            getSectionName((SectionType)(*curImport).m_section),
            getImportRelocationType((BinaryDependencies::DependencyLength)(*curImport).m_dependancyLength),
            (*curImport).m_dependancyPosition);

    }
}

void ELFLinker::buildFile(RelocationHash& relocHash)
{
    // First Register sections.
    this->registerSections();

    //
    // Local Symbols..
    // Add debug locals & globals.
    this->registerLocalFunctions(relocHash);
    this->registerGlobals();

    //
    // Visiable Symbols MUST Be Last
    // Adding Imports & Exports
    this->registerImports();
    this->registerExportedFunctions(relocHash);

    //
    // Building Debug info
    // TODO: add If build Debug before the call??
    this->buildDebugInfo();
}

void ELFLinker::buildDebugInfo()
{
    // TODO: Get File information
    this->m_elfObj.debugInit(m_sectionTextString, "test.cs", "c:\\test");
}

void ELFLinker::resolveAndExecuteAllDependencies(TokenIndex& mainMethod)
{
    // Prepare the global table
    cFileStream elfOutputFileStream(cString("output.o"), cFile::WRITE | cFile::CREATE);

    // Prepare vtable
    m_vtblBuffer.changeSize(m_apartment->getObjects().getTypedefRepository().
                                estimateVirtualTableMaximumSize(4)); // Pointer size is 4 bytes long. TODO! 32bit / 16bit

    // resolve all of the data/functions.
    m_codeSize = 0;
    m_importLength = 0;
    m_importPositions.removeAll();
    m_imports.removeAll();
    m_exports.removeAll();

    MethodResolvedObject solved;
    MethodStackObject stack;
    RelocationHash relocHash;
    stack.push(mainMethod);
    resolveAll(relocHash, stack, solved);

    // Fill our members to elf object
    m_vtblBuffer.changeSize(m_vtblFilledSize);
    // And copy .text (relocated)
    m_secText.changeSize(m_codeSize);
    cList<TokenIndex> relocFuncs(relocHash.keys());
    cList<TokenIndex>::iterator i(relocFuncs.begin());
    for (; i != relocFuncs.end(); ++i)
    {
        TokenIndex method = *i;
        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(method));
        memcpy(m_secText.getBuffer() + relocHash[method],
               binaryPtr->getData().getBuffer(),
               binaryPtr->getData().getSize());
    }

    // And output file
    buildFile(relocHash);

    // save the elf object to file.
    m_elfObj.saveAs(elfOutputFileStream);
}

uint ELFLinker::getVTableStartAddress()
{
    // VTable will have it's own section
    return 0;
}

void ELFLinker::appendMethod(const TokenIndex& tokenIndex,
                             RelocationHash& relocHash)
{
    if (!relocHash.hasKey(tokenIndex))
    {
        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(tokenIndex));
        uint32 currentMethodAddress = m_codeSize;
        m_codeSize+= binaryPtr->getData().getSize();
        relocHash.append(tokenIndex, currentMethodAddress);
    }
}

void ELFLinker::appendImport(ExternalModuleFunctionEntry* entry)
{
    addressNumericValue addr = getNumeric(entry);
    if (!m_importPositions.hasKey(addr))
    {
        m_importPositions.append(addr, m_importLength);
        m_importLength+= m_pointerSize;
    }
}

void ELFLinker::resolveAll(RelocationHash& relocHash,
                           MethodStackObject& stack,
                           MethodResolvedObject& resolved)
{
    uint32 methodStartPtr = 0;
    while (!stack.isEmpty())
    {
        // Get the binary
        TokenIndex nextMethod(stack.getArg(0));
        stack.pop2null();
        // Check for already resolved method
        if (resolved.hasKey(nextMethod))
            continue;

        // Add new function
        appendMethod(nextMethod, relocHash);
        resolved.append(nextMethod,1);

        // Add the method to m_secText
        SecondPassBinaryPtr binaryPtr(m_engine.getBinaryRepository().getSecondPassMethod(nextMethod));
        appendMethod(nextMethod, relocHash);
        uint32 currentMethodAddress = relocHash[nextMethod];

        // Check for exports
        if (binaryPtr->getDebugInformation().getExportName() != SecondPassInfoAndDebug::gNoExport)
        {
            m_exports.append(nextMethod);
        }

        // Scan the method and resolve it
        const BinaryDependencies::DependencyObjectList& dependencies = binaryPtr->getDependencies().getList();
        BinaryDependencies::DependencyObjectList::iterator j = dependencies.begin();

        for (; j != dependencies.end(); ++j)
        {
            BinaryDependencies::DependencyObject o = *j;
            TokenIndex methodToken;
            uint globalIndex;
            if (CallingConvention::deserializeMethod((*j).m_name, methodToken))
            {
                addressNumericValue addr = 0;

                // Check for external method
                ExternalModuleFunctionEntry* entry;
                if (m_externalModuleResolver.resolveExternalMethod(methodToken, addr, &entry))
                {
                    // Add external module
                    appendImport(entry);
                    ImportObject import;
                    import.m_entry = entry;
                    import.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                    import.m_dependancyLength = (*j).m_length;
                    import.m_section = SECTION_TEXT;
                    m_imports.append(import);
                    // Make the function point to itself.
                    binaryPtr->resolveDependency(*j, currentMethodAddress, import.m_dependancyPosition);
                } else
                {
                    if (!m_engine.getBinaryRepository().isMethodExist(methodToken))
                    {
                        // Throw a nice exception
                        ExecuterTrace("ELFLinker: METHOD NOT FOUND!" << HEXTOKEN(methodToken) << endl);
                        CHECK_FAIL();
                    }

                    // Resolve the address
                    appendMethod(methodToken, relocHash);
                    stack.push(methodToken);
                    addr = relocHash[methodToken];
                    // And add dependancy (for absoulte calls)
                    if ((*j).m_type == BinaryDependencies::DEP_ABSOLUTE)
                    {
                        GlobalObject newfuncref;
                        newfuncref.m_dependancyLength = (*j).m_length;
                        newfuncref.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                        newfuncref.m_sectionTarget = SECTION_TEXT;
                        newfuncref.m_sectionSource = SECTION_TEXT;
                        newfuncref.m_globalIndex = addr;
                        m_globals.append(newfuncref);
                        binaryPtr->resolveDependency(*j, currentMethodAddress, addr, 0, true);
                    } else
                    {
                        binaryPtr->resolveDependency(*j, currentMethodAddress, addr);
                    }
                }
            /*
            } else if (CallingConvention::deserializeGlobal((*j).m_name, globalIndex))
            {
                GlobalObject newGlobal;
                newGlobal.m_dependancyLength = (*j).m_length;
                newGlobal.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                newGlobal.m_sectionTarget = SECTION_BSS;
                newGlobal.m_sectionSource = SECTION_TEXT;
                newGlobal.m_globalIndex = globalIndex;
                m_globals.append(newGlobal);
                binaryPtr->resolveDependency(*j, currentMethodAddress, globalIndex, 0, true);
            */
            } else if (CallingConvention::deserializeGlobalData((*j).m_name, globalIndex))
            {
                GlobalObject newGlobal;
                newGlobal.m_dependancyLength = (*j).m_length;
                newGlobal.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                newGlobal.m_sectionTarget = SECTION_RDATA;
                newGlobal.m_sectionSource = SECTION_TEXT;
                newGlobal.m_globalIndex = globalIndex;
                m_globals.append(newGlobal);
                binaryPtr->resolveDependency(*j, currentMethodAddress, globalIndex, 0, true);
#ifdef CLR_UNICODE
            } else if (m_apartment->getObjects().getStringRepository().deserializeStringU((*j).m_name, globalIndex))
            {
                // TODO! Add unicode support
                CHECK_FAIL();
#else
            } else if (m_apartment->getObjects().getStringRepository().deserializeStringA((*j).m_name, globalIndex))
            {
                GlobalObject newGlobal;
                newGlobal.m_dependancyLength = (*j).m_length;
                newGlobal.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                newGlobal.m_sectionTarget = SECTION_DATA;
                newGlobal.m_sectionSource = SECTION_TEXT;
                newGlobal.m_globalIndex = globalIndex;
                m_globals.append(newGlobal);
                binaryPtr->resolveDependency(*j, currentMethodAddress, globalIndex, 0, true);
#endif // CLR_UNICODE
            } else if (CallingConvention::deserializeToken((*j).m_name, methodToken))
            {
                if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == TABLE_FIELD_TABLE)
                {
                    // Static field token
                    addressNumericValue addr = getStaticAddress(methodToken);
                    GlobalObject newGlobal;
                    newGlobal.m_dependancyLength = (*j).m_length;
                    newGlobal.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                    newGlobal.m_sectionTarget = SECTION_BSS;
                    newGlobal.m_sectionSource = SECTION_TEXT;
                    newGlobal.m_globalIndex = addr;
                    m_globals.append(newGlobal);
                    binaryPtr->resolveDependency(*j, currentMethodAddress, addr, 0, true);
                } else if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == TABLE_TYPEDEF_TABLE)
                {
                    // Adding virtual table pointer
                    addressNumericValue addr = getVtblAddress(methodToken);
                    if (addr == 0)
                    {
                        // Build new vtbl
                        addr = m_vtblFilledSize + getVTableStartAddress();

                        // Adding parents table
                        ResolverInterface& resolver = m_apartment->getObjects().getTypedefRepository();
                        const ResolverInterface::ParentDictonary& parents = resolver.getParentDirectory(methodToken);
                        cList<TokenIndex> parentsId;
                        parents.keys(parentsId);
                        uint16 ancestorLength = 0;
                        uint16* l = (uint16*)(m_vtblBuffer.getBuffer() + m_vtblFilledSize);
                        uint8* startTable = (uint8*)l;

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
                            alignTo16();
                            // write offset
                            cEndian::writeUint16((uint8*)l, parents[*j], isLittleEndian); l++;
                            alignTo16();
                            m_vtblFilledSize+= 4;
                        }
                        // Adding self rtti
                        ancestorLength++;
                        cEndian::writeUint16((uint8*)l, resolver.getRTTI(methodToken), isLittleEndian); l++;
                        alignTo16();
                        // write offset
                        cEndian::writeUint16((uint8*)l, 0, isLittleEndian); l++;
                        alignTo16();
                        m_vtblFilledSize+= 4;

                        // Adding number of parents including self
                        cEndian::writeUint16((uint8*)l, ancestorLength, isLittleEndian); l++;
                        alignTo16();
                        // Adding object-size
                        CHECK(resolver.getTypeSize(methodToken) < 0x10000);
                        cEndian::writeUint16((uint8*)l, (uint16)resolver.getTypeSize(methodToken), isLittleEndian); l++;
                        alignTo16();
                        m_vtblFilledSize+= 4;

                        // Increase address for actual v-table
                        addr+= ((uint8*)l) - startTable;
                        setVtblAddress(methodToken, addr);
                        const ResolverInterface::VirtualTable& vTbl =
                                m_apartment->getObjects().getTypedefRepository().getVirtualTable(methodToken);
                        ResolverInterface::VirtualTable::iterator i = vTbl.begin();
                        // TODO! Change this for ELF 64
                        // addressNumericValue* ftbl = (addressNumericValue*)(l);
                        uint32* ftbl = (uint32*)(l);

                        for (; i != vTbl.end(); i++)
                        {
                            TokenIndex func = getVtblMethodIndexOverride(*i);
                            appendMethod(func, relocHash);
                            addressNumericValue binaryAddress = relocHash[func];
                            // And write it flat to the vtbl
                            *ftbl = (uint32)binaryAddress;
                            stack.push(func);
                            // And add link to .text
                            GlobalObject newGlobal;
                            newGlobal.m_dependancyLength = m_pointerSize;
                            newGlobal.m_dependancyPosition = ((uint8*)ftbl) - m_vtblBuffer.getBuffer();
                            newGlobal.m_sectionTarget = SECTION_TEXT;
                            newGlobal.m_sectionSource = SECTION_VTBL;
                            newGlobal.m_globalIndex = binaryAddress;
                            m_globals.append(newGlobal);
                            // Incrase pointers
                            ftbl++;
                            m_vtblFilledSize += m_pointerSize;
                        }
                    }

                    // And add dependancy, inside .text
                    GlobalObject newGlobal;
                    newGlobal.m_dependancyLength = (*j).m_length;
                    newGlobal.m_dependancyPosition = (*j).m_position + currentMethodAddress;
                    newGlobal.m_sectionTarget = SECTION_VTBL;
                    newGlobal.m_sectionSource = SECTION_TEXT;
                    newGlobal.m_globalIndex = addr;
                    m_globals.append(newGlobal);
                    binaryPtr->resolveDependency(*j, currentMethodAddress, addr, 0, true);
                } else if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == 0x70)
                {
                    // String token, should be get here
                    CHECK_FAIL();
                } else if (EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) == 0xFF) // TODO! Add enum/define
                {
                    // TODO!
                    CHECK_FAIL();
                } else
                {
                    ExecuterTrace("DefaultCompilerAlgorithm::resolve(): ERROR! "
                              "Unknown token ID: " << HEXTOKEN(methodToken) << endl);
                    CHECK_FAIL();
                }
            } else
            {
                // TODO! Add symbol (use custom attribute) to ELF import section
                //CHECK_FAIL();
            }
        }
    }
}

uint32 ELFLinker::getCodeSize()
{
    uint32 res = 0;
    MethodTransTable transTable = m_engine.getBinaryRepository().getMethodTransTable();
    const cList<TokenIndex>& transKeys(transTable.keys());
    cList<TokenIndex>::iterator i = transKeys.begin();
    for (;  i != transKeys.end(); ++i)
    {
        SecondPassBinaryPtr methodPtr(transTable[*i]);
        res += methodPtr->getData().getSize();
    }
    return res;
}

