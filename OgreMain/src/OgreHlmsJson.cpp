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

#if !OGRE_NO_JSON

#include "OgreHlmsJson.h"
#include "OgreHlmsJsonCompute.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreVector2.h"
#include "OgreLwString.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

#include "rapidjson/document.h"

namespace Ogre
{
    static HlmsJsonListener sDefaultHlmsJsonListener;

    const char* c_filterOptions[FO_ANISOTROPIC+1] =
    {
        "none",
        "point",
        "linear",
        "anisotropic"
    };

    const char* c_textureAddressingMode[TAM_BORDER+1] =
    {
        "wrap",
        "mirror",
        "clamp",
        "border"
    };

    const char* c_compareFunctions[NUM_COMPARE_FUNCTIONS+1] =
    {
        "never",
        "always",
        "less",
        "less_equal",
        "equal",
        "not_equal",
        "greater_equal",
        "greater",
        "disabled"
    };

    const char* c_cullModes[CULL_ANTICLOCKWISE+1] =
    {
        "INVALID_CULL_MODE",
        "none",
        "clockwise",
        "anticlockwise"
    };

    const char* c_polygonModes[PM_SOLID+1] =
    {
        "INVALID_POLYGON_MODE",
        "points",
        "wireframe",
        "solid"
    };

    const char* c_sceneBlendFactor[SBF_ONE_MINUS_SOURCE_ALPHA+1] =
    {
        "one",
        "zero",
        "dst_colour",
        "src_colour",
        "one_minus_dst_colour",
        "one_minus_src_colour",
        "dst_alpha",
        "src_alpha",
        "one_minus_dst_alpha",
        "one_minus_src_alpha"
    };

    const char* c_sceneBlendOperation[SBO_MAX+1] =
    {
        "add",
        "subtract",
        "reverse_subtract",
        "min",
        "max"
    };

