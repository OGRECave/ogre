/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __GLESRenderTexture_H__
#define __GLESRenderTexture_H__

#include "OgreGLESTexture.h"

namespace Ogre {
    class GLESHardwarePixelBuffer;

    /** GL surface descriptor. Points to a 2D surface that can be rendered to.
    */
    struct _OgrePrivate GLESSurfaceDesc
    {
        public:
            GLESHardwarePixelBuffer *buffer;
            size_t zoffset;
            uint numSamples;

            GLESSurfaceDesc() :buffer(0), zoffset(0), numSamples(0) {}
    };

    /** Base class for GL Render Textures
    */
    class _OgrePrivate GLESRenderTexture : public RenderTexture
    {
        public:
            GLESRenderTexture(const String &name, const GLESSurfaceDesc &target, bool writeGamma, uint fsaa);
            virtual ~GLESRenderTexture();
            bool requiresTextureFlipping() const { return true; }
    };

    /** Manager/factory for RenderTextures.
    */
    class _OgrePrivate GLESRTTManager : public Singleton<GLESRTTManager>
    {
        public:
            virtual ~GLESRTTManager();

            /** Create a texture rendertarget object
            */
            virtual RenderTexture *createRenderTexture(const String &name, const GLESSurfaceDesc &target, bool writeGamma, uint fsaa) = 0;

             /** Check if a certain format is usable as rendertexture format
            */
            virtual bool checkFormat(PixelFormat format) = 0;

            /** Bind a certain render target.
            */
            virtual void bind(RenderTarget *target) = 0;

            /** Unbind a certain render target. This is called before binding another RenderTarget, and
                before the context is switched. It can be used to do a copy, or just be a noop if direct
                binding is used.
            */
            virtual void unbind(RenderTarget *target) = 0;

            /** Create a multi render target
            */
            virtual MultiRenderTarget* createMultiRenderTarget(const String & name);

            /** Get the closest supported alternative format. If format is supported, returns format.
            */
            virtual PixelFormat getSupportedAlternative(PixelFormat format);
    };

    /** RenderTexture for simple copying from frame buffer
    */
    class GLESCopyingRTTManager;
    class _OgrePrivate GLESCopyingRenderTexture : public GLESRenderTexture
    {
        public:
            GLESCopyingRenderTexture(GLESCopyingRTTManager *manager,
                                   const String &name,
                                   const GLESSurfaceDesc &target,
                                   bool writeGamma, uint fsaa);

            virtual void getCustomAttribute(const String& name, void* pData);
    };

    /** Simple, copying manager/factory for RenderTextures. This is only used as the last fallback if
        both PBuffers and FBOs aren't supported.
    */
    class _OgrePrivate GLESCopyingRTTManager : public GLESRTTManager
    {
        public:
            GLESCopyingRTTManager();
            virtual ~GLESCopyingRTTManager();

            /** @copydoc GLESRTTManager::createRenderTexture
            */
            virtual RenderTexture *createRenderTexture(const String &name, const GLESSurfaceDesc &target, bool writeGamma, uint fsaa);
            
            /** @copydoc GLESRTTManager::checkFormat
            */
            virtual bool checkFormat(PixelFormat format);

            /** @copydoc GLESRTTManager::bind
            */
            virtual void bind(RenderTarget *target);

            /** @copydoc GLESRTTManager::unbind
            */
            virtual void unbind(RenderTarget *target);
    };
}

#endif
