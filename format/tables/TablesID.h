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

#ifndef __TBA_CLR_FORMAT_TABLES_TABLESID_H
#define __TBA_CLR_FORMAT_TABLES_TABLESID_H

/*
 * TablesID.h
 *
 * A list of all possibles modules and thier ID's
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

enum TablesID {
    // See ModuleTable.h
    TABLE_MODULE_TABLE = 0,
    // See TyperefTable.h
    TABLE_TYPEREF_TABLE = 1,
    // See TypedefTable.h
    TABLE_TYPEDEF_TABLE = 2,
    // See FieldTable
    TABLE_FIELD_TABLE = 4,
    // See MethodTable
    TABLE_METHOD_TABLE = 6,
    // See ParamTable
    TABLE_PARAM_TABLE = 8,
    // See InterfaceImplTable
    TABLE_INTERFACEIMPL_TABLE = 9,
    // See MemberRefTable
    TABLE_MEMBERREF_TABLE = 0x0A,
    // See ConstantTable
    TABLE_CONSTANT_TABLE = 0x0B,
    // See CustomAttributeTable
    TABLE_CUSTOMATTRIBUTE_TABLE  = 0x0C,
                                                // TODO! See FieldMarshal
    // See DeclSecurityTable
    TABLE_DECLSECURITY_TABLE  = 0x0E,
    // See ClassLayoutTable
    TABLE_CLASSLAYOUT_TABLE = 0x0F,
                                                // TODO! Some more tables hers
    // See StandAloneSigTable
    TABLE_STANDALONGESIG_TABLE = 0x11,
                                                // TODO! Some more tables hers
                                                // TODO!
                                                TABLE_EVENT_TABLE = 0x14,
    // See PropertyMapTable
    TABLE_PROPERTYMAP_TABLE = 0x15,
    // See PropertyTable
    TABLE_PROPERTY_TABLE = 0x17,
    // See MethodSemanticsTable
    TABLE_METHODSEMANTICS_TABLE = 0x18,
    // See MethodImplTable
    TABLE_METHODIMPL_TABLE = 0x19,

                                                TABLE_MODULEREF_TABLE = 0x1A,
    // See TypeSpecTable
    TABLE_TYPESPEC_TABLE = 0x1B,
    // See FieldRVATable
    TABLE_FIELDRVA_TABLE = 0x1D,
    // See AssemblyTable
    TABLE_ASSEMBLY_TABLE = 0x20,
                                                // TODO! Some more tables hers
    // See AssemblyRefTable
    TABLE_ASSEMBLYREF_TABLE = 0x23,
                                                // TODO!
                                                TABLE_FILE_TABLE = 0x26,
                                                TABLE_EXPORTTYPE_TABLE = 0x27,
                                                TABLE_MANIFESTRESOURCE_TABLE = 0x28,
    // See NestedClassTable
    TABLE_NESTEDCLASS_TABLE = 0x29,
    // See GenericParamTable
    TABLE_GENERICPARAM_TABLE = 0x2A,
    // See MethodSpecTable
    TABLE_METHODSPEC_TABLE = 0x2B,

    // See TypedefRepository instance destructor
    TABLE_CLR_METHOD_INSTANCE_DETOR = 0xFB,
    // See MethodCompiler
    TABLE_CLR_METHOD_HELPERS = 0xFC,
    // See FieldResolverInterface - A new tokens for generic instances
    TABLE_CLR_GENERICS_INSTANCES = 0xFD,
    // See CallingConvention::cctor
    TABLE_CLR_CCTOR_WRAPPERS = 0xFE,
    // See FrameworksMethods
    TABLE_CLR_INTERNAL = 0xFF
};

#endif // __TBA_CLR_FORMAT_TABLES_TABLESID_H
