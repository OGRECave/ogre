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

#ifndef __GLNativeSupport_H__
#define __GLNativeSupport_H__

#include "OgreGLSupportPrerequisites.h"
#include "OgreConfigOptionMap.h"
#include "OgrePixelFormat.h"
#include "OgreException.h"
#include "OgreGLRenderSystemCommon.h"

namespace Ogre
{
    class GLPBuffer;

    /** \addtogroup RenderSystems RenderSystems
    *  @{
    */
    /** \defgroup GLSupport GLSupport
    * provides OpenGL Context creation using GLX, WGL, EGL, Cocoa
    *  @{
    */
    class _OgreGLExport GLNativeSupport
    {
        public:
            typedef GLRenderSystemCommon::VideoModes VideoModes;
            typedef std::set<String> ExtensionList;

            enum ContextProfile {
                CONTEXT_CORE = 1,
                CONTEXT_COMPATIBILITY = 2,
                CONTEXT_ES = 4
            };

            GLNativeSupport(int profile) : mContextProfile(ContextProfile(profile)) {}
            virtual ~GLNativeSupport() {}

            /// @copydoc RenderSystem::_createRenderWindow
            virtual RenderWindow* newWindow(const String &name,
                                            unsigned int width, unsigned int height,
                                            bool fullScreen,
                                            const NameValuePairList *miscParams = 0) = 0;

            virtual GLPBuffer* createPBuffer(PixelComponentType format, size_t width, size_t height) {
                return NULL;
            }

            /**
            * Get the address of a function
            */
            virtual void *getProcAddress(const char* procname) const = 0;

            bool checkExtension(const String& ext) const {
                return extensionList.find(ext) != extensionList.end();
            }

            /// @copydoc RenderSystem::getDisplayMonitorCount
            virtual unsigned int getDisplayMonitorCount() const
            {
                return 1;
            }

            /**
            * Start anything special
            */
            virtual void start() = 0;
            /**
            * Stop anything special
            */
            virtual void stop() = 0;

            /**
            * Add any special config values to the system.
            */
            virtual ConfigOptionMap getConfigOptions() { return ConfigOptionMap(); }

            const std::vector<int>& getFSAALevels() const { return mFSAALevels; }
            const VideoModes& getVideoModes() const { return mVideoModes; }

            ContextProfile getContextProfile() const { return mContextProfile; }
        protected:
            typedef GLRenderSystemCommon::VideoMode VideoMode;

            // Allowed video modes
            VideoModes mVideoModes;
            std::vector<int> mFSAALevels;

            // Supported platform extensions (e.g EGL_*, GLX_*)
            ExtensionList extensionList;

            ContextProfile mContextProfile;
    };
    /** @} */
    /** @} */

}

#endif
