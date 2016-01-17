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

#ifndef __CONSOLE_DEVICE_IOCTLS_H
#define __CONSOLE_DEVICE_IOCTLS_H

/*
 * consoleDeviceIoctl.h
 *
 * The IOCTL codes for the Console device-driver and their parameters description
 * Note: This file is compile for both ring3 application and ring0 applications.
 *
 * See cConsoleDevice.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

// Ring3 applications include files
#ifdef XSTL_WINDOWS
#include <winioctl.h>
#endif

// Ring0 applications include files
#ifdef XSTL_NTDDK
#include "XDK/kernel.h"
#endif


/*
 * The interface for the console devices for both ring3 and ring0 applications.
 * All the methods in this class are private since only the cConsoleDevice and
 * the cConsolePooler should access this information.
 *
 * The class has the prototype for all console functions. These functions are
 * implements in completely different ways for ring0 driver and ring3 applications,
 * the ring0 driver is a server and returns a result or commit an operation for
 * request and the ring3 application only invoke the function to the driver.
 */
class cConsoleDeviceIoctl
{
public:
    /*
     * Destructor. This is an interface class.
     */
    virtual ~cConsoleDeviceIoctl() {};

    // All alone am I
    friend class cConsoleDevice;
protected:
    // Codes 0x000...0x7FF reserved for microsoft.
    // Codes 0x800...0xFFF free for private use.
    enum { BASE = 0x900 };

    // All device ioctl codes
    enum {
        /*
         * See getVersion().
         *
         * Input buffer: (NULL,0)
         * Output buffer: (uint32*, 4)
         */
        IOCTL_CONSOLE_GETVERSION =
            CTL_CODE(FILE_DEVICE_UNKNOWN, BASE + 0xB0, METHOD_BUFFERED, FILE_WRITE_ACCESS),

        /*
         * See getNextLineSize().
         *
         * Input buffer: (NULL,0)
         * Output buffer: (uint32*, 4)
         */
        IOCTL_CONSOLE_GETNEXTLINE_SIZE =
            CTL_CODE(FILE_DEVICE_UNKNOWN, BASE + 0xB1, METHOD_BUFFERED, FILE_WRITE_ACCESS),

        /*
         * See poolLine().
         *
         * Input buffer: (NULL,0)
         * Output buffer: (character*, getNextLineSize() / 2)
         */
        IOCTL_CONSOLE_POOL_LINE =
            CTL_CODE(FILE_DEVICE_UNKNOWN, BASE + 0xB2, METHOD_BUFFERED, FILE_WRITE_ACCESS),
    };

    // The different implementation of this protocol
    enum {
        CONSOLE_VERSION_1 = 0x00010000
    };

    /*
     * Returns the version of the console utility.
     */
    virtual uint getVersion() = 0;

    /*
     * Return the length in bytes for the pending output line. This number
     * includes the null-terminate character.
     *
     * 0 means that there aren'y any lines in the queue.
     */
    virtual uint getNextLineSize() = 0;

    /*
     * Filles a buffer with the queue line. The output line is in unicode
     * null-terminate format for XSTL_UNICODE compilation, and ASCII string
     * when the flag is not included.
     *
     * outputLine       - The buffer where the line will be written to.
     * outputLineLength - The size in bytes of the outputLine
     *
     * Return true if the line is polled or false if an error occuered.
     */
    virtual bool poolLine(uint8* outputLine, uint outputLineLength) = 0;
};

#endif // __CONSOLE_DEVICE_IOCTLS_H
