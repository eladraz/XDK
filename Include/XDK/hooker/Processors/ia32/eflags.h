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

#ifndef __TBA_XDK_HOOKER_IA32_EFLAGS_H
#define __TBA_XDK_HOOKER_IA32_EFLAGS_H

/*
 * eflags.h
 *
 * The content of the IA32 EFLAGS register
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * See Intel System Programming manual, chapter 2
 *          "SYSTEM FLAGS AND FIELDS IN THE EFLAGS REGISTER"
 */
typedef union {
    // The selector raw data
    uint32 m_value;
    // The flags bits
    struct {
    // First Byte
        // The CF
        unsigned m_carryFlag : 1;
        // Always 1
        unsigned m_reserved1 : 1;
        // TODO!
        unsigned pf : 1;
        // Always 0
        unsigned m_reserved2 : 1;
        // TODO!
        unsigned af : 1;
        // Always 0
        unsigned m_reserved3 : 1;
        // The ZF
        unsigned m_zeroFlag : 1;
        // TODO!
        unsigned sf : 1;

    // Second Byte
        // Set (1) to enable single-step operation
        unsigned m_trapFlag : 1;
        // 0 to mask-out all interrupts.
        unsigned m_interruptEnable : 1;
        // TODO!
        unsigned df : 1;
        // The OF
        unsigned m_overflowFlag : 1;
        // The IOPL
        unsigned m_iopl : 2;
        // The NT.
        unsigned m_nestedTask : 1;
        // Always 0
        unsigned m_reserved4 : 1;

    // Third Byte
        // The RF
        unsigned m_resumeFlag : 1;
        // The VM
        unsigned m_virtualMode : 1;
        // The AC
        unsigned m_alignmentCheck : 1;
        // The VIF
        unsigned m_virtualInterruptFlag : 1;
        // The VIP
        unsigned m_virtualInterruptPending : 1;
        // The ID
        unsigned m_identificationFlag : 1;
        // Reserved
        unsigned m_reserved : 10;
    } m_flags;
} EFlags;

#endif // __TBA_XDK_HOOKER_IA32_EFLAGS_H