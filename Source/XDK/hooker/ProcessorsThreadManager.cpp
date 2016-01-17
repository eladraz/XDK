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
 * ProcessorsThreadManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/os/lock.h"
#include "xStl/os/event.h"
#include "xStl/os/mutex.h"
#include "xStl/os/thread.h"
#include "xStl/stream/traceStream.h"
#include "xStl/except/assert.h"
#include "xStl/except/trace.h"
#include "xdk/ehlib/ehlib.h"
#include "xdk/hooker/ProcessorsThread.h"
#include "xdk/hooker/ProcessorsThreadManager.h"

ProcessorsThreadManager::ProcessorsThreadManager() :
    m_processorsHandler(NULL),
    m_isRunning(false),
    m_exitCounter(ProcessorsThread::getNumberOfProcessors()),
    m_jobStartCounter(ProcessorsThread::getNumberOfProcessors()),
    m_executedProcessor(ProcessorsThread::getNumberOfProcessors())
{
    m_processorsHandler =
        cSmartPtr<ProcessorsThread>(new ProcessorsThread(*this));
    m_isRunning = true;
}

ProcessorsThreadManager& ProcessorsThreadManager::getInstance()
{
     static ProcessorsThreadManager gSingleton;
     return gSingleton;
}

void ProcessorsThreadManager::addJob(const ProcessorJobPtr& newJob)
{
    CHECK(m_isRunning);
    // Add job
    cLock lock(m_jobsMutex);
    m_waitingJobs.append(newJob);
    // Signal threads
    m_newJobNotificationEvent.setEvent();
}

ProcessorsThreadManager::~ProcessorsThreadManager()
{
    terminate();
}

void ProcessorsThreadManager::removeDriver()
{
    terminate();
}

void ProcessorsThreadManager::terminate()
{
    m_isRunning = false;
    m_newJobNotificationEvent.setEvent();
    // Busy wait.
    while (m_exitCounter.getValue() > 0)
        cOS::sleepMillisecond(5);
    cOS::sleepMillisecond(50);
}

ProcessorJobPtr ProcessorsThreadManager::getJob()
{
    --m_jobStartCounter;
    if (m_jobStartCounter.getValue() == 0)
    {
        // All threads got the job
        m_newJobNotificationEvent.resetEvent();
        m_jobStartCounter.setValue(ProcessorsThread::getNumberOfProcessors());
        m_newJobNotificationEventAck.setEvent();
    } else
    {
        // Wait until all threads get their jobs
        m_newJobNotificationEventAck.wait();
    }

    // Return the objects
    cLock lock(m_jobsMutex);
    ASSERT(m_waitingJobs.length() != 0);
    /*
    if (0 == m_waitingJobs.length())
        return ProcessorJobPtr();
    */

    return *m_waitingJobs.begin();
}

void ProcessorsThreadManager::endJob()
{
    cLock lock(m_jobsMutex);
    m_executedProcessor--;
    if (m_executedProcessor.getValue() == 0)
    {
        // Remove the job if needed
        m_waitingJobs.remove(m_waitingJobs.begin());
        m_executedProcessor.setValue(ProcessorsThread::getNumberOfProcessors());
        m_newJobNotificationEventAck.resetEvent();
    }
}

void ProcessorsThreadManager::run(const uint processorID,
                                  const uint numberOfProcessors)
{
    while (1)
    {
        // New event notification waiting...
        m_newJobNotificationEvent.wait();

        // Test for exit event.
        if (!m_isRunning)
        {
            --m_exitCounter;
            return;
        }

        // New job was added
        ProcessorJobPtr job = getJob();
        /*
        if (job.isEmpty())
            return;
        */
        // Safely execute the processor job ID
        XSTL_TRY
        {
            job->run(processorID, numberOfProcessors);
        }
        XSTL_CATCH_ALL
        {
            // Notify user?
            traceHigh("ProcessorsThreadManager: Unknown exception at processor ");
            traceHigh(processorID);
            traceHigh(". Exception: ");
            traceHigh(EHLib::getUnknownException() << endl);
        }
        // Mark job processor termination
        endJob();
    }
}
