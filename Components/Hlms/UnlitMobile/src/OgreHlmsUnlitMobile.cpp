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

#include "OgreHlmsUnlitMobile.h"
#include "OgreHlmsUnlitMobileDatablock.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreCamera.h"
#include "Compositor/OgreCompositorShadowNode.h"
#include "OgreCamera.h"

namespace Ogre
{
    const IdString UnlitMobileProp::TexMatrixCount      = IdString( "hlms_texture_matrix_count" );
    const IdString UnlitMobileProp::TexMatrixCount0     = IdString( "hlms_texture_matrix_count0" );
    const IdString UnlitMobileProp::TexMatrixCount1     = IdString( "hlms_texture_matrix_count1" );
    const IdString UnlitMobileProp::TexMatrixCount2     = IdString( "hlms_texture_matrix_count2" );
    const IdString UnlitMobileProp::TexMatrixCount3     = IdString( "hlms_texture_matrix_count3" );
    const IdString UnlitMobileProp::TexMatrixCount4     = IdString( "hlms_texture_matrix_count4" );
    const IdString UnlitMobileProp::TexMatrixCount5     = IdString( "hlms_texture_matrix_count5" );
    const IdString UnlitMobileProp::TexMatrixCount6     = IdString( "hlms_texture_matrix_count6" );
    const IdString UnlitMobileProp::TexMatrixCount7     = IdString( "hlms_texture_matrix_count7" );

    const IdString UnlitMobileProp::DiffuseMap          = IdString( "diffuse_map" );

    const IdString UnlitMobileProp::Diffuse             = IdString( "diffuse" );
    const IdString UnlitMobileProp::DiffuseMapCount0    = IdString( "diffuse_map_count0" );
    const IdString UnlitMobileProp::DiffuseMapCount1    = IdString( "diffuse_map_count1" );
    const IdString UnlitMobileProp::DiffuseMapCount2    = IdString( "diffuse_map_count2" );
    const IdString UnlitMobileProp::DiffuseMapCount3    = IdString( "diffuse_map_count3" );
    const IdString UnlitMobileProp::DiffuseMapCount4    = IdString( "diffuse_map_count4" );
    const IdString UnlitMobileProp::DiffuseMapCount5    = IdString( "diffuse_map_count5" );
    const IdString UnlitMobileProp::DiffuseMapCount6    = IdString( "diffuse_map_count6" );
    const IdString UnlitMobileProp::DiffuseMapCount7    = IdString( "diffuse_map_count7" );
    const IdString UnlitMobileProp::DiffuseMapCount8    = IdString( "diffuse_map_count8" );
    const IdString UnlitMobileProp::DiffuseMapCount9    = IdString( "diffuse_map_count9" );
    const IdString UnlitMobileProp::DiffuseMapCount10   = IdString( "diffuse_map_count10" );
    const IdString UnlitMobileProp::DiffuseMapCount11   = IdString( "diffuse_map_count11" );
    const IdString UnlitMobileProp::DiffuseMapCount12   = IdString( "diffuse_map_count12" );
    const IdString UnlitMobileProp::DiffuseMapCount13   = IdString( "diffuse_map_count13" );
    const IdString UnlitMobileProp::DiffuseMapCount14   = IdString( "diffuse_map_count14" );
    const IdString UnlitMobileProp::DiffuseMapCount15   = IdString( "diffuse_map_count15" );

    const IdString UnlitMobileProp::BlendModeIdx0       = IdString( "blend_mode_idx0" );
    const IdString UnlitMobileProp::BlendModeIdx1       = IdString( "blend_mode_idx1" );
    const IdString UnlitMobileProp::BlendModeIdx2       = IdString( "blend_mode_idx2" );
    const IdString UnlitMobileProp::BlendModeIdx3       = IdString( "blend_mode_idx3" );
    const IdString UnlitMobileProp::BlendModeIdx4       = IdString( "blend_mode_idx4" );
    const IdString UnlitMobileProp::BlendModeIdx5       = IdString( "blend_mode_idx5" );
    const IdString UnlitMobileProp::BlendModeIdx6       = IdString( "blend_mode_idx6" );
    const IdString UnlitMobileProp::BlendModeIdx7       = IdString( "blend_mode_idx7" );
    const IdString UnlitMobileProp::BlendModeIdx8       = IdString( "blend_mode_idx8" );
    const IdString UnlitMobileProp::BlendModeIdx9       = IdString( "blend_mode_idx9" );
    const IdString UnlitMobileProp::BlendModeIdx10      = IdString( "blend_mode_idx10" );
    const IdString UnlitMobileProp::BlendModeIdx11      = IdString( "blend_mode_idx11" );
    const IdString UnlitMobileProp::BlendModeIdx12      = IdString( "blend_mode_idx12" );
    const IdString UnlitMobileProp::BlendModeIdx13      = IdString( "blend_mode_idx13" );
    const IdString UnlitMobileProp::BlendModeIdx14      = IdString( "blend_mode_idx14" );
    const IdString UnlitMobileProp::BlendModeIdx15      = IdString( "blend_mode_idx15" );

