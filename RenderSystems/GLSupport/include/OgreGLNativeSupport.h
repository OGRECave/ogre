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

namespace Ogre
{
    class GLPBuffer;

    class _OgreGLExport GLNativeSupport
    {
        public:
            typedef set<String>::type ExtensionList;
            virtual ~GLNativeSupport() {}

            /**
            * Add any special config values to the system.
            * Must have a "Full Screen" value that is a bool and a "Video Mode" value
            * that is a string in the form of wxh
            */
            virtual void addConfig() = 0;

            virtual void setConfigOption(const String& name, const String& value) {
                ConfigOptionMap::iterator option = mOptions.find(name);
                if (option == mOptions.end()) {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                                "Option named " + name + " does not exist.",
                                "GLNativeSupport::setConfigOption");
                }
                option->second.currentValue = value;
            }

           /**
            * Make sure all the extra options are valid
            * @return string with error message
            */
            virtual String validateConfig() = 0;
            //virtual ConfigOptionMap& getConfigOptions() = 0;
            virtual RenderWindow* createWindow(bool autoCreateWindow,
                                               RenderSystem *renderSystem,
                                               const String& windowTitle) = 0;

            /// @copydoc RenderSystem::_createRenderWindow
            virtual RenderWindow* newWindow(const String &name,
                                            unsigned int width, unsigned int height,
                                            bool fullScreen,
                                            const NameValuePairList *miscParams = 0) = 0;

            virtual bool supportsPBuffers() const {
                return false;
            }

            //virtual GLPBuffer* createPBuffer(PixelComponentType format, size_t width, size_t height) {
            //    return NULL;
            //}

            /**
            * Get the address of a function
            */
            virtual void *getProcAddress(const String& procname) = 0;

            /** Initialises GL extensions, must be done AFTER the GL context has been
               established.
            */
            virtual void initialiseExtensions(ExtensionList& extensionList) = 0;

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

            ConfigOptionMap& getConfigOptions() {
                return mOptions;
            }

        protected:
            // TODO: remove the members here: this should be a pure interface

            // Stored options
            ConfigOptionMap mOptions;
    };

}

#endif
