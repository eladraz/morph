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

#ifndef __TBA_CLR_EXECUTER_RUNTIME_ELFLINKER_H
#define __TBA_CLR_EXECUTER_RUNTIME_ELFLINKER_H

/*
 * ELFLinker.h
 *
 * ELF format linker
 *
 * Author: Pavel Ferencz
 */
#include "executer/compiler/CompilerEngineThread.h"
#include "executer/linker/LinkerInterface.h"
#include "dismount/assembler/BinaryDependencies.h"
#include "elf/cElfFile.h"

class ELFLinker : public LinkerInterface
{
public:
    // Constructor
    ELFLinker(CompilerEngineThread& compilerEngineThread, ApartmentPtr apartment);

    // See LinkerInterface::resolveAndExecuteAllDependencies
    virtual void resolveAndExecuteAllDependencies(TokenIndex& mainMethod);

protected:
    /*
     * Bind into a method and fills all the dependencies needed in order to
     * execute the method.
     *
     * Return a numeric value which can be transfer into the 'ExecuteInterface'
     */
    virtual addressNumericValue bind(SecondPassBinary& pass);

private:
    // Disable copy construction
    ELFLinker(const ELFLinker& other);
    ELFLinker& operator=(const ELFLinker& other);

    enum SectionType
    {
        // Uninitialized data section
        SECTION_BSS = 0,
        // Initialized data section
        SECTION_DATA = 1,
        // Section for .text
        SECTION_TEXT = 2,
        // Section for .vtbl
        SECTION_VTBL = 3,
        // Initialized data section
        SECTION_RDATA = 4,
    };

    class GlobalObject
    {
    public:
        // Type of the
        // Which section the dependancy at [target] (See SECTION_*)
        uint8 m_sectionTarget;
        // Which section refer to the dependancy [source] (See SECTION_*)
        uint8 m_sectionSource;
        // Length of the global dependancy
        uint8 m_dependancyLength;
        // The position of the dependancy (should be the same as globalIndex)
        uint m_dependancyPosition;
        // The position of the global
        uint m_globalIndex;

        // Build export string
        cString buildExportString(cHash<uint, cString> functionNames);
    };

    friend class GlobalObject;
    // Typedef for List
    typedef cList<GlobalObject> GlobalObjectList;

    class ImportObject
    {
    public:
        // Name of import symbol
        ExternalModuleFunctionEntry* m_entry;
        // Position, length and section
        uint m_dependancyPosition;
        uint8 m_section;
        uint8 m_dependancyLength;
    };

    /*
     * At end of the resolver, build the actual ELF file
     */
    void buildFile(RelocationHash& relocHash);

    /*
     * Recursive method that resolves dependencies.
     */
    void resolveAll(RelocationHash& relocHash,
                    MethodStackObject& stack,
                    MethodResolvedObject& resolved);

    /*
     * Return the type of relocation according to the dependency length
     */
    uint getImportRelocationType(BinaryDependencies::DependencyLength length);

    /*
     * Get the compiled code section size
     */
    uint32 getCodeSize();

    /*
     * Make sure a function is inside relocation table and add to section .text
     */
    void appendMethod(const TokenIndex& tokenIndex,
                      RelocationHash& relocHash);

    /*
     * Add a new import table value
     */
    void appendImport(ExternalModuleFunctionEntry* entry);

    //
    // This method will register Imports into elf lib.
    //
    void registerImports();

    //
    // This method will register Exports into elf lib.
    //
    void registerExportedFunctions(RelocationHash& relocHash);

    //
    // This method will register Globals into elf lib.
    //
    void registerGlobals();

    //
    // This method will register Locals into elf lib.
    //
    void registerLocalFunctions(RelocationHash& relocHash);

    //
    // This method will register Sections into elf lib.
    //
    void registerSections();

    //
    // this method will build debug info about the file
    //
    void buildDebugInfo();

    // Output linked ELF file
    cElfFile m_elfObj;

    // Type of relocation for globals and imports (architecture specific)
    uint m_globalRelocationType;

    // The code.
    cBuffer m_secText;
    uint m_codeSize;

    // Static data
    cBuffer m_secRdata;

    // Return the sarting point of the vTable inside the .data section
    uint getVTableStartAddress();
    // The vtable size
    cBuffer m_vtblBuffer;

    // Return section name by type
    const cString& getSectionName(SectionType sectionType);
    static const cString m_sectionBssString;
    static const cString m_sectionDataString;
    static const cString m_sectionRDataString;
    static const cString m_sectionVtblString;
    static const cString m_sectionTextString;
    static const cString m_sectionImportString;

    // All global dependancies
    GlobalObjectList m_globals;
    // All imports
    cList<ImportObject> m_imports;
    cList<TokenIndex> m_exports;
    cHash<addressNumericValue, uint> m_importPositions;  // Pointer to ExternalModuleFunctionEntry*
    uint m_importLength;

    // Length of pointer
    uint m_pointerSize;
    // Alignment
    uint m_alignment;

    // Is (arm) Thumb mode ?
    bool m_isThumb;
};

#endif // __TBA_CLR_EXECUTER_RUNTIME_ELFLINKER_H
