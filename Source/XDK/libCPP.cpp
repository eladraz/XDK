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
 * libcpp.cpp
 *
 * Implementation of the XDK C++ library.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/data/char.h"
#include "xStl/data/string.h"
#include "xStl/data/smartptr.h"
#include "xStl/data/datastream.h"
#include "xStl/except/exception.h"
#include "xStl/stream/traceStream.h"
#include "xdk/kernel.h"
#include "xdk/driver.h"
#include "xdk/libCPP.h"
#include "xdk/memory.h"
#include "xdk/atexit.h"
#include "xdk/driverFactory.h"
#include "xdk/xdkTraceSingleton.h"
#include "xdk/ehlib/ehlib.h"
#include "xdk/utils/utils.h"
#include "xdk/utils/processorUtil.h"
#include "xdk/utils/bugcheck.h"


/*
 * For static initializations the linker exports an array of functions. During
 * the start of the driver these functions need to be called.
 * The DriverEntry must call initialize() function to setup the data-structucre
 * needed for the system.
 *
 * These values must be declared inside a single .OBJ file.
 */
#pragma data_seg(".CRT$XCA")
void (*___StartInitCalls__[1])(void)={0};
#pragma data_seg(".CRT$XCZ")
void (*___EndInitCalls__[1])(void)={0};
#pragma data_seg()

DRIVERENTRY void cXDKLibCPP::driverUnload(PDRIVER_OBJECT driverObject)
{
    KdPrint(("XDK: Driver unloaded... %08X\n", driverObject));

    // Destruct the objects
    cXDKLibCPP::unload();
}


DRIVERENTRY NTSTATUS cXDKLibCPP::driverEntry(PDRIVER_OBJECT driverObject,
                                             PUNICODE_STRING registryPath)
{
    KdPrint(("XDK: driverEntry %08X called at %08X\n",
             driverObject,
             driverObject->DriverStart));

    // Setup the driver object
    cDriver::setDriverObject(driverObject);

    // Setup all driver callbacks
    for (uint i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        driverObject->MajorFunction[i] = cDriver::irpCallback;
    }
    driverObject->DriverUnload = cXDKLibCPP::driverUnload;


    // Init the KDK library
    if (!cXDKLibCPP::initialize())
    {
        // Destruct the objects
        cXDKLibCPP::unload();
        // An exception was caught.
        return STATUS_UNSUCCESSFUL;
    }

    // Setup driver entry points
    NTSTATUS retCode = STATUS_UNSUCCESSFUL;

    XSTL_TRY
    {
        // Create the driver instance
        // Test whether the factory module is in use...
        if (cPrivateDriverFactory::m_driverFactory != NULL)
        {
            // Generate the driver object
            cPrivateDriverFactory::getInstance().generateDriverObject();
        }

        // Start the entry point
        retCode = cDriver::getInstance().driverEntry();
        traceHigh("XDK: cDriver::driverEntry completed " << HEXDWORD(retCode) <<
                  "  Loaded at " << HEXDWORD((uint32)driverObject->DriverStart) << endl);
    }
    XSTL_CATCH(cException& e)
    {
        traceHigh("XDK: driverEntry throw exception: " << e.getMessage() << endl);
        retCode = STATUS_UNSUCCESSFUL;
    }
    XSTL_CATCH_ALL
    {
        traceHigh("XDK: driverEntry throw unknown exception: " <<
                  EHLib::getUnknownException() << endl);
        traceHigh("XDK: Loaded at " <<
                   HEXDWORD((uint32)driverObject->DriverStart) << endl);
        retCode = STATUS_UNSUCCESSFUL;
    }

    // Terminate the execution of the driver.
    if (retCode != STATUS_SUCCESS)
    {
        traceHigh("XDK: Main cDriver terminates the driver." << endl);
        cXDKLibCPP::unload();
    }

    return retCode;
}

bool cXDKLibCPP::initialize()
{
    // Init the memory-manager
    cXdkDriverMemoryManager::initialize();
    // Inits the exception mechanizm.
    EHLib::createInstance();
    // Inits the at-exit library
    cAtExit::init();

    // Note: This function execute only when the driver's entry-point is
    //       being called. There is only a single-flow and a single-thread
    //       which can execute this function.
    //       As a result of that, no spin-lock locking is requierd.
    void (**p)(void);

    // Check IRQL level
    #ifdef _DEBUG
    if (cProcessorUtil::getCurrentIrql() != PASSIVE_LEVEL)
    {
        cBugCheck::bugCheck(0xAAA, 0xDEADFACE, 0xDEADFACE, 0xDEADFACE, 1);
    }
    #endif

    XSTL_TRY
    {
        // Inits the XDK-utils, throw exception
        cXDKUtils::getInstance().getCurrentProcessName();
        // Check IRQL level
        #ifdef _DEBUG
        if (cProcessorUtil::getCurrentIrql() != PASSIVE_LEVEL)
        {
            cBugCheck::bugCheck(0xAAA, 0xDEADFACE, 0xDEADFACE, 0xDEADFACE, 2);
        }
        #endif

        // Inits all other static constructors.
        p = ___StartInitCalls__ + 1;
        while (p < ___EndInitCalls__)
        {
            (*p)();
            // Check IRQL level
            #ifdef _DEBUG
            if (cProcessorUtil::getCurrentIrql() != PASSIVE_LEVEL)
            {
                cBugCheck::bugCheck(0xAAA, 0xDEADFACE,
                                           0xDEADFACE, getNumeric(p), 3);
            }
            #endif
            p++;
        }
    }
    XSTL_CATCH_ALL
    {
        KdPrint(("XDK: initialize failed.\n"));
        return false;
    }

    KdPrint(("XDK: initialize completed successfuly.\n"));
    return true;
}


void cXDKLibCPP::unload()
{
    // Clear queue memory objects
    cXdkDriverMemoryManager::clearDtorQueue();

    // Destroies the at-exit functions
    cAtExit::exit();

    // Destruct the exception mechanizm.
    EHLib::destroyInstance();

    #ifdef _DEBUG
        // Cleanup the trace steam memory
        traceStream::cleanUpMemory();
        // Destruct the trace mechanizm
        cXdkTraceSingleton::destroyTraceObject();
    #endif

    // Destruct the smart-pointer lockable
    cSmartPtrLockable::unload();

    // Clear memory-manager
    cXdkDriverMemoryManager::terminate();
}


