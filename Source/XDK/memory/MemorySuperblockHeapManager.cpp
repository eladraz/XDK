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
 * MemorySuperblockHeapManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/memory/MemorySuperblockHeapManager.h"

MemorySuperblockHeapManager::MemorySuperblockHeapManager(void* superBlock,
                                                         uint superBlockLength) :
    m_superBlock(superBlock),
    m_superBlockLength(superBlockLength),
    m_allocatedBytes(0)
{
}

MemorySuperblockHeapManager::~MemorySuperblockHeapManager()
{
}

uint MemorySuperblockHeapManager::getNumberOfAllocatedBytes() const
{
    return m_allocatedBytes;
}

uint MemorySuperblockHeapManager::getNumberOfFreeBytes() const
{
    return m_superBlockLength - m_allocatedBytes;
}

bool MemorySuperblockHeapManager::isInBoundries(void* addr,
                                                uint prefixLength,
                                                uint postfixLength)
{
    addressNumericValue naddr = getNumeric(addr);
    addressNumericValue saddr = getNumeric(m_superBlock);

    return (((naddr - prefixLength) >= saddr)) &&
            (((naddr + postfixLength) <= (saddr + m_superBlockLength)));
}
