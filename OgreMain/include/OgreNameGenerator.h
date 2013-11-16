/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __NameGenerator_H__
#define __NameGenerator_H__

#include "OgreString.h"

#if defined ( OGRE_GCC_VISIBILITY )
#   pragma GCC visibility push(default)
#endif

#include <sstream>

#if defined ( OGRE_GCC_VISIBILITY )
#   pragma GCC visibility pop
#endif

#include "OgreHeaderPrefix.h"

#include "Threading/OgreThreadHeaders.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /// Utility class to generate a sequentially numbered series of names
    class _OgreExport NameGenerator
    {
    protected:
        String mPrefix;
        unsigned long long int mNext;
        OGRE_AUTO_MUTEX;
    public:
        NameGenerator(const NameGenerator& rhs)
            : mPrefix(rhs.mPrefix), mNext(rhs.mNext) {}
        
        NameGenerator(const String& prefix) : mPrefix(prefix), mNext(1) {}

        /// Generate a new name
        String generate()
        {
            OGRE_LOCK_AUTO_MUTEX;
            StringStream s;
            s << mPrefix << mNext++;
            return s.str();
        }

        /// Reset the internal counter
        void reset()
        {
            OGRE_LOCK_AUTO_MUTEX;
            mNext = 1ULL;
        }

        /// Manually set the internal counter (use caution)
        void setNext(unsigned long long int val)
        {
            OGRE_LOCK_AUTO_MUTEX;
            mNext = val;
        }

        /// Get the internal counter
        unsigned long long int getNext() const
        {
            // lock even on get because 64-bit may not be atomic read
            OGRE_LOCK_AUTO_MUTEX;
            return mNext;
        }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
