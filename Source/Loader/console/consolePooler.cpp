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
 * consolePooler.cpp
 *
 * Implementation file for NT operation system.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/data/string.h"
#include "xStl/os/osdef.h"
#include "XDK/utils/consoleDeviceIoctl.h"
#include "loader/command.h"
#include "loader/loader.h"
#include "loader/console/consolePooler.h"

cConsolePooler::cConsolePooler(const cString& deviceName,
                               const cString& deviceFilename) :
    m_deviceName(deviceName),
    m_deviceFilename(deviceFilename),
    m_consoleDevice(NULL),
    m_isPendingCalled(false),
    m_cacheStringSize(0),
    m_command(NULL)
{
    // Open the device. An exception can be raised here
    m_consoleDevice = cDeviceLoader::loadDevice(m_deviceName,
                                                m_deviceFilename);

    XSTL_TRY
    {
        // Setup command-center
        m_command = new cCommand(m_consoleDevice);

        // Test the version of the device. An exception can be thrown here also
        if (getVersion() != CONSOLE_VERSION_1)
        {
            TRACE(TRACE_VERY_HIGH, "ConsolePooler: Version isn't matches!");
            XSTL_THROW(cException, EXCEPTION_FAILED);
        }
    }
    XSTL_CATCH_ALL
    {
        // Free the opened device
        cDeviceLoader::unloadDevice(m_deviceName,
                                    m_deviceFilename,
                                    m_consoleDevice);
        // Rethrow exception...
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }
}

cConsolePooler::~cConsolePooler()
{
    // Free the command-center
    delete m_command;

    // Free the device
    cDeviceLoader::unloadDevice(m_deviceName,
                                m_deviceFilename,
                                m_consoleDevice);
}

bool cConsolePooler::isStringPending()
{
    // Query the
    m_cacheStringSize = getNextLineSize();
    if (m_cacheStringSize == 0)
    {
        // There aren't any lines waiting
        return false;
    }
    m_isPendingCalled = true;
    return true;
}

cString cConsolePooler::poolString()
{
    if (!m_isPendingCalled)
    {
        CHECK(isStringPending());
    }

    cBuffer retString(m_cacheStringSize);
    CHECK(poolLine(retString.getBuffer(), m_cacheStringSize));

    return cString((character*)retString.getBuffer());
}

uint cConsolePooler::getVersion()
{
    uint32 version;

    // Execute
    CHECK(m_command->invoke(IOCTL_CONSOLE_GETVERSION,
                        NULL, 0,
                        (uint8*)&version, sizeof(version)) == sizeof(version));

    return version;
}

uint cConsolePooler::getNextLineSize()
{
    uint32 size;

    // Execute
    CHECK(m_command->invoke(IOCTL_CONSOLE_GETNEXTLINE_SIZE,
                        NULL, 0,
                        (uint8*)&size, sizeof(size)) == sizeof(size));

    return size;
}

bool cConsolePooler::poolLine(uint8* outputLine, uint outputLineLength)
{
    // Execute
    CHECK(m_command->invoke(IOCTL_CONSOLE_POOL_LINE,
                        NULL, 0,
                        outputLine, outputLineLength) == outputLineLength);

    return true;
}

