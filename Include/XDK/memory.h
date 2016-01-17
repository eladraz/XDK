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

#ifndef __TBA_XDK_MEMORY_H
#define __TBA_XDK_MEMORY_H

/*
 * memory.h
 *
 * Memory management functions for ring0 code
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/thread.h"
#include "xStl/os/threadedClass.h"
#include "xStl/data/list.h"
#include "xdk/memory/SuperiorMemoryManager.h"
#include "xdk/memory/SuperiorMemoryManagerInterface.h"

#ifndef XDK_TEST
    #include "xdk/utils/interruptSpinLock.h"
    #define LOCKABLE_OBJECT cInterruptSpinLock
#else
    #include "xStl/os/mutex.h"
    #define LOCKABLE_OBJECT cMutex
#endif

/*
 * Global operator new
 *
 * The memory allocation is implement as ExFreePool and ExAllocatePool.
 */
void * __cdecl operator new(unsigned int cbSize);

/*
 * Global operator delete
 *
 * The memory allocation is implement as ExFreePool and ExAllocatePool.
 */
void __cdecl operator delete(void* memory);

/*
 * Allocates memory for functions that executed above DISPATCH_LEVEL.
 *
 * In IRQL higher then DISPATCH_LEVEL the ExAllocatePool/ExFreePool cause dead-
 * locks when executed on dual-processor and hangs in single-processor machines.
 * This class helps the operator new/delete by managing a separate non volatile
 * queue.
 *
 * NOTE: This class is not singleton and not a normal object since the
 *       constructor and the destructor cannot be gurente to be executed at the
 *       beginning and ending of the program
 *
 * The operator new and operator delete divide the allocation into two groups:
 * - Normal mode. Executed at PASSIVE_LEVEL to DISPATCH_LEVEL.
 *      Operator new will allocate memory using ExAllocatePool(NonPaged)
 *      Operator new will delete memory that free at interrupt-time
 *      Operator delete will delete memory which allocate by the ExAllocatePool
 *         using ExFreePool
 *      Operator delete will delete memory which allocate in interrupt time
 *         using cXdkDriverMemoryManager.
 *      Operator delete will delete memory that free at interrupt-time
 *
 * - Interrupt mode. Executed at IRQL higher than DISPATCH_LEVEL
 *      Operator new will allocate memory using cXdkDriverMemoryManager
 *      Operator delete will delete memory that allocated at interrupt time
 *      Operator delete will queue memory that allocated at normal time
 *
 * TODO! After the final algorithm will be written, please document the way
 *       heaps are allocated.
 */
class cXdkDriverMemoryManager {
private:
    // Only the memory-management utilities can access this API
    class MemoryBlockDescriptor;
    friend void * __cdecl operator new(unsigned int cbSize);
    friend void __cdecl operator delete(void* memory);
    friend class cXDKLibCPP;
    friend class XdkMemoryTestSingleton;
    friend class MemoryBlockDescriptor;

    // The memory-address destructor value
    typedef cList<addressNumericValue> DtorWorkItem;

    /////////////////////////////////////////////
    // Functions

    /*
     * Allocate the memory buffer of the XDK.
     *
     * Called by the cXDKLibCPP when the driver is loaded, before all other
     * global objects are constructed.
     */
    static void initialize();

    /*
     * Destory all memory of the memory-manager.
     *
     * Called by cXDKLibCPP when the driver is terminate, after all destructor
     * are called
     */
    static void terminate();

    /*
    * Try to allocate 'length' bytes.
    *
    * Return NULL if there isn't enough resources for the allocation.
    * Return the pointer of the allocated memory
    */
    static void* allocate(uint length);

    /*
     * Free the memory block. Return true if the memory belongs to our private
     * memory stash
     */
    static bool free(void* address);

    //////////////////////////////////////////////////////////////////////////
    // The dtor queue-item.
    #ifndef XDK_TEST

    /*
     * Free all objects which destructed at interrupt time.
     * Called in IRQL PASSIVE_LEVEL to DISPATCH_LEVEL and free memory that dtor
     * at higher IRQL than DISPATCH_LEVEL.
     */
    static void clearDtorQueue();

    /*
     * Called by the 'clearDtorQueue'. Used as atomic, exception safe module
     * for the acquire/releasing mutex
     */
    static void clearingProcess();

    /*
     * Return true if clearing in progress was acquire.
     * Return false if another clearing operation already active
     */
    static bool acquireClearing();

    /*
     * Notify that the clearing operation completed.
     */
    static void releaseClearing();

    /*
     * Safely retrieves the memory to be deleted and remove it from the queue
     *
     * Return the next/first element to be free.
     * Return NULL if the list is empty
     */
    static void pollList(DtorWorkItem& output);
    #endif

    //////////////////////////////////////////////////////////////////////////
    // Debug functions

    /*
     * Throw exception if the cXdkDriverMemoryManager is not initialize
     */
    static void checkValid();

    #ifdef _DEBUG
    // Internal API for debuging bugs inside the memory manager
    static cOSDef::threadHandle m_debugThread;
    #endif

    //////////////////////////////////////////////////////////////////////////
    // Members

