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
 * typeInfo.cpp
 *
 * Implements the type_info class.
 * The type_info is the class resopnsianle for run-time information. The class
 * doing so by giving unique names to each class. This class is constructed by
 * the compiler only.
 *
 * class type_info {
 * public:
 *     _CRTIMP virtual ~type_info();
 *     _CRTIMP int operator==(const type_info& rhs) const;
 *     _CRTIMP int operator!=(const type_info& rhs) const;
 *     _CRTIMP int before(const type_info& rhs) const;
 *     _CRTIMP const char* name() const;
 *     _CRTIMP const char* raw_name() const;
 * private:
 *     void *_m_data;
 *     char _m_d_name[1];
 *     type_info(const type_info& rhs);
 *     type_info& operator=(const type_info& rhs);
 * };
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

#ifdef XSTL_CEDRIVER
    // eMbedded Visual C++ 4.0 doesn't have the include file
    #include "ce/typeInfo.h"
#else
    // Visual C++ 6/.NET has the right include file in thier standard path
    #include <typeinfo.h>
#endif

type_info::~type_info()
{
}

#if (_MSC_VER >= 1600)
   // VS 2010 returns bool
   #define RET int

   __type_info_node __type_info_root_node;
#else
   #define RET int
#endif

RET type_info::operator == (const type_info& other) const
{
    return (strcmp(name(), other.name()) == 0);
}

RET type_info::operator != (const type_info& other) const
{
    return (strcmp(name(), other.name()) != 0);
}

int type_info::before(const type_info& other) const
{
    return (strcmp(name(), other.name()) < 0);
}

const char* type_info::name(
                                #if (_MSC_VER >= 2600)
                                    __type_info_node* __ptype_info_node
                                #endif
                           ) const
{
    return
        #if (_MSC_VER >= 2600)
            _M_d_name;
        #else
            _m_d_name;
        #endif
}

const char* type_info::raw_name() const
{
    return
        #if (_MSC_VER >= 2600)
            _M_d_name;
        #else
            _m_d_name;
        #endif
}
