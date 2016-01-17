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

#ifndef __TBA_XDK_HOOKER_IA32_TSS32_H
#define __TBA_XDK_HOOKER_IA32_TSS32_H

/*
 * tss32.h
 *
 * The content of the IA32 32 bit task state segment
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

#pragma pack(push)
#pragma pack(1)

/*
 * The content of the IA32 32 bit task state segment
 */
struct TaskStateSegment32 {
    uint16 previousTaskLink;
    uint16 empty1;
    // SS0:ESP0
    uint32 ring0StackPointer;
    uint16 ring0StackSegment;
    uint16 empty2;
    // SS1:ESP1
    uint32 ring1StackPointer;
    uint16 ring1StackSegment;
    uint16 empty3;
    // SS2:ESP2
    uint32 ring2StackPointer;
    uint16 ring2StackSegment;
    uint16 empty4;
    uint32 cr3;
    uint32 eip;
    uint32 eflags;
    uint32 eax;
    uint32 ecx;
    uint32 edx;
    uint32 ebx;
    uint32 esp;
    uint32 ebp;
    uint32 esi;
    uint32 edi;
    uint16 es;
    uint16 empty5;  // not in used
    uint16 cs;
    uint16 empty6;  // not in used
    uint16 ss;
    uint16 empty7;  // not in used
    uint16 ds;
    uint16 empty8;  // not in used
    uint16 fs;
    uint16 empty9;  // not in used
    uint16 gs;
    uint16 emptyA;  // not in used
    uint16 ldt;
    uint16 emptyB;  // not in used
    uint16 emptyC;  // not in used
    uint16 ioMapBaseAddress;
};
#pragma pack(pop)

/*
 * Some useful function regarding to IA32 task register and task API.
 */
class TaskStateSegment {
public:
    /*
     * Returns the based virtual memory location of the TSS32 segment inside
     * the GDT tables.
     *
     * selector - The GDT selector
     *
     * Throw exception if the selector is invalid.
     */
    static TaskStateSegment32* getTSS32(uint16 selector);

    /*
     * Returns the current selector of the task-register
     */
    static uint16 getTaskRegister();
};

#endif // __TBA_XDK_HOOKER_IA32_TSS32_H