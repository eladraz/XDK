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
 * SuperiorMemoryManager.cpp
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
#include "xdk/memory/SuperiorMemoryManager.h"

// NOTE: The overhead size is taken care of inside the
//       'getBucketAllocationUnit' function
const SuperiorMemoryManager::BucketSizeAndStatistics
    SuperiorMemoryManager::m_bucketSizes[MAX_BUCKETS] = {
    { 4,          65536 }, //  1: Allocation 0 to 4 bytes.    64k units = 256kb
    { 8,          32768 }, //  1: Allocation 5 to 8 bytes.    32k units = 256kb
    { 16,         16386 }, //  2: Allocation 9 to 16 bytes    16k units = 256kb
    { 32,          8192 }, //  3: Allocation 17 to 32 bytes    8k units = 256kb
    { 64,          4196 }, //  4: Allocation 33 to 64 bytes    4k units = 256kb
    { 128,         2048 }, //  5: Allocation 65 to 128 bytes   2k units = 256kb
    { 256,         1024 }, //  6: Allocation 129 to 256 bytes  1k units = 256kb
    { 512,          512 }, //  7: Allocation 257 to 512 bytes             256kb
    { 1024,         256 }, //  8: Allocation 513 to 1024 bytes            256kb
    { 2048,         128 }, //  9: Allocation 1024 to 2048 bytes           256kb
    { 4*1024,        64 }, // 10:      2kb-4kb         256kb
    { 8*1024,        32 }, // 11:      4kb-8kb         256kb
    { 16*1024,       16 }, // 12:     8kb-16kb         256kb
    { 32*1024,        8 }, // 13:    16kb-32kb         256kb
    { 64*1024,        4 }, // 14:    32kb-64kb         256kb
    { 128*1024,       2 }, // 15:   64kb-128kb         256kb
    { 256*1024,       2 }, // 16:  128kb-256kb         512kb
    // All other allocation will be throw into this bucket
    { BUCKET_DEFAULT_CACHE_SIZE, 0 }
};

SuperiorMemoryManager::SuperiorMemoryManager(
            const SuperiorOSMemePtr& osmem,
            uint initializeSize,
            void* privateMemPool,
            uint privateMemPoolLength,
            uint maximumSize) :
    // Initialize parent class, but first allocate the initialize memory
    MemorySuperblockHeapManager(NULL, 0),
    m_osmem(osmem),
    m_osMaximumSize(maximumSize),
    m_osMemorySize(initializeSize),
    m_allocatedOsMemorySize(0),
    m_superBlockRepository(NULL),
    m_manageInProgress(false),
    m_privatePool(privateMemPool, privateMemPoolLength, PRIVATE_POOL_SIZE)
{
    ASSERT(m_superBlock == NULL);

    // Check that the initialize memory is good enough
    CHECK(initializeSize >= INITIALIZE_SIZE_MINIMUM_SIZE);

    // Allocate the first super-block.
    void* firstSuperblock = m_osmem->allocateNewSuperblock(initializeSize);
    // If the initialize allocate memory failed, throw an exception
    CHECK(firstSuperblock != NULL);
    // Initialize the repository for the first allocated superblock
    m_superBlockRepository = new(m_privatePool)
        SuperblockRepository(firstSuperblock, initializeSize, NULL);

    // Start by creating empty buckets
    uint i;
    for (i = 0; i < MAX_BUCKETS; i++)
        m_firstBucketHandler[i] = NULL;
}

SuperiorMemoryManager::~SuperiorMemoryManager()
{
    // Assume no operator new/delete is called.

    // Scan all buckets and test no memory is still allocated
    for (uint i = 0; i < MAX_BUCKETS; i++)
    {
        Bucket* bucket = m_firstBucketHandler[i];
        while (bucket != NULL)
        {
            // Trace out information
            #ifdef _DEBUG
            uint left = bucket->getManager().getNumberOfAllocatedBytes();
            if (left != 0)
            {
                // We cannot call operator new
                TRACE(TRACE_VERY_HIGH,
                    XSTL_STRING("SuperiorMemoryManager dtor, memory-leak!!\n"));
                //traceHigh("SuperiorMemoryManager dtor, memory-leak: bucket: " <<
                //          i << "   Size: " << HEXNUMBER(left) << endl);
            }
            #endif

            // Free the bucket from the private-pool
            // This is not necessary, but still I'm the dictator of correct
            // coding.
            Bucket* temp = bucket;
            bucket = bucket->getNextBucket();
            temp->operator delete(temp, m_privatePool);
        }
    }


    // Freeing all superblocks
    SuperblockRepository* superblock = m_superBlockRepository;
    while (superblock != NULL)
    {
        m_osmem->freeSuperblock(superblock->getOSBuffer());

        SuperblockRepository* temp = superblock;
        superblock = superblock->getNextRepository();
        temp->operator delete(temp, m_privatePool);
    }

    // The global private memory is allocate outside this class and must be
    // free there.
}

