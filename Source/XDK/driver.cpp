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
 * driver.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/stream/traceStream.h"
#include "xStl/data/hash.h"
#include "xStl/data/datastream.h"
#include "xStl/os/lock.h"
#include "xStl/os/mutex.h"
#include "XDK/kernel.h"
#include "XDK/driver.h"

// Meanwhile there aren't any instances of the class
cDriver* cDriver::m_singleton = NULL;

// The global driver object
PDRIVER_OBJECT cDriver::m_driverObject = NULL;

cDriver::cDriver()
{
    ASSERT(m_singleton == NULL);
    if (m_singleton != NULL)
    {
        TRACE(TRACE_VERY_HIGH,
              "cDriver: Constructor failed, multiple instances confliction.\n");
        // There is already instance of the class!
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }

    m_singleton = this;
}

cDriver::~cDriver()
{
    // The hash-table will destruct it's self and the devices reference-count
    // will remove the generated devices.
}

cDriver& cDriver::getInstance()
{
    ASSERT(m_singleton != NULL);

    // Test whether the cDriver object was instanced
    if (m_singleton == NULL)
    {
        TRACE(TRACE_VERY_HIGH,
              "cDriver: getInstance() cannot find cDriver instance.\n");
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }

    return *m_singleton;
}

void cDriver::setDriverObject(PDRIVER_OBJECT driverObject)
{
    ASSERT(driverObject != NULL);
    m_driverObject = driverObject;
}

void cDriver::addDevice(cDevicePtr deviceObject)
{
    // Test the device
    CHECK(!deviceObject.isEmpty());
    CHECK(deviceObject->isValid());

    cLock lock(m_devicesLock);
    // Add the device into the device list
    m_devices.append(deviceObject->getDeviceObject(), deviceObject);
}

NTSTATUS cDriver::irpCallback(PDEVICE_OBJECT deviceObject, PIRP irp)
{
    XSTL_TRY
    {
        return cDriver::getInstance().dispatchIRP(deviceObject, irp);
    }
    XSTL_CATCH_ALL
    {
        TRACE(TRACE_VERY_HIGH,
              XSTL_STRING("cDriver: BUG! Uncaught exception throw when irp dispatched\n"));
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS cDriver::dispatchIRP(PDEVICE_OBJECT deviceObject, PIRP irp)
{
    // Gets the device object
    cDevicePtr device = getDevice(deviceObject);

    // Test the validance of the pointer
    if (device.isEmpty())
    {
        TRACE(TRACE_VERY_HIGH,
              XSTL_STRING("cDriver: BUG! Device pointer is invalid\n"));
        return STATUS_UNSUCCESSFUL;
    }

	// If the Interrupt flag was off, it caused an exception in bugcheck in cProcessorLock
	// The issue was resolved, and this print remained for sanity.
	if (!cProcessorLock::isInterruptFlagSetEnabled()) {
		TRACE(TRACE_VERY_HIGH, XSTL_STRING("cProcessorLock::isInterruptFlagSetEnabled is not enabled\n"));
    }


    // Dispatch the command
    return device->handleIrp(irp);
}

cDevicePtr cDriver::getDevice(PDEVICE_OBJECT deviceObject)
{
    cLock lock(m_devicesLock);
    // If the device is not exist return NULL pointer.
    if (!m_devices.hasKey(deviceObject))
    {
        return cDevicePtr(NULL);
    }
    // Return the device object
    return m_devices[deviceObject];
}

PDRIVER_OBJECT cDriver::getDriverObject()
{
    ASSERT(m_driverObject != NULL);
    return m_driverObject;
}
