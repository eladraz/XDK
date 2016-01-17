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
 * SmallMemoryHeapManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/except/assert.h"
#include "xStl/except/trace.h"
#include "xdk/memory/SmallMemoryHeapManager.h"

SmallMemoryHeapManager::SmallMemoryHeapManager(void* superBlock,
                                               uint superBlockLength,
                                               uint allocationUnit) :
    MemorySuperblockHeapManager(superBlock,
                                superBlockLength),
    m_allocationUnit(allocationUnit),
    m_maxAllocationUnit((uint64)(allocationUnit) * 65535)
{
    // Some assertion for binary compatability
    ASSERT(sizeof(AllocatedDescriptorBlock) == MINIMUM_ALLOCATION_UNIT);
    ASSERT(sizeof(FreeBlock) == MINIMUM_ALLOCATION_UNIT);

    // Test the allocation unit
    CHECK(m_allocationUnit >= MINIMUM_ALLOCATION_UNIT);

    // The total number of block is the lower-bound of blocks which can be
    // fit into 'superBlockLength'
    m_totalNumberOfBlocks = superBlockLength  / m_allocationUnit;

    // Start by reset the memory. Set all memory to be free blocks
    // The first block will point to the last block and vis versa
    for (uint32 i = 0; i < m_totalNumberOfBlocks; i++)
        *(getFreeBlock(i)) = FreeBlock(i + 1);

    // The last block will be pointed out to 'm_totalNumberOfBlocks' which
    // means, no more free blocks. end of list

    m_firstFreeBlock = 0;
}

void* SmallMemoryHeapManager::allocate(uint length)
{
    // Cannot allocate more then MAX_ALLOCATED_MEMORY
    if ((length >= m_maxAllocationUnit) ||
        (length >= m_superBlockLength) ||
        // Should I return a valid pointer?!
        (length == 0))
        return NULL;

    // Align (truncate-up) the length to m_allocationUnit and append the
    // size of the AllocatedDescriptorBlock.
    uint16 numberOfBlocks = (uint16)((length + sizeof(AllocatedDescriptorBlock) +
                                     m_allocationUnit - 1) /
                                     m_allocationUnit);

    cLock lock(m_lock);

    // Try to find 'numberOfBlocks' contiguous blocks of data
    // NOTE: The search algorithm is very slow. We are depending that the
    //       SuperiorMemoryManager will manager the allocation without
    //       fragmentation so that the first-free block is actually a pointer
    //       into a valid block
    uint32 startBlockID = m_firstFreeBlock;
    bool found = false;

    // Check for no more memory
    if (m_firstFreeBlock == m_totalNumberOfBlocks)
        return NULL;

    // Try to discover the
    uint32 previousFreeBlockID = startBlockID;
    // Try to find a block
    while (!found)
    {
        // Assume OK
        found = true;

        // NOTE: We are trying to allocated 'numberOfBlocks' contiguous blocks,
        //       that means that only numberOfBlocks blocks should have the
        //       next pointer of them follow the previous one.
        for (uint16 i = 0; i < (numberOfBlocks - 1); i++)
        {
            if (getFreeBlock(startBlockID + i)->m_nextFreeBlock !=
                (startBlockID + i + 1))
            {
                previousFreeBlockID = startBlockID;
                startBlockID = getFreeBlock(startBlockID)->m_nextFreeBlock;
                found = false;

                // Check for no more memory
                if (startBlockID == m_totalNumberOfBlocks)
                    return NULL;

                // No more scanning needed
                // TODO! Add statics, mismatch
                break;
            }
        }
    }

    // First cache this block
    uint32 nextFreeBlockID =
        getFreeBlock(startBlockID + numberOfBlocks - 1)->m_nextFreeBlock;

    // Found! Allocate this block
    AllocatedDescriptorBlock* ac = getAllocatedBlock(startBlockID);
    *ac = AllocatedDescriptorBlock(numberOfBlocks);

    // Increase the first pointer was allocated.
    if (previousFreeBlockID == startBlockID)
    {
        ASSERT(previousFreeBlockID == m_firstFreeBlock);
        m_firstFreeBlock = nextFreeBlockID;
    } else
    {
        getFreeBlock(previousFreeBlockID)->m_nextFreeBlock = nextFreeBlockID;
    }

    // There are numberOfBlocks allocation blocks
    m_allocatedBytes+= numberOfBlocks * m_allocationUnit;

    return (void*)(ac + 1);
}

bool SmallMemoryHeapManager::free(void* buffer)
{
    // First check the boundries of the buffer
    if (!isInBoundries(buffer, sizeof(AllocatedDescriptorBlock),
                               m_allocationUnit - ALLOCATED_UNIT_OVERHEAD))
    {
        return false;
    }

    // Get the allocation block
    AllocatedDescriptorBlock* block = getAllocatedDescriptorBlock(buffer);
    uint32 thisBlockID = (getNumeric(block) - getNumeric(m_superBlock)) /
                          m_allocationUnit;
    uint16 count = block->m_numberOfBlocks;

    // Test that all blocks are fitted
    if ((thisBlockID + count) > m_totalNumberOfBlocks)
    {
        return false;
    }

    // Test the magic
    if (block->m_magic != ALLOCATED_DESCRIPTOR_MAGIC)
    {
        return false;
    }

    // All operation for now on require a protection
    cLock lock(m_lock);

    // Unchain the blocks
    for (uint16 i = 0; i < (count - 1); i++)
        *(getFreeBlock(thisBlockID + i)) = FreeBlock(thisBlockID + i + 1);
    getFreeBlock(thisBlockID + count - 1)->m_nextFreeBlock = m_firstFreeBlock;
    m_firstFreeBlock = thisBlockID;

    // TODO! Defragment!

    // The number of free allocate blocks are the 'allocated-descriptor' and
    // the 'count' number of blocks
    m_allocatedBytes-= count * m_allocationUnit;

    return true;
}

uint SmallMemoryHeapManager::getMaximumAllocationUnit() const
{
    if (m_maxAllocationUnit > 0xFFFFFFFF)
        return 0xFFFFFFFF;

    return (uint32)(m_maxAllocationUnit);
}

uint SmallMemoryHeapManager::getMinimumAllocationUnit() const
{
    return m_allocationUnit;
}

//////////////////////////////////////////////////////////////////////////
SmallMemoryHeapManager::AllocatedDescriptorBlock::AllocatedDescriptorBlock(
        uint16 blocksCount) :
    m_numberOfBlocks(blocksCount),
    m_magic((uint16)ALLOCATED_DESCRIPTOR_MAGIC)
{
}

SmallMemoryHeapManager::FreeBlock::FreeBlock(uint16 nextFreeBlock) :
    m_nextFreeBlock(nextFreeBlock)
{
}
