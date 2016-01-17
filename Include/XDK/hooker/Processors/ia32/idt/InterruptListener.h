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

#ifndef __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTLISTENER_H
#define __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTLISTENER_H

/*
 * InterrutpListener.h
 *
 * The main interrupt listener handler
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "xStl/utils/callbacker.h"
#include "xdk/hooker/Processors/ia32/registers.h"
#include "xdk/hooker/Processors/ia32/idt/idt.h"

// forward deceleration
class IdtRouterHooker;

/*
 * Interface for interrupt hooking.
 *
 * VERY VERY IMPORTANT NOTE:
 *    The execution context of the interrupt is unknown. It can be invoked
 *    from User-mode thread, kernel-mode thread, PASSIVE_LEVEL and/or higher.
 *
 *    The HookIdt have some mechanism to integrated with the operating system
 *    so some of the kernel function should be accessables, however notice
 *    carefully over the following warning:
 *
 *    DON'T USE THE NTOSKRNL DDK AT ANY WAY! Try to keep a simple and small
 *    code.
 *
 *    The interrupt code may execute the following commands:
 *     - Exception handling - All try/catch events are good
 *     - XDK operator new/delete design specially for interrupt handler
 *     - xStl SmartPtr, trace, console commands are locked with interrupt-spin
 *       lock.
 *
 * VERY VERY ANOTHER IMPORTANT NOTE:
 *    When the interrupt handler is executed all hardware interrupt are masked
 *    out. However, an interrupt may and can cause another software inerrupt.
 *    NOTICE!
 *
 * This class can be integrated with the IdtRouterHooker class in order to
 * safely hook/unhook interrupts.
 */
class InterruptListener {
public:
    // Default virtual empty destructor. You can inherit from me.
    virtual ~InterruptListener();

    /*
     * Called upon interrupt.
     * See InterruptListener text body for handler convensions.
     *
     * vector          - The interrupt number
     * processorID        - The ID of the processor. Values from 0 to
     *                      'numberOfProcessors-1'
     * numberOfProcessors - The total number of the processor in the system
     * regs            - Pointer to the Trap frame, which is the registers
     * interruptFrame  - Pointer to the interrupt frame insert by the processor
     * errorCode       - For some trap interrupts, the processor error-code.
     *                   See isErrorCodeTrap
     * oldIrql         - The IRQL before the interrupt.
     *                   if oldIrql equal to TBA_INTERRUPT_IRQL then a recursive
     *                   has happened.
     *
     * Return true for executing the original interrupt.
     *        false for ignoring and perform an IRET.
     */
    virtual bool onInterrupt(uint8 vector,
                             uint processorID,
                             uint numberOfProcessors,
                             Registers* regs,
                             InterruptFrame* interruptFrame,
                             uint32 errorCode,
                             KIRQL oldIrql) = 0;
};
// The reference listener object
typedef cSmartPtr<InterruptListener> InterruptListenerPtr;

#endif // __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTLISTENER_H
