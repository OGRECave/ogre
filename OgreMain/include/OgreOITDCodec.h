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
#ifndef _OgreOITDCodec_H_
#define _OgreOITDCodec_H_

#include "OgreImageCodec.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */

    // Forward declarations
    struct DXTColourBlock;
    struct DXTExplicitAlphaBlock;
    struct DXTInterpolatedAlphaBlock;

    /** Codec specialized in loading OITD (Ogre Internal Texture Dump) format.
    @remarks
        This format has two versions:
            Version 0 -> Used by Ogre 2.1
            Version 1 -> Used by Ogre 2.2
        The difference between the two are pixel format enums, and that Version 1 has
        rows aligned to 4 bytes, whereas Version 0 has no alignment restrictions.
        Version 0 is only used out of necessity, and is not generally recommended.
    */
    class _OgreExport OITDCodec : public ImageCodec
    {
    private:
        String mType;

        // invokes Bitwise::bswapChunks() if OGRE_ENDIAN_BIG
        static void flipEndian(void * pData, size_t size, size_t count);
        // invokes Bitwise::bswapBuffer() if OGRE_ENDIAN_BIG
        static void flipEndian(void * pData, size_t size);

        /// Single registered codec instance
        static OITDCodec* msInstance;
    public:
        OITDCodec();
        virtual ~OITDCodec() {}

        /// @copydoc Codec::encode
        virtual DataStreamPtr encode( MemoryDataStreamPtr &input, CodecDataPtr &pData ) const;
        /// @copydoc Codec::encodeToFile
        virtual void encodeToFile( MemoryDataStreamPtr &input, const String &outFileName,
                                   CodecDataPtr &pData ) const;
        /// @copydoc Codec::decode
        virtual DecodeResult decode( DataStreamPtr &input ) const;

        /// @copydoc Codec::magicNumberToFileExt
        virtual String magicNumberToFileExt( const char *magicNumberPtr, size_t maxbytes ) const;

        virtual String getType(void) const;

        /// Static method to startup and register the DDS codec
        static void startup(void);
        /// Static method to shutdown and unregister the DDS codec
        static void shutdown(void);
    };
    /** @} */
    /** @} */

} // namespace

#endif

