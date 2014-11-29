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

namespace Ogre
{
    const IdString UnlitProperty::HwGammaRead       = IdString( "hw_gamma_read" );
    const IdString UnlitProperty::HwGammaWrite      = IdString( "hw_gamma_write" );
    const IdString UnlitProperty::SignedIntTex      = IdString( "signed_int_textures" );
    const IdString UnlitProperty::MaterialsPerBuffer= IdString( "materials_per_buffer" );
    const IdString UnlitProperty::AnimationMatricesPerBuffer = IdString( "animation_matrices_per_buffer" );

    const IdString UnlitProperty::TexMatrixCount        = IdString( "hlms_texture_matrix_count" );
    const IdString UnlitProperty::TexMatrixCount0       = IdString( "hlms_texture_matrix_count0" );
    const IdString UnlitProperty::TexMatrixCount1       = IdString( "hlms_texture_matrix_count1" );
    const IdString UnlitProperty::TexMatrixCount2       = IdString( "hlms_texture_matrix_count2" );
    const IdString UnlitProperty::TexMatrixCount3       = IdString( "hlms_texture_matrix_count3" );
    const IdString UnlitProperty::TexMatrixCount4       = IdString( "hlms_texture_matrix_count4" );
    const IdString UnlitProperty::TexMatrixCount5       = IdString( "hlms_texture_matrix_count5" );
    const IdString UnlitProperty::TexMatrixCount6       = IdString( "hlms_texture_matrix_count6" );
    const IdString UnlitProperty::TexMatrixCount7       = IdString( "hlms_texture_matrix_count7" );

    const IdString UnlitProperty::Diffuse               = IdString( "diffuse" );

    const IdString UnlitProperty::NumTextures           = IdString( "num_textures" );

    const IdString UnlitProperty::DiffuseMap            = IdString( "diffuse_map" );
    //const IdString UnlitProperty::DiffuseMap0           = IdString( "diffuse_map0" );

    const IdString UnlitProperty::UvDiffuse0            = IdString( "uv_diffuse0" );
    const IdString UnlitProperty::UvDiffuse1            = IdString( "uv_diffuse1" );
    const IdString UnlitProperty::UvDiffuse2            = IdString( "uv_diffuse2" );
    const IdString UnlitProperty::UvDiffuse3            = IdString( "uv_diffuse3" );
    const IdString UnlitProperty::UvDiffuse4            = IdString( "uv_diffuse4" );
    const IdString UnlitProperty::UvDiffuse5            = IdString( "uv_diffuse5" );
    const IdString UnlitProperty::UvDiffuse6            = IdString( "uv_diffuse6" );
    const IdString UnlitProperty::UvDiffuse7            = IdString( "uv_diffuse7" );
    const IdString UnlitProperty::UvDiffuse8            = IdString( "uv_diffuse8" );
    const IdString UnlitProperty::UvDiffuse9            = IdString( "uv_diffuse9" );
    const IdString UnlitProperty::UvDiffuse10           = IdString( "uv_diffuse10" );
    const IdString UnlitProperty::UvDiffuse11           = IdString( "uv_diffuse11" );
    const IdString UnlitProperty::UvDiffuse12           = IdString( "uv_diffuse12" );
    const IdString UnlitProperty::UvDiffuse13           = IdString( "uv_diffuse13" );
    const IdString UnlitProperty::UvDiffuse14           = IdString( "uv_diffuse14" );
    const IdString UnlitProperty::UvDiffuse15           = IdString( "uv_diffuse15" );

