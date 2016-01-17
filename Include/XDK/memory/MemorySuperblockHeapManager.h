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

#ifndef __TBA_XDK_MEMORY_MEMORYSUPERBLOCKHEAPMANAGER_H
#define __TBA_XDK_MEMORY_MEMORYSUPERBLOCKHEAPMANAGER_H

/*
 * MemorySuperblockHeapManager.h
 *
 * Contain interface for super-block memory allocation routines.
 * Super-block is a large block from the operating system which is dedicated
 * for heap.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/memory/MemoryLockableObject.h"

/*
 * The interface for superblock manager.
 */
class MemorySuperblockHeapManager {
public:
    /*
     * Virtual destructor. You can inherit from me
     *
     * NOTE: The destructor doesn't release the memory block!
     */
    virtual ~MemorySuperblockHeapManager();

    /*
     * Constructor. Initialize the super-block manager
     *
     * superBlock - Pointer to the superblock memory
     * superBlockLength - The length in bytes of the super-block
     *
     * NOTE: The pointer is allocated and free by outside module.
     */
    MemorySuperblockHeapManager(void* superBlock,
                                uint superBlockLength);

    /*
     * Allocate a pool from the superblock heap
     *
     * length - The number of bytes to be allocated
     *
     * Return NULL incase there isn't enough room in the super-block
     *
     * Implementation notes:
     *    1. Lock the mutex for the enitre function (m_lock)
     *    2. Don't forget to update the 'm_allocatedBytes'
     */
    virtual void* allocate(uint length) = 0;

    /*
     * Free a block of memory
     *
     * Return false if 'buffer' is not a valid pointer
     * Return true if the buffer was free.
     *
     * NOTE: For NULL pointers, the free routine will return false.
     *
     * Implementation notes:
     *    1. Lock the mutex for the enitre function (m_lock)
     *    2. Don't forget to update the 'm_allocatedBytes'
     */
    virtual bool free(void* buffer) = 0;


    /*
     * Return the maximum number of bytes this superblock allows to
     */
    virtual uint getMaximumAllocationUnit() const = 0;

    /*
     * Return the minimum number of bytes when the superblock allocation
     * overhead will be most affective.
     */
    virtual uint getMinimumAllocationUnit() const = 0;


    /*
     * Return 'm_allocatedBytes'
     */
    virtual uint getNumberOfAllocatedBytes() const;

    /*
     * Return 'm_superBlockLength' - 'm_allocatedBytes'
     */
    virtual uint getNumberOfFreeBytes() const;

protected:
    /*
     * Return true if the pointer 'addr' is inside the memory range of
     * m_superBlock length m_superBlockLength. return false otherwise.
     *
     * prefixLength - The number of bytes which must exist before 'addr'
     * postfixLength - The number of bytes which must exist after 'addr'
     */
    bool isInBoundries(void* addr,
                       uint prefixLength,
                       uint postfixLength);

    // The super-block memory
    void* m_superBlock;
    // The size of the super-block. Filled by the constructor.
    uint m_superBlockLength;

    // The lockable object over the super-block fragmentation
    mutable MemoryLockableObject m_lock;
    // Cached value for total number of allocated bytes in the super-block
    uint m_allocatedBytes;
};

#endif // __TBA_XDK_MEMORY_MEMORYSUPERBLOCKHEAPMANAGER_H
