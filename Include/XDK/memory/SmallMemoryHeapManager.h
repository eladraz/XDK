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

#ifndef __TBA_XDK_MEMORY_SMALLMEMORYHEAPMANAGER_H
#define __TBA_XDK_MEMORY_SMALLMEMORYHEAPMANAGER_H

/*
 * SmallMemoryHeapManager.h
 *
 * A simple implementation of MemorySuperblockHeapManager for small allocation
 * units.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/memory/MemorySuperblockHeapManager.h"

/*
 * Allocation of small memory blocks (from 1 byte to 32 bytes) can be
 * effectively manage by small overhead packets. This done by dividing
 * the block into 4 bytes sub-blocks. Each sub-block can be either free
 * allocated or allocate-descriptor block.
 *
 * Here is a small chart of empty superblock memory:
 *
 *   /---------------------------------------------------------------------\
 *   | Free  | Free  |  Free  |  Free  |  Free  |  Free  |  Free  |  Free  |
 *   \-N-------N---------N--------N-------N---------N-------N--------N-----/
 *     \-----/  \-----/  \-----/  \-----/  \-----/  \-----/  \-----/ \-||
 *
 * As you can see each free block points in the beginning to the next free
 * block.
 *
 * When a memory is allocated a pool of 1 or more contiguous blocks are united
 * for the allocated block and the one block previous for the contiguous block
 * will be used for the allocation-descriptor block. This special block contains
 * the information about the allocated block: Number of blocks and a magic
 * number.
 *
 * See 'FreeBlock' for the free block descriptor
 * See 'AllocatedDescriptorBlock' for the allocation-descriptor block
 *
 * The block are index by thier position in the chain. Block number 5 start
 * after 20 bytes from the beginning of the superblock.
 *
 * In order to reduce the over-head blocks, the number of blocks are limited
 * to 2^16, which limit the super-block size to 2^18 - 256kb of data.
 * Also, the maximum allocating unit is 1kb of data.
 *
 * Some more information about this kind of allocation:
 *    - The overhead for each packet is very small.
 *    - Allocation is very fast, the same for destruction.
 *
 * Fragmentation note:
 *      It's recommended to allocate similar block sizes to reduce fragmentation
 *      see how the SuperiorMemoryManager construct couple of small-memory heaps
 *      and play with them.
 *
 * NOTE: This class is thread-safe. See MemorySuperblockHeapManager::m_lock
 */
class SmallMemoryHeapManager : public MemorySuperblockHeapManager {
public:
    // The size in bytes of the
    enum { ALLOCATED_UNIT_OVERHEAD = 4 };

    /*
     * Constructor.
     * See MemorySuperblockHeapManager::MemorySuperblockHeapManager
     *
     * superBlock       - The super block memory
     * superBlockLength - The length of the super-block. Limited to
     *                    MAX_SUPERBLOCK_SIZE. Also the superBlockLength must
     *                    be bigger than 2 blocks
     * allocationUnit   - The allocation unit. Including 4 bytes of the
     *                    allocation header. See ALLOCATED_UNIT_OVERHEAD
     *
     * It's recommended to allocate (N_ELEMENTS*ELEMENT_SIZE) + BLOCK_SIZE.
     */
    SmallMemoryHeapManager(void* superBlock,
                           uint superBlockLength,
                           uint allocationUnit);

    /*
     * See MemorySuperblockHeapManager::allocate
     */
    virtual void* allocate(uint length);

    /*
     * See MemorySuperblockHeapManager::free
     */
    virtual bool free(void* buffer);


    /*
     * See MemorySuperblockHeapManager::getMaximumAllocationUnit.
     *
     * Return MAX_ALLOCATED_MEMORY
     */
    virtual uint getMaximumAllocationUnit() const;

    /*
     * See MemorySuperblockHeapManager::getMaximumAllocationUnit.
     *
     * Return the allocationUnit pass by the SuperiorMemoryManager in the
     * constructor
     */
    virtual uint getMinimumAllocationUnit() const;

private:
    // Deny copy-constructor and operator =
    SmallMemoryHeapManager(const SmallMemoryHeapManager& other);
    SmallMemoryHeapManager& operator = (const SmallMemoryHeapManager& other);

    // The minimum allocation unit is 4 bytes
    enum { MINIMUM_ALLOCATION_UNIT = 4 };

    // Convert index into FreeBlock*
    #define getFreeBlock(index) ((FreeBlock*)getPtr( \
        getNumeric(m_superBlock) + ((index) * m_allocationUnit)))
    // Convert index into AllocatedDescriptorBlock*
    #define getAllocatedBlock(index) ((AllocatedDescriptorBlock*)getPtr( \
        getNumeric(m_superBlock) + ((index) * m_allocationUnit)))

    // Convert a pointer base into AllocatedDescriptorBlock*
    #define getAllocatedDescriptorBlock(base) \
        ((AllocatedDescriptorBlock*)(base) - 1)


    // The magic for the allocated descriptor block
    enum { ALLOCATED_DESCRIPTOR_MAGIC = 0xBEEF };

    //////////////////////////////////////////////////////////////////////////
    // Private structs

    #pragma pack(push)
    #pragma pack(1)
    /*
     * An allocated descriptor block
     * Size: 4 bytes.
     */
    class AllocatedDescriptorBlock {
    public:
        // Default constructor
        AllocatedDescriptorBlock(uint16 blocksCount);

        // The magic. Used for test overrun by the previous block
        // See ALLOCATED_DESCRIPTOR_MAGIC
        uint16 m_magic;
        // The number of allocated blocks (Each block is m_allocationUnit bytes),
        // NOTE: including this one!
        uint16 m_numberOfBlocks;
    };

    /*
     * A free block
     * Packed size: 4 bytes.
     */
    class FreeBlock {
    public:
        // Default constructor
        FreeBlock(uint16 nextFreeBlock = 0);

        // Pointer to the next free. (Each block is 4 bytes)
        uint32 m_nextFreeBlock;
    };
    #pragma pack(pop)

    //////////////////////////////////////////////////////////////////////////
    // Members

    // The first free block pointer
    uint32 m_firstFreeBlock;
    // The total number of blocks
    uint m_totalNumberOfBlocks;
    // The allocation unit
    uint m_allocationUnit;
    // The maximum number of bytes which can be allocated.
    // Calculated as the 16bit * m_allocationUnit
    uint64 m_maxAllocationUnit;
};

#endif // __TBA_XDK_MEMORY_SMALLMEMORYHEAPMANAGER_H