    const IdString UnlitProperty::UvDiffuseSwizzle0     = IdString( "uv_diffuse_swizzle0" );
    const IdString UnlitProperty::UvDiffuseSwizzle1     = IdString( "uv_diffuse_swizzle1" );
    const IdString UnlitProperty::UvDiffuseSwizzle2     = IdString( "uv_diffuse_swizzle2" );
    const IdString UnlitProperty::UvDiffuseSwizzle3     = IdString( "uv_diffuse_swizzle3" );
    const IdString UnlitProperty::UvDiffuseSwizzle4     = IdString( "uv_diffuse_swizzle4" );
    const IdString UnlitProperty::UvDiffuseSwizzle5     = IdString( "uv_diffuse_swizzle5" );
    const IdString UnlitProperty::UvDiffuseSwizzle6     = IdString( "uv_diffuse_swizzle6" );
    const IdString UnlitProperty::UvDiffuseSwizzle7     = IdString( "uv_diffuse_swizzle7" );
    const IdString UnlitProperty::UvDiffuseSwizzle8     = IdString( "uv_diffuse_swizzle8" );
    const IdString UnlitProperty::UvDiffuseSwizzle9     = IdString( "uv_diffuse_swizzle9" );
    const IdString UnlitProperty::UvDiffuseSwizzle10    = IdString( "uv_diffuse_swizzle10" );
    const IdString UnlitProperty::UvDiffuseSwizzle11    = IdString( "uv_diffuse_swizzle11" );
    const IdString UnlitProperty::UvDiffuseSwizzle12    = IdString( "uv_diffuse_swizzle12" );
    const IdString UnlitProperty::UvDiffuseSwizzle13    = IdString( "uv_diffuse_swizzle13" );
    const IdString UnlitProperty::UvDiffuseSwizzle14    = IdString( "uv_diffuse_swizzle14" );
    const IdString UnlitProperty::UvDiffuseSwizzle15    = IdString( "uv_diffuse_swizzle15" );

    const IdString UnlitProperty::BlendModeIndex0       = IdString( "blend_mode_idx0" );
    const IdString UnlitProperty::BlendModeIndex1       = IdString( "blend_mode_idx1" );
    const IdString UnlitProperty::BlendModeIndex2       = IdString( "blend_mode_idx2" );
    const IdString UnlitProperty::BlendModeIndex3       = IdString( "blend_mode_idx3" );
    const IdString UnlitProperty::BlendModeIndex4       = IdString( "blend_mode_idx4" );
    const IdString UnlitProperty::BlendModeIndex5       = IdString( "blend_mode_idx5" );
    const IdString UnlitProperty::BlendModeIndex6       = IdString( "blend_mode_idx6" );
    const IdString UnlitProperty::BlendModeIndex7       = IdString( "blend_mode_idx7" );
    const IdString UnlitProperty::BlendModeIndex8       = IdString( "blend_mode_idx8" );
    const IdString UnlitProperty::BlendModeIndex9       = IdString( "blend_mode_idx9" );
    const IdString UnlitProperty::BlendModeIndex10      = IdString( "blend_mode_idx10" );
    const IdString UnlitProperty::BlendModeIndex11      = IdString( "blend_mode_idx11" );
    const IdString UnlitProperty::BlendModeIndex12      = IdString( "blend_mode_idx12" );
    const IdString UnlitProperty::BlendModeIndex13      = IdString( "blend_mode_idx13" );
    const IdString UnlitProperty::BlendModeIndex14      = IdString( "blend_mode_idx14" );
    const IdString UnlitProperty::BlendModeIndex15      = IdString( "blend_mode_idx15" );

    const IdString UnlitProperty::OutUvCount            = IdString( "out_uv_count" );
    /*const IdString UnlitProperty::OutUv0TextureMatrix   = IdString( "out_uv0_out_uv" );
    const IdString UnlitProperty::OutUv0TextureMatrix   = IdString( "out_uv0_texture_matrix" );
    const IdString UnlitProperty::OutUv0TexUnit         = IdString( "out_uv0_tex_unit" );
    const IdString UnlitProperty::OutUv0Swizzle         = IdString( "out_uv0_swizzle" );
    const IdString UnlitProperty::OutUv0SourceUv        = IdString( "out_uv0_source_uv" );*/

    extern const String c_unlitBlendModes[];

