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

#ifndef __TBA_XDK_HOOKER_PROCESSORSTHREAD_H
#define __TBA_XDK_HOOKER_PROCESSORSTHREAD_H

/*
 * ProcessorsThread.h
 *
 * Generate a thread for each processor in the CPU and make sure that the thread
 * will only run in the context of a single processor.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/list.h"
#include "xStl/data/smartptr.h"
#include "xStl/os/threadedClass.h"


/*
 * A threaded processor job. Executed and generate by the "ProcessorsThread".
 * From n-processors systems executes n threads one per each processor.
 *
 * NOTE: The 'run' function may be called within a context of multiple threads.
 *       All class members must be protected by shared mutex.
 */
class ProcessorJob {
public:
    /*
     * Virtual destructor. You can inherit from me
     */
    virtual ~ProcessorJob() {};

    /*
     * Called upon a life-time of a processor.
     *
     * processorID        - The ID of the processor. Values from 0 to
     *                      'numberOfProcessors-1'
     * numberOfProcessors - The total number of the processor in the system
     */
    virtual void run(const uint processorID,
                     const uint numberOfProcessors) = 0;
};

/*
 * The reference countable processor-job object
 */
typedef cSmartPtr<ProcessorJob> ProcessorJobPtr;


/*
 * The Processors thread generate thread for each processor. The thread runtime
 * gurantee to be running only in one thread.
 *
 * NOTE: About class destruction.
 *       It's recommended to terminate all ProcessorJob objects first and only
 *       then call the destructor of this class.
 *       However if the destructor is called before all threads where
 *       terminated, their destructor will be called and the threads
 *       will terminate pre-maturely.
 */
class ProcessorsThread {
public:
    /*
     * Constructor. Generate the
     */
    ProcessorsThread(ProcessorJob& job);

    /*
     * Destroy the threads and destruct the class.
     */
    ~ProcessorsThread();

    /*
     * Short-cut ProcessorInformationModule::getNumberOfProcessors
     */
    static const uint getNumberOfProcessors();

private:
    /*
     * A thread job which first change the affinity mask and then execute safely
     * the processor mode.
     */
    class ProcessorSingleThread : public cThreadedClass {
    public:
        /*
         * Constructor. Prepare the thread to be single-processor thread
         */
        ProcessorSingleThread(ProcessorJob& job,
                              uint id,
                              uint processors);

        /*
         * Change the affinity map
         * Call job.run() function.
         */
        virtual void run();

    private:
        // Deny copy-constructor and operator =
        ProcessorSingleThread(const ProcessorSingleThread& other);
        ProcessorSingleThread& operator = (const ProcessorSingleThread& other);
        // The processor ID
        uint m_id;
        // The number of processors
        uint m_processors;
        // The job to be executed
        ProcessorJob& m_job;
    };
    // The reference-countable object
    typedef cSmartPtr<ProcessorSingleThread> ProcessorSingleThreadPtr;

    // The list of all generated processors
    cList<ProcessorSingleThreadPtr> m_processorSingleThread;
};

#endif // __TBA_XDK_HOOKER_PROCESSORSTHREAD_H
