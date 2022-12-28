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

#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGL3PlusRenderSystem.h"
#include "OgreRoot.h"

// performance critical state
// this one is per-context and can be unconditionally cached
// without breaking multi-context scenarios
#define OGRE_ENABLE_STATE_CACHE_CRITICAL

namespace Ogre {
    
    GL3PlusStateCacheManager::GL3PlusStateCacheManager(void)
    {
        clearCache();
    }
    
    void GL3PlusStateCacheManager::initializeCache()
    {
        glBlendEquation(GL_FUNC_ADD);

        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

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
        mBlendEquationRGB = GL_FUNC_ADD;
        mBlendEquationAlpha = GL_FUNC_ADD;
        mCullFace = GL_BACK;
        mDepthFunc = GL_LESS;
        mStencilMask = 0xFFFFFFFF;
        mActiveTextureUnit = 0;
        mClearDepth = 1.0f;
        mLastBoundTexID = 0;

        mPolygonMode = GL_FILL;

        // Initialize our cache variables and also the GL so that the
        // stored values match the GL state
        mBlendFuncSource = GL_ONE;
        mBlendFuncDest = GL_ZERO;
        
        mClearColour[0] = mClearColour[1] = mClearColour[2] = mClearColour[3] = 0.0f;
        mColourMask[0] = mColourMask[1] = mColourMask[2] = mColourMask[3] = GL_TRUE;

#ifdef OGRE_ENABLE_STATE_CACHE
        mEnableVector.reserve(25);
        mEnableVector.clear();
#endif
        mActiveBufferMap.clear();
        mTexUnitsMap.clear();

        mActiveDrawFrameBuffer=0;
        mActiveReadFrameBuffer=0;

        mActiveVertexArray = 0;
    }

    void GL3PlusStateCacheManager::bindGLFrameBuffer(GLenum target,GLuint buffer)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        bool update = false;
       
        //GL_FRAMEBUFFER sets both GL_DRAW_FRAMEBUFFER and GL_READ_FRAMEBUFFER
        if(target==GL_FRAMEBUFFER)
        {
            if( buffer != mActiveDrawFrameBuffer
             || buffer != mActiveReadFrameBuffer)
            {
                update = true;
                mActiveReadFrameBuffer=buffer;
                mActiveDrawFrameBuffer=buffer;
            }
        }
        else if( target == GL_DRAW_FRAMEBUFFER)
        {
            if(buffer != mActiveDrawFrameBuffer)
            {
                update = true;
                mActiveDrawFrameBuffer=buffer;
            }
        }
        else if( target == GL_READ_FRAMEBUFFER)
        {
            if(buffer != mActiveReadFrameBuffer)
            {
                update = true;
                mActiveReadFrameBuffer=buffer;
            }
        }

        // Update GL
        if(update)
#endif
        {
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(target, buffer));
        }
    }
    void GL3PlusStateCacheManager::bindGLRenderBuffer(GLuint buffer)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        auto ret = mActiveBufferMap.emplace(GL_RENDERBUFFER, buffer);
        if(ret.first->second != buffer) // Update the cached value if needed
        {
            ret.first->second = buffer;
            ret.second = true;
        }

        // Update GL
        if(ret.second)
#endif
        {
            OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, buffer));
        }
    }
    
    void GL3PlusStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        auto ret = mActiveBufferMap.emplace(target, buffer);
        if(ret.first->second != buffer) // Update the cached value if needed
        {
            ret.first->second = buffer;
            ret.second = true;
        }

        // Update GL
        if(ret.second)
#endif
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer(target, buffer));
        }

    }
    void GL3PlusStateCacheManager::deleteGLFrameBuffer(GLenum target, GLuint buffer)
    {   
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        //always delete the buffer, even if not currently bound
        OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &buffer));

#ifdef OGRE_ENABLE_STATE_CACHE
        if (buffer == mActiveDrawFrameBuffer )
        {
            // Currently bound read frame buffer is being deleted, update the cached values to 0,
            mActiveDrawFrameBuffer = 0;
        }
        if ( buffer == mActiveReadFrameBuffer)
        {
            // Currently bound read frame buffer is being deleted, update the cached values to 0,
            mActiveReadFrameBuffer = 0;
        }
#endif
    }
    void GL3PlusStateCacheManager::deleteGLRenderBuffer(GLuint buffer)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;

        //always delete the buffer, even if not currently bound
        OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &buffer));
        
