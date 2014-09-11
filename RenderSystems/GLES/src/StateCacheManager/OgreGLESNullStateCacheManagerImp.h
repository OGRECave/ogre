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

#ifndef __GLESNullStateCacheManagerImp_H__
#define __GLESNullStateCacheManagerImp_H__

#include "OgreGLESPrerequisites.h"

typedef _OgreGLESExport Ogre::GeneralAllocatedObject StateCacheAlloc;

namespace Ogre
{
    /** An in memory cache of the OpenGL ES state.
     @see GLESStateCacheManager
     */
    class GLESStateCacheManagerImp : public StateCacheAlloc
    {
    private:        
        /// Stores the current clear colour
        vector<GLclampf>::type mClearColour;
        /// Stores the current colour write mask
        vector<GLboolean>::type mColourMask;
        /// Stores the current depth write mask
        GLboolean mDepthMask;
        /// Stores the current polygon rendering mode
        GLenum mPolygonMode;
        /// Stores the current blend equation
        GLenum mBlendEquation;
        /// Stores the current face culling setting
        GLenum mCullFace;
        /// Stores the current depth test function
        GLenum mDepthFunc;
        /// Stores the current stencil mask
        GLuint mStencilMask;
        /// Stores the currently active texture unit
        unsigned char mActiveTextureUnit;
        /// Mask of buffers who contents can be discarded if GL_EXT_discard_framebuffer is supported
        unsigned int mDiscardBuffers;
        /// Stores the current depth clearing colour
        GLclampf mClearDepth;
        
    public:
        GLESStateCacheManagerImp(void);
        ~GLESStateCacheManagerImp(void);
        
        /// See GLESStateCacheManager.initializeCache.
        void initializeCache();
        
        /// See GLESStateCacheManager.clearCache.
        void clearCache();
		
        /// See GLESStateCacheManager.bindGLBuffer.
        void bindGLBuffer(GLenum target, GLuint buffer, GLenum attach = 0, bool force = false);
        
        /// See GLESStateCacheManager.deleteGLBuffer.
        void deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach = 0, bool force = false);
        
        /// See GLESStateCacheManager.bindGLTexture.
        void bindGLTexture(GLenum target, GLuint texture);
        
        /// See GLESStateCacheManager.setTexParameteri.
        void setTexParameteri(GLenum target, GLenum pname, GLint param);
        
        /// See GLESStateCacheManager.activateGLTextureUnit.
        bool activateGLTextureUnit(unsigned char unit);
        
        /// See GLESStateCacheManager.getBlendEquation.
        GLenum getBlendEquation(void) const { return mBlendEquation; }
        
        /// See GLESStateCacheManager.setBlendEquation.
        void setBlendEquation(GLenum eq);
        
        /// See GLESStateCacheManager.setBlendFunc.
        void setBlendFunc(GLenum source, GLenum dest);
        
        /// See GLESStateCacheManager.getDepthMask.
        GLboolean getDepthMask(void) const { return mDepthMask; }
        
        /// See GLESStateCacheManager.setDepthMask.
        void setDepthMask(GLboolean mask);
        
        /// See GLESStateCacheManager.getDepthFunc.
        GLenum getDepthFunc(void) const { return mDepthFunc; }
        
        /// See GLESStateCacheManager.setDepthFunc.
        void setDepthFunc(GLenum func);
        
        /// See GLESStateCacheManager.getClearDepth.
        GLclampf getClearDepth(void) const { return mClearDepth; }
        
        /// See GLESStateCacheManager.setClearDepth.
        void setClearDepth(GLclampf depth);
        
        /// See GLESStateCacheManager.setClearColour.
        void setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
        
        /// See GLESStateCacheManager.getColourMask.
        vector<GLboolean>::type & getColourMask(void) { return mColourMask; }
        
        /// See GLESStateCacheManager.setColourMask.
        void setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
        
        /// See GLESStateCacheManager.getStencilMask.
        GLuint getStencilMask(void) const { return mStencilMask; }
        
        /// See GLESStateCacheManager.setStencilMask.
        void setStencilMask(GLuint mask);
        
        /// See GLESStateCacheManager.setEnabled.
        void setEnabled(GLenum flag);
        
        /// See GLESStateCacheManager.setDisabled.
        void setDisabled(GLenum flag);
        
        /// See GLESStateCacheManager.getDiscardBuffers.
        unsigned int getDiscardBuffers(void) const { return mDiscardBuffers; }
        
        /// See GLESStateCacheManager.setDiscardBuffers.
        void setDiscardBuffers(unsigned int flags) { mDiscardBuffers = flags; }
        
        /// See GLESStateCacheManager.getPolygonMode.
        GLenum getPolygonMode(void) const { return mPolygonMode; }
        
        /// See GLESStateCacheManager.setPolygonMode.
        void setPolygonMode(GLenum mode) { mPolygonMode = mode; }
        
        /// See GLESStateCacheManager.getCullFace.
        GLenum getCullFace(void) const { return mCullFace; }
        
        /// See GLESStateCacheManager.setCullFace.
        void setCullFace(GLenum face);
    };
}

#endif
