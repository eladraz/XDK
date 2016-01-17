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

#ifndef __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDT_H
#define __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDT_H

/**
 * idt.h
 *
 * A parser file for the Interrupt Descriptor Table for IA32 processor family
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/hooker/Processors/ia32/descriptorTable.h"

// Change the packing method for single byte. (No alignment)
#pragma pack(push)
#pragma pack(1)

/**
 * The IDTR register content.
 */
class IDTR : public DescriptorTableRegister
{
public:
    /**
     * Default constructor. Invoke SIDT in order to query the IDTR
     * register
     */
    IDTR();
};


/**
 * Each entry inside the IDT contains 8 bytes. The higher dword contains
 * flags which can be interpted
 */
class IDTDescriptor {
public:
    /**
     * Parse the vector for a spesific IDT.
     *
     * idtr   - The descriptor for the table
     * vector - The index inside
     */
    IDTDescriptor(const IDTR& idtr, uint8 vector);

    /**
     * The different descriptors of the IDT
     *
     * See getIntType()
     */
    enum InterruptDescriptorType {
        // Task gate type
        IDT_TASK_GATE = 5,
        // Interrupt gate type
        IDT_INT_GATE  = 6,
        // Trap gate type
        IDT_TRAP_FLAG = 7
    };

    /**
     * Return the interrupt vector
     */
    uint8 getVector() const;

    /**
     * Return the descriptor value
     */
    DescriptorValue getDescriptor() const;

    /**
     * Return true if the descriptor is present
     */
    bool isPresent() const;

    /**
     * Calculate the offset for the descriptor.
     * The offset is valid only at interrupt gate or trap gate.
     * Task gate or non-present descriptor throws exception
     * 16 bit descriptor will return result in 32 bit format, but only the
     * lower 16 bits are in used
     */
    uint32 getOffset() const;

    /**
     * Return the selector for the IDT
     */
    uint16 getSelector() const;

    /**
     * Return the type of the interrupt descriptor
     * See m_interruptDescriptor::m_bits::m_flags::m_type
     */
    InterruptDescriptorType getIntType() const;

    /**
     * Return the descriptor privilege level
     */
    uint getDpl() const;

    /**
     * Trace the content of the descriptor in a TRACE statements
     */
    #ifdef COMMON_DUMP
    void traceOut() const;
    #endif // COMMON_DUMP


    /**
     * Change the offset of the interrupt.
     * NOTE: The interrupt is not required to be present
     *
     * newOffset - The changed offset
     */
    void changeOffset(uint32 newOffset);

private:
    /// The vector number
    uint m_vector;

    /// The flags of the descriptors
    union {
        // The content of the union
        DescriptorValue m_descriptor;
        // The common flags representation
        struct {
            // The lower 32 bits are different beteen Task gate and
            // between "Interrupt gate and trap gate"

            // The lower 16 bits offset inside the selector. These bits are
            // reserved for task gate
            uint16 m_offset0_15;
            // The selector to be used
            uint16 m_selector;
            // Theses bits are reserved for future use
            uint8  m_reserved;
            // The flags See InterruptDescriptorFlags
            uint8 m_flags;
            /// The high portion of the offset. Used only in interrupt gate and
            /// trap gate
            uint16 m_offset16_31;
        } m_bits;
    } m_interruptDescriptor;

    typedef union {
        uint8 m_flags;
        struct {
            /**
             * The type of interrupt descriptor:
             *   1 0 1 (5) - Task gate
             *   1 1 0 (6) - Interrupt gate
             *   1 1 1 (7) - Trap gate
             */
            unsigned m_type : 3;
            /// The size of the gate: 0 - 16bit, 1 - 32bit
            unsigned m_gateSize : 1;
            /// Must be zero
            unsigned m_zero : 1;
            /// The privieliage level
            unsigned m_dpl : 2;
            /// The present flag
            unsigned m_present : 1;
        } m_bits;
    } InterruptDescriptorFlags;
};
// Restore packing method
#pragma pack(pop)

#endif // __TBA_BLUE_COMMON_DRIVER_HOOKER_IA32_IDT_IDT_H