#ifdef OGRE_ENABLE_STATE_CACHE
        BindBufferMap::iterator i = mActiveBufferMap.find(GL_RENDERBUFFER);
        if (i != mActiveBufferMap.end() && ((*i).second == buffer))
        {
            // Currently bound render buffer is being deleted, update the cached value to 0,
            // which it likely the buffer that will be bound by the driver.
            // An update will be forced next time we try to bind on this target.
            (*i).second = 0;
        }
#endif
    }
    void GL3PlusStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        //always delete the buffer, even if not currently bound
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &buffer));

#ifdef OGRE_ENABLE_STATE_CACHE
        BindBufferMap::iterator i = mActiveBufferMap.find(target);
        
        if (i != mActiveBufferMap.end() && ((*i).second == buffer))
        {
            // Currently bound buffer is being deleted, update the cached value to 0,
            // which it likely the buffer that will be bound by the driver.
            // An update will be forced next time we try to bind on this target.
            (*i).second = 0;
        }
#endif
    }

    void GL3PlusStateCacheManager::bindGLVertexArray(GLuint vao)
    {
#ifdef OGRE_ENABLE_STATE_CACHE_CRITICAL // multi-context is handled by GLVertexArrayObject::bind
        if(mActiveVertexArray != vao)
        {
            mActiveVertexArray = vao;
            OGRE_CHECK_GL_ERROR(glBindVertexArray(vao));
            //we also need to clear the cached GL_ELEMENT_ARRAY_BUFFER value, as it is invalidated by glBindVertexArray
            mActiveBufferMap[GL_ELEMENT_ARRAY_BUFFER] = 0;
        }
#else
        OGRE_CHECK_GL_ERROR(glBindVertexArray(vao));
#endif
    }

    void GL3PlusStateCacheManager::deleteGLVertexArray(GLuint vao)
    {
#ifdef OGRE_ENABLE_STATE_CACHE_CRITICAL
        if(mActiveVertexArray == vao)
        {
            mActiveVertexArray = 0;
            //we also need to clear the cached GL_ELEMENT_ARRAY_BUFFER value, as it is invalidated by glBindVertexArray
            mActiveBufferMap[GL_ELEMENT_ARRAY_BUFFER] = 0;
        }
#else
        OGRE_CHECK_GL_ERROR(glDeleteVertexArrays(1, &vao));
#endif
    }

    void GL3PlusStateCacheManager::invalidateStateForTexture(GLuint texture)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        mTexUnitsMap.erase(texture);
#endif
    }

    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GL3PlusStateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        // Check if we have a map entry for this texture id. If not, create a blank one and insert it.
        TexUnitsMap::iterator it = mTexUnitsMap.find(mLastBoundTexID);
        if (it == mTexUnitsMap.end())
        {
            TexParameteriMap unit;
            mTexUnitsMap[mLastBoundTexID] = unit;
            
            // Update the iterator
            it = mTexUnitsMap.find(mLastBoundTexID);
        }
        
        // Get a local copy of the parameter map and search for this parameter
        TexParameteriMap &myMap = (*it).second;
        auto ret = myMap.emplace(pname, param);
        TexParameteriMap::iterator i = ret.first;

        // Update the cached value if needed
        if((*i).second != param || ret.second)
        {
            (*i).second = param;

            // Update GL
            OGRE_CHECK_GL_ERROR(glTexParameteri(target, pname, param));
        }
#else
        OGRE_CHECK_GL_ERROR(glTexParameteri(target, pname, param));
#endif
    }
    
    void GL3PlusStateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        mLastBoundTexID = texture;
#endif
        
        // Update GL
        OGRE_CHECK_GL_ERROR(glBindTexture(target, texture));
    }
    
    bool GL3PlusStateCacheManager::activateGLTextureUnit(size_t unit)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if (mActiveTextureUnit == unit)
            return true;
#endif

        OGRE_CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0 + unit));
        mActiveTextureUnit = unit;
        return true;
    }

    void GL3PlusStateCacheManager::setBlendFunc(GLenum source, GLenum dest, GLenum sourceA, GLenum destA)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mBlendFuncSource != source || mBlendFuncDest != dest || sourceA != mBlendFuncSourceAlpha || destA != mBlendFuncDestAlpha )
