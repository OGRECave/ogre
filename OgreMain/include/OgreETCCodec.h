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
#ifndef __OgreETCCodec_H__
#define __OgreETCCodec_H__

#include "OgreImageCodec.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */

    /** Codec specialized in loading ETC (Ericsson Texture Compression) images.
    @remarks
        We implement our own codec here since we need to be able to keep ETC
        data compressed if the card supports it.
    */
    class _OgreExport ETCCodec : public ImageCodec
    {
    protected:
        String mType;
        
        /// Single registered codec instance
        static ETCCodec* msPKMInstance;
        static ETCCodec* msKTXInstance;

    public:
        ETCCodec(const String &type);
        virtual ~ETCCodec() { }

        using ImageCodec::decode;
        DecodeResult decode(const DataStreamPtr& input) const override;
        String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const override;
        String getType() const override;

        /// Static method to startup and register the ETC codec
        static void startup(void);
        /// Static method to shutdown and unregister the ETC codec
        static void shutdown(void);
    private:
        bool decodePKM(const DataStreamPtr& input, DecodeResult& result) const;
        bool decodeKTX(const DataStreamPtr& input, DecodeResult& result) const;

    };
    /** @} */
    /** @} */

} // namespace

#endif

