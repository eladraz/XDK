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
 * trace.cpp
 *
 * Implementation file for nt-ddk (XDK package).
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xdk/kernel.h"
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/data/char.h"
#include "xStl/data/string.h"
#include "xdk/xdkTrace.h"
#include "xdk/xdkTraceSingleton.h"
#include "xdk/ehlib/ehlib.h"
#include "xdk/utils/processorUtil.h"

#ifdef _DEBUG

cXdkTraceSingleton* cXdkTraceSingleton::m_singleton = NULL;

cXdkTraceSingleton& cXdkTraceSingleton::getInstance()
{
    // For first use inits the singleton
    if (m_singleton == NULL)
    {
        m_singleton = new cXdkTraceSingleton();
    }
    return *m_singleton;
}

cXdkTraceSingleton::cXdkTraceSingleton() :
    m_traceObject(MAX_MESSAGES)
{
}



/*
 * Covert a cString into ASCII value
 *
 * string - The string to convert
 *
 * Return the ASCII string representation. When the return value is no longer
 * in use, the deleteStringAscii function should be called.
 */
char* getStringAscii(const cString& string)
{
    #ifdef XSTL_UNICODE
        // Delete the allocated buffer
        uint len = string.length() + 1;
        char* ret = new char[len];
        cChar::covert2Ascii(ret,
                            len,
                            string.getBuffer());
        return ret;
    #else
        return const_cast<char*>*(((const char*)string.getBuffer()));
    #endif
}

/*
 * Free all resources allocated by 'getStringAscii' in order to convert
 * UNICODE string to ASCII string.
 *
 * string - The return code from 'getStringAscii'
 */
void deleteStringAscii(char* string)
{
    #ifdef XSTL_UNICODE
        // Delete the allocated buffer
        delete[] string;
    #else
        // Nothing to do
    #endif
}

/*
 * Trace a single message to the user console.
 *
 * message - The string to output.
 */
void traceOut(const cString& message)
{
    char* msg = getStringAscii(message);
    try
    {
        DbgPrint("%s", msg);
    }
    catch (...)
    {
        DbgPrint("TRACE: Unknown exception during trace.\n");
    }
    deleteStringAscii(msg);
}

#ifdef _DEBUG
uint32 traceAddError = 0;
#endif

/*
 * The main macro function.
 */
void traceMessage(int trace_level, const cString& message)
{
	if (trace_level >= TRACE_LEVEL)
	{
        // Test the IRQL level.
        if (cProcessorUtil::getCurrentIrql() > PASSIVE_LEVEL)
        {
            // Queue the message
            try
            {
                cXdkTraceSingleton::getInstance().m_traceObject.addMessage(message);
            } catch (...)
            {
            #ifdef _DEBUG
                // The message was lost. Quata limit?
				++traceAddError;
            #endif
            }
        } else
        {
            /*
             * Pools out the messages
             */
            uint currentCount = cXdkTraceSingleton::getInstance().m_traceObject.getMessageCount();

            // Protect the polling
            try
            {
                for (uint i = 0; i < currentCount; i++)
                {
                    cString traceMessage;
                    if (!cXdkTraceSingleton::getInstance().m_traceObject.getMessage(&traceMessage))
                    {
                        // Stop iteratring. Queue is empty
                        break;
                    }

                    traceOut(traceMessage);
                }
            } catch (...)
            {
                // Trace the exception
                traceOut(cString(XSTL_STRING("TRACE: Unknown exception occured: %s\n")) +
                         EHLib::getUnknownException());
            }

            // Trace this message
            traceOut(message);
        }
    }
}

void cXdkTraceSingleton::destroyTraceObject()
{
    if (m_singleton != NULL)
    {
        // Flush all messages
        while (cXdkTraceSingleton::getInstance().m_traceObject.getMessageCount() > 0)
        {
            cString traceMessage;
            if (!cXdkTraceSingleton::getInstance().m_traceObject.getMessage(&traceMessage))
            {
                // Stop iteratring. Queue is empty
                break;
            }

            traceOut(traceMessage);
        }

        delete m_singleton;
    }
}

#endif // _DEBUG

