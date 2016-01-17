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
 * utils.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#include "xStl/except/trace.h"
#include "xStl/except/assert.h"
#include "xStl/stream/traceStream.h"
#include "xdk/kernel.h"
#include "xdk/unicodeString.h"
#include "xdk/undocumented.h"
#include "xdk/undocumentedStructs.h"
#include "xdk/utils/utils.h"
#include "xdk/utils/bugcheck.h"

cXDKUtils& cXDKUtils::getInstance()
{
    static cXDKUtils gSingleton;
    return gSingleton;
}

cXDKUtils::cXDKUtils()
{
}

cString cXDKUtils::getCurrentProcessName()
{
    uint offset = m_processNameOffset.getImageFileNameOffset();
    PEPROCESS proc = PsGetCurrentProcess();
    return getProcessNameString(((char*)proc) + offset);
}

/* TODO!
PVOID cXDKUtils::KernelGetModuleBase(PCHAR pModuleName)
{
}
*/

HANDLE cXDKUtils::KernelGetProcHandleByName(PWCHAR pModuleName)
{
    HANDLE pModuleHandle = NULL;
	PULONG pSystemInfoBuffer = NULL;

	__try
	{
		NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
		ULONG SystemInfoBufferSize = 0;

		status = ZwQuerySystemInformation(SystemProcessInformation,
			&SystemInfoBufferSize,
			0,
			&SystemInfoBufferSize);

		if (!SystemInfoBufferSize)
			return NULL;

		pSystemInfoBuffer = (PULONG)ExAllocatePool(NonPagedPool, SystemInfoBufferSize*2);

		if (!pSystemInfoBuffer)
			return NULL;

		memset(pSystemInfoBuffer, 0, SystemInfoBufferSize*2);

		status = ZwQuerySystemInformation(SystemProcessInformation,
			pSystemInfoBuffer,
			SystemInfoBufferSize*2,
			&SystemInfoBufferSize);

		if (NT_SUCCESS(status))
		{
            // Get pointer to first system process info structure
            PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)pSystemInfoBuffer;

            // Loop over each process
            while(true)
            {
                if (NULL != pInfo->ImageName.Buffer)
                {
                    if (_wcsicmp(pInfo->ImageName.Buffer, pModuleName) == 0)
				    {
					    pModuleHandle = pInfo->UniqueProcessId;
					    break;
				    }
                }

                // Load next entry
                if (pInfo->NextEntryOffset == 0)
                    break;
                pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo)+ pInfo->NextEntryOffset);
            }
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
    {
		pModuleHandle = NULL;
	}
	if(pSystemInfoBuffer)
		ExFreePool(pSystemInfoBuffer);

	return pModuleHandle;
}

PEPROCESS cXDKUtils::KernelLookupProcessByName(PWCHAR pModuleName)
{
    // Get the process ID by name
    HANDLE ProcessId = KernelGetProcHandleByName(pModuleName);
    if (NULL == ProcessId)
    {
        traceHigh("[!] KernelLookupProcessByName failed" << endl);
        return NULL;
    }

    PEPROCESS Process = NULL;
    NTSTATUS status = STATUS_INVALID_PARAMETER;

    // Get the EPROCESS structure from the process ID
    status = PsLookupProcessByProcessId(ProcessId, &Process);

    if (!NT_SUCCESS(status))
    {
        traceHigh("[!] PsLookupProcessByProcessId failed" << endl);
        return NULL;
    }

    return Process;
}

