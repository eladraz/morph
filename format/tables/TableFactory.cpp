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
 * TableFactory.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/TableFactory.h"
#include "format/tables/ModuleTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/FieldTable.h"
#include "format/tables/FieldRVATable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/ParamTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/tables/ConstantTable.h"
#include "format/tables/InterfaceImplTable.h"
#include "format/tables/CustomAttributeTable.h"
#include "format/tables/ClassLayoutTable.h"
#include "format/tables/DeclSecurityTable.h"
#include "format/tables/StandAloneSigTable.h"
#include "format/tables/PropertyMapTable.h"
#include "format/tables/PropertyTable.h"
#include "format/tables/MethodSemanticsTable.h"
#include "format/tables/MethodImplTable.h"
#include "format/tables/TypeSpecTable.h"
#include "format/tables/AssemblyTable.h"
#include "format/tables/AssemblyRefTable.h"
#include "format/tables/NestedClassTable.h"
#include "format/tables/GenericParamTable.h"
#include "format/tables/MethodSpecTable.h"


TablePtr TableFactory::readTable(MetadataStream& stream, uint id,
                                 uint tablePosition)
{
    mdToken newToken = EncodingUtils::buildToken(id, tablePosition + 1);

    switch (id)
    {
    case TABLE_MODULE_TABLE:          return TablePtr(new ModuleTable(stream, newToken));
    case TABLE_TYPEREF_TABLE:         return TablePtr(new TyperefTable(stream, newToken));
    case TABLE_TYPEDEF_TABLE:         return TablePtr(new TypedefTable(stream, newToken));
    case TABLE_FIELD_TABLE:           return TablePtr(new FieldTable(stream, newToken));
    case TABLE_METHOD_TABLE:          return TablePtr(new MethodTable(stream, newToken));
    case TABLE_PARAM_TABLE:           return TablePtr(new ParamTable(stream, newToken));
    case TABLE_INTERFACEIMPL_TABLE:   return TablePtr(new InterfaceImplTable(stream, newToken));
    case TABLE_MEMBERREF_TABLE:       return TablePtr(new MemberRefTable(stream, newToken));
    case TABLE_CONSTANT_TABLE:        return TablePtr(new ConstantTable(stream, newToken));
    case TABLE_CUSTOMATTRIBUTE_TABLE: return TablePtr(new CustomAttributeTable(stream, newToken));
    case TABLE_DECLSECURITY_TABLE:    return TablePtr(new DeclSecurityTable(stream, newToken));
    case TABLE_CLASSLAYOUT_TABLE:     return TablePtr(new ClassLayoutTable(stream, newToken));
    case TABLE_STANDALONGESIG_TABLE:  return TablePtr(new StandAloneSigTable(stream, newToken));
    case TABLE_PROPERTYMAP_TABLE:     return TablePtr(new PropertyMapTable(stream, newToken));
    case TABLE_PROPERTY_TABLE:        return TablePtr(new PropertyTable(stream, newToken));
    case TABLE_METHODSEMANTICS_TABLE: return TablePtr(new MethodSemanticsTable(stream, newToken));
    case TABLE_METHODIMPL_TABLE:      return TablePtr(new MethodImplTable(stream, newToken));
    case TABLE_TYPESPEC_TABLE:        return TablePtr(new TypeSpecTable(stream, newToken));
    case TABLE_FIELDRVA_TABLE:        return TablePtr(new FieldRVATable(stream, newToken));
    case TABLE_ASSEMBLY_TABLE:        return TablePtr(new AssemblyTable(stream, newToken));
    case TABLE_ASSEMBLYREF_TABLE:     return TablePtr(new AssemblyRefTable(stream, newToken));
    case TABLE_NESTEDCLASS_TABLE:     return TablePtr(new NestedClassTable(stream, newToken));
    case TABLE_GENERICPARAM_TABLE:    return TablePtr(new GenericParamTable(stream, newToken));
    case TABLE_METHODSPEC_TABLE:      return TablePtr(new MethodSpecTable(stream, newToken));

    default:
        // The table #id is not ready or unknown...
        traceHigh("Cannot parse CLR file table: " << HEXDWORD(id) << endl);
        CHECK_FAIL();
    }
}

