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

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsListener.h"

#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreConstBufferPacked.h"
#include "Vao/OgreTexBufferPacked.h"
#include "Vao/OgreStagingBuffer.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbTexture.h"
#include "CommandBuffer/OgreCbShaderBuffer.h"
#include "OgreUnlitProperty.h"
namespace Ogre
{

    extern const String c_unlitBlendModes[];

    HlmsUnlit::HlmsUnlit( Archive *dataFolder, ArchiveVec *libraryFolders ) :
        HlmsBufferManager( HLMS_UNLIT, "unlit", dataFolder, libraryFolders ),
        ConstBufferPool( HlmsUnlitDatablock::MaterialSizeInGpuAligned,
                         ExtraBufferParams( 64 * NUM_UNLIT_TEXTURE_TYPES ) ),
        mCurrentPassBuffer( 0 ),
        mLastBoundPool( 0 ),
        mLastTextureHash( 0 )
    {
        //Override defaults
        mLightGatheringMode = LightGatherNone;
    }
    HlmsUnlit::HlmsUnlit( Archive *dataFolder, ArchiveVec *libraryFolders,
                          HlmsTypes type, const String &typeName ) :
        HlmsBufferManager(type, typeName, dataFolder, libraryFolders),
        ConstBufferPool(HlmsUnlitDatablock::MaterialSizeInGpuAligned,
        ExtraBufferParams(64 * NUM_UNLIT_TEXTURE_TYPES)),
        mCurrentPassBuffer(0),
        mLastBoundPool(0),
        mLastTextureHash(0)
    {
        //Override defaults
        mLightGatheringMode = LightGatherNone;
    }
    //-----------------------------------------------------------------------------------
    HlmsUnlit::~HlmsUnlit()
    {
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::_changeRenderSystem( RenderSystem *newRs )
    {
        if( mVaoManager )
            destroyAllBuffers();

        ConstBufferPool::_changeRenderSystem( newRs );
        HlmsBufferManager::_changeRenderSystem( newRs );

        if( newRs )
        {
            HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
            HlmsDatablockMap::const_iterator end  = mDatablocks.end();

            while( itor != end )
            {
                assert( dynamic_cast<HlmsUnlitDatablock*>( itor->second.datablock ) );
                HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>( itor->second.datablock );

                requestSlot( datablock->mNumEnabledAnimationMatrices != 0, datablock,
                             datablock->mNumEnabledAnimationMatrices != 0 );
                ++itor;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsUnlit::createShaderCacheEntry( uint32 renderableHash,
                                                        const HlmsCache &passCache,
                                                        uint32 finalHash,
                                                        const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        if( mShaderProfile == "hlsl" )
        {
            mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                                mSetProperties, queuedRenderable );
            return retVal; //D3D embeds the texture slots in the shader.
        }

        //Set samplers.
        assert( dynamic_cast<const HlmsUnlitDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitDatablock *datablock = static_cast<const HlmsUnlitDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();

            int texUnit = 2; //Vertex shader consumes 2 slots with its two tbuffers.

            UnlitBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
            UnlitBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

            if( !getProperty( HlmsBaseProp::ShadowCaster ) )
            {
                int numTextures = 0;
                int numArrayTextures = 0;
                while( itor != end )
                {
                    if( itor->texture->getTextureType() == TEX_TYPE_2D_ARRAY )
                    {
                        psParams->setNamedConstant( "textureMapsArray[" +
                                                    StringConverter::toString( numArrayTextures++ ) +
                                                    "]", texUnit++ );
                    }
                    else
                    {
                        psParams->setNamedConstant( "textureMaps[" +
                                                    StringConverter::toString( numTextures++ ) + "]",
                                                    texUnit++ );
                    }

                    ++itor;
                }
            }
        }

        GpuProgramParametersSharedPtr vsParams = retVal->pso.vertexShader->getDefaultParameters();
        vsParams->setNamedConstant( "worldMatBuf", 0 );
        if( datablock->mNumEnabledAnimationMatrices )
            vsParams->setNamedConstant( "animationMatrixBuf", 1 );

        mListener->shaderCacheEntryCreated( mShaderProfile, retVal, passCache,
                                            mSetProperties, queuedRenderable );

        mRenderSystem->_setPipelineStateObject( &retVal->pso );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vsParams, GPV_ALL );
        if( !retVal->pso.pixelShader.isNull() )
        {
            GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();
            mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, GPV_ALL );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::setTextureProperty( IdString propertyName, HlmsUnlitDatablock *datablock,
                                        uint8 texType )
    {
        uint8 idx = datablock->getBakedTextureIdx( texType );
        if( idx != NUM_UNLIT_TEXTURE_TYPES )
        {
            //In the template the we subtract the "+1" for the index.
            //We need to increment it now otherwise @property( diffuse_map )
            //can translate to @property( 0 ) which is not what we want.
            setProperty( propertyName, idx + 1 );
        }
    }
    //-----------------------------------------------------------------------------------
    struct UvOutput
    {
        int32 uvSource;
        int32 texUnit;
        bool isAnimated;
    };
    typedef vector<UvOutput>::type UvOutputVec;
    void HlmsUnlit::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces )
    {
        assert( dynamic_cast<HlmsUnlitDatablock*>( renderable->getDatablock() ) );
        HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(
                                                        renderable->getDatablock() );

        setProperty( HlmsBaseProp::Skeleton,    0 );
        setProperty( HlmsBaseProp::Normal,      0 );
        setProperty( HlmsBaseProp::QTangent,    0 );
        setProperty( HlmsBaseProp::Tangent,     0 );
        setProperty( HlmsBaseProp::BonesPerVertex, 0 );

        int texUnit = 2; //Vertex shader consumes 2 slots with its two tbuffers.
        int numTextures = 0;
        int numArrayTextures = 0;

        {
            UnlitBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
            UnlitBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

            while( itor != end )
            {
                if( itor->texture->getTextureType() == TEX_TYPE_2D_ARRAY )
                {
                    setProperty( "array_texture_bind" + StringConverter::toString( numArrayTextures ),
                                 texUnit );
                    ++numArrayTextures;
                }
                else
                {
                    setProperty( "texture_bind" + StringConverter::toString( numArrayTextures ),
                                 texUnit );
                    ++numTextures;
                }

                ++texUnit;
                ++itor;
            }
        }

        setProperty( UnlitProperty::Diffuse, datablock->mHasColour );

        setProperty( UnlitProperty::NumArrayTextures, numArrayTextures );
        setProperty( UnlitProperty::NumTextures, numTextures );

        setProperty( UnlitProperty::DiffuseMap,
                     datablock->mBakedTextures.empty() ? 0 : NUM_UNLIT_TEXTURE_TYPES );

        UvOutputVec uvOutputs;

        for( uint8 i=0; i<NUM_UNLIT_TEXTURE_TYPES; ++i )
        {
            //Set whether the texture is used.
            String diffuseMapNStr = "diffuse_map" + StringConverter::toString( i );
            IdString diffuseMapN( diffuseMapNStr );
            setTextureProperty( diffuseMapN, datablock, i );

            TexturePtr texture = datablock->getTexture( i );

            //Sanity check.
            bool hasTexture = !texture.isNull();
            if( hasTexture && getProperty( *HlmsBaseProp::UvCountPtrs[datablock->mUvSource[i]] ) < 2 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Renderable must have at least 2 coordinates in UV set #" +
                             StringConverter::toString( datablock->mUvSource[i] ) +
                             ". Either change the mesh, or change the UV source settings",
                             "HlmsUnlit::calculateHashForPreCreate" );
            }

            if( !texture.isNull() && texture->getTextureType() == TEX_TYPE_2D_ARRAY )
            {
                IdString diffuseMapNArray( diffuseMapNStr + "_array" );
                setProperty( diffuseMapNArray, 1 );
            }

            //Set the blend mode
            uint8 blendMode = datablock->mBlendModes[i];
            inOutPieces[PixelShader][*UnlitProperty::DiffuseMapPtrs[i].blendModeIndex] =
                                "@insertpiece( " + c_unlitBlendModes[blendMode] + ")";

            //Match the texture unit to the UV output.
            if( hasTexture )
            {
                const IdString &uvSourceSwizzleN = *UnlitProperty::DiffuseMapPtrs[i].uvSourceSwizzle;

                if( datablock->mEnabledAnimationMatrices[i] )
                {
                    //Animated outputs need their own entry
                    UvOutput uvOutput;
                    uvOutput.uvSource   = datablock->mUvSource[i];
                    uvOutput.texUnit    = i;
                    uvOutput.isAnimated = true;

                    setProperty( *UnlitProperty::DiffuseMapPtrs[i].uvSource,
                                 static_cast<int32>( uvOutputs.size() >> 1u ) );
                    inOutPieces[PixelShader][uvSourceSwizzleN] = uvOutputs.size() % 2 ? "zw" : "xy";

                    uvOutputs.push_back( uvOutput );
                }
                else
                {
                    //Non-animated outputs may share their entry with another non-animated one.
                    UvOutputVec::iterator itor = uvOutputs.begin();
                    UvOutputVec::iterator end  = uvOutputs.end();

                    while( itor != end &&
                           (itor->uvSource != datablock->mUvSource[i] || itor->isAnimated) )
                    {
                        ++itor;
                    }

                    size_t rawIdx = itor - uvOutputs.begin();
                    int32 idx = static_cast<int32>( rawIdx >> 1u );
                    setProperty( *UnlitProperty::DiffuseMapPtrs[i].uvSource, idx );
                    inOutPieces[PixelShader][uvSourceSwizzleN] = rawIdx % 2 ? "zw" : "xy";

                    if( itor == end )
                    {
                        //Entry didn't exist yet.
                        UvOutput uvOutput;
                        uvOutput.uvSource   = datablock->mUvSource[i];
                        uvOutput.texUnit    = 0; //Not used
                        uvOutput.isAnimated = false;

                        uvOutputs.push_back( uvOutput );
                    }
                }

                //Generate the texture swizzle for the pixel shader.
                IdString diffuseMapNTexSwizzle( diffuseMapNStr + "_tex_swizzle" );
                String texSwizzle;
                texSwizzle.reserve( 4 );

                for( size_t j=0; j<4; ++j )
                {
                    const size_t swizzleMask = (datablock->mTextureSwizzles[i] >> (6u - j*2u)) & 0x03u;
                    if( swizzleMask == HlmsUnlitDatablock::R_MASK )
                        texSwizzle += "x";
                    else if( swizzleMask == HlmsUnlitDatablock::G_MASK )
                        texSwizzle += "y";
                    else if( swizzleMask == HlmsUnlitDatablock::B_MASK )
                        texSwizzle += "z";
                    else if( swizzleMask == HlmsUnlitDatablock::A_MASK )
                        texSwizzle += "w";
                }
                inOutPieces[PixelShader][diffuseMapNTexSwizzle] = texSwizzle;
            }
        }

        size_t halfUvOutputs = (uvOutputs.size() + 1u) >> 1u;
        setProperty( UnlitProperty::OutUvCount, static_cast<int32>( uvOutputs.size() ) );
        setProperty( UnlitProperty::OutUvHalfCount, static_cast<int32>( halfUvOutputs ) );

        for( size_t i=0; i<halfUvOutputs; ++i )
        {
            //Decide whether to use vec4 or vec2 in VStoPS_block piece:
            // vec4 uv0; //--> When interpolant contains two uvs in one
            // vec2 uv0; //--> When interpolant contains the last UV (uvOutputs.size() is odd)
            setProperty( "out_uv_half_count" + StringConverter::toString( i ),
                         (i << 1u) == (uvOutputs.size() - 1u) ? 2 : 4 );
        }

        for( size_t i=0; i<uvOutputs.size(); ++i )
        {
            String outPrefix = "out_uv" + StringConverter::toString( i );

            setProperty( outPrefix + "_out_uv", i >> 1u );
            setProperty( outPrefix + "_texture_matrix", uvOutputs[i].isAnimated );
            setProperty( outPrefix + "_tex_unit", uvOutputs[i].texUnit );
            setProperty( outPrefix + "_source_uv", uvOutputs[i].uvSource );
            inOutPieces[VertexShader][outPrefix + "_swizzle"] = i % 2 ? "zw" : "xy";
        }

        String slotsPerPoolStr = StringConverter::toString( mSlotsPerPool );
        inOutPieces[VertexShader][UnlitProperty::MaterialsPerBuffer] = slotsPerPoolStr;
        inOutPieces[PixelShader][UnlitProperty::MaterialsPerBuffer]  = slotsPerPoolStr;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces )
    {
        //HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(
        //                                              renderable->getDatablock() );

        HlmsPropertyVec::iterator itor = mSetProperties.begin();
        HlmsPropertyVec::iterator end  = mSetProperties.end();

        while( itor != end )
        {
            if( itor->keyName != UnlitProperty::HwGammaRead &&
                     //itor->keyName != UnlitProperty::UvDiffuse &&
                     itor->keyName != HlmsBaseProp::Skeleton &&
                     itor->keyName != HlmsBaseProp::BonesPerVertex &&
                     itor->keyName != HlmsBaseProp::DualParaboloidMapping &&
                     itor->keyName != HlmsBaseProp::AlphaTest &&
                     itor->keyName != HlmsBaseProp::AlphaBlend )
            {
                itor = mSetProperties.erase( itor );
                end  = mSetProperties.end();
            }
            else
            {
                ++itor;
            }
        }

        String slotsPerPoolStr = StringConverter::toString( mSlotsPerPool );
        inOutPieces[VertexShader][UnlitProperty::MaterialsPerBuffer] = slotsPerPoolStr;
        inOutPieces[PixelShader][UnlitProperty::MaterialsPerBuffer] = slotsPerPoolStr;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsUnlit::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager )
    {
        mSetProperties.clear();

        //Set the properties and create/retrieve the cache.
        if( casterPass )
            setProperty( HlmsBaseProp::ShadowCaster, 1 );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
        setProperty( HlmsBaseProp::ShadowUsesDepthTexture,
                     renderTarget->getForceDisableColourWrites() ? 1 : 0 );
        setProperty( HlmsBaseProp::RenderDepthOnly,
                     renderTarget->getForceDisableColourWrites() ? 1 : 0 );

        mListener->preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager, this );

        PassCache passCache;
        passCache.passPso = getPassPsoForScene( sceneManager );
        passCache.properties = mSetProperties;

        assert( mPassCache.size() <= (size_t)HlmsBits::PassMask &&
                "Too many passes combinations, we'll overflow the bits assigned in the hash!" );
        PassCacheVec::iterator it = std::find( mPassCache.begin(), mPassCache.end(), passCache );
        if( it == mPassCache.end() )
        {
            mPassCache.push_back( passCache );
            it = mPassCache.end() - 1;
        }

        const uint32 hash = (it - mPassCache.begin()) << HlmsBits::PassShift;

        //Fill the buffers
        HlmsCache retVal( hash, mType, HlmsPso() );
        retVal.setProperties = mSetProperties;
        retVal.pso.pass = passCache.passPso;

        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();
        Matrix4 identityProjMat;

        mRenderSystem->_convertProjectionMatrix( Matrix4::IDENTITY,
                                                 identityProjMat, true );

        if( renderTarget->requiresTextureFlipping() )
        {
            projectionMatrix[1][0]  = -projectionMatrix[1][0];
            projectionMatrix[1][1]  = -projectionMatrix[1][1];
            projectionMatrix[1][2]  = -projectionMatrix[1][2];
            projectionMatrix[1][3]  = -projectionMatrix[1][3];

            identityProjMat[1][0]   = -identityProjMat[1][0];
            identityProjMat[1][1]   = -identityProjMat[1][1];
            identityProjMat[1][2]   = -identityProjMat[1][2];
            identityProjMat[1][3]   = -identityProjMat[1][3];
        }

        mPreparedPass.viewProjMatrix[0] = projectionMatrix * viewMatrix;
        mPreparedPass.viewProjMatrix[1] = identityProjMat;

        mSetProperties.clear();

        //mat4 viewProj[2];
        size_t mapSize = (16 + 16) * 4;

        if( casterPass )
        {
            //vec2 depthRange; (+padding)
            mapSize += 4 * 4;
        }

        mapSize += mListener->getPassBufferSize( shadowNode, casterPass,
                                                 dualParaboloid, sceneManager );

        //Arbitrary 2kb (minimum supported by GL is 64kb), should be enough.
        const size_t maxBufferSize = 2 * 1024;
        assert( mapSize <= maxBufferSize );

        if( mCurrentPassBuffer >= mPassBuffers.size() )
        {
            mPassBuffers.push_back( mVaoManager->createConstBuffer( maxBufferSize,
                                                                    BT_DYNAMIC_PERSISTENT,
                                                                    0, false ) );
        }

        ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer++];
        float *passBufferPtr = reinterpret_cast<float*>( passBuffer->map( 0, mapSize ) );

#ifndef NDEBUG
        const float *startupPtr = passBufferPtr;
#endif
        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------

