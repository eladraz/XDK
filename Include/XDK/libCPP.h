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

#ifndef __TBA_XDK_LIBCPP_H
#define __TBA_XDK_LIBCPP_H

/*
 * libCPP.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/kernel.h"
#include "XDK/driver.h"

/*
 * The main entry point of the driver.
 * Calls to cXDKLibCPP::driverEntry.
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject,
                     PUNICODE_STRING registryPath);

/*
 * The namespace for the XDK lib-CPP functionallity.
 *
 * Note: This class must have any context because constructors and destructors
 *       will not be called.
 */
class cXDKLibCPP
{
private:
    friend NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject,
                                PUNICODE_STRING registryPath);

    /*
     * The main entry point. Loads the driver into the memory.
     * Safe initialize the XDK library and start the cDriver singleton.
     *
     * driverObject - pointer to the driver object
     * registryPath - pointer to a unicode string representing the path,
     *                to driver-specific key in the registry.
     *
     * Return the error code
     */
    static NTSTATUS driverEntry(PDRIVER_OBJECT driverObject,
                                PUNICODE_STRING registryPath);

    /*
     * Called when the driver unloads itself.
     * Call to lib C++ unload().
     *
     * driverObject - pointer to the driver object
     */
    static void cXDKLibCPP::driverUnload(PDRIVER_OBJECT driverObject);

    /*
     * Initialize the KDK driver. Perform the following start-up code:
     *   - Inits the exception-handling library
     *   - Inits the 'atexit' database.
     *   - Call to global constructor
     *
     * Return whether the initialzation completed successfully.
     */
    static bool initialize();

    /*
     * Unloads the driver. In order to unload the device-driver all the
     * functionallty should be disabled.
     * The functions performs the following steps:
     *   - TODO! Stops all new IRP by sending a unsuccessful completion.
     *   - TODO! Waits until all IRP's completed
     *   - Calls to all registered destructors of singletons and global
     *     objects.
     *   - Dismount the 'atexit' library
     *   - Dismount the exception-handling library
     */
    static void unload();
};

#endif // __TBA_XDK_LIBCPP_H
