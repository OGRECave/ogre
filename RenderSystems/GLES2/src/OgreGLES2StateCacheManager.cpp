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
#include "OgreGLES2StateCacheManager.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

#if OGRE_NO_GL_STATE_CACHE_SUPPORT == 0
#   include "OgreGLES2StateCacheManagerImp.h"
#else
#   include "OgreGLES2NullStateCacheManagerImp.h"
#endif

namespace Ogre {
    
    GLES2StateCacheManager::GLES2StateCacheManager() 
    {
        mImp = new GLES2StateCacheManagerImp();
    }
    
    GLES2StateCacheManager::~GLES2StateCacheManager()
    {
        delete mImp;
        mImp = 0;
    }

    void GLES2StateCacheManager::initializeCache()
    {
        mImp->initializeCache();
    }
    
    void GLES2StateCacheManager::clearCache()
    {
        mImp->clearCache();
    }
    
    void GLES2StateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        mImp->bindGLBuffer(target, buffer, force);
    }
    
    void GLES2StateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        mImp->deleteGLBuffer(target, buffer, force);
    }

    void GLES2StateCacheManager::invalidateStateForTexture(GLuint texture)
    {
        mImp->invalidateStateForTexture(texture);
    }

    void GLES2StateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        mImp->setTexParameteri(target, pname, param);
    }

    void GLES2StateCacheManager::setTexParameterf(GLenum target, GLenum pname, GLfloat param)
    {
        mImp->setTexParameterf(target, pname, param);
    }

    void GLES2StateCacheManager::getTexParameterfv(GLenum target, GLenum pname, GLfloat *param)
    {
        mImp->getTexParameterfv(target, pname, param);
    }

    void GLES2StateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        mImp->bindGLTexture(target, texture);
    }
    
    bool GLES2StateCacheManager::activateGLTextureUnit(size_t unit)
    {
        return mImp->activateGLTextureUnit(unit);
    }
    
    void GLES2StateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        mImp->setBlendFunc(source, dest);
    }
    
    void GLES2StateCacheManager::setBlendEquation(GLenum eq)
    {
        mImp->setBlendEquation(eq);
    }
    
    void GLES2StateCacheManager::setDepthMask(GLboolean mask)
    {
        mImp->setDepthMask(mask);
    }
    
    void GLES2StateCacheManager::setDepthFunc(GLenum func)
    {
        mImp->setDepthFunc(func);
    }
    
    void GLES2StateCacheManager::setClearDepth(GLclampf depth)
    {
        mImp->setClearDepth(depth);
    }
    
    void GLES2StateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        mImp->setClearColour(red, green, blue, alpha);
    }
    
    void GLES2StateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        mImp->setColourMask(red, green, blue, alpha);
    }
    
    void GLES2StateCacheManager::setStencilMask(GLuint mask)
    {
        mImp->setStencilMask(mask);
    }
    
    void GLES2StateCacheManager::setEnabled(GLenum flag)
    {
        mImp->setEnabled(flag);
    }
    
    void GLES2StateCacheManager::setDisabled(GLenum flag)
    {
        mImp->setDisabled(flag);
    }

    void GLES2StateCacheManager::setVertexAttribEnabled(GLuint attrib)
    {
        mImp->setVertexAttribEnabled(attrib);
    }

    void GLES2StateCacheManager::setVertexAttribDisabled(GLuint attrib)
    {
        mImp->setVertexAttribDisabled(attrib);
    }
    
    GLenum GLES2StateCacheManager::getBlendEquation() const
    {
        return mImp->getBlendEquation();
    }
    
    GLboolean GLES2StateCacheManager::getDepthMask() const
    {
        return mImp->getDepthMask();
    }
    
    GLenum GLES2StateCacheManager::getDepthFunc() const
    {
        return mImp->getDepthFunc();
    }
    
    GLclampf GLES2StateCacheManager::getClearDepth() const
    {
        return mImp->getClearDepth();
    }
    
    vector<GLboolean>::type & GLES2StateCacheManager::getColourMask() const
    {
        return mImp->getColourMask();
    }
    
    GLuint GLES2StateCacheManager::getStencilMask() const
    {
        return mImp->getStencilMask();
    }
    
    unsigned int GLES2StateCacheManager::getDiscardBuffers() const
    {
        return mImp->getDiscardBuffers();
    }
    
    void GLES2StateCacheManager::setDiscardBuffers(unsigned int flags)
    {
        mImp->setDiscardBuffers(flags);
    }
}
