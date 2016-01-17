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
 * ehlib.cpp
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/os.h"
#ifndef EHLIB_STATIC
    #include "xStl/data/datastream.h"
    #include "xStl/except/assert.h"
    #include "xStl/except/trace.h"
    #include "xStl/OS/mutex.h"
    #include "xStl/OS/lock.h"
#endif EHLIB_STATIC
#include "xdk/ehlib/ehlib.h"
#include "xdk/utils/bugcheck.h"

// The EHlib is not initialize yet
EHLib* EHLib::m_instance = NULL;

const character EHLib::m_noException[] =
    XSTL_STRING("Catch block couldn't be found...\n");

void EHLib::createInstance()
{
    if (m_instance != NULL)
    {
        return;
    }

    m_instance = new EHLib();
}

void EHLib::destroyInstance()
{
    if (m_instance == NULL)
    {
        return;
    }

    delete m_instance;
    m_instance = NULL;
}

EHLib::EHLib() :
#ifdef EHLIB_STATIC
    m_isExceptionContext(false)
#else
    m_exceptionThreadContext(EHLib::NORMAL_SYSTEM_THREADS)
#endif // EHLIB_STATIC
{
}

EHLib& EHLib::getInstance()
{
    // Blue screen, if the instance is exist
    if (m_instance == NULL)
    {
        createInstance();
    }

    return *m_instance;
}

EHLib::ThreadID EHLib::getCurrentThreadID()
{
    #ifdef _KERNEL
        return getNumeric(PsGetCurrentThread());
    #else
        return GetCurrentThreadId();
    #endif
}

EHLib::ExceptionThreadBlock::ExceptionThreadBlock() :
    m_exceptionObject(NULL),
    m_exceptionType(NULL)
{
}

uint32* EHLib::getStackBasePointer(EHLib::MSCPPEstablisher* establisherFrame)
{
    return &establisherFrame->m_oldEbp;
}

void* EHLib::executeProc(EHLib::MSCPPEstablisher* establisherFrame,
                         void* proc)
{
    if (proc == NULL) return NULL;

    // Calculate the base of the establisher frame context
    uint32* basePointer = getStackBasePointer(establisherFrame);

    // The return address
    void* continueAddress;

    // Execute the catch-handler and gets the return address.
    // If the catch handler throws exception it will be caught at the second time.
    #pragma warning(push)
    #pragma warning(disable:4731)  // frame pointer register 'ebp' modified by inline assembly code
    _asm {
        ; // Save the base for this function in the stack
        push ebp

        ; // Change the base of the stack and execute handler
        mov eax, proc
        mov edx, basePointer
        mov ebp, edx

        ; // Execute the catch handler
        call eax

        ; // Restore the base for this function
        pop ebp
        mov continueAddress, eax
    }
    #pragma warning(pop)

    // Return the address for
    return continueAddress;
}

uint EHLib::matchCatchBlock(EHLib::MSCPPEstablisher* establisherFrame,
                            EHLib::CatchDescriptor* catchBlocks,
                            EHLib::ExceptionTypeInformation* exceptionType,
                            void* exceptionObject)
{
    uint j;
    for (j = 0; j < catchBlocks->m_catchCount; j++)
    {
        // Get the RTTI for the catch handler
        CatchRTTI* rttiBlock = &(catchBlocks->m_rtti[j]);
        // Test for catch(...) block
        if (rttiBlock->m_rttiDescriptor == NULL)
        {
            // The catch handler can eat all exceptions.
            return j;
        }

        // Enumerate rtti with the exception-type
        if (exceptionType != NULL)
        {
            for (uint i = 0; i < exceptionType->m_rtti->m_count; i++)
            {
                type_info* typeInfo = exceptionType->m_rtti->m_types[i]->m_typeInfo;
                if (strcmp(typeInfo->name(),
                           rttiBlock->m_rttiDescriptor->name()) == 0)
                {
                    // This block is a C++ block which can handle the current
                    // C++ exception

                    // Fix the exception-handler stack
                    if (rttiBlock->m_spoof != 0)
                    {
                        uint8* ebp = (uint8*)getStackBasePointer(establisherFrame);
                        *((uint32*)(ebp + rttiBlock->m_spoof)) = getNumeric(exceptionObject);
                    }

                    // Return the index for the current enumerated handler
                    return j;
                }
            }
        }
    }

    // There aren't any handlers which can handle this exception
    return MISS_CATCH_BLOCK;
}

