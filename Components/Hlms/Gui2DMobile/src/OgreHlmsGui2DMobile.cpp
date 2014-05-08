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

#include "OgreHlmsGui2DMobile.h"
#include "OgreHlmsGui2DMobileDatablock.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"

namespace Ogre
{
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount      = IdString( "hlms_texture_matrix_count" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount0     = IdString( "hlms_texture_matrix_count0" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount1     = IdString( "hlms_texture_matrix_count1" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount2     = IdString( "hlms_texture_matrix_count2" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount3     = IdString( "hlms_texture_matrix_count3" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount4     = IdString( "hlms_texture_matrix_count4" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount5     = IdString( "hlms_texture_matrix_count5" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount6     = IdString( "hlms_texture_matrix_count6" );
    const IdString HlmsGui2DMobile::PropertyTexMatrixCount7     = IdString( "hlms_texture_matrix_count7" );

    const IdString HlmsGui2DMobile::PropertyDiffuse             = IdString( "diffuse" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount0    = IdString( "diffuse_map_count0" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount1    = IdString( "diffuse_map_count1" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount2    = IdString( "diffuse_map_count2" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount3    = IdString( "diffuse_map_count3" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount4    = IdString( "diffuse_map_count4" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount5    = IdString( "diffuse_map_count5" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount6    = IdString( "diffuse_map_count6" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount7    = IdString( "diffuse_map_count7" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount8    = IdString( "diffuse_map_count8" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount9    = IdString( "diffuse_map_count9" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount10   = IdString( "diffuse_map_count10" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount11   = IdString( "diffuse_map_count11" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount12   = IdString( "diffuse_map_count12" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount13   = IdString( "diffuse_map_count13" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount14   = IdString( "diffuse_map_count14" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount15   = IdString( "diffuse_map_count15" );

    const IdString HlmsGui2DMobile::PropertyBlendModeIdx0       = IdString( "blend_mode_idx0" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx1       = IdString( "blend_mode_idx1" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx2       = IdString( "blend_mode_idx2" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx3       = IdString( "blend_mode_idx3" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx4       = IdString( "blend_mode_idx4" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx5       = IdString( "blend_mode_idx5" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx6       = IdString( "blend_mode_idx6" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx7       = IdString( "blend_mode_idx7" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx8       = IdString( "blend_mode_idx8" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx9       = IdString( "blend_mode_idx9" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx10      = IdString( "blend_mode_idx10" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx11      = IdString( "blend_mode_idx11" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx12      = IdString( "blend_mode_idx12" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx13      = IdString( "blend_mode_idx13" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx14      = IdString( "blend_mode_idx14" );
    const IdString HlmsGui2DMobile::PropertyBlendModeIdx15      = IdString( "blend_mode_idx15" );

    const IdString HlmsGui2DMobile::PropertyUvAtlas             = IdString( "uv_atlas" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas0            = IdString( "uv_atlas0" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas1            = IdString( "uv_atlas1" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas2            = IdString( "uv_atlas2" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas3            = IdString( "uv_atlas3" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas4            = IdString( "uv_atlas4" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas5            = IdString( "uv_atlas5" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas6            = IdString( "uv_atlas6" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas7            = IdString( "uv_atlas7" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas8            = IdString( "uv_atlas8" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas9            = IdString( "uv_atlas9" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas10           = IdString( "uv_atlas10" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas11           = IdString( "uv_atlas11" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas12           = IdString( "uv_atlas12" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas13           = IdString( "uv_atlas13" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas14           = IdString( "uv_atlas14" );
    const IdString HlmsGui2DMobile::PropertyUvAtlas15           = IdString( "uv_atlas15" );

    extern const String c_blendModes[];

    const IdString *DiffuseMapCountPtrs[16][3] =
    {
        { &HlmsGui2DMobile::PropertyDiffuseMapCount0, &HlmsGui2DMobile::PropertyBlendModeIdx0,
          &HlmsGui2DMobile::PropertyUvAtlas0 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount1, &HlmsGui2DMobile::PropertyBlendModeIdx1,
          &HlmsGui2DMobile::PropertyUvAtlas1 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount2, &HlmsGui2DMobile::PropertyBlendModeIdx2,
          &HlmsGui2DMobile::PropertyUvAtlas2 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount3, &HlmsGui2DMobile::PropertyBlendModeIdx3,
          &HlmsGui2DMobile::PropertyUvAtlas3 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount4, &HlmsGui2DMobile::PropertyBlendModeIdx4,
          &HlmsGui2DMobile::PropertyUvAtlas4 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount5, &HlmsGui2DMobile::PropertyBlendModeIdx5,
          &HlmsGui2DMobile::PropertyUvAtlas5 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount6, &HlmsGui2DMobile::PropertyBlendModeIdx6,
          &HlmsGui2DMobile::PropertyUvAtlas6 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount7, &HlmsGui2DMobile::PropertyBlendModeIdx7,
          &HlmsGui2DMobile::PropertyUvAtlas7 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount8, &HlmsGui2DMobile::PropertyBlendModeIdx8,
          &HlmsGui2DMobile::PropertyUvAtlas8 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount9, &HlmsGui2DMobile::PropertyBlendModeIdx9,
          &HlmsGui2DMobile::PropertyUvAtlas9 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount10, &HlmsGui2DMobile::PropertyBlendModeIdx10,
          &HlmsGui2DMobile::PropertyUvAtlas10 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount11, &HlmsGui2DMobile::PropertyBlendModeIdx11,
          &HlmsGui2DMobile::PropertyUvAtlas11 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount12, &HlmsGui2DMobile::PropertyBlendModeIdx12,
          &HlmsGui2DMobile::PropertyUvAtlas12 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount13, &HlmsGui2DMobile::PropertyBlendModeIdx13,
          &HlmsGui2DMobile::PropertyUvAtlas13 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount14, &HlmsGui2DMobile::PropertyBlendModeIdx14,
          &HlmsGui2DMobile::PropertyUvAtlas14 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount15, &HlmsGui2DMobile::PropertyBlendModeIdx15,
          &HlmsGui2DMobile::PropertyUvAtlas15 },
    };


    const IdString *TexCoordAnimationMatrix[8] =
    {
        &HlmsGui2DMobile::PropertyTexMatrixCount0,
        &HlmsGui2DMobile::PropertyTexMatrixCount1,
        &HlmsGui2DMobile::PropertyTexMatrixCount2,
        &HlmsGui2DMobile::PropertyTexMatrixCount3,
        &HlmsGui2DMobile::PropertyTexMatrixCount4,
        &HlmsGui2DMobile::PropertyTexMatrixCount5,
        &HlmsGui2DMobile::PropertyTexMatrixCount6,
        &HlmsGui2DMobile::PropertyTexMatrixCount7
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

    HlmsGui2DMobile::HlmsGui2DMobile( Archive *dataFolder ) : Hlms( HLMS_GUI, dataFolder )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsGui2DMobile::~HlmsGui2DMobile()
    {
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsGui2DMobile::createShaderCacheEntry( uint32 renderableHash,
                                                              const HlmsCache &passCache,
                                                              uint32 finalHash,
                                                              const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        GpuNamedConstants *constantsDef;
        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->vertexShader->getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_vsPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pixelShader->getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_psPerObjectUniforms ) / sizeof( String ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_psPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        assert( dynamic_cast<const HlmsGui2DMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsGui2DMobileDatablock *datablock = static_cast<const HlmsGui2DMobileDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        //Set samplers.
        GpuProgramParametersSharedPtr psParams = retVal->pixelShader->getDefaultParameters();

        if( datablock->mNumTextureUnits > 0 )
        {
            uint texUnit = 0;
            vector<int>::type diffuseTex;
            diffuseTex.reserve( datablock->mNumTextureUnits );
            for( texUnit=0; texUnit<datablock->mNumTextureUnits; ++texUnit )
            {
                assert( !datablock->mDiffuseTextures[texUnit].isNull() );
                diffuseTex.push_back( texUnit );
                ++texUnit;
            }

            psParams->setNamedConstant( "texDiffuseMap", &diffuseTex[0], diffuseTex.size(), 1 );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobile::calculateHashFor( Renderable *renderable, const HlmsParamVec &params,
                                            uint32 &outHash, uint32 &outCasterHash )
    {
        mSetProperties.clear();

        setProperty( HlmsPropertySkeleton, 0 );

        RenderOperation op;
        renderable->getRenderOperation( op );
        VertexDeclaration *vertexDecl = op.vertexData->vertexDeclaration;
        const VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
        VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
        VertexDeclaration::VertexElementList::const_iterator end  = elementList.end();

        uint numTexCoords = 0;
        while( itor != end )
        {
            const VertexElement &vertexElem = *itor;
            switch( vertexElem.getSemantic() )
            {
            case VES_DIFFUSE:
                setProperty( HlmsPropertyColour, 1 );
                break;
            case VES_TEXTURE_COORDINATES:
                numTexCoords = std::max<uint>( numTexCoords, vertexElem.getIndex() + 1 );
                setProperty( *UvCountPtrs[vertexElem.getIndex()],
                              VertexElement::getTypeCount( vertexElem.getType() ) );
                break;
            default:
                break;
            }

            vertexElem.getType();
            ++itor;
        }

        setProperty( HlmsPropertyUvCount, numTexCoords );

        String paramVal;
        if( findParamInVec( params, PropertyDiffuseMap, paramVal ) )
            setProperty( PropertyDiffuseMap, 1 );

        PiecesMap pieces[NumShaderTypes];

        assert( dynamic_cast<const HlmsGui2DMobileDatablock*>( renderable->getDatablock() ) );
        const HlmsGui2DMobileDatablock *datablock = static_cast<const HlmsGui2DMobileDatablock*>(
                                                                    renderable->getDatablock() );

        setProperty( PropertyDiffuse, datablock->mHasColour );
        setProperty( PropertyDiffuseMap, datablock->mNumTextureUnits );

        if( datablock->mIsAlphaTested )
        {
            setProperty( PropertyAlphaTest, 1 );

            switch( datablock->mShaderCreationData->alphaTestCmp )
            {
            case CMPF_LESS:             pieces[PixelShader]["alpha_test_cmp_func"] = "<"; break;
            case CMPF_LESS_EQUAL:       pieces[PixelShader]["alpha_test_cmp_func"] = "<="; break;
            case CMPF_EQUAL:            pieces[PixelShader]["alpha_test_cmp_func"] = "=="; break;
            case CMPF_GREATER:          pieces[PixelShader]["alpha_test_cmp_func"] = ">"; break;
            case CMPF_GREATER_EQUAL:    pieces[PixelShader]["alpha_test_cmp_func"] = ">="; break;
            case CMPF_NOT_EQUAL:        pieces[PixelShader]["alpha_test_cmp_func"] = "!="; break;
            default:
                break;
            }
        }

        setProperty( PropertyUvAtlas, datablock->mNumUvAtlas );
        for( size_t i=0; i<datablock->mNumTextureUnits; ++i )
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
                             "HlmsGui2DMobile::calculateHashFor" );
            }

            setProperty( *DiffuseMapCountPtrs[i][0], uvSet );
            pieces[PixelShader][*DiffuseMapCountPtrs[i][1]] = "@insertpiece( " +
                                                                c_blendModes[blendModeIdx] + ")";
            setProperty( *DiffuseMapCountPtrs[i][2],
                         datablock->mShaderCreationData->mTextureIsAtlas[i] );
        }

        setProperty( PropertyTexMatrixCount, datablock->mNumTextureMatrices );
        for( size_t i=0; i<datablock->mNumTextureMatrices; ++i )
            setProperty( *TexCoordAnimationMatrix[i], 1 );

        uint32 renderableHash = this->addRenderableCache( mSetProperties, pieces );

        outHash         = renderableHash;
        outCasterHash   = renderableHash;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsGui2DMobile::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                                bool dualParaboloid, SceneManager *sceneManager )
    {
        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );
        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        //mPreparedPass.viewProjMatrix    = camera->getProjectionMatrix() * viewMatrix; //TODO
        mPreparedPass.viewProjMatrix    = Matrix4::IDENTITY;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobile::fillBuffersFor( const HlmsCache *cache,
                                          const QueuedRenderable &queuedRenderable,
                                          bool casterPass, const HlmsCache *lastCache,
                                          uint32 lastTextureHash )
    {
        GpuProgramParametersSharedPtr vpParams = cache->vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = cache->pixelShader->getDefaultParameters();
        float *vsUniformBuffer = vpParams->getFloatPointer( 0 );
#if _SECURE_SCL
        float *psUniformBuffer = 0;
        if( !psParams->getFloatConstantList().empty() )
            psUniformBuffer = psParams->getFloatPointer( 0 );
#else
        float *psUniformBuffer = psParams->getFloatPointer( 0 );
#endif

        assert( dynamic_cast<const HlmsGui2DMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsGui2DMobileDatablock *datablock = static_cast<const HlmsGui2DMobileDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        uint16 variabilityMask = cache != lastCache ? GPV_ALL : GPV_PER_OBJECT;

        //const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();
        Matrix4 worldMat;
        assert( queuedRenderable.renderable->getNumWorldTransforms() == 1 );
        queuedRenderable.renderable->getWorldTransforms( &worldMat );

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------
#if !OGRE_DOUBLE_PRECISION
        //mat4 worldViewProj
        Matrix4 tmp = mPreparedPass.viewProjMatrix * worldMat;
        memcpy( vsUniformBuffer, &tmp, sizeof(Matrix4) );
        vsUniformBuffer += 16;
#else
    #error Not Coded Yet! (can't use memcpy on Matrix4)
#endif

        memcpy( vsUniformBuffer, datablock->mTextureMatrices,
                16 * datablock->mNumTextureMatrices * sizeof(float) );
        vsUniformBuffer += 16 * datablock->mNumTextureMatrices;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------
        if( datablock->mHasColour )
        {
            memcpy( psUniformBuffer, &datablock->mR, 4 * sizeof(float) );
            psUniformBuffer += 4;
        }

        if( datablock->mIsAlphaTested )
            *psUniformBuffer++ = datablock->mAlphaTestThreshold;

        memcpy( psUniformBuffer, datablock->mUvAtlasParams,
                datablock->mNumUvAtlas * sizeof( HlmsGui2DMobileDatablock::UvAtlasParams ) );
        psUniformBuffer += datablock->mNumUvAtlas * (sizeof( HlmsGui2DMobileDatablock::UvAtlasParams ) /
                                                     sizeof(float) );

#if OGRE_DEBUG_MODE
        {
            IdString oldHash = datablock->mTextureHash;
            const_cast<HlmsGui2DMobileDatablock*>(datablock)->calculateHash();
            assert( oldHash == datablock->mTextureHash &&
                    "Forgot to call calculateHash after modifying a texture to datablock" );
        }
#endif

        if( datablock->mTextureHash != lastTextureHash )
        {
            //Rebind textures
            for( uint texUnit=0; texUnit<datablock->mNumTextureUnits; ++texUnit )
            {
                mRenderSystem->_setTexture( texUnit, true, datablock->mDiffuseTextures[texUnit] );
                ++texUnit;
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
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsGui2DMobile::createDatablockImpl( IdString datablockName,
                                                         const HlmsMacroblock *macroblock,
                                                         const HlmsBlendblock *blendblock,
                                                         const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsGui2DMobileDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
