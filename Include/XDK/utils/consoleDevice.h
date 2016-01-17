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

#ifndef __TBA_XDK_UTILS_CONSOLEDEVICE_H
#define __TBA_XDK_UTILS_CONSOLEDEVICE_H

/*
 * consoleDevice.h
 *
 * The console-device is a device-driver which holds all con-out messages
 * and communicates with ring3 application that pools the messages and
 * print them to the user screen.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/utils/callbacker.h"
#include "XDK/kernel.h"
#include "XDK/device.h"
#include "XDK/unicodeString.h"
#include "XDK/utils/IoctlListener.h"
#include "XDK/utils/IoctlDispatcher.h"
#include "XDK/utils/consoleDeviceControls.h"

/*
 * class cConsoleDevice
 *
 * Communicates with ring3 application, pools the 'cout' messages and
 * deliever them to the console application.
 * The device is excsulsive, implements, create/close and IOCTLs.
 *
 * Note: The console is save in a queue. The driverEntry must return STATUS_SUCCESS
 *       to allow the device to be generated and pools the messages.
 */
class cConsoleDevice : public cDevice
{
public:
    /*
     * Inits the device-driver and add it to the console queue.
     *
     * deviceName - A string which represents the name of the device to be generated.
     *              This name musn't contains any spaces, slash or non-letter/digits/_
     *              characters.
     */
    cConsoleDevice(const cString& deviceName);

    /*
     * Called to notify about IRP_MJ_CLEANUP command. The driver can preform
     * cleaning at PASSIVE_LEVEL.
     *
     * cleanupNotification - The notification object
     */
    void registerCleanupNotification(const cCallbackPtr& cleanupNotification);

    /*
     * Destroy the device-driver
     */
    virtual ~cConsoleDevice();

    /*
     * See cDevice::handleIrp.
     *
     * Handles the communication with the ring3 application
     */
    virtual NTSTATUS handleIrp(PIRP irp);

    /*
     * See cDevice::getDeviceObject.
     */
    virtual PDEVICE_OBJECT getDeviceObject() const;

    /*
     * The following function are arguments parser for the cConsoleDeviceControls
     * class.
     * See cConsoleDeviceIoctl commands description.
     * See cConsoleDeviceControls for execuation mode.
     * See cIoctlListener for callback's fanctors
     */

    // See cConsoleDeviceControls::getVersion()
    uint handleGetVersionIoctl(uint    ioctlCode,
                          const uint8* inputBuffer,
                          uint         inputBufferLength,
                          uint8*       outputBuffer,
                          uint         outputBufferLength);

    // See cConsoleDeviceControls::getNextLineSize()
    uint handleGetNextLineIoctl(uint    ioctlCode,
                           const uint8* inputBuffer,
                           uint         inputBufferLength,
                           uint8*       outputBuffer,
                           uint         outputBufferLength);

    // See cConsoleDeviceControls::poolLine()
    uint handlePoolLineIoctl(uint    ioctlCode,
                        const uint8* inputBuffer,
                        uint         inputBufferLength,
                        uint8*       outputBuffer,
                        uint         outputBufferLength);

    // Create the thunks
    IOCTL_CALLBACK(cConsoleDevice, handleGetVersionIoctl);
    IOCTL_CALLBACK(cConsoleDevice, handleGetNextLineIoctl);
    IOCTL_CALLBACK(cConsoleDevice, handlePoolLineIoctl);

protected:
	// The dispatcher module for the IOCTLs
    cIoctlDispatcher m_ioctlDispatcher;
private:
    // The handle for this device driver
    PDEVICE_OBJECT m_device;

    // The main handler object
    cConsoleDeviceControls m_consoleControls;

    // Unload notification
    cCallbackPtr m_cleanupNotification;

    // The device names
    cUnicodeString m_deviceNtName;
    cUnicodeString m_deviceSymbolicName;
};

#endif // __TBA_XDK_UTILS_CONSOLEDEVICE_H