bool EHLib::callCppMethod(void* object,
                          void* proc)
{
    // Test arguments
    if ((object == NULL) || (proc == NULL)) return false;

    // The library is compiled without the /EHa flag or the /GX flags buts that
    // is valid since there aren't any object which the function is generated.
    try
    {
        // Execute the function.
        _asm {
            mov ecx, object
            call proc
        }

        // The function completed successfully.
        return true;
    } catch (...)
    {
        // An error occured
        return false;
    }
}

void EHLib::continueExecuation(EHLib::MSCPPEstablisher* establisherFrame,
                               void* continueAddress)
{
    // Gets the stack base pointer
    uint32* oldEBP = getStackBasePointer(establisherFrame);
    #pragma warning(push)
    #pragma warning(disable:4731)  // frame pointer register 'ebp' modified by inline assembly code
    _asm {
        mov eax, continueAddress
        mov ebx, establisherFrame
        mov esp, [ebx-4]
        mov ebp, oldEBP
        jmp eax
    }
    #pragma warning(pop)
}

void EHLib::destructTryBlockStack(EHLib::MSCPPEstablisher* establisherFrame,
                                  EHLib::FunctionEHData*   eh,
                                  uint32 revertTryLevel)
{
    if (establisherFrame->m_tryLevel == TRY_LEVEL_NONE)
    {
        // There aren't any constructed objects
        return;
    }

    #ifndef EHLIB_STATIC
    ASSERT(establisherFrame->m_tryLevel < eh->m_countObjectsDtor);
    #endif

    uint tryLevel = TRY_LEVEL_NONE;
    // Start scan all tree levels
    for (int i = establisherFrame->m_tryLevel; i >= 0; i--)
    {
        if ((eh->m_objectsDtor[i].m_id == TRY_LEVEL_NONE) ||
            (eh->m_objectsDtor[i].m_id < tryLevel))
        {
            // Test for tree branch
            if (eh->m_objectsDtor[i].m_proc == NULL)
            {
                if ((int)revertTryLevel == i)
                {
                    if (i >= 0) --i;
                    establisherFrame->m_tryLevel = i;
                    break;
                }
            } else
            {
                // Safe execute destructor.
                // The library is compiled without the /EHa flag or the /GX flags buts that
                // is valid since there aren't any object which the function is generated.
                try
                {
                    // Execute the destructor object
                    executeProc(establisherFrame,
                                eh->m_objectsDtor[i].m_proc);
                } catch (...)
                {
                    #ifndef EHLIB_STATIC
                        TRACE(TRACE_VERY_HIGH, "EHLIB: Exception called during object destructor\n");
                    #endif // EHLIB_STATIC
                }
            }

            tryLevel = eh->m_objectsDtor[i].m_id;

            // Stop when all object destruct
            if (tryLevel == TRY_LEVEL_NONE)
            {
                // Stop
                break;
            }

            // Notice the de-progress of function inside the tree.
            establisherFrame->m_tryLevel = i;
        }
    }
}

uint EHLib::getTryCatchBlock(EHLib::MSCPPEstablisher* establisherFrame,
                             EHLib::FunctionEHData* eh)
{
    for (uint i = 0; i < eh->m_countCatchBlocks; i++)
    {
        if ((eh->m_catchBlocks[i].m_startTryLevel <= establisherFrame->m_tryLevel) &&
            (eh->m_catchBlocks[i].m_endTryLevel >= establisherFrame->m_tryLevel))
        {
            return i;
        }
    }

    // Couldn't find any try block.
    return (uint)TRY_LEVEL_NONE;
}

bool EHLib::copyThreadException(const ThreadID& id,
                                ExceptionThreadBlock& etb,
                                bool shouldRemove)
{
    // Get the context of the EHLIB
    EHLib& instance = getInstance();

#ifdef EHLIB_STATIC
    if (instance.m_isExceptionContext)
    {
        etb = instance.m_exceptionContext;
        if (shouldRemove)
            instance.m_isExceptionContext = false;
        return true;
    }
    return false;
#else
    // Block the exception database
    cLock lock(instance.m_exceptionThreadContextMutex);

    if (!instance.m_exceptionThreadContext.hasKey(id))
        return false;

    etb = instance.m_exceptionThreadContext[id];

    if (shouldRemove)
        instance.m_exceptionThreadContext.remove(id);

    return true;
#endif EHLIB_STATIC
}

