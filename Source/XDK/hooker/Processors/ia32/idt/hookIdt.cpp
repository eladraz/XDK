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
 * hookIdt.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/data/datastream.h"
#include "xStl/except/exception.h"
#include "xStl/except/trace.h"
#include "xStl/stream/traceStream.h"
#include "xStl/os/os.h"
#include "xdk/ehlib/ehlib.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/bugcheck.h"
#include "xdk/hooker/stackObject.h"
#include "xdk/hooker/Processors/ia32/tss32.h"
#include "xdk/hooker/Processors/ia32/segments.h"
#include "xdk/hooker/Processors/ia32/descriptorTable.h"
#include "xdk/hooker/Processors/ia32/idt/hookIdt.h"
#include "xdk/hooker/Processors/ia32/idt/InterruptException.h"

// Initialized global variables
InterruptFrame HookIdt::gPreviousFrameContext[cProcessorUtil::MAX_PROCESSORS_SUPPORT];
Registers HookIdt::gPreviousRegsContext[cProcessorUtil::MAX_PROCESSORS_SUPPORT];

// A global reentry counter for each processor and the class that handles it
uint32 gReentryCounter[cProcessorUtil::MAX_PROCESSORS_SUPPORT] = {0};

/*
 * Increment the global reentry counter for a given processor when created
 * and decrement it when destroyed
 */
class HookIntCounter {
public:
	HookIntCounter(uint processorID) : m_processorID(processorID) { ++gReentryCounter[m_processorID]; };
	~HookIntCounter() { --gReentryCounter[m_processorID]; };
private:
    uint m_processorID;
};

/*
 * IMPORTANT NOTE
 * ==============
 * The code of the constructor and the destructor is running under the
 * assumption that the CPU is protected.
 * CLI and STI instructions therefore will not be invoked.
 */
HookIdt::HookIdt(uint8 vector,
                 InterruptListenerPtr callbackClass) :
    m_vector(vector),
    m_callbackClass(callbackClass),
    m_hookerClass(NULL),
    m_taskHookerClass(NULL)
{
    // Check variables
    CHECK(m_callbackClass.getPointer() != NULL);

   // Get the interrupt descriptor
    IDTDescriptor intDescp(m_idt, vector);

    // Tests the descriptor
    if (!intDescp.isPresent())
    {
        traceHigh("[!] HookIDT: The interrupt " << HEXBYTE(vector) <<
                  " cannot be hooked (Not-present)..." << endl);
		return;
    }

    // Getting the segment registers value
    uint16 fsSegment;
    uint16 dsSegment;
    uint16 gsSegment;

    _asm {
        mov ax, fs
        mov fsSegment, ax
        mov ax, ds
        mov dsSegment, ax
        mov ax, gs
        mov gsSegment, ax
    }

    // Hook task-gate
    if (intDescp.getIntType() == IDTDescriptor::IDT_TASK_GATE)
    {
		traceHigh("[!] HookIDT: skipped Hooking gate " << HEXBYTE(vector) << endl);
		return;
    }

    // Gets the offset
    uint32 originalAddress = intDescp.getOffset();

    // Prepare the stub
    #ifdef SHORTPATH
    // Shortpath version - Use a stub that handles only INT3
    m_hookerClass = InterruptHookerPtr(new InterruptHooker(
        getPtr(originalAddress),
        (void*)kernelFunctionStubInt3,
        vector,
        this,
        gsSegment,
        fsSegment,
        dsSegment));
    #else
    // Regular version - Use a stub that handles all interrupts
    m_hookerClass = InterruptHookerPtr(new InterruptHooker(
        getPtr(originalAddress),
        (void*)kernelFunctionStub,
        vector,
        this,
        gsSegment,
        fsSegment,
        dsSegment));
    #endif // SHORTPATH

    // Trace the message out
    cString hookMessage;
    hookMessage << "[*] HookIDT [" <<
                   (cProcessorUtil::getCurrentProcessorNumber() + 1) <<
                   "\\" << cProcessorUtil::getNumberOfProcessors() <<
                   "]: Hooking vector: " << HEXBYTE(vector) <<
                   " [" << HEXDWORD(originalAddress) << "] <=> " <<
                   HEXDWORD(getNumeric(m_hookerClass->getFunction())) << endl;
    traceLow(hookMessage);

    // Create a dummy descriptor identical to the original Interrupt Descriptor Entry
    IDTDescriptor hookedDesc(intDescp);
    // Change the offset to be point out to the hooked function
    hookedDesc.changeOffset(getNumeric(m_hookerClass->getFunction()));

    // And perform the actual hook.
    m_idt.getBase()[m_vector] = hookedDesc.getDescriptor();
}

