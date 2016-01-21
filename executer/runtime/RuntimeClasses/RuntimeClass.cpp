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
 * RuntimeClass.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#if 0
#include "SPIBridge/CC3000SPIBridgePC/CC3000HostDriver/wlan.h"
#include "SPIBridge/CC3000SPIBridgePC/CC3000HostDriver/win32_names.h"
#include "SPIBridge/CC3000SPIBridgePC/CC3000HostDriver/socket.h"
#include "SPIBridge/CC3000SPIBridgePC/CC3000HostDriver/bridge_wrapper.h"
#endif

#include "xStl/types.h"
#include "executer/runtime/RuntimeClasses/RuntimeClass.h"
#include "executer/runtime/RuntimeClasses/Runtime.h"

#ifdef WITH_GWS
#include "executer/runtime/RuntimeClasses/EmbeddedUIX.h"
#endif

#include "data/ConstElements.h"

// Cannot use ConstElements since it's also a static...
#define sVOID    ElementType(ELEMENT_TYPE_VOID, 0,  false, false, false, TokenIndex(-1, -1))
#define sVOIDPTR ElementType(ELEMENT_TYPE_VOID, 1,  false, false, false, TokenIndex(-1, -1))
#define sU4      ElementType(ELEMENT_TYPE_U4,   0,  false, false, false, TokenIndex(-1, -1))
#define sU4PTR   ElementType(ELEMENT_TYPE_U4,   1,  false, false, false, TokenIndex(-1, -1))
#define sI4      ElementType(ELEMENT_TYPE_I4,   0,  false, false, false, TokenIndex(-1, -1))
#define sU2      ElementType(ELEMENT_TYPE_U2,   0,  false, false, false, TokenIndex(-1, -1))
#define sU1PTR   ElementType(ELEMENT_TYPE_U1,   1,  false, false, false, TokenIndex(-1, -1))
//#define sSTRING  ElementType(ELEMENT_TYPE_STRING)
#define sCHAR    ElementType(ELEMENT_TYPE_CHAR, 0,  false, false, false, TokenIndex(-1, -1))
#define sCHARPTR ElementType(ELEMENT_TYPE_CHAR, 1,  false, false, false, TokenIndex(-1, -1))
#define sOBJECT  ElementType(ELEMENT_TYPE_OBJECT, 0,false, false, false, TokenIndex(-1, -1))
#define sBOOL    ElementType(ELEMENT_TYPE_BOOLEAN,0,false, false, false, TokenIndex(-1, -1))
//#define sBYTEPTR ElementType(ELEMENT_TYPE_I1, 1)


static const ExternalModuleFunctionEntry gDebugFunctions[] = {
    { "_malloc",               sVOIDPTR, sU4,      sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::allocate),             false   },
    { "_free",                 sVOID,    sVOIDPTR, sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::free),                 false   },
    { "_callFunction",         sVOID,    sVOIDPTR, sVOIDPTR,   sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::callFunction),         false   },
    { "_debugChar",            sVOID,    sCHAR,    sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::DebugCh),              false   },
    { "_assertFalse",          sVOID,    sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::AssertFalse),          false   },
    { "_unhandledException",   sVOID,    sOBJECT,  sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::unhandledException),   false   },
    { "_getTickCount",         sI4,      sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID,    sVOID, sVOID, sVOID, getNumeric((void*)Runtime::getTickCount),         false   },

#ifdef WITH_GWS
    { "_displayInit",          sVOID,    sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::initClrUIX),        false   },
    { "_displayGetWidth",      sI4,      sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID ,    sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::getWidth),          false   },
    { "_displayGetHeight",     sI4,      sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::getHeight),         false   },
    { "_displayFillRectangle", sVOID,    sI4,      sI4,        sI4,   sI4,   sI4,   sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::fillRectangle),     false   },
    { "_displayDrawRectangle", sVOID,    sI4,      sI4,        sI4,   sI4,   sI4,   sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::drawRectangle),     false   },
    { "_displayIsKeyPressed",     sBOOL, sVOID,    sVOID,      sVOID, sVOID, sVOID, sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::isKeyPressed),      false   },
    { "_displayGetPotentiometer", sI4,   sVOID,    sVOID,      sVOID, sVOID,  sVOID, sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::getPotentiometer), false   },
    { "_displayText",             sVOID, sI4,      sI4,        sCHARPTR, sU4, sVOID, sVOID,     sVOID,  sVOID,  sVOID, getNumeric(EmbeddedUIX::text), false   },
