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

#ifndef __CompositorNodeDef_H__
#define __CompositorNodeDef_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreIdString.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	/** Compositor nodes are the core subject of compositing.
		TODO: Describe!!!
	@remarks
		We own the local textures, so it's our job to destroy them
	@author
		Matias N. Goldberg
    @version
        1.0
    */
	class _OgreExport CompositorNodeDef : public CompositorInstAlloc
	{
		friend class CompositorNode;

		/// Local texture definition
        class TextureDefinition : public CompositorInstAlloc
        {
        public:
			enum BoolSetting
			{
				Undefined = 0,
				False,
				True,
			};

            IdString name;
            uint width;       // 0 means adapt to target width
            uint height;      // 0 means adapt to target height
			float widthFactor;  // multiple of target width to use (if width = 0)
			float heightFactor; // multiple of target height to use (if height = 0)
            PixelFormatList formatList; // more than one means MRT
			bool fsaa;			// FSAA enabled; True = Use main target's, False = disable
			BoolSetting hwGammaWrite;	// Do sRGB gamma correction on write (only 8-bit per channel formats) 
			uint16 depthBufferId;//Depth Buffer's pool ID.

			TextureDefinition() :width(0), height(0), widthFactor(1.0f), heightFactor(1.0f), 
				fsaa(true), hwGammaWrite(Undefined), depthBufferId(1) {}
        };

		typedef vector<TextureDefinition>::type		TextureDefinitionVec;
		typedef vector<uint32>::type				ChannelMappings;
		/** Tells where to grab the RenderTarget from for the output channel.
			They can come either from an input channel, or from local textures.
			The first 31 bits indicate the channel #, the last 31st bit is used to
			determine whether it comes from the input channel, or the local texture
			If the bit is set, it's a local texture.
		*/
		ChannelMappings			mOutChannelMapping;

		TextureDefinitionVec	mLocalTextureDefs;

	public:
		/** Retrieves in which container to look for when wanting to know the output texture
			using the mappings from input/local texture -> output.
		@param outputChannel
			The output channel we want to know about
		@param index [out]
			The index at the container in which the texture associated with the output channel
			is stored
		@param localTexture [out]
			True if the source is a local texture, false if it's from another input
		*/
		void getTextureSource( size_t outputChannel, size_t &index, bool &localTexture ) const
		{
			uint32 value = mOutChannelMapping[outputChannel];
			index		 = value & 0x7FFFFFFF;
			localTexture = (value & 0x80000000) != 0;
		}
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
