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

#include "OgreCamera.h"

#include "Compositor/OgreCompositorShadowNode.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassScene.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassComputeDef.h"

#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTexture.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgrePass.h"
#include "OgreDepthBuffer.h"

#include "OgreShadowCameraSetupFocused.h"
#include "OgreShadowCameraSetupPSSM.h"

#include "OgreLogManager.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    #include <intrin.h>
    #pragma intrinsic(_BitScanForward)
#endif

namespace Ogre
{
    const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,    0,  0.5,
        0,   -0.5,    0,  0.5,
        0,      0,    1,    0,
        0,      0,    0,    1);

    inline uint32 ctz( uint32 value )
    {
        if( value == 0 )
            return 32u;

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
        unsigned long trailingZero = 0;
        _BitScanForward( &trailingZero, value );
        return trailingZero;
#else
        return __builtin_ctz( value );
#endif
    }

    CompositorShadowNode::CompositorShadowNode( IdType id, const CompositorShadowNodeDef *definition,
                                                CompositorWorkspace *workspace, RenderSystem *renderSys,
                                                const RenderTarget *finalTarget ) :
            CompositorNode( id, definition->getName(), definition, workspace, renderSys, finalTarget ),
            mDefinition( definition ),
            mLastCamera( 0 ),
            mLastFrame( -1 ),
            mNumActiveShadowMapCastingLights( 0 )
    {
        mShadowMapCameras.reserve( definition->mShadowMapTexDefinitions.size() );
        mLocalTextures.reserve( mLocalTextures.size() + definition->mShadowMapTexDefinitions.size() );

        SceneManager *sceneManager = workspace->getSceneManager();
        SceneNode *pseudoRootNode = 0;

        if( !definition->mShadowMapTexDefinitions.empty() )
        {
            pseudoRootNode = sceneManager->createSceneNode( SCENE_DYNAMIC );
            pseudoRootNode->setIndestructibleByClearScene( true );
        }

        //Create the local textures
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                                                            definition->mShadowMapTexDefinitions.begin();
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
                                                            definition->mShadowMapTexDefinitions.end();

        while( itor != end )
        {
            // One map, one camera
            const size_t shadowMapIdx = itor - definition->mShadowMapTexDefinitions.begin();
            ShadowMapCamera shadowMapCamera;
            shadowMapCamera.camera = sceneManager->createCamera( "ShadowNode Camera ID " +
                                                StringConverter::toString( id ) + " Map " +
                                                StringConverter::toString( shadowMapIdx ), false );
            shadowMapCamera.camera->setFixedYawAxis( false );
            shadowMapCamera.minDistance = 0.0f;
            shadowMapCamera.maxDistance = 100000.0f;
            for( size_t i=0; i<Light::NUM_LIGHT_TYPES; ++i )
                shadowMapCamera.scenePassesViewportSize[i] = -Vector2::UNIT_SCALE;

            {
                //Find out the index to our texture in both mLocalTextures & mContiguousShadowMapTex
                size_t index;
                TextureDefinitionBase::TextureSource textureSource;
                mDefinition->getTextureSource( itor->getTextureName(), index, textureSource );

                // CompositorShadowNodeDef should've prevented this from not being true.
                assert( textureSource == TextureDefinitionBase::TEXTURE_LOCAL );

                shadowMapCamera.idxToLocalTextures = static_cast<uint32>( index );

                if( itor->mrtIndex >= mLocalTextures[index].textures.size() )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Texture " +
                                 itor->getTextureNameStr() + " does not have MRT index " +
                                 StringConverter::toString( itor->mrtIndex ),
                                 "CompositorShadowNode::CompositorShadowNode" );
                }

                TexturePtr &refTex = mLocalTextures[index].textures[itor->mrtIndex];
                TextureVec::const_iterator itContig = std::find( mContiguousShadowMapTex.begin(),
                                                                 mContiguousShadowMapTex.end(), refTex );
                if( itContig == mContiguousShadowMapTex.end() )
                {
                    mContiguousShadowMapTex.push_back( refTex );
                    itContig = mContiguousShadowMapTex.end() - 1u;
                }

                shadowMapCamera.idxToContiguousTex = static_cast<uint32>(
                            itContig - mContiguousShadowMapTex.begin() );
            }


            {
                //Attach the camera to a node that exists outside the scene, so that it
                //doesn't get affected by relative origins (otherwise we'll be setting
                //the relative origin *twice*)
                shadowMapCamera.camera->detachFromParent();
                pseudoRootNode->attachObject( shadowMapCamera.camera );
            }


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
                        setup->calculateSplitPoints( itor->numSplits, 0.1f, 100.0f, 0.95f, 0.125f, 0.313f );
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

        // Shadow Nodes don't have input; and global textures should be ready by
        // the time we get created. Therefore, we can safely initialize now as our
        // output may be used in regular nodes and we're created on-demand (as soon
        // as a Node discovers it needs us for the first time, we get created)
        createPasses();

        mShadowMapCastingLights.resize( mDefinition->mNumLights );
    }
    //-----------------------------------------------------------------------------------
    CompositorShadowNode::~CompositorShadowNode()
    {
        SceneNode *pseudoRootNode = 0;
        SceneManager *sceneManager = mWorkspace->getSceneManager();

        ShadowMapCameraVec::const_iterator itor = mShadowMapCameras.begin();
        ShadowMapCameraVec::const_iterator end  = mShadowMapCameras.end();

        while( itor != end )
        {
            pseudoRootNode = itor->camera->getParentSceneNode();
            sceneManager->destroyCamera( itor->camera );
            ++itor;
        }

        if( pseudoRootNode )
            sceneManager->destroySceneNode( pseudoRootNode );
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
    class SortByLightTypeCmp
    {
        LightListInfo const *mLightList;

    public:
        SortByLightTypeCmp( LightListInfo const *lightList ) :
            mLightList( lightList )
        {
        }

        bool operator()( size_t _l, size_t _r ) const
        {
            assert( _l < mLightList->lights.size() ); //This should never happen.
            assert( _r < mLightList->lights.size() ); //This should never happen.
            return mLightList->lights[_l]->getType() < mLightList->lights[_r]->getType();
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

        clearShadowCastingLights( globalLightList );

        size_t startIndex = 0;
        size_t begEmptyLightIdx = 0;
        size_t nxtEmptyLightIdx = 0;
        findNextEmptyShadowCastingLightEntry( 1u << Light::LT_DIRECTIONAL,
                                              &begEmptyLightIdx, &nxtEmptyLightIdx );

        {
            //SceneManager puts the directional lights first. Add them first as casters.
            LightArray::const_iterator itor = globalLightList.lights.begin();
            LightArray::const_iterator end  = globalLightList.lights.end();

            uint32 const * RESTRICT_ALIAS visibilityMask = globalLightList.visibilityMask;

            while( itor != end && (*itor)->getType() == Light::LT_DIRECTIONAL &&
                   nxtEmptyLightIdx < mShadowMapCastingLights.size() )
            {
                if( (*visibilityMask & combinedVisibilityFlags) &&
                    (*visibilityMask & VisibilityFlags::LAYER_SHADOW_CASTER) )
                {
                    const size_t listIdx = itor - globalLightList.lights.begin();
                    mAffectedLights[listIdx] = true;
                    mShadowMapCastingLights[nxtEmptyLightIdx] = LightClosest( *itor, listIdx, 0 );
                    findNextEmptyShadowCastingLightEntry( 1u << Light::LT_DIRECTIONAL,
                                                          &begEmptyLightIdx, &nxtEmptyLightIdx );
                    ++mNumActiveShadowMapCastingLights;
                }

                ++visibilityMask;
                ++itor;
            }

            //Reach the end of directional lights section
            while( itor != end && (*itor)->getType() == Light::LT_DIRECTIONAL )
                ++itor;

            startIndex = itor - globalLightList.lights.begin();
        }

        const Vector3 &camPos( newCamera->getDerivedPosition() );

        const size_t numTmpSortedLights = std::min( mShadowMapCastingLights.size() - begEmptyLightIdx,
                                                    globalLightList.lights.size() - startIndex );

        mTmpSortedIndexes.resize( numTmpSortedLights, ~0 );
        std::partial_sort_copy( MemoryLessInputIterator( startIndex ),
                            MemoryLessInputIterator( globalLightList.lights.size() ),
                            mTmpSortedIndexes.begin(), mTmpSortedIndexes.end(),
                            ShadowMappingLightCmp( &globalLightList, combinedVisibilityFlags, camPos ) );

        std::sort( mTmpSortedIndexes.begin(), mTmpSortedIndexes.end(),
                   SortByLightTypeCmp( &globalLightList ) );

        vector<size_t>::type::const_iterator itor = mTmpSortedIndexes.begin();
        vector<size_t>::type::const_iterator end  = mTmpSortedIndexes.end();

        while( itor != end )
        {
            assert( *itor < globalLightList.lights.size() ); //This should never happen.

            uint32 visibilityMask = globalLightList.visibilityMask[*itor];
            if( !(visibilityMask & combinedVisibilityFlags) ||
                !(visibilityMask & VisibilityFlags::LAYER_SHADOW_CASTER) ||
                begEmptyLightIdx >= mShadowMapCastingLights.size() )
            {
                break;
            }

            findNextEmptyShadowCastingLightEntry( 1u << globalLightList.lights[*itor]->getType(),
                                                  &begEmptyLightIdx, &nxtEmptyLightIdx );

            if( nxtEmptyLightIdx < mShadowMapCastingLights.size() )
            {
                mAffectedLights[*itor] = true;
                mShadowMapCastingLights[nxtEmptyLightIdx] =
                        LightClosest( globalLightList.lights[*itor], *itor, 0 );
                ++mNumActiveShadowMapCastingLights;
            }
            ++itor;
        }

        restoreStaticShadowCastingLights( globalLightList );

        mCastersBox = sceneManager->_calculateCurrentCastersBox( viewport->getVisibilityMask(),
                                                                 mDefinition->mMinRq,
                                                                 mDefinition->mMaxRq );
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::findNextEmptyShadowCastingLightEntry(
            uint8 lightTypeMask,
            size_t * RESTRICT_ALIAS startIdx,
            size_t * RESTRICT_ALIAS entryToUse ) const
    {
        size_t lightIdx = *startIdx;

        size_t newStartIdx = mShadowMapCastingLights.size();

        LightClosestArray::const_iterator itCastingLight = mShadowMapCastingLights.begin() + lightIdx;
        LightClosestArray::const_iterator enCastingLight = mShadowMapCastingLights.end();
        while( itCastingLight != enCastingLight )
        {
            if( !itCastingLight->light )
            {
                newStartIdx = std::min( lightIdx, newStartIdx );
                if( mDefinition->mLightTypesMask[lightIdx] & lightTypeMask )
                {
                    *startIdx = newStartIdx;
                    *entryToUse = lightIdx;
                    return;
                }
            }

            ++lightIdx;
            ++itCastingLight;
        }

        //If we get here entryToUse == mShadowMapCastingLights.size() but startIdx may still
        //be valid (we found no entry that supports the requested light type but there could
        //still be empty entries for other types of light)
        *startIdx = newStartIdx;
        *entryToUse = lightIdx;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::clearShadowCastingLights( const LightListInfo &globalLightList )
    {
        mAffectedLights.clear();
        //Reserve last place for avoid crashing with static
        //lights that weren't collected into globalLightList
        mAffectedLights.resize( globalLightList.lights.size() + 1u, false );

        mNumActiveShadowMapCastingLights = 0;

        LightClosestArray::iterator itor = mShadowMapCastingLights.begin();
        LightClosestArray::iterator end  = mShadowMapCastingLights.end();

        while( itor != end )
        {
            if( !itor->isStatic )
            {
                *itor = LightClosest();
            }
            else
            {
                LightArray::const_iterator it = std::find( globalLightList.lights.begin(),
                                                           globalLightList.lights.end(),
                                                           itor->light );

                if( it != globalLightList.lights.end() )
                {
                    itor->globalIndex = it - globalLightList.lights.begin();
                    mAffectedLights[itor->globalIndex] = true;

                    //Force this light to "not cast shadow" to fool buildClosestLightList
                    //and prevent assigning this light into any other shadow map by accident
                    itor->light->setCastShadows( false );
                    globalLightList.visibilityMask[itor->globalIndex] =itor->light->getVisibilityFlags();
                }
                else
                {
                    itor->globalIndex = mAffectedLights.size() - 1u;
                }

                ++mNumActiveShadowMapCastingLights;
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::restoreStaticShadowCastingLights( const LightListInfo &globalLightList )
    {
        LightClosestArray::iterator itor = mShadowMapCastingLights.begin();
        LightClosestArray::iterator end  = mShadowMapCastingLights.end();

        while( itor != end )
        {
            if( itor->isStatic && itor->globalIndex < globalLightList.lights.size() )
            {
                itor->light->setCastShadows( true );
                globalLightList.visibilityMask[itor->globalIndex] = itor->light->getVisibilityFlags();
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::_update( Camera* camera, const Camera *lodCamera,
                                        SceneManager *sceneManager )
    {
        ShadowMapCameraVec::iterator itShadowCamera = mShadowMapCameras.begin();

        buildClosestLightList( camera, lodCamera );

        //Setup all the cameras
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                                                            mDefinition->mShadowMapTexDefinitions.begin();
        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end  =
                                                            mDefinition->mShadowMapTexDefinitions.end();

        while( itor != end )
        {
            Light const *light = mShadowMapCastingLights[itor->light].light;

            if( light )
            {
                Camera *texCamera = itShadowCamera->camera;

                //Use the material scheme of the main viewport
                //This is required to pick up the correct shadow_caster_material and similar properties.
                //dark_sylinc: removed. It's losing usefulness (Hlms), and it's broken (CompositorPassScene
                //will overwrite it anyway)
                //texCamera->getLastViewport()->setMaterialScheme( viewport->getMaterialScheme() );

                // Associate main view camera as LOD camera
                texCamera->setLodCamera( lodCamera );

                // set base
                if( light->getType() != Light::LT_POINT )
                    texCamera->setOrientation( light->getParentNode()->_getDerivedOrientation() );
                else
                    texCamera->setOrientation( Quaternion::IDENTITY );

                if( light->getType() != Light::LT_DIRECTIONAL )
                {
                    texCamera->setPosition( light->getParentNode()->_getDerivedPosition() );
                    texCamera->setAutoAspectRatio( true );
                }
                else
                {
                    texCamera->setAutoAspectRatio( false );
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
                                                    light->getShadowFarDistance(), itor->pssmLambda, itor->splitBlend, itor->splitFade );
                    }
                }

                //Set the viewport to 0, to explictly crash if accidentally using it. Compositors
                //may have many passes of different sizes and resolutions that affect the same shadow
                //map and it's impossible to tell which one is "the main one" (if there's any)
                texCamera->_notifyViewport( 0 );

                const Vector2 vpRealSize = itShadowCamera->scenePassesViewportSize[light->getType()];
                itShadowCamera->shadowCameraSetup->getShadowCamera( sceneManager, camera, light,
                                                                    texCamera, itor->split,
                                                                    vpRealSize );

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

        {
            LightClosestArray::iterator it = mShadowMapCastingLights.begin();
            LightClosestArray::iterator en = mShadowMapCastingLights.end();

            while( it != en )
            {
                it->isDirty = false;
                ++it;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::postInitializePass( CompositorPass *pass )
    {
        const CompositorPassDef *passDef = pass->getDefinition();

        //passDef->mShadowMapIdx may be invalid if this is not a pass
        //tied to a shadow map in particular (e.g. clearing an atlas)
        if( passDef->mShadowMapIdx < mShadowMapCameras.size() )
        {
            if( passDef->getType() == PASS_SCENE )
            {
                ShadowMapCamera &smCamera = mShadowMapCameras[passDef->mShadowMapIdx];

                const Viewport *vp = pass->getViewport();
                const Vector2 vpSize = Vector2( vp->getActualWidth(), vp->getActualHeight() );

                const CompositorTargetDef *targetPass = passDef->getParentTargetDef();
                uint8 lightTypesLeft = targetPass->getShadowMapSupportedLightTypes();

                //Get the viewport size set for this shadow node (which may vary per light type,
                //but for the same light type, it must remain constant for all passes to the
                //same shadow map)
                uint32 firstBitSet = ctz( lightTypesLeft );
                while( firstBitSet != 32u )
                {
                    assert( (smCamera.scenePassesViewportSize[firstBitSet].x < Real( 0.0 ) ||
                             smCamera.scenePassesViewportSize[firstBitSet].x < Real( 0.0 ) ||
                             smCamera.scenePassesViewportSize[firstBitSet] == vpSize) &&
                            "Two scene passes to the same shadow map have different viewport sizes! "
                            "Ogre cannot determine how to prevent jittering. Maybe you meant assign "
                            "assign each light types to different passes but you assigned more than "
                            "one light type (or the wrong one) to the same pass?" );

                    smCamera.scenePassesViewportSize[firstBitSet] = vpSize;

                    lightTypesLeft &= ~(1u << ((uint8)firstBitSet));
                    firstBitSet = ctz( lightTypesLeft );
                }

                assert( dynamic_cast<CompositorPassScene*>(pass) );
                static_cast<CompositorPassScene*>(pass)->_setCustomCamera( smCamera.camera );
                static_cast<CompositorPassScene*>(pass)->_setCustomCullCamera( smCamera.camera );
            }
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
            size_t slotsToSkip  = std::max<ptrdiff_t>( startLight - mCurrentLightList.size(), 0 );
            size_t slotsLeft    = std::max<ptrdiff_t>( lightsPerPass - (shadowMapEnd - shadowMapStart), 0 );
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
    bool CompositorShadowNode::isShadowMapIdxInValidRange( uint32 shadowMapIdx ) const
    {
        return shadowMapIdx < mDefinition->mShadowMapTexDefinitions.size();
    }
    //-----------------------------------------------------------------------------------
    bool CompositorShadowNode::isShadowMapIdxActive( uint32 shadowMapIdx ) const
    {
        if( shadowMapIdx < mDefinition->mShadowMapTexDefinitions.size() )
        {
            const ShadowTextureDefinition &shadowTexDef =
                    mDefinition->mShadowMapTexDefinitions[shadowMapIdx];
            return mShadowMapCastingLights[shadowTexDef.light].light != 0;
        }
        else
        {
            return true;
        }
    }
    //-----------------------------------------------------------------------------------
    bool CompositorShadowNode::_shouldUpdateShadowMapIdx( uint32 shadowMapIdx ) const
    {
        bool retVal = true;

        if( shadowMapIdx < mDefinition->mShadowMapTexDefinitions.size() )
        {
            const ShadowTextureDefinition &shadowTexDef =
                    mDefinition->mShadowMapTexDefinitions[shadowMapIdx];

            if( !mShadowMapCastingLights[shadowTexDef.light].light ||
                (mShadowMapCastingLights[shadowTexDef.light].isStatic &&
                !mShadowMapCastingLights[shadowTexDef.light].isDirty ) )
            {
                retVal = false;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint8 CompositorShadowNode::getShadowMapLightTypeMask( uint32 shadowMapIdx ) const
    {
        const ShadowTextureDefinition &shadowTexDef =
                mDefinition->mShadowMapTexDefinitions[shadowMapIdx];
        return 1u << mShadowMapCastingLights[shadowTexDef.light].light->getType();
    }
    //-----------------------------------------------------------------------------------
    const Light* CompositorShadowNode::getLightAssociatedWith( uint32 shadowMapIdx ) const
    {
        Light const *retVal = 0;

        if( shadowMapIdx < mDefinition->mShadowMapTexDefinitions.size() )
        {
            const ShadowTextureDefinition &shadowTexDef =
                    mDefinition->mShadowMapTexDefinitions[shadowMapIdx];
            retVal = mShadowMapCastingLights[shadowTexDef.light].light;
        }

        return retVal;
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
        const ShadowTextureDefinition &shadowTexDef =
                mDefinition->mShadowMapTexDefinitions[shadowMapIdx];
        Matrix4 clipToImageSpace;

        Vector3 vScale(  0.5f * shadowTexDef.uvLength.x,
                        -0.5f * shadowTexDef.uvLength.y, 1.0f );
        clipToImageSpace.makeTransform( Vector3(  vScale.x + shadowTexDef.uvOffset.x,
                                                 -vScale.y + shadowTexDef.uvOffset.y, 0.0f ),
                                        Vector3(  vScale.x, vScale.y, 1.0f ),
                                        Quaternion::IDENTITY );

        return /*PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE*/clipToImageSpace *
                mShadowMapCameras[shadowMapIdx].camera->getProjectionMatrixWithRSDepth() *
                mShadowMapCameras[shadowMapIdx].camera->getViewMatrix( true );
    }
    //-----------------------------------------------------------------------------------
    const Matrix4& CompositorShadowNode::getViewMatrix( size_t shadowMapIdx ) const
    {
        return mShadowMapCameras[shadowMapIdx].camera->getViewMatrix( true );
    }
    //-----------------------------------------------------------------------------------
    const vector<Real>::type* CompositorShadowNode::getPssmSplits( size_t shadowMapIdx ) const
    {
        vector<Real>::type const *retVal = 0;

        if( shadowMapIdx < mShadowMapCastingLights.size() )
        {
            if( mDefinition->mShadowMapTexDefinitions[shadowMapIdx].shadowMapTechnique ==
                SHADOWMAP_PSSM &&
                isShadowMapIdxActive( shadowMapIdx ) )
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
    const vector<Real>::type* CompositorShadowNode::getPssmBlends( size_t shadowMapIdx ) const
    {
        vector<Real>::type const *retVal = 0;

        if( shadowMapIdx < mShadowMapCastingLights.size() )
        {
            if( mDefinition->mShadowMapTexDefinitions[shadowMapIdx].shadowMapTechnique ==
                SHADOWMAP_PSSM &&
                isShadowMapIdxActive( shadowMapIdx ) )
            {
                assert( dynamic_cast<PSSMShadowCameraSetup*>(
                        mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() ) );

                PSSMShadowCameraSetup *pssmSetup = static_cast<PSSMShadowCameraSetup*>(
                                            mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() );
                retVal = &pssmSetup->getSplitBlendPoints();
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const Real* CompositorShadowNode::getPssmFade( size_t shadowMapIdx ) const
    {
        Real const *retVal = 0;

        if( shadowMapIdx < mShadowMapCastingLights.size() )
        {
            if( mDefinition->mShadowMapTexDefinitions[shadowMapIdx].shadowMapTechnique ==
                SHADOWMAP_PSSM &&
                isShadowMapIdxActive( shadowMapIdx ) )
            {
                assert( dynamic_cast<PSSMShadowCameraSetup*>(
                        mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() ) );

                PSSMShadowCameraSetup *pssmSetup = static_cast<PSSMShadowCameraSetup*>(
                                            mShadowMapCameras[shadowMapIdx].shadowCameraSetup.get() );
                retVal = &pssmSetup->getSplitFadePoint();
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 CompositorShadowNode::getIndexToContiguousShadowMapTex( size_t shadowMapIdx ) const
    {
        return mShadowMapCameras[shadowMapIdx].idxToContiguousTex;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::setLightFixedToShadowMap( size_t shadowMapIdx, Light *light )
    {
        assert( shadowMapIdx < mShadowMapCameras.size() );

        const size_t lightIdx = mDefinition->mShadowMapTexDefinitions[shadowMapIdx].light;
        assert( (!light ||
                mDefinition->mLightTypesMask[lightIdx] & (1u << light->getType())) &&
                "The shadow map says that type of light is not supported!" );

        mShadowMapCastingLights[lightIdx].light = light;
        mShadowMapCastingLights[lightIdx].isStatic = light != 0;
        mShadowMapCastingLights[lightIdx].isDirty = true;
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::setStaticShadowMapDirty( size_t shadowMapIdx, bool includeLinked )
    {
        assert( shadowMapIdx < mShadowMapCameras.size() );

        const ShadowTextureDefinition &shadowTexDef =
                mDefinition->mShadowMapTexDefinitions[shadowMapIdx];
        const size_t lightIdx = mDefinition->mShadowMapTexDefinitions[shadowMapIdx].light;
        assert( mShadowMapCastingLights[lightIdx].isStatic &&
                "Shadow Map is not static! Did you forget to call setLightFixedToShadowMap?" );

        mShadowMapCastingLights[lightIdx].isDirty = true;

        if( includeLinked )
        {
            CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itor =
                    mDefinition->mShadowMapTexDefinitions.begin();
            CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator end =
                    mDefinition->mShadowMapTexDefinitions.end();

            while( itor != end )
            {
                if( shadowTexDef.getTextureName() == itor->getTextureName() )
                    mShadowMapCastingLights[itor->light].isDirty = true;

                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorShadowNode::finalTargetResized( const RenderTarget *finalTarget )
    {
        CompositorNode::finalTargetResized( finalTarget );

        mContiguousShadowMapTex.clear();

        CompositorShadowNodeDef::ShadowMapTexDefVec::const_iterator itDef =
                mDefinition->mShadowMapTexDefinitions.begin();
        ShadowMapCameraVec::const_iterator itor = mShadowMapCameras.begin();
        ShadowMapCameraVec::const_iterator end  = mShadowMapCameras.end();

        while( itor != end )
        {
            if( itor->idxToContiguousTex >= mContiguousShadowMapTex.size() )
            {
                mContiguousShadowMapTex.push_back(
                            mLocalTextures[itor->idxToLocalTextures].textures[itDef->mrtIndex] );
            }

            ++itDef;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    ShadowNodeHelper::Resolution::Resolution() :
        x( 0 ), y( 0 ) {}
    //-----------------------------------------------------------------------------------
    ShadowNodeHelper::Resolution::Resolution( uint32 _x, uint32 _y ) :
        x( _x ), y( _y ) {}
    //-----------------------------------------------------------------------------------
    uint64 ShadowNodeHelper::Resolution::asUint64(void) const
    {
        return ((uint64)x << (uint64)32ul) | ((uint64)y);
    }
    //-----------------------------------------------------------------------------------
    void ShadowNodeHelper::ShadowParam::addLightType( Light::LightTypes lightType )
    {
        assert( lightType <= Light::LT_SPOTLIGHT );
        supportedLightTypes |= 1u << lightType;
    }
    //-----------------------------------------------------------------------------------
    void ShadowNodeHelper::createShadowNodeWithSettings( CompositorManager2 *compositorManager,
                                                         const RenderSystemCapabilities *capabilities,
                                                         const String &shadowNodeName,
                                                         const ShadowNodeHelper::
                                                         ShadowParamVec &shadowParams,
                                                         bool useEsm,
                                                         uint32 pointLightCubemapResolution,
                                                         Real pssmLambda, Real splitPadding,
                                                         Real splitBlend, Real splitFade )
    {
        typedef map<uint64, uint32>::type ResolutionsToEsmMap;

        ResolutionsToEsmMap resolutionsToEsmMap;
        const bool supportsCompute = capabilities->hasCapability( RSC_COMPUTE_PROGRAM );

        const uint32 spotMask           = 1u << Light::LT_SPOTLIGHT;
        const uint32 directionalMask    = 1u << Light::LT_DIRECTIONAL;
        const uint32 pointMask          = 1u << Light::LT_POINT;
        const uint32 spotAndDirMask = spotMask | directionalMask;

        typedef vector< Resolution >::type ResolutionVec;

        size_t numExtraShadowMapsForPssmSplits = 0;
        size_t numTargetPasses = 0;
        ResolutionVec atlasResolutions;

        //Validation and data gathering
        bool hasPointLights = false;

        ShadowParamVec::const_iterator itor = shadowParams.begin();
        ShadowParamVec::const_iterator end  = shadowParams.end();

        while( itor != end )
        {
            if( itor->technique == SHADOWMAP_PSSM )
            {
                if( itor->supportedLightTypes != directionalMask )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "PSSM can only only be used with directional lights!",
                                 "CompositorShadowNode::createShadowNodeWithSettings" );
                }
                if( (itor - shadowParams.begin()) != 0 )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "PSSM must be specified in the first entry of shadowParams",
                                 "CompositorShadowNode::createShadowNodeWithSettings" );
                }
                if( itor->numPssmSplits <= 1 || itor->numPssmSplits > 4u )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "Valid numPssmSplits values must be in range [2; 4]",
                                 "CompositorShadowNode::createShadowNodeWithSettings" );
                }

                numExtraShadowMapsForPssmSplits = itor->numPssmSplits - 1u;
                numTargetPasses += numExtraShadowMapsForPssmSplits + 1u; //1 per PSSM split
            }

            if( itor->atlasId >= atlasResolutions.size() )
                atlasResolutions.resize( itor->atlasId + 1u );

            Resolution &resolution = atlasResolutions[itor->atlasId];

            const size_t numSplits = itor->technique == SHADOWMAP_PSSM ? itor->numPssmSplits : 1u;
            for( size_t i=0; i<numSplits; ++i )
            {
                if( itor->resolution[i].x == 0 || itor->resolution[i].y == 0 )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 "Resolution can't be 0",
                                 "CompositorShadowNode::createShadowNodeWithSettings" );
                }

                resolution.x = std::max( resolution.x, itor->atlasStart[i].x + itor->resolution[i].x );
                resolution.y = std::max( resolution.y, itor->atlasStart[i].y + itor->resolution[i].y );
            }

            if( itor->supportedLightTypes & pointMask )
            {
                hasPointLights = true;
                numTargetPasses += 7u; //6 target passes per cubemap + 1 for copy
            }
            if( itor->supportedLightTypes & spotAndDirMask &&
                itor->technique != SHADOWMAP_PSSM )
            {
                //1 per directional/spot light (for non-PSSM techniques)
                numTargetPasses += 1u;
            }

            if( !(itor->supportedLightTypes & (spotAndDirMask|pointMask)) )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "supportedLightTypes does not indicate any valid Light::LightTypes bit",
                             "CompositorShadowNode::createShadowNodeWithSettings" );
            }

            ++itor;
        }

        //One clear for each atlas
        numTargetPasses += atlasResolutions.size();
        if( useEsm )
        {
            //ESM using compute: 1 extra pass (2 subpasses) for the gaussian filter
            //ESM using graphics: 2 extra passes for the gaussian filter
            numTargetPasses += atlasResolutions.size() * (supportsCompute ? 1u : 2u);
        }

        //Create the shadow node definition
        CompositorShadowNodeDef *shadowNodeDef =
                compositorManager->addShadowNodeDefinition( shadowNodeName );

        const size_t numTextures = atlasResolutions.size();
        {
            //Define the atlases (textures)
            shadowNodeDef->setNumLocalTextureDefinitions( numTextures + (hasPointLights ? 1u : 0u) );
            for( size_t i=0; i<numTextures; ++i )
            {
                const Resolution &atlasRes = atlasResolutions[i];

                if( atlasRes.x == 0 || atlasRes.y == 0 )
                {
                    LogManager::getSingleton().logMessage(
                                "WARNING: atlasId is having gaps (e.g. you're using IDs 0 & 2, "
                                "but not using 1). This leads to pointless GPU memory waste. "
                                "Currently not using atlasId = " + StringConverter::toString( i ) );
                }

                TextureDefinitionBase::TextureDefinition *texDef =
                        shadowNodeDef->addTextureDefinition( "atlas" + StringConverter::toString(i) );

                texDef->width   = std::max( atlasRes.x, 1u );
                texDef->height  = std::max( atlasRes.y, 1u );
                if( !useEsm )
                {
                    texDef->formatList.push_back( PF_D32_FLOAT );
                    texDef->depthBufferId = DepthBuffer::POOL_NON_SHAREABLE;
                }
                else
                {
                    texDef->formatList.push_back( PF_L16 );
                    texDef->uav = supportsCompute;
                }
                texDef->depthBufferFormat = PF_D32_FLOAT;
                texDef->preferDepthTexture = false;
                texDef->fsaa = false;

                //Make all atlases with the same resolution share the same temporary
                //gaussian filter target (to avoid wasting GPU RAM) and give
                //each one a unique ID.
                resolutionsToEsmMap[atlasRes.asUint64()] = i;
            }

            //Define the temporary needed to filter ESM using gaussian filters
            if( useEsm )
            {
                ResolutionsToEsmMap::const_iterator itEsm = resolutionsToEsmMap.begin();
                ResolutionsToEsmMap::const_iterator enEsm = resolutionsToEsmMap.end();

                while( itEsm != enEsm )
                {
                    TextureDefinitionBase::TextureDefinition *texDef =
                            shadowNodeDef->addTextureDefinition(
                                "tmpGaussianFilter" + StringConverter::toString(itEsm->second) );

                    texDef->width   = static_cast<uint32>( (itEsm->first >> (uint64)32ul) );
                    texDef->height  = static_cast<uint32>( (itEsm->first & (uint64)0xfffffffful) );
                    texDef->formatList.push_back( PF_L16 );
                    texDef->depthBufferId = DepthBuffer::POOL_NO_DEPTH;
                    texDef->preferDepthTexture = false;
                    texDef->fsaa = false;
                    texDef->uav = supportsCompute;

                    ++itEsm;
                }
            }

            //Define the cubemap needed by point lights
            if( hasPointLights )
            {
                TextureDefinitionBase::TextureDefinition *texDef =
                        shadowNodeDef->addTextureDefinition( "tmpCubemap" );

                texDef->width   = pointLightCubemapResolution;
                texDef->height  = pointLightCubemapResolution;
                texDef->depth   = 6u;
                texDef->textureType = TEX_TYPE_CUBE_MAP;
                texDef->formatList.push_back( PF_FLOAT32_R );
                texDef->depthBufferId = 1u;
                texDef->depthBufferFormat = PF_D32_FLOAT;
                texDef->preferDepthTexture = false;
                texDef->fsaa = false;
            }
        }

        //Create the shadow maps
        const size_t numShadowMaps = shadowParams.size() + numExtraShadowMapsForPssmSplits;
        shadowNodeDef->setNumShadowTextureDefinitions( numShadowMaps );

        itor = shadowParams.begin();

        while( itor != end )
        {
            const size_t lightIdx = itor - shadowParams.begin();
            const ShadowParam &shadowParam = *itor;

            const Resolution &texResolution = atlasResolutions[shadowParam.atlasId];

            const size_t numSplits =
                    shadowParam.technique == SHADOWMAP_PSSM ? shadowParam.numPssmSplits : 1u;

            for( size_t j=0; j<numSplits; ++j )
            {
                Vector2 uvOffset( shadowParam.atlasStart[j].x, shadowParam.atlasStart[j].y );
                Vector2 uvLength( shadowParam.resolution[j].x, shadowParam.resolution[j].y );

                uvOffset /= Vector2( texResolution.x, texResolution.y );
                uvLength /= Vector2( texResolution.x, texResolution.y );

                const String texName = "atlas" + StringConverter::toString( shadowParam.atlasId );

                ShadowTextureDefinition *shadowTexDef =
                        shadowNodeDef->addShadowTextureDefinition( lightIdx, j, texName,
                                                                   0, uvOffset, uvLength, 0 );
                shadowTexDef->shadowMapTechnique = shadowParam.technique;
                shadowTexDef->pssmLambda = pssmLambda;
                shadowTexDef->splitPadding = splitPadding;
                shadowTexDef->splitBlend = splitBlend;
                shadowTexDef->splitFade = splitFade;
                shadowTexDef->numSplits = numSplits;
            }

            ++itor;
        }

        shadowNodeDef->setNumTargetPass( numTargetPasses );

        //Create the passes for each atlas
        for( size_t atlasId=0; atlasId<numTextures; ++atlasId )
        {
            const String texName = "atlas" + StringConverter::toString( atlasId );
            {
                //Atlas clear pass
                CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( texName );
                targetDef->setNumPasses( 1u );

                CompositorPassDef *passDef = targetDef->addPass( PASS_CLEAR );
                CompositorPassClearDef *passClear = static_cast<CompositorPassClearDef*>( passDef );
                passClear->mColourValue = ColourValue::White;
                passClear->mDepthValue = 1.0f;
            }

            //Pass scene for directional and spot lights first
            size_t shadowMapIdx = 0;
            itor = shadowParams.begin();
            while( itor != end )
            {
                const ShadowParam &shadowParam = *itor;
                if( shadowParam.atlasId == atlasId &&
                    shadowParam.supportedLightTypes & spotAndDirMask )
                {
                    const size_t numSplits =
                            shadowParam.technique == SHADOWMAP_PSSM ? shadowParam.numPssmSplits : 1u;
                    for( size_t i=0; i<numSplits; ++i )
                    {
                        CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( texName );
                        targetDef->setShadowMapSupportedLightTypes( shadowParam.supportedLightTypes &
                                                                    spotAndDirMask );
                        targetDef->setNumPasses( 1u );

                        CompositorPassDef *passDef = targetDef->addPass( PASS_SCENE );
                        CompositorPassSceneDef *passScene =
                                static_cast<CompositorPassSceneDef*>( passDef );

                        passScene->mShadowMapIdx = shadowMapIdx;
                        passScene->mIncludeOverlays = false;
                        ++shadowMapIdx;
                    }
                }
                else
                {
                    ++shadowMapIdx;
                }
                ++itor;
            }

            //Pass scene for point lights last
            shadowMapIdx = 0;
            itor = shadowParams.begin();
            while( itor != end )
            {
                const ShadowParam &shadowParam = *itor;
                if( shadowParam.atlasId == atlasId &&
                    shadowParam.supportedLightTypes & pointMask )
                {
                    //Render to cubemap, each face clear + render
                    for( uint32 i=0; i<6u; ++i )
                    {
                        CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( "tmpCubemap", i );
                        targetDef->setNumPasses( 2u );
                        targetDef->setShadowMapSupportedLightTypes( shadowParam.supportedLightTypes &
                                                                    pointMask );
                        {
                            //Clear pass
                            CompositorPassDef *passDef = targetDef->addPass( PASS_CLEAR );
                            CompositorPassClearDef *passClear =
                                    static_cast<CompositorPassClearDef*>( passDef );
                            passClear->mColourValue = ColourValue::White;
                            passClear->mDepthValue = 1.0f;
                            passClear->mShadowMapIdx = shadowMapIdx;
                        }

                        {
                            //Scene pass
                            CompositorPassDef *passDef = targetDef->addPass( PASS_SCENE );
                            CompositorPassSceneDef *passScene =
                                    static_cast<CompositorPassSceneDef*>( passDef );
                            passScene->mCameraCubemapReorient = true;
                            passScene->mShadowMapIdx = shadowMapIdx;
                            passScene->mIncludeOverlays = false;
                        }
                    }

                    //Copy to the atlas using a pass quad (Cubemap -> DPSM / Dual Paraboloid).
                    CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( texName );
                    targetDef->setShadowMapSupportedLightTypes( shadowParam.supportedLightTypes &
                                                                pointMask );
                    targetDef->setNumPasses( 1u );

                    CompositorPassDef *passDef = targetDef->addPass( PASS_QUAD );
                    CompositorPassQuadDef *passQuad = static_cast<CompositorPassQuadDef*>( passDef );
                    passQuad->mMaterialIsHlms = false;
                    passQuad->mMaterialName = "Ogre/DPSM/CubeToDpsm";
                    passQuad->addQuadTextureSource( 0, "tmpCubemap", 0 );
                    passQuad->mShadowMapIdx = shadowMapIdx;
                }
                const size_t numSplits =
                        shadowParam.technique == SHADOWMAP_PSSM ? shadowParam.numPssmSplits : 1u;
                shadowMapIdx += numSplits;
                ++itor;
            }

            //Apply Gaussian Filter on top of the whole atlas after we're done with it
            if( useEsm )
            {
                const String tmpGaussianFilterName =
                        "tmpGaussianFilter" +
                        StringConverter::toString(
                            resolutionsToEsmMap[atlasResolutions[atlasId].asUint64()] );
                if( supportsCompute )
                {
                    CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( texName );
                    targetDef->setNumPasses( 2u );
                    {
                        //Compute pass
                        CompositorPassDef *passDef = targetDef->addPass( PASS_COMPUTE );
                        CompositorPassComputeDef *passCompute =
                                static_cast<CompositorPassComputeDef*>( passDef );
                        passCompute->mJobName = "ESM/GaussianLogFilterH";
                        passCompute->addTextureSource( 0, texName, 0 );
                        passCompute->addUavSource( 0, tmpGaussianFilterName, 0, ResourceAccess::Write,
                                                   0, 0, PF_L16, false );
                    }
                    {
                        //Compute pass
                        CompositorPassDef *passDef = targetDef->addPass( PASS_COMPUTE );
                        CompositorPassComputeDef *passCompute =
                                static_cast<CompositorPassComputeDef*>( passDef );
                        passCompute->mJobName = "ESM/GaussianLogFilterV";
                        passCompute->addTextureSource( 0, tmpGaussianFilterName, 0 );
                        passCompute->addUavSource( 0, texName, 0, ResourceAccess::Write,
                                                   0, 0, PF_L16, false );
                    }
                }
                else
                {
                    {
                        //Quad pass
                        CompositorTargetDef *targetDef =
                                shadowNodeDef->addTargetPass( tmpGaussianFilterName );
                        targetDef->setNumPasses( 1u );

                        CompositorPassDef *passDef = targetDef->addPass( PASS_QUAD );
                        CompositorPassQuadDef *passQuad = static_cast<CompositorPassQuadDef*>( passDef );
                        passQuad->mMaterialIsHlms = false;
                        passQuad->mMaterialName = "ESM/GaussianLogFilterH";
                        passQuad->addQuadTextureSource( 0, texName, 0 );
                    }
                    {
                        //Quad  pass
                        CompositorTargetDef *targetDef = shadowNodeDef->addTargetPass( texName );
                        targetDef->setNumPasses( 1u );

                        CompositorPassDef *passDef = targetDef->addPass( PASS_QUAD );
                        CompositorPassQuadDef *passQuad = static_cast<CompositorPassQuadDef*>( passDef );
                        passQuad->mMaterialIsHlms = false;
                        passQuad->mMaterialName = "ESM/GaussianLogFilterV";
                        passQuad->addQuadTextureSource( 0, tmpGaussianFilterName, 0 );
                    }
                }
            }
        }
    }
}
