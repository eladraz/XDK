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
 * consoleDevice.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/stream/traceStream.h"
#include "XDK/kernel.h"
#include "XDK/driver.h"
#include "XDK/utils/consoleDevice.h"

cConsoleDevice::cConsoleDevice(const cString& device) :
    m_device(NULL),
    m_cleanupNotification(NULL),
    // Generates the symbolic name and the NT object tree name
    m_deviceNtName(cString(m_ntDeviceName) + device),
    m_deviceSymbolicName(cString(m_symbolicDeviceName) + device)
{
    traceHigh("-------------------------------------" << endl);
    traceHigh("[*] Driver Start" << endl);

    traceMedium("CONSOLE: about to generate device " << device << "..." << endl);

    // Trace the messages
    traceMedium("CONSOLE: Symbolic name: " << cString(((PUNICODE_STRING)(m_deviceSymbolicName))->Buffer) << endl);
    traceMedium("CONSOLE: Device name: " << cString(((PUNICODE_STRING)(m_deviceNtName))->Buffer) << endl);

    // Creates the device
    NTSTATUS ret =
        IoCreateDevice(cDriver::getDriverObject(), // The driver-object
                       0,                          // Device-extenstion is no-longer needed
                       m_deviceNtName,             // The device name
                       FILE_DEVICE_UNKNOWN,        // Kernel-mode driver
                       0,                          // Characteristics
                       TRUE,                       // Exclusive
                       &m_device);

    // Test the return code.
    if ((!NT_SUCCESS(ret)) || (m_device == NULL))
    {
        traceHigh("CONSOLE: Cannot generate console-device: " << device << endl <<
                  "         error " << HEXDWORD(ret) << endl);
        m_device = NULL;
        return;
    }

    // Register IOCTLs
    m_ioctlDispatcher.registerIoctlHandler(cConsoleDeviceIoctl::IOCTL_CONSOLE_GETVERSION,
        IOCTL_INSTANCE(handleGetVersionIoctl));
    m_ioctlDispatcher.registerIoctlHandler(cConsoleDeviceIoctl::IOCTL_CONSOLE_GETNEXTLINE_SIZE,
        IOCTL_INSTANCE(handleGetNextLineIoctl));
    m_ioctlDispatcher.registerIoctlHandler(cConsoleDeviceIoctl::IOCTL_CONSOLE_POOL_LINE,
        IOCTL_INSTANCE(handlePoolLineIoctl));

    // Link the device into a name
    ret = IoCreateSymbolicLink(m_deviceSymbolicName, m_deviceNtName);
    if (!NT_SUCCESS(ret))
    {
        traceHigh("CONSOLE: Cannot symbolic-link the device: " << device << endl <<
                  "         error " << HEXDWORD(ret) << endl);

        // Destroy the device
        IoDeleteDevice(m_device);
        m_device = NULL;
        return;
    }

    // The device is successfuly generated
    traceMedium("CONSOLE: device generated successfuly" << endl);
    m_isValid = true;
}

void cConsoleDevice::registerCleanupNotification(
    const cCallbackPtr& cleanupNotification)
{
    m_cleanupNotification = cleanupNotification;
}

cConsoleDevice::~cConsoleDevice()
{
    traceMedium("CONSOLE: destroy device..." << endl);

    traceHigh("[*] Driver End" << endl);
    traceHigh("-------------------------------------" << endl);

    if (m_device != NULL)
    {
        if (m_isValid)
        {
            // Destroy symbolic link
            IoDeleteSymbolicLink(m_deviceSymbolicName);
        }
        // Delete device
        IoDeleteDevice(m_device);
    }
}

NTSTATUS cConsoleDevice::handleIrp(PIRP irp)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS ret = STATUS_UNSUCCESSFUL;

    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:
        // Open the device
        traceMedium("CONSOLE: Create invoke by client..." << endl);
        ret = STATUS_SUCCESS;
        break;
    case IRP_MJ_CLEANUP:
        if (!m_cleanupNotification.isEmpty())
        {
            traceMedium("CONSOLE: Notifying cleanup..." << endl);
            m_cleanupNotification->call(NULL);
        }
        ret = STATUS_SUCCESS;
        break;
    case IRP_MJ_CLOSE:
        // Close the device
        traceMedium("CONSOLE: Close invoke by client" << endl);
        ret = STATUS_SUCCESS;
        break;
    case IRP_MJ_DEVICE_CONTROL:
        // Handle IOCTL from user mode
        ret = m_ioctlDispatcher.handleIrp(irp);
        break;
    default:
        traceHigh("CONSOLE: Unknown IRP invoked: " << HEXDWORD(irpStack->MajorFunction) << endl);
    }

    irp->IoStatus.Status = ret;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return ret;
}

PDEVICE_OBJECT cConsoleDevice::getDeviceObject() const
{
    return m_device;
}

uint cConsoleDevice::handleGetVersionIoctl(uint    ioctlCode,
                      const uint8* inputBuffer,
                      uint         inputBufferLength,
                      uint8*       outputBuffer,
                      uint         outputBufferLength)
{
    ASSERT(ioctlCode == cConsoleDeviceIoctl::IOCTL_CONSOLE_GETVERSION);
    CHECK((inputBufferLength == 0) && (outputBufferLength == sizeof(uint32)));
    CHECK(outputBuffer != NULL);

    // Get the version and marshle it
    uint32 version = m_consoleControls.getVersion();
    *((uint32*)outputBuffer) = version;
    return sizeof(version);
}

uint cConsoleDevice::handleGetNextLineIoctl(uint    ioctlCode,
                       const uint8* inputBuffer,
                       uint         inputBufferLength,
                       uint8*       outputBuffer,
                       uint         outputBufferLength)
{
    ASSERT(ioctlCode == cConsoleDeviceIoctl::IOCTL_CONSOLE_GETNEXTLINE_SIZE);
    CHECK((inputBufferLength == 0) && (outputBufferLength == sizeof(uint32)));
    CHECK(outputBuffer != NULL);

    // Get the version and marshle it
    uint32 lineSize = m_consoleControls.getNextLineSize();
    *((uint32*)outputBuffer) = lineSize;
    return sizeof(lineSize);
}

uint cConsoleDevice::handlePoolLineIoctl(uint    ioctlCode,
                    const uint8* inputBuffer,
                    uint         inputBufferLength,
                    uint8*       outputBuffer,
                    uint         outputBufferLength)
{
    ASSERT(ioctlCode == cConsoleDeviceIoctl::IOCTL_CONSOLE_POOL_LINE);
    CHECK((inputBufferLength == 0) && (outputBufferLength >= sizeof(character)));
    CHECK(outputBuffer != NULL);

    // Get the version and marshle it
    CHECK(m_consoleControls.poolLine(outputBuffer, outputBufferLength));

    return outputBufferLength;
}
