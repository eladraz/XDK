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

#ifndef __TBA_XDK_UNDOCUMENTED_STRUCTS_H
#define __TBA_XDK_UNDOCUMENTED_STRUCTS_H

/*
 * undocumentedStructs.h
 *
 * Undocument kernel API structs.
 * Most of the structs where collected from public documentation places over the
 * internet, "Undocumented Windows 2000 secrets" and more.
 *
 * NOTE: Most of the fields here are documented for i386 kernel.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "XDK/kernel.h"


/*
 * A single descriptor for loaded system module.
 * See ZwQuerySystemInformation;SystemModuleInformation
 */
typedef struct _MODULE_INFO
{
    DWORD dReserved1;
    DWORD dReserved2;
    PVOID pBase;
    DWORD dSize;
    DWORD dFlags;
    WORD  wIndex;
    WORD  wRank;
    WORD  wLoadCount;
    WORD  wNameOffset;
    BYTE  abPath[256];
} MODULE_INFO, *PMODULE_INFO, **PPMODULE_INFO;

/*
 * A list of all system loaded modules
 * See ZwQuerySystemInformation;SystemModuleInformation
 */
typedef struct _MODULE_LIST
{
    DWORD       m_numberOfModules;
    MODULE_INFO m_modules[1];
} MODULE_LIST, *PMODULE_LIST, **PPMODULE_LIST;

/*
 * Taken from 'Windows 2000 secrets'.
 * The internal struct of APC functions list.
 */
typedef struct _KAPC_STATE
{
    /*000*/ LIST_ENTRY        ApcListHead [2];
    /*010*/ struct _KPROCESS *Process;
    /*014*/ BOOLEAN           KernelApcInProgress;
    /*015*/ BOOLEAN           KernelApcPending;
    /*016*/ BOOLEAN           UserApcPending;
    /*018*/ }
    KAPC_STATE,
 * PKAPC_STATE,
**PPKAPC_STATE;

/*
 * Taken from 'Windows 2000 secrets'.
 */
typedef VOID (NTAPI *NTPROC_VOID) ();
typedef NTPROC_VOID *PNTPROC_VOID;
typedef NTSTATUS (NTAPI *NTPROC) ();
typedef NTPROC *PNTPROC;
typedef BOOLEAN (NTAPI *NTPROC_BOOLEAN) ();
typedef NTPROC_BOOLEAN *PNTPROC_BOOLEAN;


/*
 * Taken from 'Windows 2000 secrets'.
 * The internal struct for service functions.
 */
typedef struct _SYSTEM_SERVICE_TABLE
{
    /*000*/ PNTPROC ServiceTable;           // array of entry points
    /*004*/ PDWORD  CounterTable;           // array of usage counters
    /*008*/ DWORD   ServiceLimit;           // number of table entries
    /*00C*/ PBYTE   ArgumentTable;          // array of byte counts
    /*010*/
}   SYSTEM_SERVICE_TABLE,
 * PSYSTEM_SERVICE_TABLE,
**PPSYSTEM_SERVICE_TABLE;

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
    /*000*/ SYSTEM_SERVICE_TABLE ntoskrnl;  // ntoskrnl.exe (native api)
    /*010*/ SYSTEM_SERVICE_TABLE win32k;    // win32k.sys   (gdi/user)
    /*020*/ SYSTEM_SERVICE_TABLE Table3;    // not used
    /*030*/ SYSTEM_SERVICE_TABLE Table4;    // not used
    /*040*/ }
    SERVICE_DESCRIPTOR_TABLE,
 * PSERVICE_DESCRIPTOR_TABLE,
**PPSERVICE_DESCRIPTOR_TABLE;


/*
 * Taken from 'Windows 2000 secrets'.
 * The internal struct for threads.
 */
