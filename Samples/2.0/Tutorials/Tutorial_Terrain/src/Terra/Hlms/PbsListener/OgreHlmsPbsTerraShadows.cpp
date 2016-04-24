/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/Terra.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbTexture.h"

#include "OgreHlms.h"
#include "OgreHlmsManager.h"
#include "OgreRoot.h"

namespace Ogre
{
    const IdString PbsTerraProperty::TerraEnabled   = IdString( "terra_enabled" );

    HlmsPbsTerraShadows::HlmsPbsTerraShadows() :
          mTerra( 0 )
        , mTerraSamplerblock( 0 )
#if OGRE_DEBUG_MODE
        , mSceneManager( 0 )
#endif
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsTerraShadows::~HlmsPbsTerraShadows()
    {
        if( mTerraSamplerblock )
        {
            HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
            hlmsManager->destroySamplerblock( mTerraSamplerblock );
            mTerraSamplerblock = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsTerraShadows::setTerra( Terra *terra )
    {
        mTerra = terra;
        if( !mTerraSamplerblock )
        {
            HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
            mTerraSamplerblock = hlmsManager->getSamplerblock( HlmsSamplerblock() );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsTerraShadows::shaderCacheEntryCreated( const String &shaderProfile,
                                                       const HlmsCache *hlmsCacheEntry,
                                                       const HlmsCache &passCache,
                                                       const HlmsPropertyVec &properties,
                                                       const QueuedRenderable &queuedRenderable )
    {
        if( shaderProfile != "hlsl" &&
            Hlms::getProperty( passCache.setProperties, HlmsBaseProp::ShadowCaster ) == 0 )
        {
            if( !hlmsCacheEntry->pso.vertexShader.isNull() )
            {
                GpuProgramParametersSharedPtr vsParams = hlmsCacheEntry->pso.vertexShader->
                        getDefaultParameters();

                //The slot 12 is arbitrary. Should be high enough enough to not mess
                //with most materials (it could conflict if a material uses *A LOT* of
                //textures and they're in different arrays).
                //Note OpenGL has very low limits (usually 15-16)
                vsParams->setNamedConstant( "terrainShadows", 12 );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsTerraShadows::preparePassHash( const CompositorShadowNode *shadowNode,
                                               bool casterPass, bool dualParaboloid,
                                               SceneManager *sceneManager, Hlms *hlms )
    {
        if( !casterPass )
        {
#if OGRE_DEBUG_MODE
            mSceneManager = sceneManager;
#endif

            if( mTerra && hlms->_getProperty( HlmsBaseProp::LightsDirNonCaster ) > 0 )
            {
                //First directional light always cast shadows thanks to our terrain shadows.
                int32 shadowCasterDirectional = hlms->_getProperty( HlmsBaseProp::LightsDirectional );
                shadowCasterDirectional = std::max( shadowCasterDirectional, 1 );
                hlms->_setProperty( HlmsBaseProp::LightsDirectional, shadowCasterDirectional );
            }

            hlms->_setProperty( PbsTerraProperty::TerraEnabled, mTerra != 0 );
        }
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsPbsTerraShadows::getPassBufferSize( const CompositorShadowNode *shadowNode,
                                                   bool casterPass, bool dualParaboloid,
                                                   SceneManager *sceneManager ) const
    {
        return (!casterPass && mTerra) ? 32u : 0u;
    }
    //-----------------------------------------------------------------------------------
    float* HlmsPbsTerraShadows::preparePassBuffer( const CompositorShadowNode *shadowNode,
                                                   bool casterPass, bool dualParaboloid,
                                                   SceneManager *sceneManager,
                                                   float *passBufferPtr )
    {
        if( !casterPass && mTerra )
        {
            const float invHeight = 1.0f / mTerra->getHeight();
            const Vector3 &terrainOrigin = mTerra->getTerrainOrigin();
            const Vector2 &terrainXZInvDim = mTerra->getXZInvDimensions();
            *passBufferPtr++ = -terrainOrigin.x * terrainXZInvDim.x;
            *passBufferPtr++ = -terrainOrigin.y * invHeight;
            *passBufferPtr++ = -terrainOrigin.z * terrainXZInvDim.y;
            *passBufferPtr++ = 1.0f;

            *passBufferPtr++ = terrainXZInvDim.x;
            *passBufferPtr++ = invHeight;
            *passBufferPtr++ = terrainXZInvDim.y;
            *passBufferPtr++ = 1.0f;
        }

        return passBufferPtr;
    }
    //-----------------------------------------------------------------------------------
    void HlmsPbsTerraShadows::hlmsTypeChanged( bool casterPass, CommandBuffer *commandBuffer,
                                               const HlmsDatablock *datablock )
    {
        if( !casterPass && mTerra )
        {
            Ogre::TexturePtr terraShadowTex = mTerra->_getShadowMapTex();

            //Bind the shadows' texture. Tex. slot must match with
            //the one in HlmsPbsTerraShadows::shaderCacheEntryCreated
            *commandBuffer->addCommand<CbTexture>() = CbTexture( 12u, true, terraShadowTex.get(),
                                                                 mTerraSamplerblock );

#if OGRE_DEBUG_MODE
            const CompositorTextureVec &compositorTextures = mSceneManager->getCompositorTextures();
            CompositorTextureVec::const_iterator itor = compositorTextures.begin();
            CompositorTextureVec::const_iterator end  = compositorTextures.end();

            while( itor != end && (*itor->textures)[0] != terraShadowTex )
                ++itor;

            if( itor == end )
            {
                assert( "Hazard Detected! You should expose this Terra's shadow map texture"
                        " to the compositor pass so Ogre can place the proper Barriers" && false );
            }
#endif
        }
    }
}
