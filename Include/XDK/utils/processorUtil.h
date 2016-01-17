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

#ifndef __TBA_XDK_UTILS_PROCESSORUTIL_H
#define __TBA_XDK_UTILS_PROCESSORUTIL_H

/*
 * processorUtil.h
 *
 * Contains static function for current processor information.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * The IRQL of ProcessorLock or during interrupt
 */
enum {
    // The processor mode is either ProcessorLock or during interrupt handling
    TBA_INTERRUPT_IRQL = 0xFF,
    // The interrupt mask is about to be enabled
    RETURN_FROM_INTERRUPT_IRQL = 0x80
};


/*
 * The API that represent below is unified for all platforms and compilation
 * units.
 */
class cProcessorUtil {
public:

    // Processor API

    /*
     * Return the count of active processors using 'KeGetActiveProcessors'.
     * This API uses pre-read ahead cache number so the 'KeNumberProcessor' will
     * not be access and can be traped.
     *
     * NOTE: The operating system cannot deactivate a processor without a reboot
     *
     * IRQL: Any.
     * Can be invoke during interrupt hooking function as well.
     */
    static uint getNumberOfProcessors();

    /*
     * Return the current processor number. This number is from 0 to
     * 'getNumberOfProcessors()' according to the implementation platform.
     *
     * This function doens't use the KeGetCurrentProcessorNumber, since this
     * function might be trapped.
     *
     * NOTE: The operating system cannot deactivate a processor without a reboot
     *
     * IRQL: Any.
     * Can be invoke during interrupt hooking function as well.
     */
    static uint getCurrentProcessorNumber();

    // IRQL API

    /*
     * The bug-check code for all bugs in the system.
     *  - When the number of processors in the system is bigger than
     *    'MAX_PROCESSORS_SUPPORT'. (Minicode #1)
     *  - When IRQL is directly raise to 'TBA_INTERRUPT_IRQL' without raising
     *    the IRQL to DISPATCH_LEVEL or above (Minicode #2)
     *  - When IRQL is change from 'TBA_INTERRUPT_IRQL' without first lowering
     *    it by 'RETURN_FROM_INTERRUPT_IRQL'
     */
    enum { PROCESSOR_LOCK_BUGCHECK_CODE = 0x690C10C8 };

    /*
     * For now the machine support no more then 8 processors
     */
    enum { MAX_PROCESSORS_SUPPORT = 8 };

    /*
     * Return true if the IRQL equals to TBA_INTERRUPT_IRQL.
     *
     * This call doens't invoke any operating system calls.
     */
    static bool isInterruptModeIrql();

    /*
     * Return the current IRQL. If the code is executed during interrupt hooking
     * function, this function will return 'TBA_INTERRUPT_IRQL'.
     *
     * Here is a small description of the function behaviour:
     *    - If the current processor is locked (Processor-locked) or during
     *      interrupt, the function will return 'TBA_INTERRUPT_IRQL'
     *    - Otherwise, a call to KeGetCurrentIrql is made.
     *
     * IRQL: Any.
     * Can be invoke during interrupt hooking function as well.
     */
    static KIRQL getCurrentIrql();

    /*
     * Change the IRQL to a different IRQL level.
     *
     * There are few condition when rasing IRQL into TBA_INTERRUPT_IRQL:
     *    1. There mustn't be any context-switch in the processor, which means
     *       that all hardware interrupts are masked out.
     *    2. For opertaing-system IRQL based routines: The IRQL must be
     *       at HIGH_LEVEL or DISPATCH_LEVEL. We are strongly recommend on HIGH
     *       level but it's not required.
     *
     * There are also few condition when lowering IRQL from TBA_INTERRUPT_IRQL:
     *    1. The interrupt must be disabled, the new IRQL is ignored.
     *    2. The newIrql must be RETURN_FROM_INTERRUPT_IRQL
     *
     * newIrql - The new IRQL
     *
     * Return the old IRQL.
     * IRQL: Any.
     * Can be invoke during interrupt hooking function as well.
     */
    static KIRQL setIrql(KIRQL newIrql);
};

#endif // __TBA_XDK_UTILS_PROCESSORUTIL_H