/*
PVOID cXDKUtils::KernelGetProcAddress(PVOID ModuleBase, PCHAR pFunctionName)
{
	PVOID pFunctionAddress = NULL;

	__try
	{
		ULONG size = 0;
		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)ModuleBase;
		PIMAGE_NT_HEADERS nt  = (PIMAGE_NT_HEADERS)((ULONG)ModuleBase + dos->e_lfanew);

		PIMAGE_DATA_DIRECTORY expdir = (PIMAGE_DATA_DIRECTORY)
			(nt->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT);
		ULONG addr = expdir->VirtualAddress;
		PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((ULONG) ModuleBase + addr);

		PULONG functions = (PULONG)((ULONG)ModuleBase + exports->AddressOfFunctions);
		PSHORT ordinals = (PSHORT)((ULONG)ModuleBase + exports->AddressOfNameOrdinals);
		PULONG names = (PULONG)((ULONG)ModuleBase + exports->AddressOfNames);
		ULONG max_name = exports->NumberOfNames;
		ULONG max_func = exports->NumberOfFunctions;

		ULONG i;

		for (i = 0; i < max_name; i++)
		{
			ULONG ord = ordinals[i];
			if(i >= max_name || ord >= max_func)
				return NULL;

			if (functions[ord] < addr || functions[ord] >= addr + size)
			{
				if (strcmp((PCHAR) ModuleBase + names[i], pFunctionName)  == 0)
				{
					pFunctionAddress =(PVOID)((PCHAR) ModuleBase + functions[ord]);
					break;
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		pFunctionAddress = NULL;
	}

	return pFunctionAddress;
}

addressNumericValue cXDKUtils::findNtosFunction(CHAR * funcName)
{
	PVOID KernelImageBase = KernelGetModuleBase("ntkrnlpa.exe");

	if (0 == KernelImageBase)
    {
		traceHigh("Failed to find NtosKernel" << endl);
		return 0;
	}

	return (DWORD)KernelGetProcAddress(KernelImageBase, funcName);
}

void cXDKUtils::getNtFunctions(cHash<addressNumericValue, cString>& NtFunctions)
{
	PVOID KernelImageBase = KernelGetModuleBase("ntkrnlpa.exe");
	PVOID pFunctionAddress = NULL;

	if (KernelImageBase == 0 )
    {
		traceHigh("Failed to find NtosKernel" << endl);
		return;
	}

	//__try
	{
		ULONG size = 0;
		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)KernelImageBase;
		PIMAGE_NT_HEADERS nt  = (PIMAGE_NT_HEADERS)((ULONG)KernelImageBase + dos->e_lfanew);

		PIMAGE_DATA_DIRECTORY expdir = (PIMAGE_DATA_DIRECTORY)
			(nt->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT);

		//PIMAGE_SECTION_HEADER sectionHeaders = (PIMAGE_SECTION_HEADER)(expdir + nt->OptionalHeader.NumberOfRvaAndSizes);

		ULONG addr = expdir->VirtualAddress;
		PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((ULONG)KernelImageBase + addr);

		PULONG functions = (PULONG)((ULONG)KernelImageBase + exports->AddressOfFunctions);
		PSHORT ordinals = (PSHORT)((ULONG)KernelImageBase + exports->AddressOfNameOrdinals);
		PULONG names = (PULONG)((ULONG)KernelImageBase + exports->AddressOfNames);
		ULONG  max_name = exports->NumberOfNames;
		ULONG  max_func = exports->NumberOfFunctions;
		ULONG i;

		ULONG startOfData = nt->OptionalHeader.BaseOfCode;
		ULONG endOfData = nt->OptionalHeader.BaseOfCode + nt->OptionalHeader.SizeOfCode;

		traceHigh("base: " << HEXDWORD(getNumeric(KernelImageBase)) << endl);
		traceHigh("baseOfCode: " << HEXDWORD(startOfData) << "  " << HEXDWORD(getNumeric((PCHAR)KernelImageBase + startOfData)) << endl);
		traceHigh("endOfCode: " << HEXDWORD(endOfData) << "  " << HEXDWORD(getNumeric((PCHAR)KernelImageBase + endOfData)) << endl);

		for (i = 0; i < max_name; i++)
		{
			ULONG ord = ordinals[i];
			if(i >= max_name || ord >= max_func)
				continue;

			if (functions[ord] < addr || functions[ord] >= addr + size)
			{

				pFunctionAddress = (PVOID)((PCHAR) KernelImageBase + functions[ord]);
				if (!NtFunctions.hasKey((addressNumericValue) pFunctionAddress))
					NtFunctions.append((addressNumericValue) pFunctionAddress,
						 ((PCHAR) KernelImageBase + names[i]));
			}
		}
	}
	//__except(EXCEPTION_EXECUTE_HANDLER)
	//{
	//	listOfFunctions.removeAll();
	//}
}
*/

cString cXDKUtils::getProcessNameString(char* location)
{
    ASSERT(location != NULL);

    char processName[PROCESSNAME_LENGTH + 1];
    cOS::memcpy(processName,
           location,
           PROCESSNAME_LENGTH);
    processName[PROCESSNAME_LENGTH] = '\0';
    return cString(processName);
}

cXDKUtils::ProcessImageFileNameOffsetMatcher::ProcessImageFileNameOffsetMatcher() :
    m_imageFilenameOffset(0)
{
    PEPROCESS proc = PsGetCurrentProcess();

    for(uint i = 0; i < SCAN_RANGE; i++)
    {
        if (cXDKUtils::getProcessNameString(((char*)(proc)) + i).icompare(m_systemProcessName)
            == cString::EqualTo)
        {
            m_imageFilenameOffset = i;
            break;
        }
    }
}

