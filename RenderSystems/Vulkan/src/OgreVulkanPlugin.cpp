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

#include "OgreVulkanPlugin.h"
#include "OgreRoot.h"
#include "OgreVulkanRenderSystem.h"

namespace Ogre
{
    const String sPluginName = "Vulkan RenderSystem";
    //---------------------------------------------------------------------
    VulkanPlugin::VulkanPlugin() : mRenderSystem( 0 ) {}
    //---------------------------------------------------------------------
    const String &VulkanPlugin::getName() const { return sPluginName; }
    //---------------------------------------------------------------------
    void VulkanPlugin::install()
    {
        mRenderSystem = OGRE_NEW VulkanRenderSystem();

        Root::getSingleton().addRenderSystem( mRenderSystem );
    }
    //---------------------------------------------------------------------
    void VulkanPlugin::initialise()
    {
        // nothing to do
    }
    //---------------------------------------------------------------------
    void VulkanPlugin::shutdown()
    {
        // nothing to do
    }
    //---------------------------------------------------------------------
    void VulkanPlugin::uninstall()
    {
        OGRE_DELETE mRenderSystem;
        mRenderSystem = 0;
    }

#ifndef OGRE_STATIC_LIB
    static VulkanPlugin *plugin;

    extern "C" void _OgreVulkanExport dllStartPlugin(void);
    extern "C" void _OgreVulkanExport dllStopPlugin(void);

    extern "C" void _OgreVulkanExport dllStartPlugin( void )
    {
        plugin = OGRE_NEW VulkanPlugin();
        Root::getSingleton().installPlugin( plugin );
    }

    extern "C" void _OgreVulkanExport dllStopPlugin( void )
    {
        Root::getSingleton().uninstallPlugin( plugin );
        OGRE_DELETE plugin;
    }
#endif
}  // namespace Ogre
