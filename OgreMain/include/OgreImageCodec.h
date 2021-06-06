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
#ifndef _ImageCodec_H__
#define _ImageCodec_H__

#include "OgreCodec.h"
#include "OgrePixelFormat.h"
#include "OgreBitwise.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */
    /** Codec specialized in images.
        @remarks
            The users implementing subclasses of ImageCodec are required to return
            a valid pointer to a ImageData class from the decode(...) function.
    */
    class _OgreExport ImageCodec : public Codec
    {
    protected:
        static void flipEndian(void* pData, size_t size, size_t count)
        {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            Bitwise::bswapChunks(pData, size, count);
#endif
        }
        static void flipEndian(void* pData, size_t size)
        {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            Bitwise::bswapBuffer(pData, size);
#endif
        }

    public:
        using Codec::decode;
        using Codec::encode;
        using Codec::encodeToFile;

        void decode(const DataStreamPtr& input, const Any& output) const override;
        DataStreamPtr encode(const Any& input) const override;
        void encodeToFile(const Any& input, const String& outFileName) const override;

        virtual ~ImageCodec();
        /** Codec return class for images. Has information about the size and the
            pixel format of the image. */
        class _OgrePrivate ImageData
        {
        public:
            ImageData():
                height(0), width(0), depth(1), size(0),
                num_mipmaps(0), flags(0), format(PF_UNKNOWN)
            {
            }
            uint32 height;
            uint32 width;
            uint32 depth;
            size_t size;
            
            uint32 num_mipmaps;
            uint flags;

            PixelFormat format;
        };
        typedef SharedPtr<ImageData> CodecDataPtr;

        /// @deprecated
        OGRE_DEPRECATED virtual DataStreamPtr encode(const MemoryDataStreamPtr& input, const CodecDataPtr& pData) const { return encode(Any()); }
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
    };

    /** @} */
    /** @} */
} // namespace

#endif
