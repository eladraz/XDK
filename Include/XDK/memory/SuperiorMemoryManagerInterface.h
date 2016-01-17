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

#ifndef __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGERINTERFACE_H
#define __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGERINTERFACE_H

/*
 * SuperiorMemoryManagerInterface.h
 *
 * This interface allow connection between the SuperiorMemoryManager and the
 * target operating system by implementing a simple interface for superblock
 * allocation.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"

/*
 * The interface is implemented by the operating system and transfer to the
 * SuperiorMemoryManager by calling the 'manageMemory()' function. In this
 * function the memory-manager examine the current memory state and by the
 * allocated statics of the runtime enviroment decide whether more
 * superblocks are requiered.
 *
 * See SuperiorMemoryManager.
 */
class SuperiorMemoryManagerInterface {
public:
    // Virtual Destructor. You can inherit from me
    virtual ~SuperiorMemoryManagerInterface() {};

    /*
     * Return the default best page-alignment for the super-block
     */
    virtual uint getSuperblockPageAlignment() = 0;

    /*
     * Called by the SuperiorMemoryManager in order to expand the
     *
     * length - The length of the requested superblock. Align to
     *          'getSuperblockPageAlignment'
     *
     * Return NULL if no more memory is avaliable. In that case the
     * SuperiorMemoryManager will try to mange memory with the memory already
     * allocated, but the system will suffer from major performance penalties.
     * Return the memory reference of the new-superblock. This pointer must be
     * valid for use at all time, and cannot be paged-out.
     *
     * NOTE: It's gurentee that no mutable are kept lock by the
     *       SuperiorMemoryManager while this function is invoked
     */
    virtual void* allocateNewSuperblock(uint length) = 0;

    /*
     * Free a superblock
     *
     * pointer - the pointer retrieve from the 'allocateNewSuperblock'
     *
     * NOTE: It's gurentee that no mutable are kept lock by the
     *       SuperiorMemoryManager while this function is invoked
     */
    virtual void freeSuperblock(void* pointer) = 0;

    // TODO! Add more functions to save the statistic's repository
};

// A reference counter object. The name is shorten in order to clarifies codes
typedef cSmartPtr<SuperiorMemoryManagerInterface>
    SuperiorOSMemePtr;

#endif // __TBA_XDK_MEMORY_SUPERIORMEMORYMANAGERINTERFACE_H
