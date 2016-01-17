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

#ifndef __TBA_XDK_UTILS_UTILS_H
#define __TBA_XDK_UTILS_UTILS_H

/*
 * utils.h
 *
 * Contains utility functions for NT operating system.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/hash.h"
#include "xdk/kernel.h"

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
//#define IMAGE_DIRECTORY_ENTRY_EXPORT        0   // Export Directory

/*
 * Accepts the process ID of a process and returns a referenced pointer to
 * EPROCESS structure of the process.
 *
 * ProcessId - Specifies the process ID of the process
 * Process - Returns a referenced pointer to the EPROCESS structure of process
 *           specified by ProcessId
 *
 * Return STATUS_SUCCESS to indicates success.
 * Return STATUS_INVALID_PARAMETER if the process ID was not found
 */
EXTERNC NTSTATUS NTAPI PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS *Process);

/*
 * Attaches the current thread to the address space of the target process.
 *
 * Process - Pointer to the target process object
 * ApcState - An opaque pointer to a KAPC_STATE structure
 */
EXTERNC VOID NTAPI KeStackAttachProcess(PEPROCESS Process, KAPC_STATE *ApcState);

/*
 * Detaches the current thread from the address space of a process and
 * restores the previous attach state.
 *
 * ApcState - Opaque pointer to a KAPC_STATE structure that was returned
 *            from a previous call to KeStackAttachProcess
 */
EXTERNC VOID NTAPI KeUnstackDetachProcess(KAPC_STATE *ApcState);

#define SystemProcessInformation 5 // SYSTEMINFOCLASS

typedef struct _SYSTEM_MODULE_ENTRY
{
	ULONG  Unused;
	ULONG  Always0;
	PVOID  ModuleBaseAddress;
	ULONG  ModuleSize;
	ULONG  Unknown;
	ULONG  ModuleEntryIndex;
	USHORT ModuleNameLength;
	USHORT ModuleNameOffset;
	CHAR   ModuleName [256];
} SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;

