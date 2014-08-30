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
#ifndef __Ogre_Volume_CacheSource_H__
#define __Ogre_Volume_CacheSource_H__

#include "OgreVector4.h"

#include "OgreVolumeSource.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {
    
    /** A less operator. 
    @note
        This operator is needed so that Vertex can serve as the key in a map structrue 
    @param a
        The first vector to test.
    @param b
        The second vector to test.
    */
    bool _OgreVolumeExport operator<(const Vector3& a, const Vector3& b);

    /** A caching Source.
    */
    class _OgreVolumeExport CacheSource : public Source
    {
    protected:
        
        /// Map for the cache
        typedef map<Vector3, Vector4>::type UMapPositionValue;
        mutable UMapPositionValue mCache;

        /// The source to cache.
        const Source *mSrc;
        
        /** Gets a density value and gradient from the cache.
        @param position
            The position of the density value and gradient.
        @return
            The density value (w-component) and the gradient (x, y and z component).
        */
        inline Vector4 getFromCache(const Vector3 &position) const
        {
            Vector4 result;
            map<Vector3, Vector4>::iterator it = mCache.find(position);
            if (it == mCache.end())
            {
                result = mSrc->getValueAndGradient(position);
                mCache[position] = result;
            }
            else
            {
                result = it->second;
            }
            return result;
        }

    public:
        
        /** Constructor.
        @param src
            The source to cache.
        */
        CacheSource(const Source *src);
        
        /** Overridden from Source.
        */
        virtual Vector4 getValueAndGradient(const Vector3 &position) const;
        
        /** Overridden from Source.
        */
        virtual Real getValue(const Vector3 &position) const;

    };

}
}

#endif
