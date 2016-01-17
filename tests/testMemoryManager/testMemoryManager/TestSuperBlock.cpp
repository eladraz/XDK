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
 * TestSuperBlock.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/except/trace.h"
#include "TestSuperBlock.h"

TestSuperBlock::TestSuperBlock(MemorySuperblockHeapManager& manager,
                               cStringerStream& logStream,
                               uint minimumToAllocate /* = 1 */,
                               uint maximumtoAllocate /* = MAX_ALLOCATION */) :
    out(logStream),
    m_manager(manager),
    m_minimumToAllocate(minimumToAllocate),
    m_maximumtoAllocate(maximumtoAllocate)
{
    CHECK(m_maximumtoAllocate >= m_minimumToAllocate);
    m_lengthDegree = m_maximumtoAllocate - m_minimumToAllocate;
    if (m_lengthDegree == 0)
        m_lengthDegree = 1;
}

void TestSuperBlock::test()
{
    firstTest();
}


void TestSuperBlock::firstTest()
{
    uint i;
    uint8* pointers[FIRST_TEST_NUMBER_OF_ALLOCATION];
    uint pointersLength[FIRST_TEST_NUMBER_OF_ALLOCATION];
    for (i = 0; i < FIRST_TEST_NUMBER_OF_ALLOCATION; i++) { pointers[i] = NULL; }

    // The loop
    for (i = 0; i < FIRST_TEST_NUMBER_OF_ROUND; i++)
    {
        if ((cOS::rand() & 1) == 1)
        {
            // Allocate
            uint ptr = 0;
            for (; ptr < FIRST_TEST_NUMBER_OF_ALLOCATION; ptr++)
            {
                if (pointers[ptr] == NULL)
                {
                    uint length = (cOS::rand() % m_lengthDegree) +
                                   m_minimumToAllocate;

                    pointersLength[ptr] = length;
                    pointers[ptr] = (uint8*)m_manager.allocate(length);
                    // Might be failed...
                    if (pointers[ptr] != NULL)
                    {
                        memset(pointers[ptr], (uint8)pointersLength[ptr], length);
                        break;
                    }
                }
            }
        } else
        {
            // Free
            for (uint k = 0; k < 10; k++)
            {
                uint ptr = cOS::rand() % FIRST_TEST_NUMBER_OF_ALLOCATION;

                if (pointers[ptr] != NULL)
                {
                    // Test memory
                    bool ok = true;
                    for (uint x = 0; x < pointersLength[ptr]; x++)
                        ok = ok && (pointers[ptr][x] == (uint8)pointersLength[ptr]);
                    CHECK(ok);
                    CHECK(m_manager.free(pointers[ptr]));
                    pointers[ptr] = NULL;
                    if ((cOS::rand() & 1) == 1)
                        break;
                }
            }
        }

        // Test
        if ((i & 0x1F) == 0x1F)
        {
            bool ok = true;
            for (uint j = 0; j < FIRST_TEST_NUMBER_OF_ALLOCATION; j++)
            {
                if (pointers[j] != NULL)
                    for (uint x = 0; x < pointersLength[j]; x++)
                        ok = ok && (pointers[j][x] == (uint8)pointersLength[j]);
            }
            CHECK(ok);
        }

        if ((i % 0x1FF) == 0)
        {
            out << ".";
            out.flush();
        }
    }
}
