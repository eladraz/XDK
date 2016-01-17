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

#ifndef __TBA_XSTL_IOCTL_UTILS_LISTENER_H
#define __TBA_XSTL_IOCTL_UTILS_LISTENER_H

/*
 * IoctlListener.h
 *
 * Interface for IOCTL callbacks. See IoctlDispatcher.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */

#include "xStl/types.h"
#include "xStl/data/smartPtr.h"

/*
 * class cIoctlListener
 *
 * Interface for IOCTL callbacks.
 * See cIoctlDispatcher
 */
class cIoctlListener {
public:
    /*
     * Virtual destructor. The class is abstracts.
     */
    virtual ~cIoctlListener() {};

    /*
     * Callback for this interface.
     *
     *  ioctlCode           - The IOCTL command
     *  inputBuffer         - The buffer from the user
     *  inputBufferLength   - The length of the inputBuffer in bytes
     *  outputBuffer        - The buffer which the result should be written to
     *                        NOTICE: This buffer can be overlapped with the
     *                                inputBuffer
     *  outputBufferLength  - The size in bytes of the 'outputBuffer'
     *
     * Return the number of bytes written to the output buffer
     */
    virtual uint handleIoctl(uint    ioctlCode,
                        const uint8* inputBuffer,
                        uint         inputBufferLength,
                        uint8*       outputBuffer,
                        uint         outputBufferLength) = 0;
};

// The smart-pointer for this class
typedef cSmartPtr<cIoctlListener> cIoctlListenerPtr;

/*
 * This macro generate fanctor thunk that will generate a call to an handleIoctl
 * implementation under a different name.
 * The usage of this macro come in order when a class wants to implement different
 * handlers ('handleIoctl') in the same class. This is ofcourse impossiable.
 *
 * However using this class this can be done easly:
 *
 * class ManyIoctlsClass {
 * protected:
 *      uint handleLockVolumeIoctl(uint ioctlCode, ...);
 *      uint handleUnlockVolumeIoctl(uint ioctlCode, ...);
 *      uint handleReadSectorIoctl(uint ioctlCode, ...);
 *
 *      IOCTL_CALLBACK(ManyIoctlsClass, handleLockVolumeIoctl);
 *      IOCTL_CALLBACK(ManyIoctlsClass, handleUnlockVolumeIoctl);
 *      IOCTL_CALLBACK(ManyIoctlsClass, handleReadSectorIoctl);
 * };
 *
 * ManyIoctlsClass::registerIoctls()
 * {
 *    m_dispatcher.registerIoctlHandler(MANY_LOCK_IOCTL,
 *              IOCTL_INSTANCE(handleLockVolumeIoctl));
 *    ...
 * }
 */
#define IOCTL_CALLBACK(class_name, function_name)                      \
    class cIoctlDispatcherClass##function_name : public cIoctlListener { \
    public:                                                              \
        /* Default constructor. Setup the instance for the class */      \
        cIoctlDispatcherClass##function_name(class_name* instance) :     \
            m_instance(instance)                                         \
        {                                                                \
        }                                                                \
                                                                         \
        /* The dispatcher function */                                    \
        virtual uint handleIoctl(uint    ioctlCode,                      \
                            const uint8* inputBuffer,                    \
                            uint         inputBufferLength,              \
                            uint8*       outputBuffer,                   \
                            uint         outputBufferLength)             \
        {                                                                \
            return m_instance->function_name(ioctlCode,                  \
                                      inputBuffer,                       \
                                      inputBufferLength,                 \
                                      outputBuffer,                      \
                                      outputBufferLength);               \
        }                                                                \
                                                                         \
    private:                                                             \
        /* Deny copy-constructor and operator = */                       \
        cIoctlDispatcherClass##function_name                             \
            (const cIoctlDispatcherClass##function_name& other);         \
        cIoctlDispatcherClass##function_name& operator=                  \
            (const cIoctlDispatcherClass##function_name& other);         \
        /* The instance to invoke the callback to */                     \
        class_name* m_instance;                                          \
    };


/*
 * This macro generate a unique functor that will invoke the 'function' when
 * it's called. See IOCTL_CALLBACK
 */
#define IOCTL_INSTANCE(function_name) \
    cIoctlListenerPtr((new cIoctlDispatcherClass##function_name(this)))


#endif // __TBA_XSTL_IOCTL_UTILS_LISTENER_H
