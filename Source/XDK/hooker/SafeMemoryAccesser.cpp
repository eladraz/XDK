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
 * SafeMemoryAccesser.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/except/trace.h"
#include "xdk/hooker/MemoryManager.h"
#include "xdk/hooker/SafeMemoryAccesser.h"

SafeMemoryAccesser::SafeMemoryAccesser(addressNumericValue startAddress,
                                       addressNumericValue endAddress) :
    m_startAddress(startAddress),
    m_endAddress(endAddress),
    m_isVirtualRelocation(false)
{
}

SafeMemoryAccesser::SafeMemoryAccesser(addressNumericValue startAddress,
                                       addressNumericValue endAddress,
                                       addressNumericValue virtualBase) :
    m_startAddress(startAddress),
    m_endAddress(endAddress),
    m_isVirtualRelocation(true),
    m_virtualBase(virtualBase)
{
}


bool SafeMemoryAccesser::memread(addressNumericValue address,
                                 void* buffer,
                                 uint length,
                                 cFragmentsDescriptor* fragments) const
{
    if (length == 0)
        return true;

    // Check out-of-range exception
    getAddress(address + length - 1);

    // Get the first address
    addressNumericValue addr = getAddress(address);

    // Start copying page-by page
    if (MemoryManager::chkMemPresent(getPtr(addr), length))
    {
        // Easy implementation...
        cOS::memcpy(buffer, getPtr(address), length);
        return true;
    }

    // Some more complex implementation. Read only the present pages
    uint pageSize = MemoryManager::getPageSize(addr);
    addressNumericValue page = MemoryManager::alignToStartPage(addr);
    // Get the page boundries
    uint8* data = (uint8*)buffer;
    uint left = (page + pageSize) - addr;
    left = t_min(left, length);
    // Check for internal errors
    addressNumericValue endAddress = addr + length;

    bool isFragment = false;
    addressNumericValue startFragment = 0;

    do
    {
        // Copy memory
        if (MemoryManager::isPagePresent(page))
        {
            if (isFragment && (fragments != NULL))
            {
                // NOTE: The driver works with a single address convenstion!
                fragments->appendFragmentBlock(startFragment,
                                               addr - 1);
                isFragment = false;
            }
            cOS::memcpy(data, getPtr(addr), left);
        }
        else
        {
            // Eye-catcher
            startFragment = addr;
            isFragment = true;
            memset(data, PAGEOUT_DEFAULT_BYTE_VALUE, left);
        }

        // Add indexes
        data+= left;
        addr+= left;
        page+= pageSize;
        pageSize = MemoryManager::getPageSize(page);
        left = t_min(pageSize, endAddress - addr);
    } while (left > 0);

    // Add the last fragmentation
    if (isFragment && (fragments != NULL))
        // NOTE: The driver works with a single address convenstion!
        fragments->appendFragmentBlock(startFragment,
                                       endAddress);

    return false;
}

bool SafeMemoryAccesser::write(addressNumericValue address,
                               const void* buffer,
                               uint length)
{
    if (length == 0)
        return true;

    // Check out-of-range exception
    getAddress(address + length - 1);
    // Assume page-fault handler will raise exception: STATUS_ACCESS_VIOLATION
    // TODO!
    cOS::memcpy(getPtr(getAddress(address)), buffer, length);
    return true;
}

bool SafeMemoryAccesser::isWritableInterface() const
{
    return true;
}

addressNumericValue SafeMemoryAccesser::getAddress(addressNumericValue address)
                                                                           const
{
    if (m_isVirtualRelocation)
    {
        CHECK(address >= m_virtualBase);
        addressNumericValue ret = address - m_virtualBase;
        CHECK(((uint64)ret + m_startAddress) < MAX_PHYSICAL_ADDRESS);
        ret+= m_startAddress;
        CHECK(ret <= m_endAddress);
        return ret;
    }

    // Test for overflow
    CHECK(((uint64)address + m_startAddress) < MAX_PHYSICAL_ADDRESS);
    addressNumericValue ret = address + m_startAddress;
    CHECK(ret <= m_endAddress);
    return ret;
}