/*
 * IMPORTANT NOTE
 * ==============
 * The code of the constructor and the destructor is running under the
 * assumption that the CPU is protected.
 * CLI and STI instructions therefore will not be invoked.
 */
HookIdt::~HookIdt()
{
    // Get the interrupt descriptor
    IDTDescriptor intDescp(m_idt, m_vector);

    // Test for interrupt task-gate Vss. interrupt gate and trap gate
    if (!m_hookerClass.isEmpty())
    {
        // Restore the original descriptor
        intDescp.changeOffset(
            getNumeric(m_hookerClass->getOriginalInterruptHandler()));
        // Protect the CPU
        m_idt.getBase()[m_vector] = intDescp.getDescriptor();

        //traceLow("[*] HookIDT: Removed handler" << endl);
    }
}

bool HookIdt::isErrorCodeTrap(uint8 vector)
{
    switch (vector)
    {
    case 8:  // Double fault exception. Always zero
    case 10: // Invalid TSS exception
    case 11: // Segment not present
    case 12: // Stack Fault.
    case 13: // General Protection
    case 14: // Page-fault. uses also CR2
    case 17: // Alignment check exception. Always zero
        return true;
    }

    return false;
}

void __fastcall HookIdt::throwInterruptException(uint8 vector,
                                                 uint32 data)
{
    // Just throw the exception
    throw InterruptException(vector, data);
}

bool HookIdt::interruptSetIrql(KIRQL &oldIrql,
                               uint8 vector,
                               uint& irqlFlags)
{
    // If and only if the interrupt flag is set, raise the IRQL in order to
    // avoid double fault.
    // If a software interrupt was made the previous IRQL will identify it by
    // value of 'TBA_INTERRUPT_IRQL'
    if (cProcessorUtil::isInterruptModeIrql())
    {
        // Interrupt from interrupt
        oldIrql = TBA_INTERRUPT_IRQL;

        // Returning false will cause an exception.
        return false;
    } else
    {
        oldIrql = cProcessorUtil::setIrql(TBA_INTERRUPT_IRQL);

        // Find race between interrupt and user lock
        #ifdef _DEBUG
        if (oldIrql == TBA_INTERRUPT_IRQL)
            cBugCheck::bugCheck(0xDEAD0142, vector, 0xDEAD0140,
                                            oldIrql, 0xDEAD0140);
        #endif
        return true;
    }
}

void HookIdt::interruptRestoreIrql(uint flags, KIRQL oldIrql)
{
    cProcessorUtil::setIrql(RETURN_FROM_INTERRUPT_IRQL);
}