    HlmsJson::HlmsJson( HlmsManager *hlmsManager, HlmsJsonListener *listener ) :
        mHlmsManager( hlmsManager ),
        mListener( listener )
    {
        if( !mListener )
            mListener = &sDefaultHlmsJsonListener;
    }
    //-----------------------------------------------------------------------------------
    HlmsJson::~HlmsJson()
    {
    }
    //-----------------------------------------------------------------------------------
    FilterOptions HlmsJson::parseFilterOptions( const char *value )
    {
        if( !strcmp( value, "none" ) )
            return FO_NONE;
        if( !strcmp( value, "point" ) )
            return FO_POINT;
        if( !strcmp( value, "linear" ) )
            return FO_LINEAR;
        if( !strcmp( value, "anisotropic" ) )
            return FO_ANISOTROPIC;

        return FO_LINEAR;
    }
    //-----------------------------------------------------------------------------------
    TextureAddressingMode HlmsJson::parseTextureAddressingMode( const char *value )
    {
        if( !strcmp( value, "wrap" ) )
            return TAM_WRAP;
        if( !strcmp( value, "mirror" ) )
            return TAM_MIRROR;
        if( !strcmp( value, "clamp" ) )
            return TAM_CLAMP;
        if( !strcmp( value, "border" ) )
            return TAM_BORDER;

        return TAM_WRAP;
    }
    //-----------------------------------------------------------------------------------
    CompareFunction HlmsJson::parseCompareFunction( const char *value )
    {
        if( !strcmp( value, "less" ) )
            return CMPF_LESS;
        if( !strcmp( value, "less_equal" ) )
            return CMPF_LESS_EQUAL;
        if( !strcmp( value, "equal" ) )
            return CMPF_EQUAL;
        if( !strcmp( value, "not_equal" ) )
            return CMPF_NOT_EQUAL;
        if( !strcmp( value, "greater_equal" ) )
            return CMPF_GREATER_EQUAL;
        if( !strcmp( value, "greater" ) )
            return CMPF_GREATER;
        if( !strcmp( value, "never" ) )
            return CMPF_ALWAYS_FAIL;
        if( !strcmp( value, "always" ) )
            return CMPF_ALWAYS_PASS;
        if( !strcmp( value, "disabled" ) )
            return NUM_COMPARE_FUNCTIONS;

        return CMPF_LESS;
    }
    //-----------------------------------------------------------------------------------
    CullingMode HlmsJson::parseCullMode( const char *value )
    {
        if( !strcmp( value, "none" ) )
            return CULL_NONE;
        if( !strcmp( value, "clockwise" ) )
            return CULL_CLOCKWISE;
        if( !strcmp( value, "anticlockwise" ) )
            return CULL_ANTICLOCKWISE;

        return CULL_CLOCKWISE;
    }
    //-----------------------------------------------------------------------------------
    PolygonMode HlmsJson::parsePolygonMode( const char *value )
    {
        if( !strcmp( value, "points" ) )
            return PM_POINTS;
        if( !strcmp( value, "wireframe" ) )
            return PM_WIREFRAME;
        if( !strcmp( value, "solid" ) )
            return PM_SOLID;

        return PM_SOLID;
    }
    //-----------------------------------------------------------------------------------
    SceneBlendFactor HlmsJson::parseBlendFactor( const char *value )
    {
        if( !strcmp( value, "one" ) )
            return SBF_ONE;
        if( !strcmp( value, "zero" ) )
            return SBF_ZERO;
        if( !strcmp( value, "dst_colour" ) )
            return SBF_DEST_COLOUR;
        if( !strcmp( value, "src_colour" ) )
            return SBF_SOURCE_COLOUR;
        if( !strcmp( value, "one_minus_dst_colour" ) )
            return SBF_ONE_MINUS_DEST_COLOUR;
        if( !strcmp( value, "one_minus_src_colour" ) )
            return SBF_ONE_MINUS_SOURCE_COLOUR;
        if( !strcmp( value, "dst_alpha" ) )
            return SBF_DEST_ALPHA;
        if( !strcmp( value, "src_alpha" ) )
            return SBF_SOURCE_ALPHA;
        if( !strcmp( value, "one_minus_dst_alpha" ) )
            return SBF_ONE_MINUS_DEST_ALPHA;
        if( !strcmp( value, "one_minus_src_alpha" ) )
            return SBF_ONE_MINUS_SOURCE_ALPHA;

        return SBF_ONE;
    }
    //-----------------------------------------------------------------------------------
    SceneBlendOperation HlmsJson::parseBlendOperation( const char *value )
    {
        if( !strcmp( value, "add" ) )
            return SBO_ADD;
        if( !strcmp( value, "subtract" ) )
            return SBO_SUBTRACT;
        if( !strcmp( value, "reverse_subtract" ) )
            return SBO_REVERSE_SUBTRACT;
        if( !strcmp( value, "min" ) )
            return SBO_MIN;
        if( !strcmp( value, "max" ) )
            return SBO_MAX;

        return SBO_ADD;
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadSampler( const rapidjson::Value &samplers, HlmsSamplerblock &samplerblock )
    {
        rapidjson::Value::ConstMemberIterator itor = samplers.FindMember("min");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mMinFilter = parseFilterOptions( itor->value.GetString() );

        itor = samplers.FindMember("mag");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mMagFilter = parseFilterOptions( itor->value.GetString() );

        itor = samplers.FindMember("mip");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mMipFilter = parseFilterOptions( itor->value.GetString() );

        itor = samplers.FindMember("u");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mU = parseTextureAddressingMode( itor->value.GetString() );

        itor = samplers.FindMember("v");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mV = parseTextureAddressingMode( itor->value.GetString() );

        itor = samplers.FindMember("w");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mW = parseTextureAddressingMode( itor->value.GetString() );

        itor = samplers.FindMember("miplodbias");
        if( itor != samplers.MemberEnd() && itor->value.IsNumber() )
            samplerblock.mMipLodBias = static_cast<float>( itor->value.GetDouble() );

        itor = samplers.FindMember("max_anisotropic");
        if( itor != samplers.MemberEnd() && itor->value.IsNumber() )
            samplerblock.mMaxAnisotropy = static_cast<float>( itor->value.GetDouble() );

        itor = samplers.FindMember("compare_function");
        if( itor != samplers.MemberEnd() && itor->value.IsString() )
            samplerblock.mCompareFunction = parseCompareFunction( itor->value.GetString() );

        itor = samplers.FindMember("border");
        if( itor != samplers.MemberEnd() && itor->value.IsArray() )
        {
            const rapidjson::Value& array = itor->value;
            const rapidjson::SizeType arraySize = std::min( 4u, array.Size() );
            for( rapidjson::SizeType i=0; i<arraySize; ++i )
            {
                if( array[i].IsNumber() )
                    samplerblock.mBorderColour[i] = static_cast<float>( array[i].GetDouble() );
            }
        }

        itor = samplers.FindMember("min_lod");
        if( itor != samplers.MemberEnd() && itor->value.IsNumber() )
            samplerblock.mMinLod = static_cast<float>( itor->value.GetDouble() );

        itor = samplers.FindMember("max_lod");
        if( itor != samplers.MemberEnd() && itor->value.IsNumber() )
            samplerblock.mMaxLod = static_cast<float>( itor->value.GetDouble() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadMacroblock( const rapidjson::Value &macroblocksJson, HlmsMacroblock &macroblock )
    {
        rapidjson::Value::ConstMemberIterator itor = macroblocksJson.FindMember("scissor_test");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsBool() )
            macroblock.mScissorTestEnabled = itor->value.GetBool();

        itor = macroblocksJson.FindMember("depth_check");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsBool() )
            macroblock.mDepthCheck = itor->value.GetBool();

        itor = macroblocksJson.FindMember("depth_write");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsBool() )
            macroblock.mDepthWrite = itor->value.GetBool();

        itor = macroblocksJson.FindMember("depth_function");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsString() )
            macroblock.mDepthFunc = parseCompareFunction( itor->value.GetString() );

