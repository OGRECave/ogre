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

#include "OgreHlmsLowLevel.h"
#include "OgreHlmsLowLevelDatablock.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"

#include "Animation/OgreSkeletonInstance.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbLowLevelMaterial.h"

namespace Ogre
{
    const IdString LowLevelProp::PassId                     = IdString( "pass_id" );

    HlmsLowLevel::HlmsLowLevel() :
        Hlms( HLMS_LOW_LEVEL, "", 0, 0 ),
        mAutoParamDataSource( 0 ),
        mCurrentSceneManager( 0 )
    {
        mAutoParamDataSource = OGRE_NEW AutoParamDataSource();
    }
    //-----------------------------------------------------------------------------------
    HlmsLowLevel::~HlmsLowLevel()
    {
        OGRE_DELETE mAutoParamDataSource;
        mAutoParamDataSource = 0;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsLowLevel::createShaderCacheEntry( uint32 renderableHash,
                                                           const HlmsCache &passCache,
                                                           uint32 finalHash,
                                                           const QueuedRenderable &queuedRenderable )
    {
        Renderable *renderable = queuedRenderable.renderable;
        const MaterialPtr &mat = renderable->getMaterial();
        Technique *technique = mat->getBestTechnique( renderable->getCurrentMaterialLod(), renderable );
        Pass *pass = technique->getPass( 0 );

        if( !pass->isProgrammable() )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Fixed Function pipeline is no longer allowed nor supported. "
                         "The material " + mat->getName() + " must use shaders",
                         "HlmsLowLevel::createShaderCacheEntry" );
        }

        HlmsPso pso;
        pso.initialize();
        if( pass->hasVertexProgram() )
            pso.vertexShader            = pass->getVertexProgram();
        if( pass->hasGeometryProgram() )
            pso.geometryShader          = pass->getGeometryProgram();
        if( pass->hasTessellationHullProgram() )
            pso.tesselationHullShader   = pass->getTessellationHullProgram();
        if( pass->hasTessellationDomainProgram() )
            pso.tesselationDomainShader = pass->getTessellationDomainProgram();
        if( pass->hasFragmentProgram() )
            pso.pixelShader             = pass->getFragmentProgram();

        bool casterPass = getProperty( HlmsBaseProp::ShadowCaster ) != 0;

        const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();
        pso.macroblock = datablock->getMacroblock( casterPass );
        pso.blendblock = datablock->getBlendblock( casterPass );
        pso.pass = passCache.pso.pass;

        if( queuedRenderable.renderable )
        {
            const VertexArrayObjectArray &vaos =
                    queuedRenderable.renderable->getVaos( static_cast<VertexPass>(casterPass) );
            if( !vaos.empty() )
            {
                //v2 object. TODO: LOD? Should we allow Vaos with different vertex formats on LODs?
                //(also the datablock hash in the renderable would have to account for that)
                pso.operationType = vaos.front()->getOperationType();
                pso.vertexElements = vaos.front()->getVertexDeclaration();
            }
            else
            {
                //v1 object.
                v1::RenderOperation renderOp;
                queuedRenderable.renderable->getRenderOperation( renderOp, casterPass );
                pso.operationType = renderOp.operationType;
                pso.vertexElements = renderOp.vertexData->vertexDeclaration->convertToV2();
            }

            pso.enablePrimitiveRestart = true;
        }

        applyStrongMacroblockRules( pso );

        mRenderSystem->_hlmsPipelineStateObjectCreated( &pso );

