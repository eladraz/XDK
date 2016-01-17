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
 * main.cpp
 *
 * The main entry point of the application
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/data/char.h"
#include "xStl/stream/ioStream.h"
#include "loader/loader.h"
#include "loader/device.h"
#include "common/const.h"
#include "client/PcSpeakerClient.h"

/*
 * The main entry point. Captures all unexcepected exceptions and make sure
 * that the application will notify the programmer.
 *
 * Invoke a call to the following modules:
 *   1. Load the device-driver
 *   2. Connects the RPC module to the driver object
 *   3. Activate the driver.
 */
int main(const uint argc, const character** argv)
{
    XSTL_TRY
    {
        cout << "PcPlayer version 1.0" << endl << endl;
        cout << "Welcome to PcPlayer - the first fully featured application written" << endl
             << "for the pc-speaker component." << endl << endl;

        if (cDeviceLoader::is64bitSystem())
        {
            cout << "This driver is been written for 32-bit process" << endl;
            return RC_ERROR;
        }

        cout << endl;
        cout << "Play audio files using the PcSpeaker device driver" << endl;
        cout << "Both driver and console application created by:" << endl;
        cout << " ELAD RAZ" << endl;
        cout << "  T.B.A. "<< endl;
        cout << "   2004  "<< endl;
        cout << endl;
        cout << "Please make sure the PcSpeaker file is located at: " << Const::PCSPEAKER_DEVICE_FILENAME << endl;
        cout << endl;
        cout << endl;
        cout << "Try accessing the device driver..." << endl;

        // Open the device
        cDevice myDevice(Const::PCSPEAKER_DEVICE_NAME,
                         Const::PCSPEAKER_DEVICE_FILENAME);

        // Create the client RPC object
        cPcSpeakerClient client(myDevice);

        client.setFrequency(300);
        client.soundOn();
        Sleep(1000);
        client.setFrequency(400);
        Sleep(1000);
        client.setFrequency(500);
        Sleep(1000);
        client.soundOff();

        // Close the device

        return RC_OK;
    }
    XSTL_CATCH(cException& e)
    {
        // Print the exception
        e.print();
        return RC_ERROR;
    }
    XSTL_CATCH_ALL
    {
        TRACE(TRACE_VERY_HIGH,
                XSTL_STRING("Unknwon exceptions caught at main()..."));
        return RC_ERROR;
    }
}