    /*
     * The interface for dynamic superblock allocators.
     * Allocates memory from the non-paged pool if possiable.
     */
    class XdkMemoryAllocator : public SuperiorMemoryManagerInterface {
    public:
        /*
         * See SuperiorMemoryManagerInterface::getSuperblockPageAlignment
         * Allocate PAGE_SIZE bytes
         */
        virtual uint getSuperblockPageAlignment();

        /*
         * See SuperiorMemoryManagerInterface::allocateNewSuperblock
         * Allocate memory from the non-paged-pool
         */
        virtual void* allocateNewSuperblock(uint length);

        /*
         * Free the non-paged pool
         */
        virtual void freeSuperblock(void* pointer);
    };

    /*
     * This module is responsible for dynamic memory allocation and expanding
     * using a dedicated thread which manage this code
     *
     * Also when compiled with special preprocessor flag: XDK_TRACE_MEMORY,
     * this module trace out in each time general information regarding to the
     * memory state. If the SUPERIOR_MEMORY_MANAGER_STATISTICS is also defined
     * the amount of information will be greater.
     */
    class XdkMemoryExpandor : public cThreadedClass {
    public:
        // In order to prevent long waiting during destructor, divide the
        // waiting milliseconds times into smaller blocks
        // For now this value is defined as 100 milliseconds, which means that
        // the thread is interrupt every 10 times a second.
        enum { REFERSH_UNIT_IN_MILLISECONDS = 100 };

        /*
         * Constructor. Initialize
         *
         * manager                   - The memory manager to work with. Valid
         *                             from the of thread instance
         *                             (See cThreadedClass::start()) until
         *                             'signalTerminate()'
         * refreshRateInMilliseconds - The number of millisecond until the
         *                             instance will refresh the memory and
         *                             trace statistics.
         */
        XdkMemoryExpandor(SuperiorMemoryManager& manager,
                          uint refreshRateInMilliseconds =
                                    REFERSH_UNIT_IN_MILLISECONDS);

        /*
         * Signal of thread-termination.
         * See cThreadedClass::wait
         */
        void signalTerminate();

    protected:
        /*
         * The thread main routine. Sleep 'm_refreshRateInMilliseconds' and
         * expand the memory
         */
        virtual void run();

    private:
        // Deny copy-constructor and operator =
        XdkMemoryExpandor(const XdkMemoryExpandor& other);
        XdkMemoryExpandor& operator = (const XdkMemoryExpandor& other);

        // The memory manager pointer
        SuperiorMemoryManager& m_manager;

        // The number of millisecond until the instance will refresh the memory
        uint m_refreshRateInUnits;
        // Flag to indicate whether the thread should exit
        volatile bool m_shouldTerminate;
    };

    /*
     * All the static objects should initialize when the 'initialize' will
     * executed. Otherwise interrupt might occured while the global objects
     * table is still under construction.
     * The same for destruction.
     *
     * Using the 'm_members' and operator new/delete to construct/destruct this
     * class
     */
    class Members {
    public:
        // The number of bytes for each superblock. Must be bigger than
        // SuperiorMemoryManager::INITIALIZE_SIZE_MINIMUM_SIZE
        enum { XDM_SUPERBLOCK_LENGTH = 16*1024*1024 };  // 8-mb

        /*
         * Constructor.
         */
        Members(uint initializeSize = XDM_SUPERBLOCK_LENGTH,
                uint maxSize = XDM_SUPERBLOCK_LENGTH * 20);  // Expand to 80mb

        /*
         * Destructor. Free the superior-memory-manager
         */
        ~Members();

        // Set to true if the memory-manager infrastructure are valid
        bool m_isValid;


        // Private cache data
        uint8 m_privatePoolMemory[
            SuperiorMemoryManager::DEFAULT_SUPRIOR_MEMORY_PRIVATE_MEM];

        // The memory manager
        SuperiorMemoryManager* m_memManager;

        // Every 1 minute the memory should be refreshed
        enum { DEFAULT_REFRESH_RATE = 60*1000 };
        // The expandor thread
        cXdkDriverMemoryManager::XdkMemoryExpandor* m_expandor;

        #ifndef XDK_TEST
        //////////////////////////////////////////////////////////////////
        // The dtor-queue is a protected list of all normal-memory which
        // is deallocated during an interrupt. After the interrupt will
        // be free, the operating-system free() method will be called

        // The dtor queue mutable
        LOCKABLE_OBJECT m_dtorQueueLock;
        // The dtor queue
        DtorWorkItem* m_dtorQueue;
        // Protect against 'm_isOperationInProgress' corruptions.
        LOCKABLE_OBJECT m_dtorClearingAcquire;
        // Set to true to indicate that queue clearing is in progress
        bool m_isOperationInProgress;
        #endif // XDK_TEST

    private:
        // Deny operator = and copy-constructor
        Members(const Members& other);
        Members& operator = (const Members& other);
    };

    // The memory struct
    static Members* m_members;
    // A flag which indicate that the memory is about to be free
    static bool m_aboutToTerminate;
};

#endif // __TBA_XDK_MEMORY_H
