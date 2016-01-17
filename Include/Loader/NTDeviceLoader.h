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

#ifndef __TBA_XDK_LOADER_NT_DEVICELOADER_H
#define __TBA_XDK_LOADER_NT_DEVICELOADER_H

/*
 * NTDeviceLoader.h
 *
 * Header class for the
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/smartptr.h"

/*
 * This is an helper class which manage the SCM registration and the device
 * loading for Windows NT family operation systems
 */
class cNTServiceControlManager
{
public:
    // Forward declartion
    class cNTService;
    typedef cSmartPtr<cNTService> cNTServicePtr;

    /*
     * Constructor for the class. Obtain the handler for the SCM.
     *
     * Throws 'FAILD' exception in case of faliure (Access denied).
     */
    cNTServiceControlManager();

    /*
     * Destructor for the class. Close all handles for the SCM.
     */
    ~cNTServiceControlManager();

    /*
     * Register a device-driver into the SCM.
     *
     * driverName  - The name of the driver. Cannot contains any forward-slash /
     *               ot back-slash \. The maximum length of the string should be
     *               less than 256 digits.
     *               The name of the driver will act as unique identifier for
     *               accessing the device.
     * displayName - The string that will describes the device for the user
     * pathName    - The full-path of the binary
     * bootTime    - Set to true if the device is a boot device
     *
     * If the service is already registered the request is ignored.
     *
     * Throws 'FAILD' exception in case of faliure.
     *
     * Return false is the device is already registered, even if the parameters
     * are changed.
     */
    bool registerDevice(const cString& driverName,
                        const cString& displayName,
                        const cString& pathName,
                        bool bootTime = false);

    /*
     * Deletes the driver service.
     *
     * driverName  - The name of the driver. Cannot contains any forward-slash /
     *               ot back-slash \. The maximum length of the string should be
     *               less than 256 digits.
     *               The name of the driver will act as unique identifier for
     *               accessing the device.
     *
     * Throws 'FAILD' exception in case of faliure.
     */
    void unregisterDevice(const cString& driverName);

    /*
     * Open a device.
     *
     * Return an handler for the service.
     *
     * Throws 'FAILD' exception in case of faliure.
     */
    cNTServicePtr openDevice(const cString& driverName);

    /*
     * Return a handle to the Service Control Manager
     */
    SC_HANDLE getSCMhandle() const;

private:
    // The handler for the SCM
    SC_HANDLE m_scmHandle;

public:
    /*
     * Manages a driver-object for the SCM.
     */
    class cNTService
    {
    public:
        /*
         * Private constructor. Open a service for modification
         *
         * scmManager  - The manager of the SCM
         * serviceName - Unique identifier for the service to be opened.
         *
         * Throws 'FAILD' exception in case of faliure (Access denied).
         */
        cNTService(cNTServiceControlManager* scmManager,
                   const cString& serviceName);

        /*
         * Destructor. Free the handle for the opened service.
         */
        ~cNTService();

        /*
         * Send a 'START' command for the service.
         * If the service is already running, the command will be ignored.
         *
         * Throws 'FAILD' exception in case of failure.
         */
        void startService();

        /*
         * Send a 'STOP' command for the service.
		 * The command will be ignored if the service is not running.
         *
         * Throws 'FAILD' exception in case of failure.
         */
        void stopService();

        /*
         * Unregister the service from the SCM.
         *
         * Throws 'FAILD' exception in case of faliure.
         */
        void deleteService();

    private:
        // A pointer to the SCM manager
        cNTServiceControlManager* m_scmManager;

        // The handle for the service
        SC_HANDLE m_serviceHandle;
    };
};


#endif // __TBA_XDK_LOADER_NT_DEVICELOADER_H
