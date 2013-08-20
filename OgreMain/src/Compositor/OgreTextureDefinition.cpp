/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "Compositor/OgreTextureDefinition.h"

namespace Ogre
{
	TextureDefinitionBase::TextureDefinitionBase( TextureSource defaultSource ) :
			mDefaultLocalTextureSource( defaultSource )
	{
		assert( mDefaultLocalTextureSource == TEXTURE_LOCAL ||
				mDefaultLocalTextureSource == TEXTURE_GLOBAL );
	}
	//-----------------------------------------------------------------------------------
	IdString TextureDefinitionBase::addTextureSourceName( const String &name, size_t index,
															TextureSource textureSource )
	{
		String::size_type findResult = name.find( "global_" );
		if( textureSource == TEXTURE_LOCAL && findResult == 0 )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
						"Local textures can't start with global_ prefix! '" + name + "'",
						"TextureDefinitionBase::addLocalTextureDefinition" );
		}
		else if( textureSource == TEXTURE_GLOBAL && findResult != 0 )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
						"Global textures must start with global_ prefix! '" + name + "'",
						"TextureDefinitionBase::addLocalTextureDefinition" );
		}

		assert( index <= 0x3FFFFFFF && "Texture Source Index out of supported range" );
		uint32 value = (index & 0x3FFFFFFF)|(textureSource<<30);

		IdString hashedName( name );
		NameToChannelMap::const_iterator itor = mNameToChannelMap.find( hashedName );
		if( itor != mNameToChannelMap.end() && itor->second != value )
		{
			OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM,
						"Texture with same name '" + name +
						"' in the same scope already exists",
						"TextureDefinitionBase::addLocalTextureDefinition" );
		}

		mNameToChannelMap[hashedName] = value;

		return hashedName;
	}
	//-----------------------------------------------------------------------------------
	void TextureDefinitionBase::getTextureSource( IdString name, size_t &index,
													TextureSource &textureSource ) const
	{
		NameToChannelMap::const_iterator itor = mNameToChannelMap.find( name );
		if( itor == mNameToChannelMap.end() )
		{
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
						"Can't find texture with name: '" + name.getFriendlyText(),
						"CompositorNodeDef::getTextureSource" );
		}

		uint32 value		= itor->second;
		uint32 texSource	= (value & 0xC0000000) >> 30;
		assert( texSource < NUM_TEXTURES_SOURCES );

		index		 = value & 0x3FFFFFFF;
		textureSource = static_cast<TextureSource>( texSource );
	}
	//-----------------------------------------------------------------------------------
	TextureDefinitionBase::TextureDefinition* TextureDefinitionBase::addLocalTextureDefinition
																		( const String &name )
	{
		IdString hashedName = addTextureSourceName( name, mLocalTextureDefs.size(),
													mDefaultLocalTextureSource );
		mLocalTextureDefs.push_back( TextureDefinition( hashedName ) );
		return &mLocalTextureDefs.back();
	}
}