    const UnlitProperty::DiffuseMapPtr UnlitProperty::DiffuseMapPtrs[NUM_UNLIT_TEXTURE_TYPES] =
    {
        { &UnlitProperty::UvDiffuse0, &UnlitProperty::UvDiffuseSwizzle0, &UnlitProperty::BlendModeIndex0 },
        { &UnlitProperty::UvDiffuse1, &UnlitProperty::UvDiffuseSwizzle1, &UnlitProperty::BlendModeIndex1 },
        { &UnlitProperty::UvDiffuse2, &UnlitProperty::UvDiffuseSwizzle2, &UnlitProperty::BlendModeIndex2 },
        { &UnlitProperty::UvDiffuse3, &UnlitProperty::UvDiffuseSwizzle3, &UnlitProperty::BlendModeIndex3 },
        { &UnlitProperty::UvDiffuse4, &UnlitProperty::UvDiffuseSwizzle4, &UnlitProperty::BlendModeIndex4 },
        { &UnlitProperty::UvDiffuse5, &UnlitProperty::UvDiffuseSwizzle5, &UnlitProperty::BlendModeIndex5 },
        { &UnlitProperty::UvDiffuse6, &UnlitProperty::UvDiffuseSwizzle6, &UnlitProperty::BlendModeIndex6 },
        { &UnlitProperty::UvDiffuse7, &UnlitProperty::UvDiffuseSwizzle7, &UnlitProperty::BlendModeIndex7 },
        { &UnlitProperty::UvDiffuse8, &UnlitProperty::UvDiffuseSwizzle8, &UnlitProperty::BlendModeIndex8 },
        { &UnlitProperty::UvDiffuse9, &UnlitProperty::UvDiffuseSwizzle9, &UnlitProperty::BlendModeIndex9 },
        { &UnlitProperty::UvDiffuse10, &UnlitProperty::UvDiffuseSwizzle10, &UnlitProperty::BlendModeIndex10 },
        { &UnlitProperty::UvDiffuse11, &UnlitProperty::UvDiffuseSwizzle11, &UnlitProperty::BlendModeIndex11 },
        { &UnlitProperty::UvDiffuse12, &UnlitProperty::UvDiffuseSwizzle12, &UnlitProperty::BlendModeIndex12 },
        { &UnlitProperty::UvDiffuse13, &UnlitProperty::UvDiffuseSwizzle13, &UnlitProperty::BlendModeIndex13 },
        { &UnlitProperty::UvDiffuse14, &UnlitProperty::UvDiffuseSwizzle14, &UnlitProperty::BlendModeIndex14 },
        { &UnlitProperty::UvDiffuse15, &UnlitProperty::UvDiffuseSwizzle15, &UnlitProperty::BlendModeIndex15 },
    };

    HlmsUnlit::HlmsUnlit( Archive *dataFolder ) :
        Hlms( HLMS_UNLIT, "unlit", dataFolder ),
        ConstBufferPool( HlmsUnlitDatablock::MaterialSizeInGpuAligned,
                         ExtraBufferParams( 64 * NUM_UNLIT_TEXTURE_TYPES ) ),
        mCurrentConstBuffer( 0 ),
        mCurrentTexBuffer( 0 ),
        mLastBoundPool( 0 ),
        mStartMappedConstBuffer( 0 ),
        mCurrentMappedConstBuffer( 0 ),
        mCurrentConstBufferSize( 0 ),
        mRealStartMappedTexBuffer( 0 ),
        mStartMappedTexBuffer( 0 ),
        mCurrentMappedTexBuffer( 0 ),
        mCurrentTexBufferSize( 0 ),
        mTexLastOffset( 0 ),
        mLastTexBufferCmdOffset( (size_t)~0 ),
        mLastTextureHash( 0 ),
        mTextureBufferDefaultSize( 4 * 1024 * 1024 )
    {
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
        Hlms::_changeRenderSystem( newRs );

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

        //Set samplers.
        GpuProgramParametersSharedPtr vsParams = retVal->vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = retVal->pixelShader->getDefaultParameters();

        int texUnit = 2; //Vertex shader consumes 2 slots with its two tbuffers.

        assert( dynamic_cast<const HlmsUnlitDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitDatablock *datablock = static_cast<const HlmsUnlitDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        int numTextures = getProperty( UnlitProperty::NumTextures );
        for( int i=0; i<numTextures; ++i )
        {
            psParams->setNamedConstant( "textureMaps[" + StringConverter::toString( i ) + "]",
                                        texUnit++ );
        }

        vsParams->setNamedConstant( "worldMatBuf", 0 );
        if( datablock->mNumEnabledAnimationMatrices )
            vsParams->setNamedConstant( "animationMatrixBuf", 1 );

        mRenderSystem->_setProgramsFromHlms( retVal );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vsParams, GPV_ALL );
        mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, GPV_ALL );

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

