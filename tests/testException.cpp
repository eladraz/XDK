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
 * testException.cpp
 *
 * Tests the exception mechanism for the XDK utility.
 * The module tests the following:
 * 1. Destructor are called when exception occurred.
 * 2. The exception is received as needed.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/except/trace.h"
#include "xStl/data/smartptr.h"
#include "xStl/data/string.h"
#include "xStl/data/list.h"
#include "xStl/data/char.h"
#include "xStl/data/smartptr.h"
#include "../tests/tests.h"

class cTestException : public cTestObject
{
public:
    /*
     * Reference-count for destruction objects.
     * The module count all counstructd object and all destruction objects.
     */
    class DestructorSignner
    {
    public:
        DestructorSignner() { gCounter++; }
        ~DestructorSignner() { gCounter--; }
        static uint gCounter;
    };

    void stackObject()
    {
        DestructorSignner object;
    }

    void heapObject()
    {
        cSmartPtr<DestructorSignner> object(new DestructorSignner());
    }

    void exceptObject()
    {
        XSTL_TRY
        {
            // Construct the object
            DestructorSignner object;
            // Throw exception
            XSTL_THROW(cException(EXCEPTION_FAILED));
            // Object is still exist
        }
        XSTL_CATCH_ALL
        {
            // Object must destruct
        }
    }

    void testDestructors()
    {
        TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);

        for (uint i = 0; i < 100; i++)
        {
            stackObject();
            heapObject();
            exceptObject();
        }

        TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);
    }

    class CTOR_EXCEPTION { public: CTOR_EXCEPTION() { XSTL_THROW(cException(EXCEPTION_FAILED)); }; };
    void testConstructorException()
    {
        uint i;
        bool th = false;
        XSTL_TRY {
            TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);
            for (i = 0; i < 100; i++)
            {
                DestructorSignner object;
                bool b = false;
                XSTL_TRY
                {
                    TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 1);
                    DestructorSignner object;
                    TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 2);
                    CTOR_EXCEPTION tempObject;
                } XSTL_CATCH_ALL {
                    b = true;
                }
                TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 1);
                TESTS_ASSERT(b);
                DestructorSignner object1;
                if (i == 50)
                {
                    DestructorSignner object;
                    CTOR_EXCEPTION tempObject;
                }
            }
        } XSTL_CATCH (cException&) {
            th = true;
        } XSTL_CATCH_ALL {
            TESTS_ASSERT(false);
        }
        TESTS_ASSERT(th);
        TESTS_ASSERT_EQUAL(i, 50);
        TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);
    }

    void doExceptionFromHeapObject()
    {
        cSmartPtr<CTOR_EXCEPTION> blabla(new CTOR_EXCEPTION());
    }

    void dualFunctionBridge()
    {
        doExceptionFromHeapObject();
    }

    void testFromHeapObject()
    {
        XSTL_TRY {
            dualFunctionBridge();
        } XSTL_CATCH_ALL {
            return;
        }
        TESTS_ASSERT(false);
    }

    bool alwaysTrue()
    {
        return true;
    }

    void testNumberOfDtors()
    {
        TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);
        for (uint i = 0; i < 10; i++)
        {
            DestructorSignner minus2;
            DestructorSignner minus1;
            XSTL_TRY {
                DestructorSignner a;
                DestructorSignner b;
                DestructorSignner c;
                if (alwaysTrue()) {
                    DestructorSignner d;
                } else {
                    DestructorSignner e;
                }
                if (alwaysTrue()) {
                    XSTL_THROW(0);
                }
            } XSTL_CATCH_ALL {
                DestructorSignner plus1;
                DestructorSignner plus2;
            }
        }
        TESTS_ASSERT_EQUAL(DestructorSignner::gCounter, 0);
    }

    // Perform the test
    virtual void test()
    {
        testDestructors();
        testConstructorException();
        testFromHeapObject();
        testNumberOfDtors();
    };

    // Return the name of the module
    virtual cString getName() { return __FILE__; }
};


// Instance test object
cTestException g_global;

// Init the count object
uint cTestException::DestructorSignner::gCounter = 0;
