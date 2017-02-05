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

#if OGRE_DEBUG_MODE
    #define CACHED_TRANSFORM_OUT_OF_DATE() this->setCachedTransformOutOfDate()
#else
    #define CACHED_TRANSFORM_OUT_OF_DATE() ((void)0)
#endif

namespace Ogre
{
    //-----------------------------------------------------------------------
    inline Quaternion Bone::getOrientation() const
    {
        return mTransform.mOrientation->getAsQuaternion( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    inline void Bone::setOrientation( Quaternion q )
    {
        assert(!q.isNaN() && "Invalid orientation supplied as parameter");
        q.normalise();
        mTransform.mOrientation->setFromQuaternion( q, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    inline void Bone::setPosition(const Vector3& pos)
    {
        assert(!pos.isNaN() && "Invalid vector supplied as parameter");
        mTransform.mPosition->setFromVector3( pos, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    inline Vector3 Bone::getPosition(void) const
    {
        return mTransform.mPosition->getAsVector3( mTransform.mIndex );
    }
    //-----------------------------------------------------------------------
    inline void Bone::setScale(const Vector3& scale)
    {
        assert(!scale.isNaN() && "Invalid vector supplied as parameter");
        mTransform.mScale->setFromVector3( scale, mTransform.mIndex );
        CACHED_TRANSFORM_OUT_OF_DATE();
    }
    //-----------------------------------------------------------------------
    inline Vector3 Bone::getScale(void) const
    {
        return mTransform.mScale->getAsVector3( mTransform.mIndex );
    }
}

#undef CACHED_TRANSFORM_OUT_OF_DATE
