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

#ifndef __TBA_XDK_HOOKER_IA32_PAGETABLE_WIN32PAGETABLE386_H
#define __TBA_XDK_HOOKER_IA32_PAGETABLE_WIN32PAGETABLE386_H

/*
 * Win32PageTable386.h
 *
 * Functions which are used inside Windows 32bit operating system and supports
 * page-table operation.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/stringerStream.h"
#include "xdk/hooker/Processors/ia32/PageTable/PageTable386.h"

/*
 * Windows saves the current memory map (e.g. CR3 pyshical memory mapping) into
 * address 0xC0000000. This core module extents the normal operating behaviour
 * by allowing ring0 utilities to manage this core area.
 *
 * NOTE: Use extreme cautions when manipulates this data.
 */
class Win32PageTable386 {
public:
    /*
     * Return the page-directory entry reference
     */
    static GlobalPage* getPageDirectory(void* linearAddress);

    /*
     * Reads the PDE and try to access to the PTE for the linear-address.
     *
     * Throw exception if the page is 4MB page.
     * Note: To access the attributes of a linear-address use the getPageEntry()
     *       function.
     */
    static GlobalPage* getPageTableEntry(void* linearAddress);

    /*
     * Return the page descriptor which represent the 'linearAddress'. This page
     * can be either PTE or PDE.
     */
    static GlobalPage* getPageEntry(void* linearAddress);

    #ifdef COMMON_DUMP
    /*
     * Dump a page to a human readable stream.
     *
     * pageContent - The content of the page
     * out         - The stream to write the data to.
     */
    static void dumpPage(GlobalPage pageContent,
                         cStringerStream& out);
    #endif

private:
    // The virtual address for the page-
    static const uint32 PagesVirtualAddress;
    static const uint32 PagesDirectoryVirtualAddress;

    // The array of all PDE's
    static GlobalPage* m_pdeArray;

    // The array of all PTE's
    static GlobalPage* m_pteArray;
};

#endif // __TBA_XDK_HOOKER_IA32_PAGETABLE_WIN32PAGETABLE386_H
