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

#ifndef __TBA_XDK_DRIVERFACTORY_H
#define __TBA_XDK_DRIVERFACTORY_H

/*
 * driverFactory.h
 *
 * The driver-factory is a class which instance driver-objects. The purpose of
 * this class is to generate driver object after all global objects constructed.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "XDK/driver.h"

/*
 * Used to remember the template object. Don't use this class. See
 * cDriverFactory for more information.
 */
class cPrivateDriverFactory {
private:
    // Deny operator = and copy constructor.
    cPrivateDriverFactory(const cPrivateDriverFactory& other);
    cPrivateDriverFactory& operator = (const cPrivateDriverFactory& other);

protected:
    friend class cXDKLibCPP;

    /*
     * Default private constructor. (EMPTY)
     */
    cPrivateDriverFactory() {};

    /*
     * Returns the instance to the global factory.
     */
    static cPrivateDriverFactory& getInstance();

    /*
     * Creates the main driver object.
     */
    virtual cDriver& generateDriverObject() = 0;

    /// The main factory instance
    static cPrivateDriverFactory* m_driverFactory;
};

/*
 * Register the factory inside a global list.
 */
template<class T>
class cDriverFactory : public cPrivateDriverFactory {
public:
    /*
     * Constructor. Register the driver.
     */
    cDriverFactory()
    {
        ASSERT(m_driverFactory == NULL);
        if (m_driverFactory != NULL)
        {
            // There is an instance of the class!
            XSTL_THROW(cException, EXCEPTION_FAILED);
        }
        m_driverFactory = this;
    }

protected:
    // Implements the driver creation using the template information
    virtual cDriver& generateDriverObject()
    {
        static T gDriverObject;
        return gDriverObject;
    }
};

#endif // __TBA_XDK_DRIVERFACTORY_H
