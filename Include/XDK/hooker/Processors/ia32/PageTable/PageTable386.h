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

#ifndef __TBA_XDK_HOOKER_IA32_PAGETABLE_PAGETABLE386_H
#define __TBA_XDK_HOOKER_IA32_PAGETABLE_PAGETABLE386_H

/*
 * PageTable386.h
 *
 * The global definitions file for IA32 bit processor page-table structs
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * The global-page entry descriptor. Used to describe protected mode memory
 * management used by Intel 32bit processor (I386+) without any extentions:
 * PTE 4kb, PDE and 4MB pages.
 */
typedef struct {
    // Is the page present
    unsigned m_present : 1;
    // 1 means that the page is read/write, otherwise the page is read-only
    unsigned m_isWriteable : 1;
    // The protection flags. 1 means that user applications can access the page
    unsigned m_isUserAccess : 1;
    // Caching controls bits
    unsigned m_pageWriteThrough : 1;
    unsigned m_cacheDisabled    : 1;
    // Set to 1 by the processor each time the page accessed. Used by the
    // memory manager to control page-swap operations.
    unsigned m_accessed : 1;
    // Set to 1 by the processor each time the page was written to.
    // NOTE: This flag exist only in the PTE and pages with 4MB size
    unsigned m_pteDirty_pde4kReserved : 1;
    // PTE uses this bits (PentiumIII+) as Page Table Attribute Index. PDE uses
    // this bit as a page size selector, when this bit is zero, the PDE entry
    // is a descriptor for 4kb PTE table, otherwise it's 4mb page.
    unsigned m_ptePAT_pdePageSize : 1;
    // For PDE entries (Pentium pro+) indicates whether the TLB should cache
    // the pages for the entry system.
    unsigned m_pdeGlobalPage : 1;
    // Free to used by operating system developments. For Windows enviroment
    // these pages indicate the location inside the page-file.
    unsigned m_avaliable : 3;

    // The base address for the page
    unsigned m_address : 20;
} GlobalPage;

// Each page for the ia32 bit processor is a 32bit entry
typedef uint32 GlobalPageValue;

/*
 * Translation functions. Translate from a page-struct and a numeric value.
 *
 * page - The page to be translated to numeric value
 * pageContent - The numeric value to translate to page value.
 *
 * Return the page/page content depending on the call.
 */
GlobalPageValue getPageValue(GlobalPage page);
GlobalPage getPageFromValue(GlobalPageValue pageContent);

enum {
    // 4kb page-size
    IA32_PAGE_SIZE = 0x1000,
    // 4Mb page-size
    IA32_LARGEPAGE_SIZE = 0x400000
};

#endif // __TBA_XDK_HOOKER_IA32_PAGETABLE_PAGETABLE386_H
