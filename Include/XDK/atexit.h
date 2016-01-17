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

#ifndef __ATEXIT_LIBC_DRIVER_H
#define __ATEXIT_LIBC_DRIVER_H

/*
 * atexit.h
 *
 * Since I can't find a single discent "LIBC" for the kernel, I will have to
 * implements the "atexit" function. The atexit function should be able to
 * register functions and call them when the driver is terminated or when the
 * Unload function calls.
 *
 * The atexit implements as a list of entries.
 *
 * Author: Elad Raz <e@eladraz.com>
 */

#include "xStl/types.h"
#include "xStl/data/list.h"
#include "xdk/kernel.h"
#include "xdk/libCPP.h"
#include "xdk/utils/interruptSpinLock.h"

EXTERNC {
    // The prototype of the atexit function.
    typedef void (__cdecl* ATEXITFUNC)(void);

    /*
     * Register a new callback to be called when the driver unloaded.
     * The atexit function is passed the address of a function ("function") to
     * be called when the program terminates normally. Successive calls to
     * atexit create a register of functions that are executed in LIFO (last-in
     * -first-out) order. The functions passed to atexit cannot take parameters.
     * The register function stored in an array with limited size, MAX_ATEXIT_FUNCTIONS.
     */
    int __cdecl atexit(void (__cdecl *function)(void));
}; // extern "C"

/*
 * This class is a special singleton which
 */
class cAtExit
{
private:
    // The lib-cpp initialize this class
    friend class cXDKLibCPP;
    // This function is the main-API for the at-exit class.
    friend int __cdecl atexit(void (__cdecl *function)(void));

    /*
     * Prepare the at-exit library.
     */
    static void init();

    /*
     * Destory the instance with all registered functions.
     * Exit safely.
     */
    static void exit();



    // Unexported functions.
    // The following functions should be called internaly from the cAtExit class

    /*
     * Return the instance of the current at-exit module
     */
    static cAtExit& getInstance();

    /*
     * Call to the destructors of all registered functions.
     * Called when the XDK is unloaded from the system.
     *
     * Exception will be caught and will be traced.
     */
    void destroyObjects();

    /*
     * Safely generate a copy of the atexit list.
     *
     * list - Will be filled with the atexit object list
     */
    void copyAtexitObjects(cList<ATEXITFUNC>& list);

    /*
     * Add a function to the end of the list of destructors.
     */
    void addFunction(ATEXITFUNC newFunction);

    // Members

    /// The link-list and the interrupt-spinlock guarding it
	//// NOTE: This allows calling static objects from interrupt routines.
    cInterruptSpinLock m_atExitProtector;
    cList<ATEXITFUNC> m_atExitList;

    // The global at-exit instance
    static cAtExit* m_instance;
};

#endif // __ATEXIT_LIBC_DRIVER_H
