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

    const IdString UnlitProperty::NumArrayTextures      = IdString( "num_array_textures" );
    const IdString UnlitProperty::NumTextures           = IdString( "num_textures" );

    const IdString UnlitProperty::DiffuseMap            = IdString( "diffuse_map" );
    //const IdString UnlitProperty::DiffuseMap0           = IdString( "diffuse_map0" );
    //const IdString UnlitProperty::DiffuseMap0Array      = IdString( "diffuse_map0_array" );

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
        HlmsBufferManager( HLMS_UNLIT, "unlit", dataFolder ),
        ConstBufferPool( HlmsUnlitDatablock::MaterialSizeInGpuAligned,
                         ExtraBufferParams( 64 * NUM_UNLIT_TEXTURE_TYPES ) ),
        mCurrentPassBuffer( 0 ),
        mLastBoundPool( 0 ),
        mLastTextureHash( 0 )
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
            return retVal; //D3D embeds the texture slots in the shader.

        //Set samplers.
        GpuProgramParametersSharedPtr vsParams = retVal->vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = retVal->pixelShader->getDefaultParameters();

        int texUnit = 2; //Vertex shader consumes 2 slots with its two tbuffers.

        assert( dynamic_cast<const HlmsUnlitDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitDatablock *datablock = static_cast<const HlmsUnlitDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        UnlitBakedTextureArray::const_iterator itor = datablock->mBakedTextures.begin();
        UnlitBakedTextureArray::const_iterator end  = datablock->mBakedTextures.end();

        int numTextures = 0;
        int numArrayTextures = 0;
        while( itor != end )
        {
            if( itor->texture->getTextureType() == TEX_TYPE_2D_ARRAY )
            {
                psParams->setNamedConstant( "textureMapsArray[" +
                                            StringConverter::toString( numArrayTextures++ ) + "]",
                                            texUnit++ );
            }
            else
            {
                psParams->setNamedConstant( "textureMaps[" +
                                            StringConverter::toString( numTextures++ ) + "]",
                                            texUnit++ );
            }

            ++itor;
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
                     itor->keyName != HlmsBaseProp::AlphaTest )
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
        HlmsCache retVal( casterPass, HLMS_UNLIT );

        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();
        Matrix4 identityProjMat;

        mRenderSystem->_convertProjectionMatrix( Matrix4::IDENTITY,
                                                 identityProjMat, true );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
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

        if( casterPass )
        {
            setProperty( HlmsBaseProp::ShadowCaster, 1 );
            retVal.setProperties = mSetProperties;

            //vec2 depthRange;
            size_t mapSize = 4 * 4;

            //Arbitrary 16kb (minimum supported by GL), should be enough.
            const size_t maxBufferSize = 16 * 1024;
            assert( mapSize <= maxBufferSize );

            if( mCurrentPassBuffer >= mPassBuffers.size() )
            {
                mPassBuffers.push_back( mVaoManager->createConstBuffer( /*maxBufferSize*/mapSize,
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

            //vec2 depthRange;
            Real fNear, fFar;
            shadowNode->getMinMaxDepthRange( camera, fNear, fFar );
            const Real depthRange = fFar - fNear;
            *passBufferPtr++ = fNear;
            *passBufferPtr++ = 1.0f / depthRange;
            passBufferPtr += 2;

            assert( (size_t)(passBufferPtr - startupPtr) * 4u == mapSize );

            passBuffer->unmap( UO_KEEP_PERSISTENT );
        }

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

            if( casterPass )
            {
                //layout(binding = 0) uniform PassBuffer {} pass
                ConstBufferPacked *passBuffer = mPassBuffers[mCurrentPassBuffer-1];
                *commandBuffer->addCommand<CbShaderBuffer>() = CbShaderBuffer( VertexShader,
                                                                               0, passBuffer, 0,
                                                                               passBuffer->
                                                                               getTotalSizeBytes() );
            }

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
        }

        if( mLastBoundPool != datablock->getAssignedPool() )
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
