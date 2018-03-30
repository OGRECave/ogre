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
#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"
#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"
#include "OgreGLPBuffer.h"

#include "OgreGLNativeSupport.h"

namespace Ogre
{

class _OgreGLExport GLSupport
{
public:
    GLSupport(GLNativeSupport* native) : mNative(native) { }
    virtual ~GLSupport() {
        delete mNative;
    }

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    void addConfig() {
        mNative->addConfig();
    }

    void setConfigOption(const String &name, const String &value) {
        mNative->setConfigOption(name, value);
    }

    /**
    * Make sure all the extra options are valid
    * @return string with error message
    */
    String validateConfig() {
        return mNative->validateConfig();
    }

    ConfigOptionMap& getConfigOptions(void) {
        return mNative->getConfigOptions();
    }

    NameValuePairList parseOptions(uint& w, uint& h, bool& fullscreen) {
        return mNative->parseOptions(w, h, fullscreen);
    }

    /// @copydoc RenderSystem::_createRenderWindow
    RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height,
        bool fullScreen, const NameValuePairList *miscParams = 0) {
        return mNative->newWindow(name, width, height, fullScreen, miscParams);
    }

    bool supportsPBuffers();

    GLPBuffer *createPBuffer(PixelComponentType format, size_t width, size_t height) {
        return mNative->createPBuffer(format, width, height);
    }

    /**
    * Start anything special
    */
    void start() {
        mNative->start();
    }
    /**
    * Stop anything special
    */
    void stop() {
        mNative->stop();
    }

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
    * Compare GL version numbers
    */
    bool hasMinGLVersion(const String& v) const;

    /**
    * Check if an extension is available
    */
    bool checkExtension(const String& ext) const;
    /**
    * Get the address of a function
    */
    void* getProcAddress(const char* procname) {
        return mNative->getProcAddress(procname);
    }

    /** Initialises GL extensions, must be done AFTER the GL context has been
        established.
    */
    void initialiseExtensions();

    /// @copydoc RenderSystem::getDisplayMonitorCount
    virtual unsigned int getDisplayMonitorCount() const
    {
        return mNative->getDisplayMonitorCount();
    }

protected:
    // Stored options
    ConfigOptionMap mOptions;

    // This contains the complete list of supported extensions
    std::set<String> extensionList;
private:
    String mVersion;
    String mVendor;

    GLNativeSupport* mNative;

}; // class GLSupport

} // namespace Ogre

#endif // OGRE_GLSUPPORT_H
