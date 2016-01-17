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
 * processorUtil.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/utils/algorithm.h"
#include "xdk/kernel.h"
#include "xdk/utils/bugcheck.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/processorLock.h"

//////////////////////////////////////////////////////////////////////////
// Global tools

/*
 * A global initializer. Called during driver startup and cache the number
 * of processors and thier locations.
 */
class ProcessorUtilPublicInitializer {
public:
    /*
     * Default constructor. Init the number of processors
     */
    ProcessorUtilPublicInitializer()
    {
        #ifdef NTDDK
            // TODO! All processors are active in windows NT.
            gNumberOfActiveProcessors = *KeNumberProcessors;
        #else
            // 2000 DDK and above
            // TODO! KeQueryActiveProcessors()
            gNumberOfActiveProcessors = KeNumberProcessors;
        #endif

        if (gNumberOfActiveProcessors > cProcessorUtil::MAX_PROCESSORS_SUPPORT)
        {
            // Need more processors
            cBugCheck::bugCheck(cProcessorUtil::PROCESSOR_LOCK_BUGCHECK_CODE,
                                1,1,1,1);
        }
    }

    // The default not initialize number
    enum { DEFAULT_NO_INITIALIZE = 0xFFFFFFFF };

    // The cache number of active processors
    static uint gNumberOfActiveProcessors;
};

// The number of processor is not initialize yet
uint ProcessorUtilPublicInitializer::gNumberOfActiveProcessors
    = DEFAULT_NO_INITIALIZE;

// When the driver is first loaded, the default constructor will be invoked.
// In the mean time, the gNumberOfActiveProcessors value will be -1.
static ProcessorUtilPublicInitializer gProcessorNumberCache;


/*
 * An array of all processors which either in interrupt mode or in a state
 * of processor lock
 */
static volatile bool gMaxIrql[cProcessorUtil::MAX_PROCESSORS_SUPPORT] =
    { false, false, false, false,
      false, false, false, false };


/*
 * Define which IRQL debug data will be included
 */
#ifdef _DEBUG
    // #define _IRQL_DEBUG
    // #define _STACK_IRQL_DEUG
#endif

/*
 * Debug information for processor-util class
 *
 * Saves the last 8 irql's update which encoded by the pair {processor,irql}
 */
#ifdef _IRQL_DEBUG
    #define DBG_PROCESSOR_SAVE_COUNT (16)
    static volatile uint32 gDbgLastIrqlsUpdates[DBG_PROCESSOR_SAVE_COUNT][3] =
        { {0xFC,0xFC,0}, {0xFC,0xFC,0}, {0xFC,0xFC,0}, {0xFC,0xFC,0},
        {0xFC,0xFC,0}, {0xFC,0xFC,0}, {0xFC,0xFC,0}, {0xFC,0xFC,0} };
    static volatile KIRQL gDbgOldIrql = 0xFC;

    #ifdef _STACK_IRQL_DEBUG
        #define DBG_STACK_SIZE (PAGE_SIZE)
        static uint8 gDbgLastStackTrace[DBG_STACK_SIZE];
    #endif // _STACK_IRQL_DEBUG
#endif // _IRQL_DEBUG

//////////////////////////////////////////////////////////////////////////
// cProcessorUtil implementation
//////////////////////////////////////////////////////////////////////////

uint cProcessorUtil::getNumberOfProcessors()
{
    if (gProcessorNumberCache.gNumberOfActiveProcessors ==
        ProcessorUtilPublicInitializer::DEFAULT_NO_INITIALIZE)
    {
        // The constructor is not initialized this value yet.
        // Use normal implementation
        #ifdef NTDDK
            return *KeNumberProcessors;
        #else
            return KeNumberProcessors;
        #endif
    }

    return gProcessorNumberCache.gNumberOfActiveProcessors;
}

uint cProcessorUtil::getCurrentProcessorNumber()
{
    // The KeGetCurrentProcessorNumber is defined as inline function by the
    // kernel.

    // TODO! Check compilation!
    uint ret = KeGetCurrentProcessorNumber();

    #ifdef _DEUBG
    // TODO! Remove this block from source
    if (ret > 1)
        KeBugCheck(0x66667777);
    #endif

    return ret;
}

bool cProcessorUtil::isInterruptModeIrql()
{
    // Some-kind of wired work-around
    if (cProcessorLock::isInterruptFlagSetEnabled())
    {
        // This is a caching bug?
        #ifdef _DEBUG
        static uint faultCacheMiss = 0;
        if (gMaxIrql[cProcessorUtil::getCurrentProcessorNumber()])
        {
            faultCacheMiss++;
        }
        #endif

        return false;
    } else
    {
        // When interrupt is thrown, the gMaxIrqls tells the different
        // between recursive behaviour or not...
        return gMaxIrql[cProcessorUtil::getCurrentProcessorNumber()];
    }
}

