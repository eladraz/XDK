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
 * device.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/refCountObject.h"
#include "loader/device.h"
#include "loader/loader.h"

REFCOUNTABLE_CPP(cDevice);

cDevice::cDevice(const cString& deviceName,
                 const cString& deviceFilename) :
    REFCOUNTABLE_INIT,
    m_deviceName(deviceName),
    m_deviceFilename(deviceFilename),
    m_command(NULL)
{
    cOSDef::deviceHandle handle = cDeviceLoader::loadDevice(deviceName,
                                                            deviceFilename);
    m_command = new cCommand(handle);
    init(handle);
}

cCommand& cDevice::getCommand()
{
    ASSERT(m_command != NULL);
    return *m_command;
}

cDevice::~cDevice()
{
    if (free()) {
        // Delete the command-center
        delete m_command;
        // Unload the device.
        cDeviceLoader::unloadDevice(m_deviceName,
            m_deviceFilename,
            getHandle());
    }
}

void cDevice::copy(const cDevice& other)
{
    m_deviceFilename = other.m_deviceFilename;
    m_deviceName = other.m_deviceName;
    m_command = other.m_command;
}

cOSDef::deviceHandle cDevice::getHandle() const
{
    return getObject();
}
