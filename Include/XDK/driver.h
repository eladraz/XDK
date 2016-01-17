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

#ifndef __TBA_XDK_DRIVER_H
#define __TBA_XDK_DRIVER_H

/*
 * driver.h
 *
 * Interface for OS driver object.
 * The interface wrapps the NT-driver-object, PDRIVER_OBJECT and the communication
 * from the operation system and the driver.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/hash.h"
#include "xStl/os/mutex.h"
#include "XDK/kernel.h"
#include "XDK/device.h"


/*
 * cDriver
 *
 * Interface which describes the communication between the NT operating system
 * driver object (PDRIVER_OBJECT) and the C++ XDK library. The XDK library inits
 * the C++ runtime, static objects and try to locate an instance of cDriver.
 * When the instance is found, the XDK library transfer control from the OS into
 * C++ driver object.
 * This class is an interface and singleton. Only one instance of the type cDriver
 * class is allowed.
 *
 * Usage:
 * Inherit a class from cDriver and implement the abstract functions. During the
 * driver runtime, the operating system callbacks will be translated to C++ call
 * into the class context.
 * In order to instance the cDriver classtype object a global object instansiation
 * is requiered.
 *
 * Here is a simple driver example:
 * class cMyDriver : public cDriver
 * {
 * public:
 *     virtual NTSTATUS driverEntry()
 *     {
 *         TRACE(TRACE_VERY_HIGH, "Hello world! From My driver!\n");
 *         // create device objects....
 *         return STATUS_SUCCESS;
 *     }
 * };
 *
 * Another example include generating a device-object:
 * class cMyDriver1 : public cDriver
 * {
 * public:
 *     virtual NTSTATUS driverEntry()
 *     {
 *         // Construct the device.
 *         m_device = new cMyDevice(this);
 *
 *         // Add the device to the queue
 *         addDevice(m_device);
 *
 *         // Return whether the generation succeded.
 *         return m_device->getStatus();
 *     }
 *
 * private:
 *    // The device can initialize both at the constructor and at the driverEnrty
 *    cDevicePtr m_device;
 * };
 *
 *
 * NOTE: During the device creation and the 'addDevice()' call any io-request
 *       that goes to the device will be omitted.
 *       The IRP can be generated only if the cretaed device will linked to the
 *       hardware or attached itself into exist device.
 *       In order to solve this probelm seperate between the device creation
 *       using a call to IoCreateDevice, and the attachment (IoAttachDevice)
 *       For example:
 *       virtual NTSTATUS driverEntry()
 *       {
 *         // Construct the device, IoCreateDevice
 *         m_device = new cMyDevice(this);
 *         // Add the device to the queue
 *         addDevice(m_device);
 *         // Link the device, using (IoAttachDevice, IoRegisterFS...)
 *         m_device->linkDevice();
 *       }
 *
 * see cDevice
 */
class cDriver
{
public:
    /*
     * Constructor.
     * Register the class instance to the singleton.
     *
     * NOTE: Since the cDriver is a global objects other global object cannot
     *       be accessed during the constructor!
     *
     * Throw exception in case cDriver object already exist.
     */
    cDriver();

    /*
     * Virtual destructor
     */
    virtual ~cDriver();

    /*
     * Return the instance for driver singleton object.
     *
     * Throws FAILD exception when:
     *  1. The object hasn't being initialized. (During global ctors)
     *  2. There aren't any instances of the cDriver object in the system
     *  3. There are more than one instances of the cDriver object
     */
    static cDriver& getInstance();

    /*
     * Return the driver-object for this driver.
     */
    static PDRIVER_OBJECT getDriverObject();

    /*
     * Handle the driver entry. Creates the devices needed for the driver
     * operation.
     *
     * In case of exception, the driver will unload and the destructors will be
     * called.
     *
     * Return the NT error-code of the DriverEntry function. This return code
     * goes to the operating system, Error-code (Other value than STATUS_SUCCESS)
     * will cause the termination of the driver.
     */
    virtual NTSTATUS driverEntry() = 0;

    /*
     * Add a device driver to the driver object.
     * Use this function to link a device's into the driver core.
     *
     * deviceObject - The device to be added to the reciepient devices list.
     *
     * Throws cDriverException.
     */
    void addDevice(cDevicePtr deviceObject);

    /*
     * Return the number of bytes needed in order to store
     */
    static uint getDeviceExtenstionSize();

protected:
    /*
     * The DriverEntry (driver's main) is a friend, since this function execute
     * the callback functions
     */
    friend class cXDKLibCPP;

    /*
     * Deny operator = and copy-constructor
     */
    cDriver(const cDriver& other);
    cDriver& operator = (const cDriver& other);

    /*
     * Change the driver object to point to the operating system handle.
     * Note: This function is called by the libC startup code.
     *
     * driverObject - The OS handle for the driver
     */
    static void setDriverObject(PDRIVER_OBJECT driverObject);

    /*
     * Called by the operating system in order to dispatch an IRP into a device
     * object.
     *
     * The function transfer the call to the cDriver instance for processing.
     *
     * deviceObject - The device which related to the IRP.
     * irp          - The IO-request-packet
     *
     * See dispatchIRP for more information
     */
    static NTSTATUS irpCallback(PDEVICE_OBJECT deviceObject,
                                PIRP irp);

    /*
     * Take an IRP and dispatch it to the device-object for processing.
     *
     * deviceObject - The device which related to the IRP.
     * irp          - The IO-request-packet
     *
     * See irpCallback.
     */
    NTSTATUS dispatchIRP(PDEVICE_OBJECT deviceObject,
                         PIRP irp);


    /*
     * Return the device object which represent the kernel handle 'deviceObject'.
     *
     * deviceObject - The device to query.
     *
     * Note: The device that returns increases the device-reference count in order
     *       to protect the driver from sudden depth.
     *
     * The functions throw cException.EXCEPTION_OUT_OF_RANGE exception if the device
     * object is not exist.
     */
    cDevicePtr getDevice(PDEVICE_OBJECT deviceObject);

    // Members:

    // The OS handle for the driver
    static PDRIVER_OBJECT m_driverObject;

    // The list of all devices.
    cHash<PDEVICE_OBJECT, cDevicePtr> m_devices;
    // The device-list lockable
    cMutex m_devicesLock;

    // The singleton object.
    static cDriver* m_singleton;
};

#endif // __TBA_XDK_DRIVER_H
