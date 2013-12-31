/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __AtomicScalar_H__
#define __AtomicScalar_H__

#include <signal.h>
#include "OgrePrerequisites.h"
#include "OgreException.h"
#include "OgrePlatformInformation.h"

#if (((OGRE_COMPILER == OGRE_COMPILER_GNUC) && (OGRE_COMP_VER >= 412)) || (OGRE_COMPILER == OGRE_COMPILER_CLANG)) && OGRE_THREAD_SUPPORT

// Atomics are not yet supported for the unsigned long long int(ResourceHandle) type as of Clang 5.0. So only GCC for now.
#if ((OGRE_COMPILER == OGRE_COMPILER_GNUC) && (OGRE_COMP_VER >= 473))
    #define BUILTIN_FETCH_ADD(var, add) __atomic_fetch_add (var, add, __ATOMIC_SEQ_CST);
    #define BUILTIN_ADD_FETCH(var, add) __atomic_add_fetch (var, add, __ATOMIC_SEQ_CST);
    #define BUILTIN_SUB_FETCH(var, sub) __atomic_sub_fetch (var, sub, __ATOMIC_SEQ_CST);
#else
    #define BUILTIN_FETCH_ADD(var, add) __sync_fetch_and_add (var, add);
    #define BUILTIN_ADD_FETCH(var, add) __sync_add_and_fetch (var, add);
    #define BUILTIN_SUB_FETCH(var, sub) __sync_sub_and_fetch (var, sub);
#endif

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
    template<class T> class AtomicScalar
    {

        public:

        AtomicScalar (const T &initial)
            : mField(initial)
        {   }

        AtomicScalar (const AtomicScalar<T> &cousin)
            : mField(cousin.mField)
        {   }

        AtomicScalar () 
        {   }

        void operator= (const AtomicScalar<T> &cousin)
        {
            mField = cousin.mField;
        }

        T get (void) const
        {
            return mField;
        }

        void set (const T &v)
        {
            mField = v; 
        }   

        bool cas (const T &old, const T &nu)
        {
            return __sync_bool_compare_and_swap (&mField, old, nu);
        }
            
        T operator++ (void)
        {
            return BUILTIN_ADD_FETCH (&mField, 1);
        }
            
        T operator-- (void)
        {
            return BUILTIN_ADD_FETCH (&mField, -1);
        }

        T operator++ (int)
        {
            return BUILTIN_FETCH_ADD (&mField, 1);
        }
            
        T operator-- (int)
        {
            return BUILTIN_FETCH_ADD (&mField, -1);
        }

		T operator+=(const T &add)
		{
			return BUILTIN_ADD_FETCH (&mField, add);
		}

		T operator-=(const T &sub)
		{
			return BUILTIN_SUB_FETCH (&mField, sub);
		}

        // Need special alignment for atomic functions on ARM CPU's
#if OGRE_CPU == OGRE_CPU_ARM
#   if OGRE_COMPILER == OGRE_COMPILER_MSVC
        __declspec(align(16)) volatile T mField;
#   elif (OGRE_COMPILER == OGRE_COMPILER_GNUC) || (OGRE_COMPILER == OGRE_COMPILER_CLANG)
        volatile T mField __attribute__((__aligned__(16)));
#   endif
#else
        volatile T mField;
#endif

    };
	/** @} */
	/** @} */

}


 #elif OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1400 && OGRE_THREAD_SUPPORT

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif
#include <windows.h>
#include <intrin.h>
#include "Threading/OgreThreadHeaders.h"

// Save warnings state
#   pragma warning (push)
#   pragma warning (disable : 4244)



namespace Ogre {

    // a hack so we can support windows xp.
#define NEED_TO_INIT_INTERLOCKEDCOMPAREEXCHANGE64WRAPPER
    struct _OgreExport InterlockedCompareExchange64Wrapper
    {
        InterlockedCompareExchange64Wrapper();

