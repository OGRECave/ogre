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
#ifndef _Codec_H__
#define _Codec_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrapper.h"
#include "OgreStringVector.h"
#include "OgreException.h"
#include "OgreHeaderPrefix.h"
#include "OgreAny.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

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
        typedef std::map< String, Codec* > CodecList; 
        /** A map that contains all the registered codecs.
        */
        static CodecList msMapCodecs;

    public:
        class _OgrePrivate CodecData : public CodecAlloc
        {
        public:
            virtual ~CodecData() {}

            /// @deprecated do not use
            OGRE_DEPRECATED virtual String dataType() const { return "CodecData"; }
        };
        typedef SharedPtr<CodecData> CodecDataPtr;

        typedef ConstMapIterator<CodecList> CodecIterator;

    public:
        virtual ~Codec();
        
        /** Registers a new codec in the database.
        */
        static void registerCodec( Codec *pCodec );

        /** Return whether a codec is registered already. 
        */
        static bool isCodecRegistered( const String& codecType )
        {
            return msMapCodecs.find(codecType) != msMapCodecs.end();
        }

        /** Unregisters a codec from the database.
        */
        static void unregisterCodec( Codec *pCodec )
        {
            msMapCodecs.erase(pCodec->getType());
        }

        /// @deprecated use getExtensions()
        OGRE_DEPRECATED static CodecIterator getCodecIterator(void)
        {
            return CodecIterator(msMapCodecs.begin(), msMapCodecs.end());
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

        /** Codes the input and saves the result in the output
            stream.
        */
        virtual DataStreamPtr encode(const Any& input) const;

        /// @deprecated
        OGRE_DEPRECATED virtual DataStreamPtr encode(const MemoryDataStreamPtr& input, const CodecDataPtr& pData) const { return encode(Any()); }

        /** Codes the data in the input chunk and saves the result in the output
            filename provided. Provided for efficiency since coding to memory is
            progressive therefore memory required is unknown leading to reallocations.
        @param input The input data (codec type specific)
        @param outFileName The filename to write to
        */
        virtual void encodeToFile(const Any& input, const String& outFileName) const;

        /// @deprecated
        OGRE_DEPRECATED virtual void encodeToFile(const MemoryDataStreamPtr& input, const String& outFileName, const CodecDataPtr& pData) const
        { encodeToFile(Any(), ""); }

        /// Result of a decoding; both a decoded data stream and CodecData metadata
        typedef std::pair<MemoryDataStreamPtr, CodecDataPtr> DecodeResult;
        /// @deprecated
        OGRE_DEPRECATED virtual DecodeResult decode(const DataStreamPtr& input) const
        {
            return DecodeResult();
        }

        /** Codes the data from the input chunk into the output chunk.
            @param input Stream containing the encoded data
            @param output codec type specific result
        */
        virtual void decode(const DataStreamPtr& input, const Any& output) const {}

        /** Returns the type of the codec as a String
        */
        virtual String getType() const = 0;

        /// @deprecated do not use
        OGRE_DEPRECATED virtual String getDataType() const { return ""; }

        /** Returns whether a magic number header matches this codec.
        @param magicNumberPtr Pointer to a stream of bytes which should identify the file.
            Note that this may be more than needed - each codec may be looking for 
            a different size magic number.
        @param maxbytes The number of bytes passed
        */
        bool magicNumberMatch(const char *magicNumberPtr, size_t maxbytes) const
        { return !magicNumberToFileExt(magicNumberPtr, maxbytes).empty(); }
        /** Maps a magic number header to a file extension, if this codec recognises it.
        @param magicNumberPtr Pointer to a stream of bytes which should identify the file.
            Note that this may be more than needed - each codec may be looking for 
            a different size magic number.
        @param maxbytes The number of bytes passed
        @return A blank string if the magic number was unknown, or a file extension.
        */
        virtual String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const = 0;
    };
    /** @} */
    /** @} */

} // namespace

#include "OgreHeaderSuffix.h"

#endif
