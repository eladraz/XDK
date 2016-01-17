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
 * KbHit.cpp
 *
 * The implementation of keyboard hitting class.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include <windows.h>
#include "KbHit.h"

KbHit::KbHit() :
    m_char(0)
{
    m_consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    CHECK(m_consoleInput != INVALID_HANDLE_VALUE);
}

bool KbHit::isHit()
{
    DWORD count;
    INPUT_RECORD input;
    GetNumberOfConsoleInputEvents(m_consoleInput, &count);

    for (uint i = 0; i < count; i++)
    {
        DWORD read = 0;
        if (ReadConsoleInput(m_consoleInput, &input, 1, &read))
        {
            if (input.EventType == KEY_EVENT)
            {
                if (input.Event.KeyEvent.bKeyDown)
                {
                    m_char = input.Event.KeyEvent.uChar.UnicodeChar;
                    return true;
                }
            }
        }
    }

    return false;
}

character KbHit::readKey()
{
    return m_char;
    m_char = 0;
}
