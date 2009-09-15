/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef _AtomicWrapper_H__
#define _AtomicWrapper_H__

#include <signal.h>
#include "OgrePrerequisites.h"
#include "OgreException.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
    template <class T> class AtomicObject {

        public:

        AtomicObject (const T &initial)
            : mField(initial)
        {   }

        AtomicObject (const AtomicObject<T> &cousin)
            : mField(cousin.get())
        {   }

        AtomicObject ()
        {   }

        void operator= (const AtomicObject<T> &cousin)
        {
            set(cousin.get());
        }

        T get (void) const
        {
            OGRE_LOCK_AUTO_MUTEX
            return mField;
        }

        void set (const T &v)
        {
            OGRE_LOCK_AUTO_MUTEX
            mField = v;
        }

        bool cas (const T &old, const T &nu)
        {
            OGRE_LOCK_AUTO_MUTEX
            if (mField != old) return false;
            mField = nu;
            return true;
        }

        T operator++ (void)
        {
            OGRE_LOCK_AUTO_MUTEX
            return ++mField;
        }

        T operator++ (int)
        {
            OGRE_LOCK_AUTO_MUTEX
            return mField++;
        }

        T operator-- (int)
        {
            OGRE_LOCK_AUTO_MUTEX
            return mField--;
        }

        protected:

        OGRE_AUTO_MUTEX

        volatile T mField;

    };
	/** @} */
	/** @} */

}

// These GCC instrinsics are not supported on ARM - masterfalcon
#if OGRE_COMPILER == OGRE_COMPILER_GNUC && OGRE_COMP_VER >= 412 && OGRE_THREAD_SUPPORT && OGRE_CPU != OGRE_CPU_ARM

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
            __sync_add_and_fetch (&mField, 1);
        }
            
        T operator-- (void)
        {
            __sync_add_and_fetch (&mField, -1);
        }

        T operator++ (int)
        {
            __sync_fetch_and_add (&mField, 1);
        }
            
        T operator-- (int)
        {
            __sync_fetch_and_add (&mField, -1);
        }


        volatile T mField;

    };
	/** @} */
	/** @} */

}

 #elif OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER >= 1400 && OGRE_THREAD_SUPPORT

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // required to stop windows.h messing up std::min
#include <windows.h>
#include <intrin.h>

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
            if (sizeof(T)==2) {
                return _InterlockedCompareExchange16((SHORT*)&mField, static_cast<SHORT>(nu), static_cast<SHORT>(old)) == static_cast<SHORT>(old);
            } 
			else if (sizeof(T)==4) 
			{
                return _InterlockedCompareExchange((LONG*)&mField, static_cast<LONG>(nu), static_cast<LONG>(old)) == static_cast<LONG>(old);
			} 
			else if (sizeof(T)==8) {
                return _InterlockedCompareExchange64((LONGLONG*)&mField, static_cast<LONGLONG>(nu), static_cast<LONGLONG>(old)) == static_cast<LONGLONG>(old);
            } 
			else {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,"Only 16, 32, and 64 bit scalars supported in win32.","AtomicScalar::cas");
            }
        }
            
        T operator++ (void)
        {
            if (sizeof(T)==2) {
                return InterlockedIncrement16((SHORT*)&mField);
            } else if (sizeof(T)==4) {
                return InterlockedIncrement((LONG*)&mField);
            } else if (sizeof(T)==8) {
                return InterlockedIncrement64((LONGLONG*)&mField);
            } else {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,"Only 16, 32, and 64 bit scalars supported in win32.","AtomicScalar::operator++(prefix)");
            }
        }
            
        T operator-- (void)
        {
            if (sizeof(T)==2) {
                return InterlockedDecrement16((SHORT*)&mField);
            } else if (sizeof(T)==4) {
                return InterlockedDecrement((LONG*)&mField);
            } else if (sizeof(T)==8) {
                return InterlockedDecrement64((LONGLONG*)&mField);
            } else {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,"Only 16, 32, and 64 bit scalars supported in win32.","AtomicScalar::operator--(prefix)");
            }
        }

        T operator++ (int)
        {
            if (sizeof(T)==2) {
                return InterlockedIncrement16((SHORT*)&mField)-1;
            } else if (sizeof(T)==4) {
                return InterlockedIncrement((LONG*)&mField)-1;
            } else if (sizeof(T)==8) {
                return InterlockedIncrement64((LONGLONG*)&mField)-1;
            } else {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,"Only 16, 32, and 64 bit scalars supported in win32.","AtomicScalar::operator++(postfix)");
            }
        }
            
        T operator-- (int)
        {
            if (sizeof(T)==2) {
                return InterlockedDecrement16((SHORT*)&mField)+1;
            } else if (sizeof(T)==4) {
                return InterlockedDecrement((LONG*)&mField)+1;
            } else if (sizeof(T)==8) {
                return InterlockedDecrement64((LONGLONG*)&mField)+1;
            } else {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,"Only 16, 32, and 64 bit scalars supported in win32.","AtomicScalar::operator--(postfix)");
            }
        }

        volatile T mField;

    };
	/** @} */
	/** @} */

}

#else

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
            OGRE_LOCK_AUTO_MUTEX
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
            OGRE_LOCK_AUTO_MUTEX
            mField = v;
        }

        bool cas (const T &old, const T &nu)
        {
            OGRE_LOCK_AUTO_MUTEX
            if (mField != old) return false;
            mField = nu;
            return true;
        }

        T operator++ (void)
        {
            OGRE_LOCK_AUTO_MUTEX
            return ++mField;
        }

        T operator-- (void)
        {
            OGRE_LOCK_AUTO_MUTEX
            return --mField;
        }

        T operator++ (int)
        {
            OGRE_LOCK_AUTO_MUTEX
            return mField++;
        }

        T operator-- (int)
        {
            OGRE_LOCK_AUTO_MUTEX
            return mField--;
        }

        protected:

        OGRE_AUTO_MUTEX

        volatile T mField;

    };
	/** @} */
	/** @} */

}

#endif


#endif

