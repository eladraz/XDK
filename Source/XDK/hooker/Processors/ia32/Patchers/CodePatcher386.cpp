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
 * CodePatcher386.cpp
 *
 * Implementation file for the CodePatcher interface over i386 machines
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/except/trace.h"
#include "xStl/except/exception.h"
#include "xStl/stream/traceStream.h"
#include "xdk/hooker/CodePatcher.h"
#include "xdk/hooker/Processors/ia32/Intel386Utils.h"
#include "xdk/hooker/Processors/ia32/PageTable/PageTable386.h"
#include "xdk/hooker/Processors/ia32/PageTable/Win32PageTable386.h"

CodePatcher::CodePatcher(void*          originalFunction,
                         const cBuffer& originalFunctionOpcode,
                         void*          callbackFunction,
                         void*          callbackFunctionObject /*=NULL*/) :
    m_context(callbackFunctionObject),
    m_callbackFunction(callbackFunction),
    m_originalFunction((uint8*)originalFunction),
    m_originalFunctionOpcode(originalFunctionOpcode)
{
    // TODO!
    CHECK(m_context == NULL);

    uint opcodeBoundyLength = m_originalFunctionOpcode.getSize();
    CHECK(opcodeBoundyLength >= NUMBER_OF_KNOWN_OPCODES_BYTES);
    // Test that the memory map is known
    uint8* function = (uint8*)m_originalFunction;
    for (uint i = 0; i < opcodeBoundyLength; i++)
    {
         if (function[i] != m_originalFunctionOpcode[i])
             XSTL_THROW(cException, EXCEPTION_FAILED);
    }

    // Prepare the callback buffer
    m_originalFunctionCallStub = m_originalFunctionOpcode;
    m_originalFunctionCallStub.changeSize(opcodeBoundyLength +
                                          Intel386Utils::ABSOLUTE_JMP_LENGTH);
    Intel386Utils::generateAbsoluteJmpOpcode(
        m_originalFunctionCallStub.getBuffer() +
        opcodeBoundyLength,
        getNumeric(m_originalFunction) + opcodeBoundyLength);

    // If this is a C++ call then the function generate a small stub
    uint32 callback = getNumeric(m_callbackFunction);
    if (m_context != NULL)
    {
        callback = generateCPPcode();
    }

    // Change the memory attribute to enable writing code to...
    traceHigh("CP: About to change the protection of the page..." << endl);
    #ifndef _KERNEL
        // Ring3 application should invoke a call to VirtualProtect
        CHECK(VirtualProtect(m_originalFunction,
                            ABSOLUTE_JMP_LENGTH,
                            PAGE_EXECUTE_READWRITE,
                            &m_oldProtection));
    #else
        // Ring0 drivers should invoke a call to the memory-manager
        GlobalPage* pe = Win32PageTable386::getPageEntry(m_originalFunction);
        // Test that the page is present
        CHECK(pe->m_present == 1);
        // Mark the page as writable
        if (pe->m_isWriteable == 0)
        {
            traceHigh("CP: The code is read-only, change attributes..." << endl);
            pe->m_isWriteable = 1;
        }
        else
        {
            traceHigh("CP: The code is already writable." << endl);
        }
    #endif

    // Put a stub
    cLock lock(m_patcher);

    // And hook the function. Notice that the hook is done by changing the
    // temporary variable
    traceHigh("CP: About to put the hook..." << endl);
    Intel386Utils::generateAbsoluteJmpOpcode((uint8*)originalFunction,
                                             (uint32)callback);
    traceHigh("CP: Code patched." << endl);
}

uint32 CodePatcher::generateCPPcode()
{
    /*
     * The function should invoke a JMP to the m_callbackFunction, but it also
     * should changes the ECX register.
     * This may be a problem since the ECX register cannot be restore.
     *
     * This function remains unimplments.
     */
    CHECK_FAIL();
}

CodePatcher::~CodePatcher()
{
    // Remove the stub
    cLock lock(m_patcher);

    // Unhook
    traceHigh("CP: Restore patching..." << endl);
    uint8* originalFunction = m_originalFunction;
    for (uint i = 0; i < NUMBER_OF_KNOWN_OPCODES_BYTES; i++)
    {
        originalFunction[i] = m_originalFunctionOpcode[i];
    }

    // And done! Other resource will automatically be free!
}

void* CodePatcher::getOriginalFunction()
{
    // Returns the stub!
    return (void*)m_originalFunctionCallStub.getBuffer();
}
