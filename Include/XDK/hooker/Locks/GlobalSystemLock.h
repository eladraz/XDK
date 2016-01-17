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

#ifndef __TBA_XDK_HOOKER_LOCKS_GLOBALSYSTEMLOCK_H
#define __TBA_XDK_HOOKER_LOCKS_GLOBALSYSTEMLOCK_H

/*
 * GlobalSystemLock.h
 *
 * Locks the entire operating system. Hooking over multi-processors, threads
 * or any other high-priority, non-preemtive task should be locked by this
 * class.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lockable.h"

// The kernel-mode uses the Win386Lock
#ifdef XSTL_NTDDK
    #include "xdk/utils/processorLock.h"
#endif

/*
 * Locks the entire operating system. Hooking over multi-processors, threads
 * or any other high-priority, non-preemtive task should be locked by this
 * class.
 *
 * TODO: For now this class applies only for a single processor.
 *       Add complete processors locks
 */
class GlobalSystemLock : public cLockableObject
{
public:
    /*
     * See cLockable::lock()
     */
    virtual void lock();

    /*
     * See cLockable::unlock()
     */
    virtual void unlock();

private:
    #ifdef _KERNEL
        // The lockable object
        cProcessorLock m_lock;
    #endif
};

#endif // __TBA_XDK_HOOKER_LOCKS_GLOBALSYSTEMLOCK_H
