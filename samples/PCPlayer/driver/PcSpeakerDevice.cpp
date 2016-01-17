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
 * PcSpeakerDevice.cpp
 *
 * Implementation file for the pc-player device-driver.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/except/exception.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "XDK/kernel.h"
#include "XDK/device.h"
#include "driver/PcSpeakerDevice.h"
#include "driver/PcSpeakerServer.h"
#include "common/PcSpeakerIoctl.h"

PcSpeakerDevice::PcSpeakerDevice(const cString& name) :
    m_device(NULL),
    m_rpcServer(NULL),
    // Generates the symbolic name and the NT object tree name
    m_deviceNtName(cString(m_ntDeviceName) + name),
    m_deviceSymbolicName(cString(m_symbolicDeviceName) + name)
{
    traceMedium("PcSpeakerDevice: about to generated device " << name <<
                "..." << endl);

    // Creates the device
    NTSTATUS ret =
        IoCreateDevice(cDriver::getDriverObject(), // The driver-object
                    0,                    // Device-extention is no-longer needed
                    m_deviceNtName,       // The device name
                    FILE_DEVICE_UNKNOWN,  // Kernel-mode driver
                    0,                    // Characteristics
                    FALSE,                // Exclusive
                    &m_device);

    // Test the return code.
    if ((!NT_SUCCESS(ret)) || (m_device == NULL))
    {
        traceHigh("PcSpeakerDevice: Cannot generate console-device: " <<
                  name << endl << "         error " << HEXDWORD(ret) << endl);
        m_device = NULL;
        // Throw exception
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }

    // Register IOCTL RPC stub...
    m_rpcServer = cSmartPtr<cPcSpeakerIoctl>(new
        cPcSpeakerServer(m_ioctlDispatcher));

    // Link the device into a name
    ret = IoCreateSymbolicLink(m_deviceSymbolicName, m_deviceNtName);
    if (!NT_SUCCESS(ret))
    {
        traceHigh("PcSpeakerDevice: Cannot symbolic-link the device: " <<
                  name << endl << "         error " << HEXDWORD(ret) << endl);
        // Destroy the device
        IoDeleteDevice(m_device);
        m_device = NULL;
        // Throw exception
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }

    // The device is successfuly generated
    traceMedium("PcSpeakerDevice: device generated successfuly" << endl);
    m_isValid = true;
}

PcSpeakerDevice::~PcSpeakerDevice()
{
    if (m_device != NULL)
    {
        traceMedium("PcSpeakerDevice: destroy device..." << endl);
        if (m_isValid)
        {
            // Destroy symbolic link
            IoDeleteSymbolicLink(m_deviceSymbolicName);
        }
        // Delete device
        IoDeleteDevice(m_device);
    }
}

NTSTATUS PcSpeakerDevice::handleIrp(PIRP irp)
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS ret = STATUS_UNSUCCESSFUL;

    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:
        // Open the device
        traceMedium("PcSpeakerDevice: Create invoke by client..." << endl);
        ret = STATUS_SUCCESS;
        break;
    case IRP_MJ_CLOSE:
    case IRP_MJ_CLEANUP:
        // Close the device
        traceMedium("PcSpeakerDevice: Close invoke by client" << endl);
        ret = STATUS_SUCCESS;
        break;
    case IRP_MJ_DEVICE_CONTROL:
        // Handle IOCTL from user mode
        ret = m_ioctlDispatcher.handleIrp(irp);
        break;
    default:
        traceHigh("PcSpeakerDevice: Unknown IRP invoked: " <<
                  HEXDWORD(irpStack->MajorFunction) << endl);
    }

    irp->IoStatus.Status = ret;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return ret;
}

PDEVICE_OBJECT PcSpeakerDevice::getDeviceObject() const
{
    return m_device;
}
