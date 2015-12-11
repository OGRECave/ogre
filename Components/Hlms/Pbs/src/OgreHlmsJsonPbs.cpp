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

#ifdef OGRE_USE_JSON

#include "OgreHlmsJsonPbs.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"

#include "rapidjson/document.h"

namespace Ogre
{
    extern const String c_pbsBlendModes[];

    HlmsJsonPbs::HlmsJsonPbs( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsDatablock::Workflows HlmsJsonPbs::parseWorkflow( const char *value )
    {
        if( !strcmp( value, "specular_ogre" ) )
            return HlmsPbsDatablock::SpecularWorkflow;
        if( !strcmp( value, "specular_fresnel" ) )
            return HlmsPbsDatablock::SpecularAsFresnelWorkflow;
        if( !strcmp( value, "metallic" ) )
            return HlmsPbsDatablock::MetallicWorkflow;

        return HlmsPbsDatablock::SpecularWorkflow;
    }
    //-----------------------------------------------------------------------------------
    HlmsPbsDatablock::TransparencyModes HlmsJsonPbs::parseTransparencyMode( const char *value )
    {
        if( !strcmp( value, "None" ) )
            return HlmsPbsDatablock::None;
        if( !strcmp( value, "Transparent" ) )
            return HlmsPbsDatablock::Transparent;
        if( !strcmp( value, "Fade" ) )
            return HlmsPbsDatablock::Fade;

        return HlmsPbsDatablock::None;
    }
    //-----------------------------------------------------------------------------------
    PbsBlendModes HlmsJsonPbs::parseBlendMode( const char *value )
    {
        for( int i=0; i<NUM_PBSM_BLEND_MODES; ++i )
        {
            if( !strcmp( value, c_pbsBlendModes[i].c_str() ) )
                return static_cast<PbsBlendModes>( i );
        }

        return PBSM_BLEND_NORMAL_NON_PREMUL;
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonPbs::parseFresnelMode( const char *value, bool &outIsColoured, bool &outUseIOR )
    {
        if( !strcmp( value, "coeff" ) )
        {
            outUseIOR       = false;
            outIsColoured   = false;
        }
        else if( !strcmp( value, "ior" ) )
        {
            outUseIOR       = true;
            outIsColoured   = false;
        }
        else if( !strcmp( value, "coloured" ) )
        {
            outUseIOR       = false;
            outIsColoured   = true;
        }
        else if( !strcmp( value, "coloured_ior" ) )
        {
            outUseIOR       = true;
            outIsColoured   = true;
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonPbs::parseOffset( const rapidjson::Value &jsonArray, Vector4 offsetScale )
    {
        const rapidjson::SizeType arraySize = std::min( 2u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                offsetScale[i] = static_cast<float>( jsonArray[i].GetDouble() );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonPbs::parseScale( const rapidjson::Value &jsonArray, Vector4 offsetScale )
    {
        const rapidjson::SizeType arraySize = std::min( 2u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                offsetScale[i+2u] = static_cast<float>( jsonArray[i].GetDouble() );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonPbs::loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                   PbsTextureTypes textureType, HlmsPbsDatablock *datablock,
                                   PackedTexture textures[] )
    {
        const HlmsTextureManager::TextureMapType texMapTypes[NUM_PBSM_TEXTURE_TYPES] =
        {
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_NORMALS,
            HlmsTextureManager::TEXTURE_TYPE_DIFFUSE,
            HlmsTextureManager::TEXTURE_TYPE_MONOCHROME,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_DETAIL_NORMAL_MAP,
            HlmsTextureManager::TEXTURE_TYPE_ENV_MAP
        };

        rapidjson::Value::ConstMemberIterator itor = json.FindMember("texture");
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();

            const char *textureName = itor->value.GetString();
            HlmsTextureManager::TextureLocation texLocation =
                    hlmsTextureManager->createOrRetrieveTexture( textureName, texMapTypes[textureType] );
            textures[textureType].texture = texLocation.texture;
            textures[textureType].xIdx = texLocation.xIdx;
        }

        itor = json.FindMember("sampler");
        if( itor != json.MemberEnd() && itor->value.IsString() )
        {
            map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
                    blocks.samplerblocks.find( LwConstString::FromUnsafeCStr(itor->value.GetString()) );
            if( it != blocks.samplerblocks.end() )
            {
                textures[textureType].samplerblock = it->second;
                mHlmsManager->addReference( textures[textureType].samplerblock );
            }
        }

        itor = json.FindMember("uv");
        if( itor != json.MemberEnd() && itor->value.IsUint() )
        {
            unsigned uv = itor->value.GetUint();
            datablock->setTextureUvSource( textureType, static_cast<uint8>( uv ) );
        }
    }
    //-----------------------------------------------------------------------------------
    inline Vector3 HlmsJsonPbs::parseVector3Array( const rapidjson::Value &jsonArray )
    {
        Vector3 retVal( Vector3::ZERO );

        const rapidjson::SizeType arraySize = std::min( 3u, jsonArray.Size() );
        for( rapidjson::SizeType i=0; i<arraySize; ++i )
        {
            if( jsonArray[i].IsNumber() )
                retVal[i] = static_cast<float>( jsonArray[i].GetDouble() );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsJsonPbs::loadMaterial( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                                    HlmsDatablock *datablock )
    {
        assert( dynamic_cast<HlmsPbsDatablock*>(datablock) );
        HlmsPbsDatablock *pbsDatablock = static_cast<HlmsPbsDatablock*>(datablock);

        rapidjson::Value::ConstMemberIterator itor = json.FindMember("workflow");
        if( itor != json.MemberEnd() && itor->value.IsString() )
            pbsDatablock->setWorkflow( parseWorkflow( itor->value.GetString() ) );

        itor = json.FindMember("transparency");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;

            float transparencyValue = pbsDatablock->getTransparency();
            HlmsPbsDatablock::TransparencyModes transpMode = pbsDatablock->getTransparencyMode();
            bool useAlphaFromTextures = pbsDatablock->getUseAlphaFromTextures();

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                transparencyValue = static_cast<float>( itor->value.GetDouble() );

            itor = subobj.FindMember( "mode" );
            if( itor != subobj.MemberEnd() && itor->value.IsString() )
                transpMode = parseTransparencyMode( itor->value.GetString() );

            itor = subobj.FindMember( "use_alpha_from_textures" );
            if( itor != subobj.MemberEnd() && itor->value.IsBool() )
                useAlphaFromTextures = itor->value.GetBool();

            const bool changeBlendblock = !json.HasMember( "blendblock" );
            pbsDatablock->setTransparency( transparencyValue, transpMode,
                                           useAlphaFromTextures, changeBlendblock );
        }

        PackedTexture packedTextures[NUM_PBSM_TEXTURE_TYPES];

        itor = json.FindMember("diffuse");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_DIFFUSE, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                pbsDatablock->setDiffuse( parseVector3Array( itor->value ) );
        }

        itor = json.FindMember("specular");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_SPECULAR, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                pbsDatablock->setSpecular( parseVector3Array( itor->value ) );
        }

        itor = json.FindMember("roughness");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_ROUGHNESS, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setRoughness( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("fresnel");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_SPECULAR, pbsDatablock, packedTextures );

            bool useIOR = false;
            bool isColoured = false;
            itor = subobj.FindMember( "mode" );
            if( itor != subobj.MemberEnd() && itor->value.IsString() )
                parseFresnelMode( itor->value.GetString(), isColoured, useIOR );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsArray() )
            {
                if( !useIOR )
                    pbsDatablock->setFresnel( parseVector3Array( itor->value ), isColoured );
                else
                    pbsDatablock->setIndexOfRefraction( parseVector3Array( itor->value ), isColoured );
            }
        }

        itor = json.FindMember("metallness");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_METALLIC, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setMetallness( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("normal");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_NORMAL, pbsDatablock, packedTextures );

            itor = subobj.FindMember( "value" );
            if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                pbsDatablock->setNormalMapWeight( static_cast<float>( itor->value.GetDouble() ) );
        }

        itor = json.FindMember("detail_weight");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_DETAIL_WEIGHT, pbsDatablock, packedTextures );
        }

        for( int i=0; i<4; ++i )
        {
            const String iAsStr = StringConverter::toString(i);
            String texTypeName = "detail_diffuse" + iAsStr;

            itor = json.FindMember(texTypeName.c_str());
            if( itor != json.MemberEnd() && itor->value.IsObject() )
            {
                const rapidjson::Value &subobj = itor->value;
                loadTexture( subobj, blocks, static_cast<PbsTextureTypes>(PBSM_DETAIL0 + i),
                             pbsDatablock, packedTextures );

                itor = subobj.FindMember( "value" );
                if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                    pbsDatablock->setDetailMapWeight( i, static_cast<float>( itor->value.GetDouble() ) );

                itor = subobj.FindMember( "mode" );
                if( itor != subobj.MemberEnd() && itor->value.IsString() )
                    pbsDatablock->setDetailMapBlendMode( i, parseBlendMode( itor->value.GetString() ) );

                Vector4 offsetScale( 0, 0, 1, 1 );

                itor = subobj.FindMember( "offset" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseOffset( itor->value, offsetScale );

                itor = subobj.FindMember( "scale" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseScale( itor->value, offsetScale );

                pbsDatablock->setDetailMapOffsetScale( i, offsetScale );
            }

            texTypeName = "detail_normal" + iAsStr;
            itor = json.FindMember(texTypeName.c_str());
            if( itor != json.MemberEnd() && itor->value.IsObject() )
            {
                const rapidjson::Value &subobj = itor->value;
                loadTexture( subobj, blocks, static_cast<PbsTextureTypes>(PBSM_DETAIL0_NM + i),
                             pbsDatablock, packedTextures );

                itor = subobj.FindMember( "value" );
                if( itor != subobj.MemberEnd() && itor->value.IsNumber() )
                {
                    pbsDatablock->setDetailNormalWeight( i,
                                                         static_cast<float>( itor->value.GetDouble() ) );
                }

                Vector4 offsetScale( 0, 0, 1, 1 );

                itor = subobj.FindMember( "offset" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseOffset( itor->value, offsetScale );

                itor = subobj.FindMember( "scale" );
                if( itor != subobj.MemberEnd() && itor->value.IsArray() )
                    parseScale( itor->value, offsetScale );

                pbsDatablock->setDetailMapOffsetScale( i + 4, offsetScale );
            }
        }

        itor = json.FindMember("reflection");
        if( itor != json.MemberEnd() && itor->value.IsObject() )
        {
            const rapidjson::Value &subobj = itor->value;
            loadTexture( subobj, blocks, PBSM_REFLECTION, pbsDatablock, packedTextures );
        }

        pbsDatablock->_setTextures( packedTextures );
    }
}

#endif