typedef struct _KTHREAD
{
    /*000*/ DISPATCHER_HEADER         Header; // DO_TYPE_THREAD (0x6C)
    /*010*/ LIST_ENTRY                MutantListHead;
    /*018*/ PVOID                     InitialStack;
    /*01C*/ PVOID                     StackLimit;
    /*020*/ struct _TEB              *Teb;
    /*024*/ PVOID                     TlsArray;
    /*028*/ PVOID                     KernelStack;
    /*02C*/ BOOLEAN                   DebugActive;
    /*02D*/ BYTE                      State; // THREAD_STATE_*
    /*02E*/ BOOLEAN                   Alerted;
    /*02F*/ BYTE                      bReserved01;
    /*030*/ BYTE                      Iopl;
    /*031*/ BYTE                      NpxState;
    /*032*/ BYTE                      Saturation;
    /*033*/ BYTE                      Priority;
    /*034*/ KAPC_STATE                ApcState;
    /*04C*/ DWORD                     ContextSwitches;
    /*050*/ DWORD                     WaitStatus;
    /*054*/ BYTE                      WaitIrql;
    /*055*/ BYTE                      WaitMode;
    /*056*/ BYTE                      WaitNext;
    /*057*/ BYTE                      WaitReason;
    /*058*/ PLIST_ENTRY               WaitBlockList;
    /*05C*/ LIST_ENTRY                WaitListEntry;
    /*064*/ DWORD                     WaitTime;
    /*068*/ BYTE                      BasePriority;
    /*069*/ BYTE                      DecrementCount;
    /*06A*/ BYTE                      PriorityDecrement;
    /*06B*/ BYTE                      Quantum;
    /*06C*/ KWAIT_BLOCK               WaitBlock [4];
    /*0CC*/ DWORD                     LegoData;
    /*0D0*/ DWORD                     KernelApcDisable;
    /*0D4*/ KAFFINITY                 UserAffinity;
    /*0D8*/ BOOLEAN                   SystemAffinityActive;
    /*0D9*/ BYTE                      Pad [3];
    /*0DC*/ PSERVICE_DESCRIPTOR_TABLE pServiceDescriptorTable;
    /*0E0*/ PVOID                     Queue;
    /*0E4*/ PVOID                     ApcQueueLock;
    /*0E8*/ KTIMER                    Timer;
    /*110*/ LIST_ENTRY                QueueListEntry;
    /*118*/ KAFFINITY                 Affinity;
    /*11C*/ BOOLEAN                   Preempted;
    /*11D*/ BOOLEAN                   ProcessReadyQueue;
    /*11E*/ BOOLEAN                   KernelStackResident;
    /*11F*/ BYTE                      NextProcessor;
    /*120*/ PVOID                     CallbackStack;
    /*124*/ struct _WIN32_THREAD     *Win32Thread;
    /*128*/ PVOID                     TrapFrame;
    /*12C*/ PKAPC_STATE               ApcStatePointer;
    /*130*/ PVOID                     p130;
    /*134*/ BOOLEAN                   EnableStackSwap;
    /*135*/ BOOLEAN                   LargeStack;
    /*136*/ BYTE                      ResourceIndex;
    /*137*/ KPROCESSOR_MODE           PreviousMode;
    /*138*/ DWORD                     KernelTime; // ticks
    /*13C*/ DWORD                     UserTime;   // ticks
    /*140*/ KAPC_STATE                SavedApcState;
    /*157*/ BYTE                      bReserved02;
    /*158*/ BOOLEAN                   Alertable;
    /*159*/ BYTE                      ApcStateIndex;
    /*15A*/ BOOLEAN                   ApcQueueable;
    /*15B*/ BOOLEAN                   AutoAlignment;
    /*15C*/ PVOID                     StackBase;
    /*160*/ KAPC                      SuspendApc;
    /*190*/ KSEMAPHORE                SuspendSemaphore;
    /*1A4*/ LIST_ENTRY                ThreadListEntry;  // see KPROCESS
    /*1AC*/ BYTE                      FreezeCount;
    /*1AD*/ BYTE                      SuspendCount;
    /*1AE*/ BYTE                      IdealProcessor;
    /*1AF*/ BOOLEAN                   DisableBoost;
    /*1B0*/ }
    KTHREAD,
 * PKTHREAD,
