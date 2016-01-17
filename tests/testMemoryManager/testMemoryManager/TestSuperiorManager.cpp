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
 * TestSuperiorManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/types.h"
#include "xStl/data/char.h"
#include "xStl/except/trace.h"
#include "xStl/stream/iostream.h"
#include "xStl/except/exception.h"
#include "xdk/memory/SuperiorMemoryManager.h"
#include "xdk/memory/SuperiorMemoryManagerInterface.h"
#include "TestSuperBlock.h"

//////////////////////////////////////////////////////////////////////////

class OSMem : public SuperiorMemoryManagerInterface {
public:
    virtual uint getSuperblockPageAlignment() {
        return 1;
    }

    virtual void* allocateNewSuperblock(uint length) {
        return new uint8[length];
    }

    virtual void freeSuperblock(void* pointer) {
        delete [] pointer;
    }
};

//////////////////////////////////////////////////////////////////////////

void test1()
{
    uint privatePoolLength =
        SuperiorMemoryManager::DEFAULT_SUPRIOR_MEMORY_PRIVATE_MEM;
    uint8* privatePool = new uint8[privatePoolLength];

    SuperiorMemoryManager* memmanager = new SuperiorMemoryManager(
        SuperiorOSMemePtr(new OSMem()),
        10*1024*1024,
        privatePool,
        privatePoolLength);

    TestSuperBlock test(*memmanager, cout, 1, 1024);
    test.test();


    // The destructor of SuperiorMemoryManager should be called before the
    // private memory pool will be free
    delete memmanager;
    delete[] privatePool;
}

void testMemoryExpander()
{
    uint privatePoolLength =
        SuperiorMemoryManager::DEFAULT_SUPRIOR_MEMORY_PRIVATE_MEM;
    uint8* privatePool = new uint8[privatePoolLength];

    SuperiorMemoryManager* memmanager = new SuperiorMemoryManager(
        SuperiorOSMemePtr(new OSMem()),
        SuperiorMemoryManager::INITIALIZE_SIZE_MINIMUM_SIZE,
        privatePool,
        privatePoolLength);

    // Allocate 4mb of data
    #define MAX_UNIT (1023)
    #define BLOCK_SIZE (4*1024)
    void* data[MAX_UNIT];
    uint i;
    for (i = 0; i < MAX_UNIT; i++) {
        data[i] = memmanager->allocate(BLOCK_SIZE);
        CHECK(data[i] != NULL);
    }

    // Try to play with the left memory
    for (i = 0; i < 100; i++)
    {
        CHECK(memmanager->allocate(BLOCK_SIZE) == NULL);
    }

    // Expand the superblocks
    memmanager->manageMemory();

    // And allocate some more data
    void* datax[MAX_UNIT];
    for (i = 0; i < MAX_UNIT; i++)
    {
        datax[i] = memmanager->allocate(BLOCK_SIZE);
        CHECK(datax[i] != NULL);
        memmanager->free(datax[i]);
    }

    // And free
    for (i = 0; i < MAX_UNIT; i++) {
        memmanager->free(data[i]);
    }

    // And free memory
    delete memmanager;
    delete[] privatePool;
}

//////////////////////////////////////////////////////////////////////////

void testSuperiorManager()
{
    test1();
    testMemoryExpander();
}

