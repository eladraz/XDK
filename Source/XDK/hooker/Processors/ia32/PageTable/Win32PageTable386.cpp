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
 * Win32PageTable386.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/traceStream.h"
#include "xdk/hooker/Processors/ia32/PageTable/Win32PageTable386.h"

// Set for large kernel mm. NOTE: Should be move to dynamic cast
#define PTE_EXTENSTION // TODO: Find this out programmaticly

#ifdef PTE_EXTENSTION
    const uint32 Win32PageTable386::PagesVirtualAddress = 0xC0000000;           // *NEW PTE is 64 bit
    const uint32 Win32PageTable386::PagesDirectoryVirtualAddress = 0xC0600000;  // *NEW so address is 0xC0600000 and PDE size is 0x10 and not 0x4
    #define PTE_ENTRY_SIZE (0x8)
    #define PDE_ENTRY_SIZE (0x10)
#else
    const uint32 Win32PageTable386::PagesVirtualAddress = 0xC0000000;
    const uint32 Win32PageTable386::PagesDirectoryVirtualAddress = 0xC0300000;
    #define PTE_ENTRY_SIZE (0x4)
    #define PDE_ENTRY_SIZE (0x4)
#endif // PTE_EXTENSTION

GlobalPage* Win32PageTable386::m_pteArray =
    (GlobalPage*)getPtr(PagesVirtualAddress);

GlobalPage* Win32PageTable386::m_pdeArray =
    (GlobalPage*)getPtr(PagesDirectoryVirtualAddress);


GlobalPage* Win32PageTable386::getPageDirectory(void* linearAddress)
{
    uint32 address = getNumeric(linearAddress);
    uint pdeIndex = address >> 22;

    // return m_pdeArray + pdeIndex;
    return (GlobalPage*)getPtr((pdeIndex*PDE_ENTRY_SIZE)+PagesDirectoryVirtualAddress);
}

GlobalPage* Win32PageTable386::getPageTableEntry(void* linearAddress)
{
    uint32 address = getNumeric(linearAddress);
    uint pteIndex = address >> 12;

    //return m_pteArray + pteIndex;
    return (GlobalPage*)getPtr((pteIndex*PTE_ENTRY_SIZE)+PagesVirtualAddress);
}

GlobalPage* Win32PageTable386::getPageEntry(void* linearAddress)
{
    GlobalPage* pde = getPageDirectory(linearAddress);
    if (pde->m_present == 0) {
        return pde;
    }

    // Test for 4MB page
    if (pde->m_ptePAT_pdePageSize == 1) {
        return pde;
    }

    return getPageTableEntry(linearAddress);
}

#ifdef COMMON_DUMP
void Win32PageTable386::dumpPage(GlobalPage pageContent,
                                 cStringerStream& out)
{
    if (pageContent.m_present == 1)
    {
        // Page present
        if (pageContent.m_isWriteable == 1)
            out << "Read/Write ";
        else
            out << "Read only  ";
        if (pageContent.m_isUserAccess)
            out << "Ring3 ";
        else
            out << "Ring0 ";
        if (pageContent.m_ptePAT_pdePageSize == 1)
            out << "PS  ";
        else
            out << "    ";
        out << "PFN: " << HEXDWORD(pageContent.m_address);
    } else
    {
        out << "Not Present: " << HEXDWORD(getPageValue(pageContent) >> 1);
    }
}
#endif
