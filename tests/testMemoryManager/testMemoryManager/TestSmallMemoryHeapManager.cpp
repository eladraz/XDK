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
 * TestSmallMemoryHeapManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/char.h"
#include "xStl/except/trace.h"
#include "xStl/stream/iostream.h"
#include "xStl/except/exception.h"
#include "xdk/memory/SmallMemoryHeapManager.h"
#include "TestSuperBlock.h"

void simpleOverrunTest()
{
    uint superblockLength = 16; // Allocate 2 units
    uint8* superblockBuffer = new uint8[superblockLength];

    // Contains only 2 units
    SmallMemoryHeapManager newManager(superblockBuffer,
                                      superblockLength,
                                      8);

    CHECK(newManager.allocate(0) == NULL);

    void* x = newManager.allocate(12); CHECK(x != NULL);
    CHECK(newManager.allocate(1) == NULL);
    CHECK(newManager.free(x));

    x = newManager.allocate(4);   CHECK(x != NULL);
    void* y = newManager.allocate(4);   CHECK(y != NULL);
    CHECK(newManager.allocate(1) == NULL);
    CHECK(newManager.allocate(0) == NULL);
    CHECK(newManager.free(x));
    x = newManager.allocate(1); CHECK(x != NULL);
    CHECK(newManager.allocate(4) == NULL);
    CHECK(newManager.free(x));
    CHECK(newManager.free(y));

    CHECK(newManager.allocate(17) == NULL);
    CHECK(newManager.allocate(16) == NULL);

    // Allocating two blocks here will decline since the memory is defragment.
    // TODO for future works
}

void simpleRandomTest()
{
    uint superblockLength = 5*1024*1024; // Allocate 5mb
    uint8* superblockBuffer = new uint8[superblockLength];

    SmallMemoryHeapManager newManager(superblockBuffer,
        superblockLength,
        16);
    TestSuperBlock test(newManager, cout, 8, 12);
    test.test();

    // And delete the block
    delete[] superblockBuffer;
}

void testSmallMemoryHeapManager()
{
    simpleOverrunTest();
    simpleRandomTest();
}
