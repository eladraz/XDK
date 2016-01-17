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
 * interruptSpinLock.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xdk/utils/bugcheck.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/interruptSpinLock.h"

cInterruptSpinLock::cInterruptSpinLock() :
    m_isLocked(0)
{
    if (cProcessorUtil::getNumberOfProcessors() >
                    cProcessorUtil::MAX_PROCESSORS_SUPPORT)
        cBugCheck::bugCheck(0xBAD09C05);

    // Change the size of the number of the processor running in the system
    for (uint i = 0; i < cProcessorUtil::getNumberOfProcessors(); i++)
        m_processorLockAcquire[i] = false;
}

cInterruptSpinLock::~cInterruptSpinLock()
{
}

//
// IMPORTANT NOTE:
//   Each change in the cInterruptSpinLock::lock() will result in a similar
//   change in the cInterruptSpinLock::tryLock implementation!!
//
void cInterruptSpinLock::lock()
{
    // Generic information about the lock and the unlock API.
    // During these function there shouldn't be any call to operator new/delete
    // since a recursive might be occured.

    // Get the current IRQL used only for debug mode
    // THIS IS NOT SAFE OPERATION SINCE CONTEXT SWITCH MIGHT OCCURED RIGHT AFTER
    // THIS INSTRUCTION.
    #ifdef _DEBUG
    uint dummeyCurrentPid = cProcessorUtil::getCurrentProcessorNumber();
    uint realCurrentPid = 0x666;
    #endif // _DEBUG

    // Test for interrupt-locked
    KIRQL startIrql = cProcessorUtil::getCurrentIrql();
    if (startIrql != TBA_INTERRUPT_IRQL)
    {
        // Protect the processor
        cProcessorLock dummyLock;
        dummyLock.lock();

        uint currentPid = cProcessorUtil::getCurrentProcessorNumber();
        #ifdef _DEBUG
        realCurrentPid = currentPid;
        #endif

        //////////////////////////////////////////////////////////////////////////
        // Protect logic in debug mode
        #ifdef _DEBUG
        KIRQL myIrql = cProcessorUtil::getCurrentIrql();
        // My own assertion
        uint reason = 0xFFFF;
        if (m_lock[currentPid].m_lockAcquire)
            reason = 1;
        if (m_processorLockAcquire[currentPid])
            reason = 2;
        if (myIrql != TBA_INTERRUPT_IRQL)
            reason = 3;
        if ((m_lock[currentPid].m_lockAcquire) ||
            (m_processorLockAcquire[currentPid]) ||
            (myIrql != TBA_INTERRUPT_IRQL))
        {
            // Test that the processor stays stable.
            uint currentProcessor = cProcessorUtil::getCurrentProcessorNumber();
            for (uint x = 0; x < 0x5000000; x++)
            {
                if (currentProcessor != cProcessorUtil::getCurrentProcessorNumber())
                    cBugCheck::bugCheck(0xDEADDEAD, 0xDEADDEAD,0xDEADDEAD,
                                        0x66600666, 0x66600666);

                if (cProcessorUtil::getCurrentIrql() != TBA_INTERRUPT_IRQL)
                    cBugCheck::bugCheck(0x666);
            }

            uint32 tf;
            _asm {
                pushf
                pop eax
                mov tf, eax
            }

            cBugCheck::bugCheck(0xDEAD1777, tf,
                                myIrql,
                                currentPid,
                                reason);
        }
        #endif
        //////////////////////////////////////////////////////////////////////////

        // Now context-switch is disabled, and the current processor number is
        // fixed number
        m_lock[currentPid] = dummyLock;
        m_processorLockAcquire[currentPid] = true;
    } else
    {
        // Mark that the IRQL was already HIGH_LEVEL and shouldn't be drop
        // to the previous m_lock mode.
        #ifdef _DEBUG
        if (m_processorLockAcquire[cProcessorUtil::getCurrentProcessorNumber()] != false)
            cBugCheck::bugCheck(0xDEAD10C6, cProcessorUtil::getCurrentIrql(),
                                2, 2, 2);
        #endif
    }

    //////////////////////////////////////////////////////////////////////////
    // Test for IRQL bugs
    #ifdef _DEBUG
    if (cProcessorUtil::getCurrentIrql() != TBA_INTERRUPT_IRQL)
        cBugCheck::bugCheck(0xDEAD10C6, cProcessorUtil::getCurrentIrql(),
                            startIrql, 6, 6);
    //////////////////////////////////////////////////////////////////////////
    // Test for dead-lock. Locks which held the CPU for long period of time
    // will raise BSOD
    uint count = 0xA000000;
    #endif

    // Until the exchange gets 0...
    while (InterlockedExchange((PLONG)&m_isLocked, 1) == 1)
    #ifndef _DEBUG
         ;

    #else
    //////////////////////////////////////////////////////////////////////////
    // Test for dead-lock
    { // WHILE
        count--;
        if (count == 0)
        {
            SimpleStackTrace stack;
            getStackTrace(stack);
            cBugCheck::bugCheck(0xDEAD10C6, getNumeric(&stack),
                                            getNumeric(&m_lastStack),
                                            stack.m_addr1, stack.m_addr2);
        }
    }
    // Spinlock is acquired, register the stack-trace
    getStackTrace(m_lastStack);
    #endif
    //////////////////////////////////////////////////////////////////////////
}

