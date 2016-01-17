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
 * NTLoader.cpp
 *
 * Implementation file for NT operation system.
 *
 * The loader register the device as 'service' inside the registry and send a
 * 'start' command to the SCM manager. After it the device is created using
 * a 'IRP_MJ_CREATE' request to the NT-object manager. (::CreateFile with \\.\)
 *
 * NOTE: This operation require an administrator permissions.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/data/string.h"
#include "loader/loader.h"
#include "loader/deviceException.h"
#include "loader/NTDeviceLoader.h"


// The maximum digits allowed inside the NT-object manager name tree.
#define MAX_DEVICE_PATH (256)

/*
 * TODO: This is the normal implementation. The loadDevice register the device
 *       and start it. The unloadDevice, stop the device and unregister it.
 *
 *       There is one major problem with this implementation. In case two R3
 *       application access the same driver there will be sync. errors.
 */

cOSDef::deviceHandle cDeviceLoader::loadDevice(const cString& deviceName,
                                               const cString& deviceFilename)
{
    if (deviceFilename.length() > 0)
    {
        cNTServiceControlManager scm;
        // Register the device.
        scm.registerDevice(deviceName, deviceName, deviceFilename);
        // Start the device
        cSmartPtr<cNTServiceControlManager::cNTService> service =
            scm.openDevice(deviceName);
        service->startService();
    }

    // Open the device and return an handle for the device
    character completeDeviceName[MAX_DEVICE_PATH];

    if ((::GetVersion() & 0xFF) >= 5) // Windows 2000 or Whistler
    {
        // We reference the global name so that the application can
        // be executed in Terminal Services sessions on Win2K
        cChar::strncpy(completeDeviceName,
                    XSTL_STRING("\\\\.\\Global\\"),
                    MAX_DEVICE_PATH);
        cChar::strappend(completeDeviceName,
                        deviceName.getBuffer(),
                        MAX_DEVICE_PATH);
    } else
    {
        cChar::strncpy(completeDeviceName,
                    XSTL_STRING("\\\\.\\"),
                    MAX_DEVICE_PATH);
        cChar::strappend(completeDeviceName,
                        deviceName.getBuffer(),
                        MAX_DEVICE_PATH);
    }

    // Open an instance of the device (IRP_MJ_CREATE) and retrieve the FileObject
    cOSDef::deviceHandle ret = CreateFile(completeDeviceName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (ret == INVALID_HANDLE_VALUE)
    {
        // Cannot open the device
        XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
    }

    return ret;
}

void cDeviceLoader::unloadDevice(const cString& deviceName,
                                 const cString& deviceFilename,
                                 cOSDef::deviceHandle handle)
{
    // Close the handle for the device
    ::CloseHandle(handle);

    if (deviceFilename.length() > 0)
    {
        cNTServiceControlManager scm;
        // Stop the device
        scm.openDevice(deviceName)->stopService();
        // Unregister the device
        scm.unregisterDevice(deviceName);
    }
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

bool cDeviceLoader::is64bitSystem()
{
    BOOL bIsWow64 = FALSE;
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if(fnIsWow64Process != NULL)
    {
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    return bIsWow64 != FALSE;
}