KIRQL cProcessorUtil::getCurrentIrql()
{
    KIRQL ret;
    if (isInterruptModeIrql())
    {
        ret = TBA_INTERRUPT_IRQL;
    } else
    {
        ret = KeGetCurrentIrql();

        #ifdef _DEBUG
            if (ret == TBA_INTERRUPT_IRQL)
            {
                cBugCheck::bugCheck(0xDEAD, 0x444, 6);
            }
        #endif
    }

    #ifdef _IRQL_DEBUG
    gDbgOldIrql = ret;
    #endif

    return ret;
}

KIRQL cProcessorUtil::setIrql(KIRQL newIrql)
{
    #ifdef _IRQL_DEBUG
        // Shift struct
        uint i;
        for (i = 1; i < DBG_PROCESSOR_SAVE_COUNT; i++)
        {
            gDbgLastIrqlsUpdates[DBG_PROCESSOR_SAVE_COUNT-i][0] =
                gDbgLastIrqlsUpdates[DBG_PROCESSOR_SAVE_COUNT-i-1][0];
            gDbgLastIrqlsUpdates[DBG_PROCESSOR_SAVE_COUNT-i][1] =
                gDbgLastIrqlsUpdates[DBG_PROCESSOR_SAVE_COUNT-i-1][1];
        }
        // Remember the last update
        gDbgLastIrqlsUpdates[0][0] = (uint8)cProcessorUtil::getCurrentProcessorNumber();
        gDbgLastIrqlsUpdates[0][1] = newIrql;

        #ifdef _STACK_IRQL_DEUG
            // Remember last stack trace
            addressNumericValue stackPointer;
            _asm {
                mov stackPointer, esp
            }
            memset(gDbgLastStackTrace, 0xCD, DBG_STACK_SIZE);
            uint stackLength = PAGE_SIZE -
                                (stackPointer - (stackPointer & 0xFFFFF000));
            stackLength = t_min(stackLength, (uint)DBG_STACK_SIZE);
            cOS::memcpy(gDbgLastStackTrace, getPtr(stackPointer), stackLength);
        #endif // _STACK_IRQL_DEUG
    #endif // _DEUBG

    // The new IRQL indicate on interrupt
    if (newIrql == TBA_INTERRUPT_IRQL)
    {
        // Interrupt must be disabled for this processor
        // Otherwise a bug will occured.
        #ifdef _DEBUG
            if (cProcessorLock::isInterruptFlagSetEnabled())
                cBugCheck::bugCheck(0xDEAD, 0x444, 1);
        #endif // _DEBUG

        // Test whether the processor is already in trap-mode
        if (gMaxIrql[cProcessorUtil::getCurrentProcessorNumber()])
            return TBA_INTERRUPT_IRQL;

        // Get the old IRQL, this code might be dangerous...
        // TODO!
        KIRQL oldIrql = KeGetCurrentIrql();

        #ifdef _DEBUG
            if (cProcessorLock::isInterruptFlagSetEnabled())
                cBugCheck::bugCheck(0xDEAD, 0x444, 3);
        #endif // _DEBUG

        // The processor is locked. No context switch at all.
        gMaxIrql[cProcessorUtil::getCurrentProcessorNumber()] = true;
        return oldIrql;
    }

    KIRQL oldIrql = getCurrentIrql();
    if (oldIrql == TBA_INTERRUPT_IRQL)
    {
        // The context-switch routines aren't enabled yet.
        // Interrupts disabled.
        #ifdef _DEBUG
            if (cProcessorLock::isInterruptFlagSetEnabled())
                cBugCheck::bugCheck(0xDEAD, 0x444, 2);
        #endif // _DEBUG


        // Change the IRQL from interrupt mode back to normal
        gMaxIrql[cProcessorUtil::getCurrentProcessorNumber()] = false;

        if (newIrql != RETURN_FROM_INTERRUPT_IRQL)
            cBugCheck::bugCheck(cProcessorUtil::PROCESSOR_LOCK_BUGCHECK_CODE,
                                3,3,3,3);

        return TBA_INTERRUPT_IRQL;
    }

    // Check IRQL is lower then High level
    if (newIrql > HIGH_LEVEL) // (newIrql == RETURN_FROM_INTERRUPT_IRQL)
        cBugCheck::bugCheck(cProcessorUtil::PROCESSOR_LOCK_BUGCHECK_CODE,
            4,4,3,3);

    // Otherwise just swap IRQL
    KeRaiseIrql(newIrql, &oldIrql);
    return oldIrql;
}
