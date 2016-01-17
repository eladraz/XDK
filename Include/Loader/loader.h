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

#ifndef __TBA_XDK_LOADER_LOADER_H
#define __TBA_XDK_LOADER_LOADER_H

/*
 * loader.h
 *
 * This file contains the basic device-loader for the XDK package.
 *
 * So far the code had being implemented for Windows NT device-drivers
 *
 * Author: Elad Raz <e@eladraz.com>
 */

#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/os/osdef.h"

/*
 * cDeviceLoader
 *
 * This class responsiable for device-loading. The loader connects OS driver
 * with the application model. In order to load the device, the loader needs
 * the name of the device and it's pyhsical location.
 *
 * NOTE: This functions gets a strings in order to load the device. It's
 *       highly recommand to use functions which builds those strings.
 *       For example: A utility builds for windows enviroment can supports
 *                    both Windows 9x and Windows NT families. The name of
 *                    the device is different between these two envirmonet
 *                    (.sys and .vxd)
 *
 * TODO: Sync. between all application:
 *       If the module is not exist in the system, the loader should register
 *       the device, but only the last application should remove the device
 *       registration.
 */
class cDeviceLoader {
public:
    /*
     * Install and execute device handle
     *
     * deviceName     - The device to be loaded into the system
     * deviceFilename - The device file location. If the device filename is NULL
     *                  then the loader assume that the device is already loaded
     *                  and don't try to register it or load it.
     *
     * Return a handle to the device.
     * Throws cDeviceException in case of failure
     */
    static cOSDef::deviceHandle loadDevice(const cString& deviceName,
                                           const cString& deviceFilename);

    /*
     * Stop the executation of the device. (TODO: Add a reference-count to the
     * number of application uses in the device)
     * Unregister the device.
     *
     * deviceName     - The device to be loaded into the system
     * deviceFilename - The device file location. If the device filename is NULL
     *                  then the loader assume that the device is already loaded
     *                  and don't try to register it or load it.
     * handle         - The device handle
     *
     */
    static void unloadDevice(const cString& deviceName,
                             const cString& deviceFilename,
                             cOSDef::deviceHandle handle);

    /*
     * Return true if we are running in 64-bit operating system
     */
    static bool is64bitSystem();
};

#endif // __TBA_XDK_LOADER_LOADER_H