bool HookIdt::executeHandler(HookIdt* thisObject,
                             bool reuseStack,
                             uint8 vector,
                             Registers* generalRegisters,
                             InterruptFrame* interruptFrame,
                             uint32 errorCode,
                             KIRQL oldIrql)
{
	/*
    XSTL_TRY
    {
	*/
        if (reuseStack)
        {
            return thisObject->m_callbackClass->onInterrupt(vector,
                cProcessorUtil::getCurrentProcessorNumber(),
                cProcessorUtil::getNumberOfProcessors(),
                generalRegisters,
                interruptFrame,
                errorCode,
                oldIrql);
        } else
        {
            // Execute handler using new stack
            StackObject interruptStack(STACK_MIN_SIZE);
            InterruptStackCaller caller(thisObject->m_callbackClass,
                vector,
                generalRegisters,
                interruptFrame,
                errorCode,
                oldIrql);
            return (bool)getNumeric(interruptStack.spawnNewStackJob(
                cCallbackPtr(&caller, SMARTPTR_DESTRUCT_NONE),
                NULL));
        }
		/*
    }

	XSTL_CATCH_ALL
    {
        traceHigh("[!] HookIdt catch all: Unknown exception " <<
            EHLib::getUnknownException() << " occurred during INT " <<
            HEXBYTE(vector) << endl);
    }
	*/
    return true;
}

uint64 kernelFunctionStubCalculateRet(uint32 eflags, bool ret)
{
    // Handle the return-type.
    uint64 returnFlagsAndCode = (ret) ? 1 : 0;
    uint64 flags = eflags;
    flags = (flags << 32);
    returnFlagsAndCode+= flags;
    return returnFlagsAndCode;
}

static uint32 gIntHandleCounter = 0;
static Registers* savedRegs = NULL;
static Registers savedRegsBak = {0};

/*
 * Various structures and object for debugging:
 *  bpRec - A struct containing information about a handled interrupt
 *  reEntry - A struct containing information about interrupt reentry
 *  lastVector - An object keeping track of the interrupts that have
 *               been handled by each processor so far
 *  lastVectorCount - The current indexes in lastVector for each processor
 *  lastReentry - An object keeping track of reentries we encounterd for each
 *                processor so far
 *  lastReentryCount - The current indexes in lastReentry for each processor
 */
#ifdef HOOK_IDT_DEBUG
struct bpRec {
	uint32 eip;
	uint32 dest;
	uint32 op_size;
	InterruptFrame frame;
	Registers regs;
    bool is_call;
    bool is_attack;
    bool is_found;
    uint8 processor;
    uint32 progress;
    uint8 op_byte;
    uint8 action;
};

struct reEntry {
    uint32 eip;
};

struct bpRec lastVector[cProcessorUtil::MAX_PROCESSORS_SUPPORT][0x100] = {0};
uint32 lastVectorCount[cProcessorUtil::MAX_PROCESSORS_SUPPORT] = {0};
struct reEntry lastReentry[cProcessorUtil::MAX_PROCESSORS_SUPPORT][0x100]=  {0};
uint32 lastReentryCount[cProcessorUtil::MAX_PROCESSORS_SUPPORT] = {0};
#endif // HOOK_IDT_DEBUG

extern bool renter;

