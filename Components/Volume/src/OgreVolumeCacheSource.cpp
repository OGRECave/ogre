/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd
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
#include "OgreVolumeCacheSource.h"

namespace Ogre {
namespace Volume {

    CacheSource::CacheSource(const Source *src) : mSrc(src)
    {
    }
    
    //-----------------------------------------------------------------------

    Real CacheSource::getValueAndGradient(const Vector3 &position, Vector3 &gradient) const
    {
        Vector4 result;
        if (mCache.find(position) == mCache.end())
        {
            result.w = mSrc->getValueAndGradient(position, gradient);
            result.x = gradient.x;
            result.y = gradient.y;
            result.z = gradient.z;
            mCache[position] = result;
        }
        else
        {
            result = mCache[position];
            gradient.x = result.x;
            gradient.y = result.y;
            gradient.z = result.z;
        }
        return result.w;
    }
    
    //-----------------------------------------------------------------------

    Real CacheSource::getValue(const Vector3 &position) const
    {
        Vector4 result;
        if (mCache.find(position) == mCache.end())
        {
            Vector3 gradient;
            result.w = mSrc->getValueAndGradient(position, gradient);
            result.x = gradient.x;
            result.y = gradient.y;
            result.z = gradient.z;
            mCache[position] = result;
        }
        else
        {
            result = mCache[position];
        }
        return result.w;
    }

}
}