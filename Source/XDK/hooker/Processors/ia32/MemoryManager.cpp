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
 * MemoryManager.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/hooker/MemoryManager.h"
#include "xdk/hooker/Processors/ia32/PageTable/PageTable386.h"
#include "xdk/hooker/Processors/ia32/PageTable/Win32PageTable386.h"


// TODO!
// For Windows XP+ the page-tables might be swap out
// For each 'getPageDirectory' check the returned pointer

bool MemoryManager::chkMemPresent(void* startAddress, uint length)
{
    uint8* address = (uint8*)startAddress;

    while (length > 0)
    {
        GlobalPage* page = Win32PageTable386::getPageDirectory(address);
        uint thisPageLength;

        if (page->m_present == 0)
            return false;

        if (page->m_ptePAT_pdePageSize == 1)
        {
            // 4Mb page
            thisPageLength =
                IA32_LARGEPAGE_SIZE - (getNumeric(address) % IA32_LARGEPAGE_SIZE);
        } else
        {
            page = Win32PageTable386::getPageTableEntry(address);
            if (page->m_present == 0)
                return false;
            // 4kb page
            thisPageLength =
                IA32_PAGE_SIZE - (getNumeric(address) % IA32_PAGE_SIZE);
        }

        // Test the left bytes
        if (length <= thisPageLength)
            return true;

        length-= thisPageLength;
        address+= thisPageLength;
    }

    return true;
}

bool MemoryManager::makeMemoryWriteable(void* startAddress, uint length)
{
    uint8* address = (uint8*)startAddress;

    while (length > 0)
    {
        GlobalPage* page = Win32PageTable386::getPageDirectory(address);
        uint thisPageLength;

        if (page->m_present == 0)
            return false;

        if (page->m_ptePAT_pdePageSize == 1)
        {
            // 4Mb page
            thisPageLength =
                IA32_LARGEPAGE_SIZE - (getNumeric(address) % IA32_LARGEPAGE_SIZE);
        } else
        {
            page = Win32PageTable386::getPageTableEntry(address);
            if (page->m_present == 0)
                return false;
            // 4kb page
            thisPageLength =
                IA32_PAGE_SIZE - (getNumeric(address) % IA32_PAGE_SIZE);
        }

        // Change the attribute to write page...
        page->m_isWriteable = 1;

        // Flush the TLB
        _asm {
            mov eax, address
            invlpg [eax]
        }

        // Test the left bytes
        if (length <= thisPageLength)
            return true;

        length-= thisPageLength;
        address+= thisPageLength;
    }

    return true;
}

bool MemoryManager::changePageIsWritable(void* pageAddress, bool isWritable, bool& originalIsWritable)
{
    uint8* address = (uint8*)pageAddress;

    GlobalPage* page = Win32PageTable386::getPageDirectory(address);
    uint thisPageLength;

    if (page->m_present == 0)
        return false;

    if (page->m_ptePAT_pdePageSize == 1)
    {
        // 4Mb page
        thisPageLength =
            IA32_LARGEPAGE_SIZE - (getNumeric(address) % IA32_LARGEPAGE_SIZE);
    } else
    {
        page = Win32PageTable386::getPageTableEntry(address);
        if (page->m_present == 0)
            return false;
        // 4kb page
        thisPageLength =
            IA32_PAGE_SIZE - (getNumeric(address) % IA32_PAGE_SIZE);
    }

    // Keep the original attributes
    if (1 == page->m_isWriteable)
        originalIsWritable = true;
    else
        originalIsWritable = false;

    // Change the attribute according to parameter
    if (isWritable)
        page->m_isWriteable = 1;
    else
        page->m_isWriteable = 0;

    // Flush the TLB
    _asm {
        mov eax, address
        invlpg [eax]
    }

    return true;
}

bool MemoryManager::isPagePresent(addressNumericValue pageAlignAddress)
{
    return (Win32PageTable386::getPageEntry(getPtr(pageAlignAddress))->m_present
            == 1);
}

uint MemoryManager::getPageSize(addressNumericValue address)
{
    GlobalPage* page = Win32PageTable386::getPageDirectory(getPtr(address));

    if (page->m_present == 0)
        // PDE doesn't exist. Assume 4mb page
        return IA32_LARGEPAGE_SIZE;

    if (page->m_ptePAT_pdePageSize == 1)
    {
        // 4Mb page
        return IA32_LARGEPAGE_SIZE;
    } else
    {
        return IA32_PAGE_SIZE;
    }

    return PAGE_SIZE;
}

addressNumericValue MemoryManager::alignToStartPage(addressNumericValue address)
{
    // Since the page-size is measure by power of 2 we can forget about modulue
    // and uses the binary functions.
    uint pageSize = getPageSize(address);
    return (address & (~(pageSize - 1)));
}
