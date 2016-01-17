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

#ifndef __TBA_XDK_HOOKER_IA32_DESCRIPTORTABLE_H
#define __TBA_XDK_HOOKER_IA32_DESCRIPTORTABLE_H

/*
 * descriptorTable.h
 *
 * Intel IA32 processor descriptors.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/// Each interrupt descriptor is 64bits long
typedef uint64 DescriptorValue;

/**
 * A virtual descriptor register representation.
 * This class is match the LDT table, GDT table and IDT tables
 * Even the TSS can be formated in this way.
 *
 * See Intel System programming vol I for more information.
 */
class DescriptorTableRegister
{
public:
    /// Virtual class
    virtual ~DescriptorTableRegister();

    /// Return the limit part
    uint16 getLimit() const;

    /// Calculate the number of entries in the table
    uint16 getEntries() const;

    /// Return the base part
    DescriptorValue* getBase() const;

protected:
    /// This class cannot be constructed
    DescriptorTableRegister();
    /// The limit is the number of bytes for the table
    uint16 m_limit;
    /// The base entry.
    DescriptorValue* m_base;
};

#endif // __TBA_XDK_HOOKER_IA32_DESCRIPTORTABLE_H
