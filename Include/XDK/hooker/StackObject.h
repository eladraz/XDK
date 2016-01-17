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

#ifndef __TBA_XDK_HOOKER_STACKOBJECT_H
#define __TBA_XDK_HOOKER_STACKOBJECT_H

/*
 * StackObject.h
 *
 * A new generated stack API
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/array.h"
#include "xStl/utils/callbacker.h"

/*
 * TODO! This code should only be executed from HIGH_LEVEL irql, otherwise
 *       the NT_TIB can context-switch into illegal value.
 */
class StackObject {
public:
    // The default generated stack is 32kb stack
    enum { DefaultStackSize = 32*1024 };

    /*
     * Default constructor
     */
    StackObject(uint stackSize = DefaultStackSize );

    /*
     * Changes the stack pointer to the new allocated stack.
     *
     * function - The function to be executed
     * parameter - The parameter to send
     *
     * Return the function return code.
     * NOTE: In case the function throw uncaught exception, An exception-failed
     *       exception will be sent to the main thread.
     */
    void* spawnNewStackJob(cCallbackPtr function, void* parameter);

private:
    // The stack object
    cBuffer m_stack;
};

#endif // __TBA_XDK_HOOKER_STACKOBJECT_H
