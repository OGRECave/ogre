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
    const IdString HlmsGui2DMobile::PropertyDiffuse             = IdString( "diffuse" );
    const IdString HlmsGui2DMobile::PropertyDiffuseMapCount     = IdString( "diffuse_map_count" );
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


    const IdString c_blendModes[] =
    {
        "NormalNonPremul", "NormalPremul", "Add", "Subtract", "Multiply",
        "Multiply2x", "Screen", "Overlay", "Lighten", "Darken", "GrainExtract",
        "GrainMerge", "Difference"
    };

    extern const String c_diffuseMap[15];
    const IdString *DiffuseMapCountPtrs[15][2] =
    {
        { &HlmsGui2DMobile::PropertyDiffuseMapCount1, &HlmsGui2DMobile::PropertyBlendModeIdx1 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount2, &HlmsGui2DMobile::PropertyBlendModeIdx2 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount3, &HlmsGui2DMobile::PropertyBlendModeIdx3 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount4, &HlmsGui2DMobile::PropertyBlendModeIdx4 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount5, &HlmsGui2DMobile::PropertyBlendModeIdx5 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount6, &HlmsGui2DMobile::PropertyBlendModeIdx6 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount7, &HlmsGui2DMobile::PropertyBlendModeIdx7 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount8, &HlmsGui2DMobile::PropertyBlendModeIdx8 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount9, &HlmsGui2DMobile::PropertyBlendModeIdx9 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount10, &HlmsGui2DMobile::PropertyBlendModeIdx10 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount11, &HlmsGui2DMobile::PropertyBlendModeIdx11 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount12, &HlmsGui2DMobile::PropertyBlendModeIdx12 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount13, &HlmsGui2DMobile::PropertyBlendModeIdx13 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount14, &HlmsGui2DMobile::PropertyBlendModeIdx14 },
        { &HlmsGui2DMobile::PropertyDiffuseMapCount15, &HlmsGui2DMobile::PropertyBlendModeIdx15 }
    };

    const String c_vsPerObjectUniforms[] =
    {
        "worldViewProj"
    };
    const String c_psPerObjectUniforms[] =
    {
        "constColour"
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
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String* ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_vsPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pixelShader->getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String* ); ++i )
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
        setProperty( PropertyDiffuseMapCount, datablock->mNumTextureUnits );

        if( datablock->mIsAlphaTested )
        {
            setProperty( PropertyAlphaTest, 1 );
            Hlms::findParamInVec( params, PropertyAlphaTest, paramVal );

            pieces[PixelShader]["alpha_test_cmp_func"] = "<";

            StringVector vec = StringUtil::split( paramVal );

            StringVector::const_iterator itor = vec.begin();
            StringVector::const_iterator end  = vec.end();

            while( itor != end )
            {
                if( *itor == "less" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = "<";
                else if( *itor == "less_equal" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = "<=";
                else if( *itor == "equal" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = "==";
                else if( *itor == "greater" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = ">";
                else if( *itor == "greater_equal" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = ">=";
                else if( *itor == "not_equal" )
                    pieces[PixelShader]["alpha_test_cmp_func"] = "!=";
                else if( StringConverter::parseReal( *itor, -1.0f ) == -1.0f )
                {
                    String paramValName;
                    Hlms::findParamInVec( params, "name", paramValName );
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 paramValName + ": unknown alpha_test cmp function '" + *itor + "'",
                                 "HlmsGui2DMobile::calculateHashFor" );
                }

                ++itor;
            }
        }

        //Deal with base texture
        if( Hlms::findParamInVec( params, PropertyDiffuseMap, paramVal ) ||
            Hlms::findParamInVec( params, "diffuse_map0", paramVal ) )
        {
            setProperty( PropertyDiffuseMap, 1 );

            size_t pos = paramVal.find_first_of( "\t\n " );
            while( pos < paramVal.size() )
            {
                size_t nextPos = paramVal.find_first_of( "\t\n ", pos + 1 );
                nextPos = std::min( nextPos, paramVal.size() );

                String subString = paramVal.substr( pos + 1, nextPos - pos - 1 );
                uint val = StringConverter::parseUnsignedInt( subString, ~0 );
                if( val >= 8 && val != ~0 )
                {
                    //It's a number, but UV sets can't be 8 or more
                    String paramValName;
                    Hlms::findParamInVec( params, "name", paramValName );
                    OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                 paramValName + ": diffuse_map's UV set must be in range [0; 8)."
                                 " Actual value: " + paramVal,
                                 "HlmsGui2DMobile::calculateHashFor" );
                }
                else if( val < 8 )
                {
                    if( val >= numTexCoords )
                    {
                        String paramValName;
                        Hlms::findParamInVec( params, "name", paramVal );
                        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                     paramValName + ": diffuse_map's is trying to use more UV "
                                     "sets than the mesh has ( " + subString + "vs" +
                                     StringConverter::toString(numTexCoords) + " )",
                                     "HlmsGui2DMobile::calculateHashFor" );
                    }

                    //Valid UV set
                    setProperty( PropertyDiffuseMapCount0, val );
                }

                pos = nextPos;
            }
        }

        //Deal with top textures
        for( size_t i=0; i<sizeof( c_diffuseMap ) / sizeof( String* ); ++i )
        {
            if( Hlms::findParamInVec( params, c_diffuseMap[i], paramVal ) )
            {
                pieces[PixelShader][*DiffuseMapCountPtrs[i][1]] = "@insertpiece( NormalPremul )";

                size_t pos = paramVal.find_first_of( "\t\n " );
                while( pos < paramVal.size() )
                {
                    size_t nextPos = paramVal.find_first_of( "\t\n ", pos + 1 );
                    nextPos = std::min( nextPos, paramVal.size() );

                    String subString = paramVal.substr( pos + 1, nextPos - pos - 1 );
                    uint val = StringConverter::parseUnsignedInt( subString, ~0 );
                    if( val >= 8 && val != ~0 )
                    {
                        //It's a number, but UV sets can't be 8 or more
                        String paramValName;
                        Hlms::findParamInVec( params, "name", paramValName );
                        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                     paramValName + ": diffuse_map's UV set must be in range [0; 8)."
                                     " Actual value: " + paramVal,
                                     "HlmsGui2DMobile::calculateHashFor" );
                    }
                    else if( val < 8 )
                    {
                        //Valid UV set
                        setProperty( *DiffuseMapCountPtrs[i][0], val );
                    }
                    else
                    {
                        //Must be a blend
                        const IdString *it = std::find( c_blendModes, c_blendModes +
                                                        sizeof(c_blendModes) / sizeof( IdString* ),
                                                        IdString( subString ) );

                        if( it == c_blendModes + sizeof(c_blendModes) / sizeof( IdString* ) )
                        {
                            String paramValName;
                            Hlms::findParamInVec( params, "name", paramValName );
                            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                                         paramValName + ": Invalid parameter. A blend mode or UV "
                                         "set must be specified. Actual value: " + paramVal,
                                         "HlmsGui2DMobile::calculateHashFor" );
                        }

                        pieces[PixelShader][*DiffuseMapCountPtrs[i][1]] =
                                                        "@insertpiece( " + subString + " )";
                    }

                    pos = nextPos;
                }
            }
        }

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
        //mPreparedPass.viewProjMatrix    = camera->getProjectionMatrix() * viewMatrix;
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
        float *psUniformBuffer = psParams->getFloatPointer( 0 );

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
        assert( psUniformBuffer - psParams->getFloatPointer( 0 ) == psParams->getFloatConstantList().size() );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vpParams, variabilityMask );
        mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, variabilityMask );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsGui2DMobile::createDatablockImpl( const HlmsParamVec &paramVec,
                                                         const HlmsMacroblock *macroblock,
                                                         const HlmsBlendblock *blendblock,
                                                         IdString datablockName )
    {
        return OGRE_NEW HlmsGui2DMobileDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
