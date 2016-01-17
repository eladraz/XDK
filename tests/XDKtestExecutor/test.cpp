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
 * test.cpp
 *
 * The main enrty point for the testing utility.
 * The application bring up the test-driver and perform the uni-testing
 * for the driver.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/iostream.h"
#include "xStl/os/osdef.h"
#include "loader/loader.h"
#include "loader/deviceException.h"
#include "loader/console/consolePooler.h"
#include "../const.h"

/*
 * The path for the device-driver
 */
static const character PATHNAME[] = XSTL_STRING("c:\\temp\\xdk_test.sys");

/*
 * Perform the testing and printing the result. The function performs the
 * following:
 * 1. Load the device-driver.
 * 2. During the loading of the device the testing will be run at ring0.
 * 3. Query the status of the driver and print the result to the console.
 * 4. Unload the device-driver.
 */
int main(const unsigned int argc, const char** argv)
{
    XSTL_TRY
    {
        cout << "Welcome for T.B.A. XDK testing utility" << endl;
        cout << "(C) Elad Raz T.B.A." << endl << endl;
        cout << "Load testing driver..." << endl;

        XSTL_TRY
        {
            // Open the cout device
            cConsolePooler deviceCout(XDKConsts::CON_DEVICENAME, PATHNAME);

            // Pools all messages
            while (deviceCout.isStringPending())
            {
                cout << deviceCout.poolString();
            }

            // Terminate device
            cout << endl << endl << endl << "Testing completed." << endl;
            return RC_OK;
        }
        XSTL_CATCH (cDeviceException& e)
        {
            cout << "Exception: An exception occured during device loading: "
                 << e.getMessage() << endl;
            return RC_ERROR;
        }
    }
    XSTL_CATCH(cException& e)
    {
        cout << "Exception: Exception throw from main: " << e.getMessage() << endl;
        return RC_ERROR;
    }
    XSTL_CATCH(...)
    {
        cout << "Exception: Unknown exception" << endl;
        return RC_ERROR;
    }
}