uint64 __stdcall HookIdt::kernelFunctionStub(void* context,
                                             uint32 eflags,
                                             uint8 vector,
                                             Registers* generalRegisters)
{
    uint processorID = cProcessorUtil::getCurrentProcessorNumber();
    HookIntCounter cnt(processorID);
	InterruptFrame savedFrame = {0};

	// Fix ESP stack to enable stack shift for call / ret implementation
	generalRegisters->esp_orig = generalRegisters->frame_holder3;

#ifdef HOOK_IDT_DEBUG
    lastVector[processorID][lastVectorCount[processorID]].progress = 0;
#endif // HOOK_IDT_DEBUG

	// Check re-entry
	if (gReentryCounter[processorID] > 1)
	{
        //if (!renter)

        #ifdef HOOK_IDT_DEBUG
        lastVector[processorID][lastVectorCount[processorID]].progress = 100;
        lastVectorCount[processorID] = (lastVectorCount[processorID] + 1) %
                                       (sizeof(lastVector[processorID]) /
                                       sizeof(lastVector[processorID][0]));
        lastReentry[processorID][lastReentryCount[processorID]].eip =
                                        ((InterruptFrame* )(generalRegisters + 1))->eip;
        lastReentryCount[processorID] = (lastReentryCount[processorID] + 1) %
                                        (sizeof(lastReentry[processorID]) /
                                        sizeof(lastReentry[processorID][0]));
        #endif // HOOK_IDT_DEBUG

		return kernelFunctionStubCalculateRet(eflags, true);
	}

    #ifdef HOOK_IDT_DEBUG
    lastVector[processorID][lastVectorCount[processorID]].progress = 1;
    #endif // HOOK_IDT_DEBUG

	gIntHandleCounter += 1;
	savedRegs = generalRegisters;
	savedRegsBak = *generalRegisters;

	uint32 origEsp = generalRegisters->esp;

	//__debugbreak();

	bool shouldExecuteOriginalHandler = true;

    // Test for stack size space
    bool stackBigEnough = true;

    // Calculate the position of the interrupt frame
    InterruptFrame* interruptFrame = (InterruptFrame* )(generalRegisters + 1);
    uint32 errorCode = 0;
    if (isErrorCodeTrap(vector))
    {
        errorCode = *((uint32*)interruptFrame);
        interruptFrame = (InterruptFrame*)(((uint32*)interruptFrame) + 1);
    }

	savedFrame = *interruptFrame;

    #ifdef HOOK_IDT_DEBUG
    lastVector[processorID][lastVectorCount[processorID]].eip = interruptFrame->eip;
	lastVector[processorID][lastVectorCount[processorID]].frame = *interruptFrame;
	lastVector[processorID][lastVectorCount[processorID]].regs = *generalRegisters;
    lastVector[processorID][lastVectorCount[processorID]].processor = processorID;
    #endif // HOOK_IDT_DEBUG

    // Changes the current IRQL
    KIRQL oldIrql;
    uint irqlFlags;
    if (!interruptSetIrql(oldIrql, vector, irqlFlags))
    {
        __debugbreak();

        // Test for special cases
        if (vector == 14)
        {
            // Save & clear the content of CR2 - Page Fault Linear Address
            _asm
            {
                mov eax, cr2
                mov errorCode, eax
                xor eax, eax
                mov cr2, eax
            }
        }

        // Important NOTE!
        // When we are returning from interrupt with an error-code, the error
        // code should have being poped. We are making any effort to keep the
        // original error-code since the operating system will executed normally
        // (The code near normal return shows how to handle that)
        // In our case we are going to throw an exception so fixing the stack is
        // not necessary. What we should do is caused a normal IRET which can
        // easily being accomplished using the following syntax.
        interruptFrame = (InterruptFrame*)(generalRegisters + 1);


        // Maybe the context was changed (e.g. the cs/fs/gs/ds)
        // Also check what was the previous context
        *interruptFrame =
            gPreviousFrameContext[processorID];
        *generalRegisters =
            gPreviousRegsContext[processorID];

        #ifdef HOOK_IDT_DEBUG
        lastVector[processorID][lastVectorCount[processorID]].progress = 2;
        lastVectorCount[processorID] = (lastVectorCount[processorID] + 1) %
                                       (sizeof(lastVector[processorID]) /
                                       sizeof(lastVector[processorID][0]));
        #endif // HOOK_IDT_DEBUG

        // The important don't touch value should be the eflags
        // Otherwise the InterruptFlag will not be valid!
        interruptFrame->eflags = eflags;

        // Change registers
        interruptFrame->eip = getNumeric(&throwInterruptException);
        generalRegisters->ecx = vector;
        generalRegisters->edx = errorCode;
        // Deny recursion, and throw exception instead.
        return kernelFunctionStubCalculateRet(eflags,
                                              false);
    }

    // TODO! Optimized this code
    gPreviousFrameContext[processorID] =
        *interruptFrame;
    gPreviousRegsContext[processorID] =
       *generalRegisters;

    // Fix the NT_TIB
    uint oldExceptionHandler;
    _asm {
        mov eax, fs:[0]
        mov oldExceptionHandler, eax
        mov eax, 0FFFFFFFFh
        mov fs:[0], eax
    }

    #ifdef HOOK_IDT_DEBUG
    lastVector[processorID][lastVectorCount[processorID]].progress = 3;
    #endif // HOOK_IDT_DEBUG

    /*
     * TODO!
     *
    int leftStack = 0;
    _asm {
    // Get the current stack location
    mov eax, esp
    // Subtract the end of stack.
    sub eax, fs:[8]
    mov leftStack, eax
    }
    // Safety!!!
    if (leftStack < 0x400) //STACK_MIN_SIZE)
        stackBigEnough = false;
    /**/

    if ((context != NULL) &&
        (((HookIdt*)context)->m_callbackClass.getPointer() != NULL))
    {
		// TODO: For some reason, ESP has 3 extra dwords,
		// This is the lowest place i've found that handles interrupts.
		// so i put a temporary fix in here.
		// but in the future we need to understand the meaning of this
		// and give it a proper fix.
		generalRegisters->esp += 0x0C + 0x10; // !YOAV! added an extra dw

        shouldExecuteOriginalHandler = executeHandler((HookIdt*)context,
                       stackBigEnough,
                       vector,
                       generalRegisters,
                       &savedFrame,//interruptFrame, //!YOAV!
                       errorCode,
                       oldIrql);
    } else
    {
        #ifdef HOOK_IDT_DEBUG
        lastVector[processorID][lastVectorCount[processorID]].progress = 4;
        #endif // HOOK_IDT_DEBUG
        traceHigh("HookIdt: Invalid object handle occurred during INT " <<
                  HEXBYTE(vector) << endl);
    }

    // Restore the NT_TIB
    _asm {
        mov eax, oldExceptionHandler
        mov fs:[0], eax
    }
    interruptRestoreIrql(irqlFlags, oldIrql);

    // If there is an error-code and the code should be refault, a special
    // return should be enabled, since the error-code must be take out from
    // the stack
    if (!shouldExecuteOriginalHandler &&
        isErrorCodeTrap(vector))
    {
        // Copy the general-registers and the return address 4 bytes lower.
        // STACK:
        //  FFFF     EIP  | interruptFrame
        //        CS NOP  | interruptFrame
        //        EFLAGS  | interruptFrame
        //         Error  | CPU error-code                 <= destinationFromEnd
        //        PUSHAD  | InterruptHooker stub registers
        //
        //          arg1  | void* context,
        //          arg2  | uint32 eflags,
        //          arg3  | uint8 vector,
        //          arg4  | Registers* generalRegisters
        //
        //           EIP  | kernelFunctionStub return address
        //       old-EBP  | kernelFunctionStub push ebp; mov ebp, esp      <=EBP
        //  0000     ???  | kernelFunctionStub local-variable
        uint32* destinationFromEnd = ((uint32*)(interruptFrame)) - 1;
        uint count = sizeof(Registers) / sizeof(uint32);
        for (uint i = 0; i < count; i++)
        {
            *destinationFromEnd = *(destinationFromEnd - 1);
            destinationFromEnd--;
        }

        // Calculate the stub parameters
        uint64 ret64 =
            kernelFunctionStubCalculateRet(eflags, shouldExecuteOriginalHandler);
        uint32 eaxValue = (uint32)((ret64 >>  0) & 0xFFFFFFFF);
        uint32 edxValue = (uint32)((ret64 >> 32) & 0xFFFFFFFF);

        #ifdef HOOK_IDT_DEBUG
        lastVector[processorID][lastVectorCount[processorID]].progress = 5;
        lastVectorCount[processorID] = (lastVectorCount[processorID] + 1) %
                                       (sizeof(lastVector[processorID]) /
                                       sizeof(lastVector[processorID][0]));
        #endif // HOOK_IDT_DEBUG

        // Change the stack and returns
        _asm {
            // Get the return parameters
            mov eax, eaxValue
            mov edx, edxValue
            // Revert the stack
            mov esp, ebp
            // Revert the old-ebp, unneeded
            pop ebp
            // The stack points to the return address and now we return, revert
            // the arguments and another 4 bytes of the error-code.
            retn 0x14
        }
    }

	if (origEsp + 0x10 != generalRegisters->esp)
	{
		// If the stack pointer has moved throught the interrupt handler we must handle this,
		// which means we must move the interrupt frame to the corrent place and return from
		// interrupt.

		uint32 delta = generalRegisters->esp - (origEsp + sizeof(InterruptFrame) + 0x10);
		void * endFrame = (void *)(generalRegisters->esp_orig + delta);
		cOS::memcpy(endFrame, (void *)&savedFrame, sizeof(InterruptFrame));
		generalRegisters->esp_orig = (uint32)endFrame;
        #ifdef HOOK_IDT_DEBUG
        lastVector[processorID][lastVectorCount[processorID]].progress = 6;
        #endif // HOOK_IDT_DEBUG
	}

    #ifdef HOOK_IDT_DEBUG
    lastVector[processorID][lastVectorCount[processorID]].progress = 7;
    lastVectorCount[processorID] = (lastVectorCount[processorID] + 1) %
                                   (sizeof(lastVector[processorID]) /
                                   sizeof(lastVector[processorID][0]));
    #endif HOOK_IDT_DEBUG

    // Normal return
    return kernelFunctionStubCalculateRet(eflags, shouldExecuteOriginalHandler);
}

