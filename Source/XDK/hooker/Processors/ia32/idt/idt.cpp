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
 * idt.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "xdk/hooker/Processors/ia32/descriptorTable.h"
#include "xdk/hooker/Processors/ia32/idt/idt.h"

IDTR::IDTR()
{
    // The IDTR is 48 bits long
    uint32 idtr[2] = {0,0};

    // Store the IDT
    _asm { sidt idtr }

    // Parse the IDTR
    m_limit = (uint16)(idtr[0] & 0xFFFF);
    m_base = (DescriptorValue*)getPtr((idtr[0] >> 16) + (idtr[1] << 16));
}


/**************************************/

IDTDescriptor::IDTDescriptor(const IDTR& idtr, uint8 vector) :
    m_vector(vector)
{
    CHECK((vector*sizeof(DescriptorValue)) < idtr.getLimit());
    m_interruptDescriptor.m_descriptor = idtr.getBase()[vector];

    /*
    traceHigh(HEXDWORD((uint32)(m_interruptDescriptor.m_descriptor >> 32)) <<
              ':' <<
              HEXDWORD((uint32)(m_interruptDescriptor.m_descriptor)) <<
              sizeof(m_interruptDescriptor));
    */
}

DescriptorValue IDTDescriptor::getDescriptor() const
{
    return m_interruptDescriptor.m_descriptor;
}

uint8 IDTDescriptor::getVector() const
{
    return m_vector;
}

bool IDTDescriptor::isPresent() const
{
    InterruptDescriptorFlags bits;
    bits.m_flags = m_interruptDescriptor.m_bits.m_flags;
    if (bits.m_bits.m_present == 1) {
        CHECK((bits.m_bits.m_type == IDT_TASK_GATE) ||
              (bits.m_bits.m_type == IDT_INT_GATE) ||
              (bits.m_bits.m_type == IDT_TRAP_FLAG));
        return true;
    } else {
        return false;
    }
}

uint16 IDTDescriptor::getSelector() const
{
    CHECK(isPresent());
    return m_interruptDescriptor.m_bits.m_selector;
}

uint32 IDTDescriptor::getOffset() const
{
    CHECK(getIntType() != IDT_TASK_GATE);

    InterruptDescriptorFlags bits;
    bits.m_flags = m_interruptDescriptor.m_bits.m_flags;

    if (bits.m_bits.m_gateSize == 0) {
        // 16 bits
        return m_interruptDescriptor.m_bits.m_offset0_15;
    } else {
        // 32 bit
        return m_interruptDescriptor.m_bits.m_offset0_15 +
               (m_interruptDescriptor.m_bits.m_offset16_31 << 16);
    }
}

void IDTDescriptor::changeOffset(uint32 newOffset)
{
    m_interruptDescriptor.m_bits.m_offset0_15 = (uint16)(newOffset & 0xFFFF);
    m_interruptDescriptor.m_bits.m_offset16_31 = (uint16)((newOffset >> 16) & 0xFFFF);
}

IDTDescriptor::InterruptDescriptorType IDTDescriptor::getIntType() const
{
    CHECK(isPresent());

    InterruptDescriptorFlags bits;
    bits.m_flags = m_interruptDescriptor.m_bits.m_flags;

    return (InterruptDescriptorType)bits.m_bits.m_type;
}

uint IDTDescriptor::getDpl() const
{
    CHECK(isPresent());

    InterruptDescriptorFlags bits;
    bits.m_flags = m_interruptDescriptor.m_bits.m_flags;

    return bits.m_bits.m_dpl;
}

#ifdef COMMON_DUMP
void IDTDescriptor::traceOut() const
{
    // Test if the IDT is present
    if (!isPresent()) {
        traceHigh(HEXBYTE(m_vector) << "  INVALID" << endl);
        return;
    }

    switch (getIntType()) {
    case IDT_TASK_GATE:
        traceHigh(HEXBYTE(m_vector) << "  Task gate.      DPL: " <<
                  getDpl() << "  Selector " << HEXWORD(getSelector()) << endl);
        break;
    case IDT_INT_GATE:
        traceHigh(HEXBYTE(m_vector) << "  Interrupt gate. DPL: " <<
                  getDpl() << "  Selector " << HEXWORD(getSelector()) <<
                  "  Offset " << HEXDWORD(getOffset()) << endl);
        break;
    case IDT_TRAP_FLAG:
        traceHigh(HEXBYTE(m_vector) << "  Trap gate.      DPL: " <<
                  getDpl() << "  Selector " << HEXWORD(getSelector()) <<
                  "  Offset " << HEXDWORD(getOffset()) << endl);
        break;
    default:
        // Error!
        CHECK_FAIL();
    }
}
#endif // COMMON_DUMP
