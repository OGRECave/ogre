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
#include "Cubemaps/OgreParallaxCorrectedCubemap.h"

#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "OgreLogManager.h"
#include "OgreLwString.h"
#include "OgreId.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "OgreSceneManager.h"

//Disable as OpenGL version of copyToTexture is super slow (makes a GPU->CPU->GPU roundtrip)
#define USE_RTT_DIRECTLY 0

namespace Ogre
{
    CubemapProbe::CubemapProbe( ParallaxCorrectedCubemap *creator ) :
        mArea( Aabb::BOX_NULL ),
        mAreaInnerRegion( Vector3::ZERO ),
        mOrientation( Matrix3::IDENTITY ),
        mInvOrientation( Matrix3::IDENTITY ),
        mProbeShape( Aabb::BOX_NULL ),
        mFsaa( 1 ),
        mWorkspace( 0 ),
        mCamera( 0 ),
        mCreator( creator ),
        mStatic( true ),
        mEnabled( true ),
        mDirty( true ),
        mNumIterations( 32 ),
        mMask( 0xffffffff )
    {
    }
    //-----------------------------------------------------------------------------------
    CubemapProbe::~CubemapProbe()
    {
        destroyWorkspace();
        destroyTexture();

        if( mCamera )
        {
            SceneManager *sceneManager = mCamera->getSceneManager();
            sceneManager->destroyCamera( mCamera );
            mCamera = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::destroyWorkspace(void)
    {
        if( mWorkspace )
        {
#if USE_RTT_DIRECTLY
            if( mStatic )
            {
                const CompositorChannel &channel = mWorkspace->getExternalRenderTargets()[0];
                mCreator->releaseTmpRtt( channel.textures[0] );
            }
#endif

            CompositorManager2 *compositorManager = mWorkspace->getCompositorManager();
            compositorManager->removeWorkspace( mWorkspace );
            mWorkspace = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::destroyTexture(void)
    {
        assert( !mWorkspace );
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
        float cameraNear = 0.5;
        float cameraFar = 1000;

        if( mCamera )
        {
            cameraNear = mCamera->getNearClipDistance();
            cameraFar = mCamera->getFarClipDistance();
        }

        const bool reinitWorkspace = mWorkspace != 0;
        destroyWorkspace();
        destroyTexture();

        char tmpBuffer[64];
        LwString texName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        texName.a( "CubemapProbe_", Id::generateNewId<CubemapProbe>() );

#if USE_RTT_DIRECTLY
        const uint32 flags = isStatic ? TU_STATIC_WRITE_ONLY : (TU_RENDERTARGET|TU_AUTOMIPMAP);
#else
    #if GENERATE_MIPMAPS_ON_BLEND
        const uint32 flags = TU_RENDERTARGET;
    #else
        const uint32 flags = TU_RENDERTARGET|TU_AUTOMIPMAP;
    #endif
#endif
        mFsaa = fsaa;
        fsaa = isStatic ? 0 : fsaa;
        mTexture = TextureManager::getSingleton().createManual(
                    texName.c_str(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_CUBE_MAP, width, height,
            #if GENERATE_MIPMAPS_ON_BLEND
                    0,
            #else
                    PixelUtil::getMaxMipmapCount( width, height, 1 ),
            #endif
                    pf, flags, 0, true, fsaa );
        mStatic = isStatic;
        mDirty = true;

        if( reinitWorkspace )
            initWorkspace( cameraNear, cameraFar, mWorkspaceDefName );
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::initWorkspace( float cameraNear, float cameraFar, IdString workspaceDefOverride )
    {
        assert( !mTexture.isNull() && "Call setTextureParams first!" );

        destroyWorkspace();

        CompositorWorkspaceDef const *workspaceDef = mCreator->getDefaultWorkspaceDef();
        CompositorManager2 *compositorManager = workspaceDef->getCompositorManager();

        if( workspaceDefOverride != IdString() )
            workspaceDef = compositorManager->getWorkspaceDefinition( workspaceDefOverride );

        mWorkspaceDefName = workspaceDef->getName();
        SceneManager *sceneManager = mCreator->getSceneManager();
        mCamera = sceneManager->createCamera( mTexture->getName(), true, true );
        mCamera->setFOVy( Degree(90) );
        mCamera->setAspectRatio( 1 );
        mCamera->setFixedYawAxis(false);
        mCamera->setNearClipDistance( cameraNear );
        mCamera->setFarClipDistance( cameraFar );

        TexturePtr rtt = mTexture;
        if( mStatic )
        {
#if USE_RTT_DIRECTLY
            //Grab tmp texture
            rtt = mCreator->findTmpRtt( mTexture );
#endif
            //Set camera to skip light culling (efficiency)
            mCamera->setLightCullingVisibility( false, false );
        }
        else
        {
            mCamera->setLightCullingVisibility( true, true );
        }

        CompositorChannel channel;
        channel.target = rtt->getBuffer()->getRenderTarget();
        channel.textures.push_back( rtt );
        CompositorChannelVec channels( 1, channel );

        mWorkspace = compositorManager->addWorkspace( sceneManager, channels, mCamera,
                                                      mWorkspaceDefName, false );
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::set( const Aabb &area, const Vector3 &areaInnerRegion,
                            const Matrix3 &orientation, const Aabb &probeShape )
    {
        mArea               = area;
        mAreaInnerRegion    = areaInnerRegion;
        mOrientation        = orientation;
        mInvOrientation     = mOrientation.Inverse();
        mProbeShape         = probeShape;

        mAreaInnerRegion.makeCeil( Vector3::ZERO );
        mAreaInnerRegion.makeFloor( Vector3::UNIT_SCALE );

        Aabb areaLocalToShape = mArea;
        areaLocalToShape.mCenter -= mProbeShape.mCenter;
        areaLocalToShape.mCenter = mInvOrientation * areaLocalToShape.mCenter;
        areaLocalToShape.mCenter += mProbeShape.mCenter;

        mDirty = true;
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::setStatic( bool isStatic )
    {
        if( mStatic != isStatic && !mTexture.isNull() )
        {
            setTextureParams( mTexture->getWidth(), mTexture->getHeight(),
                              mTexture->getFormat(), isStatic, mTexture->getFSAA() );
        }
        else
        {
            //We're not initialized yet, but still save the intention...
            mStatic = isStatic;
        }
    }
    //-----------------------------------------------------------------------------------
    Real CubemapProbe::getNDF( const Vector3 &posLS ) const
    {
        //Work in the upper left corner of the box. (Like Aabb::distance)
        Vector3 dist;
        dist.x = Math::Abs( posLS.x );
        dist.y = Math::Abs( posLS.y );
        dist.z = Math::Abs( posLS.z );

        const Vector3 innerRange = mArea.mHalfSize * mAreaInnerRegion;
        const Vector3 outerRange = mArea.mHalfSize;

        //1e-6f avoids division by zero.
        Vector3 ndf = (dist - innerRange) / (outerRange - innerRange + Real(1e-6f));

        return Ogre::max( Ogre::max( ndf.x, ndf.y ), ndf.z );
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::_prepareForRendering(void)
    {
        mCamera->setPosition( mArea.mCenter );
        mCamera->setOrientation( Quaternion( mOrientation ) );
        if( mStatic )
            mCamera->setLightCullingVisibility( true, true );
    }
    //-----------------------------------------------------------------------------------
    void CubemapProbe::_updateRender(void)
    {
        assert( mDirty || !mStatic );
        mWorkspace->_update();

        if( mStatic )
        {
#if USE_RTT_DIRECTLY
            //Copy from tmp RTT to real texture.
            const CompositorChannel &channel = mWorkspace->getExternalRenderTargets()[0];
            channel.textures[0]->copyToTexture( mTexture );
#endif

            mCamera->setLightCullingVisibility( false, false );
        }
    }
}
