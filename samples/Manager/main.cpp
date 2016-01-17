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
 * Main.cpp
 *
 * The main manager console API, load-register and remove device-drivers.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/data/char.h"
#include "xStl/data/string.h"
#include "xStl/stream/iostream.h"
#include "loader/loader.h"
#include "loader/device.h"
#include "loader/deviceException.h"
#include "loader/NTDeviceLoader.h"
#include "loader/console/consolePooler.h"
#include "InfiniteProgressBar.h"
#include "KbHit.h"

#include <stdio.h>

// The maximum number of character to be used in the console device.
#define MAX_LINE (77)

// Prints the application usage
void printUsage()
{
    cout << "Manager version 1.0" << endl << endl;
    cout << "Manager.exe B <file> <device> [-start]  Register a boot device driver." << endl;
    cout << "Manager.exe L <file> <device>           Dynamic loading of a device." << endl;
    cout << "Manager.exe C <file> <device>           Dynamic loading of a console device." << endl;
	cout << "Manager.exe U <device>                  Stop and unregister the device."  << endl;
    cout << endl << "(C) Integrity Project" << endl;
}


void handleBootDevice(const cString& filename,
					  const cString& devicename,
					  bool shouldStart)
{
	cout << "Register file '" << filename << "' as a boot device named '" << devicename << "'..." << endl;
	cNTServiceControlManager scm;
	scm.registerDevice(devicename, devicename, filename, true);
	if (shouldStart)
	{
		cout << "Starting device " << devicename << endl;
		scm.openDevice(devicename)->startService();
	}
}

void handleRegularDevice(const cString& filename,
						 const cString& devicename)
{
	cout << "Loading file '" << filename << "' for export device named '" << devicename << "'..." << endl;
	cDevice device(devicename, filename);
	cout << "Press any key to stop application..." << endl;
	char temp[MAX_PATH];
	scanf_s("%255s", temp);
	cout << "Unloading device." << endl;
}


void handleConsoleDevice(const cString& filename,
						 const cString& devicename)
{
	cout << "Loading file '" << filename << "' for export device named '" << devicename << "'..." << endl;
	cConsolePooler pooler(devicename, filename);
	InfiniteProgressBar bar;
	KbHit hit;
    cString readString;
	cout << "Press 'Q' key to stop application..." << endl << endl;
    bool shouldExit = false;

	uint32 start = GetTickCount();
	//uint32 i = 0;
	uint32 i = 580;

	while (!shouldExit)
	{
        if (hit.isHit())
        {
            if (cChar::getUppercase(hit.readKey()) == XSTL_CHAR('Q'))
                shouldExit = true;
        }

		if ((GetTickCount() - start) > 15000)
		{
			cout << i << endl;
			i++;
			if (i > 1000)
			{
				break;
			}
			start = GetTickCount();
		}

		// Print the message
		while (pooler.isStringPending())
		{
			bar.clear();
            readString = readString + pooler.poolString();
            // Write the until last new line character
            while (readString.find("\n") != readString.length())
            {
                uint pos = readString.find("\n");
                cout << readString.left(pos) << endl;
                readString = readString.right(readString.length() - pos - 1);
            }

            if (readString.length() > MAX_LINE)
            {
                cout << readString.left(MAX_LINE) << endl;
                readString = readString.right(readString.length() - MAX_LINE);
            }

            // Write the left over
			cout << readString;
		}

		bar.out();
		Sleep(30);
	}
	Sleep(30);
	cout << "Unloading device." << endl;
}

void handleUnregisterDevice(const cString& devicename)
{
	cout << "Unregistering device '" << devicename << "'..." << endl;
	cNTServiceControlManager scm;
	cNTServiceControlManager::cNTServicePtr device = scm.openDevice(devicename);
    XSTL_TRY
    {
        device->stopService();
    } XSTL_CATCH (cException& e)
    {
        cout << "Device '" << devicename << "'cannot be stopped: "
             << e.getMessage() << " (" << e.getID() << ")" << endl;
        cout << "Trying to delete registered device..." << endl;
    }
	device->deleteService();
    cout << "OK." << endl;
}


// The main console application

#define RETURN_USAGE_ERROR(string) { cout << string << endl; printUsage(); return RC_ERROR; }

int main(const uint argc, const char** argv)
{

	// Basic command line validity check.
	if (argc < 2)
	{
		RETURN_USAGE_ERROR("Not enough parameters");
	}

	XSTL_TRY
    {

		// Get the command.
        cString command(argv[1]);
		command.makeUpper();

        // Boot device registrator
        if (command == XSTL_STRING("B"))
        {
			if ((argc != 4) && (argc != 5))
			{
                RETURN_USAGE_ERROR("Not enough parameters for boot device");
			}

			bool shouldStart = false;
            if (argc == 5)
            {
				cString startParameter(argv[4]);
				startParameter.makeUpper();
				if (startParameter == XSTL_STRING("-START"))
				{
                    shouldStart = true;
				} else {
                    RETURN_USAGE_ERROR("'-start' parameter expected...");
				}
            }
            cString filename = argv[2];
            cString devicename = argv[3];

			handleBootDevice(filename, devicename, shouldStart);
			return RC_OK;
		}

        // Dynamic loading of a device
        if (command == XSTL_STRING("L"))
        {
			if (argc != 4)
			{
				RETURN_USAGE_ERROR("Not engouth parameters for loading a device");
			}
            cString filename = argv[2];
            cString devicename = argv[3];

			handleRegularDevice(filename, devicename);
			return RC_OK;
        }

        // Dynamic loading of a console device
        if (command == XSTL_STRING("C"))
        {
			if (argc != 4)
			{
                RETURN_USAGE_ERROR("Not enough parameters for loading a console device");
			}
            cString filename = argv[2];
            cString devicename = argv[3];

			handleConsoleDevice(filename, devicename);
			return RC_OK;
		}

		// Unregister the device.
		if (command == XSTL_STRING("U"))
		{
			if (argc != 3)
			{
				RETURN_USAGE_ERROR("Wrong parameters for unregistering a device.");
			}
			cString devicename = argv[2];

			handleUnregisterDevice(devicename);
			return RC_OK;
		}

        // Unknown command
        RETURN_USAGE_ERROR("Unknown commad");

        return RC_OK;
    }
    XSTL_CATCH(cException& e)
    {
        cout << "Manager: Exception occured: " << e.getMessage() << " (" << e.getID() << ")" << endl;
        return RC_ERROR;
    }
    XSTL_CATCH(...)
    {
        cout << "Manager: Unknown exception occured..." << endl;
        return RC_ERROR;
    }
}
