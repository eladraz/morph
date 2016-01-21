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

#ifndef __TBA_CLR_ENGINE_DUMMY_DEFAULTSTRINGSTABLEFORMATTER_H
#define __TBA_CLR_ENGINE_DUMMY_DEFAULTSTRINGSTABLEFORMATTER_H

/*
 * This module is only good if TRACED CLR is defined!
 */
#ifdef TRACED_CLR

/*
 * DefaultStringsTableFormatter.h
 *
 * Implements the StringsTableFormatter interface using default CLR file-format
 * implementation.
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "format/metadataStream.h"
#include "format/MSILStreams.h"
#include "format/tables/StringsTableFormatter.h"

/*
 * The class takes MetadataHeader and extract references into the #Strings
 * and the #GUID streams.
 *
 * NOTE: This class is NOT thread-safe!
 *
 * Author: Elad Raz <e@eladraz.com>
 */
class DefaultStringsTableFormatter : public StringsTableFormatter {
public:
    /*
     * Default constructor.
     *
     * streams - The different streams of the MSIL file
     */
    DefaultStringsTableFormatter(MSILStreams& streams);

    // See StringsTableFormatter::getString
    virtual cString getString(mdToken strID) const;
    // See StringsTableFormatter::getGuidString
    virtual cString getGuidString(mdToken guidID) const;

private:
    // Deny copy-constructor and operator =
    DefaultStringsTableFormatter(const DefaultStringsTableFormatter& other);
    DefaultStringsTableFormatter& operator =
        (const DefaultStringsTableFormatter& other);

    // The maximum number of characters for entry in the #Strings.
    // Equal to MAX_PATH + some safity value
    enum { MAX_STRING = 270 };
    // The size of each entry in the GUID
    enum { GUID_SIZE = 16 };

    // The MSIL streams
    MSILStreams& m_streams;
};

#endif // TRACED_CLR


#endif // __TBA_CLR_ENGINE_DUMMY_DEFAULTSTRINGSTABLEFORMATTER_H
