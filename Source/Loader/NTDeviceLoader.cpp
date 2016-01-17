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
 * NTDeviceLoader.cpp
 *
 * Implementation file for NT operation system.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/os/os.h"
#include "loader/loader.h"
#include "loader/deviceException.h"
#include "loader/NTDeviceLoader.h"

cNTServiceControlManager::cNTServiceControlManager() :
    m_scmHandle(NULL)
{
    // Open the Service Control Manager for all permissions
    m_scmHandle = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (m_scmHandle == NULL)
    {
        // Access deined.
        XSTL_THROW(cDeviceException, (cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED));
    }
}

cNTServiceControlManager::~cNTServiceControlManager()
{
    if (m_scmHandle != NULL)
    {
        ::CloseServiceHandle(m_scmHandle);
    }
}

bool cNTServiceControlManager::registerDevice(const cString& driverName,
                                              const cString& displayName,
                                              const cString& pathName,
                                              bool bootTime)
{
    ASSERT(driverName.length() < 256);
    ASSERT(displayName.length() < 256);

    SC_HANDLE serviceHandle;

    serviceHandle = ::CreateService(m_scmHandle,
        driverName.getBuffer(),                               // name of service
        displayName.getBuffer(),                              // name to display
        SERVICE_ALL_ACCESS,                                   // desired access
        SERVICE_KERNEL_DRIVER,                                // service type
        bootTime ? SERVICE_BOOT_START : SERVICE_DEMAND_START, // start type
        SERVICE_ERROR_NORMAL,                                 // error control type
        pathName.getBuffer(),                                 // service's binary
        NULL,                                                 // no load ordering group
        NULL,                                                 // no tag identifier
        NULL,                                                 // no dependencies
        NULL,                                                 // LocalSystem account
        NULL);                                                // no password

    if (serviceHandle == NULL)
    {
        DWORD lastError = ::GetLastError();
        if (lastError != ERROR_SERVICE_EXISTS)
        {
            // Access deined.
            XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
        } else
        {
            return false;
        }
    }

    ::CloseServiceHandle(serviceHandle);
    return true;
}

void cNTServiceControlManager::unregisterDevice(const cString& driverName)
{
    cNTService service(this, driverName);

    service.deleteService();
}

cSmartPtr<cNTServiceControlManager::cNTService>
    cNTServiceControlManager::openDevice(const cString& driverName)
{
    return cSmartPtr<cNTServiceControlManager::cNTService>(new cNTService(this, driverName));
}

SC_HANDLE cNTServiceControlManager::getSCMhandle() const
{
    return m_scmHandle;
}

cNTServiceControlManager::cNTService::cNTService(
                cNTServiceControlManager* scmManager,
                const cString& serviceName) :
    m_scmManager(scmManager),
    m_serviceHandle(NULL)
{
    ASSERT(m_scmManager != NULL);

    // Open the service
    m_serviceHandle = ::OpenService(m_scmManager->getSCMhandle(),
        serviceName.getBuffer(),
        SERVICE_ALL_ACCESS);

    if (m_serviceHandle == NULL)
    {
        // Access deined.
        XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
    }
}

cNTServiceControlManager::cNTService::~cNTService()
{
    if (m_serviceHandle != NULL)
    {
        ::CloseServiceHandle(m_serviceHandle);
    }
}

void cNTServiceControlManager::cNTService::startService()
{
    BOOL ret = ::StartService(m_serviceHandle, 0, NULL);

    if (!ret)
    {
        // Test failure reason
        DWORD lastError = ::GetLastError();
        if (lastError != ERROR_SERVICE_ALREADY_RUNNING)
        {
            // The service couldn't be started
            XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
        }
    }
}

void cNTServiceControlManager::cNTService::stopService()
{
    SERVICE_STATUS serviceStatus;
    BOOL ret = ::ControlService(m_serviceHandle,
        SERVICE_CONTROL_STOP,
        &serviceStatus);

    if (!ret)
    {
		// Test failure reason
		DWORD lastError = ::GetLastError();
		if (lastError != ERROR_SERVICE_NOT_ACTIVE)
		{
			// The service couldn't be stopped.
			XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
		}
    }
}

void cNTServiceControlManager::cNTService::deleteService()
{
    BOOL ret = ::DeleteService(m_serviceHandle);

    if (!ret)
    {
        // The service couldn't be deleted.
        XSTL_THROW(cDeviceException, cOS::getLastErrorString().getBuffer(), EXCEPTION_FAILED);
    }
}
