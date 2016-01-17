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

#ifndef __TBA_XDK_HOOKER_IA32_IDT_HOOKIDT_H
#define __TBA_XDK_HOOKER_IA32_IDT_HOOKIDT_H

/*
 * hookIdt.h
 *
 * Hooks the Interrupt descriptor table.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/smartptr.h"
#include "xStl/utils/callbacker.h"
#include "xdk/kernel.h"
#include "xdk/hooker/Processors/ia32/registers.h"
#include "xdk/hooker/Processors/ia32/idt/idt.h"
#include "xdk/hooker/Processors/ia32/idt/InterruptListener.h"

/*
 * Hooks an interrupt and invoke a callback function.
 *
 * Where the argument is a pointer into the Trap registers, bool indicate
 * whether the original interrupt handler should be called, or the be ignored
 *
 * MULTI-PROCESSOR NOTE:
 *    The class operates within the current processor context. The IDT hooking
 *    is performed by switching the active processor IDT tables.
 *
 *    THE CONSTRUCTOR AND THE DESTRUCTOR MUST BE CALLED AND LOCKED WITHING SINGLE
 *    PROCESSOR CONTEXT!
 *
 *
 * Usage:
 * In order to hook INT3 (Breakpoint trap) use the following code:
 *
 *    Loader:
 *        InterruptListenerPtr myCallback(new Interrupt3Handler());
 *        HookIdt i3(3, myCallback);
 *
 *    Hooker:
 *        class InterruptListener : public cCallback {
 *        public:
 *           virtual bool onInterrupt(uint8 vector,
 *                             Registers* regs) {
 *              intTrace << "INT3 called!";
 *              return true;
 *           }
 *        }
 *
 * While interrupt is executed, the processor may thrown second interrupt.
 * Examples for that can be caused due to: Floating point instruction,
 * access violation instructions and more...
 *
 * In that case, the second interrupt should execute a recursive handler,
 * instead InterruptException is thrown.
 */
class HookIdt {
public:
    /*
     * Constructor. Hooks an interrupt.
     * The code of the constructor and the destructor are running under the
     * assumption that the CPU is protected.
     *
     * vector - The interrupt to be hooked
     * callbackClass - The callback API to be invoked.
     *
     * Throw exception if the 'callbackClass' is NULL pointer function.
     * Throw exception if the IDT entry is not present
     * Throw exception if the IDT entry is TaskGate. (Unknown)
     */
    HookIdt(uint8 vector,
            InterruptListenerPtr callbackClass);

    /*
     * Destructor. Free the interrupt hooking
     * The code of the constructor and the destructor are running under the
     * assumption that the CPU is protected and the processor context is
     * identical to the context called by the constructor.
     */
    ~HookIdt();

    /*
     * Return true if there is an error-code for the trap.
     *
     * vector - The interrupt that was executed
     */
    static bool isErrorCodeTrap(uint8 vector);

private:
    // Deny operator = and copy-constructor
    HookIdt(const HookIdt& other);
    HookIdt& operator = (const HookIdt& other);

    // The number of bytes for minium interrupt stack implementation
    enum { STACK_MIN_SIZE = 0x800 };

    /*
     * While interrupt is executed, the processor may thrown second interrupt.
     * Examples for that can be caused due to: Floating point instruction,
     * access violation instructions and more...
     *
     * In that case, the second interrupt should execute a recursive handler,
     * instead InterruptException is thrown.
     *
     * This method is called within the first interrupt context. The 'kernelStub'
     * detects a second call and IRET to this method, when ECX is pointed to the
     * vector ID and EDX pointed to the 'data'
     */
    static void __fastcall throwInterruptException(uint8 vector,
                                                   uint32 data);

    /*
     * Change the current IRQL according to the executed interrupt and the
     * current processor mode (If the Interrupt-flag is clear or not)
     *
     * oldIrql   - Will be filled with the previous PCR IRQL
     * vector    - The interrupt to executed
     * irqlFlags - Fill the flag which should be transfer back to
     *             interruptRestoreIrql
     *
     * Return true if the interrupt is free to be executed
     * Return false if recursive behaviour was occured.
     */
    static bool interruptSetIrql(KIRQL &oldIrql,
                                 uint8 vector,
                                 uint& irqlFlags);

    /*
     * Returns the state of the processor back to it's original mode, before
     * the 'interruptSetIrql' was called
     */
    static void interruptRestoreIrql(uint flags, KIRQL oldIrql);

