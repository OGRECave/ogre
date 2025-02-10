/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#ifndef OGRE_OSXGLSupport_H
#define OGRE_OSXGLSupport_H

#include "OgreGLNativeSupport.h"
#include <CoreFoundation/CoreFoundation.h>

namespace Ogre
{
    
class OSXGLSupport : public GLNativeSupport
{
public:
    OSXGLSupport(int profile) : GLNativeSupport(profile) {}
    ~OSXGLSupport() {}

    ConfigOptionMap getConfigOptions() override;
    
    /// @copydoc RenderSystem::createRenderWindow
    RenderWindow* newWindow( const String &name, unsigned int width, unsigned int height,
        bool fullScreen, const NameValuePairList *miscParams = 0 ) override;
    
    /**
    * Start anything special
    */
    void start() override;

    /**
    * Stop anything special
    */
    void stop() override;

    /**
    * Get the address of a function
    */
    void* getProcAddress( const char *name ) const override;
    
    // Core Foundation Array callback function for sorting, must be static for the function ptr
    static CFComparisonResult _compareModes (const void *val1, const void *val2, void *context);
    // Core Fondation Dictionary helper functions, also static for ease of use in above static
    static Boolean _getDictionaryBoolean(CFDictionaryRef dict, const void* key);
    static long _getDictionaryLong(CFDictionaryRef dict, const void* key);

};

} // namespace Ogre

#endif // OGRE_OSXGLSupport_H
