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

#ifndef __GL3PlusSupport_H__
#define __GL3PlusSupport_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"

namespace Ogre
{
    class GL3PlusRenderSystem;

    class _OgreGL3PlusExport GL3PlusSupport
    {
        public:
            GL3PlusSupport() { }
            virtual ~GL3PlusSupport() { }

            /**
            * Add any special config values to the system.
            * Must have a "Full Screen" value that is a bool and a "Video Mode" value
            * that is a string in the form of wxh
            */
            virtual void addConfig() = 0;
            virtual void setConfigOption(const String &name, const String &value);

           /**
            * Make sure all the extra options are valid
            * @return string with error message
            */
            virtual String validateConfig() = 0;
            virtual ConfigOptionMap& getConfigOptions(void);
            virtual RenderWindow* createWindow(bool autoCreateWindow,
                                               GL3PlusRenderSystem *renderSystem,
                                               const String& windowTitle) = 0;

            /// @copydoc RenderSystem::_createRenderWindow
            virtual RenderWindow* newWindow(const String &name,
                                            unsigned int width, unsigned int height,
                                            bool fullScreen,
                                            const NameValuePairList *miscParams = 0) = 0;

            /**
            * Get vendor information
            */
            const String& getGLVendor(void) const
            {
                return mVendor;
            }

            /**
            * Get version information
            */
            const String& getGLVersion(void) const
            {
                return mVersion;
            }

            /**
            * Get shader cache path
            */
            const String& getShaderCachePath(void) const
            {
                return mShaderCachePath;
            }

            /**
            * Get shader library path
            */
            const String& getShaderLibraryPath(void) const
            {
                return mShaderLibraryPath;
            }

            /**
            * Set shader cache path
            */
            void setShaderCachePath(String path)
            {
                mShaderCachePath = path;
            }

            /**
            * Set shader library path
            */
            void setShaderLibraryPath(String path)
            {
                mShaderLibraryPath = path;
            }

            /**
            * Compare GL version numbers
            */
            bool checkMinGLVersion(const String& v) const;

            /**
            * Get the address of a function
            */
            virtual void *getProcAddress(const String& procname) = 0;

            /** Initialises GL extensions, must be done AFTER the GL context has been
               established.
            */
            virtual void initialiseExtensions();

            /**
            * Check if an extension is available
            */
            virtual bool checkExtension(const String& ext) const;

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

        private:
            String mVersion;
            String mVendor;
            String mShaderCachePath;
            String mShaderLibraryPath;

        protected:
            // Stored options
            ConfigOptionMap mOptions;

            // This contains the complete list of supported extensions
            set<String>::type extensionList;
    };

}

#endif
