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
#ifndef __OgreGL3PlusFBORTT_H__
#define __OgreGL3PlusFBORTT_H__

#include "OgreGL3PlusRenderTexture.h"
#include "OgreGL3PlusContext.h"
#include "OgreGL3PlusFrameBufferObject.h"

namespace Ogre {
    class GL3PlusFBOManager;
    class GL3PlusRenderBuffer;

    /** RenderTexture for GL FBO
    */
    class _OgreGL3PlusExport GL3PlusFBORenderTexture: public GL3PlusRenderTexture
    {
    public:
        GL3PlusFBORenderTexture(GL3PlusFBOManager *manager, const String &name, const GL3PlusSurfaceDesc &target, bool writeGamma, uint fsaa);

        virtual void getCustomAttribute(const String& name, void* pData);

		/// Override needed to deal with multisample buffers
		virtual void swapBuffers();

		/// Override so we can attach the depth buffer to the FBO
		virtual bool attachDepthBuffer( DepthBuffer *depthBuffer );
		virtual void detachDepthBuffer();
		virtual void _detachDepthBuffer();
    protected:
        GL3PlusFrameBufferObject mFB;
    };
    
    /** Factory for GL Frame Buffer Objects, and related things.
    */
    class _OgreGL3PlusExport GL3PlusFBOManager: public GL3PlusRTTManager
    {
    public:
        GL3PlusFBOManager();
		~GL3PlusFBOManager();
        
        /** Bind a certain render target if it is a FBO. If it is not a FBO, bind the
            main frame buffer.
        */
        void bind(RenderTarget *target);
        
        /** Unbind a certain render target. No-op for FBOs.
        */
        void unbind(RenderTarget *target) {};
        
        /** Get best depth and stencil supported for given internalFormat
        */
        void getBestDepthStencil(GLenum internalFormat, GLenum *depthFormat, GLenum *stencilFormat);
        
        /** Create a texture rendertarget object
        */
        virtual GL3PlusFBORenderTexture *createRenderTexture(const String &name, 
			const GL3PlusSurfaceDesc &target, bool writeGamma, uint fsaa);

		/** Create a multi render target 
		*/
		virtual MultiRenderTarget* createMultiRenderTarget(const String & name);
        
        /** Request a render buffer. If format is GL_NONE, return a zero buffer.
        */
        GL3PlusSurfaceDesc requestRenderBuffer(GLenum format, uint32 width, uint32 height, uint fsaa);
        /** Request the specify render buffer in case shared somewhere. Ignore
            silently if surface.buffer is 0.
        */
        void requestRenderBuffer(const GL3PlusSurfaceDesc &surface);
        /** Release a render buffer. Ignore silently if surface.buffer is 0.
        */
        void releaseRenderBuffer(const GL3PlusSurfaceDesc &surface);
        
        /** Check if a certain format is usable as FBO rendertarget format
        */
        bool checkFormat(PixelFormat format) { return mProps[format].valid; }
        
        /** Get a FBO without depth/stencil for temporary use, like blitting between textures.
        */
        GLuint getTemporaryFBO(size_t id);
    private:
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
            
            vector<Mode>::type modes;
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
            RBFormat(GLenum inFormat, size_t inWidth, size_t inHeight, uint fsaa):
                format(inFormat), width(inWidth), height(inHeight), samples(fsaa)
            {}
            RBFormat() {}
            GLenum format;
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
            RBRef(){}
            RBRef(GL3PlusRenderBuffer *inBuffer):
                buffer(inBuffer), refcount(1)
            { }
            GL3PlusRenderBuffer *buffer;
            size_t refcount;
        };
        typedef map<RBFormat, RBRef>::type RenderBufferMap;
        RenderBufferMap mRenderBufferMap;
        
        /** Temporary FBO identifier
         */
        std::vector<GLuint> mTempFBO;
        
        /** Detect allowed FBO formats */
        void detectFBOFormats();
        GLuint _tryFormat(GLenum depthFormat, GLenum stencilFormat);
        bool _tryPackedFormat(GLenum packedFormat);
        void _createTempFramebuffer(int ogreFormat, GLuint internalFormat, GLuint fmt, GLenum dataType, GLuint &fb, GLuint &tid);
    };
}

#endif
