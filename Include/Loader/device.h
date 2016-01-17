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

#ifndef __TBA_XDK_LOADER_DEVICE_H
#define __TBA_XDK_LOADER_DEVICE_H

/*
 * device.h
 *
 * The device is a module is wraps the operating system device-handle, performs
 * a reference-count over the device and have a single method for sending
 * commands (In a safe threaded modal)
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/refCountObject.h"
#include "xStl/os/osdef.h"
#include "loader/command.h"

/*
 * Manages a reference-count over a device-handle.
 * Usage:
 *
 * cDevice myObject(handle);   // Or loadable code.
 * myObject.invokeCommand(...);
 * startThread(myObject);
 */
class cDevice {
public:
    /*
     * Constructor. Load a device-driver from file-name and operating system
     * descriptor name.
     *
     * deviceName     - The device to be loaded into the system
     * deviceFilename - The device file location. If the device filename is NULL
     *                  then the loader assume that the device is already loaded
     *                  and don't try to register it or load it.
     *
     * Throws cDeviceException in case of failure
     */
    cDevice(const cString& deviceName,
            const cString& deviceFilename);

    /*
     * Decrease reference-count.
     */
    ~cDevice();

    /*
     * Return the OS handle. Make sure that the device is safe and lock as long
     * as the return code from this function might be in used.
     */
    cOSDef::deviceHandle getHandle() const;

    /*
     * Return the command-center object.
     * Throw exception if the command is not initialized.
     */
    cCommand& getCommand();

    // Make this class a reference-countable
    REFCOUNTABLE(cOSDef::deviceHandle);
    // Make this class supports copy-constructor and operator =
    REFCOUNTABLE_COPY_MORE(cDevice);

private:
    /*
     * Copy-constructor and operator = will invoke call to this function.
     */
    void copy(const cDevice& other);

    // The name of the device
    cString m_deviceName;
    // The file-name of the device
    cString m_deviceFilename;
    // The command-ceneter
    cCommand* m_command;
};

#endif // __TBA_XDK_LOADER_DEVICE_H
