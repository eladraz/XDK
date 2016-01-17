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

#ifndef __XDK_TRACE_H
#define __XDK_TRACE_H

/*
 * xdkTrace.h
 *
 * Declare the cXdkTrace module which responisable to store trace messages
 * for the driver. Using this message-queue. The driver can
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/list.h"
#include "xdk/utils/interruptSpinLock.h"

/*
 * The trace class is a storage for messages.
 * This class is a thread-safe.
 */
class cXdkTrace
{
public:
    // The maximum number of messages for default behaviour
    enum { MAX_QUEUE = 0xFFFFFFFF };

    /*
     * Construct a new message queue
     *
     * queueQuata - The limit (total number of message) for the queue
     */
    cXdkTrace(uint queueQuata = MAX_QUEUE);

    /*
     * Appends message to the queue.
     *
     * Throw exception if the queue is full.
     */
    void addMessage(const cString& message);

    /*
     * Pools a message from the queue.
     *
     * outputString - Will be filled with the first waiting message.
     *
     * Return true if the message was pooled. False if no messages are waiting.
     */
    bool getMessage(cString* outputString);

    /*
     * Returns the number of active messages in the queue.
     */
    uint getMessageCount();

    /*
     * Returns the number of bytes which are in used (including the NULL-
     * terminate character) for the queued pending line.
     * 0 when there aren't any messages waiting.
     */
    uint getQueueMessageLength();

private:
    // The queue limits.
    uint m_queueMaxSize;
    // The queue synchrounzier
    cInterruptSpinLock m_listMutex;
    // The string storage queue
    cList<cString> m_traceStrings;
    // Cache value for queue size
    uint m_queueSize;
};

#endif // __XDK_TRACE_H
