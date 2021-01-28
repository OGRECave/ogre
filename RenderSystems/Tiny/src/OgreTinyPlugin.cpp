// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreTinyPlugin.h"
#include "OgreRoot.h"
#include "OgreTinyRenderSystem.h"
#include "OgreTinyExports.h"

namespace Ogre
{
const String sPluginName = "Tiny RenderSystem";

TinyPlugin::TinyPlugin() : mRenderSystem(0) {}

const String& TinyPlugin::getName() const
{
    return sPluginName;
}

void TinyPlugin::install()
{
    mRenderSystem = OGRE_NEW TinyRenderSystem();

    Root::getSingleton().addRenderSystem(mRenderSystem);
}

void TinyPlugin::initialise()
{
    // nothing to do
}

void TinyPlugin::shutdown()
{
    // nothing to do
}

void TinyPlugin::uninstall()
{
    OGRE_DELETE mRenderSystem;
    mRenderSystem = 0;
}

#ifndef OGRE_STATIC_LIB
static TinyPlugin* plugin;

extern "C" void _OgreTinyExport dllStartPlugin(void);
extern "C" void _OgreTinyExport dllStopPlugin(void);

extern "C" void _OgreTinyExport dllStartPlugin(void)
{
    plugin = OGRE_NEW TinyPlugin();
    Root::getSingleton().installPlugin(plugin);
}

extern "C" void _OgreTinyExport dllStopPlugin(void)
{
    Root::getSingleton().uninstallPlugin(plugin);
    OGRE_DELETE plugin;
}
#endif
} // namespace Ogre