void EHLib::freeCurrentThreadException()
{
    // Get the current thread-id
    ThreadID id = getCurrentThreadID();

    ExceptionThreadBlock etb;
    if (copyThreadException(id, etb, true))
    {
        // Destroy the C++ exception
        if ((etb.m_exceptionObject != NULL) &&
            (etb.m_exceptionType != NULL) &&
            (etb.m_exceptionType->m_destructor != NULL))
        {
            // Executed at the previous mode.
            if (!callCppMethod(etb.m_exceptionObject,
                               etb.m_exceptionType->m_destructor))
            {
                #ifndef EHLIB_STATIC
                    TRACE(TRACE_VERY_HIGH, "EHLIB: Exception throw during exception destruction\n");
                #endif
            }
        }
    }
}

#ifndef EHLIB_STATIC
cString EHLib::getUnknownException()
{
    // Get the current thread-id
    ThreadID id = getCurrentThreadID();

    // Gets the current exception.
    ExceptionThreadBlock etb;
    if (copyThreadException(id, etb))
    {
        cString ret;

        if ((etb.m_exceptionObject != NULL) &&
            (etb.m_exceptionType != NULL))
        {
            // C++ exception
            ret = cString(etb.m_exceptionType->m_rtti->m_types[0]->m_typeInfo->name());
        } else
        {
            // OS exception
            ret = HEXDWORD(etb.m_exceptionRecord.ExceptionCode);
        }

        // Add the location
        ret+= XSTL_STRING("   EIP: ");
        ret+= HEXDWORD(getNumeric(etb.m_exceptionRecord.ExceptionAddress));
        return ret;
    } else
    {
        return m_noException;
    }
}
#endif EHLIB_STATIC

void EHLib::storeNewThreadException(const ExceptionThreadBlock& exception)
{
    // Get the current thread-id
    ThreadID id = getCurrentThreadID();

    // Get the context of the EHLIB
    EHLib& instance = getInstance();

#ifdef EHLIB_STATIC
    instance.m_isExceptionContext = true;
    instance.m_exceptionContext = exception;
#else
    // Block the exception database
    cLock lock(instance.m_exceptionThreadContextMutex);
    // Append the exception object
    instance.m_exceptionThreadContext.append(id, exception);
#endif // EHLIB_STATIC
}

EXCEPTION_DISPOSITION EHLib::internalFrameHandler(
        EHLib::ExceptionRecord*  exceptionRecord,
        EHLib::MSCPPEstablisher* establisherFrame,
        struct _CONTEXT*         contextRecord,
        struct _CONTEXT**        dispatcherContext,
        EHLib::FunctionEHData*   eh,
        uint                     exceptionHandlingVersion)
{
    // Free the previous exception for this list.
    freeCurrentThreadException();

    // Test whether the exception state is unwinding
    if ((eh == NULL) ||
        (eh->m_magic != exceptionHandlingVersion) ||
        ((exceptionRecord->ExceptionFlags & EH_UNWIND_MODE) != 0))
    {
        // Not my problem.
        return ExceptionContinueSearch;
    }

    if ((eh->m_magic != FRAME_HANDLER_TYPE_0) &&
        (eh->m_magic != FRAME_HANDLER_TYPE_3))
    {
        // Unkown frame handler
        return ExceptionContinueSearch;
    }


    // Test whether the exception is a OS exception or C++ exception.
    if ((exceptionRecord->ExceptionCode != EHLIB_EXCEPTION_CODE) ||
        (exceptionRecord->NumberParameters != NUMBER_OF_EXCEPTION_ARGUMENTS) ||
        (exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_MAGIC] != EHLIB_EXCEPTION_CODE) ||
        (exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_CONTEXT] == NULL) ||
        (exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_RTTI] == NULL))
    {
        // OS exception, change the filleds to point to OS exception
        /*
        TRACE(TRACE_VERY_HIGH, cString(XSTL_STRING("EHLIB: OS exception throw at ")) +
                                       HEXDWORD((uint32)exceptionRecord->ExceptionAddress));
        /**/
        exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_CONTEXT] = NULL;
        exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_RTTI] = NULL;
    }

    // Generate new exception context
    ExceptionThreadBlock newException;

    cOS::memcpy(&newException.m_exceptionRecord,
           exceptionRecord,
           sizeof(ExceptionRecord));
    newException.m_exceptionObject =
        (void*)exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_CONTEXT];
    newException.m_exceptionType =
        (ExceptionTypeInformation*)exceptionRecord->ExceptionInformation[EXCEPTION_ARGUMENT_RTTI];

    // Enumerates all exception handling blocks
    for (uint i = getTryCatchBlock(establisherFrame, eh);
         i != TRY_LEVEL_NONE;
         i = getTryCatchBlock(establisherFrame, eh))
    {
        // Revert the current stack frame
        destructTryBlockStack(establisherFrame,
                              eh,
                              eh->m_catchBlocks[i].m_startTryLevel);

        // Test whether
        uint catchCode = matchCatchBlock(establisherFrame,
                                         &(eh->m_catchBlocks[i]),
                                         newException.m_exceptionType,
                                         newException.m_exceptionObject);

        if (catchCode == MISS_CATCH_BLOCK)
        {
            // There isn't any handler in the current try-catch block.
            // Continue revert stack backwards.
            continue;
        }

        #ifdef EHLIB_SPECIAL_EXCEPTIONS
            // Simply restore the fs:[0] pointer to the establisher frame
            EhlibUnwind(establisherFrame);
        #else
            // Invoke a call to RtlUnwind(establisherFrame, 0,0,0)
            // Change the fs:[0] to the previous exception-block
            _global_unwind2(establisherFrame);
        #endif

        // Save the exception record in the current thread-exception-context
        storeNewThreadException(newException);

        // Execute the catch handler
        void* continueAddress = executeProc(establisherFrame,
                                            eh->m_catchBlocks[i].m_rtti[catchCode].m_proc);

        // Destruct the exception
        freeCurrentThreadException();

        // Continue the flow of the application
        continueExecuation(establisherFrame,
                           continueAddress);

        // Should never return
        #ifdef _KERNEL
            cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE, 0,0,0,1);
        #else
            ExitProcess(EHLIB_EXCEPTION_CODE);
        #endif
    }

    // Revert the entire stack frame
    destructTryBlockStack(establisherFrame,
                          eh,
                          TRY_LEVEL_NONE);

    // The function doens't contain any matching handler, throw the exception to the
    // previous handler.
    return ExceptionContinueSearch;
}


