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

#ifndef __TBA_PCPLAYER_PCSPEAKERDRIVER_H
#define __TBA_PCPLAYER_PCSPEAKERDRIVER_H

/*
 * PcSpeakerDriver.h
 *
 * The main driver object for
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/kernel.h"
#include "XDK/driver.h"
#include "driver/PcSpeakerDriver.h"

/*
 * The driver contains implementation for the several interfaces:
 *   PcSpeaker  - Control the timer chip and the PC-Speaker port.
 *   PcPlayer   - The sound-blaster PCM player interface.
 */
class PcSpeakerDriver : public cDriver {
public:
    // Default constructor
    PcSpeakerDriver();

    /*
     * The driver's "main" function. Creates the different devices and
     * connects the interfaces to them.
     *
     * See cDriver::driverEntry() for more information
     */
    virtual NTSTATUS driverEntry();

private:
    // The main device-driver
    cDevicePtr m_speakerDevice;
};

#endif // __TBA_PCPLAYER_PCSPEAKERDRIVER_H

