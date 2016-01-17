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

#ifndef __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTEXCEPTION_H
#define __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTEXCEPTION_H

/*
 * InterruptException.h
 *
 * Caused by the interrupt handler. When interrupt is executed in the current
 * processor, a second interrupt may occured. In that case the exception handler
 * will not trigger a second interrupt, but throw exception for the current
 * execution.
 *
 * This interrupt can be catch using the InterruptException handler.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * The InterruptException should be constructed by the HookIdt class.
 * Usage: (Inside interrupt code)
 *     XSTL_TRY {
 *         uint8* maybeBadPtr;
 *         // Access to data
 *         uint8 data = *maybeBadPtr;
 *         // Exception will thrown is 'maybeBadPtr' is a bad pointer
 *
 *         thePointerIsGood = true;
 *     }
 *     XSTL_CATCH(InterruptException& e)
 *     {
 *         // 'e' is on the current stack but in distance from the current stack
 *         // frame. There is no need to concern about overruned data.
 *         thePointerIsGood = false;
 *     }
 */
class InterruptException {
public:
    /*
     * Constructor. Generate & initialized new InterruptException object
     *
     * vector - The exception vector number
     * data   - Unique information for that exception vector ID.
     *          See getData() for unique explaination.
     */
    InterruptException(uint8 vector,
                       uint data);

    /*
     * Return the vector number for the exception.
     */
    uint8 getVector() const;

    /*
     * Return the unique information data
     *
     * For IA32 exception which contains error-code this data will be the error
     * code.
     *
     * Special cases:
     *    Int14 (0x0E) - This number will be the CR2 which contains the address
     *                   for the memory exception.
     */
    uint getData() const;

private:
    // The vector for the second interrupt
    uint8 m_vector;
    // The data for the exception
    uint m_data;
};

#endif // __TBA_XDK_HOOKER_IA32_IDT_INTERRUPTEXCEPTION_H
