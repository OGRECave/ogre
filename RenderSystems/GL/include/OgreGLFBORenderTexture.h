/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __OgreGLFBORTT_H__
#define __OgreGLFBORTT_H__

#include "OgreGLRenderTexture.h"
#include "OgreGLContext.h"
#include "OgreGLFrameBufferObject.h"

/// Extra GL constants
#define GL_DEPTH24_STENCIL8_EXT                           0x88F0


namespace Ogre {
    class GLFBOManager;

    /** RenderTexture for GL FBO
    */
    class _OgrePrivate GLFBORenderTexture: public GLRenderTexture
    {
    public:
        GLFBORenderTexture(GLFBOManager *manager, const String &name, const GLSurfaceDesc &target, bool writeGamma, uint fsaa);

        virtual void getCustomAttribute(const String& name, void* pData);

		/// Override needed to deal with multisample buffers
		virtual void swapBuffers(bool waitForVSync = true);
    protected:
        GLFrameBufferObject mFB;
    };
    
    /** Factory for GL Frame Buffer Objects, and related things.
    */
    class _OgrePrivate GLFBOManager: public GLRTTManager
    {
    public:
        GLFBOManager(bool atimode);
		~GLFBOManager();
        
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
        virtual GLFBORenderTexture *createRenderTexture(const String &name, 
			const GLSurfaceDesc &target, bool writeGamma, uint fsaa);

		/** Create a multi render target 
		*/
		virtual MultiRenderTarget* createMultiRenderTarget(const String & name);
        
        /** Request a render buffer. If format is GL_NONE, return a zero buffer.
        */
        GLSurfaceDesc requestRenderBuffer(GLenum format, size_t width, size_t height, uint fsaa);
        /** Request the specify render buffer in case shared somewhere. Ignore
            silently if surface.buffer is 0.
        */
        void requestRenderBuffer(const GLSurfaceDesc &surface);
        /** Release a render buffer. Ignore silently if surface.buffer is 0.
        */
        void releaseRenderBuffer(const GLSurfaceDesc &surface);
        
        /** Check if a certain format is usable as FBO rendertarget format
        */
        bool checkFormat(PixelFormat format) { return mProps[format].valid; }
        
        /** Get a FBO without depth/stencil for temporary use, like blitting between textures.
        */
        GLuint getTemporaryFBO() { return mTempFBO; }
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
            RBFormat(GLenum format, size_t width, size_t height, uint fsaa):
                format(format), width(width), height(height), samples(fsaa)
            {}
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
            RBRef(GLRenderBuffer *buffer):
                buffer(buffer), refcount(1)
            { }
            GLRenderBuffer *buffer;
            size_t refcount;
        };
        typedef map<RBFormat, RBRef>::type RenderBufferMap;
        RenderBufferMap mRenderBufferMap;
        // map(format, sizex, sizey) -> [GLSurface*,refcount]
        
        /** Temporary FBO identifier
         */
        GLuint mTempFBO;
        
		/// Buggy ATI driver?
		bool mATIMode;
        
        /** Detect allowed FBO formats */
        void detectFBOFormats();
        GLuint _tryFormat(GLenum depthFormat, GLenum stencilFormat);
        bool _tryPackedFormat(GLenum packedFormat);
    };
    

}

#endif
