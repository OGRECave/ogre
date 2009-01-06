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
#ifndef _Codec_H__
#define _Codec_H__

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreDataStream.h"
#include "OgreIteratorWrappers.h"
#include "OgreStringVector.h"

namespace Ogre {

    /** Abstract class that defines a 'codec'.
        @remarks
            A codec class works like a two-way filter for data - data entered on
            one end (the decode end) gets processed and transformed into easily
            usable data while data passed the other way around codes it back.
        @par
            The codec concept is a pretty generic one - you can easily understand
            how it can be used for images, sounds, archives, even compressed data.
    */
	class _OgreExport Codec : public CodecAlloc
    {
    protected:
        typedef map< String, Codec* >::type CodecList; 
        /** A map that contains all the registered codecs.
        */
        static CodecList ms_mapCodecs;

    public:
        class _OgrePrivate CodecData : public CodecAlloc
        {
        public:
            virtual ~CodecData() {};

            /** Returns the type of the data.
            */
            virtual String dataType() const { return "CodecData"; };
        };
        typedef SharedPtr<CodecData> CodecDataPtr;

        typedef ConstMapIterator<CodecList> CodecIterator;

    public:
    	virtual ~Codec();
    	
        /** Registers a new codec in the database.
        */
        static void registerCodec( Codec *pCodec )
        {
            ms_mapCodecs[pCodec->getType()] = pCodec;
        }

        /** Unregisters a codec from the database.
        */
        static void unRegisterCodec( Codec *pCodec )
        {
            ms_mapCodecs.erase(pCodec->getType());
        }

        /** Gets the iterator for the registered codecs. */
        static CodecIterator getCodecIterator(void)
        {
            return CodecIterator(ms_mapCodecs.begin(), ms_mapCodecs.end());
        }

        /** Gets the file extension list for the registered codecs. */
        static StringVector getExtensions(void);

        /** Gets the codec registered for the passed in file extension. */
        static Codec* getCodec(const String& extension);

		/** Gets the codec that can handle the given 'magic' identifier. 
		@param magicNumberPtr Pointer to a stream of bytes which should identify the file.
			Note that this may be more than needed - each codec may be looking for 
			a different size magic number.
		@param maxbytes The number of bytes passed
		*/
		static Codec* getCodec(char *magicNumberPtr, size_t maxbytes);

        /** Codes the data in the input stream and saves the result in the output
            stream.
        */
        virtual DataStreamPtr code(MemoryDataStreamPtr& input, CodecDataPtr& pData) const = 0;
        /** Codes the data in the input chunk and saves the result in the output
            filename provided. Provided for efficiency since coding to memory is
            progressive therefore memory required is unknown leading to reallocations.
        @param input The input data
        @param outFileName The filename to write to
        @param pData Extra information to be passed to the codec (codec type specific)
        */
        virtual void codeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const = 0;

        /// Result of a decoding; both a decoded data stream and CodecData metadata
        typedef std::pair<MemoryDataStreamPtr, CodecDataPtr> DecodeResult;
        /** Codes the data from the input chunk into the output chunk.
            @param input Stream containing the encoded data
            @note
                Has a variable number of arguments, which depend on the codec type.
        */
        virtual DecodeResult decode(DataStreamPtr& input) const = 0;

        /** Returns the type of the codec as a String
        */
        virtual String getType() const = 0;

        /** Returns the type of the data that supported by this codec as a String
        */
        virtual String getDataType() const = 0;

		/** Returns whether a magic number header matches this codec.
		@param magicNumberPtr Pointer to a stream of bytes which should identify the file.
			Note that this may be more than needed - each codec may be looking for 
			a different size magic number.
		@param maxbytes The number of bytes passed
		*/
		virtual bool magicNumberMatch(const char *magicNumberPtr, size_t maxbytes) const 
		{ return !magicNumberToFileExt(magicNumberPtr, maxbytes).empty(); }
		/** Maps a magic number header to a file extension, if this codec recognises it.
		@param magicNumberPtr Pointer to a stream of bytes which should identify the file.
			Note that this may be more than needed - each codec may be looking for 
			a different size magic number.
		@param maxbytes The number of bytes passed
		@returns A blank string if the magic number was unknown, or a file extension.
		*/
		virtual String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const = 0;
    };

} // namespace

#endif