        const HlmsCache* retVal = addShaderCache( finalHash, pso );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsLowLevel::calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash )
    {
        mSetProperties.clear();

        const MaterialPtr &mat = renderable->getMaterial();

        Material::TechniqueIterator techniqueIt = mat->getTechniqueIterator();
        while( techniqueIt.hasMoreElements() )
        {
            Technique *technique = techniqueIt.getNext();
            Technique::PassIterator passIt = technique->getPassIterator();

            while( passIt.hasMoreElements() )
            {
                Pass *pass = passIt.getNext();

                if( !pass->isProgrammable() )
                {
                    OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                 "Fixed Function pipeline is no longer allowed nor supported. "
                                 "The material " + mat->getName() + " must use shaders",
                                 "HlmsLowLevel::calculateHashFor" );
                }
            }
        }

        setProperty( HlmsPsoProp::Macroblock, renderable->getDatablock()->getMacroblock(false)->mId );
        setProperty( HlmsPsoProp::Blendblock, renderable->getDatablock()->getBlendblock(false)->mId );

        Technique *technique = mat->getBestTechnique( renderable->getCurrentMaterialLod(), renderable );
        Pass *pass = technique->getPass( 0 );

        setProperty( LowLevelProp::PassId, static_cast<Ogre::int32>( pass->getId() ) );

        outHash = this->addRenderableCache( mSetProperties, (const PiecesMap*)0 );

        setProperty( HlmsBaseProp::ShadowCaster, true );
        setProperty( HlmsPsoProp::Macroblock, renderable->getDatablock()->getMacroblock(true)->mId );
        setProperty( HlmsPsoProp::Blendblock, renderable->getDatablock()->getBlendblock(true)->mId );

        outCasterHash = this->addRenderableCache( mSetProperties, (const PiecesMap*)0 );
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsLowLevel::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                             bool dualParaboloid, SceneManager *sceneManager )
    {
        mCurrentSceneManager = sceneManager;

        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );

        //Never produce a HlmsCache.hash of 0. Only affects LowLevel because HLMS_LOW_LEVEL = 0
        retVal.hash += 1;

        //TODO: Update auto params here
        Camera *camera = sceneManager->getCameraInProgress();
        mAutoParamDataSource->setCurrentCamera( camera );
        mAutoParamDataSource->setCurrentSceneManager( sceneManager );
        mAutoParamDataSource->setCurrentShadowNode( shadowNode );
        mAutoParamDataSource->setCurrentViewport(sceneManager->getCurrentViewport());

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsLowLevel::fillBuffersFor( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         uint32 lastTextureHash )
    {
        executeCommand( queuedRenderable.movableObject, queuedRenderable.renderable, casterPass );
        return 0;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsLowLevel::fillBuffersForV1( const HlmsCache *cache,
                                           const QueuedRenderable &queuedRenderable,
                                           bool casterPass, uint32 lastCacheHash,
                                           CommandBuffer *commandBuffer )
    {
        *commandBuffer->addCommand<CbLowLevelMaterial>() =
                CbLowLevelMaterial( casterPass, this, queuedRenderable.movableObject,
                                    queuedRenderable.renderable );
        return 0;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsLowLevel::fillBuffersForV2( const HlmsCache *cache,
                                           const QueuedRenderable &queuedRenderable,
                                           bool casterPass, uint32 lastCacheHash,
                                           CommandBuffer *commandBuffer )
    {
        *commandBuffer->addCommand<CbLowLevelMaterial>() =
                CbLowLevelMaterial( casterPass, this, queuedRenderable.movableObject,
                                    queuedRenderable.renderable );

        return 0;
    }
    //-----------------------------------------------------------------------------------
    void HlmsLowLevel::executeCommand( const MovableObject *movableObject, Renderable *renderable,
                                       bool casterPass )
    {
        unsigned short numMatrices = 1;
        if( renderable->getVaos( static_cast<VertexPass>(casterPass) ).empty() )
        {
            numMatrices = renderable->getNumWorldTransforms();
            renderable->getWorldTransforms( mTempXform );
        }
        else
        {
            if( !renderable->hasSkeletonAnimation() )
            {
                mTempXform[0] = movableObject->_getParentNodeFullTransform();
            }
            else
            {
                SkeletonInstance *skeleton = movableObject->getSkeletonInstance();
#if OGRE_DEBUG_MODE
                assert( dynamic_cast<const RenderableAnimated*>( renderable ) );
#endif

                const RenderableAnimated *renderableAnimated = static_cast<const RenderableAnimated*>(
                                                                                            renderable );

                const RenderableAnimated::IndexMap *indexMap = renderableAnimated->getBlendIndexToBoneIndexMap();

                assert( indexMap->size() < 256 &&
                        "Up to 256 bones per submesh are supported for low level materials!" );

                RenderableAnimated::IndexMap::const_iterator itBone = indexMap->begin();
                RenderableAnimated::IndexMap::const_iterator enBone = indexMap->end();

                size_t matIdx = 0;
                while( itBone != enBone )
                {
                    const SimpleMatrixAf4x3 &mat4x3 = skeleton->_getBoneFullTransform( *itBone );
                    mat4x3.store( &mTempXform[matIdx++] );

                    ++itBone;
                }
            }
        }

        const MaterialPtr &mat = renderable->getMaterial();
        Technique *technique = mat->getBestTechnique( renderable->getCurrentMaterialLod(), renderable );
        const Pass *pass = technique->getPass( 0 );

        mAutoParamDataSource->setCurrentRenderable( renderable );
        mAutoParamDataSource->setWorldMatrices( mTempXform, numMatrices );
        mAutoParamDataSource->setCurrentPass( pass );

        if( pass->getFogOverride() )
        {
            mAutoParamDataSource->setFog( pass->getFogMode(), pass->getFogColour(),
                                          pass->getFogDensity(), pass->getFogStart(),
                                          pass->getFogEnd() );
        }
        else
        {
            mAutoParamDataSource->setFog( mCurrentSceneManager->getFogMode(),
                                          mCurrentSceneManager->getFogColour(),
                                          mCurrentSceneManager->getFogDensity(),
                                          mCurrentSceneManager->getFogStart(),
                                          mCurrentSceneManager->getFogEnd() );
        }

        Pass::ConstTextureUnitStateIterator texIter =  pass->getTextureUnitStateIterator();
        size_t unit = 0;
        while( texIter.hasMoreElements() )
        {
            TextureUnitState *pTex = texIter.getNext();
            if( !casterPass && pTex->getContentType() != TextureUnitState::CONTENT_SHADOW )
            {
                // Manually set texture projector for shaders if present
                // This won't get set any other way if using manual projection
                const TextureUnitState::EffectMap &effectMap = pTex->getEffects();
                TextureUnitState::EffectMap::const_iterator effi = effectMap.find(
                                                TextureUnitState::ET_PROJECTIVE_TEXTURE );
                if( effi != effectMap.end() )
                    mAutoParamDataSource->setTextureProjector( effi->second.frustum, unit );
            }

            if( pTex->getContentType() == TextureUnitState::CONTENT_COMPOSITOR )
            {
                const CompositorTextureVec &compositorTextures = mCurrentSceneManager->
                                                                                getCompositorTextures();
                CompositorTextureVec::const_iterator itor = std::find(
                                                    compositorTextures.begin(), compositorTextures.end(),
                                                    pTex->getReferencedTextureName() );

                if( itor == compositorTextures.end() )
                {
                    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                        "Invalid compositor content_type compositor name '" +
                        pTex->getReferencedTextureName().getFriendlyText() + "'",
                        "HlmsLowLevel::fillBuffersFor" );
                }

                pTex->_setTexturePtr( (*itor->textures)[pTex->getReferencedMRTIndex()] );
            }

            mRenderSystem->_setTextureUnitSettings( unit, *pTex );
            ++unit;
        }

        // Disable remaining texture units
        mRenderSystem->_disableTextureUnitsFrom( pass->getNumTextureUnitStates() );

        pass->_updateAutoParams( mAutoParamDataSource, GPV_ALL );

        if( pass->hasVertexProgram() )
        {
            mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM,
                                                     pass->getVertexProgramParameters(), GPV_ALL );
        }
        if( pass->hasGeometryProgram() )
        {
            mRenderSystem->bindGpuProgramParameters( GPT_GEOMETRY_PROGRAM,
                                                     pass->getGeometryProgramParameters(), GPV_ALL );
        }
        if( pass->hasTessellationHullProgram() )
        {
            mRenderSystem->bindGpuProgramParameters( GPT_HULL_PROGRAM,
                                                     pass->getTessellationHullProgramParameters(),
                                                     GPV_ALL );
        }
        if( pass->hasTessellationDomainProgram() )
        {
            mRenderSystem->bindGpuProgramParameters( GPT_DOMAIN_PROGRAM,
                                                     pass->getTessellationDomainProgramParameters(),
                                                     GPV_ALL );
        }
        if( pass->hasFragmentProgram() )
        {
            mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM,
                                                     pass->getFragmentProgramParameters(), GPV_ALL );
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsLowLevel::createDatablockImpl( IdString datablockName,
                                                      const HlmsMacroblock *macroblock,
                                                      const HlmsBlendblock *blendblock,
                                                      const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsLowLevelDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
