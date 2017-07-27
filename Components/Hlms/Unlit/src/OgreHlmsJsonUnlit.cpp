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

#include "OgreHlmsJsonUnlit.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreTextureManager.h"

#include "OgreLwString.h"

#include "OgreStringConverter.h"

#include "rapidjson/document.h"

namespace Ogre
{
	extern const String c_unlitBlendModes[];

    HlmsJsonUnlit::HlmsJsonUnlit( HlmsManager *hlmsManager ) :
        mHlmsManager( hlmsManager )
    {
    }
	//-----------------------------------------------------------------------------------
	UnlitBlendModes HlmsJsonUnlit::parseBlendMode( const char *value )
	{
		for ( int i = 0; i<NUM_UNLIT_BLEND_MODES; ++i )
		{
			if ( !strcmp(value, c_unlitBlendModes[i].c_str()) )
				return static_cast<UnlitBlendModes>(i);
		}

		return UNLIT_BLEND_NORMAL_NON_PREMUL;
	}
	//-----------------------------------------------------------------------------------
	void HlmsJsonUnlit::parseAnimation( const rapidjson::Value &jsonArray, Matrix4 &mat )
	{
		const rapidjson::SizeType arraySize = std::min( 4u, jsonArray.Size() );
		for ( rapidjson::SizeType i = 0; i<arraySize; ++i )
		{
			const rapidjson::Value& jasonInnerArray = jsonArray[i];
			if ( jasonInnerArray.IsArray() )
			{
				const rapidjson::SizeType innerArraySize = std::min( 4u, jasonInnerArray.Size() );
				for ( rapidjson::SizeType j = 0; j < innerArraySize; ++j )
				{
					if ( jasonInnerArray[j].IsNumber() )
						mat[i][j] = static_cast<float>( jasonInnerArray[j].GetDouble() );
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsJsonUnlit::loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
		uint8 textureType, HlmsUnlitDatablock *datablock )
	{
		bool setTex = false;
		HlmsTextureManager::TextureLocation texLocation;
		HlmsSamplerblock const * samplerBlock = 0;
		rapidjson::Value::ConstMemberIterator itor = json.FindMember( "texture" );
		if ( itor != json.MemberEnd() && itor->value.IsString() )
		{
			const char *textureName = itor->value.GetString();

			HlmsTextureManager *hlmsTextureManager = mHlmsManager->getTextureManager();
			texLocation = hlmsTextureManager->createOrRetrieveTexture( textureName, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE );
			setTex = true;
		}

		itor = json.FindMember( "sampler" );
		if ( itor != json.MemberEnd() && itor->value.IsString() )
		{
			map<LwConstString, const HlmsSamplerblock*>::type::const_iterator it =
				blocks.samplerblocks.find( LwConstString::FromUnsafeCStr( itor->value.GetString()) );
			if ( it != blocks.samplerblocks.end() )
			{
				samplerBlock = it->second;
				mHlmsManager->addReference( samplerBlock );
			}
		}

		itor = json.FindMember( "blendmode" );
		if ( itor != json.MemberEnd() && itor->value.IsString() )
			datablock->setBlendMode( textureType, parseBlendMode( itor->value.GetString()) );

		itor = json.FindMember( "uv" );
		if ( itor != json.MemberEnd() && itor->value.IsUint() )
		{
			unsigned uv = itor->value.GetUint();
			datablock->setTextureUvSource( textureType, static_cast<uint8>(uv) );
		}

		Matrix4 mat = Matrix4::IDENTITY;
		itor = json.FindMember( "animate" );
		if ( itor != json.MemberEnd() && itor->value.IsArray() )
		{
			parseAnimation( itor->value, mat );
			datablock->setEnableAnimationMatrix( textureType, true );
			datablock->setAnimationMatrix( textureType, mat );
		}

		if ( setTex )
			datablock->setTexture( textureType, texLocation.xIdx, texLocation.texture, samplerBlock );
	}
	//-----------------------------------------------------------------------------------
	void HlmsJsonUnlit::loadMaterial(const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
		HlmsDatablock *datablock)
	{
		assert(dynamic_cast<HlmsUnlitDatablock*>(datablock));
		HlmsUnlitDatablock *unlitDatablock = static_cast<HlmsUnlitDatablock*>(datablock);

		rapidjson::Value::ConstMemberIterator itor = json.FindMember("diffuse");
		if (itor != json.MemberEnd() && itor->value.IsArray())
		{
			const rapidjson::Value& array = itor->value;
			const rapidjson::SizeType arraySize = std::min(4u, array.Size());
			ColourValue col;
			for (rapidjson::SizeType i = 0; i<arraySize; ++i)
			{
				if (array[i].IsNumber())
					col[i] = static_cast<float>(array[i].GetDouble());
			}
			unlitDatablock->setUseColour(true);
			unlitDatablock->setColour(col);
		}

		for (uint8 i = 0; i<NUM_UNLIT_TEXTURE_TYPES; ++i)
		{
			char tmpBuffer[64];
			LwString blockName(LwString::FromEmptyPointer(tmpBuffer, sizeof(tmpBuffer)));
			blockName.a("diffuse_map", i);
			itor = json.FindMember(blockName.c_str());
			if (itor != json.MemberEnd())
			{
				const rapidjson::Value &subobj = itor->value;
				loadTexture(subobj, blocks, i, unlitDatablock);
			}
		}
	}
	//-----------------------------------------------------------------------------------
    void HlmsJsonUnlit::saveTexture( const char *blockName,
                                     uint8 textureType,
                                     const HlmsUnlitDatablock *datablock, String &outString,
                                     bool writeTexture )
    {
		outString += ",\n\t\t\t\"";
		outString += blockName;
		outString += "\" :\n\t\t\t{\n";

		const size_t currentOffset = outString.size();

		if (writeTexture)
		{
			HlmsTextureManager::TextureLocation texLocation;
			texLocation.texture = datablock->getTexture(textureType);
			if (!texLocation.texture.isNull())
			{
                texLocation.xIdx = datablock->_getTextureIdx( textureType );
				texLocation.yIdx = 0;
				texLocation.divisor = 1;

				const String *texName = mHlmsManager->getTextureManager()->findAliasName(texLocation);

				if (texName)
				{
					outString += ",\n\t\t\t\t\"texture\" : \"";
					outString += *texName;
					outString += '"';
				}
			}

			const HlmsSamplerblock *samplerblock = datablock->getSamplerblock(textureType);
			if (samplerblock)
			{
				outString += ",\n\t\t\t\t\"sampler\" : ";
				outString += HlmsJson::getName(samplerblock);
			}

			if (textureType > 0)
			{
				UnlitBlendModes blendMode = datablock->getBlendMode(textureType);

				if (blendMode != UNLIT_BLEND_NORMAL_NON_PREMUL)
				{
					outString += ",\n\t\t\t\t\"blendmode\" : \"";
					outString += c_unlitBlendModes[blendMode];
					outString += '"';
				}
			}

			if (textureType < NUM_UNLIT_TEXTURE_TYPES && datablock->getTextureUvSource(textureType) != 0)
			{
				outString += ",\n\t\t\t\t\"uv\" : ";
				outString += StringConverter::toString(datablock->getTextureUvSource(textureType));
			}

			if (textureType < NUM_UNLIT_TEXTURE_TYPES &&
				datablock->getEnableAnimationMatrix(textureType))
			{
				Matrix4 mat = datablock->getAnimationMatrix(textureType);
				if (mat != Matrix4::IDENTITY)
				{
					outString += ",\n\t\t\t\t\"animate\" : ";
					outString += "\n\t\t\t\t[";

					outString += "[";
					outString += StringConverter::toString(mat[0][0]);
					outString += ", ";
					outString += StringConverter::toString(mat[0][1]);
					outString += ", ";
					outString += StringConverter::toString(mat[0][2]);
					outString += ", ";
					outString += StringConverter::toString(mat[0][3]);
					outString += ']';
				
					outString += ",\n\t\t\t\t[";
					outString += StringConverter::toString(mat[1][0]);
					outString += ", ";
					outString += StringConverter::toString(mat[1][1]);
					outString += ", ";
					outString += StringConverter::toString(mat[1][2]);
					outString += ", ";
					outString += StringConverter::toString(mat[1][3]);
					outString += ']';

					outString += ",\n\t\t\t\t[";
					outString += StringConverter::toString(mat[2][0]);
					outString += ", ";
					outString += StringConverter::toString(mat[2][1]);
					outString += ", ";
					outString += StringConverter::toString(mat[2][2]);
					outString += ", ";
					outString += StringConverter::toString(mat[2][3]);
					outString += ']';

					outString += ",\n\t\t\t\t[";
					outString += StringConverter::toString(mat[3][0]);
					outString += ", ";
					outString += StringConverter::toString(mat[3][1]);
					outString += ", ";
					outString += StringConverter::toString(mat[3][2]);
					outString += ", ";
					outString += StringConverter::toString(mat[3][3]);
					outString += ']';

					outString += "]";
				}
			}
		}

		if (outString.size() != currentOffset)
		{
			//Remove an extra comma and newline characters.
			outString.erase(currentOffset, 2);
		}

		outString += "\n\t\t\t}";
	}
    //-----------------------------------------------------------------------------------
    void HlmsJsonUnlit::saveMaterial( const HlmsDatablock *datablock, String &outString )
    {
        assert( dynamic_cast<const HlmsUnlitDatablock*>(datablock) );
        const HlmsUnlitDatablock *unlitDatablock = static_cast<const HlmsUnlitDatablock*>(datablock);

		ColourValue value = unlitDatablock->getColour();;
		if (unlitDatablock->hasColour() && value != ColourValue::White)
		{
			outString += ",\n\t\t\t\"diffuse\" : ";
			HlmsJson::toStr(value, outString);
		}

		for (uint8 i = 0; i<NUM_UNLIT_TEXTURE_TYPES; ++i)
		{
			if (!unlitDatablock->getTexture(i).isNull())
			{
				char tmpBuffer[64];
				LwString blockName(LwString::FromEmptyPointer(tmpBuffer, sizeof(tmpBuffer)));
				blockName.a("diffuse_map", i);
				saveTexture(blockName.c_str(), i, unlitDatablock, outString);
			}
		}
	}
    //-----------------------------------------------------------------------------------
    void HlmsJsonUnlit::collectSamplerblocks( const HlmsDatablock *datablock,
                                              set<const HlmsSamplerblock*>::type &outSamplerblocks )
    {
        assert( dynamic_cast<const HlmsUnlitDatablock*>(datablock) );
        const HlmsUnlitDatablock *unlitDatablock = static_cast<const HlmsUnlitDatablock*>(datablock);
		
		for (uint8 i = 0; i<NUM_UNLIT_TEXTURE_TYPES; ++i)
		{
			const HlmsSamplerblock *samplerblock = unlitDatablock->getSamplerblock(i);
			if (samplerblock)
				outSamplerblocks.insert(samplerblock);
		}
	}
}

#endif