void* SuperiorMemoryManager::allocate(uint length)
{
    // TODO?!
    if (length == 0)
        return NULL;

    // Get the best bucket position
    uint originalBucket = getBucketIndex(length);

    uint bucket = originalBucket;
    do {
        // Try to allocate from the bucket.
        // NOTE: There is no need to lock here since even if another pointer is
        //       being added then still all pointers are valid.
        Bucket* bucketPtr = safeGetFirstBucket(bucket);
        Bucket* originalBucketPtr = bucketPtr;
        while (bucketPtr != NULL)
        {
            // Try to allocate from the new bucket
            void* ret = bucketPtr->getManager().allocate(length);
            if (ret != NULL)
                return ret;
            // Get the next bucket from the same group
            bucketPtr = bucketPtr->getNextBucket();
        }

        // Performance and race-conditions simple prevent condition
        if (originalBucketPtr != safeGetFirstBucket(bucket))
            continue;

        // This bucket group is full. Try to expand the bucket
        uint allocatedSize;
        uint aunit = getBucketAllocationUnit(bucket, length);
        void* newBuffer = allocateSuperblock(getNewBucketSize(bucket, length),
                                             getMinimumBucketSize(bucket, length),
                                             aunit,
                                             allocatedSize);
        if (newBuffer != NULL)
        {
            // Bucket can be expand
            ASSERT(allocatedSize > length);

            // Lock the bucket and expand
            cLock lock(m_lock);
            Bucket* newBucket = new(m_privatePool) Bucket(newBuffer,
                                        allocatedSize,
                                        aunit,
                                        m_firstBucketHandler[bucket]);
            m_firstBucketHandler[bucket] = newBucket;

            // Allocate and return
            void* ret = newBucket->getManager().allocate(length);
            ASSERT(ret != NULL);
            return ret;
        }

        // Performance and race-conditions simple prevent condition
        if (originalBucketPtr == safeGetFirstBucket(bucket))
        {
            // The current allocation length cannot be allocate from the current
            // bucket. Not the bucket can be expand. Continue to the next bucket
            // and wrap around.
            bucket++;
            if (bucket == MAX_BUCKETS)
                bucket = 0;
        }
    } while (bucket != originalBucket);

    // All buckets are full
    traceHigh("SuperiorMemoryManager: No more memory. All buckets full." << endl);
    traceHigh("SuperiorMemoryManager: Try allocating " << length << " bytes." << endl);
    // Raise the debugger. Wait a minute, we are the debugger, aren't we?!
    // When this case happens, trace out the statistics...
    return NULL;
}

bool SuperiorMemoryManager::free(void* buffer)
{
    // Try to free the block from all buckets.
    // This is the right thing to do since the allocate buffer might be in a
    // different bucket group depending on memory fragmentation.
    for (uint i = 0; i < MAX_BUCKETS; i++)
    {
        // NOTE: There is no need to lock here since even if another pointer is
        //       being added then still all pointers are valid.
        Bucket* bucket = safeGetFirstBucket(i);
        while (bucket != NULL)
        {
            // If the buffer is not one of buckets then false will be returned.
            if (bucket->getManager().free(buffer))
                return true;
            // Get the next bucket from the same group
            bucket = bucket->getNextBucket();
        }
    }

    // Cannot free the block
    return false;
}

uint SuperiorMemoryManager::getMaximumAllocationUnit() const
{
    return BUCKET_DEFAULT_CACHE_SIZE;
}

uint SuperiorMemoryManager::getMinimumAllocationUnit() const
{
    return 0;
}

uint SuperiorMemoryManager::getNumberOfAllocatedBytes() const
{
    uint ret = 0;
    for (uint i = 0; i < MAX_BUCKETS; i++)
    {
        ret+= getBucketSize(i, SIZE_ALLOCATE);
    }
    return ret;
}