uint64 __stdcall HookIdt::kernelFunctionStubInt3(void* context,
                                                 uint32 eflags,
                                                 uint8 vector,
                                                 Registers* generalRegisters)
{
    ASSERT(vector == 3);

    uint processorID = cProcessorUtil::getCurrentProcessorNumber();
    HookIntCounter cnt(processorID);
	InterruptFrame savedFrame = {0};

	// Fix ESP stack to enable stack shift for call / ret implementation
	generalRegisters->esp_orig = generalRegisters->frame_holder3;

	// Check reentry
	if (gReentryCounter[processorID] > 1)
		return kernelFunctionStubCalculateRet(eflags, true);

	uint32 origEsp = generalRegisters->esp;

    // Calculate the position of the interrupt frame
    InterruptFrame* interruptFrame = (InterruptFrame* )(generalRegisters + 1);
	savedFrame = *interruptFrame;

    // Change the current IRQL
    KIRQL oldIrql;
    uint irqlFlags;
    if (!interruptSetIrql(oldIrql, vector, irqlFlags))
        __debugbreak();

    // Fix the NT_TIB
    uint oldExceptionHandler;
    _asm {
        mov eax, fs:[0]
        mov oldExceptionHandler, eax
        mov eax, 0FFFFFFFFh
        mov fs:[0], eax
    }

    // Call the interrupt handler
    bool shouldExecuteOriginalHandler = true;
    if ((context != NULL) &&
        (((HookIdt*)context)->m_callbackClass.getPointer() != NULL))
    {
		// TODO: For some reason, ESP has 3 extra dwords,
		// This is the lowest place i've found that handles interrupts.
		// so i put a temporary fix in here.
		// but in the future we need to understand the meaning of this
		// and give it a proper fix.
		generalRegisters->esp += 0x0C + 0x10; // !YOAV! added an extra dw

        shouldExecuteOriginalHandler =
            ((HookIdt*)context)->m_callbackClass->onInterrupt(
                                            vector,
                                            processorID,
                                            cProcessorUtil::getNumberOfProcessors(),
                                            generalRegisters,
                                            &savedFrame,
                                            0,
                                            oldIrql);
    } else
        traceHigh("[!] HookIdt: Invalid object handle occurred during INT " <<
                  HEXBYTE(vector) << endl);

    // Restore the NT_TIB and the IRQL
    _asm {
        mov eax, oldExceptionHandler
        mov fs:[0], eax
    }
    interruptRestoreIrql(irqlFlags, oldIrql);

	if (origEsp + 0x10 != generalRegisters->esp)
	{
		/* If the stack pointer has moved throughout the interrupt handler we must handle this,
		   which means we must move the interrupt frame to the corrent place and return from
		   interrupt */

		uint32 delta = generalRegisters->esp - (origEsp + sizeof(InterruptFrame) + 0x10);
		void * endFrame = (void *)(generalRegisters->esp_orig + delta);
		cOS::memcpy(endFrame, (void *)&savedFrame, sizeof(InterruptFrame));
		generalRegisters->esp_orig = (uint32)endFrame;
	}

    // Normal return
    return kernelFunctionStubCalculateRet(eflags, shouldExecuteOriginalHandler);
}

