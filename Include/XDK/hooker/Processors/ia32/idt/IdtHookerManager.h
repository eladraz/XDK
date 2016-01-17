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

#ifndef __TBA_XDK_HOOKER_IA32_IDT_IDTHOOKER_MANAGER_H
#define __TBA_XDK_HOOKER_IA32_IDT_IDTHOOKER_MANAGER_H

/*
 * IdtHookerManager.h
 *
 * Handles multi-driver interrupt-table hooking.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "xdk/hooker/Processors/ia32/idt/IdtRouterHooker.h"

/*
 * A system-wide singleton which controls interrupt hooking API.
 * Using this object will provide solution for wild-west hooking.
 *
 * TODO! This class is implement as an interface only (for now only)!
 */
class IdtHookerManager {
public:
    /*
     * Return handler to the IDT manager
     */
    static IdtHookerManager& getInstance();

    /*
     * Return a pointer to the router object
     */
    IdtRouterHooker& getRouterObject();

    /*
     * Destroy the manager. Used in order to safely destroy the IDT hooking
     * table before the ProcessorThreadsManager will dead-lock.
     */
    void killManager();

private:
    /*
     * Default constructor. This class is a singleton.
     *
     * Try to scan for exist IDTRouterHooker object in other drivers, if found
     * link this manager to a main manager object. Control flow for driver
     * loading/unload etc.. If not found, hook the interrupt table and start
     * dispatch messages.
     */
    IdtHookerManager();

    /// The Hooker referenced objects
    cSmartPtr<IdtRouterHooker> m_routerObjectPtr;
};

#endif // __TBA_XDK_HOOKER_IA32_IDT_IDTHOOKER_MANAGER_H
