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
 * PcSpeakerServer.cpp
 *
 * RPC implementation file for interface PcSpeaker using
 * device-driver I/O controls.
 *
 * The device is responsible for: Basic speaker device class. Handles the
 * features of the PC-Speaker.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/endian.h"
#include "xStl/except/assert.h"
#include "XDK/utils/IoctlListener.h"
#include "XDK/utils/IoctlDispatcher.h"
#include "common/PcSpeakerIoctl.h"
#include "driver/PcSpeakerServer.h"

cPcSpeakerServer::cPcSpeakerServer(cIoctlDispatcher& dispatcher)
{
    // Register all known IOCTLs

    dispatcher.registerIoctlHandler(
        IOCTL_PLAYER_SOUNDON,
        IOCTL_INSTANCE(handlesoundOnIoctl));
    dispatcher.registerIoctlHandler(
        IOCTL_PLAYER_SOUNDOFF,
        IOCTL_INSTANCE(handlesoundOffIoctl));
    dispatcher.registerIoctlHandler(
        IOCTL_PLAYER_SETFREQUENCY,
        IOCTL_INSTANCE(handlesetFrequencyIoctl));
    dispatcher.registerIoctlHandler(
        IOCTL_PLAYER_MAKEIMPULSE,
        IOCTL_INSTANCE(handlemakeImpulseIoctl));
}

uint cPcSpeakerServer::handlesoundOnIoctl(
    uint         ioctlCode,
    const uint8* inputBuffer,
    uint         inputBufferLength,
    uint8*       outputBuffer,
    uint         outputBufferLength)
{
    // Make sure that there aren't any manualy errors
    ASSERT(ioctlCode == IOCTL_PLAYER_SOUNDON);

    // Parse arguments and call handler function
    soundOn();
    uint outputBufferUsed = 0;

    // Return the number of bytes written
    return outputBufferUsed;
}

uint cPcSpeakerServer::handlesoundOffIoctl(
    uint         ioctlCode,
    const uint8* inputBuffer,
    uint         inputBufferLength,
    uint8*       outputBuffer,
    uint         outputBufferLength)
{
    // Make sure that there aren't any manualy errors
    ASSERT(ioctlCode == IOCTL_PLAYER_SOUNDOFF);

    // Parse arguments and call handler function
    soundOff();
    uint outputBufferUsed = 0;

    // Return the number of bytes written
    return outputBufferUsed;
}

uint cPcSpeakerServer::handlesetFrequencyIoctl(
    uint         ioctlCode,
    const uint8* inputBuffer,
    uint         inputBufferLength,
    uint8*       outputBuffer,
    uint         outputBufferLength)
{
    // Make sure that there aren't any manualy errors
    ASSERT(ioctlCode == IOCTL_PLAYER_SETFREQUENCY);

    // Parse arguments and call handler function
    uint32 frequency;
    frequency = (uint32)cLittleEndian::readUint32(inputBuffer + 0);
    setFrequency(frequency);
    uint outputBufferUsed = 0;

    // Return the number of bytes written
    return outputBufferUsed;
}

uint cPcSpeakerServer::handlemakeImpulseIoctl(
    uint         ioctlCode,
    const uint8* inputBuffer,
    uint         inputBufferLength,
    uint8*       outputBuffer,
    uint         outputBufferLength)
{
    // Make sure that there aren't any manualy errors
    ASSERT(ioctlCode == IOCTL_PLAYER_MAKEIMPULSE);

    // Parse arguments and call handler function
    uint32 counter;
    counter = (uint32)cLittleEndian::readUint32(inputBuffer + 0);
    makeImpulse(counter);
    uint outputBufferUsed = 0;

    // Return the number of bytes written
    return outputBufferUsed;
}

