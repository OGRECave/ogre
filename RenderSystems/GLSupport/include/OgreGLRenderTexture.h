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

#ifndef __GLRENDERTEXTURE_H__
#define __GLRENDERTEXTURE_H__

#include "OgreGLSupportPrerequisites.h"
#include "OgreRenderTexture.h"
#include "OgreSingleton.h"

namespace Ogre {
    class GLHardwarePixelBufferCommon;

    /** GL surface descriptor. Points to a 2D surface that can be rendered to.
     */
    struct _OgrePrivate GLSurfaceDesc
    {
    public:
        GLHardwarePixelBufferCommon *buffer;
        uint32 zoffset;
        uint numSamples;

        GLSurfaceDesc() : buffer(0), zoffset(0), numSamples(0) {}
    };

    /** Base class for GL Render Textures
     */
    class _OgreGLExport GLRenderTexture : public RenderTexture
    {
    public:
        GLRenderTexture(const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        bool requiresTextureFlipping() const { return true; }

        static const String CustomAttributeString_FBO;
        static const String CustomAttributeString_TARGET;
        static const String CustomAttributeString_GLCONTEXT;
    };

    /** Manager/factory for RenderTextures.
     */
    class _OgreGLExport GLRTTManager : public Singleton<GLRTTManager>
    {
    public:
        GLRTTManager();
        virtual ~GLRTTManager();

        /** Create a texture rendertarget object
         */
        virtual RenderTexture *createRenderTexture(const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa) = 0;

        /** Release a render buffer. Ignore silently if surface.buffer is 0.
         */
        void releaseRenderBuffer(const GLSurfaceDesc &surface);

        /** Check if a certain format is usable as FBO rendertarget format
        */
        bool checkFormat(PixelFormat format) { return mProps[format].valid; }

        /** Bind a certain render target.
         */
        virtual void bind(RenderTarget *target) = 0;

        /** Unbind a certain render target. This is called before binding another RenderTarget, and
            before the context is switched. It can be used to do a copy, or just be a noop if direct
            binding is used.
        */
        virtual void unbind(RenderTarget *target) = 0;

        virtual void getBestDepthStencil(PixelFormat internalFormat, uint32 *depthFormat, uint32 *stencilFormat)
        {
            *depthFormat = 0;
            *stencilFormat = 0;
        }

        /** Create a multi render target
         */
        virtual MultiRenderTarget* createMultiRenderTarget(const String & name);

        /** Get the closest supported alternative format. If format is supported, returns format.
         */
        PixelFormat getSupportedAlternative(PixelFormat format);

        /// @copydoc Singleton::getSingleton()
        static GLRTTManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static GLRTTManager* getSingletonPtr(void);
    protected:
        /** Frame Buffer Object properties for a certain texture format.
         */
        struct FormatProperties
        {
            bool valid; // This format can be used as RTT (FBO)

            /** Allowed modes/properties for this pixel format
             */
            struct Mode
            {
                size_t depth;     // Depth format (0=no depth)
                size_t stencil;   // Stencil format (0=no stencil)
            };

            std::vector<Mode> modes;
        };
        /** Properties for all internal formats defined by OGRE
         */
        FormatProperties mProps[PF_COUNT];

        /** Stencil and depth renderbuffers of the same format are re-used between surfaces of the
            same size and format. This can save a lot of memory when a large amount of rendertargets
            are used.
        */
        struct RBFormat
        {
            RBFormat(uint inFormat, size_t inWidth, size_t inHeight, uint fsaa)
                : format(inFormat), width(inWidth), height(inHeight), samples(fsaa)
            {
            }
            RBFormat() {}
            uint format;
            size_t width;
            size_t height;
            uint samples;
            // Overloaded comparison operator for usage in map
            bool operator < (const RBFormat &other) const
            {
                if(format < other.format)
                {
                    return true;
                }
                else if(format == other.format)
                {
                    if(width < other.width)
                    {
                        return true;
                    }
                    else if(width == other.width)
                    {
                        if(height < other.height)
                            return true;
                        else if (height == other.height)
                        {
                            if (samples < other.samples)
                                return true;
                        }
                    }
                }
                return false;
            }
        };
        struct RBRef
        {
            RBRef() {}
            RBRef(GLHardwarePixelBufferCommon* inBuffer) : buffer(inBuffer), refcount(1) {}
            GLHardwarePixelBufferCommon* buffer;
            size_t refcount;
        };
        typedef std::map<RBFormat, RBRef> RenderBufferMap;
        RenderBufferMap mRenderBufferMap;
    };

}

#endif
