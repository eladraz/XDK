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

#ifndef __TBA_XDK_KERNEL_H
#define __TBA_XDK_KERNEL_H

/*
 * kernel.h
 *
 * Include all information needed in order to use the spesific DDK.
 * The module also handles the startup of the libC and it's termination.
 * All the kernel functions are wrapped in "extern C" block to allow
 * completion with C++ and C files.
 *
 * Author: Elad Raz <e@eladraz.com>
 */

// Include the modules of the DDK
/*
 * Usage of extern "C" in a C/C++ compilers. This macro is also defined
 * inside "xStl/types.h", the XDK needed it sooner than that.
 */
#ifdef __cplusplus
    #define EXTERNC extern "C"
#else
    #define EXTERNC
#endif

#ifdef _KERNEL
// User-mode application (testing utilities) might want to include this file
// without the kernel API

EXTERNC {
    #if defined(XPDDK)
        // In windows XP DDK, the folder struct is a little different
        #include <ntddk.h>
        #include <ntdef.h>
        #include <basetsd.h>
        #include <windef.h>
    #elif defined (NTDDK)
        // Windows NT DDK

        // NT DDK doesn't support 64bit platforms
        #pragma warning(push)
        #pragma warning(disable:4311)
        #include <ntddk.h>
        #include <ntdef.h>
        typedef unsigned int* UINT_PTR;
        typedef unsigned long* LONG_PTR;
        #include <windef.h>
        #pragma warning(pop)
    #else
        // Windows 2000 DDK
        #include <ddk/ntddk.h>
        #include <ntdef.h>
        #include <basetsd.h>
    #endif

    // This macro causes a lot of problems!!!
    // Don't include it ever!
    #ifdef NT_UP
        #error RAZI: Please remove this preprocessor from your project!!!\n
    #endif
};

// Include the xStl main include file, if not defined.
#ifndef __TBA_STL_TYPES_H
#include "xStl/types.h"
#endif


/*
 * The DRIVERENTRY suffix used in-order to define callback into API kernel
 * export functions. The macro export function in the non-mangle format.
 */
#define DRIVERENTRY EXTERNC

/*
 * Function which requires IRQL PASSIVE_LEVEL to operate should call this macro
 * at the beginning. The macro test that the IRQL is indeed PASSIVE_LEVEL, and
 * if not trace a message and throw exception...
 */
void testPageableCode();


////////////////////////////////////
// Undocument kernel API functions.
#include "XDK/undocumented.h"

#endif // _KERNEL

#endif //__TBA_XDK_KERNEL_H