**PPKTHREAD;

/*
 * Taken from 'Windows 2000 secrets'.
 * The internal struct per each processor.
 * The virtual address location of that struct is: 0xFFDFF120
 */
typedef struct _KPRCB // processor control block
{
    WORD                   MinorVersion;        // 000
    WORD                   MajorVersion;        // 002
    struct _KTHREAD       *CurrentThread;       // 004
    struct _KTHREAD       *NextThread;          // 008
    struct _KTHREAD       *IdleThread;          // 00C
    CHAR                   Number;              // 010
    CHAR                   Reserved;            // 011
    WORD                   BuildType;           // 012
    KAFFINITY              SetMember;           // 014
    struct _RESTART_BLOCK *RestartBlock;        // 018
    }                                           // 01C
    KPRCB,
 * PKPRCB,
**PPKPRCB;
/**/

// Trap frame
//
//  NOTE - We deal only with 32bit registers, so the assembler equivalents
//         are always the extended forms.
//
//  NOTE - Unless you want to run like slow molasses everywhere in the
//         the system, this structure must be of DWORD length, DWORD
//         aligned, and its elements must all be DWORD aligned.
//
//  NOTE WELL   -
//
//      The i386 does not build stack frames in a consistent format, the
//      frames vary depending on whether or not a privilege transition
//      was involved.
//
//      In order to make NtContinue work for both user mode and kernel
//      mode callers, we must force a canonical stack.
//
//      If we're called from kernel mode, this structure is 8 bytes longer
//      than the actual frame!
//
//  WARNING:
//
//      KTRAP_FRAME_LENGTH needs to be 16byte integral (at present.)
//
typedef struct _KTRAP_FRAME {


    //
    //  Following 4 values are only used and defined for DBG systems,
    //  but are always allocated to make switching from DBG to non-DBG
    //  and back quicker.  They are not DEVL because they have a non-0
    //  performance impact.
    //

    ULONG   DbgEbp;         // Copy of User EBP set up so KB will work.
    ULONG   DbgEip;         // EIP of caller to system call, again, for KB.
    ULONG   DbgArgMark;     // Marker to show no args here.
    ULONG   DbgArgPointer;  // Pointer to the actual args

    //
    //  Temporary values used when frames are edited.
    //
    //
    //  NOTE:   Any code that want's ESP must materialize it, since it
    //          is not stored in the frame for kernel mode callers.
    //
    //          And code that sets ESP in a KERNEL mode frame, must put
    //          the new value in TempEsp, make sure that TempSegCs holds
    //          the real SegCs value, and put a special marker value into SegCs.
    //

    ULONG   TempSegCs;
    ULONG   TempEsp;

    //
    //  Debug registers.
    //

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

    //
    //  Segment registers
    //

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

    //
    //  Volatile registers
    //

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

    //
    //  Nesting state, not part of context record
    //

    ULONG   PreviousPreviousMode;

    PVOID ExceptionList; // PEXCEPTION_REGISTRATION_RECORD
    // Trash if caller was user mode.
    // Saved exception list if caller
    // was kernel mode or we're in
    // an interrupt.

    //
    //  FS is TIB/PCR pointer, is here to make save sequence easy
    //

    ULONG   SegFs;

    //
    //  Non-volatile registers
    //

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

    //
    //  Control registers
    //

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
    ULONG   HardwareSegSs;  // that involve a ring transition.

    ULONG   V86Es;          // these will be present for all transitions from
    ULONG   V86Ds;          // V86 mode
    ULONG   V86Fs;
    ULONG   V86Gs;
  } KTRAP_FRAME,
 * PKTRAP_FRAME;

/*
 * Taken from 'Windows 2000 secrets'.
 * The internal of objects
 */