uint SuperiorMemoryManager::getNumberOfFreeBytes() const
{
    uint ret = 0;
    for (uint i = 0; i < MAX_BUCKETS; i++)
    {
        ret+= getBucketSize(i, SIZE_FREE);
    }
    return ret;
}


SuperiorMemoryManager::Bucket* SuperiorMemoryManager::safeGetFirstBucket(
                                                              uint bucket) const
{
    ASSERT(bucket < MAX_BUCKETS);

    cLock lock(m_lock);
    return m_firstBucketHandler[bucket];
}

uint SuperiorMemoryManager::getBucketIndex(uint length) const
{
    for (uint i = 0; i < MAX_BUCKETS; i++)
        // NOTE: The overhead size is taken care of inside the
        //       'getBucketAllocationUnit' function
        if (length <= m_bucketSizes[i].m_bucketUnitSize)
            return i;

    // Something went wrong with the table
    ASSERT_FAIL(XSTL_STRING("Invalid terminate bucket"));
    // Return the default bucket
    return MAX_BUCKETS - 1;
}

uint SuperiorMemoryManager::getBucketSize(uint bucket,
                                          uint type) const
{
    ASSERT(bucket < MAX_BUCKETS);

    uint ret = 0;
    Bucket* bucketPtr = safeGetFirstBucket(bucket);
    while (bucketPtr != NULL)
    {
        if ((type & SIZE_ALLOCATE) != 0)
            ret+= bucketPtr->getManager().getNumberOfAllocatedBytes();
        if ((type & SIZE_FREE) != 0)
            ret+= bucketPtr->getManager().getNumberOfFreeBytes();

        // Get the next bucket from the same group
        bucketPtr = bucketPtr->getNextBucket();
    }
    return ret;
}

uint SuperiorMemoryManager::getNewBucketSize(uint bucket,
                                             uint requestedMem) const
{
    ASSERT(bucket < MAX_BUCKETS);

    // For the nil bucket
    if (m_bucketSizes[bucket].m_bucketUnitSize == BUCKET_DEFAULT_CACHE_SIZE)
    {
        return requestedMem + SmallMemoryHeapManager::ALLOCATED_UNIT_OVERHEAD;
    }

    uint initSize = getBucketSize(bucket, SIZE_ALLOCATE_AND_FREE);
    if (initSize == 0)
    {
        // There is not bucket allocated yet
        return getMinimumBucketSize(bucket, requestedMem);
    }

    // Allocate x0.7 of the enitre old allocated size
    return initSize;
}

uint SuperiorMemoryManager::getMinimumBucketSize(uint bucket,
                                                 uint requestedMem) const
{
    ASSERT(bucket < MAX_BUCKETS);

    // For the nil bucket
    if (m_bucketSizes[bucket].m_bucketUnitSize == BUCKET_DEFAULT_CACHE_SIZE)
    {
        return requestedMem + SmallMemoryHeapManager::ALLOCATED_UNIT_OVERHEAD;
    }

    return m_bucketSizes[bucket].m_defaultElementsForBucket *
            getBucketAllocationUnit(bucket, requestedMem);
}

uint SuperiorMemoryManager::getBucketAllocationUnit(uint bucket,
                                                    uint requestedMem) const
{
    ASSERT(bucket < MAX_BUCKETS);

    // For the nil bucket
    if (m_bucketSizes[bucket].m_bucketUnitSize == BUCKET_DEFAULT_CACHE_SIZE)
    {
        return requestedMem + SmallMemoryHeapManager::ALLOCATED_UNIT_OVERHEAD;
    }

    // Don't forget to include the over-head as well!
    return m_bucketSizes[bucket].m_bucketUnitSize +
           SmallMemoryHeapManager::ALLOCATED_UNIT_OVERHEAD;
}

