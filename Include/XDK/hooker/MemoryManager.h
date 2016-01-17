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

#ifndef __TBA_XDK_HOOKER_MEMORYMANAGER_H
#define __TBA_XDK_HOOKER_MEMORYMANAGER_H

/*
 * MemoryManager.h
 *
 * The memory manager provide a set of cross-platform API for memory
 * operation.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * The memory manager provide a set of cross-platform API for memory
 * operation.
 */
class MemoryManager {
public:
    /*
     * Test that a memory range starting from 'startAddress' with length bytes
     * is currently loaded inside the physical memory, and it's readable.
     *
     * startAddress - The starting address to test
     * length - The number of bytes to test
     *
     * Return true if the memory is present, otherwise, return false
     */
    static bool chkMemPresent(void* startAddress, uint length);

    /*
     * Change the attribute of a memory range and make it writable.
     *
     * startAddress - The starting address
     * length - The number of bytes
     *
     * Return true if the memory is present and the change has being made,
     * false otherwise.
     */
    static bool makeMemoryWriteable(void* startAddress, uint length);

    /*
     * Change the attribute of a single memory page.
     *
     * pageAddress - The address of the page
     * isWritable - true to make the page writable, false to make it read-only
     * originalIsWritable - Will contain the original page attribute
     *
     * Return true if the memory is present and the change was made,
     * false otherwise.
     *
     */
    static bool changePageIsWritable(void* pageAddress, bool isWritable, bool& originalIsWritable);

    /*
     * Return true if the page is present and can be read from
     */
    static bool isPagePresent(addressNumericValue pageAlignAddress);

    /*
     * Return the page-size of the current memory-management module
     *
     * address - The address of the request page
     *
     * NOTE: If the page is not present return the size of the page which
     *       would have fit into the location
     */
    static uint getPageSize(addressNumericValue address);

    /*
     * Align the 'address' to it's starting page address
     */
    static addressNumericValue alignToStartPage(addressNumericValue address);
};

#endif // __TBA_XDK_HOOKER_MEMORYMANAGER_H