// =================================================================
// MISCELLANEOUS STRUCTURES
// =================================================================
// see PsChargeSharedPoolQuota()
// and PsReturnSharedPoolQuota()

typedef struct _QUOTA_BLOCK
        {
/*000*/ DWORD Flags;
/*004*/ DWORD ChargeCount;
/*008*/ DWORD PeakPoolUsage [2]; // NonPagedPool, PagedPool
/*010*/ DWORD PoolUsage     [2]; // NonPagedPool, PagedPool
/*018*/ DWORD PoolQuota     [2]; // NonPagedPool, PagedPool
/*020*/ }
        QUOTA_BLOCK,
     * PQUOTA_BLOCK,
    **PPQUOTA_BLOCK;

#define QUOTA_BLOCK_ \
        sizeof (QUOTA_BLOCK)

// =================================================================
// BASIC OBJECT STRUCTURES
// =================================================================
//
// OBJECT PARTS:
// -------------
// OBJECT_QUOTA_CHARGES if Header.QuotaChargesOffset != 0
// OBJECT_HANDLE_DB     if Header.HandleDBOffset     != 0
// OBJECT_NAME          if Header.NameOffset         != 0
// OBJECT_CREATOR_INFO  if Header.ObjectFlags & OB_FLAG_CREATOR_INFO
// OBJECT_HEADER        always present
// OBJECT               OBJECT_TYPE specific

typedef PVOID POBJECT, *PPOBJECT;

// -----------------------------------------------------------------
// if (oti.MaintainHandleCount) ObpObjectsWithHandleDB++;
// if (oti.MaintainTypeList   ) ObpObjectsWithCreatorInfo++;

typedef struct _OBJECT_TYPE_INITIALIZER
        {
/*000*/ WORD            Length;          //0x004C
/*002*/ BOOLEAN         UseDefaultObject;//OBJECT_TYPE.DefaultObject
/*003*/ BOOLEAN         Reserved1;
/*004*/ DWORD           InvalidAttributes;
/*008*/ GENERIC_MAPPING GenericMapping;
/*018*/ ACCESS_MASK     ValidAccessMask;
/*01C*/ BOOLEAN         SecurityRequired;
/*01D*/ BOOLEAN         MaintainHandleCount; // OBJECT_HANDLE_DB
/*01E*/ BOOLEAN         MaintainTypeList;    // OBJECT_CREATOR_INFO
/*01F*/ BYTE            Reserved2;
/*020*/ BOOL            PagedPool;
/*024*/ DWORD           DefaultPagedPoolCharge;
/*028*/ DWORD           DefaultNonPagedPoolCharge;
/*02C*/ NTPROC          DumpProcedure;
/*030*/ NTPROC          OpenProcedure;
/*034*/ NTPROC          CloseProcedure;
/*038*/ NTPROC          DeleteProcedure;
/*03C*/ NTPROC_VOID     ParseProcedure;
/*040*/ NTPROC_VOID     SecurityProcedure; // SeDefaultObjectMethod
/*044*/ NTPROC_VOID     QueryNameProcedure;
/*048*/ NTPROC_BOOLEAN  OkayToCloseProcedure;
/*04C*/ }
        OBJECT_TYPE_INITIALIZER,
     * POBJECT_TYPE_INITIALIZER,
    **PPOBJECT_TYPE_INITIALIZER;

#define OBJECT_TYPE_INITIALIZER_ \
        sizeof (OBJECT_TYPE_INITIALIZER)

// -----------------------------------------------------------------
// see ObCreateObjectType()
// and ObpAllocateObject()

