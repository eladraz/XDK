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

#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/except/trace.h"
#include "xStl/stream/iostream.h"
#include "xdk/kernel.h"
#include "xdk/memory.h"

/*
 * The singleton initialer class.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
class XdkMemoryTestSingleton {
public:
    static uint getManagerMemoryLength()
    {
        return (cXdkDriverMemoryManager::XDM_LENGTH * 8) / 10;
    }
};

#define SIM_CACHE (10)
#define MAX_POINTERS (8192)
// Set in order to overwrite internal allocated blocks
#define TEST_INTERNAL_BLOCKS
// Set in order to test how fast is operator new/delete
#define FAST_ALLOCATE_DELETE_TEST

// An information block
struct Data {
    uint8* ptr;
    uint size;
};


/*
 * Start the testing
 */
int main(int argc, const char** argv)
{
    uint i;
    uint totalAllocated = 0;
    uint8* pointers[MAX_POINTERS];
    for (i = 0; i < MAX_POINTERS; i++) { pointers[i] = NULL; }

    cOSDef::systemTime start = cOS::getSystemTime();

    #ifdef FAST_ALLOCATE_DELETE_TEST
    // Second test: How fast is operator new/delete...
    cList<Data> m_ptr;
    for (i = 0; i < 100000; i++)
    {
        if ((cOS::rand() & 1) == 1)
        {
            uint length = (cOS::rand() % MAX_POINTERS) + 1;
            length = t_min(length,
                           XdkMemoryTestSingleton::getManagerMemoryLength() -
                           totalAllocated);

            if (length != 0)
            {
                Data data;
                data.size = length;
                data.ptr = new uint8[length];
                if (data.ptr != NULL)
                {
                    m_ptr.append(data);
                    totalAllocated+= length;
                } else
                {
                    cout << "$";
                    cout.flush();
                }
            }
        }

        if ((cOS::rand() & 1) == 1)
        {
            if (m_ptr.begin() != m_ptr.end())
            {
                Data data = *m_ptr.begin();
                delete[] data.ptr;
                totalAllocated-= data.size;
                m_ptr.remove(m_ptr.begin());
            }
        }

        if ((i % 0x1FF) == 0)
        {
            cout << ".";
            cout.flush();
        }
    }
    #else

    // First test: Random allocation and filling...
    uint pointersLength[MAX_POINTERS];

    // The loop
    for (i = 0; i < 100000; i++)
    {
        if ((cOS::rand() & 1) == 1)
        {
            // Allocate
            uint ptr = 0;
            for (; ptr < MAX_POINTERS; ptr++)
            {
                if (pointers[ptr] == NULL)
                {
                    uint length = (cOS::rand() % MAX_POINTERS) + 1;
                    length = t_min(length,
                        XdkMemoryTestSingleton::getManagerMemoryLength() -
                        totalAllocated);
                    if (length == 0)
                        break;
                    totalAllocated+= length;
                    pointersLength[ptr] = length;
                    pointers[ptr] = new uint8[length];
                    #ifdef TEST_INTERNAL_BLOCKS
                    memset(pointers[ptr], (uint8)pointersLength[ptr], length);
                    #endif // TEST_INTERNAL_BLOCKS
                    break;
                }
            }
        } else
        {
            // Free
            for (uint k = 0; k < 10; k++)
            {
                uint ptr = cOS::rand() % MAX_POINTERS;
                if (pointers[ptr] != NULL)
                {
                    // Test memory
                    bool ok = true;
                    #ifdef TEST_INTERNAL_BLOCKS
                    for (uint x = 0; x < pointersLength[ptr]; x++)
                        ok = ok && (pointers[ptr][x] == (uint8)pointersLength[ptr]);
                    CHECK(ok);
                    #endif // TEST_INTERNAL_BLOCKS
                    // Free it
                    totalAllocated-= pointersLength[ptr];
                    delete[] pointers[ptr];
                    pointers[ptr] = NULL;
                    if ((cOS::rand() & 1) == 1)
                        break;
                }
            }
        }

        // Test
        #ifdef TEST_INTERNAL_BLOCKS
        if ((i & 0x1F) == 0x1F)
        {
            bool ok = true;
            for (uint j = 0; j < MAX_POINTERS; j++)
            {
                if (pointers[j] != NULL)
                    for (uint x = 0; x < pointersLength[j]; x++)
                        ok = ok && (pointers[j][x] == (uint8)pointersLength[j]);
            }
            CHECK(ok);
        }
        #endif // TEST_INTERNAL_BLOCKS

        if ((i % 0x1FF) == 0)
        {
            cout << ".";
            cout.flush();
        }
    }
    #endif // FAST_ALLOCATE_DELETE_TEST

    uint timePassed = cOS::calculateTimesDiffMilli(cOS::getSystemTime(),
                                                   start);

    cout << endl;
    cout << "Number of milliseconds pass: " << timePassed << endl;
    cout << "Seconds:                     " << timePassed / 1000 << endl;
    cout << "Minutes:                     " << timePassed / (1000 * 60) << endl;

	return 0;
}


/*
uint pointersStart[SIM_CACHE];
uint pointersLength[SIM_CACHE];
uint simPtr = 0;
for (i = 0; i < SIM_CACHE; i++)
pointersLength[i] = 0;
// Allocate
if (pointersLength[simPtr] == 0)
{
pointersLength[simPtr] = (cOS::rand() % (MAX_POINTERS >> 1)) + 1;
pointersStart[simPtr] =
for (uint j = 0; j < pointersLength[simPtr]; j++)
{
uint length = (cOS::rand() % MAX_POINTERS) + 1;
length = t_min(length,
XdkMemoryTestSingleton::getManagerMemoryLength() -
totalAllocated);
if (length == 0)
{
// No more memory
pointersLength[simPtr] = j;
break;
}
pointers[pointersStart[simPtr] + j] = new uint8[length];
totalAllocated+= length;
}
}
simPtr++;

if ((cOS::rand() & 1) == 1)
{
// Random free
uint loc = cOS::rand() % SIM_CACHE;
if (pointersLength[loc] != 0)
{
for (uint j = 0; j < pointersLength[loc]; j++)
{
delete[] pointers[pointersStart[loc] + j];
}
pointersLength[loc] = 0;
}
}
}
*/