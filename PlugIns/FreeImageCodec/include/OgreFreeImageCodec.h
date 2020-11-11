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

#include "OgreFreeImageCodecExports.h"

#include "OgreImageCodec.h"
#include "OgrePlugin.h"

// Forward-declaration to avoid external dependency on FreeImage
struct FIBITMAP;

namespace Ogre {

    /** \addtogroup Plugins Plugins
    *  @{
    */
    /** \defgroup FreeImageCodec FreeImageCodec
    * %Codec specialized in images loaded using [FreeImage](https://freeimage.sourceforge.io/)
    *  @{
    */
    class FreeImageCodec : public ImageCodec
    {
    private:
        String mType;
        unsigned int mFreeImageType;

        typedef std::list<ImageCodec*> RegisteredCodecList;
        static RegisteredCodecList msCodecList;

        /** Common encoding routine. */
        FIBITMAP* encodeBitmap(const MemoryDataStreamPtr& input, const CodecDataPtr& pData) const;

    public:
        FreeImageCodec(const String &type, unsigned int fiType);
        virtual ~FreeImageCodec() { }

        using ImageCodec::decode;
        using ImageCodec::encode;
        using ImageCodec::encodeToFile;

        DataStreamPtr encode(const MemoryDataStreamPtr& input, const CodecDataPtr& pData) const override;
        void encodeToFile(const MemoryDataStreamPtr& input, const String& outFileName, const CodecDataPtr& pData) const  override;
        DecodeResult decode(const DataStreamPtr& input) const  override;

        String getType() const override;
        String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const override;

        /// Static method to startup FreeImage and register the FreeImage codecs
        _OgreFreeImageCodecExport static void startup(void);
        /// Static method to shutdown FreeImage and unregister the FreeImage codecs
        _OgreFreeImageCodecExport static void shutdown(void);
    };

    class FreeImagePlugin : public Plugin
    {
    public:
        const String& getName() const;
        void install() { FreeImageCodec::startup(); }
        void uninstall() { FreeImageCodec::shutdown(); }
        void initialise() {}
        void shutdown() {}
    };
    /** @} */
    /** @} */

} // namespace

#endif
