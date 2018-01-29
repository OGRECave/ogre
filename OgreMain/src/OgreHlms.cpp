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

#include "OgreHlms.h"
#include "OgreHlmsManager.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "Vao/OgreVertexArrayObject.h"

#include "Compositor/OgreCompositorShadowNode.h"

#include "OgreLight.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreForward3D.h"
#include "OgreCamera.h"
//#include "OgreMovableObject.h"
//#include "OgreRenderable.h"
#include "OgreViewport.h"
#include "OgreRenderTarget.h"
#include "OgreDepthBuffer.h"
#include "OgreLwString.h"

#include "OgreHlmsListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    #include "iOS/macUtils.h"
#endif

namespace Ogre
{
    const int HlmsBits::HlmsTypeBits    = 3;
    const int HlmsBits::RenderableBits  = 14;
    const int HlmsBits::PassBits        = 8;
    const int HlmsBits::InputLayoutBits = 7;

    const int HlmsBits::HlmsTypeShift   = 32 - HlmsTypeBits;
    const int HlmsBits::RenderableShift = HlmsTypeShift - RenderableBits;
    const int HlmsBits::PassShift       = RenderableShift - PassBits;
    const int HlmsBits::InputLayoutShift= PassShift - InputLayoutBits;

    const int HlmsBits::RendarebleHlmsTypeMask = (1 << (HlmsTypeBits + RenderableBits)) - 1;
    const int HlmsBits::HlmsTypeMask    = (1 << HlmsTypeBits) - 1;
    const int HlmsBits::RenderableMask  = (1 << RenderableBits) - 1;
    const int HlmsBits::PassMask        = (1 << PassBits) - 1;
    const int HlmsBits::InputLayoutMask = (1 << InputLayoutBits) - 1;

    //Change per mesh (hash can be cached on the renderable)
    const IdString HlmsBaseProp::Skeleton           = IdString( "hlms_skeleton" );
    const IdString HlmsBaseProp::BonesPerVertex     = IdString( "hlms_bones_per_vertex" );
    const IdString HlmsBaseProp::Pose               = IdString( "hlms_pose" );

    const IdString HlmsBaseProp::Normal             = IdString( "hlms_normal" );
    const IdString HlmsBaseProp::QTangent           = IdString( "hlms_qtangent" );
    const IdString HlmsBaseProp::Tangent            = IdString( "hlms_tangent" );

    const IdString HlmsBaseProp::Colour             = IdString( "hlms_colour" );

    const IdString HlmsBaseProp::IdentityWorld      = IdString( "hlms_identity_world" );
    const IdString HlmsBaseProp::IdentityViewProj   = IdString( "hlms_identity_viewproj" );
    const IdString HlmsBaseProp::IdentityViewProjDynamic= IdString( "hlms_identity_viewproj_dynamic" );

    const IdString HlmsBaseProp::UvCount            = IdString( "hlms_uv_count" );
    const IdString HlmsBaseProp::UvCount0           = IdString( "hlms_uv_count0" );
    const IdString HlmsBaseProp::UvCount1           = IdString( "hlms_uv_count1" );
    const IdString HlmsBaseProp::UvCount2           = IdString( "hlms_uv_count2" );
    const IdString HlmsBaseProp::UvCount3           = IdString( "hlms_uv_count3" );
    const IdString HlmsBaseProp::UvCount4           = IdString( "hlms_uv_count4" );
    const IdString HlmsBaseProp::UvCount5           = IdString( "hlms_uv_count5" );
    const IdString HlmsBaseProp::UvCount6           = IdString( "hlms_uv_count6" );
    const IdString HlmsBaseProp::UvCount7           = IdString( "hlms_uv_count7" );
    
    //Change per frame (grouped together with scene pass)
    const IdString HlmsBaseProp::LightsDirectional  = IdString( "hlms_lights_directional" );
    const IdString HlmsBaseProp::LightsDirNonCaster = IdString( "hlms_lights_directional_non_caster" );
    const IdString HlmsBaseProp::LightsPoint        = IdString( "hlms_lights_point" );
    const IdString HlmsBaseProp::LightsSpot         = IdString( "hlms_lights_spot" );
    const IdString HlmsBaseProp::LightsAttenuation  = IdString( "hlms_lights_attenuation" );
    const IdString HlmsBaseProp::LightsSpotParams   = IdString( "hlms_lights_spotparams" );

    //Change per scene pass
    const IdString HlmsBaseProp::GlobalClipDistances= IdString( "hlms_global_clip_distances" );
    const IdString HlmsBaseProp::DualParaboloidMapping= IdString( "hlms_dual_paraboloid_mapping" );
    const IdString HlmsBaseProp::NumShadowMapLights = IdString( "hlms_num_shadow_map_lights" );
    const IdString HlmsBaseProp::NumShadowMapTextures= IdString("hlms_num_shadow_map_textures" );
    const IdString HlmsBaseProp::PssmSplits         = IdString( "hlms_pssm_splits" );
    const IdString HlmsBaseProp::PssmBlend          = IdString( "hlms_pssm_blend" );
    const IdString HlmsBaseProp::PssmFade           = IdString( "hlms_pssm_fade" );
    const IdString HlmsBaseProp::ShadowCaster       = IdString( "hlms_shadowcaster" );
    const IdString HlmsBaseProp::ShadowCasterDirectional= IdString( "hlms_shadowcaster_directional" );
    const IdString HlmsBaseProp::ShadowCasterPoint  = IdString( "hlms_shadowcaster_point" );
    const IdString HlmsBaseProp::ShadowUsesDepthTexture= IdString( "hlms_shadow_uses_depth_texture" );
    const IdString HlmsBaseProp::RenderDepthOnly    = IdString( "hlms_render_depth_only" );
    const IdString HlmsBaseProp::FineLightMask      = IdString( "hlms_fine_light_mask" );
    const IdString HlmsBaseProp::PrePass            = IdString( "hlms_prepass" );
    const IdString HlmsBaseProp::UsePrePass         = IdString( "hlms_use_prepass" );
    const IdString HlmsBaseProp::UsePrePassMsaa     = IdString( "hlms_use_prepass_msaa" );
    const IdString HlmsBaseProp::UseSsr             = IdString( "hlms_use_ssr" );
    const IdString HlmsBaseProp::EnableVpls         = IdString( "hlms_enable_vpls" );
    const IdString HlmsBaseProp::ForwardPlus        = IdString( "hlms_forwardplus" );
    const IdString HlmsBaseProp::ForwardPlusFlipY   = IdString( "hlms_forwardplus_flipY" );
    const IdString HlmsBaseProp::ForwardPlusDebug   = IdString( "hlms_forwardplus_debug" );
    const IdString HlmsBaseProp::ForwardPlusFadeAttenRange
                                                    = IdString( "hlms_forward_fade_attenuation_range" );
    const IdString HlmsBaseProp::ForwardPlusFineLightMask
                                                    = IdString( "hlms_forwardplus_fine_light_mask" );
    const IdString HlmsBaseProp::ForwardPlusCoversEntireTarget
                                                    = IdString("hlms_forwardplus_covers_entire_target");
    const IdString HlmsBaseProp::Forward3DNumSlices = IdString( "forward3d_num_slices" );
    const IdString HlmsBaseProp::FwdClusteredWidthxHeight  = IdString( "fwd_clustered_width_x_height" );
    const IdString HlmsBaseProp::FwdClusteredWidth         = IdString( "fwd_clustered_width" );
    const IdString HlmsBaseProp::FwdClusteredLightsPerCell = IdString( "fwd_clustered_lights_per_cell" );
    const IdString HlmsBaseProp::Forward3D          = IdString( "forward3d" );
    const IdString HlmsBaseProp::ForwardClustered   = IdString( "forward_clustered" );
    const IdString HlmsBaseProp::VPos               = IdString( "hlms_vpos" );

    //Change per material (hash can be cached on the renderable)
    const IdString HlmsBaseProp::AlphaTest      = IdString( "alpha_test" );
    const IdString HlmsBaseProp::AlphaBlend     = IdString( "hlms_alphablend" );

    const IdString HlmsBaseProp::Syntax         = IdString( "syntax" );
    const IdString HlmsBaseProp::Hlsl           = IdString( "hlsl" );
    const IdString HlmsBaseProp::Glsl           = IdString( "glsl" );
    const IdString HlmsBaseProp::Glsles         = IdString( "glsles" );
    const IdString HlmsBaseProp::Metal          = IdString( "metal" );
    const IdString HlmsBaseProp::GL3Plus        = IdString( "GL3+" );
    const IdString HlmsBaseProp::iOS            = IdString( "iOS" );
    const IdString HlmsBaseProp::macOS          = IdString( "macOS" );
    const IdString HlmsBaseProp::HighQuality    = IdString( "hlms_high_quality" );
    const IdString HlmsBaseProp::FastShaderBuildHack= IdString( "fast_shader_build_hack" );
    const IdString HlmsBaseProp::TexGather      = IdString( "hlms_tex_gather" );
    const IdString HlmsBaseProp::DisableStage   = IdString( "hlms_disable_stage" );

    const IdString HlmsBasePieces::AlphaTestCmpFunc = IdString( "alpha_test_cmp_func" );

    //GL extensions
    const IdString HlmsBaseProp::GlAmdTrinaryMinMax = IdString( "hlms_amd_trinary_minmax" );

    const IdString *HlmsBaseProp::UvCountPtrs[8] =
    {
        &HlmsBaseProp::UvCount0,
        &HlmsBaseProp::UvCount1,
        &HlmsBaseProp::UvCount2,
        &HlmsBaseProp::UvCount3,
        &HlmsBaseProp::UvCount4,
        &HlmsBaseProp::UvCount5,
        &HlmsBaseProp::UvCount6,
        &HlmsBaseProp::UvCount7
    };

    const IdString HlmsPsoProp::Macroblock      = IdString( "PsoMacroblock" );
    const IdString HlmsPsoProp::Blendblock      = IdString( "PsoBlendblock" );
    const IdString HlmsPsoProp::OperationTypeV1 = IdString( "OperationTypeV1" );

    const String ShaderFiles[] = { "VertexShader_vs", "PixelShader_ps", "GeometryShader_gs",
                                   "HullShader_hs", "DomainShader_ds" };
    const String PieceFilePatterns[] = { "piece_vs", "piece_ps", "piece_gs", "piece_hs", "piece_ds" };

    //Must be sorted from best to worst
    const String BestD3DShaderTargets[NumShaderTypes][5] =
    {
        {
            "vs_5_0", "vs_4_1", "vs_4_0",
            "vs_4_0_level_9_3", "vs_4_0_level_9_1"
        },
        {
            "ps_5_0", "ps_4_1", "ps_4_0",
            "ps_4_0_level_9_3", "ps_4_0_level_9_1"
        },
        {
            "gs_5_0", "gs_4_1", "gs_4_0", "placeholder", "placeholder"
        },
        {
            "hs_5_0", "hs_4_1", "hs_4_0", "placeholder", "placeholder"
        },
        {
            "ds_5_0", "ds_4_1", "ds_4_0", "placeholder", "placeholder"
        },
    };

    HlmsListener c_defaultListener;

