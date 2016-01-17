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

#ifndef __TBA_XDK_DEVICE_H
#define __TBA_XDK_DEVICE_H

/*
 * device.h
 *
 * Defines the cDevice class which wraps the PDEVICE_OBJECT kernel object.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/SmartPtr.h"
#include "XDK/kernel.h"

/*
 * cDevice
 *
 * Interface for device objects. The device object is a wrapper for the DEVICE
 * kernel object. The interface is a callbacker from the operation system. When
 * a command is send to the device, it's first sends to the DRIVER object. Using
 * the cDriver class, the command will be invoke as a virtual class callback
 * operation.
 *
 * Note, this class is design as a base for all devices types, including kernel
 *       devices, filter devices and plug&play devices.
 *
 * See cDriver.
 *
 * Usage:
 *   In order to create a device object, a new class should be constructed base
 *   on the cDevice class. The implementation should implement the virtual
 *   functions according to the requierment.
 *
 *   class cMyDevice : public cDevice
 *   {
 *   public:
 *       // Create the device
 *       cMyDevice() : m_device(NULL)
 *       {
 *           // Generate the device
 *           m_device = IoCreateDevice(...)
 *       }
 *
 *       ~cMyDevice()
 *       {
 *          if (m_device != NULL)
 *             IoDeleteDevice(m_device)
 *       }
 *
 *       virtual NTSTATUS handleIrp(PIRP irp)
 *       {
 *          ...
 *       }
 *
 *       virtual PDEVICE_OBJECT getDeviceObject() { return m_device; }
 *
 *   private:
 *       PDEVICE_OBJECT m_device;
 *   }
 */
class cDevice
{
public:
    /*
     * Virtual destructor. Remove the device.
     */
    virtual ~cDevice();

    /*
     * Callback when irp dispatch to the device.
     * Note: The function can be called at both passive and dispatch level
     *
     * irp - The system IRP.
     *
     * Return NTSTATUS which will transfer back to the operation system. This
     * value is NOT the IoStatus value within the irp.
     */
    virtual NTSTATUS handleIrp(PIRP irp) = 0;

    /*
     * Return the device-object handle.
     *
     * This function is in used by the cDriver module in order to dispatch IRP
     * from the operating system into the device's class instance.
     */
    virtual PDEVICE_OBJECT getDeviceObject() const = 0;

    /*
     * Return true if the device constructed successfuly or false if some error
     * occured.
     */
    bool isValid() const;

protected:
    /*
     * Default constructor.
     * The inherit constructor must change the validance of the class to true.
     */
    cDevice();

    // The prefix for the NT-object device-driver tree
    static const character m_ntDeviceName[];
    // The prefix for the NT device-driver symbolic name
    static const character m_symbolicDeviceName[];

    /*
     * Set to true when the constructor generate the device successfuly.
     */
    bool m_isValid;

private:
    /*
     * Deny operator = and copy-constructor
     */
    cDevice(const cDevice& other);
    cDevice& operator = (const cDevice& other);
};

/// A device reference count
typedef cSmartPtr<cDevice> cDevicePtr;

// Translate between a device-object and a point into the hash table
uint cHashFunction(const PDEVICE_OBJECT& index, uint range);

#endif // __TBA_XDK_DEVICE_H