#endif

#if 0
    { "_wlan_init_wrapper",           sVOID,  sVOID,  sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_init_wrapper),            false   },
    { "_wlan_start",                  sVOID,  sU2,    sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_start),                   false   },
    { "_wlan_stop",                   sVOID,  sU2,    sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_stop),                    false   },
    { "_wlan_connect",                sI4,    sU4,    sCHARPTR,  sI4,      sU1PTR,   sU1PTR,   sI4,    sVOID,  sVOID, sVOID, getNumeric(wlan_connect),                 false   },
    { "_wlan_is_connected",           sU4,    sVOID,  sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(IsCC3000Connected),            false   },
    { "_wlan_is_dhcp",                sU4,    sVOID,  sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(IsCC3000DHCP),                 false   },
    { "_wlan_disconnect",             sI4,    sVOID,  sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_disconnect),              false   },
    { "_wlan_ioctl_statusget",        sI4,    sVOID,  sVOID,     sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_ioctl_statusget),         false   },
    { "_wlan_ioctl_get_scan_results", sI4,    sU4,    sCHARPTR,  sVOID,    sVOID,    sVOID,    sVOID,  sVOID,  sVOID, sVOID, getNumeric(wlan_ioctl_get_scan_results),  false   },
    { "_wlan_ioctl_set_scan_params",  sI4,    sU4,    sU4,       sU4,      sU4,      sU4,      sI4,    sU4,    sU4,   sU4,   getNumeric(wlan_ioctl_set_scan_params),   false   },

    { "_socket",                      sI4,    sI4,      sI4,       sI4,      sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_socket),                false   },
    { "_closesocket",                 sI4,    sI4,      sVOIDPTR,  sVOID,    sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_closesocket),           false   },
    { "_accept",                      sI4,    sI4,      sVOIDPTR,  sU4PTR,   sU4PTR,   sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_accept),                false   },
    { "_bind",                        sI4,    sI4,      sVOIDPTR,  sI4,      sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_bind),                  false   },
    { "_listen",                      sI4,    sI4,      sI4,       sVOID,    sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_listen),                false   },
    { "_connect",                     sI4,    sI4,      sVOIDPTR,  sI4,      sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_connect),               false   },
    { "_select",                      sI4,    sI4,      sVOIDPTR,  sVOIDPTR, sVOIDPTR, sVOIDPTR, sVOIDPTR, sVOID, sVOID, sVOID, getNumeric(CC3000_select),                false   },
    { "_recv",                        sI4,    sI4,      sVOIDPTR,  sI4,      sI4,      sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_recv),                  false   },
    { "_recvfrom",                    sI4,    sI4,      sVOIDPTR,  sI4,      sI4,      sVOIDPTR, sU4PTR,   sVOID, sVOID, sVOID, getNumeric(CC3000_recvfrom),              false   },
    { "_send",                        sI4,    sI4,      sVOIDPTR,  sI4,      sI4,      sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_send),                  false   },
    { "_sendto",                      sI4,    sI4,      sVOIDPTR,  sI4,      sI4,      sVOIDPTR, sU4,      sVOID, sVOID, sVOID, getNumeric(CC3000_sendto),                false   },
    { "_resolve_hostname",            sI4,    sCHARPTR, sU2,       sU4PTR,   sVOID,    sVOID,    sVOID,    sVOID, sVOID, sVOID, getNumeric(CC3000_gethostbyname),         false   },
#endif

};

RuntimeClass::RuntimeClass() :
    m_debugClassFunctionsTable(gDebugFunctions,
        sizeof(gDebugFunctions)/sizeof(ExternalModuleFunctionEntry), 0)
{
}

ExternalModuleFunctionsTable& RuntimeClass::getFunctionTable()
{
    return m_debugClassFunctionsTable;
}
