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

#ifndef __TBA_XDK_HOOKER_IA32_REGISTERS_H
#define __TBA_XDK_HOOKER_IA32_REGISTERS_H

/**
 * register.h
 *
 * The content of the IA32 registers
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

#pragma pack(push)
#pragma pack(1)

/**
 * When the processor execute 32bits PUSHAD instruction (opcode: 60h)
 * the following register is pushed (in reveresed order, but esp is expand
 * downward).
 *
 * The reason why the 'Registers' struct is separated than the InterruptFrame
 * is that the processor may insert an ErrorCode or push the stack registers
 * on the stack
 */
struct Registers {
    // The segment selectors.
    uint16 es;
    uint16 empty1;  // not in used
    uint16 ds;
    uint16 empty2;  // not in used
    uint16 ss;
    uint16 empty3;  // not in used
    uint16 fs;
    uint16 empty4;  // not in used
    uint16 gs;
    uint16 empty5;  // not in used
    // The pusha registers
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 esp;
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;
	uint32 esp_orig; // !YOAV!
	uint32 frame_holder1; // !YOAV!
	uint32 frame_holder2; // !YOAV!
	uint32 frame_holder3; // !YOAV!
};

/**
 * When a 32bit task execute an interrupt the EFLAGS, CS and EIP are pushed
 * and the function should return in IRET.
 */
struct InterruptFrame {
    // The trap frame
    uint32 eip;
    uint16 cs;     // The cs is 16 bits but at 32bit task it's taken 32 bit
    uint16 empty;  // not in used
    uint32 eflags;
    // For some traps there is an uint32 error-code here.
};
#pragma pack(pop)

#endif // __TBA_XDK_HOOKER_IA32_REGISTERS_H