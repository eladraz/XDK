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

#ifndef __PCSPEAKER_SERVER_H
#define __PCSPEAKER_SERVER_H

/*
 * PcSpeakerServer.h
 *
 * Stub code which marshal the requests from ring0 device I/O request
 * application to a normal function call.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/kernel.h"
#include "XDK/utils/IoctlListener.h"
#include "XDK/utils/IoctlDispatcher.h"
#include "common/PcSpeakerIoctl.h"

/*
 * The server side implementation code for the PcSpeaker interface
 * which responsible for: Basic speaker device class. Handles the features of
 * the PC-Speaker.
 *
 * In order to use this class append this class to the device-IOCTL dispatcher
 * code and implement the file: PcSpeakerCode.cpp
 */
class cPcSpeakerServer : public cPcSpeakerIoctl
{
public:
    /*
     * Constructor. Register as IOCTL handler for some dispatcher.
     *
     * dispatcher - The device IOCTL handler to be registered to.
     */
    cPcSpeakerServer(cIoctlDispatcher& dispatcher);

    // The implementation code.
    // See PcSpeakerCode.cpp for the implementation code.

    // Activate the sound signal.
    virtual void soundOn();

    // Stop the sound signal.
    virtual void soundOff();

    // Change the frequency of the timer chip 8254 The timer connected
    // to the speaker and transfer a sinus wave to the speaker.
    // The frequency is from 10Hz to 119250Hz.
    //
    // frequency - The number of times when 1 puls will be heard.
    virtual void setFrequency(uint32 frequency);

    // This method change the 8254 channel 2 counter to generate a
    // single impulse for a short prioed of time.
    //
    // counter - Number of counts to pass over them. The frequency
    //           of the chip is 119250Hz. This number is the length
    //           of the out signal. The counter is limited to the
    //           low 16bit.
    virtual void makeImpulse(uint32 counter);


private:
    // Deny copy-constructor and operator =
    cPcSpeakerServer(const cPcSpeakerServer& other);
    cPcSpeakerServer& operator = (const cPcSpeakerServer& other);


    // Unpack the request for soundOn and call soundOn.
    uint handlesoundOnIoctl(uint ioctlCode,
                     const uint8* inputBuffer,
                     uint         inputBufferLength,
                     uint8*       outputBuffer,
                     uint         outputBufferLength);

    // Unpack the request for soundOff and call soundOff.
    uint handlesoundOffIoctl(uint ioctlCode,
                     const uint8* inputBuffer,
                     uint         inputBufferLength,
                     uint8*       outputBuffer,
                     uint         outputBufferLength);

    // Unpack the request for setFrequency and call setFrequency.
    uint handlesetFrequencyIoctl(uint ioctlCode,
                     const uint8* inputBuffer,
                     uint         inputBufferLength,
                     uint8*       outputBuffer,
                     uint         outputBufferLength);

    // Unpack the request for makeImpulse and call makeImpulse.
    uint handlemakeImpulseIoctl(uint ioctlCode,
                     const uint8* inputBuffer,
                     uint         inputBufferLength,
                     uint8*       outputBuffer,
                     uint         outputBufferLength);


    // The callback loops
    IOCTL_CALLBACK(cPcSpeakerServer, handlesoundOnIoctl);
    IOCTL_CALLBACK(cPcSpeakerServer, handlesoundOffIoctl);
    IOCTL_CALLBACK(cPcSpeakerServer, handlesetFrequencyIoctl);
    IOCTL_CALLBACK(cPcSpeakerServer, handlemakeImpulseIoctl);
};

#endif // __PCSPEAKER_SERVER_H

