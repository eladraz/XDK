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
 * KTMem.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/ioStream.h"
#include "xStl/stream/traceStream.h"
#include "XDK/kernel.h"
#include "XDK/driver.h"
#include "XDK/device.h"
#include "XDK/driverFactory.h"
#include "XDK/ehlib/ehlib.h"
#include "XDK/utils/consoleDevice.h"
#include "../testMemoryManager/TestSuperBlock.h"
#include "KTMem.h"
#include "DefaultMem.h"

const character KTMem::m_consoleDeviceName[] =
    XSTL_STRING("KTMem");

KTMem::KTMem() :
    m_consoleDevice(NULL)
{
}

NTSTATUS KTMem::driverEntry()
{
    // Leave this lines if you wishes to
    // use the console device...
    // Construct the console device
    m_consoleDevice = cDevicePtr(new cConsoleDevice(m_consoleDeviceName));
    // Add the device to the queue
    addDevice(m_consoleDevice);

    DefaultMem mem;
    TestSuperBlock test(mem, cout, 1, 1024);
    uint8* temp[100];
    // Allocate and free from interrupt time
    uint i;
    for (i = 0; i < 100; i++)
        temp[i] = new uint8[i + 50];

    KIRQL oldIrql;
    KeRaiseIrql(HIGH_LEVEL, &oldIrql);
    // Before test
    for (i = 0; i < 50; i++)
        delete[] temp[i];
    test.test();
    // After test
    for (i = 50; i < 100; i++)
        delete[] temp[i];
    KeLowerIrql(oldIrql);

    // TODO! Add your handler here
    return STATUS_SUCCESS;
}

// The single global object...
cDriverFactory<KTMem> theDriver;