        typedef 
            LONGLONG
            (WINAPI *func_InterlockedCompareExchange64)( 
            __inout LONGLONG volatile *Destination,
            __in    LONGLONG Exchange,
            __in    LONGLONG Comperand) ;

        static func_InterlockedCompareExchange64 Ogre_InterlockedCompareExchange64;

        static FORCEINLINE
            LONGLONG
            Ogre_InterlockedIncrement64 (
            __inout LONGLONG volatile *Addend
            )
        {
            LONGLONG Old;

            do {
                Old = *Addend;
            } while (Ogre_InterlockedCompareExchange64(Addend,
                Old + 1,
                Old) != Old);

            return Old + 1;
        }

        static FORCEINLINE
            LONGLONG
            Ogre_InterlockedDecrement64 (
            __inout LONGLONG volatile *Addend
            )
        {
            LONGLONG Old;

            do {
                Old = *Addend;
            } while (Ogre_InterlockedCompareExchange64(Addend,
                Old - 1,
                Old) != Old);

            return Old - 1;
        }

    };

    /** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
    template<class T> class AtomicScalar
    {

        public:

        AtomicScalar (const T &initial)
            : mField(initial)
        {   }

        AtomicScalar (const AtomicScalar<T> &cousin)
            : mField(cousin.mField)
        {   }

        AtomicScalar () 
        {   }

        void operator= (const AtomicScalar<T> &cousin)
        {
            mField = cousin.mField;
        }

        T get (void) const
        {
            return mField;
        }

        void set (const T &v)
        {
            mField = v;
        }   

        bool cas (const T &old, const T &nu)
        {
            if (sizeof(T)==2) {
                return _InterlockedCompareExchange16((SHORT*)&mField, static_cast<SHORT>(nu), static_cast<SHORT>(old)) == static_cast<SHORT>(old);
            } 
			else if (sizeof(T)==4) 
			{
                return _InterlockedCompareExchange((LONG*)&mField, static_cast<LONG>(nu), static_cast<LONG>(old)) == static_cast<LONG>(old);
			} 
			else if (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL) {
                return InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64((LONGLONG*)&mField, static_cast<LONGLONG>(nu), static_cast<LONGLONG>(old)) == static_cast<LONGLONG>(old);
            } 
			else {
                OGRE_LOCK_AUTO_MUTEX;
                if (mField != old) return false;
                mField = nu;
                return true;
            }
        }
            
        T operator++ (void)
        {
            if (sizeof(T)==2) {
                return _InterlockedIncrement16((SHORT*)&mField);
            } else if (sizeof(T)==4) {
                return InterlockedIncrement((LONG*)&mField);
            } else if (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL) {
                return InterlockedCompareExchange64Wrapper::Ogre_InterlockedIncrement64((LONGLONG*)&mField);
            } else {
                OGRE_LOCK_AUTO_MUTEX;
                return ++mField;
            }
        }
            
        T operator-- (void)
        {
            if (sizeof(T)==2) {
                return _InterlockedDecrement16((SHORT*)&mField);
            } else if (sizeof(T)==4) {
                return InterlockedDecrement((LONG*)&mField);
            } else if (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL) {
                return InterlockedCompareExchange64Wrapper::Ogre_InterlockedDecrement64((LONGLONG*)&mField);
            } else {
                OGRE_LOCK_AUTO_MUTEX;
                return --mField;
            }
        }

        T operator++ (int)
        {
            if (sizeof(T)==2) {
                return _InterlockedIncrement16((SHORT*)&mField)-1;
            } else if (sizeof(T)==4) {
                return InterlockedIncrement((LONG*)&mField)-1;
            } else if (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL) {
                return InterlockedCompareExchange64Wrapper::Ogre_InterlockedIncrement64((LONGLONG*)&mField)-1;
            } else {
                OGRE_LOCK_AUTO_MUTEX;
                return mField++;
            }
        }
            
        T operator-- (int)
        {
            if (sizeof(T)==2) {
                return _InterlockedDecrement16((SHORT*)&mField)+1;
            } else if (sizeof(T)==4) {
                return InterlockedDecrement((LONG*)&mField)+1;
            } else if (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL) {
                return InterlockedCompareExchange64Wrapper::Ogre_InterlockedDecrement64((LONGLONG*)&mField)+1;
            } else {
                OGRE_LOCK_AUTO_MUTEX;
                return mField--;
            }
        }

		T operator+=(const T &add)
		{
            if ((sizeof(T)==2) || (sizeof(T)==4) || (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL)) {
                //The function InterlockedExchangeAdd is not available for 64 and 16 bit version
                //We will use the cas operation instead. 
                T newVal;
                do {
                    //Create a value of the current field plus the added value
                    newVal = mField + add;
                    //Replace the current field value with the new value. Ensure that the value 
                    //of the field hasn't changed in the mean time by comparing it to the new value
                    //minus the added value. 
                } while (!cas(newVal - add, newVal)); //repeat until successful
                return newVal;
            }
            else
            {
                OGRE_LOCK_AUTO_MUTEX;
                mField += add;
                return mField;
            }
		}

		T operator-=(const T &sub)
		{
            if ((sizeof(T)==2) || (sizeof(T)==4) || (sizeof(T)==8 && InterlockedCompareExchange64Wrapper::Ogre_InterlockedCompareExchange64 != NULL)) {
                //The function InterlockedExchangeAdd is not available for 64 and 16 bit version
                //We will use the cas operation instead. 
                T newVal;
                do {
                    //Create a value of the current field plus the added value
                    newVal = mField - sub;
                    //Replace the current field value with the new value. Ensure that the value 
                    //of the field hasn't changed in the mean time by comparing it to the new value
                    //minus the added value. 
                } while (!cas(newVal + sub, newVal)); //repeat until successful
                return newVal;
            }
            else
            {
                OGRE_LOCK_AUTO_MUTEX;
                mField -= sub;
                return mField;
            }
		}

        protected:

        OGRE_AUTO_MUTEX;

        volatile T mField;

    };
	/** @} */
	/** @} */

}

