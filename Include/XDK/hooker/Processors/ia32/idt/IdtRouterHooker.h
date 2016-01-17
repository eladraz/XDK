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

#ifndef __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDRROUTERHOOKER_H
#define __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDRROUTERHOOKER_H

/*
 * IdtRouterHooker.h
 *
 * Route interrupt message via handlers list.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/data/list.h"
#include "xStl/data/counter.h"
#include "xStl/data/smartptr.h"
#include "xdk/utils/interruptSpinLock.h"
#include "xdk/hooker/Processors/ia32/idt/IdtTableHooker.h"
#include "xdk/hooker/Processors/ia32/idt/HookIdt.h"

/*
 * This object should be used as a singleton. Hook the interrupt table and
 * calls to handlers list.
 * Each handler can return 'true' or 'false' as a result from an interrupt.
 * When a 'false' is returned, then the router will stop forward the messages
 * and the original interrupt will not executed.
 */
class IdtRouterHooker : public InterruptListener {
public:
    /*
     * Hooks the table.
     * See IdtTableHooker
     */
    IdtRouterHooker(uint startRange,
                    uint endRange);

    /*
     * Safely remove the routing.
     */
    virtual ~IdtRouterHooker();

    /*
     * Add new interrupt handler. This function will also setup the handler to
     * notify this object from deleting. In this way, the routing process can
     * be bullet-proof from unexcpected deleteing...
     *
     * NOTE: While the new handler is appended all registers interrupts will
     *       not get executing time.
     * NOTE: This function cannot be called within an interrupt handler!
     *
     * newHandler - The new added handler
     */
    void registerNewHandler(const InterruptListenerPtr& newHandler);

    /*
     * Add new interrupt handler to the start of the handler list.
     * See registerNewHandler
     */
    void registerFirstNewHandler(const InterruptListenerPtr& newHandler);

    /*
     * Unregistering an handler from the recipients list.
     *
     * NOTE: This function cannot be called within an interrupt handler!
     *
     * object - The object to be removed.
     *
     * Throw exception if the handler couldn't be found
     */
    void unregisterHandler(InterruptListener* object);

    /*
     * Signal the router manager to stop routing interrupts to the handlers
     */
    void pauseRouting();

    /*
     * Signal he router manager to continue routing interrupts to the handlers
     */
    void resumeRouting();

    /*
     * Waits until all executed interrupts will finish executing. Used after
     * a call to pauseRouting() to make sure there aren't any executing ints.
     *
     * NOTE: This function cannot be called within an interrupt handler!
     */
    void waitUntilAllInterruptsExecuted();

protected:
    /*
     * See InterruptListener::onInterrupt
     */
    virtual bool onInterrupt(uint8 vector,
                             uint processorID,
                             uint numberOfProcessors,
                             Registers* regs,
                             InterruptFrame* interruptFrame,
                             uint32 errorCode,
                             KIRQL oldIrql);

private:
    /*
     * Allocate the list memory in a special and protected place.
     */
    void initList();

    /*
     * Free the memory of the list
     */
    void destroyList();

    /*
     * Add the newHandler into memory in a special and protected place.
     * Called by registerNewHandler.
     * See registerNewHandler
     */
    void appendListenerHandlerSafeMemory(const InterruptListenerPtr& newHandler);

    /*
     * Add the newHandler into memory, to the start of the handler list,
     * in a special and protected place.
     * Called by registerFirstNewHandler.
     * See registerFirstNewHandler
     */
    void insertFirstListenerHandlerSafeMemory(const InterruptListenerPtr& newHandler);

    // The hooker device
    IdtTableHookerPtr m_idtHooker;
    // The list of all handlers. Used as a pointer since
    cList<InterruptListenerPtr>* m_handlers;
    // The number of interrupts being served.
    cCounter m_executingCount;
    // Set to true in order to indicate that the router manager is about to
    // destruct
    volatile bool m_exitFlag;
};

#endif // __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDRROUTERHOOKER_H
