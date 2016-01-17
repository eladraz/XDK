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

#ifndef __TBA_PCPLAYER_DRIVERCONSTS_H
#define __TBA_PCPLAYER_DRIVERCONSTS_H

/*
 * driverConsts.h
 *
 * All the consts needed for the driver.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/char.h"

/*
 * Contains all data needed for proper operation of the pc-speaker
 */
class DriverConsts {
public:
};


/*
* Timer8254Defines.h
*
* Definition of the system ports.
*/

#define TIMER_PORT          (PUCHAR)(0x43) // Timer - 8254 I/O Address
#define TIMER_DATA_0        (PUCHAR)(0x40) // Timer channel 0 (IRQ0) counter data register.
#define TIMER_DATA_2        (PUCHAR)(0x42) // Timer channel 2 counter data register.
#define SPEAKER_PORT        (PUCHAR)(0x61) // Speaker IO control register
#define SPEAKER_SIGNAL_BIT          (0x03) // Speaker signal bit.
#define TIMER_SPEAKER_COUNTER_S3    (0xB6) // Binary counter, singal mode 3 (quarte signal), LSB first, channel 2.
#define TIMER_SPEAKER_COUNTER_S0    (0xB0) // Binary counter, singal mode 0 (impolse), LSB first, channel 2.
#define TIMER_FREQUENCY          (1193181) // The frequency of the 8254 chip: 1.193181mHz

/*
* Analog to impulse setting
*/
#define SIGNAL_AMPLITUDE (1)     // Increase the signal
#define TIMER_CTCCLK (14318180.0) // clock frequency for 8254 in internal mesuare
#define TIMER_HZCCLK (12.0)       // The clock in Hz.


#endif // __TBA_PCPLAYER_DRIVERCONSTS_H
