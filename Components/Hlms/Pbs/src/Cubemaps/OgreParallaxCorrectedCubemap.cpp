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

#include "Cubemaps/OgreParallaxCorrectedCubemap.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"

#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreRenderTexture.h"
#include "OgreHlmsManager.h"

#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreLwString.h"

namespace Ogre
{
    const char *cSuffixes[6] =
    {
        "PX", "NX",
        "PY", "NY",
        "PZ", "NZ",
    };

    ParallaxCorrectedCubemap::ParallaxCorrectedCubemap( IdType id, Root *root,
                                                        SceneManager *sceneManager,
                                                        const CompositorWorkspaceDef *probeWorkspcDef ) :
        IdObject( id ),
        mPaused( false ),
        mTrackedPosition( Vector3::ZERO ),
        mBlankProbe( this ),
        mBlendDummyCamera( 0 ),
        mBlendWorkspace( 0 ),
        mSamplerblockPoint( 0 ),
        mSamplerblockTrilinear( 0 ),
        mCurrentMip( 0 ),
        mRoot( root ),
        mSceneManager( sceneManager ),
        mDefaultWorkspaceDef( probeWorkspcDef )
    {
        memset( mBlendCubemapTUs, 0, sizeof(mBlendCubemapTUs) );
        createCubemapBlendWorkspaceDefinition();

        //Save the TextureUnitStates for setting the cubemap probes for blending every frame.
        char tmpBuffer[64];
        LwString materialName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        materialName = "Cubemap/BlendCubemap_";
        const size_t matNameSize = materialName.size();

        for( size_t i=0; i<6; ++i )
        {
            materialName.resize( matNameSize );
            materialName.a( cSuffixes[i] );
            MaterialPtr material = MaterialManager::getSingleton().load(
                        materialName.c_str(), ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                    staticCast<Material>();
            Pass *pass = material->getTechnique(0)->getPass(0);

            mBlendCubemapParams[i] = pass->getFragmentProgramParameters();
            for( size_t j=0; j<OGRE_MAX_CUBE_PROBES; ++j )
            {
                const size_t idx = (i * OGRE_MAX_CUBE_PROBES) + j;
                mBlendCubemapTUs[idx] = pass->getTextureUnitState( j );
            }
        }

        mBlankProbe.setTextureParams( 1, 1 );

        HlmsManager *hlmsManager = mRoot->getHlmsManager();
        HlmsSamplerblock samplerblock;
        samplerblock.mMipFilter = FO_LINEAR;
        mSamplerblockTrilinear = hlmsManager->getSamplerblock( samplerblock );
        samplerblock.mMipFilter = FO_NONE;
        mSamplerblockPoint = hlmsManager->getSamplerblock( samplerblock );
    }
    //-----------------------------------------------------------------------------------
    ParallaxCorrectedCubemap::~ParallaxCorrectedCubemap()
    {
        setEnabled( false );

        destroyAllProbes();

        if( !mBlendCubemap.isNull() )
        {
            TextureManager::getSingleton().remove( mBlendCubemap->getHandle() );
            mBlendCubemap.setNull();
        }

        HlmsManager *hlmsManager = mRoot->getHlmsManager();
        hlmsManager->destroySamplerblock( mSamplerblockTrilinear );
        mSamplerblockTrilinear = 0;
        hlmsManager->destroySamplerblock( mSamplerblockPoint );
        mSamplerblockPoint = 0;
    }
    //-----------------------------------------------------------------------------------
    CubemapProbe* ParallaxCorrectedCubemap::createProbe(void)
    {
        CubemapProbe *probe = OGRE_NEW CubemapProbe( this );
        mProbes.push_back( probe );
        return probe;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::destroyProbe( CubemapProbe *probe )
    {
        CubemapProbeVec::iterator itor = std::find( mProbes.begin(), mProbes.end(), probe );
        if( itor == mProbes.end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "Probe to delete does not belong to us, or was already freed",
                         "ParallaxCorrectedCubemap::destroyProbe" );
        }

        OGRE_DELETE *itor;
        efficientVectorRemove( mProbes, itor );
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::destroyAllProbes(void)
    {
        {
            TempRttVec::const_iterator itor = mTmpRtt.begin();
            TempRttVec::const_iterator end  = mTmpRtt.end();
            while( itor != end )
            {
                TextureManager::getSingleton().remove( itor->texture->getHandle() );
                ++itor;
            }
            mTmpRtt.clear();
        }

        {
            CubemapProbeVec::iterator itor = mProbes.begin();
            CubemapProbeVec::iterator end  = mProbes.end();

            while( itor != end )
            {
                OGRE_DELETE *itor;
                ++itor;
            }

            mProbes.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::setEnabled( bool bEnabled )
    {
        if( bEnabled == getEnabled() )
            return;

        if( bEnabled )
        {
            createCubemapBlendWorkspace();

            mRoot->addFrameListener( this );
            CompositorManager2 *compositorManager = mDefaultWorkspaceDef->getCompositorManager();
            compositorManager->addListener( this );
        }
        else
        {
            destroyCompositorData();

            CompositorManager2 *compositorManager = mDefaultWorkspaceDef->getCompositorManager();
            compositorManager->removeListener( this );

            mRoot->removeFrameListener( this );
        }
    }
    //-----------------------------------------------------------------------------------
    bool ParallaxCorrectedCubemap::getEnabled(void) const
    {
        return mBlendWorkspace != 0;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::createCubemapBlendWorkspaceDefinition(void)
    {
        String workspaceName = "AutoGen_ParallaxCorrectedCubemapBlending_Workspace";
        CompositorManager2 *compositorManager = mDefaultWorkspaceDef->getCompositorManager();
        CompositorWorkspaceDef *workspaceDef = 0;
                compositorManager->getWorkspaceDefinitionNoThrow( workspaceName );
        if( !workspaceDef )
        {
            char tmpBuffer[64];
            LwString materialName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
            materialName = "Cubemap/BlendCubemap_";
            const size_t matNameSize = materialName.size();

            CompositorNodeDef *nodeDef = compositorManager->addNodeDefinition(
                        "AutoGen_ParallaxCorrectedCubemapBlending_Node" );
            //Input texture
            nodeDef->addTextureSourceName( "BlendedProbeRT", 0, TextureDefinitionBase::TEXTURE_INPUT );
            nodeDef->setNumTargetPass( 6 );

            for( uint32 i=0; i<6; ++i )
            {
                CompositorTargetDef *targetDef = nodeDef->addTargetPass( "BlendedProbeRT", i );
                targetDef->setNumPasses( 2 );
                {
                    {
                        CompositorPassClearDef *passClear = static_cast<CompositorPassClearDef*>
                                                                ( targetDef->addPass( PASS_CLEAR ) );
                        passClear->mColourValue      = Ogre::ColourValue::Black;
                        passClear->mClearBufferFlags = FBT_COLOUR;
                        passClear->mDiscardOnly      = true;
                    }
                    {
                        CompositorPassQuadDef *passQuad = static_cast<CompositorPassQuadDef*>
                                                                ( targetDef->addPass( PASS_QUAD ) );
                        materialName.resize( matNameSize );
                        materialName.a( cSuffixes[i] );
                        passQuad->mMaterialName = materialName.c_str();
                    }
                }
            }

            CompositorWorkspaceDef *workDef = compositorManager->addWorkspaceDefinition( workspaceName );
            workDef->connectExternal( 0, nodeDef->getName(), 0 );
        }
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::createCubemapBlendWorkspace(void)
    {
        mBlendDummyCamera = mSceneManager->createCamera( "Dummy ParallaxCorrectedCubemap for blending " +
                                                         StringConverter::toString( getId() ),
                                                         false );
        CompositorChannel channel;
        channel.target = mBlendCubemap->getBuffer()->getRenderTarget();
        channel.textures.push_back( mBlendCubemap );
        CompositorChannelVec channels( 1, channel );

        const IdString workspaceName( "AutoGen_ParallaxCorrectedCubemapBlending_Workspace" );

        CompositorManager2 *compositorManager = mDefaultWorkspaceDef->getCompositorManager();
        mBlendWorkspace = compositorManager->addWorkspace( mSceneManager,
                                                           channels,
                                                           mBlendDummyCamera,
                                                           workspaceName,
                                                           false );
        mBlendWorkspace->setListener( this );
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::destroyCompositorData(void)
    {
        mBlendWorkspace->setListener( 0 );
        CompositorManager2 *compositorManager = mDefaultWorkspaceDef->getCompositorManager();
        compositorManager->removeWorkspace( mBlendWorkspace );
        mBlendWorkspace = 0;

        mSceneManager->destroyCamera( mBlendDummyCamera );
        mBlendDummyCamera = 0;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::calculateBlendFactors(void)
    {
        assert( mNumCollectedProbes < OGRE_MAX_CUBE_PROBES );

        //See Sebastien Lagarde "Local Image-based Lighting With Parallax-corrected Cubemap"
        //https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
        //https://seblagarde.wordpress.com/2012/11/28/siggraph-2012-talk/

        //There he explains:
        // Primitive have a normalized distance function which is 0 at center and 1 at boundary
        // When blending multiple primitive, we want the following constraint to be respect:
        // A - 100% (full weight) at center of primitive whatever the number of primitive overlapping
        // B - 0% (zero weight) at boundary of primitive whatever the number of primitive overlapping
        // For this we calc two weight and modulate them.
        // Weight0 is calc with NDF and allow to respect constraint B
        // Weight1 is calc with inverse NDF, which is (1 - NDF) and allow to respect constraint A
        // What enforce the constraint is the special case of 0 which once multiply by another value is 0.
        // For Weight 0, the 0 will enforce that boundary is always at 0%, but center will not always be 100%
        // For Weight 1, the 0 will enforce that center is always at 100%, but boundary will not always be 0%
        // Modulate weight0 and weight1 then renormalizing will allow to respects A and B at the same time.
        // The in between is not linear but give a pleasant result.
        // In practice the algorithm fail to avoid popping when leaving inner range of a primitive
        // which is include in at least 2 other primitives.
        // As this is a rare case, we do with it.

        //This will allow us to blend between the cubemaps. Notes:
        //  * What he calls "inverse NDF" we call "reverse NDF"
        //  * The math has been slightly changed but still has mathematical equivalent results.

        Real sumNdf = 0.0;

        for( int i=0; i<mNumCollectedProbes; ++i )
            sumNdf += mProbeNDFs[i];

        const Real invSumNdf = 1.0 / sumNdf;

        const Real reverseSumNdf = mNumCollectedProbes - sumNdf;
        const Real invRevSumNdf = 1.0 / reverseSumNdf;

        Real sumBlendFactor = 0;

        // "Weight0 = normalized NDF, inverted to have 1 at center, 0 at boundary.
        // And as we invert, we need to divide by Num-1 to stay normalized (else sum is > 1).
        // respect constraint B.
        // Weight1 = normalized inverted NDF, so we have 1 at center, 0 at boundary
        // and respect constraint A."
        for( int i=0; i<mNumCollectedProbes; ++i )
        {
            mProbeBlendFactors[i] = 1.0f - (mProbeNDFs[i] * invSumNdf);
            mProbeBlendFactors[i] *= (1.0f - mProbeNDFs[i]) * invRevSumNdf;
            sumBlendFactor += mProbeBlendFactors[i];
        }

        if( sumBlendFactor <= 0.0 )
            sumBlendFactor = 1.0f;

        Real invSumBlendFactor = 1.0 / sumBlendFactor;

        for( int i=0; i<mNumCollectedProbes; ++i )
            mProbeBlendFactors[i] *= invSumBlendFactor;
        for( int i=mNumCollectedProbes; i<OGRE_MAX_CUBE_PROBES; ++i )
            mProbeBlendFactors[i] = 0;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::updateSceneGraph(void)
    {
        mCurrentMip = 0;

        for( int i=0; i<OGRE_MAX_CUBE_PROBES; ++i )
        {
            mCollectedProbes[i] = 0;
            mProbeNDFs[i] = std::numeric_limits<Real>::max();
        }

        mNumCollectedProbes = 0;

        const Vector3 camPos = Vector3::ZERO;
        CubemapProbeVec::iterator itor = mProbes.begin();
        CubemapProbeVec::iterator end  = mProbes.end();

        while( itor != end )
        {
            CubemapProbe *probe = *itor;

            const Vector3 posLS = probe->mAabbOrientation * (camPos - probe->mArea.mCenter);
            const Aabb areaLS = probe->getAreaLS();
            if( areaLS.contains( posLS ) )
            {
                const Real ndf = probe->getNDF( posLS );

                if( ndf > 0 )
                {
                    //Collect this probe, ensuring we collect the ones with the lowest NDF.
                    uint32 probeIdx = mNumCollectedProbes;

                    if( mNumCollectedProbes >= OGRE_MAX_CUBE_PROBES )
                    {
                        Real highestNdf = -1;
                        int highestNdfIdx = OGRE_MAX_CUBE_PROBES;

                        //Drop the probe with the highest NDF (note: we may drop this probe)
                        for( int i=0; i<OGRE_MAX_CUBE_PROBES; ++i )
                        {
                            if( ndf < mProbeNDFs[i] && mProbeNDFs[i] >= highestNdf )
                            {
                                highestNdf = mProbeNDFs[i];
                                highestNdfIdx = i;
                            }
                        }

                        probeIdx = highestNdfIdx;
                    }

                    if( probeIdx < OGRE_MAX_CUBE_PROBES )
                    {
                        mProbeNDFs[probeIdx]        = ndf;
                        mCollectedProbes[probeIdx]  = probe;
                    }
                }
                else
                {
                    //Early out. Use ONLY this probe.
                    mProbeNDFs[0]       = ndf;
                    mCollectedProbes[0] = probe;
                    mNumCollectedProbes = 1;
                    itor = end;
                    break;
                }
            }

            ++itor;
        }

        for( size_t i=mNumCollectedProbes; i<OGRE_MAX_CUBE_PROBES; ++i )
            mCollectedProbes[i] = &mBlankProbe;

        calculateBlendFactors();

        //TODO: Update could be done over several frames.
        for( size_t i=0; i<mNumCollectedProbes; ++i )
            mCollectedProbes[i]->_prepareForRendering();

        bool requiresTrilinear = false;
        for( int i=0; i<mNumCollectedProbes; ++i )
        {
            if( mCollectedProbes[i]->mTexture->getNumMipmaps() != mBlendCubemap->getNumMipmaps() )
                requiresTrilinear = true;
        }

        //Cubemaps 1 to OGRE_MAX_CUBE_PROBES-1 are oriented relative to cubemap 0.
        float cubemaps[3*3*(OGRE_MAX_CUBE_PROBES-1)];
        Matrix3 invFirstCubemap = mCollectedProbes[0]->mAabbOrientation.Inverse();
        for( size_t i=1; i<OGRE_MAX_CUBE_PROBES; ++i )
        {
            Matrix3 cubemap;
            cubemap = invFirstCubemap * mCollectedProbes[i]->mAabbOrientation;
            for( size_t j=0; j<12; ++j )
                cubemaps[i * 12u + j] = cubemap[0][j];
        }

        //Setup the TUs for blending.
        for( size_t i=0; i<6; ++i )
        {
            mBlendCubemapParams[i]->setNamedConstant( "weights", &mProbeBlendFactors[0],
                                                      OGRE_MAX_CUBE_PROBES, 1 );
            mBlendCubemapParams[i]->setNamedConstant( "packed3x3Mat", cubemaps,
                                                      3*3*(OGRE_MAX_CUBE_PROBES-1), 1 );

            for( size_t j=0; j<OGRE_MAX_CUBE_PROBES; ++j )
            {
                const size_t idx = (i * OGRE_MAX_CUBE_PROBES) + j;
                mBlendCubemapTUs[idx]->setTexture( mCollectedProbes[j]->mTexture );
                mBlendCubemapTUs[idx]->_setSamplerblock( requiresTrilinear ? mSamplerblockTrilinear :
                                                                             mSamplerblockPoint );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::updateRender(void)
    {
        for( size_t i=0; i<mNumCollectedProbes; ++i )
        {
            if( mCollectedProbes[i]->mDirty || !mCollectedProbes[i]->mStatic )
                mCollectedProbes[i]->_updateRender();
        }

        mBlendWorkspace->_update();
    }
    //-----------------------------------------------------------------------------------
    TexturePtr ParallaxCorrectedCubemap::findTmpRtt( const TexturePtr &baseParams )
    {
        TexturePtr retVal;

        TempRttVec::iterator itor = mTmpRtt.begin();
        TempRttVec::iterator end  = mTmpRtt.end();

        while( itor != end )
        {
            if( itor->texture->getWidth() == baseParams->getWidth() &&
                itor->texture->getHeight() == baseParams->getHeight() &&
                itor->texture->getFormat() == baseParams->getFormat() &&
                itor->texture->getFSAA() == baseParams->getFSAA() )
            {
                retVal = itor->texture;
                ++itor->refCount;
            }

            ++itor;
        }

        if( retVal.isNull() )
        {
            TempRtt tmpRtt;
            retVal = TextureManager::getSingleton().createManual(
                        "ParallaxCorrectedCubemap Temp RTT " + StringConverter::toString( getId() ),
                        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        TEX_TYPE_CUBE_MAP, baseParams->getWidth(), baseParams->getHeight(),
                        baseParams->getNumMipmaps(), baseParams->getFormat(),
                        TU_RENDERTARGET|TU_AUTOMIPMAP, 0, true, baseParams->getFSAA() );
            tmpRtt.texture = retVal;
            tmpRtt.refCount = 1;
            mTmpRtt.push_back( tmpRtt );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::releaseTmpRtt( const TexturePtr &tmpRtt )
    {
        TempRttVec::iterator itor = mTmpRtt.begin();
        TempRttVec::iterator end  = mTmpRtt.end();

        while( itor != end && itor->texture != tmpRtt )
            ++itor;

        if( itor != end )
        {
            --itor->refCount;
            if( !itor->refCount )
            {
                TextureManager::getSingleton().remove( tmpRtt->getHandle() );
                efficientVectorRemove( mTmpRtt, itor );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    SceneManager* ParallaxCorrectedCubemap::getSceneManager(void) const
    {
        return mSceneManager;
    }
    //-----------------------------------------------------------------------------------
    const CompositorWorkspaceDef* ParallaxCorrectedCubemap::getDefaultWorkspaceDef(void) const
    {
        return mDefaultWorkspaceDef;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::passPreExecute( CompositorPass *pass )
    {
        float mipLevels[OGRE_MAX_CUBE_PROBES];
        for( size_t i=0; i<OGRE_MAX_CUBE_PROBES; ++i )
        {
            mipLevels[i] = (mCurrentMip * (mCollectedProbes[i]->mTexture->getNumMipmaps() + 1.0f)) /
                           (mBlendCubemap->getNumMipmaps() + 1.0f);
        }

        for( size_t i=0; i<6; ++i )
        {
            mBlendCubemapParams[i]->setNamedConstant( "lodLevel", &mipLevels[0],
                                                      OGRE_MAX_CUBE_PROBES, 1 );
        }

        ++mCurrentMip;
    }
    //-----------------------------------------------------------------------------------
    bool ParallaxCorrectedCubemap::frameStarted( const FrameEvent& evt )
    {
        if( !mPaused )
            this->updateSceneGraph();
        return true;
    }
    //-----------------------------------------------------------------------------------
    void ParallaxCorrectedCubemap::allWorkspacesBeginUpdate(void)
    {
        if( !mPaused )
            this->updateRender();
    }
}
