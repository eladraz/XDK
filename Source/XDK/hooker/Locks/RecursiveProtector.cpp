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
 * RecursiveProtector.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/mutex.h"
#include "xStl/os/lock.h"
#include "xStl/data/smartptr.h"
#include "xStl/utils/algorithm.h"
#include "xStl/utils/callbacker.h"
#include "xdk/hooker/Locks/RecursiveProtector.h"

// The global list
cList<RecursiveProtector::LockRegion> RecursiveProtector::m_lockedRegions;
// The global list locked
#ifdef XSTL_NTDDK
    cInterruptSpinLock
#else
    cMutex
#endif
    RecursiveProtector::m_lockedRegionsLocked;

RecursiveProtector::RecursiveProtector(uint id1, uint id2,
                                       cCallback& callbackClass) :
    m_isRegionLocked(false),
    m_region(id1, id2)
{
    cLock lock(m_lockedRegionsLocked);
    if (isRegionLocked(m_region))
    {
        // RAISE EXCEPTION!
        callbackClass.call(NULL);
    } else
    {
        m_lockedRegions.append(m_region);
        m_isRegionLocked = true;
    }
}

RecursiveProtector::~RecursiveProtector()
{
    if (m_isRegionLocked)
    {
        // Remove the locked region.
        cLock lock(m_lockedRegionsLocked);
        m_lockedRegions.remove(m_region);
    }
}

bool RecursiveProtector::isRegionLocked(const LockRegion& other)
{
    return (find(m_lockedRegions.begin(),
                 m_lockedRegions.end(),
                 other)
            != m_lockedRegions.end());
}

RecursiveProtector::LockRegion::LockRegion(uint id1, uint id2) :
    m_id1(id1),
    m_id2(id2)
{
}

bool RecursiveProtector::LockRegion::operator == (const LockRegion& other)
{
    return (m_id1 == other.m_id1) && (m_id2 == other.m_id2);
}
