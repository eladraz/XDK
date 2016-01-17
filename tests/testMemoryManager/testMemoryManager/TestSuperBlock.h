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

#ifndef __TBA_TESTMEMORY_TESTSUPERBLOCK_H
#define __TBA_TESTMEMORY_TESTSUPERBLOCK_H

/*
 * TestSuperBlock.h
 *
 * Uni-test for a super-block implementation
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/stringerStream.h"
#include "xdk/memory/MemorySuperblockHeapManager.h"

class TestSuperBlock {
public:
    // The default allocation for the test: 1mb of memory
    enum { MAX_ALLOCATION = (1 << 20) };

    /*
     * Constructor, test a memory-manager
     *
     * manager           - The manager to be tested
     * logStream         - The output logging stream
     * minimumToAllocate - The minimum number of bytes to allocate
     * maximumtoAllocate - The maximum number of bytes to allocate
     */
    TestSuperBlock(MemorySuperblockHeapManager& manager,
                   cStringerStream& logStream,
                   uint minimumToAllocate = 1,
                   uint maximumtoAllocate = MAX_ALLOCATION);

    /*
     * Invoke a single round of testing
     *
     * Throw exception if the test is failed.
     */
    void test();

private:
    // Deny operator = and copy-constructor
    TestSuperBlock(const TestSuperBlock& other);
    TestSuperBlock& operator = (const TestSuperBlock& other);

    // The number of rounds for the first test
    enum { FIRST_TEST_NUMBER_OF_ROUND = 100000 };
    // The number of simultation allocation and destruction
    enum { FIRST_TEST_NUMBER_OF_ALLOCATION = 512 };

    /*
     * Invoke the first test for the memory-manager:
     *
     * Random block allocation and filling. Testing that the block are attach
     * and everything goes well
     */
    void firstTest();

    // The allocation manager to be testsed
    MemorySuperblockHeapManager& m_manager;
    // The output stream
    cStringerStream& out;
    // The minimum number of bytes to allocate
    uint m_minimumToAllocate;
    // The maximum number of bytes to allocate
    uint m_maximumtoAllocate;
    // The freedom degree
    uint m_lengthDegree;
};

#endif // __TBA_TODO_TestSuperBlock_H
