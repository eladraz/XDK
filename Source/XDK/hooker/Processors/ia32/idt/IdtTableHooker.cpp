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
 * IdtTableHooker.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "xdk/utils/processorLock.h"
#include "xdk/hooker/ProcessorsThread.h"
#include "xdk/hooker/ProcessorsThreadManager.h"
#include "xdk/hooker/Processors/ia32/idt/hookIdt.h"
#include "xdk/hooker/Processors/ia32/idt/IdtTableHooker.h"

IdtTableHooker::IdtTableHooker(InterruptListenerPtr callbackClass,
                               uint startRange /* = 0*/,
                               uint endRange /* = MAX_VECTOR_COUNT*/) :
    m_startRange(startRange),
    m_endRange(endRange),
    m_jobReasonIsHook(true),
    m_tableSize(endRange - startRange + 1),
    m_numberOfProcessors(ProcessorsThread::getNumberOfProcessors()),
    m_processorTaskNumber(ProcessorsThread::getNumberOfProcessors()),
    m_tempCallbackClass(callbackClass)
{
    CHECK(m_startRange <= m_endRange);
    CHECK(m_endRange < MAX_VECTOR_COUNT);

    //traceHigh("[*] Initializing IDtTableHooker" << endl);

    // Generate the dispatch table
    m_hooking.changeSize(m_tableSize * m_numberOfProcessors);

    // Add the job
    ProcessorsThreadManager::getInstance().addJob(
        ProcessorJobPtr(this, SMARTPTR_DESTRUCT_NONE));

    // Wait until finish
    while (m_processorTaskNumber.getValue() > 0)
    {
        cOS::sleepMillisecond(10);
        /*
        if (m_processorTaskNumber.getValue() > 0)
        {
            traceHigh("IdtTableHooker: Ctor wait" << endl);
        }
        */
    }

    // Free the allocated resource
    m_tempCallbackClass = InterruptListenerPtr();
}

IdtTableHooker::~IdtTableHooker()
{
    testPageableCode();
    m_jobReasonIsHook = false;

    // Wait until constructor will done.
    while (m_processorTaskNumber.getValue() > 0)
        cOS::sleepMillisecond(10);

    m_processorTaskNumber.setValue(ProcessorsThread::getNumberOfProcessors());

    // Add the a new remove job
    ProcessorsThreadManager::getInstance().addJob(
        ProcessorJobPtr(this, SMARTPTR_DESTRUCT_NONE));

    // Wait until finish
    while (m_processorTaskNumber.getValue() > 0)
    {
        cOS::sleepMillisecond(10);
        /*
        if (m_processorTaskNumber.getValue() > 0)
        {
            traceHigh("IdtTableHooker: Dtor wait" << endl);
        }
        */
    }
}

void IdtTableHooker::run(const uint processorID,
                         const uint numberOfProcessors)
{
    uint i;

    // TODO! I think that this code is irrelevant.
    //       And the memory-manager should be optimized!
    // cProcessorLock processorLock;
    // cLock lock(processorLock);

    // Lock the hooking lock
    //cLock lock(m_hookLock);

    if (m_jobReasonIsHook)
    {
        //traceHigh("[*] Hooking IDT for processor: " << (processorID + 1) << "\\" << numberOfProcessors << endl);

        // This code is protected from exceptions...
        for (i = m_startRange; i <= m_endRange; i++)
        {
            // Try to hook
            XSTL_TRY
            {
                getEntry(i, processorID) =
                    HookIdtPtr(new HookIdt(i, m_tempCallbackClass));
            } XSTL_CATCH_ALL
            {
                // Interrupt #I couldn't be hooked.
                traceHigh("[!] IdtTableHooker: Cannot hook interrupt #" <<
                          HEXBYTE(i) << " for processor #" << processorID <<
                          endl);
            }
        }
    } else
    {
        cString unhookMessage;
        unhookMessage << "[*] Unhooking IDT for processor: " << (processorID + 1) << "\\" << numberOfProcessors << endl;
        traceHigh(unhookMessage);

        for (i = m_startRange; i <= m_endRange; i++)
        {
            getEntry(i, processorID) = HookIdtPtr();
        }
    }
    --m_processorTaskNumber;
}

bool IdtTableHooker::isInterruptHooked(uint vector) const
{
    bool ret = false;
    for (uint i = 0; i < m_numberOfProcessors; i++)
    {
        bool isHooked = !getEntry(vector, i).isEmpty();
        ret = ret || isHooked;
    }
    return ret;
}

HookIdtPtr& IdtTableHooker::getEntry(uint vector, uint processorID) const
{
    uint position = (vector - m_startRange) + (processorID * m_tableSize);
    return m_hooking[position];
}
