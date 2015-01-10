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

#include "Compositor/OgreCompositorShadowNode.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTexture.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgrePass.h"

#include "OgreShadowCameraSetupFocused.h"
#include "OgreShadowCameraSetupPSSM.h"

namespace Ogre
{
    const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,    0,  0.5,
        0,   -0.5,    0,  0.5,
        0,      0,    1,    0,
        0,      0,    0,    1);

    CompositorShadowNode::CompositorShadowNode( IdType id, const CompositorShadowNodeDef *definition,
                                                CompositorWorkspace *workspace, RenderSystem *renderSys,
                                                const RenderTarget *finalTarget ) :
            CompositorNode( id, definition->getName(), definition, workspace, renderSys, finalTarget ),
            mDefinition( definition ),
            mLastCamera( 0 ),
            mLastFrame( -1 )
    {
        mShadowMapCameras.reserve( definition->mShadowMapTexDefinitions.size() );
        mLocalTextures.reserve( mLocalTextures.size() + definition->mShadowMapTexDefinitions.size() );

        //Normal textures must be defined last but were already created.
        const size_t numNormalTextures = mLocalTextures.size();

        //Create the local textures
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                                                            definition->mShadowMapTexDefinitions.begin();
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
                                                            definition->mShadowMapTexDefinitions.end();

        while( itor != end )
        {
            // We could still end up pushing a null RT & Texture
            // to preserve the index order from getTextureSource.
            mLocalTextures.push_back( createShadowTexture( *itor, finalTarget ) );

            // One map, one camera
            const size_t shadowMapIdx = itor - definition->mShadowMapTexDefinitions.begin();
            SceneManager *sceneManager = workspace->getSceneManager();
            ShadowMapCamera shadowMapCamera;
            shadowMapCamera.camera = sceneManager->createCamera( "ShadowNode Camera ID " +
                                                StringConverter::toString( id ) + " Map " +
                                                StringConverter::toString( shadowMapIdx ), false );
            shadowMapCamera.camera->setFixedYawAxis( false );
            shadowMapCamera.minDistance = 0.0f;
            shadowMapCamera.maxDistance = 100000.0f;


            const size_t sharingSetupIdx = itor->getSharesSetupWith();
            if( sharingSetupIdx != std::numeric_limits<size_t>::max() )
            {
                shadowMapCamera.shadowCameraSetup = mShadowMapCameras[sharingSetupIdx].shadowCameraSetup;
            }
            else
            {
                switch( itor->shadowMapTechnique )
                {
                case SHADOWMAP_UNIFORM:
                    shadowMapCamera.shadowCameraSetup =
                                    ShadowCameraSetupPtr( OGRE_NEW DefaultShadowCameraSetup() );
                    break;
                /*case SHADOWMAP_PLANEOPTIMAL:
                    break;*/
                case SHADOWMAP_FOCUSED:
                    {
                        FocusedShadowCameraSetup *setup = OGRE_NEW FocusedShadowCameraSetup();
                        shadowMapCamera.shadowCameraSetup = ShadowCameraSetupPtr( setup );
                    }
                    break;
                case SHADOWMAP_PSSM:
                    {
                        PSSMShadowCameraSetup *setup = OGRE_NEW PSSMShadowCameraSetup();
                        shadowMapCamera.shadowCameraSetup = ShadowCameraSetupPtr( setup );
                        setup->calculateSplitPoints( itor->numSplits, 0.0f, 100.0f, 0.95f );
                        setup->setSplitPadding( itor->splitPadding );
                    }
                    break;
                default:
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                "Shadow Map technique not implemented or not recognized.",
                                "CompositorShadowNode::CompositorShadowNode");
                    break;
                }
            }

            mShadowMapCameras.push_back( shadowMapCamera );

            ++itor;
        }

        //Put normal textures in the back; we couldn't split the list earlier (less moves)
        //because an Exception in the 'for' loop above would cause memory leaks
        CompositorChannelVec normalTextures( mLocalTextures.begin(),
                                             mLocalTextures.begin() + numNormalTextures );
        mLocalTextures.erase( mLocalTextures.begin(), mLocalTextures.begin() + numNormalTextures );
        mLocalTextures.insert( mLocalTextures.end(), normalTextures.begin(), normalTextures.end() );

        // Shadow Nodes don't have input; and global textures should be ready by
        // the time we get created. Therefore, we can safely initialize now as our
        // output may be used in regular nodes and we're created on-demand (as soon
        // as a Node discovers it needs us for the first time, we get created)
        createPasses();
    }
    //-----------------------------------------------------------------------------------
    CompositorShadowNode::~CompositorShadowNode()
    {
    }
    //-----------------------------------------------------------------------------------
    CompositorChannel CompositorShadowNode::createShadowTexture(
                                                            const ShadowTextureDefinition &textureDef,
                                                            const RenderTarget *finalTarget )
    {
        CompositorChannel newChannel;

        //When format list is empty, then this definition is for a shadow map atlas.
        if( !textureDef.formatList.empty() )
        {
            uint width  = textureDef.width;
            uint height = textureDef.height;
            if( finalTarget )
            {
                if( textureDef.width == 0 )
                {
                    width = static_cast<uint>( ceilf( finalTarget->getWidth() *
                                                        textureDef.widthFactor ) );
                }
                if( textureDef.height == 0 )
                {
                    height = static_cast<uint>( ceilf( finalTarget->getHeight() *
                                                        textureDef.heightFactor ) );
                }
            }

            String textureName = (textureDef.getName() + IdString( getId() )).getFriendlyText();
            if( textureDef.formatList.size() == 1 )
            {
                //Normal RT
                TexturePtr tex = TextureManager::getSingleton().createManual( textureName,
                                                ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
                                                TEX_TYPE_2D, width, height, 0,
                                                textureDef.formatList[0], TU_RENDERTARGET, 0,
                                                textureDef.hwGammaWrite, textureDef.fsaa );
                RenderTexture* rt = tex->getBuffer()->getRenderTarget();
                rt->setDepthBufferPool( textureDef.depthBufferId );
                newChannel.target = rt;
                newChannel.textures.push_back( tex );
            }
            else
            {
                //MRT
                MultiRenderTarget* mrt = mRenderSystem->createMultiRenderTarget( textureName );
                PixelFormatList::const_iterator pixIt = textureDef.formatList.begin();
                PixelFormatList::const_iterator pixEn = textureDef.formatList.end();

                mrt->setDepthBufferPool( textureDef.depthBufferId );
                newChannel.target = mrt;

                while( pixIt != pixEn )
                {
                    size_t rtNum = pixIt - textureDef.formatList.begin();
                    TexturePtr tex = TextureManager::getSingleton().createManual(
                                                textureName + StringConverter::toString( rtNum ),
                                                ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
                                                TEX_TYPE_2D, width, height, 0,
                                                *pixIt, TU_RENDERTARGET, 0, textureDef.hwGammaWrite,
                                                textureDef.fsaa );
                    RenderTexture* rt = tex->getBuffer()->getRenderTarget();
                    mrt->bindSurface( rtNum, rt );
                    newChannel.textures.push_back( tex );
                    ++pixIt;
                }
            }
        }

        return newChannel;
    }
    //-----------------------------------------------------------------------------------
    //An Input Iterator that is the same as doing vector<int> val( N ); and goes in increasing
    //order (i.e. val[0] = 0; val[1] = 1; val[n-1] = n-1) but doesn't occupy N elements in memory,
    //just one.
    struct MemoryLessInputIterator : public std::iterator<std::input_iterator_tag, size_t>
    {
        size_t index;
        MemoryLessInputIterator( size_t startValue ) : index( startValue ) {}

        MemoryLessInputIterator& operator ++() //Prefix increment
        {
            ++index;
            return *this;
        }

        MemoryLessInputIterator operator ++(int) //Postfix increment
        {
            MemoryLessInputIterator copy = *this;
            ++index;
            return copy;
        }

        size_t operator *() const   { return index; }

        bool operator == (const MemoryLessInputIterator &r) const    { return index == r.index; }
        bool operator != (const MemoryLessInputIterator &r) const    { return index != r.index; }
    };

    class ShadowMappingLightCmp
    {
        LightListInfo const *mLightList;
        uint32              mCombinedVisibilityFlags;
        Vector3             mCameraPos;

    public:
        ShadowMappingLightCmp( LightListInfo const *lightList, uint32 combinedVisibilityFlags,
                               const Vector3 &cameraPos ) :
            mLightList( lightList ), mCombinedVisibilityFlags( combinedVisibilityFlags ), mCameraPos( cameraPos )
        {
        }

        bool operator()( size_t _l, size_t _r ) const
        {
            uint32 visibilityMaskL = mLightList->visibilityMask[_l];
            uint32 visibilityMaskR = mLightList->visibilityMask[_r];

            if( (visibilityMaskL & mCombinedVisibilityFlags) &&
                !(visibilityMaskR & mCombinedVisibilityFlags) )
            {
                return true;
            }
            else if( !(visibilityMaskL & mCombinedVisibilityFlags) &&
                     (visibilityMaskR & mCombinedVisibilityFlags) )
            {
                return false;
            }
            else if( (visibilityMaskL & VisibilityFlags::LAYER_SHADOW_CASTER) &&
                    !(visibilityMaskR & VisibilityFlags::LAYER_SHADOW_CASTER) )
            {
                return true;
            }
            else if( !(visibilityMaskL & VisibilityFlags::LAYER_SHADOW_CASTER) &&
                     (visibilityMaskR & VisibilityFlags::LAYER_SHADOW_CASTER) )
            {
                return false;
            }

            Real fDistL = mCameraPos.distance( mLightList->boundingSphere[_l].getCenter() ) -
                          mLightList->boundingSphere[_l].getRadius();
            Real fDistR = mCameraPos.distance( mLightList->boundingSphere[_r].getCenter() ) -
                          mLightList->boundingSphere[_r].getRadius();
            return fDistL < fDistR;
        }
    };
    void CompositorShadowNode::buildClosestLightList( Camera *newCamera, const Camera *lodCamera )
    {
        const size_t currentFrameCount = mWorkspace->getFrameCount();
        if( mLastCamera == newCamera && mLastFrame == currentFrameCount )
        {
            return;
        }

        mLastFrame = currentFrameCount;

        mLastCamera = newCamera;

        const Viewport *viewport = newCamera->getLastViewport();
        const SceneManager *sceneManager = newCamera->getSceneManager();
        const LightListInfo &globalLightList = sceneManager->getGlobalLightList();

        uint32 combinedVisibilityFlags = viewport->getVisibilityMask() &
                                            sceneManager->getVisibilityMask();

        const size_t numLights = std::min( mDefinition->mNumLights, globalLightList.lights.size() );
        mShadowMapCastingLights.clear();
        mShadowMapCastingLights.reserve( numLights );
        mAffectedLights.clear();
        mAffectedLights.resize( globalLightList.lights.size(), false );

        const Vector3 &camPos( newCamera->getDerivedPosition() );

        //mShadowMapCastingLights.resize( numLights, 0 );
        vector<size_t>::type sortedIndexes;
        sortedIndexes.resize( numLights, ~0 );
        std::partial_sort_copy( MemoryLessInputIterator( 0 ),
                            MemoryLessInputIterator( globalLightList.lights.size() ),
                            sortedIndexes.begin(), sortedIndexes.end(),
                            ShadowMappingLightCmp( &globalLightList, combinedVisibilityFlags, camPos ) );

        vector<size_t>::type::const_iterator itor = sortedIndexes.begin();
        vector<size_t>::type::const_iterator end  = sortedIndexes.end();

        while( itor != end )
        {
            uint32 visibilityMask = globalLightList.visibilityMask[*itor];
            if( !(visibilityMask & combinedVisibilityFlags) ||
                !(visibilityMask & VisibilityFlags::LAYER_SHADOW_CASTER) )
            {
                break;
            }

            mAffectedLights[*itor] = true;
            mShadowMapCastingLights.push_back( LightClosest( globalLightList.lights[*itor], *itor, 0 ) );
            ++itor;
        }

        mCastersBox = sceneManager->_calculateCurrentCastersBox( viewport->getVisibilityMask(),
                                                                 mDefinition->mMinRq,
                                                                 mDefinition->mMaxRq );
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::_update( Camera* camera, const Camera *lodCamera,
                                        SceneManager *sceneManager )
    {
        ShadowMapCameraVec::iterator itShadowCamera = mShadowMapCameras.begin();
        const Viewport *viewport    = camera->getLastViewport();

        buildClosestLightList( camera, lodCamera );

        //Setup all the cameras
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                                                            mDefinition->mShadowMapTexDefinitions.begin();
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
                                                            mDefinition->mShadowMapTexDefinitions.end();

        while( itor != end )
        {
            if( itor->light < mShadowMapCastingLights.size() )
            {
                Light const *light = mShadowMapCastingLights[itor->light].light;

                Camera *texCamera = itShadowCamera->camera;

                //Use the material scheme of the main viewport
                //This is required to pick up the correct shadow_caster_material and similar properties.
                texCamera->getLastViewport()->setMaterialScheme( viewport->getMaterialScheme() );

                // Associate main view camera as LOD camera
                texCamera->setLodCamera( lodCamera );

                // set base
                if( light->getType() != Light::LT_POINT )
                {
                    texCamera->setOrientation( light->getParentNode()->_getDerivedOrientation() *
                                               Quaternion( Radian(Math::PI), Vector3::UNIT_Y ) );
                }
                if( light->getType() != Light::LT_DIRECTIONAL )
                {
                    texCamera->setPosition( light->getParentNode()->_getDerivedPosition() );
                }

                if( itor->shadowMapTechnique == SHADOWMAP_PSSM )
                {
                    assert( dynamic_cast<PSSMShadowCameraSetup*>
                            ( itShadowCamera->shadowCameraSetup.get() ) );

                    PSSMShadowCameraSetup *pssmSetup = static_cast<PSSMShadowCameraSetup*>
                                                        ( itShadowCamera->shadowCameraSetup.get() );
                    if( pssmSetup->getSplitPoints()[0] != camera->getNearClipDistance() ||
                        pssmSetup->getSplitPoints()[itor->numSplits-1] != light->getShadowFarDistance() )
                    {
                        pssmSetup->calculateSplitPoints( itor->numSplits, camera->getNearClipDistance(),
                                                    light->getShadowFarDistance(), itor->pssmLambda );
                    }
                }

                itShadowCamera->shadowCameraSetup->getShadowCamera( sceneManager, camera, light,
                                                                    texCamera, itor->split );

                itShadowCamera->minDistance = itShadowCamera->shadowCameraSetup->getMinDistance();
                itShadowCamera->maxDistance = itShadowCamera->shadowCameraSetup->getMaxDistance();
            }
            //Else... this shadow map shouldn't be rendered and when used, return a blank one.
            //The Nth closest lights don't cast shadows

            ++itShadowCamera;
            ++itor;
        }

        SceneManager::IlluminationRenderStage previous = sceneManager->_getCurrentRenderStage();
        sceneManager->_setCurrentRenderStage( SceneManager::IRS_RENDER_TO_TEXTURE );

        //Now render all passes
        CompositorNode::_update( lodCamera, sceneManager );

        sceneManager->_setCurrentRenderStage( previous );
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::postInitializePass( CompositorPass *pass )
    {
        const CompositorPassDef *passDef = pass->getDefinition();
        const ShadowMapCamera &smCamera = mShadowMapCameras[passDef->mShadowMapIdx];

        assert( (!smCamera.camera->getLastViewport() ||
                smCamera.camera->getLastViewport() == pass->getViewport()) &&
                "Two scene passes to the same shadow map have different viewport!" );

        smCamera.camera->_notifyViewport( pass->getViewport() );

        if( passDef->getType() == PASS_SCENE )
        {
            assert( dynamic_cast<CompositorPassScene*>(pass) );
            static_cast<CompositorPassScene*>(pass)->_setCustomCamera( smCamera.camera );
        }
    }
    //-----------------------------------------------------------------------------------
    const LightList* CompositorShadowNode::setShadowMapsToPass( Renderable* rend, const Pass* pass,
                                                                AutoParamDataSource *autoParamDataSource,
                                                                size_t startLight )
    {
        const size_t lightsPerPass = pass->getMaxSimultaneousLights();

        mCurrentLightList.clear();
        mCurrentLightList.reserve( lightsPerPass );

        const LightList& renderableLights = rend->getLights();

        size_t shadowMapStart = std::min( startLight, mShadowMapCastingLights.size() );
        size_t shadowMapEnd   = std::min( startLight + lightsPerPass, mShadowMapCastingLights.size() );

        //Push **all** shadow casting lights first.
        {
            LightClosestArray::const_iterator itor = mShadowMapCastingLights.begin() + shadowMapStart;
            LightClosestArray::const_iterator end  = mShadowMapCastingLights.begin() + shadowMapEnd;
            while( itor != end )
            {
                mCurrentLightList.push_back( *itor );
                ++itor;
            }
        }

        //Now again, but push non-shadow casting lights (if there's room left)
        {
            size_t slotsToSkip  = std::max<signed>( startLight - mCurrentLightList.size(), 0 );
            size_t slotsLeft    = std::max<signed>( lightsPerPass - (shadowMapEnd - shadowMapStart), 0 );
            LightList::const_iterator itor = renderableLights.begin();
            LightList::const_iterator end  = renderableLights.end();
            while( itor != end && slotsLeft > 0 )
            {
                if( !mAffectedLights[itor->globalIndex] )
                {
                    if( slotsToSkip > 0 )
                    {
                        --slotsToSkip;
                    }
                    else
                    {
                        mCurrentLightList.push_back( *itor );
                        --slotsLeft;
                    }
                }
                ++itor;
            }
        }

        //Set the shadow map texture units
        {
            CompositorManager2 *compoMgr = mWorkspace->getCompositorManager();

            assert( shadowMapStart < mDefinition->mShadowMapTexDefinitions.size() );

            size_t shadowIdx=0;
            CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator shadowTexItor =
                                        mDefinition->mShadowMapTexDefinitions.begin() + shadowMapStart;
            CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator shadowTexItorEnd  =
                                        mDefinition->mShadowMapTexDefinitions.end();
            while( shadowTexItor != shadowTexItorEnd && shadowIdx < pass->getNumShadowContentTextures() )
            {
                size_t texUnitIdx = pass->_getTextureUnitWithContentTypeIndex(
                                                    TextureUnitState::CONTENT_SHADOW, shadowIdx );
                // I know, nasty const_cast
                TextureUnitState *texUnit = const_cast<TextureUnitState*>(
                                                    pass->getTextureUnitState(texUnitIdx) );

                // Projective texturing needs to be disabled explicitly when using vertex shaders.
                texUnit->setProjectiveTexturing( false, (const Frustum*)0 );
                autoParamDataSource->setTextureProjector( mShadowMapCameras[shadowIdx].camera,
                                                            shadowIdx );

                //TODO: textures[0] is out of bounds when using shadow atlas. Also see how what
                //changes need to be done so that UV calculations land on the right place
                const TexturePtr& shadowTex = mLocalTextures[shadowIdx].textures[0];
                texUnit->_setTexturePtr( shadowTex );

                ++shadowIdx;
                ++shadowTexItor;
            }

            for( ; shadowIdx<pass->getNumShadowContentTextures(); ++shadowIdx )
            {
                //If we're here, the material supports more shadow maps than the
                //shadow node actually renders. This probably smells slopy setup.
                //Put blank textures
                size_t texUnitIdx = pass->_getTextureUnitWithContentTypeIndex(
                                                    TextureUnitState::CONTENT_SHADOW, shadowIdx );
                // I know, nasty const_cast
                TextureUnitState *texUnit = const_cast<TextureUnitState*>(
                                                    pass->getTextureUnitState(texUnitIdx) );
                texUnit->_setTexturePtr( compoMgr->getNullShadowTexture( PF_R8G8B8A8 ) );

                // Projective texturing needs to be disabled explicitly when using vertex shaders.
                texUnit->setProjectiveTexturing( false, (const Frustum*)0 );
                autoParamDataSource->setTextureProjector( 0, shadowIdx );
            }
        }

        return &mCurrentLightList;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::getMinMaxDepthRange( const Frustum *shadowMapCamera,
                                                    Real &outMin, Real &outMax ) const
    {
        ShadowMapCameraVec::const_iterator itor = mShadowMapCameras.begin();
        ShadowMapCameraVec::const_iterator end  = mShadowMapCameras.end();

        while( itor != end )
        {
            if( itor->camera == shadowMapCamera )
            {
                outMin = itor->minDistance;
                outMax = itor->maxDistance;
                return;
            }
            ++itor;
        }

        outMin = 0.0f;
        outMax = 100000.0f;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::getMinMaxDepthRange( size_t shadowMapIdx,
                                                    Real &outMin, Real &outMax ) const
    {
        outMin = mShadowMapCameras[shadowMapIdx].minDistance;
        outMax = mShadowMapCameras[shadowMapIdx].maxDistance;
    }
    //-----------------------------------------------------------------------------------
    Matrix4 CompositorShadowNode::getViewProjectionMatrix( size_t shadowMapIdx ) const
    {
        return PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE *
                mShadowMapCameras[shadowMapIdx].camera->getProjectionMatrixWithRSDepth() *
                mShadowMapCameras[shadowMapIdx].camera->getViewMatrix();
    }
    //-----------------------------------------------------------------------------------
    const vector<Real>::type* CompositorShadowNode::getPssmSplits( size_t shadowMapIdx ) const
    {
        vector<Real>::type const *retVal = 0;

        if( shadowMapIdx < mShadowMapCastingLights.size() )
        {
            if( mDefinition->mShadowMapTexDefinitions[shadowMapIdx].shadowMapTechnique ==
                SHADOWMAP_PSSM )
            {
                assert( dynamic_cast<PSSMShadowCameraSetup*>(
                        mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() ) );

                PSSMShadowCameraSetup *pssmSetup = static_cast<PSSMShadowCameraSetup*>(
                                            mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() );
                retVal = &pssmSetup->getSplitPoints();
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::finalTargetResized( const RenderTarget *finalTarget )
    {
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                                                        mDefinition->mShadowMapTexDefinitions.begin();
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
                                                        mDefinition->mShadowMapTexDefinitions.end();

        CompositorChannelVec::iterator itorTex = mLocalTextures.begin();

        while( itor != end )
        {
            if( (itor->width == 0 || itor->height == 0) && itorTex->isValid() )
            {
                for( size_t i=0; i<itorTex->textures.size(); ++i )
                    TextureManager::getSingleton().remove( itorTex->textures[i]->getName() );

                CompositorChannel newChannel = createShadowTexture( *itor, finalTarget );

                CompositorPassVec::const_iterator passIt = mPasses.begin();
                CompositorPassVec::const_iterator passEn = mPasses.end();
                while( passIt != passEn )
                {
                    (*passIt)->notifyRecreated( *itorTex, newChannel );
                    ++passIt;
                }

                CompositorNodeVec::const_iterator itNodes = mConnectedNodes.begin();
                CompositorNodeVec::const_iterator enNodes = mConnectedNodes.end();

                while( itNodes != enNodes )
                {
                    (*itNodes)->notifyRecreated( *itorTex, newChannel );
                    ++itNodes;
                }

                if( !itorTex->isMrt() )
                    mRenderSystem->destroyRenderTarget( itorTex->target->getName() );

                *itorTex = newChannel;
            }
            ++itorTex;
            ++itor;
        }

        //Now recreate the regular textures (i.e. local textures used for
        //postprocessing and ping-ponging, which aren't shadow maps)
        CompositorChannelVec normalLocalTextures;
        const size_t normalStart = mDefinition->mShadowMapTexDefinitions.size();
        normalLocalTextures.reserve( mLocalTextures.size() - normalStart );
        normalLocalTextures.insert( normalLocalTextures.end(), mLocalTextures.begin() + normalStart,
                                                                mLocalTextures.end() );
        TextureDefinitionBase::recreateResizableTextures( mDefinition->mLocalTextureDefs, normalLocalTextures,
                                                            finalTarget, mRenderSystem, mConnectedNodes,
                                                            &mPasses );
        mLocalTextures.erase( mLocalTextures.begin() + normalStart, mLocalTextures.end() );
        mLocalTextures.insert( mLocalTextures.end(), normalLocalTextures.begin(),
                                                     normalLocalTextures.end() );
    }
}