void __stdcall HookIdt::kernelInterruptTaskStub(void* context,
                                                uint8 vector)
{
    // Changes the current IRQL
    KIRQL oldIrql;
    uint irqlFlags;

    if (!interruptSetIrql(oldIrql, vector, irqlFlags))
        return;

    HookIdt* thisObject = (HookIdt*)context;

    if ((thisObject != NULL) &&
        (thisObject->m_callbackClass.getPointer() != NULL))
    {
        XSTL_TRY
        {
            // Prepare the state
            Registers registers;
            InterruptFrame interruptFrame;
            memset(&interruptFrame, 0, sizeof(InterruptFrame));
            memset(&registers, 0, sizeof(Registers));

            TaskStateSegment32* currentTask =
                TaskStateSegment::getTSS32(TaskStateSegment::getTaskRegister());
            TaskStateSegment32* previousTask =
                TaskStateSegment::getTSS32(currentTask->previousTaskLink);

            interruptFrame.cs = previousTask->cs;
            interruptFrame.eflags = previousTask->eflags;
            interruptFrame.eip = previousTask->eip;

            registers.eax = previousTask->eax;
            registers.ebx = previousTask->ebx;
            registers.ecx = previousTask->ecx;
            registers.edx = previousTask->edx;
            registers.esi = previousTask->esi;
            registers.edi = previousTask->edi;
            registers.ebp = previousTask->ebp;
            registers.esp = previousTask->esp;
            registers.ds = previousTask->ds;
            registers.es = previousTask->es;
            registers.fs = previousTask->fs;
            registers.gs = previousTask->gs;


            // Call the handler
            thisObject->m_callbackClass->onInterrupt(vector,
                cProcessorUtil::getCurrentProcessorNumber(),
                cProcessorUtil::getNumberOfProcessors(),
                &registers,
                &interruptFrame,
                0,  // Unknown error-code.
                oldIrql);
        }
        XSTL_CATCH_ALL
        {
            traceHigh("HookIdtTrap: Unknown exception " <<
                EHLib::getUnknownException() << " occurred during INT " <<
                HEXBYTE(vector) << endl);
        }
    } else
    {
        traceHigh("HookIdtTrap: Invalid object handle occurred during INT " <<
            HEXBYTE(vector) << endl);
    }

    interruptRestoreIrql(irqlFlags, oldIrql);
}

