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
 * ehlibpp.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */

#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/except/assert.h"
#include "XDK/EHLib/ehlib.h"
#include "XDK/EHLib/ehlibcpp.h"


void __cdecl stackFaild()
{
#ifndef EHLIB_STATIC
    TRACE(TRACE_VERY_HIGH, XSTL_STRING("Stack failed!"));
#endif // EHLIB_STATIC
    // Force a breakpoint
    #ifdef _KERNEL
        KdBreakPoint();
    #else
        FatalExit(EHLib::EHLIB_EXCEPTION_CODE);
    #endif
    // Throw exception. Revert the stack to it's original state.
    XSTL_THROW(0);
}

EXTERNC __declspec(naked) void __cdecl _chkesp()
{
    _asm {
        jnz  error
        retn
    error:
        call stackFaild
        retn
    }
}

EXTERNC __declspec(naked) void __cdecl _EH_prolog()
{
    _asm {
        push    0FFFFFFFFh                ; // Try level.
        push    eax                       ; // Exception handling vector
        mov     eax, dword ptr fs:[0]
        push    eax                       ; // Old stack handler
        mov     eax, [esp+0Ch]
        mov     dword ptr fs:[0], esp     ; // Frame handler
        mov     [esp+0Ch], ebp
        lea     ebp, [esp+0Ch]
        push    eax
        retn
    }
}

/*
 * This function is in remark since the kernel stack is limited in bytes.
 * See ehlibcpp.h for more information
 *
EXTERNC __declspec(naked) void __cdecl _chkstk()
{
_asm {
push    ecx
cmp     eax, 1000h
lea     ecx, [esp+8]
jb      end

next:
sub     ecx, 1000h
sub     eax, 1000h
test    [ecx], eax
cmp     eax, 1000h
jnb     next

end:
sub     ecx, eax
mov     eax, esp
test    [ecx], eax
mov     esp, ecx
mov     ecx, [eax]
mov     eax, [eax+4]
push    eax
retn
}
*/