        itor = macroblocksJson.FindMember("depth_bias_constant");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsNumber() )
            macroblock.mDepthBiasConstant = static_cast<float>( itor->value.GetDouble() );

        itor = macroblocksJson.FindMember("depth_bias_slope_scale");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsNumber() )
            macroblock.mDepthBiasSlopeScale = static_cast<float>( itor->value.GetDouble() );

        itor = macroblocksJson.FindMember("cull_mode");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsString() )
            macroblock.mCullMode = parseCullMode( itor->value.GetString() );

        itor = macroblocksJson.FindMember("polygon_mode");
        if( itor != macroblocksJson.MemberEnd() && itor->value.IsString() )
            macroblock.mPolygonMode = parsePolygonMode( itor->value.GetString() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadBlendblock( const rapidjson::Value &blendblocksJson, HlmsBlendblock &blendblock )
    {
        rapidjson::Value::ConstMemberIterator itor = blendblocksJson.FindMember("alpha_to_coverage");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsBool() )
            blendblock.mAlphaToCoverageEnabled = itor->value.GetBool();

        itor = blendblocksJson.FindMember("blendmask");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
        {
            uint8 mask = 0;
            const char *blendmask = itor->value.GetString();
            if( strchr( blendmask, 'r' ) )
                mask |= HlmsBlendblock::BlendChannelRed;
            if( strchr( blendmask, 'g' ) )
                mask |= HlmsBlendblock::BlendChannelGreen;
            if( strchr( blendmask, 'b' ) )
                mask |= HlmsBlendblock::BlendChannelBlue;
            if( strchr( blendmask, 'a' ) )
                mask |= HlmsBlendblock::BlendChannelAlpha;

            blendblock.mBlendChannelMask = mask;
        }

        itor = blendblocksJson.FindMember("separate_blend");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsBool() )
            blendblock.mSeparateBlend = itor->value.GetBool();

        itor = blendblocksJson.FindMember("src_blend_factor");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mSourceBlendFactor = parseBlendFactor( itor->value.GetString() );

        itor = blendblocksJson.FindMember("dst_blend_factor");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mDestBlendFactor = parseBlendFactor( itor->value.GetString() );

        itor = blendblocksJson.FindMember("src_alpha_blend_factor");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mSourceBlendFactorAlpha = parseBlendFactor( itor->value.GetString() );

        itor = blendblocksJson.FindMember("dst_alpha_blend_factor");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mDestBlendFactorAlpha = parseBlendFactor( itor->value.GetString() );

        itor = blendblocksJson.FindMember("blend_operation");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mBlendOperation = parseBlendOperation( itor->value.GetString() );

        itor = blendblocksJson.FindMember("blend_operation_alpha");
        if( itor != blendblocksJson.MemberEnd() && itor->value.IsString() )
            blendblock.mBlendOperationAlpha = parseBlendOperation( itor->value.GetString() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadDatablockCommon( const rapidjson::Value &json, const NamedBlocks &blocks,
                                        HlmsDatablock *datablock )
    {
        //Support both:
        //  "macroblock" : "unique_name"
        //and:
        //  "macroblock" : ["unique_name", "unique_name_for_shadows"],
        rapidjson::Value::ConstMemberIterator itor = json.FindMember("macroblock");
        if( itor != json.MemberEnd() )
        {
            if( itor->value.IsString() )
            {
                map<LwConstString, const HlmsMacroblock*>::type::const_iterator it =
                        blocks.macroblocks.find( LwConstString( itor->value.GetString(),
                                                                itor->value.GetStringLength() + 1u ) );
                if( it != blocks.macroblocks.end() )
                    datablock->setMacroblock( it->second );
            }
            else if( itor->value.IsArray() )
            {
                const rapidjson::Value& array = itor->value;
                const rapidjson::SizeType arraySize = std::min( 2u, array.Size() );
                for( rapidjson::SizeType i=0; i<arraySize; ++i )
                {
                    if( array[i].IsString() )
                    {
                        map<LwConstString, const HlmsMacroblock*>::type::const_iterator it =
                                blocks.macroblocks.find( LwConstString( array[i].GetString(),
                                                                        array[i].GetStringLength() + 1u ) );
                        if( it != blocks.macroblocks.end() )
                            datablock->setMacroblock( it->second, i == 0 );
                    }
                }
            }
        }

        itor = json.FindMember("blendblock");
        if( itor != json.MemberEnd() )
        {
            if( itor->value.IsString() )
            {
                map<LwConstString, const HlmsBlendblock*>::type::const_iterator it =
                        blocks.blendblocks.find( LwConstString( itor->value.GetString(),
                                                                itor->value.GetStringLength() + 1u ) );
                if( it != blocks.blendblocks.end() )
                    datablock->setBlendblock( it->second );
            }
            else if( itor->value.IsArray() )
            {
                const rapidjson::Value& array = itor->value;
                const rapidjson::SizeType arraySize = std::min( 2u, array.Size() );
                for( rapidjson::SizeType i=0; i<arraySize; ++i )
                {
                    if( array[i].IsString() )
                    {
                        map<LwConstString, const HlmsBlendblock*>::type::const_iterator it =
                                blocks.blendblocks.find(
                                    LwConstString( array[i].GetString(),
                                                   array[i].GetStringLength() + 1u ) );
                        if( it != blocks.blendblocks.end() )
                            datablock->setBlendblock( it->second, i == 0 );
                    }
                }
            }
        }

        itor = json.FindMember("alpha_test");
        if( itor != json.MemberEnd() && itor->value.IsArray() )
        {
            const rapidjson::Value& array = itor->value;
            const rapidjson::SizeType arraySize = array.Size();
            if( arraySize > 0 && array[0].IsString() )
            {
                CompareFunction alphaTestCmp = parseCompareFunction( array[0].GetString() );
                if( alphaTestCmp == NUM_COMPARE_FUNCTIONS )
                    alphaTestCmp = CMPF_ALWAYS_PASS;
                datablock->setAlphaTest( alphaTestCmp );
            }

            if( arraySize > 1 && array[1].IsNumber() )
                datablock->setAlphaTestThreshold( static_cast<float>( array[1].GetDouble() ) );
        }

        itor = json.FindMember("shadow_const_bias");
        if( itor != json.MemberEnd() && itor->value.IsNumber() )
            datablock->mShadowConstantBias = static_cast<float>( itor->value.GetDouble() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadDatablocks( const rapidjson::Value &json, const NamedBlocks &blocks, Hlms *hlms,
                                   const String &filename, const String &resourceGroup,
                                   const String &additionalTextureExtension )
    {
        rapidjson::Value::ConstMemberIterator itor = json.MemberBegin();
        rapidjson::Value::ConstMemberIterator end  = json.MemberEnd();

        while( itor != end )
        {
            if( itor->value.IsObject() )
            {
                const char *datablockName = itor->name.GetString();
                try
                {
                    HlmsDatablock *datablock = hlms->createDatablock( datablockName, datablockName,
                                                                      HlmsMacroblock(), HlmsBlendblock(),
                                                                      HlmsParamVec(), true,
                                                                      filename, resourceGroup );
                    loadDatablockCommon( itor->value, blocks, datablock );

                    hlms->_loadJson( itor->value, blocks, datablock, mListener,
                                     additionalTextureExtension );
                }
                catch( Exception &e )
                {
                    //Ignore datablocks that already exist (useful for reloading materials)
                    if( e.getNumber() != Exception::ERR_DUPLICATE_ITEM )
                        throw e;
                    else
                        LogManager::getSingleton().logMessage( e.getFullDescription() );
                }
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadMaterials( const String &filename, const String &resourceGroup,
                                  const char *jsonString,
                                  const String &additionalTextureExtension )
    {
        rapidjson::Document d;
        d.Parse( jsonString );

        if( d.HasParseError() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "HlmsJson::loadMaterials",
                         "Invalid JSON string in file " + filename );
        }

        NamedBlocks blocks;

        //Load samplerblocks
        rapidjson::Value::ConstMemberIterator itor = d.FindMember("samplers");
        if( itor != d.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &samplers = itor->value;

            rapidjson::Value::ConstMemberIterator itSampler = samplers.MemberBegin();
            rapidjson::Value::ConstMemberIterator enSampler = samplers.MemberEnd();

            while( itSampler != enSampler )
            {
                HlmsSamplerblock samplerblock;
                loadSampler( itSampler->value, samplerblock );

                LwConstString keyName( LwConstString( itSampler->name.GetString(),
                                                      itSampler->name.GetStringLength() + 1u ) );

                blocks.samplerblocks[keyName] = mHlmsManager->getSamplerblock( samplerblock );

                ++itSampler;
            }
        }

        //Load macroblocks
        itor = d.FindMember("macroblocks");
        if( itor != d.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &macroblocksJson = itor->value;

            rapidjson::Value::ConstMemberIterator itMacros = macroblocksJson.MemberBegin();
            rapidjson::Value::ConstMemberIterator enMacros = macroblocksJson.MemberEnd();

            while( itMacros != enMacros )
            {
                HlmsMacroblock macroblock;
                loadMacroblock( itMacros->value, macroblock );

                LwConstString keyName( LwConstString( itMacros->name.GetString(),
                                                      itMacros->name.GetStringLength() + 1u ) );

                blocks.macroblocks[keyName] = mHlmsManager->getMacroblock( macroblock );

                ++itMacros;
            }
        }

        //Load blendblocks
        itor = d.FindMember("blendblocks");
        if( itor != d.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &blendblocksJson = itor->value;

            rapidjson::Value::ConstMemberIterator itBlends = blendblocksJson.MemberBegin();
            rapidjson::Value::ConstMemberIterator enBlends = blendblocksJson.MemberEnd();

            while( itBlends != enBlends )
            {
                HlmsBlendblock blendblock;
                loadBlendblock( itBlends->value, blendblock );

                LwConstString keyName( LwConstString( itBlends->name.GetString(),
                                                      itBlends->name.GetStringLength() + 1u ) );

                blocks.blendblocks[keyName] = mHlmsManager->getBlendblock( blendblock );

                ++itBlends;
            }
        }

        rapidjson::Value::ConstMemberIterator itDatablock = d.MemberBegin();
        rapidjson::Value::ConstMemberIterator enDatablock = d.MemberEnd();

        while( itDatablock != enDatablock )
        {
            const IdString typeName( itDatablock->name.GetString() );

            for( int i=0; i<HLMS_MAX; ++i )
            {
                Hlms *hlms = mHlmsManager->getHlms( static_cast<HlmsTypes>( i ) );

                if( hlms && typeName == hlms->getTypeName() )
                {
                    loadDatablocks( itDatablock->value, blocks, hlms, filename, resourceGroup,
                                    additionalTextureExtension );
                }
            }

            if( typeName == "compute" )
            {
                HlmsJsonCompute jsonCompute( mHlmsManager );
                jsonCompute.loadJobs( itDatablock->value, blocks );
            }

            ++itDatablock;
        }

        {
            map<LwConstString, const HlmsMacroblock*>::type::const_iterator it =
                    blocks.macroblocks.begin();
            map<LwConstString, const HlmsMacroblock*>::type::const_iterator en =
                    blocks.macroblocks.end();

            while( it != en )
            {
                mHlmsManager->destroyMacroblock( it->second );
                ++it;
            }

            blocks.macroblocks.clear();
        }
        {
            map<LwConstString, const HlmsBlendblock*>::type::const_iterator it =
                    blocks.blendblocks.begin();
            map<LwConstString, const HlmsBlendblock*>::type::const_iterator en =
                    blocks.blendblocks.end();

            while( it != en )
            {
                mHlmsManager->destroyBlendblock( it->second );
                ++it;
            }

            blocks.blendblocks.clear();
        }
        {
            map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
                    blocks.samplerblocks.begin();
            map<LwConstString, const HlmsSamplerblock*>::type::const_iterator en =
                    blocks.samplerblocks.end();

            while( it != en )
            {
                mHlmsManager->destroySamplerblock( it->second );
                ++it;
            }

            blocks.samplerblocks.clear();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( FilterOptions value, String &outString )
    {
        outString += '"';
        outString += c_filterOptions[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( TextureAddressingMode value, String &outString )
    {
        outString += '"';
        if( value == TAM_UNKNOWN )
            value = TAM_WRAP;
        outString += c_textureAddressingMode[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( CompareFunction value, String &outString )
    {
        outString += '"';
        outString += c_compareFunctions[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( CullingMode value, String &outString )
    {
        outString += '"';
        outString += c_cullModes[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( PolygonMode value, String &outString )
    {
        outString += '"';
        outString += c_polygonModes[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( SceneBlendFactor value, String &outString )
    {
        outString += '"';
        outString += c_sceneBlendFactor[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toQuotedStr( SceneBlendOperation value, String &outString )
    {
        outString += '"';
        outString += c_sceneBlendOperation[value];
        outString += '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toStr( const ColourValue &value, String &outString )
    {
        outString += '[';
        outString += StringConverter::toString( value.r );
        outString += ", ";
        outString += StringConverter::toString( value.g );
        outString += ", ";
        outString += StringConverter::toString( value.b );
        outString += ", ";
        outString += StringConverter::toString( value.a );
        outString += ']';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toStr( const Vector2 &value, String &outString )
    {
        outString += '[';
        outString += StringConverter::toString( value.x );
        outString += ", ";
        outString += StringConverter::toString( value.y );
        outString += ']';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toStr( const Vector3 &value, String &outString )
    {
        outString += '[';
        outString += StringConverter::toString( value.x );
        outString += ", ";
        outString += StringConverter::toString( value.y );
        outString += ", ";
        outString += StringConverter::toString( value.z );
        outString += ']';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::toStr( const Vector4 &value, String &outString )
    {
        outString += '[';
        outString += StringConverter::toString( value.x );
        outString += ", ";
        outString += StringConverter::toString( value.y );
        outString += ", ";
        outString += StringConverter::toString( value.z );
        outString += ", ";
        outString += StringConverter::toString( value.w );
        outString += ']';
    }
    //-----------------------------------------------------------------------------------
    String HlmsJson::getName( const HlmsMacroblock *macroblock ) const
    {
        return "\"Macroblock_" + StringConverter::toString( macroblock->mId ) + '"';
    }
    //-----------------------------------------------------------------------------------
    String HlmsJson::getName( const HlmsBlendblock *blendblock ) const
    {
        return "\"Blendblock_" + StringConverter::toString( blendblock->mId ) + '"';
    }
    //-----------------------------------------------------------------------------------
    String HlmsJson::getName( const HlmsSamplerblock *samplerblock )
    {
        return "\"Sampler_" + StringConverter::toString( samplerblock->mId ) + '"';
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveSamplerblock( const HlmsSamplerblock *samplerblock, String &outString )
    {
        outString += "\n\t\t";
        outString += getName( samplerblock );
        outString += " :\n\t\t{\n";

        outString += "\t\t\t\"min\" : ";
        toQuotedStr( samplerblock->mMinFilter, outString );

        outString += ",\n\t\t\t\"mag\" : ";
        toQuotedStr( samplerblock->mMagFilter, outString );

        outString += ",\n\t\t\t\"mip\" : ";
        toQuotedStr( samplerblock->mMipFilter, outString );

        outString += ",\n\t\t\t\"u\" : ";
        toQuotedStr( samplerblock->mU, outString );

        outString += ",\n\t\t\t\"v\" : ";
        toQuotedStr( samplerblock->mV, outString );

        outString += ",\n\t\t\t\"w\" : ";
        toQuotedStr( samplerblock->mW, outString );

        outString += ",\n\t\t\t\"miplodbias\" : ";
        outString += StringConverter::toString( samplerblock->mMipLodBias );

        outString += ",\n\t\t\t\"max_anisotropic\" : ";
        outString += StringConverter::toString( samplerblock->mMaxAnisotropy );

        outString += ",\n\t\t\t\"compare_function\" : ";
        toQuotedStr( samplerblock->mCompareFunction, outString );

        outString += ",\n\t\t\t\"border\" :";
        toStr( samplerblock->mBorderColour, outString );

        outString += ",\n\t\t\t\"min_lod\" : ";
        outString += StringConverter::toString( samplerblock->mMinLod );

        outString += ",\n\t\t\t\"max_lod\" : ";
        outString += StringConverter::toString( samplerblock->mMaxLod );

        outString += "\n\t\t},";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveMacroblock( const HlmsMacroblock *macroblock, String &outString )
    {
        outString += "\n\t\t";
        outString += getName( macroblock );
        outString += " :\n\t\t{\n";

        outString += "\t\t\t\"scissor_test\" : ";
        outString += macroblock->mScissorTestEnabled ? "true" : "false";

        outString += ",\n\t\t\t\"depth_check\" : ";
        outString += macroblock->mDepthCheck ? "true" : "false";

        outString += ",\n\t\t\t\"depth_write\" : ";
        outString += macroblock->mDepthWrite ? "true" : "false";

        outString += ",\n\t\t\t\"depth_function\" : ";
        toQuotedStr( macroblock->mDepthFunc, outString );

        outString += ",\n\t\t\t\"depth_bias_constant\" : ";
        outString += StringConverter::toString( macroblock->mDepthBiasConstant );

        outString += ",\n\t\t\t\"depth_bias_slope_scale\" : ";
        outString += StringConverter::toString( macroblock->mDepthBiasSlopeScale );

        outString += ",\n\t\t\t\"cull_mode\" : ";
        toQuotedStr( macroblock->mCullMode, outString );

        outString += ",\n\t\t\t\"polygon_mode\" : ";
        toQuotedStr( macroblock->mPolygonMode, outString );

        outString += "\n\t\t},";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveBlendblock( const HlmsBlendblock *blendblock, String &outString )
    {
        outString += "\n\t\t";
        outString += getName( blendblock );
        outString += " :\n\t\t{\n";

        outString += "\t\t\t\"alpha_to_coverage\" : ";
        outString += blendblock->mAlphaToCoverageEnabled ? "true" : "false";

        outString += ",\n\t\t\t\"blendmask\" : \"";
        if( blendblock->mBlendChannelMask & HlmsBlendblock::BlendChannelRed )
            outString += 'r';
        if( blendblock->mBlendChannelMask & HlmsBlendblock::BlendChannelGreen )
            outString += 'g';
        if( blendblock->mBlendChannelMask & HlmsBlendblock::BlendChannelBlue )
            outString += 'b';
        if( blendblock->mBlendChannelMask & HlmsBlendblock::BlendChannelAlpha )
            outString += 'a';
        outString += "\"";

        outString += ",\n\t\t\t\"separate_blend\" : ";
        outString += blendblock->mSeparateBlend ? "true" : "false";

        outString += ",\n\t\t\t\"src_blend_factor\" : ";
        toQuotedStr( blendblock->mSourceBlendFactor, outString );

        outString += ",\n\t\t\t\"dst_blend_factor\" : ";
        toQuotedStr( blendblock->mDestBlendFactor, outString );

        if( blendblock->mSeparateBlend )
        {
            outString += ",\n\t\t\t\"src_alpha_blend_factor\" : ";
            toQuotedStr( blendblock->mSourceBlendFactorAlpha, outString );

            outString += ",\n\t\t\t\"dst_alpha_blend_factor\" : ";
            toQuotedStr( blendblock->mDestBlendFactorAlpha, outString );
        }

        outString += ",\n\t\t\t\"blend_operation\" : ";
        toQuotedStr( blendblock->mBlendOperation, outString );

        if( blendblock->mSeparateBlend )
        {
            outString += ",\n\t\t\t\"blend_operation_alpha\" : ";
            toQuotedStr( blendblock->mBlendOperationAlpha, outString );
        }

        outString += "\n\t\t},";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveDatablock( const String &fullName, const HlmsDatablock *datablock,
                                  String &outString, const String &additionalTextureExtension )
    {
        outString += "\n\n\t\t\"";
        outString += fullName;
        outString += "\" :\n\t\t{\n";

        outString += "\t\t\t\"macroblock\" : ";

        if( datablock->hasCustomShadowMacroblock() )
        {
            outString += '[';
            outString += getName( datablock->getMacroblock(false) );
            outString += ", ";
            outString += getName( datablock->getMacroblock(true) );
            outString += ']';
        }
        else
        {
            outString += getName( datablock->getMacroblock() );
        }

        outString += ",\n\t\t\t\"blendblock\" : ";

        if( datablock->getBlendblock( false ) != datablock->getBlendblock( true ) )
        {
            outString += '[';
            outString += getName( datablock->getBlendblock(false) );
            outString += ", ";
            outString += getName( datablock->getBlendblock(true) );
            outString += ']';
        }
        else
        {
            outString += getName( datablock->getBlendblock() );
        }

        if( datablock->getAlphaTest() != CMPF_ALWAYS_PASS )
        {
            outString += ",\n\t\t\t\"alpha_test\" : ";
            outString += '[';
            toQuotedStr( datablock->getAlphaTest(), outString );
            outString += ", ";
            outString += StringConverter::toString( datablock->getAlphaTestThreshold() );
            outString += ']';
        }

        outString += ",\n\t\t\t\"shadow_const_bias\" : ";
        outString += StringConverter::toString( datablock->mShadowConstantBias );

        const Hlms *hlms = datablock->getCreator();
        hlms->_saveJson( datablock, outString, mListener, additionalTextureExtension );

        outString += "\n\t\t},";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveMaterials( const Hlms *hlms, String &outString,
                                  const String &additionalTextureExtension )
    {
        outString += "{";

        const Hlms::HlmsDatablockMap &datablockMap = hlms->getDatablockMap();

        set<const HlmsMacroblock*>::type macroblocks;
        set<const HlmsBlendblock*>::type blendblocks;
        set<const HlmsSamplerblock*>::type samplerblocks;

        {
            Hlms::HlmsDatablockMap::const_iterator itor = datablockMap.begin();
            Hlms::HlmsDatablockMap::const_iterator end  = datablockMap.end();

            while( itor != end )
            {
                const HlmsDatablock *datablock = itor->second.datablock;

                const HlmsMacroblock *macroblock = datablock->getMacroblock( false );
                macroblocks.insert( macroblock );

                if( datablock->hasCustomShadowMacroblock() )
                    macroblocks.insert( datablock->getMacroblock( true ) );

                const HlmsBlendblock *blendblock = datablock->getBlendblock( false );
                blendblocks.insert( blendblock );

                const HlmsBlendblock *blendblockCaster = datablock->getBlendblock( true );
                if( blendblock != blendblockCaster )
                    blendblocks.insert( blendblockCaster );

                hlms->_collectSamplerblocks( samplerblocks, datablock );

                ++itor;
            }
        }

        {
            set<const HlmsSamplerblock*>::type::const_iterator itor = samplerblocks.begin();
            set<const HlmsSamplerblock*>::type::const_iterator end  = samplerblocks.end();

            if( !samplerblocks.empty() )
                outString += "\n\t\"samplers\" :\n\t{";

            while( itor != end )
                saveSamplerblock( *itor++, outString );

            if( !samplerblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            set<const HlmsMacroblock*>::type::const_iterator itor = macroblocks.begin();
            set<const HlmsMacroblock*>::type::const_iterator end  = macroblocks.end();

            if( !macroblocks.empty() )
                outString += "\n\n\t\"macroblocks\" :\n\t{";

            while( itor != end )
                saveMacroblock( *itor++, outString );

            if( !macroblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            set<const HlmsBlendblock*>::type::const_iterator itor = blendblocks.begin();
            set<const HlmsBlendblock*>::type::const_iterator end  = blendblocks.end();

            if( !blendblocks.empty() )
                outString += "\n\n\t\"blendblocks\" :\n\t{";

            while( itor != end )
                saveBlendblock( *itor++, outString );

            if( !blendblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            const size_t numDatablocks = datablockMap.size();
            if( numDatablocks > 1u )
            {
                outString += "\n\n\t\"";
                outString += hlms->getTypeNameStr();
                outString += "\" : \n\t{";
            }

            const HlmsDatablock *defaultDatablock = hlms->getDefaultDatablock();

            Hlms::HlmsDatablockMap::const_iterator itor = datablockMap.begin();
            Hlms::HlmsDatablockMap::const_iterator end  = datablockMap.end();

            while( itor != end )
            {
                const HlmsDatablock *datablock = itor->second.datablock;

                if( datablock != defaultDatablock )
                    saveDatablock( itor->second.name, datablock, outString, additionalTextureExtension );
                ++itor;
            }

            if( numDatablocks > 1u )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        outString.erase( outString.size() - 1 ); //Remove an extra comma
        if( !outString.empty() )
            outString += "\n}";
        else
            outString += "{}";
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::saveMaterial( const HlmsDatablock *datablock, String &outString,
                                 const String &additionalTextureExtension )
    {
        outString += "{";

        const Hlms *hlms = datablock->getCreator();

        set<const HlmsMacroblock*>::type macroblocks;
        set<const HlmsBlendblock*>::type blendblocks;
        set<const HlmsSamplerblock*>::type samplerblocks;

        {
            const HlmsMacroblock *macroblock = datablock->getMacroblock( false );
            macroblocks.insert( macroblock );

            if( datablock->hasCustomShadowMacroblock() )
                macroblocks.insert( datablock->getMacroblock( true ) );

            const HlmsBlendblock *blendblock = datablock->getBlendblock( false );
            blendblocks.insert( blendblock );

            const HlmsBlendblock *blendblockCaster = datablock->getBlendblock( true );
            if( blendblock != blendblockCaster )
                blendblocks.insert( blendblockCaster );

            hlms->_collectSamplerblocks( samplerblocks, datablock );
        }

        {
            set<const HlmsSamplerblock*>::type::const_iterator itor = samplerblocks.begin();
            set<const HlmsSamplerblock*>::type::const_iterator end  = samplerblocks.end();

            if( !samplerblocks.empty() )
                outString += "\n\t\"samplers\" :\n\t{";

            while( itor != end )
                saveSamplerblock( *itor++, outString );

            if( !samplerblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            set<const HlmsMacroblock*>::type::const_iterator itor = macroblocks.begin();
            set<const HlmsMacroblock*>::type::const_iterator end  = macroblocks.end();

            if( !macroblocks.empty() )
                outString += "\n\n\t\"macroblocks\" :\n\t{";

            while( itor != end )
                saveMacroblock( *itor++, outString );

            if( !macroblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            set<const HlmsBlendblock*>::type::const_iterator itor = blendblocks.begin();
            set<const HlmsBlendblock*>::type::const_iterator end  = blendblocks.end();

            if( !blendblocks.empty() )
                outString += "\n\n\t\"blendblocks\" :\n\t{";

            while( itor != end )
                saveBlendblock( *itor++, outString );

            if( !blendblocks.empty() )
            {
                outString.erase( outString.size() - 1 ); //Remove an extra comma
                outString += "\n\t},";
            }
        }

        {
            outString += "\n\n\t\"";
            outString += hlms->getTypeNameStr();
            outString += "\" : \n\t{";

            String datablockName = "[Unnamed]";

            {
                const String *fullName = datablock->getNameStr();
                if( fullName )
                    datablockName = *fullName;
            }

            saveDatablock( datablockName, datablock, outString, additionalTextureExtension );

            outString.erase( outString.size() - 1 ); //Remove an extra comma
            outString += "\n\t},";
        }

        outString.erase( outString.size() - 1 ); //Remove an extra comma
        if( !outString.empty() )
            outString += "\n}";
        else
            outString += "{}";
    }
}
#endif