    /**
     * Sometimes the interrupt stack callback doesn't enough for operation.
     * In order to fix this problem, InterruptStackCaller execute the interrupt
     * handler code in a context of fixed size stack frame.
     */
    class InterruptStackCaller : public cCallback
    {
    public:
        /*
         * Prepare the call to the interrupt listener.
         * See InterruptListener::onInterrupt
         */
        InterruptStackCaller(const InterruptListenerPtr& callbackClass,
            uint8 vector,
            Registers* regs,
            InterruptFrame* interruptFrame,
            uint32 errorCode,
            KIRQL oldIrql);

        /*
         * Execute the handler function.
         *
         * args - HookIdt* pointer
         */
        virtual void* call(void* args);

    private:
        // The context of the call. See InterruptListener::onInterrupt
        InterruptListenerPtr m_callbackClass;
        uint8 m_vector;
        Registers* m_regs;
        InterruptFrame* m_interruptFrame;
        uint32 m_errorCode;
        KIRQL m_oldIrql;
    };


    /**
     * The function is called by the assembler hook (@see InterruptHooker) as
     * a before handler code. The function can decide whether the original
     * interrupt is execute or not.
     * NOTE: The function is declared as a stdcall since the stack should be
     *       cleared by this function.
     *
     * generalRegisters - Pointer to the current stack. Here is a simple
     *                    description of the stack when the code is execute:
     *
     *  ESP before interrupt was executed  ->   +-+-+-+-+-+-+-+-+   FFFF
     *                                          |    EFLAGS     |
     *                                          +-+-+-+-+-+-+-+-+
     *                                          |       CS      |
     *                                          +-+-+-+-+-+-+-+-+
     *                                          |      EIP      |
     *  ESP after interrupt execution           +-+-+-+-+-+-+-+-+
     *                                          |  Error-code?  |
     *                                          +-+-+-+-+-+-+-+-+
     *                               PUSHAD     |      EAX      |
     *                               command    |      EBX      |
     *                               executed   ~               ~
     *                                          ~      ...      ~
     *                                          ~               ~
     *                   generalRegisters  ->   +-+-+-+-+-+-+-+-+   0000
     *
     * vector  - The interrupt vector number
     * context - The context of the interrupt routine, which is hardcoded
     *           pointer to 'this'
     *
     *
     * Return:
     *        Low 32bit: (EAX)
     *           0 = Original interrupt should be executed.
     *           1 - Original interrupt should not be executed.
     *        High 32bit: (EDX)
     *           The flags to be use for the original interrupt handler
     */
    static uint64 __stdcall kernelFunctionStub(void* context,
                                             uint32 eflags,
                                             uint8 vector,
                                             Registers* generalRegisters);

    /*
     * A "shortpath" version of the kernelFunctionStub, that handles only
     * INT3 interrupts. Meant to run more quickly and efficiently
     *
     * See: kernelFunctionStub
     */
    static uint64 __stdcall kernelFunctionStubInt3(void* context,
                                                   uint32 eflags,
                                                   uint8 vector,
                                                   Registers* generalRegisters);

    /*
     * Execute the handler
     */
    static bool executeHandler(HookIdt* thisObject,
                               bool reuseStack,
                               uint8 vector,
                               Registers* generalRegisters,
                               InterruptFrame* interruptFrame,
                               uint32 errorCode,
                               KIRQL oldIrql);

    /*
     * Called upon interrupt task-gate. Builds a memory struct Registers,
     * InterruptFrame and Error-code which links to the previous task registers.
     *
     * vector  - The interrupt vector number
     * context - The context of the interrupt routine, which is hardcoded
     *           pointer to 'this'
     */
    static void __stdcall kernelInterruptTaskStub(void* context,
                                                  uint8 vector);

