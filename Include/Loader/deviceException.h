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

#ifndef __TBA_XDK_LOADER_EXCEPTION_H
#define __TBA_XDK_LOADER_EXCEPTION_H

/*
 * deviceException.h
 *
 * Defines the exception types regarding to handling a device
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/except/exception.h"

/*
 * class cDeviceException
 *
 * Device loader exception class. All the device exception are driven from this
 * class.
 */
class cDeviceException : public cException
{
public:
    /*
     * Construct an exception using a string from description
     *
     * msg - The error message
     * id - The error message identfication number
     */
	cDeviceException(char * file, uint32 line, const character* msg, uint32 id) : cException(file, line, msg, id) {};

    /*
     * Construct an exception using a string from description
     *
     * msg - The error message
     */
	cDeviceException(char * file, uint32 line, const character* msg) : cException(file, line, msg) {};

    /*
     * Construct an exception using a number
     *
     * id - The error message identfication number
     */
    cDeviceException(char * file, uint32 line, uint32 id) : cException(file, line, id) {};
};

#endif // __TBA_XDK_LOADER_EXCEPTION_H
