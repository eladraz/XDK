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

#ifndef __TBA_XDK_EHLIB_EHLIBCPP_H
#define __TBA_XDK_EHLIB_EHLIBCPP_H

/*
 * ehlibcpp.h
 *
 * Functions needed for the linker in order to solve C++ problems regarding
 * to the exception-handling library
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"

/*
 * Touch the stack of a function in a page-size.
 * When a stack is larger than 4kb, the page should be touch from higher to
 * bottom to allow the operation-system to commit the reserved stack page.
 *
 * Anyway, This function should not be implement since the kernel stack is
 * very limited (Windows 2000 stack size is 8KB and Windows XP has 16KB)
 */
EXTERNC void __cdecl _chkstk();

/*
 * Called when the /GZ compiler is on. Force a stack check. When the stack
 * pointer is not equal (Stack-overflow).
 * In that case the stack will be reverted using the SEH mechanizm.
 */
EXTERNC void __cdecl _chkesp();

/*
 * void __stdcall `eh vector destructor iterator'(....)
 *
 * This function is called whenever the operator delete[] is called over an
 * array. The function should safely call the destructor of each elelment
 * inside the array.
 *
 * The function has a special mangling: ??_M@YGXPAXIHP6EX0@Z@Z, since we can't
 * produce the exactly same mangling we create a function with similar mangling
 * ?ed@@YGXPAXIHP6GX0@Z@Z and replace the mangling.
 *
 * objectArray  - Pointer to the beginng of the constructed array.
 * objectSize   - The size of a single object element in the array
 * elementCount - The number of elements in the array
 * destructor   - C++ destructor pointer. (Use ecx as a context pointer)
 */
void __stdcall ed(void*  objectArray,
                  uint   objectSize,
                  int    elementsCount,
                  void (__stdcall *destructor)(void*));

/*
 * void __stdcall `eh vector constructor iterator'(...)
 *
 * This function is called whenever the operator new[] is called over an
 * array. The function should safely call the constructor of each elelment
 * inside the array. In case of exception the elements which constructed
 * so far should be destruct.
 *
 * The function has a special mangling: ??_L@YGXPAXIHP6EX0@Z1@Z, since we can't
 * produce the exactly same mangling we create a function with similar mangling
 * ?ec@@YGXPAXIHP6EX0@Z1@Z and replace the mangling.
 *
 * objectArray  - Pointer to the begining of the unconstructed array.
 * objectSize   - The size of a single object element in the array
 * elementCount - The number of elements in the array
 * constructor  - C++ destructor pointer. (Use ecx as a context pointer)
 * destructor   - C++ destructor pointer. (Use ecx as a context pointer)
 */
void __stdcall ec(void*  objectArray,
                  uint   objectSize,
                  int    elementsCount,
                  void (__stdcall *constructor)(void*),
                  void (__stdcall *destructor)(void*));

/*
 * Register the current function to the list of operating system's reciving
 * exception list.
 */
EXTERNC void __cdecl _EH_prolog();

/*
 * Safely destruct an array.
 * See ed for more information.
 */
void __stdcall arrayUnwind(uint8* objectArray,
                           uint   objectSize,
                           int    elementsCount,
                           void*  destructor);


#endif // __TBA_XDK_EHLIB_EHLIBCPP_H