uint cXDKUtils::ProcessImageFileNameOffsetMatcher::getImageFileNameOffset() const
{
    CHECK(m_imageFilenameOffset != 0);
    return m_imageFilenameOffset;
}

const char cXDKUtils::ProcessImageFileNameOffsetMatcher::m_systemProcessName[] =
    "System";

cString cXDKUtils::parseUnicodeString(PUNICODE_STRING unicodeString)
{
    CHECK(unicodeString != NULL);

    uint length = (unicodeString->Length) / sizeof(unichar);
    cString newString;
    character* string = newString.getBuffer(length + 1);
    #ifdef XSTL_UNICODE
        // The unicode xStl version have a very simple task...
        cOS::memcpy(string, unicodeString->Buffer, length * sizeof(unichar));
    #else
        // Covert the unicode string to ASCII string
        unichar* unicodeStringPtr = unicodeString->Buffer;
        for (uint i = 0; i < length; i++)
            string[i] = cChar::covert2Ascii(unicodeStringPtr[i]);

    #endif
    // And put null-terminate character
    string[length] = cChar::getNullCharacter();
    // Recalculate the length of the string
    newString.rearrangeStringVector();
    return newString;
}

cString cXDKUtils::getObjectName(POBJECT objectPointer)
{
    CHECK(objectPointer != NULL);
    POBJECT_HEADER objectHeader = ((POBJECT_HEADER)(objectPointer) - 1);
    CHECK(objectHeader != NULL);

    // Get the name
    POBJECT_NAME objectName = (POBJECT_NAME)((uint8*)(objectHeader) -
                                             objectHeader->NameOffset);
    return parseUnicodeString(&objectName->Name);
}

uint cXDKUtils::getObjectType(POBJECT objectPointer)
{
    CHECK(objectPointer != NULL);
    POBJECT_HEADER objectHeader = ((POBJECT_HEADER)(objectPointer) - 1);
    CHECK(objectHeader != NULL);

    // Get the name
    POBJECT_TYPE objectType = objectHeader->ObjectType;

    return objectType->ObjectTypeIndex;
}

cString cXDKUtils::getObjectTypeName(POBJECT objectPointer)
{
    CHECK(objectPointer != NULL);
    POBJECT_HEADER objectHeader = ((POBJECT_HEADER)(objectPointer) - 1);
    CHECK(objectHeader != NULL);

    // Get the name
    POBJECT_TYPE objectType = objectHeader->ObjectType;

    return parseUnicodeString(&objectType->ObjectTypeName);
}

uint cXDKUtils::getObjectHandlesCount(POBJECT objectPointer)
{
    CHECK(objectPointer != NULL);
    POBJECT_HEADER objectHeader = ((POBJECT_HEADER)(objectPointer) - 1);
    CHECK(objectHeader != NULL);

    return objectHeader->HandleCount;
}

PDEVICE_OBJECT cXDKUtils::getRelatedDeviceObject(const cString& deviceName,
                                                 bool shouldGetRelated)
{
    traceHigh("cXDKUtils::getRelatedDeviceObject - " << deviceName << endl);
    // Open the device driver
    // Translate name
    cUnicodeString unicodeDeviceString(deviceName);
    OBJECT_ATTRIBUTES objectAttributes;
    InitializeObjectAttributes(&objectAttributes, unicodeDeviceString,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);
    // Open device-driver
    HANDLE ntFileHandle;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS ntStatus = ZwCreateFile(&ntFileHandle,
            SYNCHRONIZE | FILE_ANY_ACCESS,
            &objectAttributes,
            &ioStatus,
            NULL,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
            NULL,
            0);
    CHECK_MSG(ntStatus == STATUS_SUCCESS,
              "getRelatedDeviceObject: Cannot open device driver");

    // Got the file handle, so now look-up the file-object it refers to
    PFILE_OBJECT fileObject;
    ntStatus = ObReferenceObjectByHandle(ntFileHandle,
                                         FILE_READ_DATA,
                                         NULL,
                                         KernelMode,
                                         (PVOID*)&fileObject,
                                         NULL);
    ZwClose(ntFileHandle);
    CHECK_MSG(ntStatus == STATUS_SUCCESS,
              "getRelatedDeviceObject: Cannot get FileObject from handle");

    PDEVICE_OBJECT ret = NULL;
    if (shouldGetRelated)
        ret = IoGetRelatedDeviceObject(fileObject);
    else
        ret = fileObject->DeviceObject;

    // Dereference object
    ObDereferenceObject(fileObject);

    CHECK_MSG(ret != NULL,
              "getRelatedDeviceObject: Related device-object is invalid...");
    return ret;
}
