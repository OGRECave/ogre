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
#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GL3PlusStateCacheManager::GL3PlusStateCacheManager(void)
    {
        clearCache();
    }

    GL3PlusStateCacheManager::~GL3PlusStateCacheManager(void)
    {
    }

    void GL3PlusStateCacheManager::initializeCache()
    {
        glBlendEquation(GL_FUNC_ADD);

        glBlendFunc(GL_ONE, GL_ZERO);

        glCullFace(mCullFace);

        glDepthFunc(mDepthFunc);

        glDepthMask(mDepthMask);

        glStencilMask(mStencilMask);

        glClearDepth(mClearDepth);

        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glActiveTexture(GL_TEXTURE0);

        glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]);

        glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]);

        glPolygonMode(GL_FRONT_AND_BACK, mPolygonMode);
    }
    
    void GL3PlusStateCacheManager::clearCache()
    {
        mDepthMask = GL_TRUE;
        mPolygonMode = GL_FILL;
        mBlendEquation = GL_FUNC_ADD;
        mCullFace = GL_BACK;
        mDepthFunc = GL_LESS;
        mStencilMask = 0xFFFFFFFF;
        mActiveTextureUnit = 0;
        mDiscardBuffers = 0;
        mClearDepth = 1.0f;

        mBlendFuncSource = GL_ONE;
        mBlendFuncDest = GL_ZERO;

        mClearColour.resize(4);
        mClearColour[0] = mClearColour[1] = mClearColour[2] = mClearColour[3] = 0.0f;
        
        mColourMask.resize(4);
        mColourMask[0] = mColourMask[1] = mColourMask[2] = mColourMask[3] = GL_TRUE;

        mViewport[0] = 0;
        mViewport[1] = 0;
        mViewport[2] = 0;
        mViewport[3] = 0;

        mPolygonMode = GL_FILL;

        mPointSize = 1;
        mPointSizeMin = 1;
        mPointSizeMax = 1;
        mPointAttenuation[0] = 1;
        mPointAttenuation[1] = 0;
        mPointAttenuation[2] = 0;
    }
    void GL3PlusStateCacheManager::bindGLFrameBuffer(GLenum target, GLuint buffer, bool force)
    {
        // Update GL
        glBindFramebuffer(target, buffer);
    }
    void GL3PlusStateCacheManager::bindGLRenderBuffer(GLuint buffer, bool force)
    {
        // Update GL
        glBindRenderbuffer(GL_RENDERBUFFER, buffer);
    }
    void GL3PlusStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        // Update GL
        glBindBuffer(target, buffer);
    }
    void GL3PlusStateCacheManager::deleteGLFrameBuffer(GLenum target, GLuint buffer)
    {
         // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;

        glDeleteFramebuffers(1, &buffer);
    }
    void GL3PlusStateCacheManager::deleteGLRenderBuffer(GLuint buffer)
    {
         // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;

        glDeleteRenderbuffers(1, &buffer);
    }
    void GL3PlusStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;

        glDeleteBuffers(1, &buffer);
    }
    
    void GL3PlusStateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        // Update GL
        glTexParameteri(target, pname, param);
    }
    
    void GL3PlusStateCacheManager::invalidateStateForTexture(GLuint texture) { }

    void GL3PlusStateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        // Update GL
        glBindTexture(target, texture);
    }
    
    bool GL3PlusStateCacheManager::activateGLTextureUnit(size_t unit)
    {
        // Always return true for the currently bound texture unit
        if (mActiveTextureUnit == unit)
            return true;
        
        if (unit < dynamic_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities()->getNumTextureUnits())
        {
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + unit));
            
            mActiveTextureUnit = unit;
            
            return true;
        }
        else
        {
            return false;
        }
    }
    
    void GL3PlusStateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        glBlendFunc(source, dest);
    }
    
    void GL3PlusStateCacheManager::setBlendEquation(GLenum eq)
    {
        mBlendEquation = eq;
        glBlendEquation(eq);
    }
    
    void GL3PlusStateCacheManager::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            glDepthMask(mask);
        }
    }
    
    void GL3PlusStateCacheManager::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            glDepthFunc(func);
        }
    }
    
    void GL3PlusStateCacheManager::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            glClearDepth(depth);
        }
    }
    
    void GL3PlusStateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
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
        }
    }
    
    void GL3PlusStateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
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
        }
    }
    
    void GL3PlusStateCacheManager::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            glStencilMask(mask);
        }
    }
    
    void GL3PlusStateCacheManager::setEnabled(GLenum flag, bool enabled)
    {
        if(enabled)
        {
            glEnable(flag);
        }
        else
        {
            glDisable(flag);
        }
    }

    void GL3PlusStateCacheManager::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            glCullFace(face);
        }
    }

    void GL3PlusStateCacheManager::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        mViewport[0] = x;
        mViewport[1] = y;
        mViewport[2] = width;
        mViewport[3] = height;
        glViewport(x, y, width, height);
    }

    void GL3PlusStateCacheManager::getViewport(int *array)
    {
        for (int i = 0; i < 4; ++i)
            array[i] = mViewport[i];
    }

    void GL3PlusStateCacheManager::setBlendEquation(GLenum eqRGB, GLenum eqAlpha)
    {
        if(mBlendEquationRGB != eqRGB || mBlendEquationAlpha != eqAlpha)
        {
            mBlendEquationRGB = eqRGB;
            mBlendEquationAlpha = eqAlpha;

            glBlendEquationSeparate(eqRGB, eqAlpha);

        }
    }

    void GL3PlusStateCacheManager::setPointSize(GLfloat size)
    {
        if (mPointSize != size)
        {
            mPointSize = size;
            glPointSize(mPointSize);
        }
    }

    void GL3PlusStateCacheManager::enableTextureCoordGen(GLenum type)
    {
        glEnable(type);
    }

    void GL3PlusStateCacheManager::disableTextureCoordGen(GLenum type)
    {
        glDisable(type);
    }

    void GL3PlusStateCacheManager::setPolygonMode(GLenum mode)
    {
        mPolygonMode = mode;
        glPolygonMode(GL_FRONT_AND_BACK, mPolygonMode);
    }
}
