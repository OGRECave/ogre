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

#ifndef __AndroidEGLWindow_H__
#define __AndroidEGLWindow_H__

#include "OgreEGLWindow.h"
#include "OgreAndroidEGLSupport.h"
#include "android/configuration.h"

#ifndef EGL_COVERAGE_BUFFERS_NV
#define EGL_COVERAGE_BUFFERS_NV 0x30E0
#endif

#ifndef EGL_COVERAGE_SAMPLES_NV
#define EGL_COVERAGE_SAMPLES_NV 0x30E1
#endif

namespace Ogre {
    class _OgrePrivate AndroidEGLWindow : public EGLWindow
    {
    private:
        int mMaxBufferSize;
        int mMinBufferSize;
        int mMaxDepthSize;
        int mMaxStencilSize;
        int mMSAA;
        int mCSAA;
        bool mPreserveContext;
        float mScale;
        
    protected:
        virtual void getLeftAndTopFromNativeWindow(int & left, int & top, uint width, uint height);
        virtual void initNativeCreatedWindow(const NameValuePairList *miscParams);
        virtual void createNativeWindow( int &left, int &top, uint &width, uint &height, String &title );
        virtual void reposition(int left, int top);
        virtual void resize(unsigned int width, unsigned int height);
        virtual void windowMovedOrResized();
        virtual void switchFullScreen(bool fullscreen);
        
    public:
        AndroidEGLWindow(AndroidEGLSupport* glsupport);
        void create(const String& name, unsigned int width, unsigned int height,
                    bool fullScreen, const NameValuePairList *miscParams);
        
        float getViewPointToPixelScale() { return mScale; }

        void _notifySurfaceDestroyed();
        void _notifySurfaceCreated(void* window, void* config);
    };
}

#endif
