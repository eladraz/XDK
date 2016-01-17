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

#ifndef __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGER_H
#define __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGER_H

/*
 * SuperiorMemoryManager.h
 *
 * The main and complete memory manager. Use to implement operator new and
 * operator delete.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/stringerStream.h"
#include "xdk/memory/MemoryLockableObject.h"
#include "xdk/memory/SmallMemoryHeapManager.h"
#include "xdk/memory/SuperiorMemoryManagerInterface.h"

/*
 * When this macro is defined the code is compiled with statistics information
 * routines and informations.
 */
#define SUPERIOR_MEMORY_MANAGER_STATISTICS

/*
 * The main heap manager for any application which need a small pool handler.
 * This class allocate and free large blocks from the operating system and
 * divide them into best allocation units possible.
 * The communication between this class and the operating system is done using
 * the 'SuperiorMemoryManagerInterface' class which allow allocation of large
 * blocks.
 *
 * When the class is constructed a small memory is given and used to allocate
 * buckets. Bucket is an internal use in order to decrease the number of
 * fragmentation. See SuperiorMemoryManager::Bucket for more information
 *
 * NOTE: No global operator new/delete is called during the construction of this
 *       class. This is done in order to prevent recursive calls.
 *
 * NOTE: This interface allows allocating blocks without any fragmentation
 *       (but with memory loss up to half of size) for allocation units 1 bytes
 *       until 256kb. In order to increase the maximum allocation unit, please
 *       change the table m_bucketSizes and append your statistics best-fit
 *       blocks.
 *
 * NOTE: This class is thread-safe and processor safe
 */
class SuperiorMemoryManager : public MemorySuperblockHeapManager {
public:
    // The default maximum allocation size if 4gb of memory
    enum { DEFAULT_MAXIMUM_SIZE = 0xFFFFFFFF };

    // The memory which the superior memory manager is allocating for itself
    // is 16kb which will be used to store about XXX of SmallMemoryHeapManagers
    enum { DEFAULT_SUPRIOR_MEMORY_PRIVATE_MEM = 16*1024 };
    // The default allocation is 4mb memory
    enum { INITIALIZE_SIZE_MINIMUM_SIZE = 4*1024*1024 };

    /*
     * Constructor. Allocate 'initializeSize' of memory from the os interface
     * and divide it into smaller groups.
     *
     * osmem          - The operating system memory allocation interface.
     * initializeSize - The initialize allocated size
     * privateMemPool - The memory pool which is in used by the
     *                  SuperiorMemoryManager. Used for new buckets allocation
     * privateMemPoolLength - The length in bytes of 'privateMemPool'
     * maximumSize    - The maximum size in bytes of which the manager can be
     *                  expand
     *
     * Throw exception if 'initializeSize' is less than the table buckets
     * initial requested memory. See INITIALIZE_SIZE_MINIMUM_SIZE
     *
     * Throw exception if the initialize 'initializeSize' memory buffer couldn't
     * be able to allocate.
     */
    SuperiorMemoryManager(const SuperiorOSMemePtr& osmem,
                          uint initializeSize,
                          void* privateMemPool,
                          uint privateMemPoolLength,
                          uint maximumSize = DEFAULT_MAXIMUM_SIZE);

    /*
     * Free all allocated operating-system memory.
     *
     * NOTE: During and after the destructor all memory manipulation by this
     *       class will be forbidden. Even from other threads.
     *       If there are still blocks which are allocated during the
     *       constructor, then a trace will be invoked.
     */
    virtual ~SuperiorMemoryManager();

    /*
     * See MemorySuperblockHeapManager::allocate
     */
    virtual void* allocate(uint length);

    /*
     * See MemorySuperblockHeapManager::free
     */
    virtual bool free(void* buffer);


    /*
     * See MemorySuperblockHeapManager::getMaximumAllocationUnit
     */
    virtual uint getMaximumAllocationUnit() const;

    /*
     * See MemorySuperblockHeapManager::getMinimumAllocationUnit
     */
    virtual uint getMinimumAllocationUnit() const;

