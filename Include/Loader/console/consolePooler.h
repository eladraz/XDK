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

#ifndef __TBA_XDKLOADER_CONSOLE_CONSOLEPOOLER_H
#define __TBA_XDKLOADER_CONSOLE_CONSOLEPOOLER_H
/*
 * consolePooler.h
 *
 * The consolePooler is a device-stub which connects to a driver cConsoleDevice
 * and start pooling messages (by order) from that device.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/os/osdef.h"
#include "loader/command.h"
#include "XDK/utils/consoleDeviceIoctl.h"

/*
 * The pooler class tries to open a console device-driver object and pools
 * it's iostream messages (By a request).
 * Usage:
 *    For each priod of time query the 'cout' device and pool it's strings:
 *
 *    while(pooler.isStringPending())
 *    {
 *       cout << pooler.poolString();
 *    }
 *
 * NOTE: This class is not thread-safe!
 */
class cConsolePooler : public cConsoleDeviceIoctl {
public:
    /*
     * Constructor. Try to obtain a device handle.
     *
     * Throw exception if the console device is invalid or cannot be opened.
     */
    cConsolePooler(const cString& deviceName,
                   const cString& deviceFilename);

    /*
     * Destructor. Frees the device-driver.
     */
    ~cConsolePooler();

    /*
     * Return true if there is a string waiting in the queue.
     */
    bool isStringPending();

    /*
     * Returns the pending string.
     * Throws exception if there aren't any string waiting
     */
    cString poolString();


    /*
     * Taken from cConsoleDeviceControls.
     * See cConsoleDeviceControls for documentation
     */
    virtual uint getVersion();
    virtual uint getNextLineSize();
    virtual bool poolLine(uint8* outputLine, uint outputLineLength);

protected:
	// The command center for the device
    cCommand* m_command;

private:
    // The handle for the console device.
    cOSDef::deviceHandle m_consoleDevice;
    // The con-device name
    cString m_deviceName;
    // The con-device file-name
    cString m_deviceFilename;
    // The last query buffer size
    uint m_cacheStringSize;
    // Whether the isStringPending is called, and the m_cacheStringSize is valid.
    bool m_isPendingCalled;
};

#endif // __TBA_XDKLOADER_CONSOLE_CONSOLEPOOLER_H
