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

#ifndef __TBA_XDK_HOOKER_CODEPATCHER_H
#define __TBA_XDK_HOOKER_CODEPATCHER_H

/*
 * CodePatcher.h
 *
 * The code patcher module responsible for runtime function hooking.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/sarray.h"
#include "xdk/hooker/Locks/GlobalSystemLock.h"
#include "xdk/hooker/Processors/ia32/Intel386Utils.h"

/*
 * Perform a safe hooking over a linear function addres. There are two modes
 * of hooking:
 *   1. Normal C function hooking. The callback declare as global C stdcall or
 *      cdecl function.
 *
 *   2. C++ Hooker function. Which are called within a context of a class.
 *      NOTE: THIS FUNCTION IS NOT IMPLEMENT YET!
 *
 * Usage:
 *    TODO!
 *
 * For more examples, see the test-code-patcher application...
 *
 * This class is implement safely for i386 kernel mode XDK applications and
 * windows user-mode applications.
 */
class CodePatcher
{
public:
    /// The number of known opcode for patching
    enum { NUMBER_OF_KNOWN_OPCODES_BYTES = Intel386Utils::ABSOLUTE_JMP_LENGTH };

    /*
     * Constructor.
     * Hooks a function.
     *
     * originalFunction       - The address of the hooked function
     * originalFunctionOpcode - The memory bytes at the target function memory
     *                          area. Used for both assertion and opcode boundry
     *                          calculation. The size of these array should be
     *                          at least NUMBER_OF_KNOWN_OPCODES_BYTES bytes.
     * callbackFunction       - The address of the callback function
     * callbackFunctionObject - If the callback function is inside C++ class
     *                          then this value should be the context (thiscall)
     *                          of the function.
     *
     * Throw exception if the hooking failed.
     */
    CodePatcher(void*          originalFunction,
                const cBuffer& originalFunctionOpcode,
                void*          callbackFunction,
                void*          callbackFunctionObject = NULL);

    /*
     * Remove the hooking.
     */
    ~CodePatcher();

    /*
     * Return the original function address. The returned address points to a
     * stub which invoke to mising opcodes and invoke a call to the original
     * function.
     */
    void* getOriginalFunction();

private:
    // Disallow copy-constructor and operator =
    CodePatcher(const CodePatcher& other);
    CodePatcher& operator = (const CodePatcher& other);

    /*
     * Generate a C++ code. Fill up the m_cppStubAPI member.
     *
     * Returns the address of the stub
     */
    uint32 generateCPPcode();

    // The lockable which blocks the
    GlobalSystemLock m_patcher;

    // The original bytes
    cBuffer m_originalFunctionOpcode;
    // The address of the hooked function
    uint8* m_originalFunction;
    // The address of the callback function
    void* m_callbackFunction;

    // The opcode which should be executed in order to call the
    // original function
    cBuffer m_originalFunctionCallStub;

    #ifndef _KERNEL
    // For User-mode application, save the page-table old properties.
    DWORD m_oldProtection;
    #endif

    // The context object for the hooked
    void* m_context;
    // The context stub function
    cBuffer m_cppStubAPI;
};

#endif // __TBA_XDK_HOOKER_CODEPATCHER_H