    /*
     * See MemorySuperblockHeapManager::getNumberOfAllocatedBytes
     */
    virtual uint getNumberOfAllocatedBytes() const;

    /*
     * See MemorySuperblockHeapManager::getNumberOfFreeBytes()
     */
    virtual uint getNumberOfFreeBytes() const;


    /*
     * Called by the managing unit of the operating system each time a memory
     * allocation is possible.
     *
     * NOTE: It's gurentee that no mutable are kept lock by the
     *       SuperiorMemoryManager when the 'osmem' API interfaces is being
     *       called.
     */
    void manageMemory();

    #ifdef XDK_TRACE_MEMORY
    /*
     * Output general information in a human readable way to the user
     */
    void traceMemory(cStringerStream& out);
    #endif

private:
    // Deny copy-constructor and operator =
    SuperiorMemoryManager(const SuperiorMemoryManager& other);
    SuperiorMemoryManager& operator = (const SuperiorMemoryManager& other);

    /*
     * A memory manager of a bucket. Can be expand to contains number of linked
     * buckets.
     */
    class Bucket {
    public:
        /*
         * Default constructor. Allocate new bucket.
         *
         * buffer   - The mini-superblock buffer.
         * length   - The length of the mini-superblock
         * unitSize - The max-allocation unit
         * nextHandler - The previous block handler
         */
        Bucket(void* buffer,
               uint length,
               uint unitSize,
               Bucket* nextHandler = NULL);

        /*
         * Overloading operator new. The bucket memory must be allocated from
         * the private stash.
         *
         * See SuperiorMemoryManager::m_privatePool
         */
        void* operator new(uint cbSize,
                           SmallMemoryHeapManager& privateStash);

        /*
         * Overloading operator delete. In order to free the resource
         * allocated by the private stash
         */
        void operator delete (void* ptr,
                              SmallMemoryHeapManager& privateStash);

        /*
         * Return the memory manager unit
         */
        SmallMemoryHeapManager& getManager();

        /*
         * Return the next bucket in the list
         */
        Bucket* getNextBucket();

    private:
        // Deny normal operator new and delete
        void* operator new (uint cbSize);
        void operator delete (void* ptr);
        // Deny copy-constructor and operator =
        Bucket(const Bucket& other);
        Bucket& operator = (const Bucket& other);

        // The manager for the current block
        SmallMemoryHeapManager m_manager;
        // The next handler
        Bucket* m_nextHandler;
    };

    /*
     * Contains a linked list of all superblocks in the system. The linked
     * blocks are allocated from the private stash.
     * Also this class contains a recursive implementation of mini-superblock
     * allocation.
     */
    class SuperblockRepository {
    public:
        /*
         * Constructor. Init the class members
         */
        SuperblockRepository(void* buffer,
                             uint length,
                             SuperblockRepository* nextRepository);

        /*
         * Return the pointer to the next block. Return NULL for the last/first
         * allocated block
         */
        SuperblockRepository* getNextRepository();

        /*
         * Return the original pointer for the operating system
         */
        void* getOSBuffer();

        /*
         * Try to allocate a mini contigus superblock.
         *
         * NOTE: The function assumes realAllocatedBlockSize equals 0
         * NOTE: This function is not thread-safe!
         */
        void* allocate(uint requestedLength,
                       uint minimumLength,
                       uint allocationUnit,
                       uint& realAllocatedBlockSize);

        /*
         * For desperate times, this function is in used
         */
        void* minimumAllocation(uint allocationUnit,
                                uint& realAllocatedBlockSize);

        /*
         * Overloading operator new. The repository memory must be allocated
         * from the private stash.
         *
         * See SuperiorMemoryManager::m_privatePool
         */
        void* operator new(uint cbSize,
            SmallMemoryHeapManager& privateStash);

        /*
         * Overloading operator delete. In order to free the resource
         * allocated by the private stash
         */
        void operator delete (void* ptr,
            SmallMemoryHeapManager& privateStash);