HookIdt::InterruptStackCaller::InterruptStackCaller(
            const InterruptListenerPtr& callbackClass,
            uint8 vector,
            Registers* regs,
            InterruptFrame* interruptFrame,
            uint32 errorCode,
            KIRQL oldIrql) :
    m_callbackClass(callbackClass),
    m_vector(vector),
    m_regs(regs),
    m_interruptFrame(interruptFrame),
    m_errorCode(errorCode),
    m_oldIrql(oldIrql)
{
}

void* HookIdt::InterruptStackCaller::call(void* args)
{
    XSTL_TRY
    {
        return getPtr(m_callbackClass->onInterrupt(m_vector,
            cProcessorUtil::getCurrentProcessorNumber(),
            cProcessorUtil::getNumberOfProcessors(),
            m_regs,
            m_interruptFrame,
            m_errorCode,
            m_oldIrql));
    }
    XSTL_CATCH_ALL
    {
        cBugCheck::bugCheck(0x666, 0xDEAD, 0x111, m_oldIrql, m_vector);
        return getPtr(true);
    }
}

HookIdt::InterruptHooker::InterruptHooker(void* originalHandler,
                                          void* interruptHandler,
                                          uint8 vector,
                                          void* context,
                                          uint16 gsSegmentValue,
                                          uint16 fsSegmentValue,
                                          uint16 dsSegmentValue) :
    m_originalHandler(getNumeric(originalHandler)),
    m_interruptHandler(getNumeric(interruptHandler)),
    m_context(getNumeric(context)),
    m_vector(vector),

    // Save context
	m_pushEsp(0x54), //!YOAV!
	m_pushEsp2(0x54), //!YOAV!
	m_pushEsp3(0x54), //!YOAV!
	m_pushEsp4(0x54), //!YOAV!
    m_pusha(0x60),
    m_pushGs(0xA80F),
    m_pushFs(0xA00F),
    m_pushSs(0x16),
    m_pushDs(0x1E),
    m_pushEs(0x06),

    // Change segment registers
    m_movAx(0xB866),
    m_fsValue(fsSegmentValue),
    m_66prefix(0x66),
    m_movFsAx(0xE08E),
    m_movAx1(0xB866),
    m_dsValue(dsSegmentValue),
    m_66prefix1(0x66),
    m_movDsAx(0xD88E),
    m_66prefix2(0x66),
    m_movEsAx(0xC08E),
    m_movAx2(0xB866),
    m_gsValue(gsSegmentValue),
    m_66prefix3(0x66),
    m_movGsAx(0xE88E),
    // Invoked hooked function
    m_pushVector(0x6A),
    m_pushf(0x9C),
    m_cli(0xFA),
	m_cld(0xFC),
    m_pushContext(0x68),
    m_pushEsp1(0x54),
    m_movEax(0xB8),
    m_callEax(0xD0FF),
    // Restore segment registers
    m_popEs(0x07),
    m_popDs(0x1F),
    m_popSsTODO(0x59),
    m_popFs(0xA10F),
    m_popGs(0xA90F),
    // Test the result
    m_testAl(0xC084),
    m_jnz(0x0575),    //!YOAV! m_jnz(0x0475),      m_jnz(0x0575)
    // al = 0, no jump,   Don't execute original API
    m_pushEdx1(0x52),
    m_popf1(0x9D),
    m_popad1(0x61),
	m_popEsp1(0x5C), //!YOAV!
    m_iret(0xCF),
    // al!= 0, jmp,       Execute original API
    m_pushEdx2(0x52),
    m_popf2(0x9D),
    m_popad2(0x61),
	m_popEsp2(0x5C), //!YOAV!
    m_push(0x68),
    m_ret(0xC3)
{
}