void EHLib::internalThrowException(void* objectContext,
                    EHLib::ExceptionTypeInformation* objectType,
                    void* exceptionLocation)
{
    //////////////////////////////////////////
    // Prepare the exception

    // Save the exception into a struct
    EHLib::ExceptionRecord exceptionRecord;

    // Fill the struct
    exceptionRecord.ExceptionCode = EHLIB_EXCEPTION_CODE;
    exceptionRecord.ExceptionAddress = exceptionLocation;
    exceptionRecord.ExceptionFlags = 0;
    exceptionRecord.ExceptionRecord = NULL;
    exceptionRecord.NumberParameters = NUMBER_OF_EXCEPTION_ARGUMENTS;
    exceptionRecord.ExceptionInformation[EXCEPTION_ARGUMENT_CONTEXT] = getNumeric(objectContext);
    exceptionRecord.ExceptionInformation[EXCEPTION_ARGUMENT_RTTI] = getNumeric(objectType);
    exceptionRecord.ExceptionInformation[EXCEPTION_ARGUMENT_MAGIC] = EHLIB_EXCEPTION_CODE;

    //////////////////////////////////////////
    // Raise the exception

    #ifdef EHLIB_SPECIAL_EXCEPTIONS
        // Raise the exception using
        EhlibRaiseException(&exceptionRecord);
    #else
        // Raise the exception using undocumented internal function.
        // The EHLIB_EXCEPTION_CODE is a marker for C++ exception.
        #ifdef _KERNEL
            RtlRaiseException(&exceptionRecord);
        #else
            RaiseException(EHLIB_EXCEPTION_CODE,
                        0,
                        NUMBER_OF_EXCEPTION_ARGUMENTS,
                        exceptionRecord.ExceptionInformation);
        #endif
    #endif // EHLIB_SPECIAL_EXCEPTIONS

    //////////////////////////////////////////
    // Error handling.

    // This code should never run! Otherwise exception is not handled
    #ifdef _KERNEL
        cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE, 0,0,0,0xDEAD);
    #else
        ExitProcess(EHLIB_EXCEPTION_CODE);
    #endif

}

