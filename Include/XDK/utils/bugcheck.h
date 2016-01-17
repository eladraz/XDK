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

#ifndef __TBA_XDK_UTILS_BUGCHECK_H
#define __TBA_XDK_UTILS_BUGCHECK_H

/*
 * bugcheck.h
 *
 * Define a private bug-check routine which might not be implemented by the
 * KeBugCheck routine of microsoft. This routine might be trapped
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xdk/kernel.h"
// For now on
class cBugCheck {
public:
    /*
     * For now the implementation is simple
     */
    static void bugCheck(uint32 code,
                         uint32 paramA = 0,
                         uint32 paramB = 0,
                         uint32 paramC = 0,
                         uint32 paramD = 0)
    {
        #ifndef _KERNEL
            DebugBreak();
        #else
			stackDump(300);
			/*__debugbreak();*/
            KeBugCheckEx(code, paramA,paramB,paramC,paramD);
        #endif // _WIN32
    }

	static void stackDump(uint32 depth)
	{
		uint32 index = 0;
		uint32 * stack = &index;

		DbgPrint("!!Dump Start!!\n");

		while (depth > index)
		{
			DbgPrint("%08x\n", stack[index]);
			index++;
		}

		DbgPrint("!!Dump End!!\n");

		//DbgPrint("!! Ebp walk !!\n");
		//uint32 * frame = NULL;
		//uint32 * next_frame = NULL;

		//_asm{
		//	mov frame, ebp
		//}

		//DbgPrint("EIP = %08x\n", frame[1]);
		//frame = (uint32 *)frame[0];
		//DbgPrint("EIP = %08x\n", frame[1]);
		//frame = (uint32 *)frame[0];
		//DbgPrint("EIP = %08x\n", frame[1]);
		//frame = (uint32 *)frame[0];
		//DbgPrint("EIP = %08x\n", frame[1]);
	}
};

#endif // __TBA_XDK_UTILS_BUGCHECK_H
