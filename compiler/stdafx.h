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
 * Precompiled header
 *
 * Author: Elad Raz <e@eladraz.com>
 */

#ifdef _DEBUG

#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK

#else

#define DEBUG_CLIENTBLOCK

#endif // _DEBUG


#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/data/alignment.h"
#include "xStl/data/hash.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"
#include "xStl/stream/forkStream.h"
#include "xStl/stream/basicIO.h"
#include "xStl/stream/memoryStream.h"
#include "xStl/enc/digest/Crc64.h"
#include "xStl/except/trace.h"
#include "xStl/parser/parser.h"

#include "dismount/assembler/StackInterface.h"

#include "data/exceptions.h"
#include "data/ConstElements.h"
#include "data/ElementType.h"

#include "format/EncodingUtils.h"
#include "format/tables/TablesID.h"
#include "format/tables/TypedefTable.h"
#include "format/tables/TypeSpecTable.h"
#include "format/tables/TyperefTable.h"
#include "format/tables/StandAloneSigTable.h"
#include "format/tables/MethodTable.h"
#include "format/tables/MemberRefTable.h"
#include "format/tables/FieldTable.h"
#include "format/tables/FieldRVATable.h"
#include "format/signatures/LocalVarSignature.h"
#include "format/signatures/MethodDefOrRefSignature.h"

#include "runnable/Apartment.h"
#include "runnable/GlobalContext.h"
#include "runnable/FrameworkMethods.h"
#include "runnable/ClrResolver.h"
#include "runnable/StringRepository.h"
#include "runnable/StringReader.h"
#include "runnable/CorlibNames.h"
#include "runnable/CustomAttribute.h"
#include "runnable/Stack.h"

