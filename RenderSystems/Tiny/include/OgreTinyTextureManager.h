// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TinyTextureManager_H__
#define __TinyTextureManager_H__

#include "OgreTextureManager.h"
#include "OgreTexture.h"

namespace Ogre {
    class TinyTextureManager : public TextureManager
    {
    public:
        TinyTextureManager();
        virtual ~TinyTextureManager();

        /// @copydoc TextureManager::getNativeFormat
        PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage) override;

    protected:
        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle,
                             const String& group, bool isManual, ManualResourceLoader* loader,
                             const NameValuePairList* createParams) override;
    };
}

#endif
