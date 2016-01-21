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
 * ExternalModuleResolver.cpp
 *
 * Implementation file
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/tables/MethodTable.h"
#include "runnable/Apartment.h"
#include "runnable/StringReader.h"
#include "runnable/FrameworkMethods.h"
#include "runnable/CustomAttribute.h"
#include "data/ConstElements.h"
#include "executer/runtime/ExternalModuleResolver.h"
#include "executer/runtime/RuntimeClasses/Runtime.h"
#include "executer/ExecuterTrace.h"

// HACK: Remove this after apartment module is fixed.
//       Without this include, you will not be able to do:
//       apartment.getObjects().getSomething()
//       getObjects() returns GlobalContext.
#include "runnable/GlobalContext.h"

const char ExternalModuleResolver::gCustomAttributeImportClass[] = "Import";

ExternalModuleResolver::ExternalModuleResolver(const ApartmentPtr& apartment) :
    m_apartment(apartment)
{
}

bool ExternalModuleResolver::resolveExternalMethod(const TokenIndex& methodToken,
                                                   addressNumericValue& addr,
                                                   ExternalModuleFunctionEntry** method)
{
    // Check for clr-resolver to work as it should
    CHECK(EncodingUtils::getTokenTableIndex(getTokenID(methodToken)) != TABLE_MEMBERREF_TABLE);

    // Check for imports
    // Check for export name
    cString importName;
    ApartmentPtr apt = m_apartment->getApt(methodToken);
    CustomAttributes customAttributes;
    CustomAttribute::getAttributes(apt,
                                   getTokenID(methodToken),
                                   gCustomAttributeImportClass,
                                   customAttributes);

    if (0 == customAttributes.getSize())
        return false;

    XSTL_TRY
    {
        CustomAttributePtr customAttributePtr = customAttributes[0];
        importName = customAttributePtr->getArgument("importName").getStringValue();
    }
    XSTL_CATCH (CustomAttribute::MissingArgument)
    {
        // Do nothing, revert to default.
        return false;
    }

    if (importName.length() == 0)
        return false;

    // Nullify return address
    addr = 0;

    // Get signature

    Apartment& apartment = *apt;
    MethodTable& methodTable = (MethodTable&)(*(apartment.getTables().getTableByToken(getTokenID(methodToken))));

    // (Re)Read the method signature
    cForkStreamPtr blob = apartment.getStreams().getBlobStream()->fork();
    blob->seek(methodTable.getHeader().m_signature, basicInput::IO_SEEK_SET);
    MethodDefOrRefSignaturePtr methodSignature =
            MethodDefOrRefSignaturePtr(new
                                MethodDefOrRefSignature(*blob,
                                                        apartment.getUniqueID(),
                                                        apartment.getObjects().getTypedefRepository()));

    bool ret = resolveExternalMethodName(importName,
                                         *methodSignature,
                                         addr,
                                         method);
    if (!ret)
    {
        ExecuterTrace("Error at signature/import table! function " << endl << "\t" <<
                      importName << "   " << *methodSignature << " couldn't be found!" << endl);
        CHECK_FAIL();
    }
    return true;
}

bool ExternalModuleResolver::resolveExternalMethodName(const cString& importName,
                                                       const MethodDefOrRefSignature& methodSignature,
                                                       addressNumericValue& addr,
                                                       ExternalModuleFunctionEntry** method)
{
    ExternalModuleFunctionEntry* moduleTable =
        m_runtimeClass.getFunctionTable().getBuffer();

    for (uint i = 0; i < m_runtimeClass.getFunctionTable().getSize(); i++)
    {
        if (!moduleTable[i].m_isResolved)
        {
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_param1Type);
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_param2Type);
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_param3Type);
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_param4Type);
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_param5Type);
            m_apartment->getObjects().getTypedefRepository().resolveTyperef(moduleTable[i].m_returnType);
            moduleTable[i].m_isResolved = true;
        }

        if ((importName.compare(moduleTable[i].m_importName) == cString::EqualTo) &&
            (methodSignature.getReturnType() == moduleTable[i].m_returnType))
        {
            // Function is valid
            bool found = false;

            // Some bad ugly nasty ifs

            if (methodSignature.getParams().getSize() == 0) { found = true; goto done; }
            if (moduleTable[i].m_param1Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param1Type != methodSignature.getParams()[0]) goto done;

            if (methodSignature.getParams().getSize() == 1) { found = true; goto done; }
            if (moduleTable[i].m_param2Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param2Type != methodSignature.getParams()[1]) goto done;


            if (methodSignature.getParams().getSize() == 2) { found = true; goto done; }
            if (moduleTable[i].m_param3Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param3Type != methodSignature.getParams()[2]) goto done;

            if (methodSignature.getParams().getSize() == 3) { found = true; goto done; }
            if (moduleTable[i].m_param4Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param4Type != methodSignature.getParams()[3]) goto done;

            if (methodSignature.getParams().getSize() == 4) { found = true; goto done; }
            if (moduleTable[i].m_param5Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param5Type != methodSignature.getParams()[4]) goto done;

            if (methodSignature.getParams().getSize() == 5) { found = true; goto done; }
            if (moduleTable[i].m_param6Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param6Type != methodSignature.getParams()[5]) goto done;

            if (methodSignature.getParams().getSize() == 6) { found = true; goto done; }
            if (moduleTable[i].m_param7Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param7Type != methodSignature.getParams()[6]) goto done;

            if (methodSignature.getParams().getSize() == 7) { found = true; goto done; }
            if (moduleTable[i].m_param8Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param8Type != methodSignature.getParams()[7]) goto done;

            if (methodSignature.getParams().getSize() == 8) { found = true; goto done; }
            if (moduleTable[i].m_param9Type == ConstElements::gVoid) goto done;
            if (moduleTable[i].m_param9Type != methodSignature.getParams()[8]) goto done;


            if (methodSignature.getParams().getSize() == 9) { found = true; goto done; }

        done:
            if (found)
            {
                if (method != NULL)
                    *method = moduleTable + i;
                addr = moduleTable[i].m_executedFunctionAddress;
                return true;
            }
        }
    }

    return false;
}