//
// Second most important functions set after 'allocate/free'. These functions
// are responsible for dynamic memory segmentation
//
void* SuperiorMemoryManager::allocateSuperblock(uint requestedLength,
                                                uint minimumLength,
                                                uint allocationUnit,
                                                uint& realAllocatedBlockSize)
{
    ASSERT(requestedLength > 0);

    // Reset the allocation buffer
    realAllocatedBlockSize = 0;
    // Lock all superblock activities. This section is critical
    cLock lock(m_lock);
    void* ret = m_superBlockRepository->allocate(requestedLength,
                                                 minimumLength,
                                                 allocationUnit,
                                                 realAllocatedBlockSize);

    if (ret == NULL)
    {
        // Try to allocate the best fit for the remainding of the memory
        for (uint j = ((minimumLength / allocationUnit) >> 1); j > 0; j>>= 1)
        {
            ret = m_superBlockRepository->minimumAllocation(
                                                       j * allocationUnit,
                                                       realAllocatedBlockSize);
            if (ret != NULL)
            {
                m_allocatedOsMemorySize+= realAllocatedBlockSize;
                return ret;
            }
        }
    }

    m_allocatedOsMemorySize+= realAllocatedBlockSize;
    return ret;
}

void SuperiorMemoryManager::manageMemory()
{
    // Lock all superblock activities. This section is critical
    cLock lock(m_lock);
    // NOTE! From now until 'lock' is free no operator new should be invoke at
    //       any way! Otherwise deadlock will happen.

    // Test previous manage code
    if (m_manageInProgress)
        return;

    // Test whether the total number of allocated memory is close to the
    // number of superblock size
    if ((m_allocatedOsMemorySize * 2) > m_osMemorySize)
    {
        // Need to allocate more superblock
        uint newSuperblockSize = m_osMaximumSize - m_osMemorySize;
        newSuperblockSize = t_min(newSuperblockSize, m_osMemorySize / 2);

        // Align superblock
        newSuperblockSize = (newSuperblockSize /
                             m_osmem->getSuperblockPageAlignment());
        newSuperblockSize*= m_osmem->getSuperblockPageAlignment();


        // This code should be executed without any guards.
        m_manageInProgress = true;
        m_lock.unlock();
        // {
        void* ptr = m_osmem->allocateNewSuperblock(newSuperblockSize);
        // }
        m_lock.lock();
        m_manageInProgress = false;

        if (ptr == NULL)
        {
            // Free the lockable
            lock.unlock();
            traceHigh("SuperiorMemoryManager: No more operating system memory..." << endl);
            cOS::debuggerBreak();
            return;
        }

        // And expand
        m_osMemorySize+= newSuperblockSize;
        SuperblockRepository* newBlock = new(m_privatePool)
            SuperblockRepository(ptr,
                                 newSuperblockSize,
                                 m_superBlockRepository);
        m_superBlockRepository = newBlock;

        // Free the lockable
        lock.unlock();
        traceHigh("SuperiorMemoryManager: +++ Expanding superblock: " <<
                  HEXDWORD(newSuperblockSize) << endl);
    }
}

#ifdef XDK_TRACE_MEMORY
void SuperiorMemoryManager::traceMemory(cStringerStream& out)
{
    out << "SuperiorMemoryManager: ***** INFO ********************" << endl;
    out << "SuperiorMemoryManager: Allocated OS memory - " <<
           m_osMemorySize / (1024 * 1024) << "Mb" << endl;
    out << "SuperiorMemoryManager: Allocated superblocks - " <<
           m_allocatedOsMemorySize / (1024 * 1024) << "Mb" << endl;

    // Scan all memory
    for (uint i = 0; i < MAX_BUCKETS; i++)
    {
        // NOTE: There is no need to lock here since even if another pointer is
        //       being added then still all pointers are valid.
        Bucket* bucket = safeGetFirstBucket(i);

        if (bucket != NULL)
        {
            out << "SuperiorMemoryManager: Bucket " << i << "  Units: "
                << m_bucketSizes[i].m_bucketUnitSize << endl;
        }

        while (bucket != NULL)
        {
            out << "SuperiorMemoryManager: Bucket " << i << "  Allocated: "
                << HEXDWORD(bucket->getManager().getNumberOfAllocatedBytes()) << endl;
            out << "SuperiorMemoryManager: Bucket " << i << "  Free:      "
                << HEXDWORD(bucket->getManager().getNumberOfFreeBytes()) << endl;

            // Get the next bucket from the same group
            bucket = bucket->getNextBucket();
        }
    }
}
#endif // XDK_TRACE_MEMORY

//////////////////////////////////////////////////////////////////////////
// Bucket

SuperiorMemoryManager::Bucket::Bucket(void* buffer,
                                      uint length,
                                      uint unitSize,
                                      Bucket* nextHandler) :
    m_manager(buffer, length, unitSize),
    m_nextHandler(nextHandler)
{
}

