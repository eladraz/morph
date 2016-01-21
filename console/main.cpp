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

#include "stdafx.h"
#include "data/exceptions.h"
#include "runnable/GlobalContext.h"
#include "runnable/ApartmentFactory.h"
#include "runnable/StringReader.h"
#include "compiler/CompilerFactory.h"
#include "executer/MethodIndex.h"
#include "executer/linker/LinkerFactory.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/AssemblyRefTable.h"
#include "console/minidump.h"
#include "console/ConsoleAlgorithm.h"

// TODO! Add class
static ApartmentPtr loadPE(const cString& exePath, const char * dllPath, MemoryLayoutInterfacePtr& memoryLayout);
static ApartmentPtr createApartment(const cString& pePath, MemoryLayoutInterfacePtr& memoryLayout, ApartmentPtr& parentApartment);
static void loadAssemblyRef(ApartmentPtr& apt, ApartmentPtr& mainApartment, MemoryLayoutInterfacePtr& memoryLayout);

struct WorkType
{
    char* param;
    CompilerFactory::CompilerType compilerType;
    LinkerFactory::LinkerType linkerType;
    char* description;
};

static const WorkType workTypes[] =
{
    {"x86",            CompilerFactory::COMPILER_IA32,             LinkerFactory::ELF_LINKER, "Create an ELF output.o file compiled for x86"},
    {"x86-mem",           CompilerFactory::COMPILER_IA32,             LinkerFactory::MEMORY_LINKER, "Create memory-linked x86 code and execute it"},
    {"32c",               CompilerFactory::COMPILER_32C,             LinkerFactory::FILE_LINKER, "Create a C/C++ language output.c file"},
    {"arm",               CompilerFactory::COMPILER_ARM,             LinkerFactory::ELF_LINKER, "Create an ARM-compiled output file"},
    {"thumb",           CompilerFactory::COMPILER_THUMB,             LinkerFactory::ELF_LINKER, "Create an THUMB-compiled output file"},
};

const uint workTypeCount = sizeof(workTypes) / sizeof(workTypes[0]);

struct CompilerParam
{
    char* param;
    char* description;
};

static const CompilerParam compilerParams[] =
{
    {"eh+", "Enable exception handling (default)"},
    {"eh-", "Disable exception handling (might be required for some output types)"},
    {"opt+", "Enable compiler optimizations"},
    {"opt-", "Disable compiler optimizations (default)"},
    {"dev", "Enable developer-level verbosity of output traces (Debug builds only)"},
};

const uint compilerParamCount = sizeof(compilerParams) / sizeof(compilerParams[0]);

// Program parameters
cString precompiledMethodsPath = "";
cSetArray works;
CompilerParameters params = CompilerInterface::defaultParameters;

static void runEngine(ApartmentPtr mainApartment,
                      const CompilerFactory::CompilerType compilerType,
                      const LinkerFactory::LinkerType linkerType,
                      const cString& repositoryFilename)
{
    CompilerEngineThread thread(compilerType, params, mainApartment, repositoryFilename);
    LinkerInterfacePtr linker = LinkerFactory::getLinker(linkerType, thread, mainApartment);
    ConsoleAlgorithm consoleAlgo(mainApartment->getEntryPointToken(), mainApartment, thread, linker);
    thread.addNotifier(consoleAlgo);
    thread.run(consoleAlgo);
}

static ApartmentPtr createApartment(const cString& pePath,
                                    MemoryLayoutInterfacePtr& memoryLayout,
                                    ApartmentPtr& parentApartment)
{
    ApartmentPtr apartment;

    XSTL_TRY
    {
        cFileStream peStream(pePath);
        cNtHeaderPtr ntFile = ApartmentFactory::loadEXEFile(peStream);
        cNtDirCli cliDirectory(*ntFile);
        apartment = ApartmentFactory::createApartment(ntFile, cliDirectory, *memoryLayout, parentApartment);
    }
    XSTL_CATCH_ALL
    {
        ConsoleTrace("Unable to create apartment." << endl);
        XSTL_RETHROW;
    }

    return apartment;
}

