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

#ifndef __TBA_CLR_FORMAT_TABLES_METHODTABLE_H
#define __TBA_CLR_FORMAT_TABLES_METHODTABLE_H

/*
 * MethodTable.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "format/coreHeadersTypes.h"
#include "format/metadataStream.h"
#include "format/MetadataTables.h"
#include "format/tables/Table.h"

class MethodTable : public Table {
public:
    /*
     * Constructor. Reads a single 'Method' table descriptor
     *
     * stream - Reference into s stream which start at the table position.
     *          The stream reference will be advance to the beginning of the
     *          next table
     * token  - A full qualified index ID for this table
     *
     * Throw exception if the first stage of table verification failed.
     */
    MethodTable(MetadataStream& stream,
                mdToken token);

    /*
     * The header of the table as stored in the Clr...
     */
    struct Header {
        // The code location in the #~ data
        uint32 m_rva;
        // See MethodImplementationAttributes
        uint16 m_implementationFlags;
        // See MethodAttributes
        uint16 m_flags;
        // Index into #Strings heap.
        mdToken m_name;
        // Index into Blob.
        mdToken m_signature;
        // Index into Param table. This class also appends the param-table-ID
        mdToken m_params;
    };

    /*
     * Return the method header
     */
    const Header& getHeader() const;

    /*
     * Return the end-token of the method's param list.
     * If this method token is the last row index of method tables, then the
     * function returns the last param token. Otherwise the function returns
     * the start field of the next method table in this row
     */
    mdToken calculateEndParamToken(MetadataTables& tables) const;

    // See Table::getIndex.  Return TABLE_METHOD_TABLE
    virtual TablesID getIndex() const;
    // See Table::out.       Format the content of the Method-header
    #ifdef TRACED_CLR
    virtual void out(cStringerStream& out,
                     StringsTableFormatter& formatter) const;
    // See Table::getTableName
    virtual cString getTableName() const;
    // See Table::getTableMSILname
    virtual cString getTableMSILname() const;
    #endif // TRACED_CLR

    mdToken getName(void) const;
    mdToken getParamListIndex(void) const;

    enum MethodAttributes
    {
        // member access mask - Use this mask to retrieve accessibility information.
        mdMemberAccessMask          =   0x0007,
        // Member not referenceable.
        mdPrivateScope              =   0x0000,
        // Accessible only by the parent type.
        mdPrivate                   =   0x0001,
        // Accessible by sub-types only in this Assembly.
        mdFamANDAssem               =   0x0002,
        // Accessibly by anyone in the Assembly.
        mdAssembly                  =   0x0003,
        // Accessible only by type and sub-types.
        mdFamily                    =   0x0004,
        // Accessibly by sub-types anywhere, plus anyone in assembly.
        mdFamORAssem                =   0x0005,
        // Accessibly by anyone who has visibility to this scope.
        mdPublic                    =   0x0006,
        // end member access mask

        // method contract attributes.
        // Defined on type, else per instance.
        mdStatic                    =   0x0010,
        // Method may not be overridden.
        mdFinal                     =   0x0020,
        // Method virtual.
        mdVirtual                   =   0x0040,
        // Method hides by name+sig, else just by name.
        mdHideBySig                 =   0x0080,

        // vtable layout mask - Use this mask to retrieve vtable attributes.
        mdVtableLayoutMask          =   0x0100,
        // The default.
        mdReuseSlot                 =   0x0000,
        // Method always gets a new slot in the vtable.
        mdNewSlot                   =   0x0100,
        // end vtable layout mask

        // method implementation attributes.
        // Method does not provide an implementation.
        mdAbstract                  =   0x0400,
        // Method is special.  Name describes how.
        mdSpecialName               =   0x0800,

        // interop attributes
        // Implementation is forwarded through pinvoke.
        mdPinvokeImpl               =   0x2000,
        // Managed method exported via thunk to unmanaged code.
        mdUnmanagedExport           =   0x0008,

        // Reserved flags for runtime use only.
        mdReservedMask              =   0xd000,
        // Runtime should check name encoding.
        mdRTSpecialName             =   0x1000,
        // Method has security associate with it.
        mdHasSecurity               =   0x4000,
        // Method calls another method containing security code.
        mdRequireSecObject          =   0x8000
    };

    enum MethodImplementationAttributes
    {
        // code impl mask
        // Flags about code type.
        miCodeTypeMask      =   0x0003,
        // Method impl is IL.
        miIL                =   0x0000,
        // Method impl is native.
        miNative            =   0x0001,
        // Method impl is OPTIL
        miOPTIL             =   0x0002,
        // Method impl is provided by the runtime.
        miRuntime           =   0x0003,
        // end code impl mask

        // managed mask
        // Flags specifying whether the code is managed or unmanaged.
        miManagedMask       =   0x0004,
        // Method impl is unmanaged, otherwise managed.
        miUnmanaged         =   0x0004,
        // Method impl is managed.
        miManaged           =   0x0000,
        // end managed mask

        // implementation info and interop
        // Indicates method is defined; used primarily in merge scenarios.
        miForwardRef        =   0x0010,
        // Indicates method sig is not to be mangled to do HRESULT conversion.
        miPreserveSig       =   0x0080,

        // Reserved for internal use.
        miInternalCall      =   0x1000,

        // Method is single threaded through the body.
        miSynchronized      =   0x0020,
        // Method may not be inlined.
        miNoInlining        =   0x0008,
        // Range check value
        miMaxMethodImplVal  =   0xffff
    };

private:
    // The table's data
    Header m_header;
};

#endif // __TBA_CLR_FORMAT_TABLES_METHODTABLE_H
