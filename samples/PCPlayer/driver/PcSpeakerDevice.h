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

#ifndef __TBA_PCPLAYER_PCSPEAKERDEVICE_H
#define __TBA_PCPLAYER_PCSPEAKERDEVICE_H

/*
 * PcSpeakerDevice.h
 *
 * The major device-driver. Execute commands sends from ring3 in IOCTL
 * format.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartPtr.h"
#include "XDK/kernel.h"
#include "XDK/device.h"
#include "XDK/driver.h"
#include "XDK/unicodeString.h"
#include "XDK/utils/IoctlDispatcher.h"
#include "common/PcSpeakerIoctl.h"

/*
 * Creates a name device driver and attach the RPC IOCTL dispatcher to it.
 * All IOCTLs are handled and transfered to the stub codes.
 */
class PcSpeakerDevice : public cDevice {
public:
    /*
     * Constructor. Creates the device-driver. Register the RPC server code
     * to the new ioctl-dispatcher.
     *
     * name - The name for the newly created device.
     *
     * Throw exception if there was an error generating the device.
     */
    PcSpeakerDevice(const cString& name);

    /*
     * Destruct the device-driver.
     */
    virtual ~PcSpeakerDevice();

    /*
     * See cDevice::handleIrp.
     *
     * Handles the communication with the ring3 application
     */
    virtual NTSTATUS handleIrp(PIRP irp);

    /*
     * See cDevice::getDeviceObject.
     */
    virtual PDEVICE_OBJECT getDeviceObject() const;

private:
    // The handle for this device driver
    PDEVICE_OBJECT m_device;

    // The IOCTL dispatcher. Execute the different RPC commands sends from
    // ring3 and dispatch them to ring0 code.
    cIoctlDispatcher m_ioctlDispatcher;

    // The RPC-server code
    cSmartPtr<cPcSpeakerIoctl> m_rpcServer;

    // The device names
    cUnicodeString m_deviceNtName;
    cUnicodeString m_deviceSymbolicName;
};

#endif // __TBA_PCPLAYER_PCSPEAKERDEVICE_H