static void loadAssemblyRef(ApartmentPtr& apt, ApartmentPtr& mainApartment, MemoryLayoutInterfacePtr& memoryLayout)
{
    UNUSED_PARAM(memoryLayout);
    UNUSED_PARAM(mainApartment);
    RowTablesPtr assemblers = apt->getTables().byTableID(TABLE_ASSEMBLYREF_TABLE);
    for (uint i = 0; i < assemblers.getSize(); i++)
    {
        const AssemblyRefTable::Header& assembly = ((AssemblyRefTable&)(*assemblers[i])).getHeader();
        cString name(StringReader::readStringName(*apt->getStreams().getStringsStream()->fork(), assembly.m_name));
        // Find the apartment and load it
        // TODO! Look for apartment...
        /*
        name = cString("c:\\temp\\") + name + ".dll";
        ApartmentPtr newapt = createApartment(name, memoryLayout, mainApartment);
        loadAssemblyRef(newapt, mainApartment, memoryLayout);
        */
    }
}

static ApartmentPtr loadPE(const cString& exePath, const char * dllPath, MemoryLayoutInterfacePtr& memoryLayout)
{
    ApartmentPtr tempApt(NULL, SMARTPTR_DESTRUCT_NONE);
    ApartmentPtr mainApartment = createApartment(exePath, memoryLayout, tempApt);
    if (0 == getTokenID(mainApartment->getEntryPointToken()))
    {
        XSTL_THROW(ClrRuntimeException, XSTL_STRING("Invalid exe, missing entry point token."));
    }

    ApartmentPtr clrApartment = createApartment(dllPath, memoryLayout, mainApartment);

    if (false == mainApartment->getObjects().getFrameworkMethods().isAllResolved())
    {
        XSTL_THROW(ClrRuntimeException, XSTL_STRING("clrcore.dll is not up-to-date. Some of the basic functions are missing!!!"));
    }

    // Add all dependancy apartments
    loadAssemblyRef(mainApartment, mainApartment, memoryLayout);

    return mainApartment;
}

void setCompilerParameter(uint index)
{
    // Todo: Find a better code structure when more parameters are introduced
    switch (index)
    {
    case 0:
        params.m_bSupportExceptionHandling = true;
        break;
    case 1:
        params.m_bSupportExceptionHandling = false;
        break;
    case 2:
        params.m_bEnableOptimizations = true;
        break;
    case 3:
        params.m_bEnableOptimizations = false;
        break;
    case 4:
        params.m_bDeveloperVerbosity = true;
        break;
    default:
        CHECK_FAIL();
    }
}

void print_usage(const char * exe)
{
    cout << "Usage: " << exe <<  " [options] <clrcore.dll path> <.NET PE file-name>" << endl;
    cout << "  Where options may be:" << endl;
    cout << "  -p <path>   Specify the path to the precompiled repository file" << endl;
    cout << "  -o <type>   Specify output type. This option may be specified more than once." << endl;
    cout << "              If not specified, the default is x86. Possible outputs are:" << endl;
    for (uint type = 0; type < workTypeCount; type++)
        cout << "                  " << workTypes[type].param << "   " << workTypes[type].description << endl;
    cout << "  -c <param>  Override a compiler parameter. This option may be specified more than once." << endl;
    cout << "              Possible compiler parameters are:" << endl;
    for (uint param = 0; param < compilerParamCount; param++)
        cout << "                  " << compilerParams[param].param << "   " << compilerParams[param].description << endl;
    cout << endl;
    cout << "Example 1: This will compile the compiler-test program into a x86 object file, and then run it from memory:" << endl;
    cout << "  " << exe << " clrcore.dll testSimpleCompiler1.exe" << endl;
    cout << endl;
    cout << "Example 2: This will compile the compiler-test program into an ARM object file using a precompiled repository, but without exception handling support:" << endl;
    cout << "  " << exe << " -p precomp.dat -c eh- -o arm clrcore.dll testSimpleCompiler1.exe" << endl;
    cout << endl;
}