#   pragma warning (pop)

#else

#include "Threading/OgreThreadHeaders.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
    template <class T> class AtomicScalar {

        public:

        AtomicScalar (const T &initial)
            : mField(initial)
        {   }

        AtomicScalar (const AtomicScalar<T> &cousin)
            : mField(cousin.mField)
        {   }

        AtomicScalar ()
        {   }

        void operator= (const AtomicScalar<T> &cousin)
        {
            mField = cousin.mField;
        }

        T get (void) const
        {
            // no lock required here
            // since get will not interfere with set or cas
            // we may get a stale value, but this is ok
            return mField;
        }

        void set (const T &v)
        {
            mField = v;
        }

        bool cas (const T &old, const T &nu)
        {
            OGRE_LOCK_AUTO_MUTEX;
            if (mField != old) return false;
            mField = nu;
            return true;
        }

        T operator++ (void)
        {
            OGRE_LOCK_AUTO_MUTEX;
            return ++mField;
        }

        T operator-- (void)
        {
            OGRE_LOCK_AUTO_MUTEX;
            return --mField;
        }

        T operator++ (int)
        {
            OGRE_LOCK_AUTO_MUTEX;
            return mField++;
        }

        T operator-- (int)
        {
            OGRE_LOCK_AUTO_MUTEX;
            return mField--;
        }

		T operator+=(const T &add)
		{
            OGRE_LOCK_AUTO_MUTEX;
			mField += add;
			return mField;
		}

		T operator-=(const T &sub)
		{
            OGRE_LOCK_AUTO_MUTEX;
			mField -= sub;
			return mField;
		}

        protected:

        OGRE_AUTO_MUTEX;

        volatile T mField;

    };
	/** @} */
	/** @} */

}

#endif

#endif

