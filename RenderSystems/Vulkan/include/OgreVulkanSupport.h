/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-present Torus Knot Software Ltd

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

#ifndef _OgreVulkanSupport_H_
#define _OgreVulkanSupport_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreConfigOptionMap.h"
#include "OgreRenderSystemCapabilities.h"

namespace Ogre
{
    class VulkanRenderSystem;

    class _OgreVulkanExport VulkanSupport
    {
        FastArray<String> mDevices;

        void enumerateDevices( VulkanRenderSystem *renderSystem );

        void initialize( VulkanRenderSystem *renderSystem );

    public:
        VulkanSupport() {}
        virtual ~VulkanSupport() {}

        /**
         * Add any special config values to the system.
         * Must have a "Full Screen" value that is a bool and a "Video Mode" value
         * that is a string in the form of wxh
         */
        virtual void addConfig( VulkanRenderSystem *renderSystem );
        virtual void setConfigOption( const String &name, const String &value );

        virtual String validateConfigOptions( void );

        uint32 getSelectedDeviceIdx( void ) const;

        ConfigOptionMap &getConfigOptions( VulkanRenderSystem *renderSystem );

        /// @copydoc RenderSystem::getDisplayMonitorCount
        virtual unsigned int getDisplayMonitorCount() const { return 1; }

    protected:
        // Stored options
        ConfigOptionMap mOptions;
    };
}  // namespace Ogre

#ifndef DEFINING_VK_SUPPORT_IMPL
#    if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#        include "Windowing/win32/OgreVulkanWin32Support.h"
namespace Ogre
{
    inline VulkanSupport *getVulkanSupport() { return new VulkanWin32Support(); }
}  // namespace Ogre
#    elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#        include "Windowing/X11/OgreVulkanXcbSupport.h"
namespace Ogre
{
    inline VulkanSupport *getVulkanSupport() { return new VulkanXcbSupport(); }
}  // namespace Ogre
#    elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#        include "Windowing/Android/OgreVulkanAndroidSupport.h"
namespace Ogre
{
    inline VulkanSupport *getVulkanSupport() { return new VulkanAndroidSupport(); }
}  // namespace Ogre
#    endif
#endif

#endif
