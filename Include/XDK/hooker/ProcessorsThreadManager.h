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

#ifndef __TBA_XDK_HOOKER_PROCESSORSTHREADMANAGER_H
#define __TBA_XDK_HOOKER_PROCESSORSTHREADMANAGER_H

/*
 * ProcessorsThreadManager.h
 *
 * Manages system contains multi-processors.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/event.h"
#include "xStl/os/mutex.h"
#include "xStl/os/threadedClass.h"
#include "xStl/data/counter.h"
#include "xdk/hooker/ProcessorsThread.h"

/*
 * A singleton class which manages system contains multi-processors.
 *
 * TODO! This class is implement as an interface only (for now only)!
 */
class ProcessorsThreadManager : public ProcessorJob {
public:
    /*
     * Destructor. When called stop all executed threads
     */
    virtual ~ProcessorsThreadManager();

    /*
     * Return the singleton instance
     */
    static ProcessorsThreadManager& getInstance();

    /*
     * Append a new task to the processor.
     * Throw exception if the manager is in remove-pending
     */
    void addJob(const ProcessorJobPtr& newJob);

    /*
     * Called in order to destroy threads. Some dynamic drivers might use this
     * function in order to kill the threads before 'unload'.
     * NOTE: The previous jobs in the queue will be deprecated
     *
     * TODO! replace that interface while generating singularity driver.
     */
    void removeDriver();

protected:
    /*
     * See ProcessorJob::run
     *
     * Execute the waiting jobs.
     */
    virtual void run(const uint processorID,
                     const uint numberOfProcessors);

private:
    /*
     * Private constructor. Singleton.
     */
    ProcessorsThreadManager();

    // Deny copy-constructor and operator =
    ProcessorsThreadManager(const ProcessorsThreadManager& other);
    ProcessorsThreadManager& operator = (const ProcessorsThreadManager& other);

    /*
     * Safely returns the first job to be executed
     */
    ProcessorJobPtr getJob();

    /*
     * Mark the a processor running end for the currrent job
     */
    void endJob();

    /*
     * Terminate the threads
     */
    void terminate();

    // Set when there are new jobs in the queue. Lower by the handler
    cEvent m_newJobNotificationEvent;
    cEvent m_newJobNotificationEventAck;

    // Protect the jobs queue
    cMutex m_jobsMutex;

    // The jobs queue
    cList<ProcessorJobPtr> m_waitingJobs;

    // The first job executed count
    cCounter m_executedProcessor;

    // The exit counter
    cCounter m_exitCounter;

    // The number of threads gots the job
    cCounter m_jobStartCounter;

    // The processor threads hooker
    cSmartPtr<ProcessorsThread> m_processorsHandler;

    // Set to indicates that the processor threads are running. Used as destroy
    // event as well
    volatile bool m_isRunning;
};

#endif // __TBA_XDK_HOOKER_PROCESSORSTHREADMANAGER_H