#define OB_TYPE_INDEX_TYPE              1 // [ObjT] "Type"
#define OB_TYPE_INDEX_DIRECTORY         2 // [Dire] "Directory"
#define OB_TYPE_INDEX_SYMBOLIC_LINK     3 // [Symb] "SymbolicLink"
#define OB_TYPE_INDEX_TOKEN             4 // [Toke] "Token"
#define OB_TYPE_INDEX_PROCESS           5 // [Proc] "Process"
#define OB_TYPE_INDEX_THREAD            6 // [Thre] "Thread"
#define OB_TYPE_INDEX_JOB               7 // [Job ] "Job"
#define OB_TYPE_INDEX_EVENT             8 // [Even] "Event"
#define OB_TYPE_INDEX_EVENT_PAIR        9 // [Even] "EventPair"
#define OB_TYPE_INDEX_MUTANT           10 // [Muta] "Mutant"
#define OB_TYPE_INDEX_CALLBACK         11 // [Call] "Callback"
#define OB_TYPE_INDEX_SEMAPHORE        12 // [Sema] "Semaphore"
#define OB_TYPE_INDEX_TIMER            13 // [Time] "Timer"
#define OB_TYPE_INDEX_PROFILE          14 // [Prof] "Profile"
#define OB_TYPE_INDEX_WINDOW_STATION   15 // [Wind] "WindowStation"
#define OB_TYPE_INDEX_DESKTOP          16 // [Desk] "Desktop"
#define OB_TYPE_INDEX_SECTION          17 // [Sect] "Section"
#define OB_TYPE_INDEX_KEY              18 // [Key ] "Key"
#define OB_TYPE_INDEX_PORT             19 // [Port] "Port"
#define OB_TYPE_INDEX_WAITABLE_PORT    20 // [Wait] "WaitablePort"
#define OB_TYPE_INDEX_ADAPTER          21 // [Adap] "Adapter"
#define OB_TYPE_INDEX_CONTROLLER       22 // [Cont] "Controller"
#define OB_TYPE_INDEX_DEVICE           23 // [Devi] "Device"
#define OB_TYPE_INDEX_DRIVER           24 // [Driv] "Driver"
#define OB_TYPE_INDEX_IO_COMPLETION    25 // [IoCo] "IoCompletion"
#define OB_TYPE_INDEX_FILE             26 // [File] "File"
#define OB_TYPE_INDEX_WMI_GUID         27 // [WmiG] "WmiGuid"

#define OB_TYPE_TAG_TYPE           'TjbO' // [ObjT] "Type"
#define OB_TYPE_TAG_DIRECTORY      'eriD' // [Dire] "Directory"
#define OB_TYPE_TAG_SYMBOLIC_LINK  'bmyS' // [Symb] "SymbolicLink"
#define OB_TYPE_TAG_TOKEN          'ekoT' // [Toke] "Token"
#define OB_TYPE_TAG_PROCESS        'corP' // [Proc] "Process"
#define OB_TYPE_TAG_THREAD         'erhT' // [Thre] "Thread"
#define OB_TYPE_TAG_JOB            ' boJ' // [Job ] "Job"
#define OB_TYPE_TAG_EVENT          'nevE' // [Even] "Event"
#define OB_TYPE_TAG_EVENT_PAIR     'nevE' // [Even] "EventPair"
#define OB_TYPE_TAG_MUTANT         'atuM' // [Muta] "Mutant"
#define OB_TYPE_TAG_CALLBACK       'llaC' // [Call] "Callback"
#define OB_TYPE_TAG_SEMAPHORE      'ameS' // [Sema] "Semaphore"
#define OB_TYPE_TAG_TIMER          'emiT' // [Time] "Timer"
#define OB_TYPE_TAG_PROFILE        'forP' // [Prof] "Profile"
#define OB_TYPE_TAG_WINDOW_STATION 'dniW' // [Wind] "WindowStation"
#define OB_TYPE_TAG_DESKTOP        'kseD' // [Desk] "Desktop"
#define OB_TYPE_TAG_SECTION        'tceS' // [Sect] "Section"
#define OB_TYPE_TAG_KEY            ' yeK' // [Key ] "Key"
#define OB_TYPE_TAG_PORT           'troP' // [Port] "Port"
#define OB_TYPE_TAG_WAITABLE_PORT  'tiaW' // [Wait] "WaitablePort"
#define OB_TYPE_TAG_ADAPTER        'padA' // [Adap] "Adapter"
#define OB_TYPE_TAG_CONTROLLER     'tnoC' // [Cont] "Controller"
#define OB_TYPE_TAG_DEVICE         'iveD' // [Devi] "Device"
#define OB_TYPE_TAG_DRIVER         'virD' // [Driv] "Driver"
#define OB_TYPE_TAG_IO_COMPLETION  'oCoI' // [IoCo] "IoCompletion"
#define OB_TYPE_TAG_FILE           'eliF' // [File] "File"
#define OB_TYPE_TAG_WMI_GUID       'GimW' // [WmiG] "WmiGuid"