        setProperty( UnlitProperty::NumTextures, datablock->mBakedTextures.size() );

        setProperty( UnlitProperty::DiffuseMap,
                     datablock->mBakedTextures.empty() ? 0 : NUM_UNLIT_TEXTURE_TYPES );

        UvOutputVec uvOutputs;

        for( uint8 i=0; i<NUM_UNLIT_TEXTURE_TYPES; ++i )
        {
            //Set whether the texture is used.
            IdString diffuseMapN( "diffuse_map" + StringConverter::toString( i ) );
            setTextureProperty( diffuseMapN, datablock, i );

            //Sanity check.
            bool hasTexture = !datablock->getTexture( i ).isNull();
            if( hasTexture && getProperty( *HlmsBaseProp::UvCountPtrs[datablock->mUvSource[i]] ) < 2 )
            {
                OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                             "Renderable must have at least 2 coordinates in UV set #" +
                             StringConverter::toString( datablock->mUvSource[i] ) +
                             ". Either change the mesh, or change the UV source settings",
                             "HlmsUnlit::calculateHashForPreCreate" );
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
                                 static_cast<int32>( uvOutputs.size() ) );
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

                    int32 idx = static_cast<int32>( itor - uvOutputs.begin() );
                    setProperty( *UnlitProperty::DiffuseMapPtrs[i].uvSource, idx );
                    inOutPieces[PixelShader][uvSourceSwizzleN] = idx % 2 ? "zw" : "xy";

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
            }
        }

        setProperty( UnlitProperty::OutUvCount, static_cast<int32>( uvOutputs.size() ) );

        for( size_t i=0; i<uvOutputs.size(); ++i )
        {
            String outPrefix = "out_uv" + StringConverter::toString( i );

            setProperty( outPrefix + "_out_uv", i >> 1 );
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
    HlmsCache HlmsUnlit::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager )
    {
        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
        if( renderTarget->requiresTextureFlipping() )
        {
            projectionMatrix[1][0] = -projectionMatrix[1][0];
            projectionMatrix[1][1] = -projectionMatrix[1][1];
            projectionMatrix[1][2] = -projectionMatrix[1][2];
            projectionMatrix[1][3] = -projectionMatrix[1][3];
        }

        mPreparedPass.viewProjMatrix[0]     = projectionMatrix * viewMatrix;
        mPreparedPass.viewProjMatrix[1]     = Matrix4::IDENTITY;

        mSetProperties.clear();

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

        return HlmsCache( 0, HLMS_UNLIT );
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
            rebindTexBuffer( commandBuffer );
        }

        if( mLastBoundPool != datablock->getAssignedPool() )
        {
            //layout(binding = 1) uniform MaterialBuf {} materialArray
            const ConstBufferPool::BufferPool *newPool = datablock->getAssignedPool();
            *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( 1, newPool->materialBuffer, 0,
                                                                           newPool->materialBuffer->
                                                                           getTotalSizeBytes() );
            if( newPool->extraBuffer )
            {
                TexBufferPacked *extraBuffer = static_cast<TexBufferPacked*>( newPool->extraBuffer );
                *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( 1, extraBuffer, 0,
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
    void HlmsUnlit::unmapConstBuffer(void)
    {
        if( mStartMappedConstBuffer )
        {
            //Unmap the current buffer
            ConstBufferPacked *constBuffer = mConstBuffers[mCurrentConstBuffer];
            constBuffer->unmap( UO_KEEP_PERSISTENT, 0,
                                (mCurrentMappedConstBuffer - mStartMappedConstBuffer) * sizeof(uint32) );

            ++mCurrentConstBuffer;

            mStartMappedConstBuffer     = 0;
            mCurrentMappedConstBuffer   = 0;
            mCurrentConstBufferSize     = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    DECL_MALLOC uint32* HlmsUnlit::mapNextConstBuffer( CommandBuffer *commandBuffer )
    {
        unmapConstBuffer();

        if( mCurrentConstBuffer >= mConstBuffers.size() )
        {
            size_t bufferSize = std::min<size_t>( 65535, mVaoManager->getConstBufferMaxSize() );
            ConstBufferPacked *newBuffer = mVaoManager->createConstBuffer( bufferSize,
                                                                           BT_DYNAMIC_PERSISTENT,
                                                                           0, false );
            mConstBuffers.push_back( newBuffer );
        }

        ConstBufferPacked *constBuffer = mConstBuffers[mCurrentConstBuffer];

        mStartMappedConstBuffer     = reinterpret_cast<uint32*>(
                                            constBuffer->map( 0, constBuffer->getNumElements() ) );
        mCurrentMappedConstBuffer   = mStartMappedConstBuffer;
        mCurrentConstBufferSize     = constBuffer->getNumElements() >> 2;

        *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( 2, constBuffer, 0, 0 );

        return mStartMappedConstBuffer;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::unmapTexBuffer( CommandBuffer *commandBuffer )
    {
        //Save our progress
        mTexLastOffset = (mCurrentMappedTexBuffer - mStartMappedTexBuffer) * sizeof(float);

        if( mStartMappedTexBuffer )
        {
            //Unmap the current buffer
            TexBufferPacked *texBuffer = mTexBuffers[mCurrentTexBuffer];
            texBuffer->unmap( UO_KEEP_PERSISTENT, 0, mTexLastOffset );

            CbShaderBuffer *shaderBufferCmd = reinterpret_cast<CbShaderBuffer*>(
                        commandBuffer->getCommandFromOffset( mLastTexBufferCmdOffset ) );
            if( shaderBufferCmd )
            {
                assert( shaderBufferCmd->bufferPacked == texBuffer );
                shaderBufferCmd->bindSizeBytes = mTexLastOffset;
                mLastTexBufferCmdOffset = (size_t)~0;
            }
        }

        mRealStartMappedTexBuffer = 0;
        mStartMappedTexBuffer   = 0;
        mCurrentMappedTexBuffer = 0;
        mCurrentTexBufferSize   = 0;

        //Ensure the proper alignment
        mTexLastOffset = alignToNextMultiple( mTexLastOffset, mVaoManager->getTexBufferAlignment() );
    }
    //-----------------------------------------------------------------------------------
    DECL_MALLOC float* HlmsUnlit::mapNextTexBuffer( CommandBuffer *commandBuffer, size_t minimumSizeBytes )
    {
        unmapTexBuffer( commandBuffer );

        TexBufferPacked *texBuffer = mTexBuffers[mCurrentTexBuffer];

        mTexLastOffset = alignToNextMultiple( mTexLastOffset, mVaoManager->getTexBufferAlignment() );

        //We'll go out of bounds. This buffer is full. Get a new one and remap from 0.
        if( mTexLastOffset + minimumSizeBytes >= texBuffer->getTotalSizeBytes() )
        {
            mTexLastOffset = 0;
            ++mCurrentTexBuffer;

            if( mCurrentTexBuffer >= mTexBuffers.size() )
            {
                size_t bufferSize = std::min<size_t>( mTextureBufferDefaultSize,
                                                      mVaoManager->getTexBufferMaxSize() );
                TexBufferPacked *newBuffer = mVaoManager->createTexBuffer( PF_FLOAT32_RGBA, bufferSize,
                                                                           BT_DYNAMIC_PERSISTENT,
                                                                           0, false );
                mTexBuffers.push_back( newBuffer );
            }

            texBuffer = mTexBuffers[mCurrentTexBuffer];
        }

        mRealStartMappedTexBuffer   = reinterpret_cast<float*>(
                                            texBuffer->map( mTexLastOffset,
                                                            texBuffer->getNumElements() - mTexLastOffset,
                                                            false ) );
        mStartMappedTexBuffer   = mRealStartMappedTexBuffer;
        mCurrentMappedTexBuffer = mRealStartMappedTexBuffer;
        mCurrentTexBufferSize   = (texBuffer->getNumElements() - mTexLastOffset) >> 2;

        CbShaderBuffer *shaderBufferCmd = commandBuffer->addCommand<CbShaderBuffer>();
        *shaderBufferCmd = CbShaderBuffer( 0, texBuffer, 0, 0 );

        mLastTexBufferCmdOffset = commandBuffer->getCommandOffset( shaderBufferCmd );

        return mStartMappedTexBuffer;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::rebindTexBuffer( CommandBuffer *commandBuffer, bool resetOffset,
                                     size_t minimumSizeBytes )
    {
        assert( minimumSizeBytes > 0 );

        //Set the binding size of the old binding command (if exists)
        CbShaderBuffer *shaderBufferCmd = reinterpret_cast<CbShaderBuffer*>(
                    commandBuffer->getCommandFromOffset( mLastTexBufferCmdOffset ) );
        if( shaderBufferCmd )
        {
            assert( shaderBufferCmd->bufferPacked == mTexBuffers[mCurrentTexBuffer] );
            shaderBufferCmd->bindSizeBytes = (mCurrentMappedTexBuffer - mStartMappedTexBuffer) *
                                                sizeof(float);
        }

        const size_t bufferSizeBytes = mCurrentTexBufferSize * sizeof(float);
        size_t currentOffset = (mCurrentMappedTexBuffer - mStartMappedTexBuffer) * sizeof(float);
        currentOffset = alignToNextMultiple( currentOffset, mVaoManager->getTexBufferAlignment() );
        currentOffset = std::min( bufferSizeBytes, currentOffset );
        const size_t remainingSize = bufferSizeBytes - currentOffset;

        if( resetOffset && remainingSize < minimumSizeBytes )
        {
            mapNextTexBuffer( commandBuffer, minimumSizeBytes );
        }
        else
        {
            size_t bindOffset = (mStartMappedTexBuffer - mRealStartMappedTexBuffer) * sizeof(float);
            if( resetOffset )
            {
                mStartMappedTexBuffer = reinterpret_cast<float*>(
                            reinterpret_cast<unsigned char*>(mStartMappedTexBuffer) + currentOffset );
                mCurrentMappedTexBuffer = mStartMappedTexBuffer;
                mCurrentTexBufferSize -= currentOffset / sizeof(float);

                bindOffset = (mCurrentMappedTexBuffer - mRealStartMappedTexBuffer) * sizeof(float);
            }

            //Add a new binding command.
            shaderBufferCmd = commandBuffer->addCommand<CbShaderBuffer>();
            *shaderBufferCmd = CbShaderBuffer( 0, mTexBuffers[mCurrentTexBuffer], bindOffset, 0 );
            mLastTexBufferCmdOffset = commandBuffer->getCommandOffset( shaderBufferCmd );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::destroyAllBuffers(void)
    {
        mCurrentConstBuffer = 0;
        mCurrentTexBuffer   = 0;
        mTexLastOffset      = 0;

        {
            TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
            TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyTexBuffer( *itor );
                ++itor;
            }

            mTexBuffers.clear();
        }

        {
            ConstBufferPackedVec::const_iterator itor = mConstBuffers.begin();
            ConstBufferPackedVec::const_iterator end  = mConstBuffers.end();

            while( itor != end )
            {
                if( (*itor)->getMappingState() != MS_UNMAPPED )
                    (*itor)->unmap( UO_UNMAP_ALL );
                mVaoManager->destroyConstBuffer( *itor );
                ++itor;
            }

            mConstBuffers.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::preCommandBufferExecution( CommandBuffer *commandBuffer )
    {
        unmapConstBuffer();
        unmapTexBuffer( commandBuffer );

        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->advanceFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::postCommandBufferExecution( CommandBuffer *commandBuffer )
    {
        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->regressFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::frameEnded(void)
    {
        mCurrentConstBuffer = 0;
        mCurrentTexBuffer   = 0;
        mTexLastOffset      = 0;

        TexBufferPackedVec::const_iterator itor = mTexBuffers.begin();
        TexBufferPackedVec::const_iterator end  = mTexBuffers.end();

        while( itor != end )
        {
            (*itor)->advanceFrame();
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlit::setTextureBufferDefaultSize( size_t defaultSize )
    {
        mTextureBufferDefaultSize = defaultSize;
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
