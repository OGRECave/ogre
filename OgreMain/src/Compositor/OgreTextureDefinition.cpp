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
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/Pass/OgreCompositorPass.h"

#include "OgreRenderTarget.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
	TextureDefinitionBase::TextureDefinitionBase( TextureSource defaultSource ) :
			mDefaultLocalTextureSource( defaultSource )
	{
		assert( mDefaultLocalTextureSource == TEXTURE_LOCAL ||
				mDefaultLocalTextureSource == TEXTURE_GLOBAL );
	}
	//-----------------------------------------------------------------------------------
	size_t TextureDefinitionBase::getNumInputChannels(void) const
	{
		size_t numInputChannels = 0;
		NameToChannelMap::const_iterator itor = mNameToChannelMap.begin();
		NameToChannelMap::const_iterator end  = mNameToChannelMap.end();

		while( itor != end )
		{
			size_t index;
			TextureSource texSource;
			decodeTexSource( itor->second, index, texSource );
			if( texSource == TEXTURE_INPUT )
				++numInputChannels;
			++itor;
		}

		return numInputChannels;
	}
	//-----------------------------------------------------------------------------------
	inline uint32 TextureDefinitionBase::encodeTexSource( size_t index, TextureSource textureSource )
	{
		assert( index <= 0x3FFFFFFF && "Texture Source Index out of supported range" );
		return (index & 0x3FFFFFFF)|(textureSource<<30);
	}
	//-----------------------------------------------------------------------------------
    void TextureDefinitionBase::decodeTexSource( uint32 encodedVal, size_t &outIdx,
                                                 TextureSource &outTexSource )
	{
		uint32 texSource = (encodedVal & 0xC0000000) >> 30;
		assert( texSource < NUM_TEXTURES_SOURCES );

		outIdx		 = encodedVal & 0x3FFFFFFF;
		outTexSource = static_cast<TextureSource>( texSource );
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
						"TextureDefinitionBase::addTextureSourceName" );
		}
		else if( textureSource == TEXTURE_GLOBAL && findResult != 0 )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
						"Global textures must start with global_ prefix! '" + name + "'",
						"TextureDefinitionBase::addTextureSourceName" );
		}

		const uint32 value = encodeTexSource( index, textureSource );

		IdString hashedName( name );
		NameToChannelMap::const_iterator itor = mNameToChannelMap.find( hashedName );
		if( itor != mNameToChannelMap.end() && itor->second != value )
		{
			OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM,
						"Texture with same name '" + name +
						"' in the same scope already exists",
						"TextureDefinitionBase::addTextureSourceName" );
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
						"Can't find texture with name: '" + name.getFriendlyText() + "'"
						" If it's a global texture, trying to use it in the output channel will fail.",
						"CompositorNodeDef::getTextureSource" );
		}

		decodeTexSource( itor->second, index, textureSource );
	}
	//-----------------------------------------------------------------------------------
	TextureDefinitionBase::TextureDefinition* TextureDefinitionBase::addTextureDefinition
																		( const String &name )
	{
		IdString hashedName = addTextureSourceName( name, mLocalTextureDefs.size(),
													mDefaultLocalTextureSource );
		mLocalTextureDefs.push_back( TextureDefinition( hashedName ) );
		return &mLocalTextureDefs.back();
	}

	//-----------------------------------------------------------------------------------
	void TextureDefinitionBase::createTextures( const TextureDefinitionVec &textureDefs,
												CompositorChannelVec &inOutTexContainer,
												IdType id, bool uniqueNames,
												const RenderTarget *finalTarget,
												RenderSystem *renderSys )
	{
		inOutTexContainer.reserve( textureDefs.size() );

		//Create the local textures
		TextureDefinitionVec::const_iterator itor = textureDefs.begin();
		TextureDefinitionVec::const_iterator end  = textureDefs.end();

		while( itor != end )
		{
			String textureName;
			if( !uniqueNames )
				textureName = (itor->name + IdString( id )).getFriendlyText();
			else
				textureName = itor->name.getFriendlyText();

			CompositorChannel newChannel = createTexture( *itor, textureName, finalTarget, renderSys );
			inOutTexContainer.push_back( newChannel );
			++itor;
		}
	}
	//-----------------------------------------------------------------------------------
	CompositorChannel TextureDefinitionBase::createTexture( const TextureDefinition &textureDef,
											const String &texName, const RenderTarget *finalTarget,
											RenderSystem *renderSys )
	{
		CompositorChannel newChannel;

		bool defaultHwGamma		= false;
		uint defaultFsaa		= 0;
		String defaultFsaaHint	= StringUtil::BLANK;
		if( finalTarget )
		{
			// Inherit settings from target
			defaultHwGamma	= finalTarget->isHardwareGammaEnabled();
			defaultFsaa		= finalTarget->getFSAA();
			defaultFsaaHint	= finalTarget->getFSAAHint();
		}

		//If undefined, use main target's hw gamma settings, else use explicit setting
		bool hwGamma	= textureDef.hwGammaWrite == BoolUndefined ?
								defaultHwGamma : (textureDef.hwGammaWrite == BoolTrue);
		//If true, use main target's fsaa settings, else disable
		uint fsaa		= textureDef.fsaa ? defaultFsaa : 0;
		const String &fsaaHint = textureDef.fsaa ? defaultFsaaHint : StringUtil::BLANK;

		uint width	= textureDef.width;
		uint height	= textureDef.height;
		if( finalTarget )
		{
			if( textureDef.width == 0 )
				width = static_cast<uint>( ceilf( finalTarget->getWidth() * textureDef.widthFactor ) );
			if( textureDef.height == 0 )
				height = static_cast<uint>( ceilf( finalTarget->getHeight() * textureDef.heightFactor ) );
		}

		if( textureDef.formatList.size() == 1 )
		{
			//Normal RT
			TexturePtr tex = TextureManager::getSingleton().createManual( texName,
											ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
											TEX_TYPE_2D, width, height, 0,
											textureDef.formatList[0], TU_RENDERTARGET, 0, hwGamma,
											fsaa, fsaaHint, textureDef.fsaaExplicitResolve );
			RenderTexture* rt = tex->getBuffer()->getRenderTarget();
			newChannel.target = rt;
			newChannel.textures.push_back( tex );
		}
		else
		{
			//MRT
			MultiRenderTarget* mrt = renderSys->createMultiRenderTarget( texName );
			PixelFormatList::const_iterator pixIt = textureDef.formatList.begin();
			PixelFormatList::const_iterator pixEn = textureDef.formatList.end();

			newChannel.target = mrt;

			while( pixIt != pixEn )
			{
				size_t rtNum = pixIt - textureDef.formatList.begin();
				TexturePtr tex = TextureManager::getSingleton().createManual(
											texName + StringConverter::toString( rtNum ),
											ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
											TEX_TYPE_2D, width, height, 0,
											*pixIt, TU_RENDERTARGET, 0, hwGamma,
											fsaa, fsaaHint, textureDef.fsaaExplicitResolve );
				RenderTexture* rt = tex->getBuffer()->getRenderTarget();
				mrt->bindSurface( rtNum, rt );
				newChannel.textures.push_back( tex );
				++pixIt;
			}
		}

		return newChannel;
	}
	//-----------------------------------------------------------------------------------
	void TextureDefinitionBase::destroyTextures( CompositorChannelVec &inOutTexContainer,
												 RenderSystem *renderSys )
	{
		CompositorChannelVec::const_iterator itor = inOutTexContainer.begin();
		CompositorChannelVec::const_iterator end  = inOutTexContainer.end();

		while( itor != end )
		{
			if( itor->isValid() )
			{
				if( !itor->isMrt() )
				{
					//Normal RT. We don't hold any reference to, so just deregister from TextureManager
					TextureManager::getSingleton().remove( itor->textures[0]->getName() );
				}
				else
				{
					//MRT. We need to destroy both the MultiRenderTarget ptr AND the textures
					renderSys->destroyRenderTarget( itor->target->getName() );
					for( size_t i=0; i<itor->textures.size(); ++i )
						TextureManager::getSingleton().remove( itor->textures[i]->getName() );
				}
			}

			++itor;
		}

		inOutTexContainer.clear();
	}
	//-----------------------------------------------------------------------------------
	void TextureDefinitionBase::recreateResizableTextures( const TextureDefinitionVec &textureDefs,
															CompositorChannelVec &inOutTexContainer,
															const RenderTarget *finalTarget,
															RenderSystem *renderSys,
															const CompositorNodeVec &connectedNodes,
															const CompositorPassVec *passes )
	{
		TextureDefinitionVec::const_iterator itor = textureDefs.begin();
		TextureDefinitionVec::const_iterator end  = textureDefs.end();

		CompositorChannelVec::iterator itorTex = inOutTexContainer.begin();

		while( itor != end )
		{
			if( (itor->width == 0 || itor->height == 0) && itorTex->isValid() )
			{
				for( size_t i=0; i<itorTex->textures.size(); ++i )
					TextureManager::getSingleton().remove( itorTex->textures[i]->getName() );

				CompositorChannel newChannel = createTexture( *itor, itorTex->target->getName(),
																finalTarget, renderSys );

				if( passes )
				{
					CompositorPassVec::const_iterator passIt = passes->begin();
					CompositorPassVec::const_iterator passEn = passes->end();
					while( passIt != passEn )
					{
						(*passIt)->notifyRecreated( *itorTex, newChannel );
						++passIt;
					}
				}

				CompositorNodeVec::const_iterator itNodes = connectedNodes.begin();
				CompositorNodeVec::const_iterator enNodes = connectedNodes.end();

				while( itNodes != enNodes )
				{
					(*itNodes)->notifyRecreated( *itorTex, newChannel );
					++itNodes;
				}

				if( !itorTex->isMrt() )
					renderSys->destroyRenderTarget( itorTex->target->getName() );

				*itorTex = newChannel;
			}
			++itorTex;
			++itor;
		}
	}
}
