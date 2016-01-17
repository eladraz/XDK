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

#ifndef __TBA_XDK_UNDOCUMENTED_H
#define __TBA_XDK_UNDOCUMENTED_H

/*
 * undocumented.h
 *
 * Undocument kernel API functions.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/kernel.h"
#include "XDK/undocumentedStructs.h"

/*
 * Throws exception for kernel drivers. In order to use this function generate
 * ExceptionRecord object and use this object to throw it.
 * See EHLib for more information.
 *
 * This function is undocumented.
 *
 * exceptionObject - The object to throw
 *
 * This function should never return. The flow of the application will be changed.
 */
EXTERNC void NTAPI RtlRaiseException(struct _EXCEPTION_RECORD* exceptionObject);

/*
 * Translate NTSTATUS to textual human readable ANSI string.
 *
 * messageId      - The message to be translated to
 * returnedString - The returned address
 *
 * NOTE! The returned string mustn't be free using the RtlFreeAnsiString!
 *       The returned pointer points inside ntoskrnl!.rsrc+???
 *
 * TODO! This function is only exported at WindowsXP
 */
//EXTERNC BOOLEAN NTAPI KeGetBugMessageText(NTSTATUS messageId,
//                                          PANSI_STRING returnedString);

/*
 * Retrieve system information.
 *
 * SystemInformationClass  - The type of information to retrieve
 * SystemInformation       - Pointer to a struct which will be filled with the
 *                           system information. This struct changes with the
 *                           SystemInformationClass value
 * SystemInformationLength - The number of bytes of the giving pointer. Used
 *                           to prevents overflows
 * ReturnLength            - The number of bytes actually returned.
 *
 * Return STATUS_SUCCESS to indicates success.
 * Return STATUS_INFO_LENGTH_MISMATCH to indicate that the giving buffer is not
 *        big enough
 * Return failure code.
 *
 * See SystemModuleInformation
 */
EXTERNC NTSTATUS NTAPI ZwQuerySystemInformation(DWORD SystemInformationClass,
        PVOID SystemInformation,
        DWORD SystemInformationLength,
        PDWORD ReturnLength);

// -----------------------------------------------------------------
    /*
     * The system-module information command is giving as a parameter to the
     * ZwQuerySystemInformation kernel API and used to retrieve a list of all
     * loaded kernel modules in the system.
     * The 'SystemInformation' is a pointer to allocated array of 'MODULE_LIST'
     * type.
     */
    #define SystemModuleInformation 11 // SYSTEMINFOCLASS

// -----------------------------------------------------------------


/*
 * Change the affinity mask of a kernel mode thread
 * NOTE: The new affinity must be a subset of the process affinity map!!
 *
 * thread   - The thread to be changed
 * affinity - The new affinity of the thread
 *
 * Returns the previous affinity of the thread
 */
EXTERNC KAFFINITY NTAPI KeSetAffinityThread(PKTHREAD thread,
                                            KAFFINITY affinity);

#endif // __TBA_XDK_UNDOCUMENTED_H
