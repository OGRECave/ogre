/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __OgreDDSCodec_H__
#define __OgreDDSCodec_H__

#include "OgreImageCodec.h"
namespace Ogre {

	// Forward declarations
	struct DXTColourBlock;
	struct DXTExplicitAlphaBlock;
	struct DXTInterpolatedAlphaBlock;

    /** Codec specialized in loading DDS (Direct Draw Surface) images.
	@remarks
		We implement our own codec here since we need to be able to keep DXT
		data compressed if the card supports it.
    */
    class _OgreExport DDSCodec : public ImageCodec
    {
    private:
        String mType;

    	void flipEndian(void * pData, size_t size, size_t count) const;
	    void flipEndian(void * pData, size_t size) const;

		PixelFormat convertFourCCFormat(uint32 fourcc) const;
		PixelFormat convertPixelFormat(uint32 rgbBits, uint32 rMask, 
			uint32 gMask, uint32 bMask, uint32 aMask) const;

		/// Unpack DXT colours into array of 16 colour values
		void unpackDXTColour(PixelFormat pf, const DXTColourBlock& block, ColourValue* pCol) const;
		/// Unpack DXT alphas into array of 16 colour values
		void unpackDXTAlpha(const DXTExplicitAlphaBlock& block, ColourValue* pCol) const;
		/// Unpack DXT alphas into array of 16 colour values
		void unpackDXTAlpha(const DXTInterpolatedAlphaBlock& block, ColourValue* pCol) const;

		/// Single registered codec instance
		static DDSCodec* msInstance;
	public:
        DDSCodec();
        virtual ~DDSCodec() { }

        /// @copydoc Codec::code
        DataStreamPtr code(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;
        /// @copydoc Codec::codeToFile
        void codeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const;
        /// @copydoc Codec::decode
        DecodeResult decode(DataStreamPtr& input) const;
		/// @copydoc Codec::magicNumberToFileExt
		String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const;
        
        virtual String getType() const;        

		/// Static method to startup and register the DDS codec
		static void startup(void);
		/// Static method to shutdown and unregister the DDS codec
		static void shutdown(void);

    };

} // namespace

#endif

