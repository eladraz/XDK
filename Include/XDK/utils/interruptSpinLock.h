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

#ifndef __TBA_XDK_UTILS_INTERRUPTSPINLOCK_H
#define __TBA_XDK_UTILS_INTERRUPTSPINLOCK_H

/*
 * interruptSpinLock.h
 *
 * Busy-wait spin-lock multi-processor safe which is able to work during
 * interrupt time.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lockable.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/processorLock.h"

/*
 * Busy-wait spin-lock multi-processor safe which is able to work during
 * interrupt time.
 *
 * The lockable object uses the cProcessorLock to lock the current processor
 * and uses a single number to acquire/release the spin-lock. When the acquire
 * failed it will try again, until acquire.
 */
class cInterruptSpinLock : public cLockableObject {
public:
    /*
     * Default constructor
     */
    cInterruptSpinLock();

    /*
     * Destructor. Free all resources
     */
    virtual ~cInterruptSpinLock();

    /*
     * See cLockable::lock()
     */
    virtual void lock();

    /*
     * See cLockable::unlock()
     */
    virtual void unlock();

    /*
     * The function tries to lock the spin-lock. If the spin-lock is already
     * acquire, the function return false.
     *
     * Return true if the spin-lock acquire.
     * Return false if the spin-lock was already locked.
     */
    bool tryLock();

private:
    // Set to 1 to indicate that the mutex is acquire, 0 otherwise
    volatile uint m_isLocked;

    // Array of all processors lockable object
    cProcessorLock m_lock[cProcessorUtil::MAX_PROCESSORS_SUPPORT];
    // Array of booleans, set to true to indicate that the processor
    // lock was acquire
    volatile bool m_processorLockAcquire[cProcessorUtil::MAX_PROCESSORS_SUPPORT];

    #ifdef _DEBUG

    /*
     * A simple struct which contains memory reference for 4 functions call.
     */
    struct SimpleStackTrace
    {
        addressNumericValue m_addr1;
        addressNumericValue m_addr2;
        addressNumericValue m_addr3;
        addressNumericValue m_addr4;
        addressNumericValue m_addr5;
        addressNumericValue m_addr6;
    };

    /*
     * Return the last 4 function in the stack which cause this chain of
     * events.
     *
     * addr? - Will be filled with the function address.
     */
    void getStackTrace(SimpleStackTrace& stackTrace);

    // Saves the last known stack trace
    SimpleStackTrace m_lastStack;
    #endif
};

#endif // __TBA_XDK_UTILS_INTERRUPTSPINLOCK_H
