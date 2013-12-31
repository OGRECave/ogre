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

#include "OgreStableHeaders.h"
#include "OgreGLESNullStateCacheManagerImp.h"
#include "OgreGLESRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GLESStateCacheManagerImp::GLESStateCacheManagerImp(void) 
    {
        clearCache();
    }
    
    GLESStateCacheManagerImp::~GLESStateCacheManagerImp(void)
    {
    }
    
    void GLESStateCacheManagerImp::initializeCache()
    {
        glBlendEquationOES(GL_FUNC_ADD_OES);
        GL_CHECK_ERROR
        
        glBlendFunc(GL_ONE, GL_ZERO);
        GL_CHECK_ERROR
        
        glCullFace(mCullFace);
        GL_CHECK_ERROR
        
        glDepthFunc(mDepthFunc);
        GL_CHECK_ERROR
        
        glDepthMask(mDepthMask);
        GL_CHECK_ERROR
        
        glStencilMask(mStencilMask);
        GL_CHECK_ERROR
        
        glClearDepthf(mClearDepth);
        GL_CHECK_ERROR
        
        glBindTexture(GL_TEXTURE_2D, 0);
        GL_CHECK_ERROR
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        GL_CHECK_ERROR
        
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
        GL_CHECK_ERROR
        
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
        GL_CHECK_ERROR
        
        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR
        
        glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]);
        GL_CHECK_ERROR
        
        glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]);
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::clearCache()
    {
        mDepthMask = GL_TRUE;
        mPolygonMode = GL_FILL;
        mBlendEquation = GL_FUNC_ADD_OES;
        mCullFace = GL_BACK;
        mDepthFunc = GL_LESS;
        mStencilMask = 0xFFFFFFFF;
        mActiveTextureUnit = 0;
        mDiscardBuffers = 0;
        mClearDepth = 1.0f;
        
        mClearColour.resize(4);
        mClearColour[0] = mClearColour[1] = mClearColour[2] = mClearColour[3] = 0.0f;
        
        mColourMask.resize(4);
        mColourMask[0] = mColourMask[1] = mColourMask[2] = mColourMask[3] = GL_TRUE;
    }
    
    void GLESStateCacheManagerImp::bindGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        // Update GL
        if(target == GL_FRAMEBUFFER_OES)
            glBindFramebufferOES(target, buffer);
        else if(target == GL_RENDERBUFFER_OES)
            glBindRenderbufferOES(target, buffer);
        else
            glBindBuffer(target, buffer);
        
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        if(target == GL_FRAMEBUFFER_OES)
            glDeleteFramebuffersOES(1, &buffer);
        else if(target == GL_RENDERBUFFER_OES)
            glDeleteRenderbuffersOES(1, &buffer);
        else
            glDeleteBuffers(1, &buffer);
        
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        // Update GL
        glTexParameteri(target, pname, param);
        GL_CHECK_ERROR
    }
    

    void GLESStateCacheManagerImp::bindGLTexture(GLenum target, GLuint texture)
    {
        // Update GL
        glBindTexture(target, texture);
        GL_CHECK_ERROR
    }
    
    bool GLESStateCacheManagerImp::activateGLTextureUnit(unsigned char unit)
	{
        // Always return true for the currently bound texture unit
        if (mActiveTextureUnit == unit)
            return true;
        
        if (unit < dynamic_cast<GLESRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities()->getNumTextureUnits())
        {
            glActiveTexture(GL_TEXTURE0 + unit);
            GL_CHECK_ERROR
            
            mActiveTextureUnit = unit;
            
            return true;
        }
        else
        {
            return false;
        }
	}
    
    void GLESStateCacheManagerImp::setBlendFunc(GLenum source, GLenum dest)
    {
        glBlendFunc(source, dest);
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::setBlendEquation(GLenum eq)
    {
        mBlendEquation = eq;
        glBlendEquationOES(eq);
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            glDepthMask(mask);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            glDepthFunc(func);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            glClearDepthf(depth);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        if((mClearColour[0] != red) ||
           (mClearColour[1] != green) ||
           (mClearColour[2] != blue) ||
           (mClearColour[3] != alpha))
        {
            mClearColour[0] = red;
            mClearColour[1] = green;
            mClearColour[2] = blue;
            mClearColour[3] = alpha;
            
            glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        if((mColourMask[0] != red) ||
           (mColourMask[1] != green) ||
           (mColourMask[2] != blue) ||
           (mColourMask[3] != alpha))
        {
            mColourMask[0] = red;
            mColourMask[1] = green;
            mColourMask[2] = blue;
            mColourMask[3] = alpha;
            
            glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            glStencilMask(mask);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setEnabled(GLenum flag)
    {
        glEnable(flag);
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::setDisabled(GLenum flag)
    {
        glDisable(flag);
        GL_CHECK_ERROR
    }
    
    void GLESStateCacheManagerImp::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            glCullFace(face);
            GL_CHECK_ERROR
        }
    }
}