#ifdef EHLIB_SPECIAL_EXCEPTIONS
void EHLib::EhlibRaiseException(EHLib::ExceptionRecord* exceptionRecord)
{
    // Get the current thread information records
    NtThreadInformationBlock tib = getCurrentThreadInformationBlock();
    // Get the first handler variable
    MSCPPEstablisher* handler = tib.exceptionListHead;
    if ((handler == NULL) ||
        (handler == (MSCPPEstablisher*)getPtr((addressNumericValue)(-1L))))
    {
        #ifndef EHLIB_STATIC
            TRACE(TRACE_VERY_HIGH, "EhlibRaiseException: Exception Not Handled bug!\n");
        #endif // EHLIB_STATIC
        cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE, 0,0,1,0xDEAD);
    }

    // Start executing all handlers in a loop
    do
    {
        // Check frame handler range
        addressNumericValue handlerPosition = getNumeric(handler);
        /* TODO NOTE: In windows 7, the TIB info is situated at a different
           location (not fs:[4] and fs:[8]). We need a better way to get
           this info for different operating systems */
        if ((0 != tib.stackBase) && (0 != tib.stackLimit))
        {
            if ((handlerPosition >= tib.stackBase) ||
                (handlerPosition <= tib.stackLimit))
            {
                #ifndef EHLIB_STATIC
                TRACE(TRACE_VERY_HIGH, "EhlibRaiseException: Exception frame handler invalid!\n");
                #endif // EHLIB_STATIC
                cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE,
                                    handlerPosition,
                                    tib.stackBase,
                                    tib.stackLimit,
                                    0xDEAD);
            }
        }

        // Execute the handler function
        EXCEPTION_DISPOSITION returnedValue =
            handler->m_exceptionHandler(exceptionRecord,
            handler,
            // The CONTEXT registers are not important for the EHLib.
            NULL,
            // The CONTEXT registers are not important for the EHLib.
            NULL);

        // Handle the returned operation
        switch (returnedValue)
        {
        case ExceptionContinueSearch:
            break;
        case ExceptionContinueExecution:
            #ifndef EHLIB_STATIC
                TRACE(TRACE_VERY_HIGH, "EhlibRaiseException: ExceptionContinueExecution returned!\n");
            #endif // EHLIB_STATIC
            // This will kill the application...
            cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE, 0,0,3,0xDEAD);
            return;
        case ExceptionNestedException:
        case ExceptionCollidedUnwind:
        default:
            #ifndef EHLIB_STATIC
            TRACE(TRACE_VERY_HIGH, "EhlibRaiseException: FrameHandler illigle returned code!\n");
            #endif // EHLIB_STATIC
            cBugCheck::bugCheck(EHLIB_EXCEPTION_CODE, 0,0,4,0xDEAD);
            return;
        }

        // Continue search.
        #ifndef EHLIB_STATIC
            ASSERT(returnedValue == ExceptionContinueSearch);
        #endif // EHLIB_STATIC
        // Revert to the old exception handling function
        handler = handler->m_previousHandler;
    } while ((handler != NULL) &&
            (handler != (MSCPPEstablisher*)getPtr((addressNumericValue)(-1L))));

    #ifndef EHLIB_STATIC
    TRACE(TRACE_VERY_HIGH, "EhlibRaiseException: Exception Not Handled.\n");
    #endif  //EHLIB_STATIC
}

void EHLib::EhlibUnwind(MSCPPEstablisher* establisherFrame)
{
    _asm {
        mov eax, establisherFrame
        mov fs:[0], eax
    }
}
#endif // EHLIB_SPECIAL_EXCEPTIONS

EHLib::NtThreadInformationBlock EHLib::getCurrentThreadInformationBlock()
{
    NtThreadInformationBlock returnedValue;
    _asm {
        mov eax, fs:[0]
        mov returnedValue.exceptionListHead, eax
        mov eax, fs:[4]
        mov returnedValue.stackBase, eax
        mov eax, fs:[8]
        mov returnedValue.stackLimit, eax
    }

    return returnedValue;
}

void* EHLib::resetExceptionHandlingList()
{
    void* returnedPointer;
    _asm {
        mov eax, fs:[0]
        mov returnedPointer, eax
        mov eax, 0FFFFFFFFh
        mov fs:[0], eax
    }
    return returnedPointer;
}

void EHLib::restoreExceptionList(void* list)
{
    _asm {
        mov eax, list
        mov fs:[0], eax
    }
}

//////////////////////////////////////////////////////////////
// Debugging API
#ifdef _DEBUG
bool EHLib::DEBUGGED = false;

void EHLib::markExceptionDebugging()
{
    DEBUGGED = true;
}
#endif