    /**
     * Internal hooker global object
     *
     *    ;//push processor state
     *    pushad
     *    push gs
     *    push fs
     *    push ss
     *    push ds
     *    push es
     *    ;// Setup kernel mode segments
     *    mov ax, FS_REGISTER_VALUE
     *    mov fs, ax
     *    mov ax, ESDS_REGISTER_VALUE
     *    mov ds, ax
     *    mov es, ax
     *    mov ax, GS_REGISTER_VALUE,
     *    mov gs, ax


     *    ;// Test stack size
     *    mov  ebx, esp
     *    sub  eax, fs:[8]
     *    cmp  eax, STACK_SIZE
     *    jl   DonExecuteHandler


     *    ;// Push the arguments
     *    push esp     ;// Register* generalRegisters
     *    push vector  ;// uint      vector
     *    pushf        ;// The flags value
     *    cli          ;// Now it's safe to lock the processor
     *    push context ;// void*     context
     *    ;// Execute the handler
     *    mov  eax, interruptHandler
     *    call eax
     * DonExecuteHandler:
     *    ;// Restore segment context
     *    pop es
     *    pop ds
     *    pop ecx-newSS ;//TODO!
     *    pop fs
     *    pop gs
     *    ;// If the function doesn't return 0 (true) execute the original
     *    ;// handler. (AL=0 -> false, AL!=0 -> true)
     *    test al,al
     *    jz   returnFromInterrupt
     *
     *    ;// Don't execute the handler, IF will restore by the IRET
     *    push edx
     *    popf
     *    popad
     *    iretd
     *
     * returnFromInterrupt:
     *    ;// Execute the original handler
     *    push edx
     *    popf
     *    popad
     *    push originalHandler
     *    ret
     */
    #pragma pack(push)
    #pragma pack(1)
    class InterruptHooker {
    public:
        /**
         * Constructor. Creates an mini assembler code as template in the class
         * documentation See InterruptHooker.
         *
         * originalHandler  - The originalHandler function
         * interruptHandler - The handler to be used
         * vector           - The index of the interrupt
         * context          - The context for the interrupt handler
         * gsSegmentValue   - The context of the GS segment
         * fsSegmentValue   - The context of the FS segment
         * dsSegmentValue   - The context of the DS segment
         */
        InterruptHooker(void* originalHandler,
                        void* interruptHandler,
                        uint8 vector,
                        void* context,
                        uint16 gsSegmentValue,
                        uint16 fsSegmentValue,
                        uint16 dsSegmentValue);

        /**
         * Return pointer to the code for this stub
         */
        void* getFunction() const;

        /**
         * Return the address of the original interrupt handler
         */
        void* getOriginalInterruptHandler() const;

        /**
         * Return pointer for the hooking interrupt handler
         */
        void* getInterruptHandler() const;

    private:
		//the push Esp			   54
		uint8 m_pushEsp; //!YOAV!
		//the push Esp			   54
		uint8 m_pushEsp2; //!YOAV!
		//the push Esp			   54
		uint8 m_pushEsp3; //!YOAV!
		//the push Esp			   54
		uint8 m_pushEsp4; //!YOAV!
        // The pusha               60
        uint8  m_pusha;
        // The push gs             0F A8
        uint16 m_pushGs;
        // The push fs             0F A0
        uint16 m_pushFs;
        // The push ss             16
        uint8  m_pushSs;
        // The push ds             1E
        uint8  m_pushDs;
        // The push es             06
        uint8  m_pushEs;
        // The mov ax, imd16.      66 B8
        uint16 m_movAx;
        // The 16 bit segment value
        uint16 m_fsValue;
        // The operand prefix      66
        uint8  m_66prefix;
        // The mov fs, ax          8E E0
        uint16 m_movFsAx;

        // The mov ax, imd16.      66 B8
        uint16 m_movAx1;
        // The 16 bit segment value
        uint16 m_dsValue;
        // The operand prefix      66
        uint8  m_66prefix1;
        // The mov ds, ax          8E D8
        uint16 m_movDsAx;
        // The operand prefix      66
        uint8  m_66prefix2;
        // The mov es, ax          8E C0
        uint16 m_movEsAx;

        // The mov ax, imd16.      66 B8
        uint16 m_movAx2;
        // The 16 bit segment value
        uint16 m_gsValue;
        // The operand prefix      66
        uint8  m_66prefix3;
        // The mov gs, ax          8E E8
        uint16 m_movGsAx;