// Class 11
typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY
{
	ULONG  Unknown1;
	ULONG  Unknown2;
#ifdef _WIN64
	ULONG Unknown3;
	ULONG Unknown4;
#endif
	PVOID  Base;
	ULONG  Size;
	ULONG  Flags;
	USHORT  Index;
	USHORT  NameLength;
	USHORT  LoadCount;
	USHORT  PathLength;
	CHAR  ImageName[256];
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG Count;
	SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION
    {
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER CreateTime;
        ULONG WaitTime;
        PVOID StartAddress;
        CLIENT_ID ClientId;
        LONG Priority;
        LONG BasePriority;
        ULONG ContextSwitches;
        ULONG ThreadState;
        ULONG WaitReason;
    } SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION
    {
        SYSTEM_THREAD_INFORMATION ThreadInfo;
        PVOID StackBase;
        PVOID StackLimit;
        PVOID Win32StartAddress;
        PVOID TebAddress; /* This is only filled in on Vista and above */
        ULONG Reserved1;
        ULONG Reserved2;
        ULONG Reserved3;
    } SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION
    {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER SpareLi1;
        LARGE_INTEGER SpareLi2;
        LARGE_INTEGER SpareLi3;
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        UNICODE_STRING ImageName;
        DWORD BasePriority;
        HANDLE UniqueProcessId;
        ULONG InheritedFromUniqueProcessId;
        ULONG HandleCount;
        ULONG SessionId;
        PVOID PageDirectoryBase;
        VM_COUNTERS VirtualMemoryCounters;
        SIZE_T PrivatePageCount;
        IO_COUNTERS IoCounters;
        SYSTEM_EXTENDED_THREAD_INFORMATION Threads[1];
    } SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

#if 0
typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	WORD   e_res2[10];                  // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD   Characteristics;
	DWORD   TimeDateStamp;
	WORD    MajorVersion;
	WORD    MinorVersion;
	DWORD   Name;
	DWORD   Base;
	DWORD   NumberOfFunctions;
	DWORD   NumberOfNames;
	DWORD   AddressOfFunctions;     // RVA from base of image
	DWORD   AddressOfNames;         // RVA from base of image
	DWORD   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;


typedef struct _IMAGE_DATA_DIRECTORY {
	DWORD   VirtualAddress;
	DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER {
	//
	// Standard fields.
	//

	WORD    Magic;
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;
	DWORD   SizeOfInitializedData;
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;
	DWORD   BaseOfCode;
	DWORD   BaseOfData;

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_FILE_HEADER {
	WORD    Machine;
	WORD    NumberOfSections;
	DWORD   TimeDateStamp;
	DWORD   PointerToSymbolTable;
	DWORD   NumberOfSymbols;
	WORD    SizeOfOptionalHeader;
	WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_NT_HEADERS {
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;
#endif

/*
 * Contains utility functions for NT operating system.
 * This class is a singleton due to initialization requirements.
 *
 * NOTE: The XDK library make sure that this class is initalized before any
 *       driver code is executed.
 */
class cXDKUtils {
public:
    /*
     * Return the singleton object.
     */
    static cXDKUtils& getInstance();

    /*
     * Return the name of the current running process
     *
     * Throw exception if from any reason the ImageFileName offset is invalid
     */
    cString getCurrentProcessName();

    /*
     * This method will search for target Module's base
     */
    //static PVOID KernelGetModuleBase(PCHAR pModuleName);

    /*
     * This method will search for target process' handle
     */
    static HANDLE KernelGetProcHandleByName(PWCHAR pModuleName);

    /*
     * This method will return the EPROCESS structue for a target
     * process
     */
    static PEPROCESS KernelLookupProcessByName(PWCHAR pModuleName);

    /*
     * Like GetProcAddress but for Kernel
     */
	//static PVOID KernelGetProcAddress(PVOID ModuleBase, PCHAR pFunctionName);

	/*
     * This method will search function inside ntosKernel
     */
	//addressNumericValue findNtosFunction(CHAR * funcName);

	//void getNtFunctions(cHash<addressNumericValue, cString>& NtFunctions);

    /*
     * Translate Windows kernel UNICODE_STRING to a normal C++ cString class.
     *
     * Return the unicode string format as a C++ cString class
     */
    static cString parseUnicodeString(PUNICODE_STRING unicodeString);

    /*
     * For NT objects, such as: PDEVICE_OBJECT, PFILE_OBJECT, may holds a name.
     * This function retrieve the name by getting the right offset (OBJECT_NAME)
     * for the object
     *
     * Return the name.
     * Throw exception if objectPointer is not a valid pointer
     */
    static cString getObjectName(POBJECT objectPointer);

    /*
     * This function retrieve the type of an object.
     *
     * Return the one of the OB_TYPE_INDEX_* values (OB_TYPE_INDEX_TYPE)
     * Throw exception if objectPointer is not a valid pointer
     */
    static uint getObjectType(POBJECT objectPointer);

    /*
     * Return the object type name by a string ("Driver", "Directory", "Device")
     */
    static cString getObjectTypeName(POBJECT objectPointer);

    /*
     * Return the number of open references to the object
     */
    static uint getObjectHandlesCount(POBJECT objectPointer);

    /*
     * Returns the device-object which is related to a device-name string.
     * This function is used mainly to dynamic attach device into lower devices.
     *
     * deviceName - The device name string to be query
     * shouldGetRelated - True means that the returned device is the upper-device,
     *                    otherwise the function returns the lower device object
     *
     * Returns the device-object pointer.
     * Throw exception if the device doesn't exist
     */
    static PDEVICE_OBJECT getRelatedDeviceObject(const cString& deviceName,
                                                 bool shouldGetRelated = true);

private:
    /*
     * Private constructor. This class is a singleton.
     */
    cXDKUtils();

    /*
     * Construct during the DriverEntry life-time.
     * Used to find the ImageFileName inside the EPROCESS struct.
     * Since the 'System' process execute the DriverEntry function, the
     * name should be somewhere inside this struct.
     */
    class ProcessImageFileNameOffsetMatcher {
    public:
        // Constructor.
        ProcessImageFileNameOffsetMatcher();
        // Return the process name offset inside the EPROCESS
        // Throw exception if the offset is invalid
        uint getImageFileNameOffset() const;
    private:
        // The process name offset inside the EPROCESS
        uint m_imageFilenameOffset;
        // The name of the system process
        static const char m_systemProcessName[];
        // The range of bytes to scan, this size should be larger than the EPROCESS
        // of the operating system
        enum { SCAN_RANGE = 3*PAGE_SIZE };
    };

    /*
     * Try to create a process-name string from a pointer.
     *
     * location - Pointer to 16 characters.
     */
    static cString getProcessNameString(char* location);

    // The process name offset
    const ProcessImageFileNameOffsetMatcher m_processNameOffset;
    // The length of the process-name member
    enum { PROCESSNAME_LENGTH = 16 };
};

#endif // __TBA_XDK_UTILS_UTILS_H
