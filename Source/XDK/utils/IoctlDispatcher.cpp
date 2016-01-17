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
 * IoctlDispatcher.cpp
 *
 * The dispatcher module.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/hash.h"
#include "xStl/data/datastream.h"
#include "xStl/except/exception.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "XDK/kernel.h"
#include "xdk/ehlib/ehlib.h"
#include "XDK/utils/IoctlListener.h"
#include "XDK/utils/IoctlDispatcher.h"

// Extract transfer type from IOCTL codes.
#define IOCTL_TRANSFER_TYPE(ioctl) (ioctl & 0x3)

cIoctlDispatcher::cIoctlDispatcher()
{
}

void cIoctlDispatcher::registerIoctlHandler(uint ioctlCode,
                                            cIoctlListenerPtr ioctlListener)
{
    // Check to see whether the IOCTL command isn't occoupy.
    if (m_handlers.hasKey(ioctlCode))
    {
        traceHigh("IoctlDispatcher: registerIoctlHandler there is already IOCTL handler for code " <<
                  HEXDWORD(ioctlCode) << endl);
        XSTL_THROW(cException, EXCEPTION_OUT_OF_RANGE);
    }

    // Register the function
    m_handlers.append(ioctlCode, ioctlListener);
}


uint cIoctlDispatcher::handleDeviceDispatcher(uint ioctlCode,
                                      const uint8* inputBuffer,
                                      uint         inputBufferLength,
                                      uint8*       outputBuffer,
                                      uint         outputBufferLength)
{
    // traceLow("IoctlDispatcher: handleDeviceDispatcher " <<
    //         HEXDWORD(ioctlCode) << " buffer " << HEXDWORD((uint32)inputBuffer) << endl);

    // Dispatch the code. Exception will be thrown if the code is not mapped.
    return (m_handlers[ioctlCode])->handleIoctl(ioctlCode,
        inputBuffer,
        inputBufferLength,
        outputBuffer,
        outputBufferLength);
}

NTSTATUS cIoctlDispatcher::handleIrp(PIRP irp)
{
    // Gets the stack location
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(irp);
    // Tests the IRPmajor function
    CHECK((irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) ||
          (irpStack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL));

    // Extract the known values
    uint ioctlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    uint inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    uint outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    const uint8* inputBuffer = (uint8*)irp->AssociatedIrp.SystemBuffer;
    uint8* outputBuffer = (uint8*)irp->AssociatedIrp.SystemBuffer;


    if (IOCTL_TRANSFER_TYPE(ioctlCode) == METHOD_NEITHER)
    {
        outputBuffer = (uint8*)irp->UserBuffer;
    }


    // For directed IRP map the user-mode pages into the kernel
    if (((irp->Flags & IRP_BUFFERED_IO) == 0) &&
        ((IOCTL_TRANSFER_TYPE(ioctlCode) == METHOD_IN_DIRECT) ||
         (IOCTL_TRANSFER_TYPE(ioctlCode) == METHOD_OUT_DIRECT)))
    {
        traceLow("IoctlDispatcher: Map MDL to kernel space..." << endl);

        /*
         * Map the MDL request to non-paged pool pages. Since this driver works
         * over NT we didn't call the "MmGetSystemAddressForMdlSafe" function.
         *
         * Notice that the IO-manager doesn't use the Parameters block at all.
         */
        ULONG size = MmGetMdlByteCount(irp->MdlAddress);
        PVOID addr = ((uint8*)MmGetSystemAddressForMdl(irp->MdlAddress)) +
                              MmGetMdlByteOffset(irp->MdlAddress);

        if (IOCTL_TRANSFER_TYPE(ioctlCode) == METHOD_IN_DIRECT)
        {
            inputBuffer = (uint8*)addr;
            inputBufferLength = size;
        } else
        {
            outputBuffer = (uint8*)addr;
            outputBufferLength = size;
        }
    }

    // Fails the return code
    irp->IoStatus.Information = 0;

    // Execute the IRP and complete it.
    XSTL_TRY
    {
        irp->IoStatus.Information = handleDeviceDispatcher(ioctlCode,
                                                    inputBuffer,
                                                    inputBufferLength,
                                                    outputBuffer,
                                                    outputBufferLength);
        return STATUS_SUCCESS;
    }
    XSTL_CATCH_ALL
    {
        traceHigh("IoctlDispatcher: IRP handler throws exception: " <<
                  EHLib::getUnknownException() << endl);

        // Exception had being caught, failed the request
        return STATUS_INVALID_DEVICE_REQUEST;
    }
}