        /// The push esp           54
        uint8  m_pushEsp1;
        /// The push imd 8bit      6A
        uint8  m_pushVector;
        /// The vector number
        uint8  m_vector;
        // The pushf               9C
        uint8  m_pushf;
        // The cli                 FA
        uint8  m_cli;
		// The cld    --- caused a bug when our code gate code uses rep stos
		uint8  m_cld;
        /// The push imd 32bit     68
        uint8  m_pushContext;
        /// The context argument
        uint32 m_context;
        /// The mov eax prefix     B8
        uint8  m_movEax;
        /// The address for the interrupt handler
        uint32 m_interruptHandler;
        /// The call eax           FF D0
        uint16 m_callEax;
        // The pop es              07
        uint8  m_popEs;
        // The pop ds              1F
        uint8  m_popDs;
        // The pop ss              59
        uint8  m_popSsTODO;
        // The pop fs              0F A1
        uint16 m_popFs;
        // The pop gs              0F A9
        uint16 m_popGs;
        /// The test al,al         84 C0
        uint16 m_testAl;
        /// The jnz rela.          74 xx (Where xx is the sizeof(popa + iret) = 04)
        uint16 m_jnz;
        // The push edx            52
        uint8 m_pushEdx1;
        // The popf                9D
        uint8 m_popf1;
        /// The popad              61
        uint8 m_popad1;
		// The pop esp			   5C
		uint8 m_popEsp1; // !YOAV!
        /// The iret               CF
        uint8 m_iret;
        // The push edx            52
        uint8 m_pushEdx2;
        // The popf                9D
        uint8 m_popf2;
        /// The popad              61
        uint8 m_popad2;
		// The pop esp			   5C
		uint8 m_popEsp2; // !YOAV!
        /// The push imd 32bit     68
        uint8 m_push;
        /// The original interrupt handler.
        uint32 m_originalHandler;
        /// The ret                C3
        uint8 m_ret;
    };
    #pragma pack(pop)
    typedef cSmartPtr<InterruptHooker> InterruptHookerPtr;

    /*
     * A small stub which hooks a task-gate interrupt. This interrupt thunk
     * cannot invoke IRET.
     * The thunk calls the
     *
     * Here is the thunk code:
     *     ; // Execute normal function
     *     push vector
     *     push context
     *     mov  eax, interruptHandler
     *     call eax
     *
     *     mov ax, FS_REGISTER_VALUE
     *     mov fs, ax
     *     mov ax, DS_REGISTER_VALUE
     *     mov ds, ax
     *     mov ax, ES_REGISTER_VALUE
     *     mov es, ax
     *     mov ax, GS_REGISTER_VALUE,
     *     mov gs, ax
     *
     *     ; // Call the original function
     *     push originalHandler
     *     ret
     */
    #pragma pack(push)
    #pragma pack(1)
    class InterruptTaskGateHooker {
    public:
        /**
         * Constructor. Creates an mini assembler code as template in the class
         * documentation See InterruptHooker.
         *
         * originalHandler  - The originalHandler function location
         * interruptHandler - The handler to be used
         * vector           - The index of the interrupt
         * context          - The context for the interrupt handler
         *
         * ?SS? - TODO!
         */
        InterruptTaskGateHooker(void* originalHandler,
                                void* interruptHandler,
                                uint8 vector,
                                void* context,
                                uint16 gsSegmentValue,
                                uint16 fsSegmentValue,
                                uint16 dsSegmentValue,
                                uint16 esSegmentValue);

        /**
         * Return pointer to the code for this stub
         */
        void* getFunction() const;

        /**
         * Return the address of the original interrupt handler
         */
        void* getOriginalInterruptHandler() const;

    private:
        /// The push imd 8bit      6A
        uint8  m_pushVector;
        /// The vector number
        uint8  m_vector;
        /// The push imd 32bit     68
        uint8  m_pushContext;
        /// The context argument
        uint32 m_context;
        /// The mov eax prefix     B8
        uint8  m_movEax;
        /// The address for the interrupt handler
        uint32 m_interruptHandler;
        /// The call eax           FF D0
        uint16 m_callEax;
        /// The push imd 32bit     68
        uint8 m_push;
        /// The original interrupt handler.
        uint32 m_originalHandler;
        /// The ret                C3
        uint8 m_ret;
    };
    #pragma pack(pop)
    typedef cSmartPtr<InterruptTaskGateHooker> InterruptTaskGateHookerPtr;


    // The IDTR
    IDTR m_idt;
    // The hooked vector
    uint8 m_vector;
    // The callbacker function
    InterruptListenerPtr m_callbackClass;
    // The interrupt hooker class.
    InterruptHookerPtr m_hookerClass;
    // The interrupt task-gate hooker
    InterruptTaskGateHookerPtr m_taskHookerClass;

    // Saved contexts
    static InterruptFrame gPreviousFrameContext[cProcessorUtil::MAX_PROCESSORS_SUPPORT];
    static Registers gPreviousRegsContext[cProcessorUtil::MAX_PROCESSORS_SUPPORT];
};

/// The reference object counter
typedef cSmartPtr<HookIdt> HookIdtPtr;

#endif // __TBA_XDK_HOOKER_IA32_IDT_HOOKIDT_H