typedef struct _OBJECT_TYPE
        {
/*000*/ ERESOURCE      Lock;
/*038*/ LIST_ENTRY     ObjectListHead; // OBJECT_CREATOR_INFO
/*040*/ UNICODE_STRING ObjectTypeName; // see above
/*048*/ union
            {
/*048*/     PVOID DefaultObject; // ObpDefaultObject
/*048*/     DWORD Code;          // File: 5C, WaitablePort: A0
            };
/*04C*/ DWORD                   ObjectTypeIndex; // OB_TYPE_INDEX_*
/*050*/ DWORD                   ObjectCount;
/*054*/ DWORD                   HandleCount;
/*058*/ DWORD                   PeakObjectCount;
/*05C*/ DWORD                   PeakHandleCount;
/*060*/ OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
/*0AC*/ DWORD                   ObjectTypeTag;   // OB_TYPE_TAG_*
/*0B0*/ }
        OBJECT_TYPE,
     * POBJECT_TYPE,
    **PPOBJECT_TYPE;

#define OBJECT_TYPE_ \
        sizeof (OBJECT_TYPE)

// -----------------------------------------------------------------
// see ObCreateObjectType()
// and ObpObjectTypes

#define MAXIMUM_OBJECT_TYPES 23

typedef struct _OBJECT_TYPES
        {
/*000*/ POBJECT_TYPE ObjectTypes [MAXIMUM_OBJECT_TYPES];
/*05C*/ }
        OBJECT_TYPES,
     * POBJECT_TYPES,
    **PPOBJECT_TYPES;

#define OBJECT_TYPES_ \
        sizeof (OBJECT_TYPES)

// -----------------------------------------------------------------
// see ObQueryTypeInfo()

typedef struct _OBJECT_TYPE_INFO
        {
/*000*/ UNICODE_STRING  ObjectTypeName; // points to Buffer[]
/*008*/ DWORD           ObjectCount;
/*00C*/ DWORD           HandleCount;
/*010*/ DWORD           Reserved1 [4];
/*020*/ DWORD           PeakObjectCount;
/*024*/ DWORD           PeakHandleCount;
/*028*/ DWORD           Reserved2 [4];
/*038*/ DWORD           InvalidAttributes;
/*03C*/ GENERIC_MAPPING GenericMapping;
/*04C*/ ACCESS_MASK     ValidAccessMask;
/*050*/ BOOLEAN         SecurityRequired;
/*051*/ BOOLEAN         MaintainHandleCount;
/*052*/ WORD            Reserved3;
/*054*/ BOOL            PagedPool;
/*058*/ DWORD           DefaultPagedPoolCharge;
/*05C*/ DWORD           DefaultNonPagedPoolCharge;
/*060*/ WORD            Buffer[1];
/*???*/ }
        OBJECT_TYPE_INFO,
     * POBJECT_TYPE_INFO,
    **PPOBJECT_TYPE_INFO;

#define OBJECT_TYPE_INFO_ \
        sizeof (OBJECT_TYPE_INFO)

// -----------------------------------------------------------------
// see ObpCaptureObjectCreateInformation()
// and ObpAllocateObject()

