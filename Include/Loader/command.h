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

#ifndef __TBA_XDK_LOADER_COMMAND_H
#define __TBA_XDK_LOADER_COMMAND_H

/*
 * command.h
 *
 * Invoke commands into the device-driver from ring3 applications.
 * The commands are IOCTLs (I/O controls) which has the following parts:
 *  - Destination device, the device for sending the commands to
 *  - Command number, index number of the command
 *  - Sending memory, block of memory which contains the information that the
 *    device-driver will be recieved.
 *  - Recieving memory, block of memory which the driver can write it's
 *    information to. The driver also return the number of bytes written
 *    to that memory area.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/osdef.h"

class cCommand {
public:
    /*
     * Constructor. Inits the command module for a device object.
     *
     * handle - Handle to the device-driver. See cDeviceLoader.
     */
    cCommand(cOSDef::deviceHandle handle);

    /*
     * Sends a command to the device.
     *
     * code                            - The code of the command.
     * inputBuffer,inputBufferLength   - The buffer which transfer to the driver
     * outputBuffer,outputBufferLength - The buffer which will be filled with
     *                                   more information from the driver side.
     *
     * Return the number of bytes written to the output buffer
     *
     * Throws exception if the device failed to process the message.
     */
    uint invoke(uint code,
        const uint8* inputBuffer,
        uint         inputBufferLength,
        uint8*       outputBuffer,
        uint         outputBufferLength);

private:
    // Handle for the device-object
    cOSDef::deviceHandle m_handle;
};

#endif // __TBA_XDK_LOADER_COMMAND_H

