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
 * atexit.cpp
 *
 * Since I can't find a single discent "LIBC" for the kernel.
 * I will have to implement the "atexit" function. The atexit
 * function should able to register functions and call them
 * when the Driver will terminate - When the Unload function
 * calls.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/list.h"
#include "xStl/data/datastream.h"
#include "xStl/except/trace.h"
#include "xStl/except/assert.h"
#include "xStl/except/exception.h"
#include "xStl/os/mutex.h"
#include "xStl/os/lock.h"
#include "XDK/kernel.h"
#include "XDK/atexit.h"

/// Inits the singleton
cAtExit* cAtExit::m_instance = NULL;

void cAtExit::init()
{
    CHECK(m_instance == NULL);
    m_instance = new cAtExit();
}

void cAtExit::exit()
{
    // Destruct objects
    getInstance().destroyObjects();
    // Free memory
    delete m_instance;
    m_instance = NULL;
}

cAtExit& cAtExit::getInstance()
{
    if (m_instance == NULL)
    {
        // Throw exception
        XSTL_THROW(cException, EXCEPTION_FAILED);
    }
    return *m_instance;
}


void cAtExit::addFunction(ATEXITFUNC newFunction)
{
    cLock lock(m_atExitProtector);
    m_atExitList.insert(newFunction);
}

void cAtExit::destroyObjects()
{
    // Keep the IRQL level for the constructed object at PASSIVE_LEVEL
    cList<ATEXITFUNC> list;
    copyAtexitObjects(list);

    for (cList<ATEXITFUNC>::iterator i = list.begin();
         i != list.end();
         i++)
    {
        // Safe call to the destructor.
        XSTL_TRY
        {
            (*i)();
        }
        XSTL_CATCH_ALL
        {
            TRACE(TRACE_VERY_HIGH, "ATEXIT: Destructor throw exception!");
        }
    }
}

void cAtExit::copyAtexitObjects(cList<ATEXITFUNC>& list)
{
    cLock lock(m_atExitProtector);
    list = m_atExitList;
}

int __cdecl atexit(void (__cdecl *function)(void))
{
    cAtExit::getInstance().addFunction(function);
    return true;
}