typedef struct _OBJECT_CREATE_INFO
        {
/*000*/ DWORD                        Attributes; // OBJ_*
/*004*/ HANDLE                       RootDirectory;
/*008*/ DWORD                        Reserved;
/*00C*/ KPROCESSOR_MODE              AccessMode;
/*010*/ DWORD                        PagedPoolCharge;
/*014*/ DWORD                        NonPagedPoolCharge;
/*018*/ DWORD                        SecurityCharge;
/*01C*/ PSECURITY_DESCRIPTOR         SecurityDescriptor;
/*020*/ PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
/*024*/ SECURITY_QUALITY_OF_SERVICE  SecurityQualityOfServiceBuffer;
/*030*/ }
        OBJECT_CREATE_INFO,
     * POBJECT_CREATE_INFO,
    **PPOBJECT_CREATE_INFO;

#define OBJECT_CREATE_INFO_ \
        sizeof (OBJECT_CREATE_INFO)

// -----------------------------------------------------------------

typedef struct _OBJECT_CREATOR_INFO
        {
/*000*/ LIST_ENTRY ObjectList;      // OBJECT_CREATOR_INFO
/*008*/ HANDLE     UniqueProcessId;
/*00C*/ WORD       Reserved1;
/*00E*/ WORD       Reserved2;
/*010*/ }
        OBJECT_CREATOR_INFO,
     * POBJECT_CREATOR_INFO,
    **PPOBJECT_CREATOR_INFO;

#define OBJECT_CREATOR_INFO_ \
        sizeof (OBJECT_CREATOR_INFO)

// -----------------------------------------------------------------

#define OB_FLAG_CREATE_INFO    0x01 // has OBJECT_CREATE_INFO
#define OB_FLAG_KERNEL_MODE    0x02 // created by kernel
#define OB_FLAG_CREATOR_INFO   0x04 // has OBJECT_CREATOR_INFO
#define OB_FLAG_EXCLUSIVE      0x08 // OBJ_EXCLUSIVE
#define OB_FLAG_PERMANENT      0x10 // OBJ_PERMANENT
#define OB_FLAG_SECURITY       0x20 // has security descriptor
#define OB_FLAG_SINGLE_PROCESS 0x40 // no HandleDBList

typedef struct _OBJECT_HEADER
        {
/*000*/ DWORD        PointerCount;       // number of references
/*004*/ DWORD        HandleCount;        // number of open handles
/*008*/ POBJECT_TYPE ObjectType;
/*00C*/ BYTE         NameOffset;         // -> OBJECT_NAME
/*00D*/ BYTE         HandleDBOffset;     // -> OBJECT_HANDLE_DB
/*00E*/ BYTE         QuotaChargesOffset; // -> OBJECT_QUOTA_CHARGES
/*00F*/ BYTE         ObjectFlags;        // OB_FLAG_*
/*010*/ union
            { // OB_FLAG_CREATE_INFO ? ObjectCreateInfo : QuotaBlock
/*010*/     PQUOTA_BLOCK        QuotaBlock;
/*010*/     POBJECT_CREATE_INFO ObjectCreateInfo;
/*014*/     };
/*014*/ PSECURITY_DESCRIPTOR SecurityDescriptor;
/*018*/ }
        OBJECT_HEADER,
     * POBJECT_HEADER,
    **PPOBJECT_HEADER;

// -----------------------------------------------------------------
// see ObpCreateTypeArray()
// and ObpDestroyTypeArray()

typedef struct _OBJECT_TYPE_ARRAY
        {
/*000*/ DWORD                ObjectCount;
/*004*/ POBJECT_CREATOR_INFO ObjectList[1];
/*???*/ }
        OBJECT_TYPE_ARRAY,
     * POBJECT_TYPE_ARRAY,
    **PPOBJECT_TYPE_ARRAY;

#define OBJECT_TYPE_ARRAY_ \
        sizeof (OBJECT_TYPE_ARRAY)