bool processParameter(const char** argv, uint& firstArg)
{
    if (strcmp(argv[firstArg], "-p") == 0)
    {
        if (precompiledMethodsPath.length() > 0)
        {
            cout << "Error: parameter -p specified more than once";
            firstArg = 0;
            return false;
        }
        // Skip the -p, then fetch the path
        firstArg++;
        precompiledMethodsPath = argv[firstArg];
        // Skip the path
        firstArg++;
        return true;
    }
    if (strcmp(argv[firstArg], "-o") == 0)
    {
        // Skip the -o, then fetch the type
        firstArg++;
        cString typeParam = argv[firstArg];
        // Skip the output type
        firstArg++;

        // Look-up this output work type
        uint type;
        for (type = 0; type < workTypeCount; type++)
        {
            if (typeParam.compare(workTypes[type].param) == cString::EqualTo)
                break;
        }

        if (type >= workTypeCount)
        {
            cout << "Error: Unsupported output type specified for -o. Please see command-line usage.";
            firstArg = 0;
            return false;
        }

        // Mark this work type for execution
        works.set(type);
        return true;
    }
    if (strcmp(argv[firstArg], "-c") == 0)
    {
        // Skip the -c, then fetch the parameter
        firstArg++;
        cString param = argv[firstArg];
        // Skip the output type
        firstArg++;

        // Look-up this compiler parameter
        uint index;
        for (index = 0; index < compilerParamCount; index++)
        {
            if (param.compare(compilerParams[index].param) == cString::EqualTo)
                break;
        }

        if (index >= compilerParamCount)
        {
            cout << "Error: Unsupported compiler parameter specified for -c. Please see command-line usage.";
            firstArg = 0;
            return false;
        }

        // Override the compiler parameter
        setCompilerParameter(index);
        return true;
    }
    return false;
}

int main(const int argc, const char** argv)
{
#ifdef CLR_CONSOLE_MINIDUMP
    bool throwUnhandledExceptions = true;
#else
    bool throwUnhandledExceptions = false;
#endif

    ConsoleTrace("Morph Console Compiler version date " << __DATE__ << " time " << __TIME__ << endl);

    works.changeSize(workTypeCount);
    works.resetArray();

    minidumpInit();

    uint firstArg = 1;

    // Need at least two arguments: clrcore and exe
    if (argc < firstArg+2)
    {
        cout << "Error: Missing required parameters" << endl;
        print_usage(argv[0]);
        return RC_ERROR;
    }

    // Process one parameter
    while (processParameter(argv, firstArg))
    {
        // A parameter was processed. Need at least two more arguments: clrcore and exe
        if (argc < firstArg+2)
        {
            cout << "Error: Missing required parameters" << endl;
            print_usage(argv[0]);
            return RC_ERROR;
        }
    }

    // processParameter may set firstArg to 0 to indicate an error
    if (firstArg == 0)
        return RC_ERROR;

    // Default work types if none specified: x86
    if (works.first() == works.getLength())
    {
        works.set(0);
        // works.set(1);  x86-mem
    }

    const char * dllPath = argv[firstArg];
    const char * exePath = argv[firstArg+1];

    XSTL_TRY
    {
        for (uint type = 0; type < workTypeCount; type++)
        {
            // Do we need to perform this work?
            if (!works.isSet(type))
                continue;

            // Perform the work.
            MemoryLayoutInterfacePtr memoryLayout = CompilerFactory::getMemoryLayout(workTypes[type].compilerType);
            ApartmentPtr mainApartment = loadPE(exePath, dllPath, memoryLayout);
            runEngine(mainApartment, workTypes[type].compilerType, workTypes[type].linkerType, precompiledMethodsPath);
        }
    }
    XSTL_CATCH(cException& e)
    {
        ConsoleTrace("Caught unhandled exception at main..." << endl);
        e.print();

        if (throwUnhandledExceptions)
            XSTL_RETHROW;
        else {
            ConsoleTrace("Shutdown gracefully." << endl);
        }

        return RC_ERROR;
    }
    XSTL_CATCH_ALL
    {
        ConsoleTrace("Caught **unknown** unhandled exception at main..." << endl);

        if (throwUnhandledExceptions)
            XSTL_RETHROW;
        else {
            ConsoleTrace("Shutdown gracefully." << endl);
        }

        return RC_ERROR;
    }

    return RC_OK;
}