    private:
        // Deny normal operator new and delete
        void* operator new (uint cbSize);
        void operator delete (void* ptr);
        // Deny copy-constructor and operator =
        SuperblockRepository(const SuperblockRepository& other);
        SuperblockRepository& operator = (const SuperblockRepository& other);

        /*
         * Return the number of bytes left for this superblock
         */
        uint getLeftSize() const;

        /*
         * Allocate 'length' bytes from this superblock and return it's pointer
         */
        void* privateMalloc(uint length);

        // The allocated OS buffer
        void* m_buffer;
        // The length of the allocated OS buffer
        uint m_bufferLength;
        // The current non-allocated buffer
        void* m_position;

        // Pointer to the next repository
        SuperblockRepository* m_nextRepository;
    };

    /*
     * Return the first handler for a bucket
     */
    Bucket* safeGetFirstBucket(uint bucket) const;

    /*
     * Return the bucket index for a certain length
     */
    uint getBucketIndex(uint length) const;

    /*
     * Return the buckets total allocated sizes.
     *
     * bucket - The bucket to enumerate
     * type   - See 'SizeType'
     */
    enum SizeType {
        // Return the allocated size
        SIZE_ALLOCATE = 1,
        // Return the free size
        SIZE_FREE     = 2,
        // Return both free and allocated count
        SIZE_ALLOCATE_AND_FREE = SIZE_ALLOCATE | SIZE_FREE,
    };
    uint getBucketSize(uint bucket,
                       uint type) const;

    /*
     * Return the prefered size of a new memory bucket
     */
    uint getNewBucketSize(uint bucket,
                          uint requestedMem) const;

    /*
     * Return the minimum size of a bucket pool
     */
    uint getMinimumBucketSize(uint bucket,
                              uint requestedMem) const;

    /*
     * Return the allocation size of a bucket. The memory that will be allocated
     * will be in multiple of this value
     */
    uint getBucketAllocationUnit(uint bucket,
                                 uint requestedMem) const;

    /*
     * Allocate new super-block by a requested length
     *
     * requestedLength - The super block request block
     * realAllocatedBlockSize - Will be filled with the real-size of the
     *                          avaliable block
     *
     * Return the superblock position. Return NULL if no superblock can be
     * allocated at all.
     */
    void* allocateSuperblock(uint requestedLength,
                             uint minimumLength,
                             uint allocationUnit,
                             uint& realAllocatedBlockSize);


    // The operating system memory allocation interface
    SuperiorOSMemePtr m_osmem;

    // The private pool is used to allocate '' units so
    enum { PRIVATE_POOL_SIZE = sizeof(Bucket) +
                               SmallMemoryHeapManager::ALLOCATED_UNIT_OVERHEAD };

    // The private pool memory
    SmallMemoryHeapManager m_privatePool;

    // The maximum allocation size is allowed
    uint m_osMaximumSize;
    // The os allocate size so far
    uint m_osMemorySize;
    // The internal superblocks size allocated so far
    uint m_allocatedOsMemorySize;

    // The maximum allocation unit is 4gb.
    enum { MAX_BUCKETS = 18 };
    // For all other allocation types
    enum { BUCKET_DEFAULT_CACHE_SIZE = 0xFFFFFFFF};

    // The first handler chain. Protected by the parent m_lock lockable
    Bucket* m_firstBucketHandler[MAX_BUCKETS];


    // The bucket sizes
    struct BucketSizeAndStatistics {
        // The allocation-unit of the bucket
        uint m_bucketUnitSize;
        // The default number of element for the bucket
        uint m_defaultElementsForBucket;
    };
    // The different sizes
    static const BucketSizeAndStatistics m_bucketSizes[MAX_BUCKETS];

    // The superblocks repository
    // Protected by the parent m_lock lockable
    SuperblockRepository* m_superBlockRepository;

    // Set to true when the 'manage' function is in a middle of processing.
    volatile bool m_manageInProgress;

    // The statistics API
    #ifdef SUPERIOR_MEMORY_MANAGER_STATISTICS
    // Count the number of fragmentation
    #endif
};

#endif // __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGER_H
