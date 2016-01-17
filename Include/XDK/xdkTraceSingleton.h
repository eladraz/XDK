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

// For debug mode only
#ifdef _DEBUG

#ifndef __XDK_TRACESIGNLETON_H
#define __XDK_TRACESIGNLETON_H

/*
 * xdkTraceSingleton.h
 *
 * Declare the singleton object which acts as a respository for global trace messages
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/xdkTrace.h"

/*
 * This class holds the trace-messages for the XDK driver
 */
class cXdkTraceSingleton {
public:
    /*
     * The singleton which holds the messages for the XDK.
     *
     * The function construct the singleton in first use by using operator new.
     * In order to safe free system resource make sure that the C++ library
     * calls to the 'destroyTraceObject' function.
     */
    static cXdkTraceSingleton& getInstance();

    /*
     * Free all resources allocated by this singleton.
     * Called by the unload process.
     */
    static void destroyTraceObject();

    // The maximum number of messages
    enum { MAX_MESSAGES = 1024 * 20 };

    // The list of elements
    cXdkTrace m_traceObject;

    // The singleton object
    static cXdkTraceSingleton* m_singleton;

private:
    /*
     * Singleton constructor.
     * Builds the main trace repository module.
     */
    cXdkTraceSingleton();
};


#endif // __XDK_TRACESIGNLETON_H
#endif // _DEBUG