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
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreLogManager.h"

#include "rapidjson/document.h"

namespace Ogre
{
    HlmsJson::HlmsJson( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager )
    {
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
            return CMPF_GREATER;
        if( !strcmp( value, "greater" ) )
            return CMPF_GREATER_EQUAL;
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
                        blocks.macroblocks.find( LwConstString::FromUnsafeCStr(
                                                     itor->value.GetString() ) );
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
                                blocks.macroblocks.find( LwConstString::FromUnsafeCStr(
                                                             array[i].GetString() ) );
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
                        blocks.blendblocks.find( LwConstString::FromUnsafeCStr(
                                                     itor->value.GetString() ) );
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
                                blocks.blendblocks.find( LwConstString::FromUnsafeCStr(
                                                             array[i].GetString() ) );
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
                datablock->setAlphaTest( parseCompareFunction( array[0].GetString() ) );

            if( arraySize > 1 && array[1].IsNumber() )
                datablock->setAlphaTestThreshold( static_cast<float>( array[1].GetDouble() ) );
        }

        itor = json.FindMember("shadow_const_bias");
        if( itor != json.MemberEnd() && itor->value.IsNumber() )
            datablock->mShadowConstantBias = static_cast<float>( itor->value.GetDouble() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadDatablocks( const rapidjson::Value &json, const NamedBlocks &blocks, Hlms *hlms )
    {
        rapidjson::Value::ConstMemberIterator itor = json.MemberBegin();
        rapidjson::Value::ConstMemberIterator end  = json.MemberEnd();

        while( itor != end )
        {
            if( itor->value.IsObject() )
            {
                const char *datablockName = itor->name.GetString();
                HlmsDatablock *datablock = hlms->createDatablock( datablockName, datablockName,
                                                                  HlmsMacroblock(), HlmsBlendblock(),
                                                                  HlmsParamVec() );
                loadDatablockCommon( itor->value, blocks, datablock );

                hlms->loadJson( itor->value, blocks, datablock );
            }

            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJson::loadMaterials( const String &filename, const char *jsonString )
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

                LwConstString keyName( LwConstString::FromUnsafeCStr(itSampler->name.GetString()) );

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

                LwConstString keyName( LwConstString::FromUnsafeCStr(itMacros->name.GetString()) );

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

                LwConstString keyName( LwConstString::FromUnsafeCStr(itBlends->name.GetString()) );

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
                    loadDatablocks( itDatablock->value, blocks, hlms );
                }
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
}
#endif
