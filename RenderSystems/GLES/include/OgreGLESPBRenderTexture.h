/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __GLESPBRenderTexture_H__
#define __GLESPBRenderTexture_H__

#include "OgreGLESRenderTexture.h"

namespace Ogre {

    /** RenderTexture that uses a PBuffer (offscreen rendering context) for rendering.
    */
    class GLESPBRTTManager;
    class GLESPBuffer;
    class GLESContext;
    class _OgreGLESExport GLESPBRenderTexture: public GLESRenderTexture
    {
        public:
            GLESPBRenderTexture(GLESPBRTTManager *manager, const String &name, const GLESSurfaceDesc &target, bool writeGamma, uint fsaa);
            virtual ~GLESPBRenderTexture();
            virtual void getCustomAttribute(const String& name, void* pData);

        protected:
            GLESPBRTTManager *mManager;
            PixelComponentType mPBFormat;
    };

    /** Manager for rendertextures and PBuffers (offscreen rendering contexts)
    */
    class _OgreGLESExport GLESPBRTTManager: public GLESRTTManager
    {
        public:
            GLESPBRTTManager(GLESSupport *support, RenderTarget *mainwindow);
            virtual ~GLESPBRTTManager();

        /** @copydoc GLRTTManager::createRenderTexture
        */
            virtual RenderTexture *createRenderTexture(const String &name,
                                                       const GLESSurfaceDesc &target,
                                                       bool writeGamma, uint fsaa);

            /** @copydoc GLRTTManager::checkFormat
             */
            virtual bool checkFormat(PixelFormat format);

            /** @copydoc GLRTTManager::bind
             */
            virtual void bind(RenderTarget *target);

            /** @copydoc GLRTTManager::unbind
             */
            virtual void unbind(RenderTarget *target);

            /** Create PBuffer for a certain pixel format and size
             */
            void requestPBuffer(PixelComponentType ctype, size_t width, size_t height);

            /** Release PBuffer for a certain pixel format
             */
            void releasePBuffer(PixelComponentType ctype);

        protected:
            /** GLESSupport reference, used to create PBuffers */
            GLESSupport *mSupport;
            /** Primary window reference */
            RenderTarget *mMainWindow;
            /** Primary window context */
            GLESContext *mMainContext;
            /** Reference to a PBuffer, with refcount */
            struct PBRef
            {
                PBRef(): pb(0),refcount(0) {}
                GLESPBuffer* pb;
                size_t refcount;
            };
            /** Type to map each component type to a PBuffer */
            PBRef mPBuffers[PCT_COUNT];
    };
}

#endif
