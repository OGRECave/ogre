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
    //Needed by the dynamic_cast assert
    #include "OgreSceneNode.h"
#endif

namespace Ogre
{
    inline void MovableObject::setDefaultVisibilityFlags(uint32 flags)
    {
        msDefaultVisibilityFlags = (flags & VisibilityFlags::RESERVED_VISIBILITY_FLAGS) |
                                    ( msDefaultVisibilityFlags &
                                      ~VisibilityFlags::RESERVED_VISIBILITY_FLAGS );
    }
    //-----------------------------------------------------------------------------------
    inline RealAsUint MovableObject::getCachedDistanceToCamera(void) const
    {
        return mObjectData.mDistanceToCamera[mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline Real MovableObject::getCachedDistanceToCameraAsReal(void) const
    {
        return (reinterpret_cast<Real*RESTRICT_ALIAS>(mObjectData.mDistanceToCamera))[mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setVisibilityFlags( uint32 flags )
    {
        mObjectData.mVisibilityFlags[mObjectData.mIndex] =
                    ( flags & VisibilityFlags::RESERVED_VISIBILITY_FLAGS ) |
                    ( mObjectData.mVisibilityFlags[mObjectData.mIndex] &
                        ~VisibilityFlags::RESERVED_VISIBILITY_FLAGS );
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::addVisibilityFlags( uint32 flags )
    {
        mObjectData.mVisibilityFlags[mObjectData.mIndex] |=
                                        flags & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::removeVisibilityFlags( uint32 flags )
    {
        mObjectData.mVisibilityFlags[mObjectData.mIndex] &=
                                        ~(flags & VisibilityFlags::RESERVED_VISIBILITY_FLAGS);
    }
    //-----------------------------------------------------------------------------------
    inline uint32 MovableObject::getVisibilityFlags(void) const
    {
        return mObjectData.mVisibilityFlags[mObjectData.mIndex] &
                                                    VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setQueryFlags( uint32 flags )
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] = flags;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::addQueryFlags( uint32 flags )
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] |= flags;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::removeQueryFlags( uint32 flags )
    {
        mObjectData.mQueryFlags[mObjectData.mIndex] &= ~flags;
    }
    //-----------------------------------------------------------------------------------
    inline uint32 MovableObject::getQueryFlags(void) const
    {
        return mObjectData.mQueryFlags[mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline uint32 MovableObject::getLightMask() const
    {
        return mObjectData.mLightMask[mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setLightMask( uint32 lightMask )
    {
        mObjectData.mLightMask[mObjectData.mIndex] = lightMask;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setRenderingDistance( Real dist )
    {
        assert( dist > 0.0f );
        if (dist > 0.0f)
        {
            mObjectData.mUpperDistance[0][mObjectData.mIndex] = dist;
            mObjectData.mUpperDistance[1][mObjectData.mIndex] = min(dist, mObjectData.mUpperDistance[1][mObjectData.mIndex]);
        }
    }
    //-----------------------------------------------------------------------------------
    inline Real MovableObject::getRenderingDistance(void) const
    {
        return mObjectData.mUpperDistance[0][mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setShadowRenderingDistance(Real dist)
    {
        assert(dist > 0.0f);
        if (dist > 0.0f)
        {
            mObjectData.mUpperDistance[1][mObjectData.mIndex] = min(dist, mObjectData.mUpperDistance[0][mObjectData.mIndex]);
        }
    }
    //-----------------------------------------------------------------------------------
    inline Real MovableObject::getShadowRenderingDistance(void) const
    {
        return mObjectData.mUpperDistance[1][mObjectData.mIndex];
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setVisible( bool visible )
    {
        assert( (!visible || mParentNode) && "Setting to visible an object without "
                "attachment is not supported!" );

        if( visible )
            mObjectData.mVisibilityFlags[mObjectData.mIndex] |= VisibilityFlags::LAYER_VISIBILITY;
        else
            mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~VisibilityFlags::LAYER_VISIBILITY;
    }
    //-----------------------------------------------------------------------------------
    inline bool MovableObject::getVisible(void) const
    {
        return (mObjectData.mVisibilityFlags[mObjectData.mIndex] &
                                                    VisibilityFlags::LAYER_VISIBILITY) != 0;
    }
    //-----------------------------------------------------------------------------------
    inline void MovableObject::setCastShadows( bool enabled )
    {
        if( enabled )
            mObjectData.mVisibilityFlags[mObjectData.mIndex] |= VisibilityFlags::LAYER_SHADOW_CASTER;
        else
            mObjectData.mVisibilityFlags[mObjectData.mIndex] &= ~VisibilityFlags::LAYER_SHADOW_CASTER;
    }
    //-----------------------------------------------------------------------------------
    inline bool MovableObject::getCastShadows(void) const
    {
        return (mObjectData.mVisibilityFlags[mObjectData.mIndex] &
                                                    VisibilityFlags::LAYER_SHADOW_CASTER) != 0;
    }
    //-----------------------------------------------------------------------------------
    inline uint8 MovableObject::getRenderQueueGroup(void) const
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------------------
    inline SceneNode* MovableObject::getParentSceneNode(void) const
    {
        assert( !mParentNode || dynamic_cast<SceneNode*>( mParentNode ) );
        return static_cast<SceneNode*>( mParentNode );
    }
}
