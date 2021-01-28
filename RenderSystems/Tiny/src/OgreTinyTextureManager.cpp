// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreTinyTextureManager.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTexture.h"
#include "OgreTinyTexture.h"
#include "OgreRoot.h"

namespace Ogre
{
TinyTextureManager::TinyTextureManager()
{
    // Register with group manager
    ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
}

TinyTextureManager::~TinyTextureManager()
{
    // Unregister with group manager
    ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
}

Resource* TinyTextureManager::createImpl(const String& name, ResourceHandle handle, const String& group,
                                         bool isManual, ManualResourceLoader* loader,
                                         const NameValuePairList* createParams)
{
    return new TinyTexture(this, name, handle, group, isManual, loader);
}

PixelFormat TinyTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
{
    // only byte formats supported
    return PF_BYTE_RGBA;
}
} // namespace Ogre
