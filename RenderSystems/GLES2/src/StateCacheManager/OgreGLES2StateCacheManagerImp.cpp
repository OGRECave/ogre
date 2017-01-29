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

namespace Ogre {
    
    GLES2StateCacheManager::GLES2StateCacheManager(void)
    {
        clearCache();
    }
    
    void GLES2StateCacheManager::initializeCache()
    {
        OGRE_CHECK_GL_ERROR(glBlendEquation(GL_FUNC_ADD));
        
        OGRE_CHECK_GL_ERROR(glBlendFunc(GL_ONE, GL_ZERO));
        
        OGRE_CHECK_GL_ERROR(glCullFace(mCullFace));

        OGRE_CHECK_GL_ERROR(glDepthFunc(mDepthFunc));

        OGRE_CHECK_GL_ERROR(glDepthMask(mDepthMask));

        OGRE_CHECK_GL_ERROR(glStencilMask(mStencilMask));

        OGRE_CHECK_GL_ERROR(glClearDepthf(mClearDepth));

        OGRE_CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, 0));

        OGRE_CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));

        OGRE_CHECK_GL_ERROR(glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]));

        OGRE_CHECK_GL_ERROR(glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]));
    }
    
    void GLES2StateCacheManager::clearCache()
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
        mLastBoundTexID = 0;
        
        // Initialize our cache variables and also the GL so that the
        // stored values match the GL state
        mBlendFuncSource = GL_ONE;
        mBlendFuncDest = GL_ZERO;
        
        mClearColour.resize(4);
        mClearColour[0] = mClearColour[1] = mClearColour[2] = mClearColour[3] = 0.0f;
        
        mColourMask.resize(4);
        mColourMask[0] = mColourMask[1] = mColourMask[2] = mColourMask[3] = GL_TRUE;
        
        mEnableVector.reserve(25);
        mEnableVector.clear();
        mActiveBufferMap.clear();
        mTexUnitsMap.clear();

        mEnabledVertexAttribs.reserve(64);
        mEnabledVertexAttribs.clear();
    }
    
    GLES2StateCacheManager::~GLES2StateCacheManager(void)
    {
        mColourMask.clear();
        mClearColour.clear();
        mActiveBufferMap.clear();
        mEnableVector.clear();
        mTexUnitsMap.clear();
    }
    
    void GLES2StateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        bool update = false;
        BindBufferMap::iterator i = mActiveBufferMap.find(target);
        if (i == mActiveBufferMap.end())
        {
            // Haven't cached this state yet.  Insert it into the map
            mActiveBufferMap.insert(BindBufferMap::value_type(target, buffer));
            update = true;
        }
        else if((*i).second != buffer || force) // Update the cached value if needed
        {
            (*i).second = buffer;
            update = true;
        }

        // Update GL
        if(update)
        {
            if(target == GL_FRAMEBUFFER)
            {
                OGRE_CHECK_GL_ERROR(glBindFramebuffer(target, buffer));
            }
            else if(target == GL_RENDERBUFFER)
            {
                OGRE_CHECK_GL_ERROR(glBindRenderbuffer(target, buffer));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glBindBuffer(target, buffer));
            }
        }
    }
    
    void GLES2StateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        BindBufferMap::iterator i = mActiveBufferMap.find(target);
        
        if (i != mActiveBufferMap.end() && ((*i).second == buffer || force))
        {
            if(target == GL_FRAMEBUFFER)
            {
                OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &buffer));
            }
            else if(target == GL_RENDERBUFFER)
            {
                OGRE_CHECK_GL_ERROR(glDeleteRenderbuffers(1, &buffer));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &buffer));
            }

            // Currently bound buffer is being deleted, update the cached value to 0,
            // which it likely the buffer that will be bound by the driver.
            // An update will be forced next time we try to bind on this target.
            (*i).second = 0;
        }
    }

    void GLES2StateCacheManager::invalidateStateForTexture(GLuint texture)
    {
        mTexUnitsMap.erase(texture);
    }

    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GLES2StateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        // Check if we have a map entry for this texture id. If not, create a blank one and insert it.
        TexUnitsMap::iterator it = mTexUnitsMap.find(mLastBoundTexID);
        if (it == mTexUnitsMap.end())
        {
            TextureUnitParams unit;
            mTexUnitsMap[mLastBoundTexID] = unit;
            
            // Update the iterator
            it = mTexUnitsMap.find(mLastBoundTexID);
        }
        
        // Get a local copy of the parameter map and search for this parameter
        TexParameteriMap &myMap = (*it).second.mTexParameteriMap;
        TexParameteriMap::iterator i = myMap.find(pname);
        
        if (i == myMap.end())
        {
            // Haven't cached this state yet.  Insert it into the map
            myMap.insert(TexParameteriMap::value_type(pname, param));
            
            // Update GL
            OGRE_CHECK_GL_ERROR(glTexParameteri(target, pname, param));
        }
        else
        {
            // Update the cached value if needed
            if((*i).second != param)
            {
                (*i).second = param;
                
                // Update GL
                OGRE_CHECK_GL_ERROR(glTexParameteri(target, pname, param));
            }
        }
    }

    void GLES2StateCacheManager::setTexParameterf(GLenum target, GLenum pname, GLfloat param)
    {
        // Check if we have a map entry for this texture id. If not, create a blank one and insert it.
        TexUnitsMap::iterator it = mTexUnitsMap.find(mLastBoundTexID);
        if (it == mTexUnitsMap.end())
        {
            TextureUnitParams unit;
            mTexUnitsMap[mLastBoundTexID] = unit;

            // Update the iterator
            it = mTexUnitsMap.find(mLastBoundTexID);
        }

        // Get a local copy of the parameter map and search for this parameter
        TexParameterfMap &myMap = (*it).second.mTexParameterfMap;
        TexParameterfMap::iterator i = myMap.find(pname);

        if (i == myMap.end())
        {
            // Haven't cached this state yet.  Insert it into the map
            myMap.insert(TexParameterfMap::value_type(pname, param));

            // Update GL
            OGRE_CHECK_GL_ERROR(glTexParameterf(target, pname, param));
        }
        else
        {
            // Update the cached value if needed
            if((*i).second != param)
            {
                (*i).second = param;

                // Update GL
                OGRE_CHECK_GL_ERROR(glTexParameterf(target, pname, param));
            }
        }
    }

    void GLES2StateCacheManager::getTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
    {
        // Check if we have a map entry for this texture id.
        TexUnitsMap::iterator it = mTexUnitsMap.find(mLastBoundTexID);

        // Get a local copy of the parameter map and search for this parameter
        TexParameterfMap::iterator i = (*it).second.mTexParameterfMap.find(pname);

        *params = i->second;
    }

    void GLES2StateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        mLastBoundTexID = texture;
        
        // Update GL
        OGRE_CHECK_GL_ERROR(glBindTexture(target, texture));
    }
    
    bool GLES2StateCacheManager::activateGLTextureUnit(uchar unit)
    {
        if (mActiveTextureUnit != unit)
        {
            if (unit < dynamic_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities()->getNumTextureUnits())
            {
                OGRE_CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0 + unit));
                mActiveTextureUnit = unit;
                return true;
            }
            else if (!unit)
            {
                // always ok to use the first unit
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    
    // TODO: Store as high/low bits of a GLuint
    void GLES2StateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        if(mBlendFuncSource != source || mBlendFuncDest != dest)
        {
            mBlendFuncSource = source;
            mBlendFuncDest = dest;
            
            OGRE_CHECK_GL_ERROR(glBlendFunc(source, dest));
        }
    }
    
    void GLES2StateCacheManager::setBlendEquation(GLenum eq)
    {
        if(mBlendEquation != eq)
        {
            mBlendEquation = eq;
            
            OGRE_CHECK_GL_ERROR(glBlendEquation(eq));
        }
    }
    
    void GLES2StateCacheManager::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            OGRE_CHECK_GL_ERROR(glDepthMask(mask));
        }
    }
    
    void GLES2StateCacheManager::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            OGRE_CHECK_GL_ERROR(glDepthFunc(func));
        }
    }
    
    void GLES2StateCacheManager::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            OGRE_CHECK_GL_ERROR(glClearDepthf(depth));
        }
    }
    
    void GLES2StateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
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
            
            OGRE_CHECK_GL_ERROR(glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]));
        }
    }
    
    void GLES2StateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
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
            
            OGRE_CHECK_GL_ERROR(glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]));
        }
    }
    
    void GLES2StateCacheManager::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            OGRE_CHECK_GL_ERROR(glStencilMask(mask));
        }
    }
    
    void GLES2StateCacheManager::setEnabled(GLenum flag)
    {
        bool found = std::find(mEnableVector.begin(), mEnableVector.end(), flag) != mEnableVector.end();
        if(!found)
        {
            mEnableVector.push_back(flag);
            
            OGRE_CHECK_GL_ERROR(glEnable(flag));
        }
    }
    
    void GLES2StateCacheManager::setDisabled(GLenum flag)
    {
        vector<GLenum>::iterator iter = std::find(mEnableVector.begin(), mEnableVector.end(), flag);
        if(iter != mEnableVector.end())
        {
            mEnableVector.erase(iter);
            
            OGRE_CHECK_GL_ERROR(glDisable(flag));
        }
    }

    void GLES2StateCacheManager::setVertexAttribEnabled(GLuint attrib)
    {
        bool found = std::find(mEnabledVertexAttribs.begin(), mEnabledVertexAttribs.end(), attrib) != mEnabledVertexAttribs.end();
        if(!found)
        {
            mEnabledVertexAttribs.push_back(attrib);

            OGRE_CHECK_GL_ERROR(glEnableVertexAttribArray(attrib));
        }
    }

    void GLES2StateCacheManager::setVertexAttribDisabled(GLuint attrib)
    {
        vector<GLuint>::iterator iter = std::find(mEnabledVertexAttribs.begin(), mEnabledVertexAttribs.end(), attrib);
        if(iter != mEnabledVertexAttribs.end())
        {
            mEnabledVertexAttribs.erase(iter);

            OGRE_CHECK_GL_ERROR(glDisableVertexAttribArray(attrib));
        }
    }

    void GLES2StateCacheManager::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            OGRE_CHECK_GL_ERROR(glCullFace(face));
        }
    }
}
