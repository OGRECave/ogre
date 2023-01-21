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

#ifndef __EmscriptenEGLWindow_H__
#define __EmscriptenEGLWindow_H__

#include "OgreEGLWindow.h"
#include "OgreEmscriptenEGLSupport.h"
#include <emscripten/html5.h>


#ifndef EGL_COVERAGE_BUFFERS_NV
#define EGL_COVERAGE_BUFFERS_NV 0x30E0
#endif

#ifndef EGL_COVERAGE_SAMPLES_NV
#define EGL_COVERAGE_SAMPLES_NV 0x30E1
#endif

namespace Ogre {
    class _OgrePrivate EmscriptenEGLWindow : public EGLWindow
    {
    private:
        int mMaxBufferSize;
        int mMinBufferSize;
        int mMaxDepthSize;
        int mMaxStencilSize;
        int mMSAA;
        int mCSAA;
        int mOldWidth;
        int mOldHeight;
        String mCanvasSelector;
        
    protected:
        void resize(unsigned int width, unsigned int height) override;
        void windowMovedOrResized() override;
        void switchFullScreen(bool fullscreen) override;
        
        static EM_BOOL canvasWindowResized(int eventType, const EmscriptenUiEvent *uiEvent, void *userData);
        static EM_BOOL fullscreenCallback(int eventType, const EmscriptenFullscreenChangeEvent* event, void* userData);
        static EM_BOOL contextLostCallback(int eventType, const void *reserved, void *userData);
        static EM_BOOL contextRestoredCallback(int eventType, const void *reserved, void *userData);
   
    public:
        EmscriptenEGLWindow(EmscriptenEGLSupport* glsupport);
        virtual ~EmscriptenEGLWindow();
        void create(const String& name, unsigned int width, unsigned int height,
                    bool fullScreen, const NameValuePairList *miscParams) override;

        void _notifySurfaceDestroyed() override;
        void _notifySurfaceCreated(void* window, void* config = NULL) override;
        void swapBuffers() override;
    };
}

#endif
