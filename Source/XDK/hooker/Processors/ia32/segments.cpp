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
 * segments.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/stream/stringerStream.h"
#include "xStl/stream/traceStream.h"
#include "xdk/hooker/Processors/ia32/descriptorTable.h"
#include "xdk/hooker/Processors/ia32/segments.h"

Selector::Selector(uint16 selector)
{
    m_selector.m_selector = selector;
}

uint Selector::getRpl() const
{
    return m_selector.m_bits.m_requestPrivilageLevel;
}

bool Selector::isGDTselector() const
{
    return (m_selector.m_bits.m_tableIndicator == 0);
}

uint Selector::getIndex() const
{
    return m_selector.m_bits.m_index;
}

#ifdef COMMON_DUMP
cString Selector::getSelectorName() const
{
    cStringStream ret;
    ret << "SL: " << HEXWORD(m_selector.m_selector) << "  (RPL:" << getRpl()
        << " " << (isGDTselector() ? "GDT" : "LDT") << ")";

    return ret.getData();
}
#endif //COMMON_DUMP

//////////////////////////////////////////////////////////////////////////

GDTR::GDTR()
{
    // The GDTR is 48 bits long
    uint32 gdtr[2] = {0,0};

    // Store the GDT
    _asm { sgdt gdtr }

    // Parse the IDTR
    m_limit = (uint16)(gdtr[0] & 0xFFFF);
    m_base = (DescriptorValue*)getPtr((gdtr[0] >> 16) + (gdtr[1] << 16));
}

//////////////////////////////////////////////////////////////////////////

SegmentDescriptor::SegmentDescriptor(const DescriptorTableRegister& dtr,
                                     const Selector& selector) :
    m_selector(selector)
{
    CHECK((selector.getIndex()*sizeof(DescriptorValue)) < dtr.getLimit());
    m_descriptor.m_descriptor = dtr.getBase()[selector.getIndex()];
}

bool SegmentDescriptor::isPresent() const
{
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    return (flags.m_bits.m_present == 1);
}

uint32 SegmentDescriptor::getOffset() const
{
    CHECK(isPresent());
    return (m_descriptor.m_bits.m_base0_15) +
           (m_descriptor.m_bits.m_base16_23 << 16) +
           (m_descriptor.m_bits.m_base24_31 << 24);
}

uint32 SegmentDescriptor::getSegmentLimit() const
{
    CHECK(isPresent());
    SegmentFlags1 flags1;
    flags1.m_flags = m_descriptor.m_bits.m_flags1;
    uint32 ret = (m_descriptor.m_bits.m_segmentLimit0_15) +
        (flags1.m_bits.m_segmentLimit16_19 << 16);

    if (flags1.m_bits.m_granularity == 1) {
        ret*= PAGE_SIZE;
        ret+= PAGE_SIZE-1;
    }

    return ret;
}

const Selector& SegmentDescriptor::getSelector() const
{
    return m_selector;
}

bool SegmentDescriptor::isSystem() const
{
    CHECK(isPresent());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    return (flags.m_bits.m_descriptorType == 0);
}

uint SegmentDescriptor::getSystemSegmentType() const
{
    CHECK(isSystem());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    return flags.m_bits.m_type;
}

bool SegmentDescriptor::isCode() const
{
    CHECK(!isSystem());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    SegementTypeFlags type;
    type.m_type = flags.m_bits.m_type;
    return (type.m_bits.m_code == 1);
}

bool SegmentDescriptor::isDataExpandDown() const
{
    CHECK(!isCode());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    SegementTypeFlags type;
    type.m_type = flags.m_bits.m_type;
    SegementTypeFlagsData data;
    data.m_typeFlags = type.m_bits.m_attributes;
    return (data.m_bits.m_expandDown == 1);
}

bool SegmentDescriptor::isDataReadOnly() const
{
    CHECK(!isCode());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    SegementTypeFlags type;
    type.m_type = flags.m_bits.m_type;
    SegementTypeFlagsData data;
    data.m_typeFlags = type.m_bits.m_attributes;
    return (data.m_bits.m_readOnly == 0);
}

bool SegmentDescriptor::isCodeConforming() const
{
    CHECK(isCode());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    SegementTypeFlags type;
    type.m_type = flags.m_bits.m_type;
    SegementTypeFlagsCode code;
    code.m_typeFlags = type.m_bits.m_attributes;
    return (code.m_bits.m_conforming == 1);
}

bool SegmentDescriptor::isCodeExecuteOnly() const
{
    CHECK(isCode());
    SegmentFlags flags;
    flags.m_flags = m_descriptor.m_bits.m_flags;
    SegementTypeFlags type;
    type.m_type = flags.m_bits.m_type;
    SegementTypeFlagsCode code;
    code.m_typeFlags = type.m_bits.m_attributes;
    return (code.m_bits.m_executeOnly == 0);
}

#ifdef COMMON_DUMP
void SegmentDescriptor::traceOut() const
{
    // Test if the descriptor is present
    if (!isPresent()) {
        traceHigh(m_selector.getSelectorName() << "  INVALID" << endl);
        return;
    }

    // Start parsing the descriptor
    traceHigh(m_selector.getSelectorName() << "  Base: " <<
              HEXDWORD(getOffset()) << "  Limit: " <<
              HEXDWORD(getSegmentLimit()) << "  ");

    if (isSystem()) {
        // Show the system descriptor
        traceHigh("SYSTEM " << HEXBYTE(getSystemSegmentType()) << " " <<
                  m_systemSegmentsStrings[getSystemSegmentType()]);
    } else {
        // Append the segment attributes
        if (isCode()) {
            // Code segment
            traceHigh("CODE " <<
                (isCodeExecuteOnly() ? "Execute only" : "Execute/read only") <<
                (isCodeConforming() ? "Conforming" : "Nonconforming"));
        } else {
            // Data segment
            traceHigh("DATA " <<
                (isDataReadOnly() ? "Read only" : "Read/write") <<
                (isDataExpandDown() ? "Expand-down" : ""));
        }
    }

    // Trace it out
    traceHigh(endl);
}

const SegmentDescriptor::lpString SegmentDescriptor::m_systemSegmentsStrings[] = {
    "Reserved for future use",
    "16 bit Task Segment (avaliable)",
    "The local descriptor table segment",
    "16 bit Task Segment (busy)",
    "16 bit call gate",
    "Task gate (selector)",
    "16 bit interrupt gate",
    "16 bit trap gate",
    "Reserved for future use",
    "32 bit Task Segment (avaliable)",
    "Reserved for future use",
    "32 bit Task Segment (busy)",
    "32 bit call gate",
    "Reserved for future use",
    "32 bit interrupt gate",
    "32 bit trap gate"
};

#endif // COMMON_DUMP
