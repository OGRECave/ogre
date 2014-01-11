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
#include "OgreGLStateCacheManager.h"
#include "OgreGLRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

#if OGRE_NO_GL_STATE_CACHE_SUPPORT == 0
#   include "OgreGLStateCacheManagerImp.h"
#else
#   include "OgreGLNullStateCacheManagerImp.h"
#endif

namespace Ogre {
    
    GLStateCacheManager::GLStateCacheManager() 
    {
        mImp = 0;
    }
    
    GLStateCacheManager::~GLStateCacheManager()
    {
        for (CachesMap::iterator it = mCaches.begin(); it != mCaches.end(); ++it)
            OGRE_DELETE it->second;
    }

    void GLStateCacheManager::switchContext(intptr_t id)
    {
        CachesMap::iterator it = mCaches.find(id);
        if (it != mCaches.end())
        {
            // Already have a cache for this context
            mImp = it->second;
        }
        else
        {
            // No cache for this context yet
            mImp = OGRE_NEW GLStateCacheManagerImp();
            mImp->initializeCache();
            mCaches[id] = mImp;
        }
    }

    void GLStateCacheManager::unregisterContext(intptr_t id)
    {
        CachesMap::iterator it = mCaches.find(id);
        if (it != mCaches.end())
        {
            if (mImp == it->second)
                mImp = NULL;
            OGRE_DELETE it->second;
            mCaches.erase(it);
        }

        // Always keep a valid cache, even if no contexts are left.
        // This is needed due to the way GLRenderSystem::shutdown works -
        // HardwareBufferManager destructor may call deleteGLBuffer even after all contexts
        // have been deleted
        if (mImp == NULL)
        {
            // Therefore we add a "dummy" cache if none are left
            if (!mCaches.size())
                mCaches[0] = OGRE_NEW GLStateCacheManagerImp();
            mImp = mCaches.begin()->second;
        }
    }
    
    void GLStateCacheManager::clearCache()
    {
        mImp->clearCache();
    }
    
    void GLStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        mImp->bindGLBuffer(target, buffer, force);
    }
    
    void GLStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        mImp->deleteGLBuffer(target, buffer, force);
    }
    
    void GLStateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        mImp->setTexParameteri(target, pname, param);
    }

    void GLStateCacheManager::invalidateStateForTexture(GLuint texture)
    {
        mImp->invalidateStateForTexture(texture);
    }

    void GLStateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        mImp->bindGLTexture(target, texture);
    }
    
    bool GLStateCacheManager::activateGLTextureUnit(size_t unit)
	{
        return mImp->activateGLTextureUnit(unit);
	}
    
    void GLStateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        mImp->setBlendFunc(source, dest);
    }
    
    void GLStateCacheManager::setBlendEquation(GLenum eq)
    {
        mImp->setBlendEquation(eq);
    }
    
    void GLStateCacheManager::setDepthMask(GLboolean mask)
    {
        mImp->setDepthMask(mask);
    }
    
    void GLStateCacheManager::setDepthFunc(GLenum func)
    {
        mImp->setDepthFunc(func);
    }
    
    void GLStateCacheManager::setClearDepth(GLclampf depth)
    {
        mImp->setClearDepth(depth);
    }
    
    void GLStateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        mImp->setClearColour(red, green, blue, alpha);
    }
    
    void GLStateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        mImp->setColourMask(red, green, blue, alpha);
    }
    
    void GLStateCacheManager::setStencilMask(GLuint mask)
    {
        mImp->setStencilMask(mask);
    }
    
    void GLStateCacheManager::setEnabled(GLenum flag, bool enabled)
    {
        mImp->setEnabled(flag, enabled);
    }

    void GLStateCacheManager::setCullFace(GLenum face)
    {
        mImp->setCullFace(face);
    }
    
    GLenum GLStateCacheManager::getBlendEquation() const
    {
        return mImp->getBlendEquation();
    }
    
    GLboolean GLStateCacheManager::getDepthMask() const
    {
        return mImp->getDepthMask();
    }
    
    GLenum GLStateCacheManager::getDepthFunc() const
    {
        return mImp->getDepthFunc();
    }
    
    GLclampf GLStateCacheManager::getClearDepth() const
    {
        return mImp->getClearDepth();
    }
    
    vector<GLboolean>::type & GLStateCacheManager::getColourMask() const
    {
        return mImp->getColourMask();
    }
    
    GLuint GLStateCacheManager::getStencilMask() const
    {
        return mImp->getStencilMask();
    }
    
    unsigned int GLStateCacheManager::getDiscardBuffers() const
    {
        return mImp->getDiscardBuffers();
    }
    
    void GLStateCacheManager::setDiscardBuffers(unsigned int flags)
    {
        mImp->setDiscardBuffers(flags);
    }
    
    GLenum GLStateCacheManager::getPolygonMode() const
    {
        return mImp->getPolygonMode();
    }
    
    void GLStateCacheManager::setPolygonMode(GLenum mode)
    {
        mImp->setPolygonMode(mode);
    }
    
    GLenum GLStateCacheManager::getCullFace() const
    {
        return mImp->getCullFace();
    }

    void GLStateCacheManager::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        mImp->setViewport(x, y, width, height);
    }

    void GLStateCacheManager::getViewport(int *array)
    {
        mImp->getViewport(array);
    }

    void GLStateCacheManager::setPointSize(GLfloat size)
    {
        mImp->setPointSize(size);
    }

    void GLStateCacheManager::setLightAmbient(GLfloat r, GLfloat g, GLfloat b)
    {
        mImp->setLightAmbient(r, g, b);
    }

    void GLStateCacheManager::setShadeModel(GLenum model)
    {
        mImp->setShadeModel(model);
    }

    void GLStateCacheManager::enableTextureCoordGen(GLenum type)
    {
        mImp->enableTextureCoordGen(type);
    }

    void GLStateCacheManager::disableTextureCoordGen(GLenum type)
    {
        mImp->disableTextureCoordGen(type);
    }

    void GLStateCacheManager::setMaterialShininess(GLfloat shininess)
    {
        mImp->setMaterialShininess(shininess);
    }

    void GLStateCacheManager::setMaterialSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        mImp->setMaterialSpecular(r, g, b, a);
    }

    void GLStateCacheManager::setMaterialEmissive(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        mImp->setMaterialEmissive(r, g, b, a);
    }

    void GLStateCacheManager::setMaterialAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        mImp->setMaterialAmbient(r, g, b, a);
    }

    void GLStateCacheManager::setMaterialDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        mImp->setMaterialDiffuse(r, g, b, a);
    }

    void GLStateCacheManager::setBlendEquation(GLenum eqRGB, GLenum eqAlpha)
    {
        mImp->setBlendEquation(eqRGB, eqAlpha);
    }

    void GLStateCacheManager::setPointParameters(GLfloat *attenuation, float minSize, float maxSize)
    {
        mImp->setPointParameters(attenuation, minSize, maxSize);
    }
}
