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
 * PcSpeakerClient.cpp
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
#include "xStl/data/sarray.h"
#include "xStl/os/osdef.h"
#include "loader/command.h"
#include "common/PcSpeakerIoctl.h"
#include "client/PcSpeakerClient.h"

cPcSpeakerClient::cPcSpeakerClient(const cDevice& deviceHandler) :
    m_device(deviceHandler)
{
}

void cPcSpeakerClient::soundOn()
{
    // Marshal the function's arguments
    // Prepare the incoming buffer
    cBuffer input;
    cBuffer output;

    // Send the request
    m_device.getCommand().invoke(IOCTL_PLAYER_SOUNDON,
        input.getBuffer(),
        input.getSize(),
        output.getBuffer(),
        output.getSize());

    // Marshal the result back
    return;
}

void cPcSpeakerClient::soundOff()
{
    // Marshal the function's arguments
    // Prepare the incoming buffer
    cBuffer input;
    cBuffer output;

    // Send the request
    m_device.getCommand().invoke(IOCTL_PLAYER_SOUNDOFF,
        input.getBuffer(),
        input.getSize(),
        output.getBuffer(),
        output.getSize());

    // Marshal the result back
    return;
}

void cPcSpeakerClient::setFrequency(uint32 frequency)
{
    // Marshal the function's arguments
    // Prepare the incoming buffer
    cBuffer input(4);
    cBuffer output;

    cLittleEndian::writeUint32(input.getBuffer() + 0, (uint32)frequency);

    // Send the request
    m_device.getCommand().invoke(IOCTL_PLAYER_SETFREQUENCY,
        input.getBuffer(),
        input.getSize(),
        output.getBuffer(),
        output.getSize());

    // Marshal the result back
    return;
}

void cPcSpeakerClient::makeImpulse(uint32 counter)
{
    // Marshal the function's arguments
    // Prepare the incoming buffer
    cBuffer input(4);
    cBuffer output;

    cLittleEndian::writeUint32(input.getBuffer() + 0, (uint32)counter);

    // Send the request
    m_device.getCommand().invoke(IOCTL_PLAYER_MAKEIMPULSE,
        input.getBuffer(),
        input.getSize(),
        output.getBuffer(),
        output.getSize());

    // Marshal the result back
    return;
}

