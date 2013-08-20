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

#include "Compositor/OgreCompositorNodeDef.h"

namespace Ogre
{
	void CompositorNodeDef::getTextureSource( size_t outputChannel, size_t &index,
												TextureSource &textureSource ) const
	{
		uint32 value		= mOutChannelMapping[outputChannel];
		uint32 texSource	= (value & 0xC0000000) >> 30;

		assert( texSource < NUM_TEXTURES_SOURCES );
		assert( texSource != TEXTURE_GLOBAL && "Can't use global textures in the output channel!" );

		index		 = value & 0x3FFFFFFF;
		textureSource = static_cast<TextureSource>( texSource );
	}
	//-----------------------------------------------------------------------------------
	CompositorTargetDef* CompositorNodeDef::addTargetPass( const String &renderTargetName )
	{
		if( renderTargetName.find( "global_" ) == 0 )
			addTextureSourceName( renderTargetName, 0, TEXTURE_GLOBAL );

		mTargetPasses.push_back( CompositorTargetDef( renderTargetName ) );
		return &mTargetPasses.back();
	}
	//-----------------------------------------------------------------------------------
}
