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
#ifndef _FreeImageCodec_H__
#define _FreeImageCodec_H__

#include "OgreImageCodec.h"
// Forward-declaration to avoid external dependency on FreeImage
struct FIBITMAP;

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */
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

        typedef list<ImageCodec*>::type RegisteredCodecList;
        static RegisteredCodecList msCodecList;

        /** Common encoding routine. */
        FIBITMAP* encodeBitmap(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;

    public:
        FreeImageCodec(const String &type, unsigned int fiType);
        virtual ~FreeImageCodec() { }

        /// @copydoc Codec::encode
        DataStreamPtr encode(MemoryDataStreamPtr& input, CodecDataPtr& pData) const;
        /// @copydoc Codec::encodeToFile
        void encodeToFile(MemoryDataStreamPtr& input, const String& outFileName, CodecDataPtr& pData) const;
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
    /** @} */
    /** @} */

} // namespace

#endif