#endif
        {
            mBlendFuncSource = source;
            mBlendFuncDest = dest;
            mBlendFuncSourceAlpha = sourceA;
            mBlendFuncDestAlpha = destA;
            
            OGRE_CHECK_GL_ERROR(glBlendFuncSeparate(source, dest, sourceA, destA));
        }
    }

    void GL3PlusStateCacheManager::setDepthMask(GLboolean mask)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mDepthMask != mask)
#endif
        {
            mDepthMask = mask;
            
            OGRE_CHECK_GL_ERROR(glDepthMask(mask));
        }
    }
    
    void GL3PlusStateCacheManager::setDepthFunc(GLenum func)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mDepthFunc != func)
#endif
        {
            mDepthFunc = func;
            
            OGRE_CHECK_GL_ERROR(glDepthFunc(func));
        }
    }
    
    void GL3PlusStateCacheManager::setClearDepth(GLclampf depth)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mClearDepth != depth)
#endif
        {
            mClearDepth = depth;
            
            OGRE_CHECK_GL_ERROR(glClearDepth(depth));
        }
    }
    
    void GL3PlusStateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if((mClearColour[0] != red) ||
           (mClearColour[1] != green) ||
           (mClearColour[2] != blue) ||
           (mClearColour[3] != alpha))
#endif
        {
            mClearColour[0] = red;
            mClearColour[1] = green;
            mClearColour[2] = blue;
            mClearColour[3] = alpha;
            
            OGRE_CHECK_GL_ERROR(glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]));
        }
    }
    
    void GL3PlusStateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if((mColourMask[0] != red) ||
           (mColourMask[1] != green) ||
           (mColourMask[2] != blue) ||
           (mColourMask[3] != alpha))
#endif
        {
            mColourMask[0] = red;
            mColourMask[1] = green;
            mColourMask[2] = blue;
            mColourMask[3] = alpha;
            
            OGRE_CHECK_GL_ERROR(glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]));
        }
    }
    
    void GL3PlusStateCacheManager::setStencilMask(GLuint mask)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mStencilMask != mask)
#endif
        {
            mStencilMask = mask;
            
            OGRE_CHECK_GL_ERROR(glStencilMask(mask));
        }
    }
    
    void GL3PlusStateCacheManager::setEnabled(GLenum flag, bool enabled)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        auto iter = std::find(mEnableVector.begin(), mEnableVector.end(), flag);
        bool was_enabled = iter != mEnableVector.end();

        if(was_enabled == enabled)
            return; // no change

        if(!enabled)
        {
            mEnableVector.erase(iter);
        }
        else
        {
            mEnableVector.push_back(flag);
        }
#endif
        if (!enabled)
        {
            OGRE_CHECK_GL_ERROR(glDisable(flag));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glEnable(flag));
        }
    }

    void GL3PlusStateCacheManager::setViewport(const Rect& r)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mViewport != r)
#endif
        {
            mViewport = r;
            OGRE_CHECK_GL_ERROR(glViewport(r.left, r.top, r.width(), r.height()));
        }
    }

    void GL3PlusStateCacheManager::setCullFace(GLenum face)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mCullFace != face)
#endif
        {
            mCullFace = face;
            
            glCullFace(face);
        }
    }

    void GL3PlusStateCacheManager::setBlendEquation(GLenum eqRGB, GLenum eqAlpha)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mBlendEquationRGB != eqRGB || mBlendEquationAlpha != eqAlpha)
#endif
        {
            mBlendEquationRGB = eqRGB;
            mBlendEquationAlpha = eqAlpha;

            OGRE_CHECK_GL_ERROR(glBlendEquationSeparate(eqRGB, eqAlpha));
        }
    }

    void GL3PlusStateCacheManager::setPolygonMode(GLenum mode)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if (mPolygonMode != mode)
#endif
        {
            mPolygonMode = mode;
            OGRE_CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK, mPolygonMode));
        }
    }

    void GL3PlusStateCacheManager::bindGLProgramPipeline(GLuint handle)
    {
#ifdef OGRE_ENABLE_STATE_CACHE
        if(mActiveProgramPipeline != handle)
#endif
        {
            mActiveProgramPipeline = handle;
            OGRE_CHECK_GL_ERROR(glBindProgramPipeline(mActiveProgramPipeline));
        }
    }
}
