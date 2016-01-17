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
 * StackObject.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/stream/traceStream.h"
#include "xdk/hooker/StackObject.h"

StackObject::StackObject(uint stackSize /* = DefaultStackSize  */) :
    m_stack(stackSize)
{
}

static void* __stdcall executeCallbackerFunction(cCallback* callback,
                                                 void* parameter,
                                                 bool* isException)
{
    XSTL_TRY
    {
        *isException = false;
        return callback->call(parameter);
    } XSTL_CATCH_ALL {
        traceHigh("StackObject: Unknown exception throw!" << endl);
        *isException = true;
        return NULL;
    }
}

void* StackObject::spawnNewStackJob(cCallbackPtr function, void* parameter)
{
    // The function pointer
    cCallback* callback = function.getPointer();
    // The stack-base
    addressNumericValue* newStack =
        (addressNumericValue*)(m_stack.getBuffer() + m_stack.getSize()) - 1;
    // The stack limits
    uint8* stackLimit = m_stack.getBuffer();
    // Set to true if the function throws unknown exception
    bool isException = false;
    // The returned value
    void* returnedValue = &isException;

    /*
     * BIG TODO!
     *    There is a small bug here, if a context switch occured or even if some
     *    operating system API called, the NT_TIB might revert into the Threads
     *    list NT_TIB.
     *    Also replace the Thread->Self->NT_TIB information as well!
     */

    // Windows NT/2000/XP i386 code.
    _asm {
        // Reset the exception handler list
        mov eax, fs:[0]
        push eax
        mov eax, 0FFFFFFFFh
        mov fs:[0], eax
        ; // Replace the NT_TIB (Thread Information Block) to the current stack
        mov eax, fs:[4] // StackBase
        push eax
        mov eax, fs:[8] // StackLimit
        push eax
        mov eax, newStack
        mov fs:[4], eax
        mov eax, stackLimit
        mov fs:[8], eax

        ; // Save the old pointers
        mov eax, esp
        ; // Switch stacks
        mov ebx, callback
        mov ecx, parameter
        mov edi, returnedValue
        mov esp, newStack
        ; // Push the old esp
        push eax
        ; // Execute the function
        push edi
        push ecx
        push ebx
        call executeCallbackerFunction
        ; // Return the stack to it's normal attributes
        pop esp
        mov returnedValue, eax
        ; // Restore the NT_TIB
        pop eax
        mov fs:[8], eax  ; // StackLimit
        pop eax
        mov fs:[4], eax  ; // StackBase
        ; // Restore the exception handling list
        pop eax
        mov fs:[0], eax
    }

    // Throw exception
    if (isException)
        CHECK_FAIL();

    // Return the function code
    return returnedValue;
}

