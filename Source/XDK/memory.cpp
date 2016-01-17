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
 * memory.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/os/lock.h"
#include "xStl/os/mutex.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "xdk/memory.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/bugcheck.h"

#ifdef _DEBUG
cOSDef::threadHandle cXdkDriverMemoryManager::m_debugThread
    = INVALID_THREAD_HANDLE;
#endif

// The singleton object
cXdkDriverMemoryManager::Members*
    cXdkDriverMemoryManager::m_members = NULL;

// The termination flag
bool cXdkDriverMemoryManager::m_aboutToTerminate = false;


//////////////////////////////////////////////////////////////////////////

uint cXdkDriverMemoryManager::XdkMemoryAllocator::getSuperblockPageAlignment()
{
    #ifndef XDK_TEST
        return PAGE_SIZE;
    #else
        return 1;
    #endif
}

void* cXdkDriverMemoryManager::XdkMemoryAllocator::allocateNewSuperblock(
                        uint length)
{
    #ifndef XDK_TEST
        return ExAllocatePool(NonPagedPool, length);
    #else
        return VirtualAlloc(NULL, length, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    #endif
}

void cXdkDriverMemoryManager::XdkMemoryAllocator::freeSuperblock(void* pointer)
{
    #ifndef XDK_TEST
        ExFreePool(pointer);
    #else
        VirtualFree(pointer, 0, MEM_RELEASE);
    #endif
}

//////////////////////////////////////////////////////////////////////////

cXdkDriverMemoryManager::Members::Members(uint initializeSize,
                                          uint maxSize) :
    m_isValid(false),
    m_memManager(NULL),
    m_expandor(NULL)
    #ifndef XDK_TEST
        , m_isOperationInProgress(false),
        m_dtorQueue(NULL)
    #endif
{
    m_memManager = new SuperiorMemoryManager(
        SuperiorOSMemePtr(new cXdkDriverMemoryManager::XdkMemoryAllocator()),
        initializeSize,
        m_privatePoolMemory,
        SuperiorMemoryManager::DEFAULT_SUPRIOR_MEMORY_PRIVATE_MEM,
        maxSize);

    // Test that operating system have enough resources
    CHECK(m_memManager != NULL);

    // Start the expander thread
    m_expandor = new cXdkDriverMemoryManager::XdkMemoryExpandor(*m_memManager,
                                                        DEFAULT_REFRESH_RATE);
    m_expandor->start();

    // So far so good
    m_isValid = true;
}

cXdkDriverMemoryManager::Members::~Members()
{
    if (m_expandor != NULL)
    {
        m_expandor->signalTerminate();
        m_expandor->wait();
    }
    delete m_expandor;
    delete m_memManager;
}

//////////////////////////////////////////////////////////////////////////
void cXdkDriverMemoryManager::checkValid()
{
    CHECK(m_members != NULL);
    CHECK(m_members->m_isValid);
}

void cXdkDriverMemoryManager::initialize()
{
    CHECK(m_members == NULL);

    // Allocate and initialize memory
    m_members = new Members();

    #ifndef XDK_TEST
        // Only ring0 have a dtor queue
        // The dtor queue item is now belongs to XDM memory
        cLock lock(m_members->m_dtorQueueLock);
        m_members->m_dtorQueue = new DtorWorkItem();
    #endif // XDK_TEST
}

void cXdkDriverMemoryManager::terminate()
{
    // Free resources safely
    checkValid();

    // Notify operator delete for 'terminate' memory lose
    m_aboutToTerminate = true;

    #ifndef XDK_TEST
        delete m_members->m_dtorQueue;
    #endif // XDK_TEST

    // Remove the member list
    Members* member = m_members;
    member->m_isValid = false;
    m_members = NULL;

    // And free memory
    delete member;

    // Termination ended.
    // Used for multi-initialized drivers
    m_aboutToTerminate = false;
}

void* cXdkDriverMemoryManager::allocate(uint length)
{
    checkValid();

    return m_members->m_memManager->allocate(length);
}

bool cXdkDriverMemoryManager::free(void* address)
{
    if (m_members == NULL)
        return false;
    if (!m_members->m_isValid)
        return false;

    return m_members->m_memManager->free(address);
}

//////////////////////////////////////////////////////////////////////////
// Ring0 operator new/delete implementation

#ifndef XDK_TEST

bool cXdkDriverMemoryManager::acquireClearing()
{
    cLock lock(m_members->m_dtorClearingAcquire);
    if (m_members->m_isOperationInProgress)
        return false;
    // Acquire clearing.
    m_members->m_isOperationInProgress = true;
    return true;
}

void cXdkDriverMemoryManager::releaseClearing()
{
    cLock lock(m_members->m_dtorClearingAcquire);
    m_members->m_isOperationInProgress = false;
}

void cXdkDriverMemoryManager::clearingProcess()
{
    XSTL_TRY
    {
        // Called in IRQL PASSIVE_LEVEL to DISPATCH_LEVEL
        #ifdef _DEBUG
        if (cProcessorUtil::getCurrentIrql() > DISPATCH_LEVEL)
            cBugCheck::bugCheck(0xDEAD0EEE, 1,1,1,1);
        #endif // _DEBUG

        DtorWorkItem listCopy;
        pollList(listCopy);

        // Must be the same as here
        #ifdef _DEBUG
        if (cProcessorUtil::getCurrentIrql() > DISPATCH_LEVEL)
            cBugCheck::bugCheck(0xDEAD0EEE, 2,2,2,2);
        #endif // _DEBUG

        for (DtorWorkItem::iterator i = listCopy.begin();
            i != listCopy.end();
            ++i)
        {
            ExFreePool(getPtr(*i));
        }

        // Must be the same as here
        #ifdef _DEBUG
        if (cProcessorUtil::getCurrentIrql() > DISPATCH_LEVEL)
            cBugCheck::bugCheck(0xDEAD0EEE, 3,3,3,3);
        #endif // _DEBUG
    }
    XSTL_CATCH_ALL
    {
        traceHigh("XDM: FATAL! Unknown exception while clearing memory!" << endl);
    }
}

void cXdkDriverMemoryManager::clearDtorQueue()
{
    // This function can be called by operator new before class initialized
    if (m_members == NULL)
        return;

    if (m_members->m_isValid == false)
        return;

    // Prevent recursing behavior in operator new.
    if (!acquireClearing())
        return;

    clearingProcess();

    releaseClearing();
}

void cXdkDriverMemoryManager::pollList(DtorWorkItem& output)
{
    #ifdef _DEBUG
    KIRQL oldIrql = cProcessorUtil::getCurrentIrql();
    #endif //_DEBUG

    // The next element to free
    cLock lock(m_members->m_dtorQueueLock);

    // Release the pool type.
    #ifdef _DEBUG
    cXdkDriverMemoryManager::m_debugThread = cThread::getCurrentThreadHandle();
    #endif

    DtorWorkItem::iterator i = m_members->m_dtorQueue->begin();
    bool shouldFreeList = false;

    for (; i != m_members->m_dtorQueue->end(); ++i)
    {
        // The list memory is cXdkDriverMemoryManager so there aren't any
        // recursions
        output.append(*i);
        shouldFreeList = true;
    }

    // All elements of m_dtorQueue where allocated from cXdkDriverMemoryManager
    // so the operator delete can be operate normally, including the constructor
    if (shouldFreeList)
    {
        #ifdef _DEBUG
        if (cProcessorUtil::getCurrentIrql() != TBA_INTERRUPT_IRQL)
            cBugCheck::bugCheck(0xDEAD10C6, cProcessorUtil::getCurrentIrql(),
                                            oldIrql, 0x4E4004E4, 3);
        #endif
        // TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!
        //
        // Me and sergei detected that there can be still some cases in which
        // multiprocessors might enter dead-lock due to the following delete
        // PLEASE CHECK IT OUT.
        //
        // TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!TODO!!!!
        m_members->m_dtorQueue->removeAll();
    }

    // Release the pool type.
    #ifdef _DEBUG
    cXdkDriverMemoryManager::m_debugThread = INVALID_THREAD_HANDLE;
    #endif
}

//////////////////////////////////////////////////////////////////////////

cXdkDriverMemoryManager::XdkMemoryExpandor::XdkMemoryExpandor(
            SuperiorMemoryManager& manager,
            uint refreshRateInMilliseconds) :
    m_manager(manager),
    m_refreshRateInUnits(refreshRateInMilliseconds / REFERSH_UNIT_IN_MILLISECONDS),
    m_shouldTerminate(false)
{
}

void cXdkDriverMemoryManager::XdkMemoryExpandor::signalTerminate()
{
    m_shouldTerminate = true;
}

void cXdkDriverMemoryManager::XdkMemoryExpandor::run()
{
    uint units = 0;
    while (!m_shouldTerminate)
    {
        cOS::sleepMillisecond(REFERSH_UNIT_IN_MILLISECONDS);
        units++;

        if (units >= m_refreshRateInUnits)
        {
            // Reset the counter
            units = 0;

            // Refresh memory
            m_manager.manageMemory();

            #ifdef XDK_TRACE_MEMORY
            // Output statistics
            m_manager.traceMemory(traceStream::getTraceHigh());
            #endif
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void * __cdecl operator new(unsigned int cbSize)
{
    if (cbSize == 0)
        // When allocating 0 bytes memory than a valid pointer must return
        cbSize = 1;


    void* ret;

    #ifdef _DEBUG
    // Save the last mode
    uint currentProcessor = cProcessorUtil::getCurrentProcessorNumber();
    KIRQL currentIrql = cProcessorUtil::getCurrentIrql();
    #endif

    if (cProcessorUtil::getCurrentIrql() <= DISPATCH_LEVEL)
    {
        //////////////////////////////////////////////////////////////////
        // Test whether known HIGH_LEVEL IRQL trying to commit illegal
        // operation.
        #ifdef _DEBUG
        if ((cXdkDriverMemoryManager::m_debugThread != INVALID_THREAD_HANDLE) &&
            (cThread::getCurrentThreadHandle() ==
                cXdkDriverMemoryManager::m_debugThread))
            cBugCheck::bugCheck(0xDEAD10C6, 0xDEAD0EEE,
                                cProcessorUtil::getCurrentIrql(), cbSize, 1);
        #endif
        //////////////////////////////////////////////////////////////////

        // This seems to cause a lot of slowness issues
        cXdkDriverMemoryManager::clearDtorQueue();
        ret = ExAllocatePool(NonPagedPool, cbSize);
    } else
    {
        #ifdef _DEBUG
        if (cXdkDriverMemoryManager::m_aboutToTerminate)
            cBugCheck::bugCheck(0xDEAD10C7, 1,0,0,0);
        #endif
        ret = cXdkDriverMemoryManager::allocate(cbSize);
    }

    // Throw exception if the memory cannot be allocated
    if (ret == NULL)
    {
        #ifdef _DEBUG
        // Debug mode cannot have the lack of memory underrun.
        cBugCheck::bugCheck(0xDEAD0EEE, cbSize, cProcessorUtil::getCurrentIrql(),
                                getNumeric(cXdkDriverMemoryManager::m_members),
                                0xDEAD0EEE);
        #endif

        TRACE(TRACE_VERY_HIGH, "Out of memory exception. Allocating ");
        TRACE(TRACE_VERY_HIGH, cString(cbSize));
        TRACE(TRACE_VERY_HIGH, " byte. IRQL ");
        TRACE(TRACE_VERY_HIGH, cString(cProcessorUtil::getCurrentIrql()));

        // Out of memory! Throw exception.
        XSTL_THROW(cException, EXCEPTION_OUT_OF_MEM);
    }

    return ret;
}

void __cdecl operator delete(void *memory)
{
    if (memory != NULL)
    {
        // No matter what IRQL we are in
        if (cXdkDriverMemoryManager::free(memory))
        {
            // The memory belongs to us. OK.
            return;
        }

        if (cProcessorUtil::getCurrentIrql() <= DISPATCH_LEVEL)
        {
            //////////////////////////////////////////////////////////////////
            // Test whether known HIGH_LEVEL IRQL trying to commit illegal
            // operation.
            #ifdef _DEBUG
            if ((cXdkDriverMemoryManager::m_debugThread != INVALID_THREAD_HANDLE) &&
                (cThread::getCurrentThreadHandle() ==
                    cXdkDriverMemoryManager::m_debugThread))
                    cBugCheck::bugCheck(0xDEAD10C6, 0xDEAD0EEE,
                                         cProcessorUtil::getCurrentIrql(),
                                         getNumeric(memory), 2);
            #endif
            //////////////////////////////////////////////////////////////////

            if (!cXdkDriverMemoryManager::m_aboutToTerminate)
                cXdkDriverMemoryManager::clearDtorQueue();

            ExFreePool(memory);
        } else
        {
            // When dtor called, IRQL must be DISPATCH_LEVEL or below!
            if (cXdkDriverMemoryManager::m_aboutToTerminate)
                cBugCheck::bugCheck(0xDEAD10C7, 2,0,0,0);

            // Interrupt time cannot free normal.
            cLock lock(cXdkDriverMemoryManager::m_members->m_dtorQueueLock);

            #ifdef _DEBUG
            // Stack eye-catcher. This way the memory block can be located.
            addressNumericValue eyeCatcher[3];
            eyeCatcher[0] = 0xDEADFACE;
            eyeCatcher[1] = getNumeric(memory);
            eyeCatcher[2] = 0xDEADFACE;
            // Now the IRQL should be HIGH_LEVEL and operator new should be
            // allocating memory only from our memory-pool
            cXdkDriverMemoryManager::m_debugThread =
                cThread::getCurrentThreadHandle();
            #endif

            // Append the memory into a queue of dtors.
            cXdkDriverMemoryManager::m_members->m_dtorQueue->append(
                getNumeric(memory));

            #ifdef _DEBUG
            // Release the pool type.
            cXdkDriverMemoryManager::m_debugThread = INVALID_THREAD_HANDLE;
            #endif
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// User-mode testing application. Uses the XDK-memory-manager only
#else
    void * __cdecl operator new(unsigned int cbSize)
    {
        // The test utility is a ring3 application and must be implement as a mini-singleton
        static bool isInit = false;
        static bool initInProgress = false;
        if (!isInit)
        {
            if (initInProgress)
                return ::malloc(cbSize);

            initInProgress = true;
            cXdkDriverMemoryManager::initialize();
            isInit = true;
        }

        return cXdkDriverMemoryManager::allocate(cbSize);
    }

    void __cdecl operator delete(void *memory)
    {
        // It's ok to delete a NULL-pointer
        if (memory == NULL)
            return;

        if (!cXdkDriverMemoryManager::free(memory))
            ::free(memory);
    }
#endif // XDK_TEST

