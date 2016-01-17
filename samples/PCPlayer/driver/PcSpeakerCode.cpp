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
 * PcSpeakerCode.cpp
 *
 * Custom server side implementation for the interface.
 * Don't forget to link this class to the device IOCTL handler code.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/traceStream.h"
#include "driver/driverConsts.h"
#include "driver/PcSpeakerServer.h"


void cPcSpeakerServer::soundOn()
{
    traceMedium("PcSpeakerServer: soundOn" << endl);

    // Activate PcSpekaer gate
    UCHAR speaker = READ_PORT_UCHAR(SPEAKER_PORT);
    WRITE_PORT_UCHAR(SPEAKER_PORT, speaker | SPEAKER_SIGNAL_BIT);
}

void cPcSpeakerServer::soundOff()
{
    traceMedium("PcSpeakerServer: soundOff" << endl);

    // Turn off the PcSpekaer gate
    UCHAR speaker = READ_PORT_UCHAR(SPEAKER_PORT);
    WRITE_PORT_UCHAR(SPEAKER_PORT, speaker & (~SPEAKER_SIGNAL_BIT));
}

void cPcSpeakerServer::setFrequency(uint32 frequency)
{
    traceMedium("PcSpeakerServer: setFrequency " << frequency << "Hz" << endl);

    // Calculate the timer step over...
    if (frequency == 0)
    {
        traceHigh("PcSpeakerServer: setFrequency invalid frequency" << endl);
        return;
    }

    // Calculate new frequency
    uint32 stepOverCount = TIMER_FREQUENCY / frequency;

    // Change the frequency.
    WRITE_PORT_UCHAR(TIMER_PORT,   TIMER_SPEAKER_COUNTER_S3);
    WRITE_PORT_UCHAR(TIMER_DATA_2, (UCHAR)((stepOverCount >> 0) & 0xFF)); // LSB first
    WRITE_PORT_UCHAR(TIMER_DATA_2, (UCHAR)((stepOverCount >> 8) & 0xFF));
}

void cPcSpeakerServer::makeImpulse(uint32 counter)
{
    // TODO: To decrease debug-print this line is in comment.
    // traceMedium("PcSpeakerServer: makeImpulse " << counter << endl);

    // Generate the impulse
    WRITE_PORT_UCHAR(TIMER_PORT,   TIMER_SPEAKER_COUNTER_S0);
    WRITE_PORT_UCHAR(TIMER_DATA_2, (UCHAR)(counter & 0xFF));        // LSB first
    WRITE_PORT_UCHAR(TIMER_DATA_2, (UCHAR)((counter >> 8) & 0xFF));
}

