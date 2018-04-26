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
#ifndef __STBICodec_H__
#define __STBICodec_H__

#include "OgreSTBICodecExports.h"
#include "OgreImageCodec.h"
#include "OgrePlugin.h"

namespace Ogre {

    /** \addtogroup Plugins Plugins
    *  @{
    */
    /** \defgroup STBIImageCodec STBIImageCodec
    * %Codec specialized in images loaded using stb image (https://github.com/nothings/stb).
    * @{
    */
    class STBIImageCodec : public ImageCodec
    {
    private:
        String mType;

        typedef std::list<ImageCodec*> RegisteredCodecList;
        static RegisteredCodecList msCodecList;

    public:
        STBIImageCodec(const String &type);
        virtual ~STBIImageCodec() { }

        /// @copydoc Codec::encode
        DataStreamPtr encode(const MemoryDataStreamPtr& input, const CodecDataPtr& pData) const;
        /// @copydoc Codec::encodeToFile
        void encodeToFile(const MemoryDataStreamPtr& input, const String& outFileName, const CodecDataPtr& pData) const;
        /// @copydoc Codec::decode
        DecodeResult decode(const DataStreamPtr& input) const;

        
        virtual String getType() const;        

        /// @copydoc Codec::magicNumberToFileExt
        String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const;

        /// Static method to startup and register the codecs
        _OgreSTBICodecExport static void startup(void);
        /// Static method to shutdown and unregister the codecs
        _OgreSTBICodecExport static void shutdown(void);
    };

    class STBIPlugin : public Plugin
    {
        const String& getName() const;
        void install() { STBIImageCodec::startup(); }
        void uninstall() { STBIImageCodec::shutdown(); }
        void initialise() {}
        void shutdown() {}
    };
    /** @} */
    /** @} */

} // namespace

#endif