        //mat4 viewProj[0];
        Matrix4 tmp = mPreparedPass.viewProjMatrix[0].transpose();
        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)tmp[0][i];

        //mat4 viewProj[1] (identityProj);
        tmp = mPreparedPass.viewProjMatrix[1].transpose();
        for( size_t i=0; i<16; ++i )
            *passBufferPtr++ = (float)tmp[0][i];

        if( casterPass )
        {
            //vec2 depthRange;
            Real fNear, fFar;
            shadowNode->getMinMaxDepthRange( camera, fNear, fFar );
            const Real depthRange = fFar - fNear;
            *passBufferPtr++ = fNear;
            *passBufferPtr++ = 1.0f / depthRange;
            passBufferPtr += 2;
        }

        passBufferPtr = mListener->preparePassBuffer( shadowNode, casterPass, dualParaboloid,
                                                      sceneManager, passBufferPtr );

        assert( (size_t)(passBufferPtr - startupPtr) * 4u == mapSize );

        passBuffer->unmap( UO_KEEP_PERSISTENT );

        //mTexBuffers must hold at least one buffer to prevent out of bound exceptions.
        if( mTexBuffers.empty() )
        {
            size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                  mVaoManager->getTexBufferMaxSize() );
            TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                       BT_DYNAMIC_PERSISTENT, 0, false );
            mTexBuffers.push_back( newBuffer );
        }

        mLastTextureHash = 0;
        mLastBoundPool = 0;

        uploadDirtyDatablocks();

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlit::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      uint32 lastTextureHash )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Trying to use slow-path on a desktop implementation. "
                     "Change the RenderQueue settings.",
                     "HlmsUnlit::fillBuffersFor" );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlit::fillBuffersForV1( const HlmsCache *cache,
                                        const QueuedRenderable &queuedRenderable,
                                        bool casterPass, uint32 lastCacheHash,
                                        CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, true );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlit::fillBuffersForV2( const HlmsCache *cache,
                                        const QueuedRenderable &queuedRenderable,
                                        bool casterPass, uint32 lastCacheHash,
                                        CommandBuffer *commandBuffer )
    {
        return fillBuffersFor( cache, queuedRenderable, casterPass,
                               lastCacheHash, commandBuffer, false );
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlit::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                      bool casterPass, uint32 lastCacheHash,
                                      CommandBuffer *commandBuffer, bool isV1 )
    {
        assert( dynamic_cast<const HlmsUnlitDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitDatablock *datablock = static_cast<const HlmsUnlitDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        if( OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH( lastCacheHash ) != HLMS_UNLIT )
        {
            //We changed HlmsType, rebind the shared textures.
            mLastTextureHash = 0;
            mLastBoundPool = 0;

            //layout(binding = 0) uniform PassBuffer {} pass
            ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader,
                                                                           0, passBuffer, 0,
                                                                           passBuffer->
                                                                           getTotalSizeBytes() );

            //layout(binding = 2) uniform InstanceBuffer {} instance
            if( mCurrentConstBuffer < mConstBuffers.size() &&
                (size_t)((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <=
                    mCurrentConstBufferSize )
            {
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( VertexShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
                *commandBuffer->addCommand<CbShaderBuffer>() =
                        CbShaderBuffer( PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0 );
            }

            rebindTexBuffer( commandBuffer );

            mListener->hlmsTypeChanged( casterPass, commandBuffer, datablock );
        }

        //Don't bind the material buffer on caster passes (important to keep
        //MDI & auto-instancing running on shadow map passes)
        if( mLastBoundPool != datablock->getAssignedPool() && !casterPass )
        {
            //layout(binding = 1) uniform MaterialBuf {} materialArray
            const ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( PixelShader,
                                                                           1, newPool->materialBuffer, 0,
                                                                           newPool->materialBuffer->
                                                                           getTotalSizeBytes() );
            if( newPool->extraBuffer )
            {
                TexBufferPacked *extraBuffer = static_cast<TexBufferPacked*>( newPool->extraBuffer );
                *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader, 1,
                                                                               extraBuffer, 0,
                                                                               extraBuffer->
                                                                               getTotalSizeBytes() );
            }

            mLastBoundPool = newPool;
        }

        uint32 * RESTRICT_ALIAS currentMappedConstBuffer    = mCurrentMappedConstBuffer;
        float * RESTRICT_ALIAS currentMappedTexBuffer       = mCurrentMappedTexBuffer;

        const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();

        bool exceedsConstBuffer = (size_t)((currentMappedConstBuffer - mStartMappedConstBuffer) + 4) >
                                                                                mCurrentConstBufferSize;

        const size_t minimumTexBufferSize = 16;
        bool exceedsTexBuffer = (currentMappedTexBuffer - mStartMappedTexBuffer) +
                                     minimumTexBufferSize >= mCurrentTexBufferSize;

        if( exceedsConstBuffer || exceedsTexBuffer )
        {
            currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );

            if( exceedsTexBuffer )
                mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
            else
                rebindTexBuffer( commandBuffer, true, minimumTexBufferSize * sizeof(float) );

            currentMappedTexBuffer = mCurrentMappedTexBuffer;
        }

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------
        bool useIdentityProjection = queuedRenderable.renderable->getUseIdentityProjection();

#if !OGRE_DOUBLE_PRECISION
        //uint materialIdx[]
        *currentMappedConstBuffer = datablock->getAssignedSlot();
        *reinterpret_cast<float * RESTRICT_ALIAS>( currentMappedConstBuffer+1 ) = datablock->
                                                                                    mShadowConstantBias;
        *(currentMappedConstBuffer+2) = useIdentityProjection;
        currentMappedConstBuffer += 4;

        //mat4 worldViewProj
        Matrix4 tmp = mPreparedPass.viewProjMatrix[useIdentityProjection] * worldMat;
        memcpy( currentMappedTexBuffer, &tmp, sizeof(Matrix4) );
        currentMappedTexBuffer += 16;
#else
    #error Not Coded Yet! (cannot use memcpy on Matrix4)
#endif

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------

        if( !casterPass )
        {
            if( datablock->mTextureHash != mLastTextureHash )
            {
                //Rebind textures
                size_t texUnit = 2;

                UnlitBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
                UnlitBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

                while( itor != end )
                {
                    *commandBuffer->addCommand<CbTexture>() =
                            CbTexture( texUnit++, true, itor->texture.get(), itor->samplerBlock );
                    ++itor;
                }

                *commandBuffer->addCommand<CbTextureDisableFrom>() = CbTextureDisableFrom( texUnit );

                mLastTextureHash = datablock->mTextureHash;
            }
        }

        mCurrentMappedConstBuffer   = currentMappedConstBuffer;
        mCurrentMappedTexBuffer     = currentMappedTexBuffer;

        return ((mCurrentMappedConstBuffer - mStartMappedConstBuffer) >> 2) - 1;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::destroyAllBuffers(void)
    {
        HlmsBufferManager::destroyAllBuffers();

        mCurrentPassBuffer  = 0;

        {
            ConstBufferPackedVec::const_iterator itor = mPassBuffers.begin();
            ConstBufferPackedVec::const_iterator end  = mPassBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyConstBuffer( *itor );
                ++itor;
            }

            mPassBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::frameEnded(void)
    {
        HlmsBufferManager::frameEnded();
        mCurrentPassBuffer  = 0;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsUnlit::createDatablockImpl( IdString datablockName,
                                                       const HlmsMacroblock *macroblock,
                                                       const HlmsBlendblock *blendblock,
                                                       const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsUnlitDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
