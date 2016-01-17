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

#ifndef __TBA_XDK_IOCTLDISPATCHER_DRIVER_H
#define __TBA_XDK_IOCTLDISPATCHER_DRIVER_H

/*
 * IoctlDispatcher.h
 *
 * Handles IOCTL commands from ring3 application into a device object.
 * The module route the IOCTLs to different registered devices.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/hash.h"
#include "XDK/kernel.h"
#include "XDK/utils/IoctlListener.h"

/*
 * class cIoctlDispatcher
 *
 * The singleton class which handles the IOCTL command and dispatching.
 * The class implements as array with maximum IOCTL dispatchers.
 * See cIoctlListener.
 */
class cIoctlDispatcher
{
public:
    // Constructor
    cIoctlDispatcher();

    /*
     * Register a new ioctl handler. Whenever 'ioctlCode' will be called in the
     * dispatcher the 'ioctlFunction' will be called.
     *
     *  ioctlCode     - The CTL_CODE to be registered.
     *  ioctlListener - The handler for the function.
     *
     *
     * Throws exception if the ioctlCode is already exist or if there are memory
     * errors
     */
    void registerIoctlHandler(uint ioctlCode,
                              cIoctlListenerPtr ioctlListener);

    /*
     * Handle the IOCTL command which sends to the driver. Dispatch the
     * ioctlCode to the registered function. If the ioctlCode is not
     * registered or some-error occuered the function returns false.
     *
     *  ioctlCode           - The IOCTL command
     *  inputBuffer         - The buffer from the user
     *  inputBufferLength   - The length of the inputBuffer in bytes
     *  outputBuffer        - The buffer which the result should be written to
     *                        NOTICE: This buffer can be overlapped with the
     *                                inputBuffer
     *  outputBufferLength  - The size in bytes of the 'outputBuffer'
     *
     * Return the number of bytes written to the output buffer
     *
     * May throws exceptions when the handling is failed, or the ioctlCode
     * is not registered.
     */
    uint handleDeviceDispatcher(uint ioctlCode,
                        const uint8* inputBuffer,
                        uint         inputBufferLength,
                        uint8*       outputBuffer,
                        uint         outputBufferLength);

    /*
     * Handles IRP_MJ_DEVICE_CONTROL or IRP_MJ_INTERNAL_DEVICE_CONTROL io
     * request packet. The operation system calling convention is transfrom
     * from MDL, System buffer or neidther into data which transfer to the
     * different handlers.
     *
     * irp - The IRP from the user.
     *
     * NOTE: The function doesn't complete the IRP.
     *
     * Throws exception if the IRP major code is not one of the two dispatcher
     * codes.
     */
    NTSTATUS handleIrp(PIRP irp);

private:
    /*
     * Disable copy-constructor and operator =.
     */
    cIoctlDispatcher(const cIoctlDispatcher& other);
    cIoctlDispatcher& operator = (const cIoctlDispatcher& other);

    // A map between ioctlCode and it's handler.
    cHash<uint, cIoctlListenerPtr> m_handlers;
};


#endif // __TBA_XDK_IOCTLDISPATCHER_DRIVER_H