void* SuperiorMemoryManager::Bucket::operator new (
          uint cbSize,
          SmallMemoryHeapManager& privateStash)
{
    void* ret = privateStash.allocate(cbSize);
    // Not enough private stash memory
    CHECK(ret != NULL);
    return ret;
}

void SuperiorMemoryManager::Bucket::operator delete (
          void *ptr,
          SmallMemoryHeapManager& privateStash)
{
    // Serious bug if this check is failed.
    CHECK(privateStash.free(ptr));
}

SmallMemoryHeapManager& SuperiorMemoryManager::Bucket::getManager()
{
    return m_manager;
}

SuperiorMemoryManager::Bucket* SuperiorMemoryManager::Bucket::getNextBucket()
{
    return m_nextHandler;
}

//////////////////////////////////////////////////////////////////////////
// SuperblockRepository
SuperiorMemoryManager::SuperblockRepository::SuperblockRepository(
                     void* buffer,
                     uint length,
                     SuperblockRepository* nextRepository) :
    m_buffer(buffer),
    m_bufferLength(length),
    m_position(m_buffer),
    m_nextRepository(nextRepository)
{
}

SuperiorMemoryManager::SuperblockRepository*
    SuperiorMemoryManager::SuperblockRepository::getNextRepository()
{
    return m_nextRepository;
}

void* SuperiorMemoryManager::SuperblockRepository::getOSBuffer()
{
    return m_buffer;
}

void* SuperiorMemoryManager::SuperblockRepository::operator new (
    uint cbSize,
    SmallMemoryHeapManager& privateStash)
{
    void* ret = privateStash.allocate(cbSize);
    // Not engouth private stash memory
    CHECK(ret != NULL);
    return ret;
}

void SuperiorMemoryManager::SuperblockRepository::operator delete (
    void *ptr,
    SmallMemoryHeapManager& privateStash)
{
    // Serious bug if this check is failed.
    CHECK(privateStash.free(ptr));
}

uint SuperiorMemoryManager::SuperblockRepository::getLeftSize() const
{
    return m_bufferLength - (getNumeric(m_position) - getNumeric(m_buffer));
}

void* SuperiorMemoryManager::SuperblockRepository::privateMalloc(uint length)
{
    // A simple test couldn't heart
    ASSERT(length <= getLeftSize());

    void* ret = m_position;
    m_position = getPtr(getNumeric(m_position) + length);
    return ret;
}

void* SuperiorMemoryManager::SuperblockRepository::allocate(
               uint requestedLength,
               uint minimumLength,
               uint allocationUnit,
               uint& realAllocatedBlockSize)
{
    // First send the request to the previous allocated blocks
    if (m_nextRepository != NULL)
    {
        void* ret = m_nextRepository->allocate(requestedLength,
                            minimumLength,
                            allocationUnit,
                            realAllocatedBlockSize);
        if (ret != NULL)
            return ret;
    }

    // Try to allocate from this superblock
    uint leftSize = getLeftSize();

    if (leftSize < minimumLength)
        return NULL;

    // Allocate best-fit size
    uint bestFitSize = requestedLength;
    if (leftSize < requestedLength)
    {
        // Calculate the best-fit memory size
        uint numOfElements = (leftSize - minimumLength) / allocationUnit;
        bestFitSize = minimumLength + (allocationUnit * numOfElements);
        // Some assertion about this calc
        ASSERT(bestFitSize <= leftSize);
    }

    // Allocate memory and return
    realAllocatedBlockSize = bestFitSize;
    ASSERT(realAllocatedBlockSize >= minimumLength);
    return privateMalloc(bestFitSize);
}


void* SuperiorMemoryManager::SuperblockRepository::minimumAllocation(
                        uint allocationUnit,
                        uint& realAllocatedBlockSize)
{
    // First send the request to the previous allocated blocks
    if (m_nextRepository != NULL)
    {
        void* ret = m_nextRepository->minimumAllocation(allocationUnit,
                                                        realAllocatedBlockSize);
        if (ret != NULL)
            return ret;
    }

    uint leftSize = getLeftSize();
    uint numOfElements = leftSize / allocationUnit;

    if (numOfElements == 0)
        return NULL;

    uint allocation = numOfElements * allocationUnit;
    realAllocatedBlockSize = allocation;
    ASSERT(realAllocatedBlockSize >= allocationUnit);
    return privateMalloc(allocation);
}
