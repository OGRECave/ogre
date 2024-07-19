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
#include "OgreGLRenderTarget.h"

namespace Ogre {
    class GLHardwarePixelBufferCommon;
    class GLRTTManager;

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

    /// Frame Buffer Object abstraction
    class _OgreGLExport GLFrameBufferObjectCommon
    {
    public:
        GLFrameBufferObjectCommon(int32 fsaa, GLRTTManager&);
        virtual ~GLFrameBufferObjectCommon();

        /** Bind FrameBufferObject. Attempt to bind on incompatible GL context will cause FBO destruction and optional recreation.
        */
        virtual bool bind(bool recreateIfNeeded) = 0;

        /** Bind a surface to a certain attachment point.
            attachment: 0..OGRE_MAX_MULTIPLE_RENDER_TARGETS-1
        */
        void bindSurface(size_t attachment, const GLSurfaceDesc &target);
        /** Unbind attachment
        */
        void unbindSurface(size_t attachment);

        /** Determines and sets mAllowRenderBufferSharing based on given render target properties
         */
        void determineFBOBufferSharingAllowed(RenderTarget&);

        /** Sets mAllowRenderBufferSharing, triggers re-initialization if value is different
         */
        void setAllowRenderBufferSharing(bool);

        /// Accessors
        int32 getFSAA() const { return mNumSamples; }
        uint32 getWidth() const;
        uint32 getHeight() const;
        PixelFormat getFormat() const;

        GLContext* getContext() const { return mContext; }
        /// Get the GL id for the FBO
        uint32 getGLFBOID() const { return mFB; }
        /// Get the GL id for the multisample FBO
        uint32 getGLMultisampleFBOID() const { return mMultisampleFB; }

        const GLSurfaceDesc &getSurface(size_t attachment) const { return mColour[attachment]; }

        void notifyContextDestroyed(GLContext* context) { if(mContext == context) { mContext = 0; mFB = 0; mMultisampleFB = 0; } }
    protected:
        GLSurfaceDesc mDepth;
        GLSurfaceDesc mStencil;
        // Arbitrary number of texture surfaces
        GLSurfaceDesc mColour[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        /// Context that was used to create FBO. It could already be destroyed, so do not dereference this field blindly
        GLContext* mContext;
        uint32 mFB;
        uint32 mMultisampleFB;
        int32 mNumSamples;
        GLRTTManager* mRTTManager;
        GLSurfaceDesc mMultisampleColourBuffer;
        bool mAllowRenderBufferSharing = true;
        // mMultisampleColourBuffer.buffer is either shared through caching, or owned,
        // if owned, mOwnedMultisampleColourBuffer contains mMultisampleColourBuffer.buffer
        // otherwise, mOwnedMultisampleColourBuffer == nullptr
        std::unique_ptr<GLHardwarePixelBufferCommon> mOwnedMultisampleColourBuffer;

        /** Initialise object (find suitable depth and stencil format).
            Must be called every time the bindings change.
            It fails with an exception (ERR_INVALIDPARAMS) if:
            - Attachment point 0 has no binding
            - Not all bound surfaces have the same size
            - Not all bound surfaces have the same internal format
        */
        virtual void initialise() = 0;
        void releaseMultisampleColourBuffer();
        void initialiseMultisampleColourBuffer(unsigned format, uint32 width, uint32 height);
    };

    /** Base class for GL Render Textures
     */
    class _OgreGLExport GLRenderTexture : public RenderTexture, public GLRenderTarget
    {
    public:
        GLRenderTexture(const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);
        bool requiresTextureFlipping() const override { return true; }

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
            @note only needed for FBO RTTs
         */
        virtual void bind(RenderTarget *target) {}

        /** Unbind a certain render target. This is called before binding another RenderTarget, and
            before the context is switched. It can be used to do a copy, or just be a noop if direct
            binding is used.
            @note only needed for Copying or PBuffer RTTs
        */
        virtual void unbind(RenderTarget *target) {}

        virtual void getBestDepthStencil(PixelFormat internalFormat, uint32 *depthFormat, uint32 *stencilFormat)
        {
            *depthFormat = 0;
            *stencilFormat = 0;
        }

        /** Get the closest supported alternative format. If format is supported, returns format.
         */
        PixelFormat getSupportedAlternative(PixelFormat format);

        /// @copydoc Singleton::getSingleton()
        static GLRTTManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static GLRTTManager* getSingletonPtr(void);

        /** Request a render buffer. If format is GL_NONE, return a zero buffer.
         */
        GLSurfaceDesc requestRenderBuffer(unsigned format, uint32 width, uint32 height, uint fsaa);

        /** Creates a new render buffer. Caller takes ownership.
         */
        virtual GLSurfaceDesc createNewRenderBuffer(unsigned format, uint32 width, uint32 height, uint fsaa)
        {
            return {};
        }

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
                uchar depth;     // Depth format (0=no depth)
                uchar stencil;   // Stencil format (0=no stencil)
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
