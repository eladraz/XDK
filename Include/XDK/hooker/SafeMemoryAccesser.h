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

#ifndef __TBA_XDK_HOOKER_SAFEMEMORYACCESSER_H
#define __TBA_XDK_HOOKER_SAFEMEMORYACCESSER_H

/*
 * SafeMemoryAccesser.h
 *
 * Implementation of cVirtualMemoryAccesser interface over secure Blue suite
 * Memory-manager protocol
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/fragmentsDescriptor.h"
#include "xStl/os/virtualMemoryAccesser.h"

/*
 * See cVirtualMemoryAccesser. Uses the MemoryManager as an anchor for all
 * memory accesses.
 *
 * Implementation based over cThreadUnsafeMemoryAccesser.
 */
class SafeMemoryAccesser : public cVirtualMemoryAccesser {
public:
    // Use the default implementation
    defaultEndianImpl;

    /*
     * Default constructor. Allow the application to block unneeded access to
     * different memory blocks.
     * When supplies different value for 'start'..'end' than the memory location
     * 0 will be the 'start' and 'end-start' will be the end memory.
     *
     * startAddress - The starting physical address that can be accessed
     * endAddress   - The ending physical address that can be accessed
     *
     * The thread will throw exception for any out-range access.
     */
    SafeMemoryAccesser(addressNumericValue startAddress = 0,
                       addressNumericValue endAddress = MAX_PHYSICAL_ADDRESS);
    /*
     * See cThreadUnsafeMemoryAccesser::cThreadUnsafeMemoryAccesser.
     *
     * Change the range [start..end] to [virtual..virtual+size], however
     * when accessing 'virtual', the address will translated to 'start'
     */
    SafeMemoryAccesser(addressNumericValue startAddress,
                       addressNumericValue endAddress,
                       addressNumericValue virtualBase);

    /*
     * Normal implementation. See VirtualMemoryAccesser::memread
     *
     * Uses fragmentation as MemoryManager provides
     */
    virtual bool memread(addressNumericValue address,
                         void* buffer,
                         uint length,
                         cFragmentsDescriptor* fragments = NULL) const;

    /*
     * Normal implementation. See VirtualMemoryAccesser::write
     *
     * Uses fragmentation as MemoryManager provides
     */
    virtual bool write(addressNumericValue address,
                       const void* buffer,
                       uint length);

    /*
     * Normal implementation. See VirtualMemoryAccesser::isWritableInterface
     * Return true.
     */
    virtual bool isWritableInterface() const;

    // The default return number
    enum { PAGEOUT_DEFAULT_BYTE_VALUE = 0xCD };

protected:
    /*
     * Perform relocation over 'address'.
     * Throw exception if the memory address is outside the context
     */
    addressNumericValue getAddress(addressNumericValue address) const;

    // The starting physical address that can be accessed
    addressNumericValue m_startAddress;
    // The ending physical address that can be accessed
    addressNumericValue m_endAddress;
    // The virtual name of 'startAddress'
    bool m_isVirtualRelocation;
    addressNumericValue m_virtualBase;
};

#endif // __TBA_XDK_HOOKER_SAFEMEMORYACCESSER_H
