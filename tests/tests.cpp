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
 * tests.cpp
 *
 * This file is main for the testing utilities.
 * The testing are set of uni-test modules which simulate the system and test
 * it's result for pre-known values.
 *
 * This file is a delagtion for the xStl test utilities and contains XDK test.
 * The tests are running in ring0.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xdk/kernel.h"
#include "xStl/types.h"
#include "xStl/types.h"
#include "xStl/data/char.h"
#include "xStl/data/list.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/stream/ioStream.h"
#include "XDK/ehlib/ehlib.h"
#include "XDK/utils/consoleDevice.h"
#include "testDriver.h"
#include "const.h"
#include "../tests/tests.h"

/*
 * All the constructed class are register inside a link-list
 * which execute them.
 */
class TestsContainer{
public:
    static cList<cTestObject*>& getTests()
    {
        static cList<cTestObject*> globalTests;
        return globalTests;
    }
};


bool cTestObject::isException = false;

/*
 * cTestObject constructor
 *
 * Register the class to the global test list
 */
cTestObject::cTestObject()
{
    TestsContainer::getTests().append(this);
}


#define MAX_STRING (400)
character* makeString(char* string)
{
    static character temp[MAX_STRING];
    uint len = t_min((int)strlen(string), (int)MAX_STRING);
    for (uint i = 0; i < len; i++)
    {
        temp[i] = (character)string[i];
    }
    temp[len] = cChar::getNullCharacter();
    return temp;
}

cTestDriver::cTestDriver() :
    m_consoleDevice(NULL)
{
}

cTestDriver::~cTestDriver()
{
}

/*
 * The driver entry-point.
 *
 * Scan all registered modules and run thier tests.
 */
NTSTATUS cTestDriver::driverEntry()
{
    // Create the console driver
    m_consoleDevice = cDevicePtr(new cConsoleDevice(XDKConsts::CON_DEVICENAME));
    // Add the device
    addDevice(m_consoleDevice);

    uint totalCounts = TestsContainer::getTests().length();

    // print logo ;)
    cout << endl;
    cout << "Wellcome to xStl test module" << endl;
    cout << endl;
    cout << endl;
    cout << "xStl written by Elad Raz T.B.A." << endl;
    cout << "The system contains tests for " << totalCounts << " modules" << endl;
    cout << "Start tests execuation" << endl << endl;

    uint32 modulesPassed = 0;
    // Spawn the tests.
    for (cList<cTestObject*>::iterator i = TestsContainer::getTests().begin();
                                       i!= TestsContainer::getTests().end();
                                       i++)
    {
        // Capture the test
        XSTL_TRY
        {
            (*i)->test();

            // all good...
            modulesPassed++;

            cout << "TEST: module " << (*i)->getName() << " completed OK." << endl;
        }
        XSTL_CATCH (cException& e)
        {
            cout << "TEST: module " << (*i)->getName() << " FALIED.  Exception: " << e.getMessage() << " (" << e.getID() << ")" << endl;
        }
        XSTL_CATCH_ALL
        {
            cout << "TEST: module " << (*i)->getName() << " FALIED.  [" << EHLib::getUnknownException() << "]" << endl;
        }
    }

    cout << endl;
    cout << endl;
    cout << "==================" << endl;

    if (modulesPassed == totalCounts)
    {
        cout << "Test completed OK!";
    } else
    {
        cout << "FAILED  " << modulesPassed << " out of " << totalCounts << " passed ("
             << ((modulesPassed*100)/totalCounts) << "%)" << endl;
    }

    // Wait until the ring3 application will print out the messages
    // and close the driver.
    return STATUS_SUCCESS;
}

/*
 * The cDriver object one and only instance
 */
cTestDriver gDriverObject;

