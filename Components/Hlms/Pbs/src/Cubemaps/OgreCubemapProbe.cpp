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

#include "OgreStableHeaders.h"

#include "Cubemaps/OgreCubemapProbe.h"

#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"
#include "OgreLwString.h"
#include "OgreId.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

namespace Ogre
{
    CubemapProbe::CubemapProbe() :
        mProbePos( Vector3::ZERO ),
        mArea( Aabb::BOX_NULL ),
        mAabbOrientation( Matrix3::IDENTITY ),
        mAabbFalloff( Vector3::ZERO ),
        mFsaa( 1 ),
        mWorkspace( 0 ),
        mDirty( false ),
        mStatic( true )
    {
    }
    //-----------------------------------------------------------------------------------
    CubemapProbe::~CubemapProbe()
    {
        if( mWorkspace )
        {
            CompositorManager2 *compositorManager = mWorkspace->getCompositorManager();
            compositorManager->removeWorkspace( mWorkspace );
            mWorkspace = 0;
        }

        if( !mTexture.isNull() )
        {
            TextureManager::getSingleton().remove( mTexture->getHandle() );
            mTexture.setNull();
        }
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::setTextureParams( uint32 width, uint32 height, PixelFormat pf,
                                         bool isStatic, uint8 fsaa )
    {
        char tmpBuffer[64];
        LwString texName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        texName.a( "CubemapProbe_", Id::generateNewId<CubemapProbe>() );

        const uint32 flags = isStatic ? TU_STATIC_WRITE_ONLY : (TU_RENDERTARGET|TU_AUTOMIPMAP);
        mFsaa = fsaa;
        fsaa = isStatic ? 1 : fsaa;
        mTexture = TextureManager::getSingleton().createManual(
                    texName.c_str(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_CUBE_MAP, width, height,
                    PixelUtil::getMaxMipmapCount( width, height, 1 ),
                    pf, flags, 0, true, fsaa );
        mDirty = true;
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::set( const Vector3 &probePos, const Aabb &area,
                            const Matrix3 &aabbOrientation, const Vector3 &aabbFalloff )
    {
        mProbePos           = probePos;
        mArea               = area;
        mAabbOrientation    = aabbOrientation;
        mAabbFalloff        = aabbFalloff;

        mAabbFalloff.makeCeil( Vector3::ZERO );
        mAabbFalloff.makeFloor( Vector3::UNIT_SCALE );

        const Vector3 localPos = mAabbOrientation * (mProbePos - area.mCenter);
        if( !getAreaLS().contains( localPos ) )
        {
            LogManager::getSingleton().logMessage(
                        "ERROR: Cubemap's probePos must be inside the probe's bounds. "
                        "Forcing the probe position to be the same as the center of its bounds." );
            mProbePos = mArea.mCenter;
        }

        mDirty = true;
    }
    //-----------------------------------------------------------------------------------
    Real CubemapProbe::getNDF( const Vector3 &posLS ) const
    {
        //Work in the upper left corner of the box. (Like Aabb::distance)
        Vector3 dist;
        dist.x = Math::Abs( posLS.x );
        dist.y = Math::Abs( posLS.y );
        dist.z = Math::Abs( posLS.z );

        const Vector3 innerRange = mArea.mHalfSize * mAabbFalloff;
        const Vector3 outerRange = mArea.mHalfSize;

        //1e-6f avoids division by zero.
        Vector3 ndf = (dist - innerRange) / (outerRange - innerRange + Real(1e-6f));

        return Ogre::max( Ogre::max( ndf.x, ndf.y ), ndf.z );
    }
}
