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
 * FileLinker.cpp
 *
 * Implementation file
 */
#include "executer/stdafx.h"
#include "compiler/CallingConvention.h"
#include "compiler/CompilerFactory.h"
#include "compiler/processors/c/32C.h"
#include "executer/runtime/Executer.h"
#include "executer/linker/FileLinker.h"
#include "format/EncodingUtils.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/fileStream.h"
#include "xStl/stream/traceStream.h"

FileLinker::FileLinker(CompilerEngineThread& compilerEngineThread, ApartmentPtr apartment) :
    LinkerInterface(compilerEngineThread, apartment)
{
}

void FileLinker::writeString(basicIO& out, const cString& string)
{
    cSArray<char> a = string.getASCIIstring();
    out.pipeWrite((const uint8*)a.getBuffer(), a.getSize() - 1);
}

cString FileLinker::demangleName(const cString& name)
{
    if ((name.length() > 0) &&
        (name[0] == '_'))
    {
        return name.right(name.length() - 1);
    }
    return name;
}

void FileLinker::writeFuntionHeaderAndBody(basicIO& out,
                                           const TokenIndex& function,
                                           const SecondPassBinary& bin,
                                           bool shouldAddBody)
{
    uint8* data = bin.getData().getBuffer();
    // Write function header
    cString header("// ");
    // Get function prototype
    header+= bin.getDebugInformation().getMethodName();
    header+= endl;
    header+= "int ";
    header+= c32CCompilerInterface::getFunctionName(function);
    // Write header
    writeString(out, header);

    // Write function body
    if (shouldAddBody)
    {
        uint size = bin.getData().getSize();
        while (data[size-1] == 0)
            size--;
        out.pipeWrite(data, size);
    } else
    {
        // Find the first location of ')'
        uint size = 0;
        while (data[size] != ')') size++;
        out.pipeWrite(data, size+1);
    }
}

void FileLinker::writeVTable(basicIO& out,
                             const TokenIndex& token)
{
    ResolverInterface& resolver = m_apartment->getObjects().getTypedefRepository();
    const ResolverInterface::ParentDictonary& parents = resolver.getParentDirectory(token);
    cList<TokenIndex> parentsId;
    parents.keys(parentsId);
    uint16 ancestorLength = 0;

    // Write header
    cString h("int impl_vtbl");
    h+= HEXDWORD(token.m_b);
    h+= HEXDWORD(token.m_a);
    h+= "[] = {";
    writeString(out, h);
    uint data;

    for (cList<TokenIndex>::iterator j = parentsId.begin();
            j != parentsId.end();
            ++j, ancestorLength++)
    {
        data = resolver.getRTTI(*j) | (parents[*j] << 16);
        writeString(out, cString("0x") + HEXDWORD(data) + ", ");
    }
    // Adding self rtti
    data = resolver.getRTTI(token);
    writeString(out, cString(data) + ", ");

    // Adding number of parents including self
    data = (ancestorLength + 1) + (resolver.getTypeSize(token) << 16);
    writeString(out, cString("0x") + HEXDWORD(data) + ", ");

    const ResolverInterface::VirtualTable& vTbl =
            m_apartment->getObjects().getTypedefRepository().getVirtualTable(token);
    ResolverInterface::VirtualTable::iterator i = vTbl.begin();

    for (; i != vTbl.end(); i++)
    {
        TokenIndex func = getVtblMethodIndexOverride(*i);
        cString fname("&");
        fname+= c32CCompilerInterface::getFunctionName(func);
        if (i + 1 != vTbl.end())
            fname+= ", ";
        writeString(out, fname);
    }
    writeString(out, cString("};") + endl);
    h = "int* vtbl";
    h+= HEXDWORD(token.m_b);
    h+= HEXDWORD(token.m_a);
    h+= " = impl_vtbl";
    h+= HEXDWORD(token.m_b);
    h+= HEXDWORD(token.m_a);
    h+= " + ";
    h+= cString(ancestorLength + 2);
    h+= ";";
    h+= endl;
    writeString(out, h);
}