    const IdString UnlitMobileProp::UvAtlas             = IdString( "uv_atlas" );
    const IdString UnlitMobileProp::UvAtlas0            = IdString( "uv_atlas0" );
    const IdString UnlitMobileProp::UvAtlas1            = IdString( "uv_atlas1" );
    const IdString UnlitMobileProp::UvAtlas2            = IdString( "uv_atlas2" );
    const IdString UnlitMobileProp::UvAtlas3            = IdString( "uv_atlas3" );
    const IdString UnlitMobileProp::UvAtlas4            = IdString( "uv_atlas4" );
    const IdString UnlitMobileProp::UvAtlas5            = IdString( "uv_atlas5" );
    const IdString UnlitMobileProp::UvAtlas6            = IdString( "uv_atlas6" );
    const IdString UnlitMobileProp::UvAtlas7            = IdString( "uv_atlas7" );
    const IdString UnlitMobileProp::UvAtlas8            = IdString( "uv_atlas8" );
    const IdString UnlitMobileProp::UvAtlas9            = IdString( "uv_atlas9" );
    const IdString UnlitMobileProp::UvAtlas10           = IdString( "uv_atlas10" );
    const IdString UnlitMobileProp::UvAtlas11           = IdString( "uv_atlas11" );
    const IdString UnlitMobileProp::UvAtlas12           = IdString( "uv_atlas12" );
    const IdString UnlitMobileProp::UvAtlas13           = IdString( "uv_atlas13" );
    const IdString UnlitMobileProp::UvAtlas14           = IdString( "uv_atlas14" );
    const IdString UnlitMobileProp::UvAtlas15           = IdString( "uv_atlas15" );

    extern const String c_blendModes[];

    const IdString *DiffuseMapCountPtrs[16][3] =
    {
        { &UnlitMobileProp::DiffuseMapCount0, &UnlitMobileProp::BlendModeIdx0,
          &UnlitMobileProp::UvAtlas0 },
        { &UnlitMobileProp::DiffuseMapCount1, &UnlitMobileProp::BlendModeIdx1,
          &UnlitMobileProp::UvAtlas1 },
        { &UnlitMobileProp::DiffuseMapCount2, &UnlitMobileProp::BlendModeIdx2,
          &UnlitMobileProp::UvAtlas2 },
        { &UnlitMobileProp::DiffuseMapCount3, &UnlitMobileProp::BlendModeIdx3,
          &UnlitMobileProp::UvAtlas3 },
        { &UnlitMobileProp::DiffuseMapCount4, &UnlitMobileProp::BlendModeIdx4,
          &UnlitMobileProp::UvAtlas4 },
        { &UnlitMobileProp::DiffuseMapCount5, &UnlitMobileProp::BlendModeIdx5,
          &UnlitMobileProp::UvAtlas5 },
        { &UnlitMobileProp::DiffuseMapCount6, &UnlitMobileProp::BlendModeIdx6,
          &UnlitMobileProp::UvAtlas6 },
        { &UnlitMobileProp::DiffuseMapCount7, &UnlitMobileProp::BlendModeIdx7,
          &UnlitMobileProp::UvAtlas7 },
        { &UnlitMobileProp::DiffuseMapCount8, &UnlitMobileProp::BlendModeIdx8,
          &UnlitMobileProp::UvAtlas8 },
        { &UnlitMobileProp::DiffuseMapCount9, &UnlitMobileProp::BlendModeIdx9,
          &UnlitMobileProp::UvAtlas9 },
        { &UnlitMobileProp::DiffuseMapCount10, &UnlitMobileProp::BlendModeIdx10,
          &UnlitMobileProp::UvAtlas10 },
        { &UnlitMobileProp::DiffuseMapCount11, &UnlitMobileProp::BlendModeIdx11,
          &UnlitMobileProp::UvAtlas11 },
        { &UnlitMobileProp::DiffuseMapCount12, &UnlitMobileProp::BlendModeIdx12,
          &UnlitMobileProp::UvAtlas12 },
        { &UnlitMobileProp::DiffuseMapCount13, &UnlitMobileProp::BlendModeIdx13,
          &UnlitMobileProp::UvAtlas13 },
        { &UnlitMobileProp::DiffuseMapCount14, &UnlitMobileProp::BlendModeIdx14,
          &UnlitMobileProp::UvAtlas14 },
        { &UnlitMobileProp::DiffuseMapCount15, &UnlitMobileProp::BlendModeIdx15,
          &UnlitMobileProp::UvAtlas15 },
    };


