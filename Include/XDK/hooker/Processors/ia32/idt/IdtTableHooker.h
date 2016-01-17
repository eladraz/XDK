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

#ifndef __TBA_XDK_HOOKER_IA32_IDT_IDTTABLEHOOKER_H
#define __TBA_XDK_HOOKER_IA32_IDT_IDTTABLEHOOKER_H

/*
 * IdtTableHooker.h
 *
 * API for hooking all entries for the current Interrupt Descriptor Table.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
//#include "xStl/os/mutex.h"
#include "xdk/hooker/ProcessorsThread.h"
#include "xdk/hooker/Processors/ia32/idt/hookIdt.h"

/*
 * For each processor and for each entry in the table, invoke a call to HookIdt
 * class.
 * See HookIdt for more information.
 *
 * NOTE: This class must destruct before the ProcessorThreadManager dtor is
 *       called.
 *       See ProcessorThreadManager for more information.
 *       See IdtHookerManager for more information.
 */
class IdtTableHooker : public ProcessorJob {
public:
    // IDT table is limited per 256 interrupts
    enum { MAX_VECTOR_COUNT = 0x100 };

    /*
     * Constructor. Performs the hooking over the table.
     *
     * callbackClass - The called function for the IDT operation. The function
     *                 doesn't use const& because the constructor can be used
     *                 within a global thread and a global listeners.
     * startRange - The starting vector number
     * endRange - The end vector number
     *
     * Throw exception if the range [start..end] is invalid.
     */
    IdtTableHooker(InterruptListenerPtr callbackClass,
                   uint startRange = 0,
                   uint endRange = (MAX_VECTOR_COUNT - 1));

    /*
     * Destructor. Free the hooking.
     * NOTE: This destructor must be called in a PASSIVE_LEVEL context-switch
     *       free enviroment
     */
    ~IdtTableHooker();

    /*
     * Return true if interrupt number 'vector' is hooked.
     */
    bool isInterruptHooked(uint vector) const;

protected:
    /*
     * See ProcessorJob::run
     *
     * Start hooking the IDT table/stop hooking the IDT table. For each
     * processor in the CPU the IDT is being hooked.
     * See m_hooking, m_hookReason
     */
    virtual void run(const uint processorID,
                     const uint numberOfProcessors);

private:
    /*
     * Return a reference inside the m_hooking table for an IDT enrty
     *
     * vector      - The index of the interrupt (from 0 to 255)
     * processorID - The ID of the processor (from 0..PCR-1)
     */
    HookIdtPtr& getEntry(uint vector, uint processorID) const;

    // All the hooking interrupts
    mutable cArray<HookIdtPtr> m_hooking;
    // The number of processors
    uint m_numberOfProcessors;
    // The table size for a processor
    uint m_tableSize;
    // Set to true to indicate that the constructor was call and a hook
    // operation must be performed, false to indicate that the constructor was
    // called and unhook operation is required.
    volatile bool m_jobReasonIsHook;
    // The number of threads working on a job
    cCounter m_processorTaskNumber;
    // The start range to be use
    uint m_startRange;
    // The end range to be use
    uint m_endRange;
    // The callback call
    InterruptListenerPtr m_tempCallbackClass;

    // The lock which protectes the table hooking processs
    //cMutex m_hookLock;
};

// Pointer to the reference object counter
typedef cSmartPtr<IdtTableHooker> IdtTableHookerPtr;

#endif // __TBA_XDK_HOOKER_IA32_IDT_IDTTABLEHOOKER_H
