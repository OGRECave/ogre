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
#ifndef __AtomicObject_H__
#define __AtomicObject_H__

#include <signal.h>
#include "OgrePrerequisites.h"
#include "Threading/OgreThreadHeaders.h"

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
            OGRE_LOCK_AUTO_MUTEX;
            return mField;
        }

        void set (const T &v)
        {
            OGRE_LOCK_AUTO_MUTEX;
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