void cInterruptSpinLock::unlock()
{
    //////////////////////////////////////////////////////////////////////////
    // Release the mutex held
    #ifdef _DEBUG
        m_lastStack.m_addr1 = 0xDEADDEAD;   m_lastStack.m_addr2 = 0xDEADDEAD;
        m_lastStack.m_addr3 = 0xDEADDEAD;   m_lastStack.m_addr4 = 0xDEADDEAD;
        m_lastStack.m_addr5 = 0xDEADDEAD;   m_lastStack.m_addr6 = 0xDEADDEAD;
        uint oldValue =
    #endif
    //////////////////////////////////////////////////////////////////////////

        InterlockedExchange((PLONG)&m_isLocked, 0);

    //////////////////////////////////////////////////////////////////////////
    #ifdef _DEBUG
        CHECK(oldValue == 1);
    #endif
    //////////////////////////////////////////////////////////////////////////

    /*
     * IMPORTANT NOTE:
     *      Every change in the following code section must also be change
     *      inside the 'tryLock()' function: Remove the processor-lock
     */
    uint pid = cProcessorUtil::getCurrentProcessorNumber();
    if (m_processorLockAcquire[pid])
    {
        // Release the processor lock
        m_processorLockAcquire[pid] = false;
        cProcessorLock plock = m_lock[pid];

        ///////////////////////////////////////////////////////////////////////
        #ifdef _DEBUG
        if (!m_lock[pid].m_lockAcquire)
            cBugCheck::bugCheck(0xDEAD10C6, pid, 3, 3, 3);
        #endif
        ///////////////////////////////////////////////////////////////////////
        m_lock[pid].clear();

        // And now it's safe to remove the lock
        plock.unlock();
    }
}

//////////////////////////////////////////////////////////////////////////
// The try-lock is based on the lock implementation only the function
// doesn't wait...
// See cInterruptSpinLock::lock
bool cInterruptSpinLock::tryLock()
{
    // TODO!
    return false;

    // Will be filled later...
    uint currentPid = 0x666;
    bool acquireDummyLock = false;
    // Test for interrupt-locked
    if (cProcessorUtil::getCurrentIrql() != TBA_INTERRUPT_IRQL)
    {
        // Protect the processor
        cProcessorLock dummyLock;
        dummyLock.lock();
        // Get the current IRQL
        currentPid = cProcessorUtil::getCurrentProcessorNumber();
        acquireDummyLock = true;
        // Now context-switch is disabled, and the cProcessorUtil::getCurrentIrql
        // will result in a safe processor-id
        m_lock[currentPid] = dummyLock;
        m_processorLockAcquire[currentPid] = true;
    }

    // Try to lock by putting 1 in the locked value.
    LONG oldValue = InterlockedExchange((PLONG)&m_isLocked, 1);

    // If the old-value was 0 then the spin-lock was free and now it's lock
    // Otherwise (1), the spin-lock was locked
    if (oldValue == 0)
        return true;

    // Remove the processor-lock
    if (acquireDummyLock)
    {
        // Release the processor lock
        m_processorLockAcquire[currentPid] = false;
        cProcessorLock plock(m_lock[currentPid]);
        m_lock[currentPid].clear();
        // And now it's safe to remove the lock
        plock.unlock();
    }

    // Until the exchange gets 0...
    return false;
}

//////////////////////////////////////////////////////////////////////////
// Debug mode-stack trace
#ifdef _DEBUG


#define CHECK_STACK(n)                          \
    if (basePointer == NULL)                    \
    {                                           \
        stackTrace.m_addr6 = 0xDEAD0000 + (n);  \
        return;                                 \
    }


void cInterruptSpinLock::getStackTrace(SimpleStackTrace& stackTrace)
{
    stackTrace.m_addr1 = 0xBADCCBAD;

    /*
     * The next code sample is a ia32bit 6 function level stack-trace.
     * Only incase of deadlock this functions are valid.
     *
    addressNumericValue* basePointer;

    // Get the base-pointer
    _asm {
        mov eax, ebp
        mov basePointer, eax
    }

    // Skip the next function, which are cLock::lock
    CHECK_STACK(0xF);
    basePointer = (addressNumericValue*)(*basePointer);
    // Skip the next function is the parent base, this way we can isolate the
    // problem with more information
    CHECK_STACK(0xF);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(0);

    // Start filling the memory.
    stackTrace.m_addr1 = *(basePointer + 1);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(1);

    stackTrace.m_addr2 = *(basePointer + 1);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(2);

    stackTrace.m_addr3 = *(basePointer + 1);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(3);

    stackTrace.m_addr4 = *(basePointer + 1);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(4);

    stackTrace.m_addr5 = *(basePointer + 1);
    basePointer = (addressNumericValue*)(*basePointer);
    CHECK_STACK(5);

    stackTrace.m_addr6 = *(basePointer + 1);
    /**/
}
#endif // _DEBUG