    const IdString *TexCoordAnimationMatrix[8] =
    {
        &UnlitMobileProp::TexMatrixCount0,
        &UnlitMobileProp::TexMatrixCount1,
        &UnlitMobileProp::TexMatrixCount2,
        &UnlitMobileProp::TexMatrixCount3,
        &UnlitMobileProp::TexMatrixCount4,
        &UnlitMobileProp::TexMatrixCount5,
        &UnlitMobileProp::TexMatrixCount6,
        &UnlitMobileProp::TexMatrixCount7
    };

    const String c_vsPerObjectUniforms[] =
    {
        "worldViewProj",
        "texture_matrix"
    };
    const String c_psPerObjectUniforms[] =
    {
        "constColour",
        "alpha_test_threshold",
        "atlasOffsets"
    };

    HlmsUnlitMobile::HlmsUnlitMobile( Archive *dataFolder, ArchiveVec *libraryFolders ) :
        Hlms( HLMS_UNLIT, "unlit", dataFolder, libraryFolders )
    {
        //Override defaults
        mLightGatheringMode = LightGatherNone;
    }
    //-----------------------------------------------------------------------------------
    HlmsUnlitMobile::~HlmsUnlitMobile()
    {
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsUnlitMobile::createShaderCacheEntry( uint32 renderableHash,
                                                              const HlmsCache &passCache,
                                                              uint32 finalHash,
                                                              const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        GpuNamedConstants *constantsDef;
        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pso.vertexShader->
                                                       getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_vsPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pso.pixelShader->
                                                       getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_psPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_psPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        assert( dynamic_cast<const HlmsUnlitMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitMobileDatablock *datablock = static_cast<const HlmsUnlitMobileDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        //Set samplers.
        GpuProgramParametersSharedPtr psParams = retVal->pso.pixelShader->getDefaultParameters();

        if( datablock->mNumTextureUnits > 0 )
        {
            uint texUnit = 0;
            vector<int>::type diffuseTex;
            diffuseTex.reserve( datablock->mNumTextureUnits );
            for( texUnit=0; texUnit<datablock->mNumTextureUnits; ++texUnit )
            {
                assert( !datablock->mBakedDiffuseTextures[texUnit].isNull() );
                diffuseTex.push_back( texUnit );
            }

            psParams->setNamedConstant( "texDiffuseMap", &diffuseTex[0], diffuseTex.size(), 1 );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsUnlitMobile::calculateHashFor( Renderable *renderable, uint32 &outHash,
                                            uint32 &outCasterHash )
    {
        mSetProperties.clear();

        setProperty( HlmsBaseProp::Skeleton, 0 );

        v1::RenderOperation op;
        renderable->getRenderOperation( op, false );
        v1::VertexDeclaration *vertexDecl = op.vertexData->vertexDeclaration;
        const v1::VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
        v1::VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
        v1::VertexDeclaration::VertexElementList::const_iterator end  = elementList.end();

        uint numTexCoords = 0;
        while( itor != end )
        {
            const v1::VertexElement &vertexElem = *itor;
            switch( vertexElem.getSemantic() )
            {
            case VES_DIFFUSE:
                setProperty( HlmsBaseProp::Colour, 1 );
                break;
            case VES_TEXTURE_COORDINATES:
                numTexCoords = std::max<uint>( numTexCoords, vertexElem.getIndex() + 1 );
                setProperty( *HlmsBaseProp::UvCountPtrs[vertexElem.getIndex()],
                              v1::VertexElement::getTypeCount( vertexElem.getType() ) );
                break;
            default:
                break;
            }

            vertexElem.getType();
            ++itor;
        }

        setProperty( HlmsBaseProp::UvCount, numTexCoords );

        PiecesMap pieces[NumShaderTypes];

        assert( dynamic_cast<const HlmsUnlitMobileDatablock*>( renderable->getDatablock() ) );
        const HlmsUnlitMobileDatablock *datablock = static_cast<const HlmsUnlitMobileDatablock*>(
                                                                    renderable->getDatablock() );

        setProperty( UnlitMobileProp::Diffuse, datablock->mHasColour );
        setProperty( UnlitMobileProp::DiffuseMap, datablock->mNumTextureUnits );

        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            setProperty( HlmsBaseProp::AlphaTest, 1 );

            pieces[PixelShader][HlmsBasePieces::AlphaTestCmpFunc] =
                    HlmsDatablock::getCmpString( datablock->getAlphaTest() );
        }

        setProperty( UnlitMobileProp::UvAtlas, datablock->mNumUvAtlas );
        size_t texUnit = 0;
        for( size_t i=0; i<16; ++i )
        {
            if( !datablock->mShaderCreationData->mDiffuseTextures[i].isNull() )
            {
                const uint8 uvSet = datablock->mShaderCreationData->mUvSetForTexture[i];
                const uint8 blendModeIdx = datablock->mShaderCreationData->mBlendModes[i];

                if( uvSet >= numTexCoords )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 datablock->getName().getFriendlyText() +
                                 ": diffuse_map's is trying to use more UV sets than the mesh "
                                 "has ( " + StringConverter::toString(uvSet) + " vs " +
                                 StringConverter::toString(numTexCoords) + " )",
                                 "HlmsUnlitMobile::calculateHashFor" );
                }

                setProperty( *DiffuseMapCountPtrs[texUnit][0], uvSet );
                pieces[PixelShader][*DiffuseMapCountPtrs[texUnit][1]] = "@insertpiece( " +
                                                                        c_blendModes[blendModeIdx] + ")";
                setProperty( *DiffuseMapCountPtrs[texUnit][2],
                             datablock->mShaderCreationData->mTextureIsAtlas[i] );
                ++texUnit;
            }
        }

        setProperty( UnlitMobileProp::TexMatrixCount, datablock->mNumTextureMatrices );
        for( size_t i=0; i<datablock->mNumTextureMatrices; ++i )
            setProperty( *TexCoordAnimationMatrix[i], 1 );

        uint32 renderableHash = this->addRenderableCache( mSetProperties, pieces );

        outHash         = renderableHash;
        outCasterHash   = renderableHash;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsUnlitMobile::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                                bool dualParaboloid, SceneManager *sceneManager )
    {
        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );
        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        Matrix4 projectionMatrix = camera->getProjectionMatrixWithRSDepth();
        Matrix4 identityProjMat;

        mRenderSystem->_convertProjectionMatrix( Matrix4::IDENTITY,
                                                 identityProjMat, true );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
        if( renderTarget->requiresTextureFlipping() )
        {
            projectionMatrix[1][0] = -projectionMatrix[1][0];
            projectionMatrix[1][1] = -projectionMatrix[1][1];
            projectionMatrix[1][2] = -projectionMatrix[1][2];
            projectionMatrix[1][3] = -projectionMatrix[1][3];

            identityProjMat[1][0]   = -identityProjMat[1][0];
            identityProjMat[1][1]   = -identityProjMat[1][1];
            identityProjMat[1][2]   = -identityProjMat[1][2];
            identityProjMat[1][3]   = -identityProjMat[1][3];
        }

        mPreparedPass.viewProjMatrix[0] = projectionMatrix * viewMatrix;
        mPreparedPass.viewProjMatrix[1] = identityProjMat;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlitMobile::fillBuffersFor( const HlmsCache *cache,
                                            const QueuedRenderable &queuedRenderable,
                                            bool casterPass, uint32 lastCacheHash,
                                            uint32 lastTextureHash )
    {
        GpuProgramParametersSharedPtr vpParams = cache->pso.vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = cache->pso.pixelShader->getDefaultParameters();
        float *vsUniformBuffer = vpParams->getFloatPointer( 0 );
#if _SECURE_SCL
        float *psUniformBuffer = 0;
        if( !psParams->getFloatConstantList().empty() )
            psUniformBuffer = psParams->getFloatPointer( 0 );
#else
        float *psUniformBuffer = psParams->getFloatPointer( 0 );
#endif

        assert( dynamic_cast<const HlmsUnlitMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsUnlitMobileDatablock *datablock = static_cast<const HlmsUnlitMobileDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        uint16 variabilityMask = cache->hash != lastCacheHash ? GPV_ALL : GPV_PER_OBJECT;

        //const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();
        Matrix4 worldMat;
        assert( queuedRenderable.renderable->getNumWorldTransforms() == 1u );
        queuedRenderable.renderable->getWorldTransforms( &worldMat );

        bool useIdentityProjection = queuedRenderable.renderable->getUseIdentityProjection();

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------
        //mat4 worldViewProj
        Matrix4 tmp = mPreparedPass.viewProjMatrix[ useIdentityProjection ] * worldMat;
#if !OGRE_DOUBLE_PRECISION
        memcpy( vsUniformBuffer, &tmp, sizeof( Matrix4 ) );
        vsUniformBuffer += 16;
#else
        for( int y = 0; y < 4; ++y )
        {
            for( int x = 0; x < 4; ++x )
            {
                *vsUniformBuffer++ = tmp[ y ][ x ];
            }
        }
#endif

        memcpy( vsUniformBuffer, datablock->mTextureMatrices,
                16 * datablock->mNumTextureMatrices * sizeof(float) );
        vsUniformBuffer += 16 * datablock->mNumTextureMatrices;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------
        //vec4 diffuseColour
        if( datablock->mHasColour )
        {
            memcpy( psUniformBuffer, &datablock->mR, 4 * sizeof(float) );
            psUniformBuffer += 4;
        }

        //float alpha_test_threshold
        if( datablock->mAlphaTestCmp != CMPF_ALWAYS_PASS )
            *psUniformBuffer++ = datablock->mAlphaTestThreshold;

        //vec3 atlasOffsets[]
        memcpy( psUniformBuffer, datablock->mUvAtlasParams,
                datablock->mNumUvAtlas * sizeof( HlmsUnlitMobileDatablock::UvAtlasParams ) );
        psUniformBuffer += datablock->mNumUvAtlas * (sizeof( HlmsUnlitMobileDatablock::UvAtlasParams ) /
                                                     sizeof(float) );

#if OGRE_DEBUG_MODE
        {
            uint32 oldHash = datablock->mTextureHash;
            const_cast<HlmsUnlitMobileDatablock*>(datablock)->calculateHash();
            assert( oldHash == datablock->mTextureHash &&
                    "Forgot to call calculateHash after modifying a texture to datablock" );
        }
#endif

        if( datablock->mTextureHash != lastTextureHash )
        {
            //Rebind textures
            for( uint texUnit=0; texUnit<datablock->mNumTextureUnits; ++texUnit )
            {
                mRenderSystem->_setTexture( texUnit, true,
                                            datablock->mBakedDiffuseTextures[texUnit].get() );
                mRenderSystem->_setHlmsSamplerblock( texUnit, datablock->mBakedSamplerblocks[texUnit] );
            }

            mRenderSystem->_disableTextureUnitsFrom( datablock->mNumTextureUnits );
        }

        assert( vsUniformBuffer - vpParams->getFloatPointer( 0 ) == vpParams->getFloatConstantList().size() );

#if _SECURE_SCL
        if( !psParams->getFloatConstantList().empty() )
            assert( psUniformBuffer - psParams->getFloatPointer( 0 ) == psParams->getFloatConstantList().size() );
#else
        assert( psUniformBuffer - psParams->getFloatPointer( 0 ) == psParams->getFloatConstantList().size() );
#endif

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vpParams, variabilityMask );
        mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, variabilityMask );

        return datablock->mTextureHash;
    }
    //-----------------------------------------------------------------------------------
    uint32 HlmsUnlitMobile::fillBuffersFor( const HlmsCache *cache,
                                            const QueuedRenderable &queuedRenderable,
                                            bool casterPass, uint32 lastCacheHash,
                                            CommandBuffer *commandBuffer )
    {
        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                     "Trying to use fast-path on a mobile implementation. "
                     "Change the RenderQueue settings.",
                     "HlmsUnlitMobile::fillBuffersFor" );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsUnlitMobile::createDatablockImpl( IdString datablockName,
                                                         const HlmsMacroblock *macroblock,
                                                         const HlmsBlendblock *blendblock,
                                                         const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsUnlitMobileDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