// -----------------------------------------------------------------
// see ObpInsertDirectoryEntry()
// and ObpDeleteDirectoryEntry()

typedef struct _OBJECT_DIRECTORY_ENTRY
        {
/*000*/ struct _OBJECT_DIRECTORY_ENTRY *NextEntry;
/*004*/ POBJECT                         Object;
/*008*/ }
        OBJECT_DIRECTORY_ENTRY,
     * POBJECT_DIRECTORY_ENTRY,
    **PPOBJECT_DIRECTORY_ENTRY;

#define OBJECT_DIRECTORY_ENTRY_ \
        sizeof (OBJECT_DIRECTORY_ENTRY)

// -----------------------------------------------------------------

#define OBJECT_HASH_TABLE_SIZE 37

typedef struct _OBJECT_DIRECTORY
        {
/*000*/ POBJECT_DIRECTORY_ENTRY HashTable [OBJECT_HASH_TABLE_SIZE];
/*094*/ POBJECT_DIRECTORY_ENTRY CurrentEntry;
/*098*/ BOOLEAN                 CurrentEntryValid;
/*099*/ BYTE                    Reserved1;
/*09A*/ WORD                    Reserved2;
/*09C*/ DWORD                   Reserved3;
/*0A0*/ }
        OBJECT_DIRECTORY,
     * POBJECT_DIRECTORY,
    **PPOBJECT_DIRECTORY;

#define OBJECT_DIRECTORY_ \
        sizeof (OBJECT_DIRECTORY)

// -----------------------------------------------------------------

typedef struct _OBJECT_NAME
        {
/*000*/ POBJECT_DIRECTORY Directory;
/*004*/ UNICODE_STRING    Name;
/*00C*/ DWORD             Reserved;
/*010*/ }
        OBJECT_NAME,
     * POBJECT_NAME,
    **PPOBJECT_NAME;

#define OBJECT_NAME_ \
        sizeof (OBJECT_NAME)

// -----------------------------------------------------------------

typedef struct _OBJECT_HANDLE_DB
        {
/*000*/ union
            {
/*000*/     struct _EPROCESS              *Process;
/*000*/     struct _OBJECT_HANDLE_DB_LIST *HandleDBList;
/*004*/     };
/*004*/ DWORD HandleCount;
/*008*/ }
        OBJECT_HANDLE_DB,
     * POBJECT_HANDLE_DB,
    **PPOBJECT_HANDLE_DB;

#define OBJECT_HANDLE_DB_ \
        sizeof (OBJECT_HANDLE_DB)

// -----------------------------------------------------------------

typedef struct _OBJECT_HANDLE_DB_LIST
        {
/*000*/ DWORD            Count;
/*004*/ OBJECT_HANDLE_DB Entries[1];
/*???*/ }
        OBJECT_HANDLE_DB_LIST,
     * POBJECT_HANDLE_DB_LIST,
    **PPOBJECT_HANDLE_DB_LIST;

#define OBJECT_HANDLE_DB_LIST_ \
        sizeof (OBJECT_HANDLE_DB_LIST)

// -----------------------------------------------------------------
// see ObpChargeQuotaForObject()
// and ObValidateSecurityQuota()

#define OB_SECURITY_CHARGE 0x00000800

typedef struct _OBJECT_QUOTA_CHARGES
        {
/*000*/ DWORD PagedPoolCharge;
/*004*/ DWORD NonPagedPoolCharge;
/*008*/ DWORD SecurityCharge;
/*00C*/ DWORD Reserved;
/*010*/ }
        OBJECT_QUOTA_CHARGES,
     * POBJECT_QUOTA_CHARGES,
    **PPOBJECT_QUOTA_CHARGES;

#define OBJECT_QUOTA_CHARGES_ \
        sizeof (OBJECT_QUOTA_CHARGES)

#endif // __TBA_XDK_UNDOCUMENTED_STRUCTS_H