void* HookIdt::InterruptHooker::getFunction() const
{
    //return (void*)(&m_pusha); !YOAV!
	return (void*)(&m_pushEsp);
}

void* HookIdt::InterruptHooker::getOriginalInterruptHandler() const
{
    return getPtr(m_originalHandler);
}

void* HookIdt::InterruptHooker::getInterruptHandler() const
{
    return getPtr(m_interruptHandler);
}

HookIdt::InterruptTaskGateHooker::InterruptTaskGateHooker(void* originalHandler,
                                                          void* interruptHandler,
                                                          uint8 vector,
                                                          void* context,
                                                          uint16,
                                                          uint16,
                                                          uint16,
                                                          uint16) :
    m_originalHandler(getNumeric(originalHandler)),
    m_interruptHandler(getNumeric(interruptHandler)),
    m_context(getNumeric(context)),
    m_vector(vector),

    m_pushVector(0x6A),
    m_pushContext(0x68),
    m_movEax(0xB8),
    m_callEax(0xD0FF),
    m_push(0x68),
    m_ret(0xC3)
{
}

void* HookIdt::InterruptTaskGateHooker::getFunction() const
{
    return (void*)(&m_pushVector);
}

void* HookIdt::InterruptTaskGateHooker::getOriginalInterruptHandler() const
{
    return getPtr(m_originalHandler);
}