void FileLinker::resolveAndExecuteAllDependencies(TokenIndex& mainMethod)
{
    cFileStream outFile(cString("c:\\temp\\testclr\\output.c"), cFile::WRITE | cFile::CREATE);
    const MethodTransTable& table = m_engine.getBinaryRepository().getMethodTransTable();
    cList<TokenIndex> functions = table.keys();
    cHash<TokenIndex, int> externFunctions;

    if (1) // TODO! Check for linker options for MS compatability
    {
        writeString(outFile, "#include \"32c_wrapper/imports.h\"");
        writeString(outFile, endl);
        // Disable all warnings
        writeString(outFile, "#ifdef _MSC_VER");
        writeString(outFile, endl);
        writeString(outFile, "    #pragma warning(disable:4102)");writeString(outFile, endl);
        writeString(outFile, "    #pragma warning(disable:4022)");writeString(outFile, endl);
        writeString(outFile, "    #pragma warning(disable:4047)");writeString(outFile, endl);
        writeString(outFile, "    #pragma warning(disable:4101)");writeString(outFile, endl);
        writeString(outFile, "    #pragma warning(disable:4024)");writeString(outFile, endl);
        writeString(outFile, "#endif");writeString(outFile, endl);

        writeString(outFile, "#ifdef __GNUC__");writeString(outFile, endl);
        writeString(outFile, "    #pragma GCC diagnostic warning \"-w\"");writeString(outFile, endl);
        writeString(outFile, "#endif");writeString(outFile, endl);
    }

    // Add all exports & imports, declerations
    cList<TokenIndex>::iterator i = functions.begin();
    cHash<TokenIndex, uint> stringTable, vtblHash, staticTable;
    cList<TokenIndex> strings;
    for (; i != functions.end(); ++i)
    {
        addressNumericValue addr;
        ExternalModuleFunctionEntry* entry;
        if (m_externalModuleResolver.resolveExternalMethod(*i, addr, &entry))
        {
            externFunctions.append(*i, 1);
            cString d("#define ");
            d+= c32CCompilerInterface::getFunctionName(*i);
            d+= " ";
            d+= demangleName(entry->m_importName);
            d+= endl;
            writeString(outFile, d);
        } else
        {
            cString exportName(table[*i]->getDebugInformation().getExportName());
            if (exportName.length() > 0)
            {
                cString d("#define ");
                d+= c32CCompilerInterface::getFunctionName(*i);
                d+= " ";
                d+= demangleName(exportName);
                d+= endl;
                writeString(outFile, d);
            }

            // Add forward deceleration
            writeFuntionHeaderAndBody(outFile, *i, *table[*i], false);
            writeString(outFile, cString(";") + endl);
        }

        // Scan dependency and compile vtbl and string table
        const BinaryDependencies::DependencyObjectList& dependencies = table[*i]->getDependencies().getList();
        BinaryDependencies::DependencyObjectList::iterator j = dependencies.begin();
        for (; j != dependencies.end(); ++j)
        {
            BinaryDependencies::DependencyObject o = *j;
            uint globalIndex;
            TokenIndex t;
            if (m_apartment->getObjects().getStringRepository().deserializeStringA((*j).m_name, globalIndex, &t))
            {
                // Just added to the string-repository
                if (!stringTable.hasKey(t))
                    stringTable.append(t, globalIndex);
            } else if (CallingConvention::deserializeToken((*j).m_name, t))
            {
                if (EncodingUtils::getTokenTableIndex(getTokenID(t)) == TABLE_TYPEDEF_TABLE)
                {
                    // Adding virtual table pointer
                    if (!vtblHash.hasKey(t))
                    {
                        // Build new vtbl
                        vtblHash.append(t, 1);
                    }
                }
                else if (EncodingUtils::getTokenTableIndex(getTokenID(t)) == TABLE_FIELD_TABLE)
                {
                    // Adding static field pointer
                    if (!staticTable.hasKey(t))
                    {
                        // Build new static field
                        staticTable.append(t, getStaticAddress(t));
                    }
                }
                else
                {
                    CHECK_FAIL();
                }
            }
        }
    }

    // Adding string table
    uint size = m_apartment->getObjects().getStringRepository().getAsciiStringRepository().getSize();
    if (size > 0)
    {
        writeString(outFile, cString("static const unsigned char gStringTable[] = {"));
        writeString(outFile, endl);
        const uint8* sdata = m_apartment->getObjects().getStringRepository().getAsciiStringRepository().getBuffer();
        for (uint j = 0; j < size; j++)
        {
            writeString(outFile, "0x");
            writeString(outFile, HEXBYTE(sdata[j]));
            if (j != size - 1)
            {
                writeString(outFile, ", ");
                if ((j & 0x0F) == 0x0F)
                    writeString(outFile, endl);
            }
        }
        writeString(outFile, cString("};") + endl);
    }
    strings = stringTable.keys();
    i = strings.begin();
    for (; i != strings.end(); ++i)
    {
        writeString(outFile, "#define str");
        writeString(outFile, HEXDWORD((*i).m_b));
        writeString(outFile, HEXDWORD((*i).m_a));
        writeString(outFile, " (gStringTable + ");
        writeString(outFile, cString(stringTable[*i]));
        writeString(outFile, ")");
        writeString(outFile, endl);
    }

    strings = vtblHash.keys();
    i = strings.begin();
    for (; i != strings.end(); ++i)
    {
        writeVTable(outFile, *i);
    }

    // Add globals
    writeString(outFile, "static unsigned char ");
    writeString(outFile, c32CCompilerInterface::getGlobalsName());
    writeString(outFile, "[0x");
    writeString(outFile, HEXDWORD(getTotalAllocatedStaticBuffer()));
    writeString(outFile, "];");
    writeString(outFile, endl);

    strings = staticTable.keys();
    i = strings.begin();
    for (; i != strings.end(); ++i)
    {
        writeString(outFile, "#define glbl");
        writeString(outFile, HEXDWORD((*i).m_b));
        writeString(outFile, HEXDWORD((*i).m_a));
        writeString(outFile, " (");
        writeString(outFile, c32CCompilerInterface::getGlobalsName() + " + ");
        writeString(outFile, cString(staticTable[*i]));
        writeString(outFile, ")");
        writeString(outFile, endl);
    }

    // Add all functions
    i = functions.begin();
    for (; i != functions.end(); ++i)
    {
        if (externFunctions.hasKey(*i))
            continue;

        writeFuntionHeaderAndBody(outFile, *i, *table[*i], true);
    }
}

addressNumericValue FileLinker::bind(SecondPassBinary& pass)
{
    return 0;
}
