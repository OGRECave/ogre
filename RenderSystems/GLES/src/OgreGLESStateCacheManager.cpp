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
#include "OgreGLESStateCacheManager.h"
#include "OgreGLESRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

#if OGRE_NO_GL_STATE_CACHE_SUPPORT == 0
#   include "OgreGLESStateCacheManagerImp.h"
#else
#   include "OgreGLESNullStateCacheManagerImp.h"
#endif

namespace Ogre {
    
    GLESStateCacheManager::GLESStateCacheManager() 
    {
        mImp = new GLESStateCacheManagerImp();
    }
    
    GLESStateCacheManager::~GLESStateCacheManager()
    {
        delete mImp;
    }

    void GLESStateCacheManager::initializeCache()
    {
        mImp->initializeCache();
    }
    
    void GLESStateCacheManager::clearCache()
    {
        mImp->clearCache();
    }
    
    void GLESStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        mImp->bindGLBuffer(target, buffer, attach, force);
    }
    
    void GLESStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        mImp->deleteGLBuffer(target, buffer, attach, force);
    }
    
    void GLESStateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        mImp->setTexParameteri(target, pname, param);
    }
    
    void GLESStateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        mImp->bindGLTexture(target, texture);
    }
    
    bool GLESStateCacheManager::activateGLTextureUnit(unsigned char unit)
	{
        return mImp->activateGLTextureUnit(unit);
	}
    
    void GLESStateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        mImp->setBlendFunc(source, dest);
    }
    
    void GLESStateCacheManager::setBlendEquation(GLenum eq)
    {
        mImp->setBlendEquation(eq);
    }
    
    void GLESStateCacheManager::setDepthMask(GLboolean mask)
    {
        mImp->setDepthMask(mask);
    }
    
    void GLESStateCacheManager::setDepthFunc(GLenum func)
    {
        mImp->setDepthFunc(func);
    }
    
    void GLESStateCacheManager::setClearDepth(GLclampf depth)
    {
        mImp->setClearDepth(depth);
    }
    
    void GLESStateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        mImp->setClearColour(red, green, blue, alpha);
    }
    
    void GLESStateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        mImp->setColourMask(red, green, blue, alpha);
    }
    
    void GLESStateCacheManager::setStencilMask(GLuint mask)
    {
        mImp->setStencilMask(mask);
    }
    
    void GLESStateCacheManager::setEnabled(GLenum flag)
    {
        mImp->setEnabled(flag);
    }
    
    void GLESStateCacheManager::setDisabled(GLenum flag)
    {
        mImp->setDisabled(flag);
    }
    
    void GLESStateCacheManager::setCullFace(GLenum face)
    {
        mImp->setCullFace(face);
    }
    
    GLenum GLESStateCacheManager::getBlendEquation() const
    {
        return mImp->getBlendEquation();
    }
    
    GLboolean GLESStateCacheManager::getDepthMask() const
    {
        return mImp->getDepthMask();
    }
    
    GLenum GLESStateCacheManager::getDepthFunc() const
    {
        return mImp->getDepthFunc();
    }
    
    GLclampf GLESStateCacheManager::getClearDepth() const
    {
        return mImp->getClearDepth();
    }
    
    vector<GLboolean>::type & GLESStateCacheManager::getColourMask() const
    {
        return mImp->getColourMask();
    }
    
    GLuint GLESStateCacheManager::getStencilMask() const
    {
        return mImp->getStencilMask();
    }
    
    unsigned int GLESStateCacheManager::getDiscardBuffers() const
    {
        return mImp->getDiscardBuffers();
    }
    
    void GLESStateCacheManager::setDiscardBuffers(unsigned int flags)
    {
        mImp->setDiscardBuffers(flags);
    }
    
    GLenum GLESStateCacheManager::getPolygonMode() const
    {
        return mImp->getPolygonMode();
    }
    
    void GLESStateCacheManager::setPolygonMode(GLenum mode)
    {
        mImp->setPolygonMode(mode);
    }
    
    GLenum GLESStateCacheManager::getCullFace() const
    {
        return mImp->getCullFace();
    }
}
