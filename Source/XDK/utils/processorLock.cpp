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
 * processorLock.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/utils/bugcheck.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/processorLock.h"

cProcessorLock::cProcessorLock() :
    m_wasInterruptOff(false),
    m_lockAcquire(false)
{
}

void cProcessorLock::clear()
{
    m_lockAcquire = false;
    m_wasInterruptOff = false;
}

void cProcessorLock::lock()
{
    // Logic assertion
    #ifdef _DEBUG
    KIRQL dbgOldIrql = cProcessorUtil::getCurrentIrql();
    if (m_lockAcquire)
        cBugCheck::bugCheck(0x10C610C6, 1,1,1,1);
    #endif // _DEUBG

    m_lockAcquire = true;

	// Check if IF already enabled
    if (!isInterruptFlagSetEnabled())
    {
		// For debugging purposes. This is what we expect
        #ifdef _DEBUG
        if (dbgOldIrql != TBA_INTERRUPT_IRQL)
            cBugCheck::bugCheck(0x10C610C6, 4,4,4,4);
        #endif // _DEBUG
		cProcessorUtil::setIrql(TBA_INTERRUPT_IRQL);
        return;
    }

    // The order of the following operation is exteremly important
    #ifdef _X86_
        _asm cli
    #else
        #error "Implement here!"
    #endif

    m_wasInterruptOff = true;

    // Notify when the interrupt clears
    cProcessorUtil::setIrql(TBA_INTERRUPT_IRQL);
}

void cProcessorLock::unlock()
{
    // Logic assertion
    #ifdef _DEBUG
    if (!m_lockAcquire)
        cBugCheck::bugCheck(0x10C610C6, 2,2,2,2);
    #endif

    m_lockAcquire = false;

	// Unlock only if we locked
    if (m_wasInterruptOff)
    {
        KIRQL oldIrql = cProcessorUtil::setIrql(RETURN_FROM_INTERRUPT_IRQL);

        #ifdef _DEBUG
        if (isInterruptFlagSetEnabled() || (oldIrql != TBA_INTERRUPT_IRQL))
        {
            cBugCheck::bugCheck(0x10C610C6, 3,3,3,3);
        }
        #endif

        // :)
        m_wasInterruptOff = false;

        // Enable interrupts
        #ifdef _X86_
            _asm sti
        #else
            #error "Implement here!"
        #endif
    }
}

bool cProcessorLock::isInterruptFlagSetEnabled()
{
    // NOTE: This function must be executed within a single processor context!
    #if defined(_X86_) && defined (_KERNEL)
    #define INTERRUPT_FLAG_SHIFT 9
    uint8 isInterruptEnabled;
    _asm {
        pushfd
        pop     eax
        shr     eax, INTERRUPT_FLAG_SHIFT
        and     eax, 1
        mov     isInterruptEnabled, al
    }
    return isInterruptEnabled == 1;
    #else
    return true;
    #endif
}
