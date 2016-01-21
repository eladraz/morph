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

#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/data/char.h"
#include "xStl/data/list.h"
#include "xStl/except/exception.h"
#include "tests.h"

/*
 * All the constructed class are register inside a link-list which execute them.
 */
class tests_container{
public:
    static cList<cTestObject*>& get_tests()
    {
        static cList<cTestObject*> g_tests;
        return g_tests;
    }
};


/*
 * cTestObject constructor
 *
 * Register the class to the g_tests class
 */
cTestObject::cTestObject()
{
    tests_container::get_tests().append(this);
}

bool cTestObject::isException = false;

/*
 * main
 *
 * Scan all registered modules and
 */
unsigned int main(const uint, const char**)
{
    uint32 modules_passed = 0;
    XSTL_TRY
    {
        uint32 total_counts = tests_container::get_tests().length();

        TESTS_LOG("Welcome to CLR test module" << endl);
        TESTS_LOG("The system contains tests for " << total_counts << " modules" << endl);
        TESTS_LOG("Start tests execuation" << endl << endl);


        // Spawn the tests.
        for (cList<cTestObject*>::iterator i = tests_container::get_tests().begin();
                                           i!= tests_container::get_tests().end();
                                           i++)
        {
            // Capture the test
            XSTL_TRY
            {
                TESTS_LOG("Testing " << (*i)->getName() << "... " << endl);
                (*i)->test();
                TESTS_LOG("PASSED!" << endl);
                // all good...
                modules_passed++;
            }
            XSTL_CATCH (cException& e)
            {
                e;
                TESTS_LOG("FAILED!" << endl);
                TESTS_LOG("Exception: " << e.getMessage() << " (" << e.getID() << ')' << endl);
                TESTS_LOG("Exception: Throw at module " << (*i)->getName() << endl);
            }
            XSTL_CATCH (...)
            {
                TESTS_LOG("FAILED!" << endl);
                TESTS_LOG("Exception: Unknown exception thrown..." << endl);
                TESTS_LOG("Exception: Throw at module " << (*i)->getName() << endl);
            }
        }

        if (modules_passed != total_counts)
        {
            TESTS_LOG(modules_passed << " out of " << total_counts << " passed ("
                 << ((modules_passed*100)/total_counts) << "%)" << endl);
            return RC_ERROR;
        }
    }
    XSTL_CATCH(...)
    {
        TESTS_LOG("Unexcpected error at tests module... "  << endl);
        TESTS_LOG("During test #" << modules_passed<< endl);
        return RC_ERROR;
    }

    TESTS_LOG("ALL TESTS PASSED!" << endl);
    return RC_OK;
}