    Hlms::Hlms( HlmsTypes type, const String &typeName, Archive *dataFolder,
                ArchiveVec *libraryFolders ) :
        mDataFolder( dataFolder ),
        mHlmsManager( 0 ),
        mLightGatheringMode( LightGatherForward ),
        mNumLightsLimit( 8 ),
        mListener( &c_defaultListener ),
        mRenderSystem( 0 ),
        mShaderProfile( "unset!" ),
        mShaderSyntax( "unset!" ),
        mShaderFileExt( "unset!" ),
        mDebugOutput( true ),
        mDebugOutputProperties( true ),
        mHighQuality( false ),
        mFastShaderBuildHack( false ),
        mDefaultDatablock( 0 ),
        mType( type ),
        mTypeName( typeName ),
        mTypeNameStr( typeName )
    {
        memset( mShaderTargets, 0, sizeof(mShaderTargets) );

        if( libraryFolders )
        {
            ArchiveVec::const_iterator itor = libraryFolders->begin();
            ArchiveVec::const_iterator end  = libraryFolders->end();

            while( itor != end )
            {
                Library library;
                library.dataFolder = *itor;
                mLibrary.push_back( library );
                ++itor;
            }
        }

        enumeratePieceFiles();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        mOutputPath = macCachePath() + '/';
#endif
    }
    //-----------------------------------------------------------------------------------
    Hlms::~Hlms()
    {
        clearShaderCache();

        _destroyAllDatablocks();

        if( mHlmsManager && mType < HLMS_MAX )
        {
            mHlmsManager->unregisterHlms( mType );
            mHlmsManager = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setCommonProperties(void)
    {
        uint16 numWorldTransforms = 2;
        //bool castShadows          = true;

        setProperty( HlmsBaseProp::Skeleton, numWorldTransforms > 1 );
        setProperty( HlmsBaseProp::UvCount, 2 );
        setProperty( "true", 1 );
        setProperty( "false", 0 );

        setProperty( HlmsBaseProp::DualParaboloidMapping, 0 );

        setProperty( HlmsBaseProp::Normal, 1 );

        setProperty( HlmsBaseProp::UvCount0, 2 );
        setProperty( HlmsBaseProp::UvCount1, 4 );
        setProperty( HlmsBaseProp::BonesPerVertex, 4 );

        setProperty( HlmsBaseProp::PssmSplits, 3 );
        setProperty( HlmsBaseProp::PssmBlend, 1 );
        setProperty( HlmsBaseProp::PssmFade, 1 );
        setProperty( HlmsBaseProp::ShadowCaster, 0 );

        setProperty( HlmsBaseProp::LightsDirectional, 1 );
        setProperty( HlmsBaseProp::LightsDirNonCaster, 1 );
        setProperty( HlmsBaseProp::LightsPoint, 2 );
        setProperty( HlmsBaseProp::LightsSpot, 3 );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::enumeratePieceFiles(void)
    {
        if( !mDataFolder )
            return; //Some Hlms implementations may not use template files at all

        bool hasValidFile = false;

        //Check this folder can at least generate one valid type of shader.
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
             //Probe both types since this may be called before we know what RS to use.
            const String filename = ShaderFiles[i];
            hasValidFile |= mDataFolder->exists( filename + ".glsl" );
            hasValidFile |= mDataFolder->exists( filename + ".hlsl" );
            hasValidFile |= mDataFolder->exists( filename + ".metal" );
            hasValidFile |= mDataFolder->exists( filename + ".any" );
        }

        if( !hasValidFile )
        {
            OGRE_EXCEPT( Exception::ERR_FILE_NOT_FOUND,
                         "Data folder provided contains no valid template shader files. "
                         "Did you provide the right folder location? Check you have the "
                         "right read pemissions. Folder: " + mDataFolder->getName(),
                         "Hlms::Hlms" );
        }

        enumeratePieceFiles( mDataFolder, mPieceFiles );

        LibraryVec::iterator itor = mLibrary.begin();
        LibraryVec::iterator end  = mLibrary.end();

        while( itor != end )
        {
            bool foundPieceFiles = enumeratePieceFiles( itor->dataFolder, itor->pieceFiles );

            if( !foundPieceFiles )
            {
                LogManager::getSingleton().logMessage(
                            "HLMS Library path '" + itor->dataFolder->getName() +
                            "' has no piece files. Are you sure you provided "
                            "the right path with read access?" );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::enumeratePieceFiles( Archive *dataFolder, StringVector *pieceFiles )
    {
        bool retVal = false;
        StringVectorPtr stringVectorPtr = dataFolder->list( false, false );

        StringVector stringVectorLowerCase( *stringVectorPtr );

        {
            StringVector::iterator itor = stringVectorLowerCase.begin();
            StringVector::iterator end  = stringVectorLowerCase.end();
            while( itor != end )
            {
                std::transform( itor->begin(), itor->end(), itor->begin(), ::tolower );
                ++itor;
            }
        }

        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            StringVector::const_iterator itLowerCase = stringVectorLowerCase.begin();
            StringVector::const_iterator itor = stringVectorPtr->begin();
            StringVector::const_iterator end  = stringVectorPtr->end();

            while( itor != end )
            {
                if( itLowerCase->find( PieceFilePatterns[i] ) != String::npos ||
                    itLowerCase->find( "piece_all" ) != String::npos )
                {
                    retVal = true;
                    pieceFiles[i].push_back( *itor );
                }

                ++itLowerCase;
                ++itor;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setProperty( IdString key, int32 value )
    {
        HlmsProperty p( key, value );
        HlmsPropertyVec::iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                         p, OrderPropertyByIdString );
        if( it == mSetProperties.end() || it->keyName != p.keyName )
            mSetProperties.insert( it, p );
        else
            *it = p;
    }
    //-----------------------------------------------------------------------------------
    int32 Hlms::getProperty( IdString key, int32 defaultVal ) const
    {
        HlmsProperty p( key, 0 );
        HlmsPropertyVec::const_iterator it = std::lower_bound( mSetProperties.begin(),
                                                               mSetProperties.end(),
                                                               p, OrderPropertyByIdString );
        if( it != mSetProperties.end() && it->keyName == p.keyName )
            defaultVal = it->value;

        return defaultVal;
    }
    //-----------------------------------------------------------------------------------
    int32 Hlms::getProperty( const HlmsPropertyVec &properties, IdString key, int32 defaultVal )
    {
        HlmsProperty p( key, 0 );
        HlmsPropertyVec::const_iterator it = std::lower_bound( properties.begin(),
                                                               properties.end(),
                                                               p, OrderPropertyByIdString );
        if( it != properties.end() && it->keyName == p.keyName )
            defaultVal = it->value;

        return defaultVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::findBlockEnd( SubStringRef &outSubString, bool &syntaxError )
    {
        const char *blockNames[] =
        {
            "foreach",
            "property",
            "piece"
        };

        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '@' )
            {
                SubStringRef subString( &outSubString.getOriginalBuffer(), it + 1 );

                size_t idx = subString.find( "end" );
                if( idx == 0 )
                {
                    --nesting;
                    it += sizeof( "end" ) - 1;
                    continue;
                }
                else
                {
                    for( size_t i=0; i<sizeof( blockNames ) / sizeof( char* ); ++i )
                    {
                        size_t idxBlock = subString.find( blockNames[i] );
                        if( idxBlock == 0 )
                        {
                            it = subString.begin() + strlen( blockNames[i] );
                            ++nesting;
                            break;
                        }
                    }
                }
            }

            ++it;
        }

        assert( nesting >= -1 );

        if( it != en && nesting < 0 )
            outSubString.setEnd( it - outSubString.getOriginalBuffer().begin() - (sizeof( "end" ) - 1) );
        else
        {
            syntaxError = true;

            char tmpData[64];
            memset( tmpData, 0, sizeof(tmpData) );
            strncpy( tmpData, &(*outSubString.begin()),
                     std::min<size_t>( 63u, outSubString.getSize() ) );

            printf( "Syntax Error at line %lu: start block (e.g. @foreach; @property) "
                    "without matching @end\nNear: '%s'\n", calculateLineCount( outSubString ),
                    tmpData );
        }
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return false;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
                                 outSubString.getStart() + expEnd );

        outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        bool textStarted = false;
        bool syntaxError = false;
        bool nextExpressionNegates = false;

        std::vector<Expression*> expressionParents;
        ExpressionVec outExpressions;
        outExpressions.clear();
        outExpressions.resize( 1 );

        Expression *currentExpression = &outExpressions.back();

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' )
            {
                currentExpression->children.push_back( Expression() );
                expressionParents.push_back( currentExpression );

                currentExpression->children.back().negated = nextExpressionNegates;

                textStarted = false;
                nextExpressionNegates = false;

                currentExpression = &currentExpression->children.back();
            }
            else if( c == ')' )
            {
                if( expressionParents.empty() )
                    syntaxError = true;
                else
                {
                    currentExpression = expressionParents.back();
                    expressionParents.pop_back();
                }

                textStarted = false;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                textStarted = false;
            }
            else if( c == '!' &&
                     //Avoid treating "!=" as a negation of variable.
                     ( (it + 1) == en || *(it + 1) != '=' ) )
            {
                nextExpressionNegates = true;
            }
            else
            {
                if( !textStarted )
                {
                    textStarted = true;
                    currentExpression->children.push_back( Expression() );
                    currentExpression->children.back().negated = nextExpressionNegates;
                }

                if( c == '&' || c == '|' ||
                    c == '=' || c == '<' || c == '>' ||
                    c == '!' /* can only mean "!=" */ )
                {
                    if( currentExpression->children.empty() || nextExpressionNegates )
                    {
                        syntaxError = true;
                    }
                    else if( !currentExpression->children.back().value.empty() &&
                             c != *(currentExpression->children.back().value.end()-1) &&
                             c != '=' )
                    {
                        currentExpression->children.push_back( Expression() );
                    }
                }

                currentExpression->children.back().value.push_back( c );
                nextExpressionNegates = false;
            }

            ++it;
        }

        bool retVal = false;

        if( !expressionParents.empty() )
            syntaxError = true;

        if( !syntaxError )
            retVal = evaluateExpressionRecursive( outExpressions, syntaxError ) != 0;

        if( syntaxError )
            printf( "Syntax Error at line %lu\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    int32 Hlms::evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const
    {
        bool syntaxError = outSyntaxError;
        bool lastExpWasOperator = true;
        ExpressionVec::iterator itor = expression.begin();
        ExpressionVec::iterator end  = expression.end();

        while( itor != end )
        {
            Expression &exp = *itor;

            if( exp.value == "&&" )
                exp.type = EXPR_OPERATOR_AND;
            else if( exp.value == "||" )
                exp.type = EXPR_OPERATOR_OR;
            else if( exp.value == "<" )
                exp.type = EXPR_OPERATOR_LE;
            else if( exp.value == "<=" )
                exp.type = EXPR_OPERATOR_LEEQ;
            else if( exp.value == "==" )
                exp.type = EXPR_OPERATOR_EQ;
            else if( exp.value == "!=" )
                exp.type = EXPR_OPERATOR_NEQ;
            else if( exp.value == ">" )
                exp.type = EXPR_OPERATOR_GR;
            else if( exp.value == ">=" )
                exp.type = EXPR_OPERATOR_GREQ;
            else if( !exp.children.empty() )
                exp.type = EXPR_OBJECT;
            else
                exp.type = EXPR_VAR;

            if( ( exp.isOperator() &&  lastExpWasOperator) ||
                (!exp.isOperator() && !lastExpWasOperator) )
            {
                syntaxError = true;
                printf( "Unrecognized token '%s'", exp.value.c_str() );
            }
            else
            {
                lastExpWasOperator = exp.isOperator();
            }

            ++itor;
        }

        //If we don't check 'expression.size() > 3u' here, we can end up in infinite recursion
        //later on (because operators get turned into EXPR_OBJECT and thus the object
        //is evaluated recusrively, and turned again into EXPR_OBJECT)
        if( !syntaxError && expression.size() > 3u )
        {
            //We will now enclose "a < b" into "(a < b)" other wise statements like these:
            //a && b < c will be parsed as (a && b) < c which is completely counterintuitive.

            //We need expression.size() > 3 which is guaranteed because if back nor front
            //are neither operators and we can't have two operators in a row, then they can
            //only be in the middle, or there is no operator at all.
            itor = expression.begin() + 1;
            end  = expression.end();
            while( itor != end )
            {
                if( itor->type >= EXPR_OPERATOR_LE && itor->type <= EXPR_OPERATOR_GREQ )
                {
                    //We need to merge n-1, n, n+1 into:
                    // (n-1)' = EXPR_OBJECT with 3 children:
                    //      n-1, n, n+1
                    //and then remove both n & n+1.
                    itor->children.resize( 3 );

                    itor->children[1].type = itor->type;
                    itor->children[1].value.swap( itor->value );
                    itor->children[0].swap( *(itor - 1) );
                    itor->children[2].swap( *(itor + 1) );

                    itor->type = EXPR_OBJECT;

                    (itor - 1)->swap( *itor );

                    itor = expression.erase( itor, itor + 2 );
                    end  = expression.end();
                }
                else
                {
                    ++itor;
                }
            }
        }

        //Evaluate the individual properties.
        itor = expression.begin();
        while( itor != end && !syntaxError )
        {
            Expression &exp = *itor;
            if( exp.type == EXPR_VAR )
            {
                char *endPtr;
                exp.result = strtol( exp.value.c_str(), &endPtr, 10 );
                if( exp.value.c_str() == endPtr )
                {
                    //This isn't a number. Let's try if it's a variable
                    exp.result = getProperty( exp.value );
                }
                lastExpWasOperator = false;
            }
            else
            {
                exp.result = evaluateExpressionRecursive( exp.children, syntaxError );
                lastExpWasOperator = false;
            }

            ++itor;
        }

        //Perform operations between the different properties.
        int32 retVal = 1;
        if( !syntaxError )
        {
            itor = expression.begin();

            ExpressionType nextOperation = EXPR_VAR;

            while( itor != end )
            {
                int32 result = itor->negated ? !itor->result : itor->result;

                switch( nextOperation )
                {
                case EXPR_OPERATOR_OR:      retVal = (retVal != 0) | (result != 0); break;
                case EXPR_OPERATOR_AND:     retVal = (retVal != 0) & (result != 0); break;
                case EXPR_OPERATOR_LE:      retVal =  retVal <  result; break;
                case EXPR_OPERATOR_LEEQ:    retVal =  retVal <= result; break;
                case EXPR_OPERATOR_EQ:      retVal =  retVal == result; break;
                case EXPR_OPERATOR_NEQ:     retVal =  retVal != result; break;
                case EXPR_OPERATOR_GR:      retVal =  retVal >  result; break;
                case EXPR_OPERATOR_GREQ:    retVal =  retVal >= result; break;

                case EXPR_OBJECT:
                case EXPR_VAR:
                    if( !itor->isOperator() )
                        retVal = result;
                    break;
                }

                nextOperation = itor->type;

                ++itor;
            }
        }

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::evaluateExpressionEnd( const SubStringRef &outSubString )
    {
        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '(' )
                ++nesting;
            else if( *it == ')' )
                --nesting;
            ++it;
        }

        assert( nesting >= -1 );

        size_t retVal = String::npos;
        if( it != en && nesting < 0 )
        {
            retVal = it - outSubString.begin() - 1;
        }
        else
        {
            printf( "Syntax Error at line %lu: opening parenthesis without matching closure\n",
                    calculateLineCount( outSubString ) );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::evaluateParamArgs( SubStringRef &outSubString, StringVector &outArgs,
                                  bool &outSyntaxError )
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
                                 outSubString.getStart() + expEnd );

        outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        int expressionState = 0;
        bool syntaxError = false;

        outArgs.clear();
        outArgs.push_back( String() );

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' || c == ')' || c == '@' || c == '&' || c == '|' )
            {
                syntaxError = true;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                if( expressionState == 1 )
                    expressionState = 2;
            }
            else if( c == ',' )
            {
                expressionState = 0;
                outArgs.push_back( String() );
            }
            else
            {
                if( expressionState == 2 )
                {
                    printf( "Syntax Error at line %lu: ',' or ')' expected\n",
                            calculateLineCount( subString ) );
                    syntaxError = true;
                }
                else
                {
                    outArgs.back().push_back( *it );
                    expressionState = 1;
                }
            }

            ++it;
        }

        if( syntaxError )
            printf( "Syntax Error at line %lu\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::copy( String &outBuffer, const SubStringRef &inSubString, size_t length )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
            outBuffer.push_back( *itor++ );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                       size_t passNum, const String &counterVar )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
        {
            if( *itor == '@' && !counterVar.empty() )
            {
                SubStringRef subString( &inSubString.getOriginalBuffer(), itor + 1 );
                if( subString.find( counterVar ) == 0 )
                {
                    char tmp[16];
                    sprintf( tmp, "%lu", passNum );
                    outBuffer += tmp;
                    itor += counterVar.size() + 1;
                }
                else
                {
                    outBuffer.push_back( *itor++ );
                }
            }
            else
            {
               outBuffer.push_back( *itor++ );
            }
        }
    }
    //-----------------------------------------------------------------------------------
        int setOp( int op1, int op2 ) { return op2; }
        int addOp( int op1, int op2 ) { return op1 + op2; }
        int subOp( int op1, int op2 ) { return op1 - op2; }
        int mulOp( int op1, int op2 ) { return op1 * op2; }
        int divOp( int op1, int op2 ) { return op1 / op2; }
        int modOp( int op1, int op2 ) { return op1 % op2; }
        int minOp( int op1, int op2 ) { return std::min( op1, op2 ); }
        int maxOp( int op1, int op2 ) { return std::max( op1, op2 ); }

        struct Operation
        {
            const char *opName;
            size_t length;
            int (*opFunc)(int, int);
            Operation( const char *_name, size_t len, int (*_opFunc)(int, int) ) :
                opName( _name ), length( len ), opFunc( _opFunc ) {}
        };

        const Operation c_operations[8] =
        {
            Operation( "pset", sizeof( "@pset" ), &setOp ),
            Operation( "padd", sizeof( "@padd" ), &addOp ),
            Operation( "psub", sizeof( "@psub" ), &subOp ),
            Operation( "pmul", sizeof( "@pmul" ), &mulOp ),
            Operation( "pdiv", sizeof( "@pdiv" ), &divOp ),
            Operation( "pmod", sizeof( "@pmod" ), &modOp ),
            Operation( "pmin", sizeof( "@pmin" ), &minOp ),
            Operation( "pmax", sizeof( "@pmax" ), &maxOp )
        };
    //-----------------------------------------------------------------------------------
    inline int Hlms::interpretAsNumberThenAsProperty( const String &argValue ) const
    {
        int opValue = StringConverter::parseInt( argValue, -std::numeric_limits<int>::max() );
        if( opValue == -std::numeric_limits<int>::max() )
        {
            //Not a number, interpret as property
            opValue = getProperty( argValue );
        }

        return opValue;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseMath( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );

        size_t pos;
        pos = subString.find( "@" );
        size_t keyword = ~0;

        while( pos != String::npos && keyword == (size_t)~0 )
        {
            size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
            maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
            SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                subString.getStart() + maxSize );

            for( size_t i=0; i<8 && keyword == (size_t)~0; ++i )
            {
                if( keywordStr.matchEqual( c_operations[i].opName ) )
                    keyword = i;
            }

            if( keyword == (size_t)~0 )
                pos = subString.find( "@", pos + 1 );
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + c_operations[keyword].length );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() < 2 || argValues.size() > 3;

            if( !syntaxError )
            {
                const IdString dstProperty = argValues[0];
                const size_t idx = argValues.size() == 3 ? 1 : 0;
                const int op1Value = interpretAsNumberThenAsProperty( argValues[idx] );
                const int op2Value = interpretAsNumberThenAsProperty( argValues[idx + 1] );

                int result = c_operations[keyword].opFunc( op1Value, op2Value );
                setProperty( dstProperty, result );
            }
            else
            {
                size_t lineCount = calculateLineCount( subString );
                if( keyword <= 1 )
                {
                    printf( "Syntax Error at line %lu: @%s expects one parameter",
                            lineCount, c_operations[keyword].opName );
                }
                else
                {
                    printf( "Syntax Error at line %lu: @%s expects two or three parameters",
                            lineCount, c_operations[keyword].opName );
                }
            }

            pos = subString.find( "@" );
            keyword = ~0;

            while( pos != String::npos && keyword == (size_t)~0 )
            {
                size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
                maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
                SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                    subString.getStart() + maxSize );

                for( size_t i=0; i<8 && keyword == (size_t)~0; ++i )
                {
                    if( keywordStr.matchEqual( c_operations[i].opName ) )
                        keyword = i;
                }

                if( keyword == (size_t)~0 )
                    pos = subString.find( "@", pos + 1 );
            }
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseForEach( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@foreach" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@foreach" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( !syntaxError )
            {
                char *endPtr;
                int count = strtol( argValues[0].c_str(), &endPtr, 10 );
                if( argValues[0].c_str() == endPtr )
                {
                    //This isn't a number. Let's try if it's a variable
                    //count = getProperty( argValues[0], -1 );
					count = getProperty( argValues[0], 0 );
                }

                /*if( count < 0 )
                {
                    printf( "Invalid parameter at line %lu (@foreach)."
                            " '%s' is not a number nor a variable\n",
                            calculateLineCount( blockSubString ), argValues[0].c_str() );
                    syntaxError = true;
                    count = 0;
                }*/

                String counterVar;
                if( argValues.size() > 1 )
                    counterVar = argValues[1];

                int start = 0;
                if( argValues.size() > 2 )
                {
                    start = strtol( argValues[2].c_str(), &endPtr, 10 );
                    if( argValues[2].c_str() == endPtr )
                    {
                        //This isn't a number. Let's try if it's a variable
                        start = getProperty( argValues[2], -1 );
                    }

                    if( start < 0 )
                    {
                        printf( "Invalid parameter at line %lu (@foreach)."
                                " '%s' is not a number nor a variable\n",
                                calculateLineCount( blockSubString ), argValues[2].c_str() );
                        syntaxError = true;
                        start = 0;
                        count = 0;
                    }
                }

                for( int i=start; i<count; ++i )
                    repeat( outBuffer, blockSubString, blockSubString.getSize(), i, counterVar );

            }

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@foreach" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseProperties( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@property" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@property" ) );
            bool result = evaluateExpression( subString, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( result && !syntaxError )
                copy( outBuffer, blockSubString, blockSubString.getSize() );

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@property" );
        }

        copy( outBuffer, subString, subString.getSize() );

        while( !syntaxError && outBuffer.find( "@property" ) != String::npos )
        {
            inBuffer.swap( outBuffer );
            syntaxError = parseProperties( inBuffer, outBuffer );
        }

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseUndefPieces( String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@undefpiece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@undefpiece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1u;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                    mPieces.erase( it );
            }
            else
            {
                printf( "Syntax Error at line %lu: @undefpiece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@undefpiece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::collectPieces( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@piece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@piece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                {
                    syntaxError = true;
                    printf( "Error at line %lu: @piece '%s' already defined",
                            calculateLineCount( subString ), argValues[0].c_str() );
                }
                else
                {
                    SubStringRef blockSubString = subString;
                    findBlockEnd( blockSubString, syntaxError );

                    String tmpBuffer;
                    copy( tmpBuffer, blockSubString, blockSubString.getSize() );
                    mPieces[pieceName] = tmpBuffer;

                    subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
                }
            }
            else
            {
                printf( "Syntax Error at line %lu: @piece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@piece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::insertPieces( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@insertpiece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@insertpiece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                    outBuffer += it->second;
            }
            else
            {
                printf( "Syntax Error at line %lu: @insertpiece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@insertpiece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
        const Operation c_counterOperations[10] =
        {
            Operation( "counter", sizeof( "@counter" ), 0 ),
            Operation( "value", sizeof( "@value" ), 0 ),
            Operation( "set", sizeof( "@set" ), &setOp ),
            Operation( "add", sizeof( "@add" ), &addOp ),
            Operation( "sub", sizeof( "@sub" ), &subOp ),
            Operation( "mul", sizeof( "@mul" ), &mulOp ),
            Operation( "div", sizeof( "@div" ), &divOp ),
            Operation( "mod", sizeof( "@mod" ), &modOp ),
            Operation( "min", sizeof( "@min" ), &minOp ),
            Operation( "max", sizeof( "@max" ), &maxOp )
        };
    //-----------------------------------------------------------------------------------
    bool Hlms::parseCounter( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVector argValues;
        SubStringRef subString( &inBuffer, 0 );

        size_t pos;
        pos = subString.find( "@" );
        size_t keyword = ~0;

        if( pos != String::npos )
        {
            size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
            maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
            SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                subString.getStart() + maxSize );

            for( size_t i=0; i<10 && keyword == (size_t)~0; ++i )
            {
                if( keywordStr.matchEqual( c_counterOperations[i].opName ) )
                    keyword = i;
            }

            if( keyword == (size_t)~0 )
                pos = String::npos;
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + c_counterOperations[keyword].length );
            evaluateParamArgs( subString, argValues, syntaxError );

            if( keyword <= 1 )
                syntaxError |= argValues.size() != 1;
            else
                syntaxError |= argValues.size() < 2 || argValues.size() > 3;

            if( !syntaxError )
            {
                if( argValues.size() == 1 )
                {
                    const IdString dstProperty = argValues[0];
                    const IdString srcProperty = dstProperty;
                    int op1Value = getProperty( srcProperty );

                    //@value & @counter write, the others are invisible
                    char tmp[16];
                    sprintf( tmp, "%i", op1Value );
                    outBuffer += tmp;

                    if( keyword == 0 )
                    {
                        ++op1Value;
                        setProperty( dstProperty, op1Value );
                    }
                }
                else
                {
                    const IdString dstProperty = argValues[0];
                    const size_t idx = argValues.size() == 3 ? 1 : 0;
                    const int op1Value = interpretAsNumberThenAsProperty( argValues[idx] );
                    const int op2Value = interpretAsNumberThenAsProperty( argValues[idx + 1] );

                    int result = c_counterOperations[keyword].opFunc( op1Value, op2Value );
                    setProperty( dstProperty, result );
                }
            }
            else
            {
                size_t lineCount = calculateLineCount( subString );
                if( keyword <= 1 )
                {
                    printf( "Syntax Error at line %lu: @%s expects one parameter",
                            lineCount, c_counterOperations[keyword].opName );
                }
                else
                {
                    printf( "Syntax Error at line %lu: @%s expects two or three parameters",
                            lineCount, c_counterOperations[keyword].opName );
                }
            }

            pos = subString.find( "@" );
            keyword = ~0;

            if( pos != String::npos )
            {
                size_t maxSize = subString.findFirstOf( " \t(", pos + 1 );
                maxSize = maxSize == String::npos ? subString.getSize() : maxSize;
                SubStringRef keywordStr( &inBuffer, subString.getStart() + pos + 1,
                                                    subString.getStart() + maxSize );

                for( size_t i=0; i<10 && keyword == (size_t)~0; ++i )
                {
                    if( keywordStr.matchEqual( c_counterOperations[i].opName ) )
                        keyword = i;
                }

                if( keyword == (size_t)~0 )
                    pos = String::npos;
            }
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parse( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        return parseForEach( inBuffer, outBuffer );
        //return parseProperties( inBuffer, outBuffer );
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::addRenderableCache( const HlmsPropertyVec &renderableSetProperties,
                                     const PiecesMap *pieces )
    {
        assert( mRenderableCache.size() <= HlmsBits::RenderableMask );

        RenderableCache cacheEntry( renderableSetProperties, pieces );

        RenderableCacheVec::iterator it = std::find( mRenderableCache.begin(), mRenderableCache.end(),
                                                     cacheEntry );
        if( it == mRenderableCache.end() )
        {
            mRenderableCache.push_back( cacheEntry );
            it = mRenderableCache.end() - 1;
        }

        //3 bits for mType (see getMaterial)
        return (mType << HlmsBits::HlmsTypeShift) |
                ((it - mRenderableCache.begin()) << HlmsBits::RenderableShift);
    }
    //-----------------------------------------------------------------------------------
    const Hlms::RenderableCache &Hlms::getRenderableCache( uint32 hash ) const
    {
        return mRenderableCache[(hash >> HlmsBits::RenderableShift) & HlmsBits::RenderableMask];
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDatablockImpl( IdString datablockName,
                                              const HlmsMacroblock *macroblock,
                                              const HlmsBlendblock *blendblock,
                                              const HlmsParamVec &paramVec )
    {
        return OGRE_NEW HlmsDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDefaultDatablock(void)
    {
        return createDatablock( IdString(), "[Default]",
                                HlmsMacroblock(), HlmsBlendblock(), HlmsParamVec(), false );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setHighQuality( bool highQuality )
    {
        clearShaderCache();
        mHighQuality = highQuality;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::saveAllTexturesFromDatablocks( const String &folderPath,
                                              set<String>::type &savedTextures,
                                              bool saveOitd, bool saveOriginal,
                                              HlmsTextureExportListener *listener )
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
        HlmsDatablockMap::const_iterator end  = mDatablocks.end();

        while( itor != end )
        {
            itor->second.datablock->saveTextures( folderPath, savedTextures, saveOitd,
                                                  saveOriginal, listener );
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::reloadFrom( Archive *newDataFolder, ArchiveVec *libraryFolders )
    {
        clearShaderCache();

        if( libraryFolders )
        {
            mLibrary.clear();

            ArchiveVec::const_iterator itor = libraryFolders->begin();
            ArchiveVec::const_iterator end  = libraryFolders->end();

            while( itor != end )
            {
                Library library;
                library.dataFolder = *itor;
                mLibrary.push_back( library );
                ++itor;
            }
        }
        else
        {
            LibraryVec::iterator itor = mLibrary.begin();
            LibraryVec::iterator end  = mLibrary.end();

            while( itor != end )
            {
                for( size_t i=0; i<NumShaderTypes; ++i )
                    itor->pieceFiles[i].clear();
                ++itor;
            }
        }

        for( size_t i=0; i<NumShaderTypes; ++i )
            mPieceFiles[i].clear();

        mDataFolder = newDataFolder;
        enumeratePieceFiles();
    }
    //-----------------------------------------------------------------------------------
    ArchiveVec Hlms::getPiecesLibraryAsArchiveVec(void) const
    {
        ArchiveVec retVal;
        LibraryVec::const_iterator itor = mLibrary.begin();
        LibraryVec::const_iterator end  = mLibrary.end();

        while( itor != end )
        {
            retVal.push_back( itor->dataFolder );
            ++itor;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::createDatablock( IdString name, const String &refName,
                                          const HlmsMacroblock &macroblockRef,
                                          const HlmsBlendblock &blendblockRef,
                                          const HlmsParamVec &paramVec, bool visibleToManager,
                                          const String &filename, const String &resourceGroup )
    {
        if( mDatablocks.find( name ) != mDatablocks.end() )
        {
            OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "A material datablock with name '" +
                         name.getFriendlyText() + "' already exists.", "Hlms::createDatablock" );
        }

        const HlmsMacroblock *macroblock = mHlmsManager->getMacroblock( macroblockRef );
        const HlmsBlendblock *blendblock = mHlmsManager->getBlendblock( blendblockRef );

        HlmsDatablock *retVal = createDatablockImpl( name, macroblock, blendblock, paramVec );

        mDatablocks[name] = DatablockEntry( retVal, visibleToManager, refName, filename, resourceGroup );

        retVal->calculateHash();

        if( visibleToManager )
            mHlmsManager->_datablockAdded( retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::getDatablock( IdString name ) const
    {
        HlmsDatablock *retVal = 0;
        HlmsDatablockMap::const_iterator itor = mDatablocks.find( name );
        if( itor != mDatablocks.end() )
            retVal = itor->second.datablock;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const String* Hlms::getNameStr(IdString name) const
    {
        String const *retVal = 0;
        HlmsDatablockMap::const_iterator itor = mDatablocks.find( name );
        if( itor != mDatablocks.end() )
            retVal = &itor->second.name;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::getFilenameAndResourceGroup( IdString name, String const * *outFilename,
                                            String const * *outResourceGroup ) const
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.find( name );
        if( itor != mDatablocks.end() )
        {
            *outFilename        = &itor->second.srcFile;
            *outResourceGroup   = &itor->second.srcResourceGroup;
        }
        else
        {
            *outFilename        = 0;
            *outResourceGroup   = 0;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::destroyDatablock( IdString name )
    {
        HlmsDatablockMap::iterator itor = mDatablocks.find( name );
        if( itor == mDatablocks.end() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "Can't find datablock with name '" + name.getFriendlyText() + "'",
                         "Hlms::destroyDatablock" );
        }

        if( itor->second.visibleToManager )
            mHlmsManager->_datablockDestroyed( name );

        OGRE_DELETE itor->second.datablock;
        mDatablocks.erase( itor );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_destroyAllDatablocks(void)
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
        HlmsDatablockMap::const_iterator end  = mDatablocks.end();

        while( itor != end )
        {
            OGRE_DELETE itor->second.datablock;
            ++itor;
        }

        mDatablocks.clear();
        mDefaultDatablock = 0;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::destroyAllDatablocks(void)
    {
        _destroyAllDatablocks();
        mDefaultDatablock = createDefaultDatablock();
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* Hlms::getDefaultDatablock(void) const
    {
        return mDefaultDatablock;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::findParamInVec( const HlmsParamVec &paramVec, IdString key, String &inOut )
    {
        bool retVal = false;
        HlmsParamVec::const_iterator it = std::lower_bound( paramVec.begin(), paramVec.end(),
                                                            std::pair<IdString, String>( key, String() ),
                                                            OrderParamVecByKey );
        if( it != paramVec.end() && it->first == key )
        {
            inOut = it->second;
            retVal = true;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::addShaderCache( uint32 hash, const HlmsPso &pso )
    {
        HlmsCache cache( hash, mType, pso );
        HlmsCacheVec::iterator it = std::lower_bound( mShaderCache.begin(), mShaderCache.end(),
                                                      &cache, OrderCacheByHash );

        assert( (it == mShaderCache.end() || (*it)->hash != hash) &&
                "Can't add the same shader to the cache twice! (or a hash collision happened)" );

        HlmsCache *retVal = new HlmsCache( cache );
        mShaderCache.insert( it, retVal );

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::getShaderCache( uint32 hash ) const
    {
        HlmsCache cache( hash, mType, HlmsPso() );
        HlmsCacheVec::const_iterator it = std::lower_bound( mShaderCache.begin(), mShaderCache.end(),
                                                            &cache, OrderCacheByHash );

        if( it != mShaderCache.end() && (*it)->hash == hash )
            return *it;

        return 0;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::clearShaderCache(void)
    {
        mPassCache.clear();

        //Empty mShaderCache so that mHlmsManager->destroyMacroblock would
        //be harmless even if _notifyMacroblockDestroyed gets called.
        HlmsCacheVec shaderCache;
        shaderCache.swap( mShaderCache );
        HlmsCacheVec::const_iterator itor = shaderCache.begin();
        HlmsCacheVec::const_iterator end  = shaderCache.end();

        while( itor != end )
        {
            mRenderSystem->_hlmsPipelineStateObjectDestroyed( &(*itor)->pso );
            if( (*itor)->pso.pass.hasStrongMacroblock() )
                mHlmsManager->destroyMacroblock( (*itor)->pso.macroblock );

            delete *itor;
            ++itor;
        }

        shaderCache.clear();
    }
    //-----------------------------------------------------------------------------------
    void Hlms::processPieces( Archive *archive, const StringVector &pieceFiles )
    {
        StringVector::const_iterator itor = pieceFiles.begin();
        StringVector::const_iterator end  = pieceFiles.end();

        while( itor != end )
        {
            //Only open piece files with current render system extension
            const String::size_type extPos0 = itor->find( mShaderFileExt );
            const String::size_type extPos1 = itor->find( ".any" );
            if( extPos0 == itor->size() - mShaderFileExt.size() ||
                extPos1 == itor->size() - 4u )
            {
                DataStreamPtr inFile = archive->open(*itor);

                String inString;
                String outString;

                inString.resize(inFile->size());
                inFile->read(&inString[0], inFile->size());

                this->parseMath(inString, outString);
                while( outString.find( "@foreach" ) != String::npos )
                {
                    this->parseForEach(outString, inString);
                    inString.swap( outString );
                }
                this->parseProperties(outString, inString);
                this->parseUndefPieces(inString, outString);
                this->collectPieces(outString, inString);
                this->parseCounter(inString, outString);
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::dumpProperties( std::ofstream &outFile )
    {
        outFile.write( "#if 0", sizeof( "#if 0" ) - 1u );

        char tmpBuffer[64];
        char friendlyText[32];
        LwString value( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

        {
            HlmsPropertyVec::const_iterator itor = mSetProperties.begin();
            HlmsPropertyVec::const_iterator end  = mSetProperties.end();

            while( itor != end )
            {
                itor->keyName.getFriendlyText( friendlyText, 32 );
                value.clear();
                value.a( itor->value );

                outFile.write( "\n\t***\t", sizeof( "\n\t***\t" ) - 1u );
                outFile.write( friendlyText, strnlen( friendlyText, 32 ) );
                outFile.write( "\t", sizeof( "\t" ) - 1u );
                outFile.write( value.c_str(), value.size() );
                ++itor;
            }
        }

        outFile.write( "\n\tDONE DUMPING PROPERTIES",
                       sizeof( "\n\tDONE DUMPING PROPERTIES" ) - 1u );

        {
            PiecesMap::const_iterator itor = mPieces.begin();
            PiecesMap::const_iterator end  = mPieces.end();

            while( itor != end )
            {
                itor->first.getFriendlyText( friendlyText, 32 );
                outFile.write( "\n\t***\t", sizeof( "\n\t***\t" ) - 1u );
                outFile.write( friendlyText, strnlen( friendlyText, 32 ) );
                outFile.write( "\t", sizeof( "\t" ) - 1u );
                outFile.write( itor->second.c_str(), itor->second.size() );
                ++itor;
            }
        }

        outFile.write( "\n\tDONE DUMPING PIECES\n#endif\n",
                       sizeof( "\n\tDONE DUMPING PIECES\n#endif\n" ) - 1u );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::applyStrongMacroblockRules( HlmsPso &pso )
    {
        if( !pso.macroblock->mDepthWrite )
        {
            //Depth writes is already off, we don't need to hold a strong reference.
            pso.pass.strongMacroblockBits &= ~HlmsPassPso::ForceDisableDepthWrites;
        }
        if( pso.macroblock->mCullMode == CULL_NONE )
        {
            //Without culling there's nothing to invert, we don't need to hold a strong reference.
            pso.pass.strongMacroblockBits &= ~HlmsPassPso::InvertVertexWinding;
        }

        if( pso.pass.hasStrongMacroblock() )
        {
            HlmsMacroblock prepassMacroblock = *pso.macroblock;

            //This is a depth prepass, disable depth writes and keep a hard copy (strong ref.)
            if( pso.pass.strongMacroblockBits & HlmsPassPso::ForceDisableDepthWrites )
                prepassMacroblock.mDepthWrite = false;
            //We need to invert culling mode.
            if( pso.pass.strongMacroblockBits & HlmsPassPso::InvertVertexWinding )
            {
                prepassMacroblock.mCullMode = prepassMacroblock.mCullMode == CULL_CLOCKWISE ?
                            CULL_ANTICLOCKWISE : CULL_CLOCKWISE;
            }

            pso.macroblock = mHlmsManager->getMacroblock( prepassMacroblock );
        }
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::createShaderCacheEntry( uint32 renderableHash, const HlmsCache &passCache,
                                                   uint32 finalHash,
                                                   const QueuedRenderable &queuedRenderable )
    {
        //Set the properties by merging the cache from the pass, with the cache from renderable
        mSetProperties.clear();
        //If retVal is null, we did something wrong earlier
        //(the cache should've been generated by now)
        const RenderableCache &renderableCache = getRenderableCache( renderableHash );
        mSetProperties.reserve( passCache.setProperties.size() + renderableCache.setProperties.size() );
        //Copy the properties from the renderable
        mSetProperties.insert( mSetProperties.end(), renderableCache.setProperties.begin(),
                                                     renderableCache.setProperties.end() );
        {
            //Now copy the properties from the pass (one by one, since be must maintain the order)
            HlmsPropertyVec::const_iterator itor = passCache.setProperties.begin();
            HlmsPropertyVec::const_iterator end  = passCache.setProperties.end();

            while( itor != end )
            {
                setProperty( itor->keyName, itor->value );
                ++itor;
            }
        }

        {
            //Add RenderSystem-specific properties
            IdStringVec::const_iterator itor = mRsSpecificExtensions.begin();
            IdStringVec::const_iterator end  = mRsSpecificExtensions.end();

            while( itor != end )
                setProperty( *itor++, 1 );
        }

        GpuProgramPtr shaders[NumShaderTypes];
        //Generate the shaders
        for( size_t i=0; i<NumShaderTypes; ++i )
        {
            //Collect pieces
            mPieces = renderableCache.pieces[i];

            const String filename = ShaderFiles[i] + mShaderFileExt;
            if( mDataFolder->exists( filename ) )
            {
                if( mShaderProfile == "glsl" ) //TODO: String comparision
                {
                    setProperty( HlmsBaseProp::GL3Plus,
                                 mRenderSystem->getNativeShadingLanguageVersion() );
                }

                setProperty( HlmsBaseProp::Syntax,  mShaderSyntax.mHash );
                setProperty( HlmsBaseProp::Hlsl,    HlmsBaseProp::Hlsl.mHash );
                setProperty( HlmsBaseProp::Glsl,    HlmsBaseProp::Glsl.mHash );
                setProperty( HlmsBaseProp::Glsles,  HlmsBaseProp::Glsles.mHash );
                setProperty( HlmsBaseProp::Metal,   HlmsBaseProp::Metal.mHash );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
                setProperty( HlmsBaseProp::iOS, 1 );
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                setProperty( HlmsBaseProp::macOS, 1 );
#endif
                setProperty( HlmsBaseProp::HighQuality, mHighQuality );

                if( mFastShaderBuildHack )
                    setProperty( HlmsBaseProp::FastShaderBuildHack, 1 );

                String debugFilenameOutput;
                std::ofstream debugDumpFile;
                if( mDebugOutput )
                {
                    debugFilenameOutput = mOutputPath + "./" +
                                            StringConverter::toString( finalHash ) +
                                            ShaderFiles[i] + mShaderFileExt;
                    debugDumpFile.open( debugFilenameOutput.c_str(), std::ios::out | std::ios::binary );

                    //We need to dump the properties before processing the files, as these
                    //may be overwritten or polñuted by the files, thus hiding why we
                    //got this permutation.
                    if( mDebugOutputProperties )
                        dumpProperties( debugDumpFile );
                }

                //Library piece files first
                LibraryVec::const_iterator itor = mLibrary.begin();
                LibraryVec::const_iterator end  = mLibrary.end();

                while( itor != end )
                {
                    processPieces( itor->dataFolder, itor->pieceFiles[i] );
                    ++itor;
                }

                //Main piece files
                processPieces( mDataFolder, mPieceFiles[i] );

                //Generate the shader file.
                DataStreamPtr inFile = mDataFolder->open( filename );

                String inString;
                String outString;

                inString.resize( inFile->size() );
                inFile->read( &inString[0], inFile->size() );

                bool syntaxError = false;

                syntaxError |= this->parseMath( inString, outString );
                while( !syntaxError && outString.find( "@foreach" ) != String::npos )
                {
                    syntaxError |= this->parseForEach( outString, inString );
                    inString.swap( outString );
                }
                syntaxError |= this->parseProperties( outString, inString );
                syntaxError |= this->parseUndefPieces( inString, outString );
                while( !syntaxError  && (outString.find( "@piece" ) != String::npos ||
                                         outString.find( "@insertpiece" ) != String::npos) )
                {
                    syntaxError |= this->collectPieces( outString, inString );
                    syntaxError |= this->insertPieces( inString, outString );
                }
                syntaxError |= this->parseCounter( outString, inString );

                outString.swap( inString );

                if( syntaxError )
                {
                    LogManager::getSingleton().logMessage( "There were HLMS syntax errors while parsing "
                                                           + StringConverter::toString( finalHash ) +
                                                           ShaderFiles[i] );
                }

                //Now dump the processed file.
                if( mDebugOutput )
                    debugDumpFile.write( &outString[0], outString.size() );

                //Don't create and compile if template requested not to
                if( !getProperty( HlmsBaseProp::DisableStage ) )
                {
                    HighLevelGpuProgramManager *gpuProgramManager =
                            HighLevelGpuProgramManager::getSingletonPtr();

                    HighLevelGpuProgramPtr gp = gpuProgramManager->createProgram(
                                StringConverter::toString( finalHash ) + ShaderFiles[i],
                                ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
                                mShaderProfile, static_cast<GpuProgramType>(i) );
                    gp->setSource( outString, debugFilenameOutput );

                    if( mShaderTargets[i] )
                    {
                        //D3D-specific
                        gp->setParameter( "target", *mShaderTargets[i] );
                        gp->setParameter( "entry_point", "main" );
                    }

                    gp->setBuildParametersFromReflection( false );
                    gp->setSkeletalAnimationIncluded( getProperty( HlmsBaseProp::Skeleton ) != 0 );
                    gp->setMorphAnimationIncluded( false );
                    gp->setPoseAnimationIncluded( getProperty( HlmsBaseProp::Pose ) );
                    gp->setVertexTextureFetchRequired( false );

                    gp->load();

                    shaders[i] = gp;
                }

                //Reset the disable flag.
                setProperty( HlmsBaseProp::DisableStage, 0 );
            }
        }

        HlmsPso pso;
        pso.initialize();
        pso.vertexShader                = shaders[VertexShader];
        pso.geometryShader              = shaders[GeometryShader];
        pso.tesselationHullShader       = shaders[HullShader];
        pso.tesselationDomainShader     = shaders[DomainShader];
        pso.pixelShader                 = shaders[PixelShader];

        bool casterPass = getProperty( HlmsBaseProp::ShadowCaster ) != 0;

        const HlmsDatablock *datablock = queuedRenderable.renderable->getDatablock();
        pso.macroblock = datablock->getMacroblock( casterPass );
        pso.blendblock = datablock->getBlendblock( casterPass );
        pso.pass = passCache.pso.pass;

        applyStrongMacroblockRules( pso );

        const size_t numGlobalClipDistances = (size_t)getProperty( HlmsBaseProp::GlobalClipDistances );
        pso.clipDistances = (1u << numGlobalClipDistances) - 1u;

        //TODO: Configurable somehow (likely should be in datablock).
        pso.sampleMask = 0xffffffff;

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

        mRenderSystem->_hlmsPipelineStateObjectCreated( &pso );

        const HlmsCache* retVal = addShaderCache( finalHash, pso );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint16 Hlms::calculateHashForV1( Renderable *renderable )
    {
        v1::RenderOperation op;
        //The Hlms uses the pass scene data to know whether this is a caster pass.
        //We want to know all the details, so request for the non-caster RenderOp.
        renderable->getRenderOperation( op, false );
        v1::VertexDeclaration *vertexDecl = op.vertexData->vertexDeclaration;
        const v1::VertexDeclaration::VertexElementList &elementList = vertexDecl->getElements();
        v1::VertexDeclaration::VertexElementList::const_iterator itor = elementList.begin();
        v1::VertexDeclaration::VertexElementList::const_iterator end  = elementList.end();

        uint numTexCoords = 0;
        while( itor != end )
        {
            const v1::VertexElement &vertexElem = *itor;
            calculateHashForSemantic( vertexElem.getSemantic(), vertexElem.getType(),
                                      vertexElem.getIndex(), numTexCoords );
            ++itor;
        }

        //v1::VertexDeclaration doesn't hold opType information. We need to save it now so
        //the PSO hash value ends up being unique (otherwise two identical vertex declarations
        //but one with tri strips another with indexed tri lists will use the same PSO!).
        //Fortunately, v1 meshes do not allow LODs with different operation types
        //(unlike v2 objects), so we can save this information here instead of doing it at
        //getMaterial time.
        setProperty( HlmsPsoProp::OperationTypeV1, op.operationType );

        return numTexCoords;
    }
    //-----------------------------------------------------------------------------------
    uint16 Hlms::calculateHashForV2( Renderable *renderable )
    {
        //TODO: Account LOD
        VertexArrayObject *vao = renderable->getVaos( VpNormal )[0];
        const VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();

        uint numTexCoords = 0;
        uint16 semIndex[VES_COUNT];
        memset( semIndex, 0, sizeof( semIndex ) );
        VertexBufferPackedVec::const_iterator itor = vertexBuffers.begin();
        VertexBufferPackedVec::const_iterator end  = vertexBuffers.end();

        while( itor != end )
        {
            const VertexElement2Vec &vertexElements = (*itor)->getVertexElements();
            VertexElement2Vec::const_iterator itElements = vertexElements.begin();
            VertexElement2Vec::const_iterator enElements = vertexElements.end();

            while( itElements != enElements )
            {
                calculateHashForSemantic( itElements->mSemantic, itElements->mType,
                                          semIndex[itElements->mSemantic-1]++, numTexCoords );
                ++itElements;
            }

            ++itor;
        }

        return numTexCoords;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::calculateHashForSemantic( VertexElementSemantic semantic, VertexElementType type,
                                         uint16 index, uint &inOutNumTexCoords )
    {
        switch( semantic )
        {
        case VES_NORMAL:
            if( v1::VertexElement::getTypeCount( type ) < 4 )
            {
                setProperty( HlmsBaseProp::Normal, 1 );
            }
            else
            {
                setProperty( HlmsBaseProp::QTangent, 1 );
            }
            break;
        case VES_TANGENT:
            setProperty( HlmsBaseProp::Tangent, 1 );
            break;
        case VES_DIFFUSE:
            setProperty( HlmsBaseProp::Colour, 1 );
            break;
        case VES_TEXTURE_COORDINATES:
            inOutNumTexCoords = std::max<uint>( inOutNumTexCoords, index + 1 );
            setProperty( *HlmsBaseProp::UvCountPtrs[index],
                          v1::VertexElement::getTypeCount( type ) );
            break;
        case VES_BLEND_WEIGHTS:
            setProperty( HlmsBaseProp::BonesPerVertex,
                         v1::VertexElement::getTypeCount( type ) );
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash )
    {
        mSetProperties.clear();

        setProperty( HlmsBaseProp::Skeleton, renderable->hasSkeletonAnimation() );

        uint16 numTexCoords = 0;
        if( renderable->getVaos( VpNormal ).empty() )
            numTexCoords = calculateHashForV1( renderable );
        else
            numTexCoords = calculateHashForV2( renderable );

        setProperty( HlmsBaseProp::UvCount, numTexCoords );

        HlmsDatablock *datablock = renderable->getDatablock();

        setProperty( HlmsBaseProp::AlphaTest, datablock->getAlphaTest() != CMPF_ALWAYS_PASS );
        setProperty( HlmsBaseProp::AlphaBlend, datablock->getBlendblock(false)->mIsTransparent );

        if( renderable->getUseIdentityWorldMatrix() )
            setProperty( HlmsBaseProp::IdentityWorld, 1 );

        if( renderable->getUseIdentityViewProjMatrixIsDynamic() )
            setProperty( HlmsBaseProp::IdentityViewProjDynamic, 1 );
        else if( renderable->getUseIdentityProjection() )
            setProperty( HlmsBaseProp::IdentityViewProj, 1 );

        setProperty( HlmsPsoProp::Macroblock, renderable->getDatablock()->getMacroblock(false)->mId );
        setProperty( HlmsPsoProp::Blendblock, renderable->getDatablock()->getBlendblock(false)->mId );

        PiecesMap pieces[NumShaderTypes];
        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            pieces[PixelShader][HlmsBasePieces::AlphaTestCmpFunc] =
                    HlmsDatablock::getCmpString( datablock->getAlphaTest() );
        }
        calculateHashForPreCreate( renderable, pieces );

        uint32 renderableHash = this->addRenderableCache( mSetProperties, pieces );

        //For shadow casters, turn normals off. UVs & diffuse also off unless there's alpha testing.
        setProperty( HlmsBaseProp::Normal, 0 );
        setProperty( HlmsBaseProp::QTangent, 0 );
        setProperty( HlmsBaseProp::AlphaBlend, datablock->getBlendblock(true)->mIsTransparent );
        PiecesMap piecesCaster[NumShaderTypes];
        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            piecesCaster[PixelShader][HlmsBasePieces::AlphaTestCmpFunc] =
                    pieces[PixelShader][HlmsBasePieces::AlphaTestCmpFunc];
        }
        calculateHashForPreCaster( renderable, piecesCaster );
        setProperty( HlmsPsoProp::Macroblock, renderable->getDatablock()->getMacroblock(true)->mId );
        setProperty( HlmsPsoProp::Blendblock, renderable->getDatablock()->getBlendblock(true)->mId );
        uint32 renderableCasterHash = this->addRenderableCache( mSetProperties, piecesCaster );

        outHash         = renderableHash;
        outCasterHash   = renderableCasterHash;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache Hlms::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                     bool dualParaboloid, SceneManager *sceneManager )
    {
        mSetProperties.clear();
        return preparePassHashBase( shadowNode, casterPass, dualParaboloid, sceneManager );
    }
    //-----------------------------------------------------------------------------------
    HlmsCache Hlms::preparePassHashBase( const CompositorShadowNode *shadowNode, bool casterPass,
                                         bool dualParaboloid, SceneManager *sceneManager )
    {
        if( !casterPass )
        {
            if( shadowNode )
            {
                int32 numPssmSplits = 0;
                const vector<Real>::type *pssmSplits = shadowNode->getPssmSplits( 0 );
                if( pssmSplits )
                    numPssmSplits = static_cast<int32>( pssmSplits->size() - 1 );
                setProperty( HlmsBaseProp::PssmSplits, numPssmSplits );

                bool isPssmBlend = false;
                const vector<Real>::type *pssmBlends = shadowNode->getPssmBlends( 0 );
                if( pssmBlends )
                    isPssmBlend = pssmBlends->size() > 0;
                setProperty( HlmsBaseProp::PssmBlend, isPssmBlend );

                bool isPssmFade = false;
                const Real *pssmFade = shadowNode->getPssmFade( 0 );
                if( pssmFade )
                    isPssmFade = *pssmFade != 0.0f;
                setProperty( HlmsBaseProp::PssmFade, isPssmFade );

                const TextureVec &contiguousShadowMapTex = shadowNode->getContiguousShadowMapTex();

                size_t numShadowMapLights = shadowNode->getNumActiveShadowCastingLights();
                if( numPssmSplits )
                    numShadowMapLights += numPssmSplits - 1;
                setProperty( HlmsBaseProp::NumShadowMapLights, numShadowMapLights );
                setProperty( HlmsBaseProp::NumShadowMapTextures, contiguousShadowMapTex.size() );

                {
                    const Ogre::CompositorShadowNodeDef *shadowNodeDef = shadowNode->getDefinition();

                    char tmpBuffer[64];
                    LwString propName( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

                    propName = "hlms_shadowmap";
                    const size_t basePropNameSize = propName.size();

                    size_t shadowMapTexIdx = 0;

                    for( size_t i=0; i<numShadowMapLights; ++i )
                    {
                        //Skip inactive lights (e.g. no directional lights are available
                        //and there's a shadow map that only accepts dir lights)
                        while( !shadowNode->isShadowMapIdxActive( shadowMapTexIdx ) )
                            ++shadowMapTexIdx;

                        const Ogre::ShadowTextureDefinition *shadowTexDef =
                                shadowNodeDef->getShadowTextureDefinition( shadowMapTexIdx );

                        propName.resize( basePropNameSize );
                        propName.a( (uint32)i ); //hlms_shadowmap0

                        const size_t basePropSize = propName.size();

                        setProperty( propName.c_str(),
                                     shadowNode->getIndexToContiguousShadowMapTex( shadowMapTexIdx ) );

                        if( shadowTexDef->uvOffset != Vector2::ZERO ||
                            shadowTexDef->uvLength != Vector2::UNIT_SCALE )
                        {
                            propName.resize( basePropSize );
                            propName.a( "_uvs_fulltex" );
                            setProperty( propName.c_str(), 1 );
                        }

                        float intPart, fractPart;

                        fractPart = modff( (float)shadowTexDef->uvOffset.x, &intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_min_x_int" );
                        setProperty( propName.c_str(), (int32)intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_min_x_fract" );
                        setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );

                        fractPart = modff( (float)shadowTexDef->uvOffset.y, &intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_min_y_int" );
                        setProperty( propName.c_str(), (int32)intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_min_y_fract" );
                        setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );

                        Vector2 uvMax = shadowTexDef->uvOffset + shadowTexDef->uvLength;
                        fractPart = modff( (float)uvMax.x, &intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_max_x_int" );
                        setProperty( propName.c_str(), (int32)intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_max_x_fract" );
                        setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );

                        fractPart = modff( (float)uvMax.y, &intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_max_y_int" );
                        setProperty( propName.c_str(), (int32)intPart );
                        propName.resize( basePropSize );
                        propName.a( "_uv_max_y_fract" );
                        setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );

                        propName.resize( basePropSize );
                        propName.a( "_array_idx" );
                        setProperty( propName.c_str(), shadowTexDef->arrayIdx );

                        const Light *light = shadowNode->getLightAssociatedWith( shadowMapTexIdx );
                        if( light->getType() == Light::LT_DIRECTIONAL )
                        {
                            propName.resize( basePropSize );
                            propName.a( "_is_directional_light" );
                            setProperty( propName.c_str(), 1 );
                        }
                        else if( light->getType() == Light::LT_POINT )
                        {
                            propName.resize( basePropSize );
                            propName.a( "_is_point_light" );
                            setProperty( propName.c_str(), 1 );

                            fractPart = modff( (float)shadowTexDef->uvLength.x, &intPart );
                            propName.resize( basePropSize );
                            propName.a( "_uv_length_x_int" );
                            setProperty( propName.c_str(), (int32)intPart );
                            propName.resize( basePropSize );
                            propName.a( "_uv_length_x_fract" );
                            setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );

                            fractPart = modff( (float)shadowTexDef->uvLength.y, &intPart );
                            propName.resize( basePropSize );
                            propName.a( "_uv_length_y_int" );
                            setProperty( propName.c_str(), (int32)intPart );
                            propName.resize( basePropSize );
                            propName.a( "_uv_length_y_fract" );
                            setProperty( propName.c_str(), (int32)(fractPart * 100000.0f) );
                        }

                        ++shadowMapTexIdx;
                    }
                }

                int usesDepthTextures = -1;

                const size_t numShadowMapTextures = contiguousShadowMapTex.size();
                for( size_t i=0; i<numShadowMapTextures; ++i )
                {
                    bool missmatch = false;

                    if( PixelUtil::isDepth( contiguousShadowMapTex[i]->getFormat() ) )
                    {
                        missmatch = usesDepthTextures == 0;
                        usesDepthTextures = 1;
                    }
                    else
                    {
                        missmatch = usesDepthTextures == 1;
                        usesDepthTextures = 0;
                    }

                    if( missmatch )
                    {
                        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                     "Mixing depth textures with non-depth textures for "
                                     "shadow mapping is not supported. Either all of "
                                     "them are depth textures, or none of them are.\n"
                                     "Shadow Node: '" + shadowNode->getName().getFriendlyText() + "'",
                                     "Hlms::preparePassHash" );
                    }
                }

                if( usesDepthTextures == -1 )
                    usesDepthTextures = 0;

                setProperty( HlmsBaseProp::ShadowUsesDepthTexture, usesDepthTextures );
            }

            ForwardPlusBase *forwardPlus = sceneManager->_getActivePassForwardPlus();
            if( forwardPlus )
                forwardPlus->setHlmsPassProperties( this );

            if( mShaderFileExt == ".glsl" )
            {
                //Actually the problem is not texture flipping, but origin. In D3D11,
                //we need to always flip because origin is different, but it's consistent
                //between texture and render window. In GL, RenderWindows don't need
                //to flip, but textures do.
                RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
                setProperty( HlmsBaseProp::ForwardPlusFlipY, renderTarget->requiresTextureFlipping() );
            }

            uint numLightsPerType[Light::NUM_LIGHT_TYPES];
            memset( numLightsPerType, 0, sizeof( numLightsPerType ) );

            uint shadowCasterDirectional = 0;

            if( mLightGatheringMode == LightGatherForwardPlus )
            {
                if( shadowNode )
                {
                    //Gather shadow casting lights, regardless of their type.
                    const LightClosestArray &lights = shadowNode->getShadowCastingLights();
                    LightClosestArray::const_iterator itor = lights.begin();
                    LightClosestArray::const_iterator end  = lights.end();
                    while( itor != end )
                    {
                        if( itor->light )
                        {
                            if( itor->light->getType() == Light::LT_DIRECTIONAL )
                                ++shadowCasterDirectional;
                            ++numLightsPerType[itor->light->getType()];
                        }
                        ++itor;
                    }
                }

                //Always gather directional lights.
                numLightsPerType[Light::LT_DIRECTIONAL] = 0;
                {
                    const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                    LightArray::const_iterator itor = globalLightList.lights.begin();
                    LightArray::const_iterator end  = globalLightList.lights.end();

                    while( itor != end )
                    {
                        if( (*itor)->getType() == Light::LT_DIRECTIONAL )
                            ++numLightsPerType[Light::LT_DIRECTIONAL];
                        ++itor;
                    }
                }
            }
            else if( mLightGatheringMode == LightGatherForward )
            {
                if( shadowNode )
                {
                    //Gather shadow casting *directional* lights.
                    const LightClosestArray &lights = shadowNode->getShadowCastingLights();
                    LightClosestArray::const_iterator itor = lights.begin();
                    LightClosestArray::const_iterator end  = lights.end();
                    while( itor != end )
                    {
                        if( itor->light && itor->light->getType() == Light::LT_DIRECTIONAL )
                            ++shadowCasterDirectional;
                        ++itor;
                    }
                }

                //Gather all lights.
                const LightListInfo &globalLightList = sceneManager->getGlobalLightList();
                LightArray::const_iterator itor = globalLightList.lights.begin();
                LightArray::const_iterator end  = globalLightList.lights.end();

                size_t numTotalLights = 0;

                while( itor != end && numTotalLights < mNumLightsLimit )
                {
                    ++numLightsPerType[(*itor)->getType()];
                    ++numTotalLights;
                    ++itor;
                }
            }

            setProperty( HlmsBaseProp::LightsAttenuation, numLightsPerType[Light::LT_POINT] +
                                                          numLightsPerType[Light::LT_SPOTLIGHT] );
            setProperty( HlmsBaseProp::LightsSpotParams,  numLightsPerType[Light::LT_SPOTLIGHT] );


            numLightsPerType[Light::LT_POINT]       += numLightsPerType[Light::LT_DIRECTIONAL];
            numLightsPerType[Light::LT_SPOTLIGHT]   += numLightsPerType[Light::LT_POINT];

            //The value is cummulative for each type (order: Directional, point, spot)
            setProperty( HlmsBaseProp::LightsDirectional, shadowCasterDirectional );
            setProperty( HlmsBaseProp::LightsDirNonCaster,numLightsPerType[Light::LT_DIRECTIONAL] );
            setProperty( HlmsBaseProp::LightsPoint,       numLightsPerType[Light::LT_POINT] );
            setProperty( HlmsBaseProp::LightsSpot,        numLightsPerType[Light::LT_SPOTLIGHT] );
        }
        else
        {
            setProperty( HlmsBaseProp::ShadowCaster, 1 );

            const CompositorPass *pass = sceneManager->getCurrentCompositorPass();

            if( pass )
            {
                const uint8 shadowMapIdx = pass->getDefinition()->mShadowMapIdx;
                const Light *light = shadowNode->getLightAssociatedWith( shadowMapIdx );
                if( light->getType() == Light::LT_DIRECTIONAL )
                    setProperty( HlmsBaseProp::ShadowCasterDirectional, 1 );
                else if( light->getType() == Light::LT_POINT )
                    setProperty( HlmsBaseProp::ShadowCasterPoint, 1 );
            }

            setProperty( HlmsBaseProp::DualParaboloidMapping, dualParaboloid );

            setProperty( HlmsBaseProp::Forward3D,         0 );
            setProperty( HlmsBaseProp::NumShadowMapLights,0 );
            setProperty( HlmsBaseProp::NumShadowMapTextures, 0 );
            setProperty( HlmsBaseProp::PssmSplits, 0 );
            setProperty( HlmsBaseProp::LightsAttenuation, 0 );
            setProperty( HlmsBaseProp::LightsSpotParams,  0 );
            setProperty( HlmsBaseProp::LightsDirectional, 0 );
            setProperty( HlmsBaseProp::LightsDirNonCaster,0 );
            setProperty( HlmsBaseProp::LightsPoint,       0 );
            setProperty( HlmsBaseProp::LightsSpot,        0 );

            RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();

            setProperty( HlmsBaseProp::ShadowUsesDepthTexture,
                         renderTarget->getForceDisableColourWrites() ? 1 : 0 );
        }

        Camera *camera = sceneManager->getCameraInProgress();
        if( camera && camera->isReflected() )
            setProperty( HlmsBaseProp::GlobalClipDistances, 1 );

        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();
        setProperty( HlmsBaseProp::RenderDepthOnly,
                     renderTarget->getForceDisableColourWrites() ? 1 : 0 );

        if( sceneManager->getCurrentPrePassMode() == PrePassCreate )
            setProperty( HlmsBaseProp::PrePass, 1 );
        else if( sceneManager->getCurrentPrePassMode() == PrePassUse )
        {
            setProperty( HlmsBaseProp::UsePrePass, 1 );
            setProperty( HlmsBaseProp::VPos, 1 );

            {
                const TextureVec *prePassTextures = sceneManager->getCurrentPrePassTextures();
                assert( prePassTextures && !prePassTextures->empty() );
                if( (*prePassTextures)[0]->getFSAA() > 1 )
                    setProperty( HlmsBaseProp::UsePrePassMsaa, (*prePassTextures)[0]->getFSAA() );
            }

            if( sceneManager->getCurrentSsrTexture() != 0 )
                setProperty( HlmsBaseProp::UseSsr, 1 );
        }

        mListener->preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager, this );

        PassCache passCache;
        passCache.passPso = getPassPsoForScene( sceneManager );
        passCache.properties = mSetProperties;

        assert( mPassCache.size() <= HlmsBits::PassMask &&
                "Too many passes combinations, we'll overflow the bits assigned in the hash!" );
        PassCacheVec::iterator it = std::find( mPassCache.begin(), mPassCache.end(), passCache );
        if( it == mPassCache.end() )
        {
            mPassCache.push_back( passCache );
            it = mPassCache.end() - 1;
        }

        const uint32 hash = (it - mPassCache.begin()) << HlmsBits::PassShift;

        HlmsCache retVal( hash, mType, HlmsPso() );
        retVal.setProperties = mSetProperties;
        retVal.pso.pass = passCache.passPso;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsPassPso Hlms::getPassPsoForScene( SceneManager *sceneManager )
    {
        RenderTarget *renderTarget = sceneManager->getCurrentViewport()->getTarget();

        HlmsPassPso passPso;

        //Needed so that memcmp in HlmsPassPso::operator == works correctly
        memset( &passPso, 0, sizeof(HlmsPassPso) );

        passPso.stencilParams = mRenderSystem->getStencilBufferParams();

        renderTarget->getFormatsForPso( passPso.colourFormat, passPso.hwGamma );

        passPso.depthFormat = PF_NULL;
        const DepthBuffer *depthBuffer = renderTarget->getDepthBuffer();
        if( depthBuffer )
            passPso.depthFormat = depthBuffer->getFormat();

        passPso.multisampleCount   = std::max( renderTarget->getFSAA(), 1u );
        passPso.multisampleQuality = StringConverter::parseInt( renderTarget->getFSAAHint() );
        passPso.adapterId = 1; //TODO: Ask RenderSystem current adapter ID.

        if( sceneManager->getCurrentPrePassMode() == PrePassUse )
            passPso.strongMacroblockBits |= HlmsPassPso::ForceDisableDepthWrites;

        const bool invertVertexWinding = mRenderSystem->getInvertVertexWinding();

        if( (renderTarget->requiresTextureFlipping() && !invertVertexWinding) ||
            (!renderTarget->requiresTextureFlipping() && invertVertexWinding) )
        {
            passPso.strongMacroblockBits |= HlmsPassPso::InvertVertexWinding;
        }

        return passPso;
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* Hlms::getMaterial( HlmsCache const *lastReturnedValue,
                                        const HlmsCache &passCache,
                                        const QueuedRenderable &queuedRenderable,
                                        uint8 inputLayout, bool casterPass )
    {
        uint32 finalHash;
        uint32 hash[2];
        hash[0] = casterPass ? queuedRenderable.renderable->getHlmsCasterHash() :
                               queuedRenderable.renderable->getHlmsHash();
        hash[1] = passCache.hash;

        //MurmurHash3_x86_32( hash, sizeof( hash ), IdString::Seed, &finalHash );

        //If this assert triggers, we've created too many shader variations (or bug)
        assert( (hash[0] >> HlmsBits::RenderableShift) <= HlmsBits::RendarebleHlmsTypeMask &&
                "Too many material / meshes variations" );
        assert( (hash[1] >> HlmsBits::PassShift) <= HlmsBits::PassMask &&
                "Should never happen (we assert in preparePassHash)" );
        assert( (inputLayout >> HlmsBits::InputLayoutShift) <= HlmsBits::InputLayoutMask &&
                "Too many vertex formats." );

        finalHash = hash[0] | hash[1] | inputLayout;

        if( lastReturnedValue->hash != finalHash )
        {
            lastReturnedValue = this->getShaderCache( finalHash );

            if( !lastReturnedValue )
            {
                lastReturnedValue = createShaderCacheEntry( hash[0], passCache, finalHash,
                                                            queuedRenderable );
            }
        }

        return lastReturnedValue;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setDebugOutputPath( bool enableDebugOutput, bool outputProperties, const String &path )
    {
        mDebugOutput            = enableDebugOutput;
        mDebugOutputProperties  = outputProperties;
        mOutputPath             = path;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setListener( HlmsListener *listener )
    {
        if( !listener )
            mListener = &c_defaultListener;
        else
            mListener = listener;
    }
    //-----------------------------------------------------------------------------------
    HlmsListener* Hlms::getListener(void) const
    {
        return mListener == &c_defaultListener ? 0 : mListener;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyShadowMappingBackFaceSetting(void)
    {
        HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
        HlmsDatablockMap::const_iterator end  = mDatablocks.end();

        while( itor != end )
        {
            HlmsDatablock *datablock = itor->second.datablock;
            datablock->setMacroblock( datablock->getMacroblock( false ), false );

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyMacroblockDestroyed( uint16 id )
    {
        bool wasUsedInWeakRefs = false;
        bool hasPsosWithStrongRefs = false;
        HlmsMacroblock macroblock;
        HlmsCacheVec::iterator itor = mShaderCache.begin();
        HlmsCacheVec::iterator end  = mShaderCache.end();

        while( itor != end )
        {
            if( (*itor)->pso.pass.hasStrongMacroblock() )
                hasPsosWithStrongRefs = true;

            if( (*itor)->pso.macroblock->mId == id )
            {
                mRenderSystem->_hlmsPipelineStateObjectDestroyed( &(*itor)->pso );
                if( !(*itor)->pso.pass.hasStrongMacroblock() )
                {
                    wasUsedInWeakRefs = true;
                    macroblock = *(*itor)->pso.macroblock;
                }
                delete *itor;
                itor = mShaderCache.erase( itor );
                end  = mShaderCache.end();
            }
            else
            {
                ++itor;
            }
        }

        if( hasPsosWithStrongRefs && wasUsedInWeakRefs )
        {
            //It's possible we made a hard clone of this macroblock with depth writes
            //disabled. We need to remove these cloned PSOs to avoid wasting memory.
            macroblock.mDepthWrite = false;
            vector<const HlmsMacroblock*>::type macroblocksToDelete;
            itor = mShaderCache.begin();
            end  = mShaderCache.end();

            while( itor != end )
            {
                if( (*itor)->pso.pass.hasStrongMacroblock() && *(*itor)->pso.macroblock == macroblock )
                    macroblocksToDelete.push_back( (*itor)->pso.macroblock );
                ++itor;
            }

            //We need to delete the macroblocks at the end because destroying a
            //macroblock could trigger _notifyMacroblockDestroyed, thus invalidating
            //iterators in mShaderCache.
            vector<const HlmsMacroblock*>::type::const_iterator itMacroblock =
                    macroblocksToDelete.begin();
            vector<const HlmsMacroblock*>::type::const_iterator enMacroblock =
                    macroblocksToDelete.end();
            while( itMacroblock != enMacroblock )
            {
                mHlmsManager->destroyMacroblock( *itMacroblock );
                ++itMacroblock;
            }
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyBlendblockDestroyed( uint16 id )
    {
        vector<const HlmsMacroblock*>::type macroblocksToDelete;
        HlmsCacheVec::iterator itor = mShaderCache.begin();
        HlmsCacheVec::iterator end  = mShaderCache.end();

        while( itor != end )
        {
            if( (*itor)->pso.blendblock->mId == id )
            {
                mRenderSystem->_hlmsPipelineStateObjectDestroyed( &(*itor)->pso );
                if( (*itor)->pso.pass.hasStrongMacroblock() )
                    macroblocksToDelete.push_back( (*itor)->pso.macroblock );
                delete *itor;
                itor = mShaderCache.erase( itor );
                end  = mShaderCache.end();
            }
            else
            {
                ++itor;
            }
        }

        //We need to delete the macroblocks at the end because destroying a
        //macroblock could trigger _notifyMacroblockDestroyed, thus invalidating
        //iterators in mShaderCache.
        vector<const HlmsMacroblock*>::type::const_iterator itMacroblock = macroblocksToDelete.begin();
        vector<const HlmsMacroblock*>::type::const_iterator enMacroblock = macroblocksToDelete.end();
        while( itMacroblock != enMacroblock )
        {
            mHlmsManager->destroyMacroblock( *itMacroblock );
            ++itMacroblock;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyInputLayoutDestroyed( uint16 id )
    {
        vector<const HlmsMacroblock*>::type macroblocksToDelete;
        HlmsCacheVec::iterator itor = mShaderCache.begin();
        HlmsCacheVec::iterator end  = mShaderCache.end();

        while( itor != end )
        {
            const uint8 inputLayout = ( (*itor)->hash >> HlmsBits::InputLayoutShift ) &
                                        HlmsBits::InputLayoutMask;
            if( inputLayout == id )
            {
                if( getProperty( (*itor)->setProperties, HlmsPsoProp::OperationTypeV1, -1 ) == -1 )
                {
                    //This is a v2 input layout.
                    mRenderSystem->_hlmsPipelineStateObjectDestroyed( &(*itor)->pso );
                    if( (*itor)->pso.pass.hasStrongMacroblock() )
                        macroblocksToDelete.push_back( (*itor)->pso.macroblock );
                    delete *itor;
                    itor = mShaderCache.erase( itor );
                    end  = mShaderCache.end();
                }
                else
                {
                    ++itor;
                }
            }
            else
            {
                ++itor;
            }
        }

        //We need to delete the macroblocks at the end because destroying a
        //macroblock could trigger _notifyMacroblockDestroyed, thus invalidating
        //iterators in mShaderCache.
        vector<const HlmsMacroblock*>::type::const_iterator itMacroblock = macroblocksToDelete.begin();
        vector<const HlmsMacroblock*>::type::const_iterator enMacroblock = macroblocksToDelete.end();
        while( itMacroblock != enMacroblock )
        {
            mHlmsManager->destroyMacroblock( *itMacroblock );
            ++itMacroblock;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_notifyV1InputLayoutDestroyed( uint16 id )
    {
        vector<const HlmsMacroblock*>::type macroblocksToDelete;
        HlmsCacheVec::iterator itor = mShaderCache.begin();
        HlmsCacheVec::iterator end  = mShaderCache.end();

        while( itor != end )
        {
            const uint8 inputLayout = ( (*itor)->hash >> HlmsBits::InputLayoutShift ) &
                                        HlmsBits::InputLayoutMask;
            if( inputLayout == id )
            {
                if( getProperty( (*itor)->setProperties, HlmsPsoProp::OperationTypeV1, -1 ) != -1 )
                {
                    //This is a v1 input layout.
                    mRenderSystem->_hlmsPipelineStateObjectDestroyed( &(*itor)->pso );
                    if( (*itor)->pso.pass.hasStrongMacroblock() )
                        macroblocksToDelete.push_back( (*itor)->pso.macroblock );
                    delete *itor;
                    itor = mShaderCache.erase( itor );
                    end  = mShaderCache.end();
                }
                else
                {
                    ++itor;
                }
            }
            else
            {
                ++itor;
            }
        }

        //We need to delete the macroblocks at the end because destroying a
        //macroblock could trigger _notifyMacroblockDestroyed, thus invalidating
        //iterators in mShaderCache.
        vector<const HlmsMacroblock*>::type::const_iterator itMacroblock = macroblocksToDelete.begin();
        vector<const HlmsMacroblock*>::type::const_iterator enMacroblock = macroblocksToDelete.end();
        while( itMacroblock != enMacroblock )
        {
            mHlmsManager->destroyMacroblock( *itMacroblock );
            ++itMacroblock;
        }
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_clearShaderCache(void)
    {
        clearShaderCache();
    }
    //-----------------------------------------------------------------------------------
    void Hlms::_changeRenderSystem( RenderSystem *newRs )
    {
        clearShaderCache();
        mRenderSystem = newRs;

        mShaderProfile = "unset!";
        mShaderFileExt = "unset!";
        mShaderSyntax  = "unset!";
        memset( mShaderTargets, 0, sizeof(mShaderTargets) );

        if( mRenderSystem )
        {
            {
                mFastShaderBuildHack = false;
                const ConfigOptionMap &rsConfigOptions = newRs->getConfigOptions();
                ConfigOptionMap::const_iterator itor = rsConfigOptions.find( "Fast Shader Build Hack" );
                if( itor != rsConfigOptions.end() )
                    mFastShaderBuildHack = StringConverter::parseBool( itor->second.currentValue );
            }

            //Prefer glsl over glsles
            const String shaderProfiles[4] = { "hlsl", "glsles", "glsl", "metal" };
            const RenderSystemCapabilities *capabilities = mRenderSystem->getCapabilities();

            for( size_t i=0; i<4; ++i )
            {
                if( capabilities->isShaderProfileSupported( shaderProfiles[i] ) )
                {
                    mShaderProfile = shaderProfiles[i];
                    mShaderSyntax = shaderProfiles[i];
                }
            }

            if( mShaderProfile == "hlsl" )
            {
                mShaderFileExt = ".hlsl";

                for( size_t i=0; i<NumShaderTypes; ++i )
                {
                    for( size_t j=0; j<5 && !mShaderTargets[i]; ++j )
                    {
                        if( capabilities->isShaderProfileSupported( BestD3DShaderTargets[i][j] ) )
                            mShaderTargets[i] = &BestD3DShaderTargets[i][j];
                    }
                }
            }
            else if( mShaderProfile == "metal" )
            {
                mShaderFileExt = ".metal";
            }
            else
            {
                mShaderFileExt = ".glsl";

                if( mRenderSystem->checkExtension( "GL_AMD_shader_trinary_minmax" ) )
                    mRsSpecificExtensions.push_back( HlmsBaseProp::GlAmdTrinaryMinMax );

                struct Extensions
                {
                    const char *extName;
                    uint32 minGlVersion;
                };

                Extensions extensions[] =
                {
                    { "GL_ARB_base_instance",               420 },
                    { "GL_ARB_shading_language_420pack",    420 },
                    { "GL_ARB_texture_buffer_range",        430 },
                };

                for( size_t i=0; i<sizeof(extensions) / sizeof(extensions[0]); ++i )
                {
                    if( mRenderSystem->getNativeShadingLanguageVersion() >= extensions[i].minGlVersion ||
                        mRenderSystem->checkExtension( extensions[i].extName ) )
                    {
                        mRsSpecificExtensions.push_back( extensions[i].extName );
                    }
                }
            }

            if( !mDefaultDatablock )
                mDefaultDatablock = createDefaultDatablock();
        }
    }
    //-----------------------------------------------------------------------------------
    /*void Hlms::generateFor()
    {
        uint16 numWorldTransforms = 1;
        bool castShadows          = true;

        *//*std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/VertexShader_vs.glsl",
                              std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_vs.glsl",
                               std::ios::out | std::ios::binary );*//*
        std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/PixelShader_ps.glsl",
                                      std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_ps.glsl",
                               std::ios::out | std::ios::binary );

        String inString;
        String outString;

        inFile.seekg( 0, std::ios::end );
        inString.resize( inFile.tellg() );
        inFile.seekg( 0, std::ios::beg );

        inFile.read( &inString[0], inString.size() );

        setCommonProperties();
        //this->parse( inString, outString );
        this->parseForEach( inString, outString );
        this->parseProperties( outString, inString );
        this->collectPieces( inString, outString );
        this->insertPieces( outString, inString );
        this->parseCounter( inString, outString );

        outFile.write( &outString[0], outString.size() );
    }*/
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const String &buffer, size_t idx )
    {
        String::const_iterator itor = buffer.begin();
        String::const_iterator end  = buffer.begin() + idx;

        size_t lineCount = 0;

        while( itor != end )
        {
            if( *itor == '\n' )
                ++lineCount;
            ++itor;
        }

        return lineCount + 1;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const SubStringRef &subString )
    {
        return calculateLineCount( subString.getOriginalBuffer(), subString.getStart() );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    inline void Hlms::Expression::swap( Expression &other )
    {
        std::swap( this->result,    other.result );
        std::swap( this->negated,   other.negated );
        std::swap( this->type,      other.type );
        this->children.swap( other.children );
        this->value.swap( other.value );
    }
}
