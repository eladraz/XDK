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
 * xdkTrace.cpp
 *
 * Implementation file for nt-ddk (XDK package).
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/exception.h"
#include "xStl/os/lock.h"
#include "XDK/kernel.h"
#include "XDK/xdkTrace.h"

cXdkTrace::cXdkTrace(uint queueQuata) :
    m_queueMaxSize(queueQuata),
    m_queueSize(0)
{
}

void cXdkTrace::addMessage(const cString& message)
{
    // Lock queue
    cLock lock(m_listMutex);

    // Test quata limits
    if (m_queueSize >= m_queueMaxSize)
    {
        XSTL_THROW(cException, EXCEPTION_OUT_OF_MEM);
    }

    // Ok to append. Adds.
    m_traceStrings.append(message);

    // Increase cache counter
    m_queueSize++;
}

bool cXdkTrace::getMessage(cString* outputString)
{
    ASSERT(outputString != NULL);

    // Lock queue
    cLock lock(m_listMutex);

    // Polls the first element
    cList<cString>::iterator i = m_traceStrings.begin();

    // Test whether the list is empty
    if (i == m_traceStrings.end())
    {
        return false;
    }

    // Polls the element
    *outputString = *i;
    // And delete the position
    m_traceStrings.remove(i);
    m_queueSize--;

    // Successfully retrieve element
    return true;
}

uint cXdkTrace::getQueueMessageLength()
{
    // Lock queue
    cLock lock(m_listMutex);
    // Polls the first element
    cList<cString>::iterator i = m_traceStrings.begin();
    // Test whether the list is empty
    if (i == m_traceStrings.end())
    {
        return 0;
    }
    return ((*i).length() + 1) * sizeof(character);
}

uint cXdkTrace::getMessageCount()
{
    // This function cannot be thread-safe anyway.
    return m_queueSize;
}
