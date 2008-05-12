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
#ifndef _FreeImageCodec_H__
#define _FreeImageCodec_H__

#include "OgreImageCodec.h"
// Forward-declaration to avoid external dependency on FreeImage
struct FIBITMAP;

namespace Ogre {

    /** Codec specialized in images loaded using FreeImage.
        @remarks
            The users implementing subclasses of ImageCodec are required to return
            a valid pointer to a ImageData class from the decode(...) function.
    */
    class _OgreExport FreeImageCodec : public ImageCodec
    {
    private:
        String mType;
        unsigned int mFreeImageType;

		typedef std::list<ImageCodec*> RegisteredCodecList;
		static RegisteredCodecList msCodecList;

		/** Common encoding routine. */
		FIBITMAP* encode(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;

    public:
        FreeImageCodec(const String &type, unsigned int fiType);
        virtual ~FreeImageCodec() { }

        /// @copydoc Codec::code
        DataStreamPtr code(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;
        /// @copydoc Codec::codeToFile
        void codeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const;
        /// @copydoc Codec::decode
        DecodeResult decode(DataStreamPtr& input) const;

        
        virtual String getType() const;        

		/// @copydoc Codec::magicNumberToFileExt
		String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const;

		/// Static method to startup FreeImage and register the FreeImage codecs
		static void startup(void);
		/// Static method to shutdown FreeImage and unregister the FreeImage codecs
		static void shutdown(void);
    };

} // namespace

#endif
