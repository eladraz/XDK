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
 * IdtRouterHooker.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/lock.h"
#include "xStl/data/datastream.h"
#include "xStl/except/exception.h"
#include "xStl/stream/traceStream.h"
#include "xdk/utils/exitCounter.h"
#include "xdk/ehlib/ehlib.h"
#include "xdk/utils/processorLock.h"
#include "xdk/hooker/Processors/ia32/idt/IdtRouterHooker.h"

IdtRouterHooker::IdtRouterHooker(uint startRange,
                                 uint endRange) :
    m_executingCount(0),
    m_idtHooker(NULL),
    m_exitFlag(false)
{
    initList();
    InterruptListenerPtr thisHandler(this, SMARTPTR_DESTRUCT_NONE);
    m_idtHooker = IdtTableHookerPtr(new IdtTableHooker(thisHandler,
        startRange,
        endRange));
}

IdtRouterHooker::~IdtRouterHooker()
{
    // First remove the hooks and than remove the hooker object
    pauseRouting();
    // Wait until all instance will be invalid.
    waitUntilAllInterruptsExecuted();
    // No it's safe to delete all handlers
    destroyList();
}

void IdtRouterHooker::initList()
{
    cProcessorLock processorLock;
    cLock lock(processorLock);
    m_handlers = new cList<InterruptListenerPtr>();
}

void IdtRouterHooker::destroyList()
{
    delete m_handlers;
}

void IdtRouterHooker::registerNewHandler(const InterruptListenerPtr& newHandler)
{
    pauseRouting();
    waitUntilAllInterruptsExecuted();
    appendListenerHandlerSafeMemory(newHandler);
    resumeRouting();
}

void IdtRouterHooker::appendListenerHandlerSafeMemory(
    const InterruptListenerPtr& newHandler)
{
    cProcessorLock processorLock;
    cLock lock(processorLock);
    m_handlers->append(newHandler);
}

void IdtRouterHooker::registerFirstNewHandler(const InterruptListenerPtr& newHandler)
{
    pauseRouting();
    waitUntilAllInterruptsExecuted();
    insertFirstListenerHandlerSafeMemory(newHandler);
    resumeRouting();
}

void IdtRouterHooker::insertFirstListenerHandlerSafeMemory(
    const InterruptListenerPtr& newHandler)
{
    cProcessorLock processorLock;
    cLock lock(processorLock);
    m_handlers->insert(m_handlers->begin(), newHandler);
}

void IdtRouterHooker::unregisterHandler(InterruptListener* object)
{
    pauseRouting();
    waitUntilAllInterruptsExecuted();

    cList<InterruptListenerPtr>::iterator i = m_handlers->begin();
    for (;i != m_handlers->end(); ++i)
    {
        if ((*i).getPointer() == object)
        {
            m_handlers->remove(i);
            resumeRouting();
            return;
        }
    }
    resumeRouting();

    XSTL_THROW(cException, EXCEPTION_OUT_OF_RANGE);
}

void IdtRouterHooker::pauseRouting()
{
    m_exitFlag = true;
}

void IdtRouterHooker::resumeRouting()
{
    m_exitFlag = false;
}

void IdtRouterHooker::waitUntilAllInterruptsExecuted()
{
    // Wait until all instances will be invalid.
    while (m_executingCount.getValue() > 0);
}

bool IdtRouterHooker::onInterrupt(uint8 vector,
                                  uint processorID,
                                  uint numberOfProcessors,
                                  Registers* regs,
                                  InterruptFrame* interruptFrame,
                                  uint32 errorCode,
                                  KIRQL oldIrql)
{
    if (m_exitFlag)
        return true;

    // Safe exit notifier
    cExitCounterAppender exitLock(m_executingCount);


	// OFER DBG REMOVED FOR DEBUGGING

    // The default returned address
	/*
    XSTL_TRY
    {
	*/
        cList<InterruptListenerPtr>::iterator i = m_handlers->begin();
        for (;i != m_handlers->end(); ++i)
        {
            if (!((*i)->onInterrupt(vector,
                                    processorID,
                                    numberOfProcessors,
                                    regs,
                                    interruptFrame,
                                    errorCode,
                                    oldIrql)))
                return false;
        }
		/*
    }
    XSTL_CATCH_ALL
    {
        traceHigh("IdtRouterHooker: Unknown exception " <<
                  EHLib::getUnknownException() << " occurred during INT " <<
                  HEXBYTE(vector) << endl);
    }
	*/
    return true;
}
