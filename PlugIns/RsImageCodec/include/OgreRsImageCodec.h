// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#pragma once

#include "OgreRsImageCodecExports.h"
#include "OgreImageCodec.h"
#include "OgrePlugin.h"

namespace Ogre {

    /** \addtogroup Plugins Plugins
    *  @{
    */
    /** \defgroup RsImageCodec RsImageCodec
    * %Codec for loading generic image formats (e.g. jpg, png) using image-rs
    *
    * This Codec is ideal for files, that you are not in control of. The Rust implementation is robust against malformed files.
    * @{
    */
    class RsImageCodec : public ImageCodec
    {
    private:
        String mType;

        typedef std::list<ImageCodec*> RegisteredCodecList;
        static RegisteredCodecList msCodecList;

    public:
        RsImageCodec(const String &type);
        virtual ~RsImageCodec() { }

        DataStreamPtr encode(const Any& input) const override;
        void encodeToFile(const Any& input, const String& outFileName) const override;
        void decode(const DataStreamPtr& input, const Any& output) const override;

        String getType() const  override;
        String magicNumberToFileExt(const char *magicNumberPtr, size_t maxbytes) const override;

        /// Static method to startup and register the codecs
        _OgreRsImageCodecExport static void startup(void);
        /// Static method to shutdown and unregister the codecs
        _OgreRsImageCodecExport static void shutdown(void);
    };
    /** @} */
    /** @} */

} // namespace Ogre