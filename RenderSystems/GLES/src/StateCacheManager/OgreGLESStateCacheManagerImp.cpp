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
#include "OgreGLESStateCacheManagerImp.h"
#include "OgreGLESRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GLESStateCacheManagerImp::GLESStateCacheManagerImp(void)
    {
        clearCache();
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
        mLastBoundedTexID = 0;
        
        // Initialize our cache variables and also the GL so that the
        // stored values match the GL state
        mBlendFuncSrc = GL_ONE;
        mBlendFuncDest = GL_ZERO;
        
        mClearColour.resize(4);
        mClearColour[0] = mClearColour[1] = mClearColour[2] = mClearColour[3] = 0.0f;
        
        mColourMask.resize(4);
        mColourMask[0] = mColourMask[1] = mColourMask[2] = mColourMask[3] = GL_TRUE;
        
        mEnableVector.reserve(25);
        mEnableVector.clear();
        mActiveBufferMap.clear();
        mTexUnitsMap.clear();
        mActiveTextureMap.clear();
    }
    
    GLESStateCacheManagerImp::~GLESStateCacheManagerImp(void)
    {
        mColourMask.clear();
        mClearColour.clear();
        mActiveBufferMap.clear();
        mEnableVector.clear();
        mTexUnitsMap.clear();
        mActiveTextureMap.clear();
    }
    
#pragma mark Buffer bindings
    void GLESStateCacheManagerImp::bindGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        GLuint hash = target | (attach << 16);
        GLBindingMap::iterator i = mActiveBufferMap.find(hash);
        if (i == mActiveBufferMap.end())
        {
            // Update GL
            if(target == GL_FRAMEBUFFER_OES)
                glBindFramebufferOES(target, buffer);
            else if(target == GL_RENDERBUFFER_OES)
                glBindRenderbufferOES(target, buffer);
            else
                glBindBuffer(target, buffer);
            
            GL_CHECK_ERROR
            
            // Haven't cached this state yet.  Insert it into the map
            mActiveBufferMap.insert(GLBindingMap::value_type(hash, buffer));
        }
        else
        {
            // Update the cached value if needed
            if((*i).second != buffer || force)
            {
                (*i).second = buffer;
                
                // Update GL
                if(target == GL_FRAMEBUFFER_OES)
                    glBindFramebufferOES(target, buffer);
                else if(target == GL_RENDERBUFFER_OES)
                    glBindRenderbufferOES(target, buffer);
                else
                    glBindBuffer(target, buffer);
                
                GL_CHECK_ERROR
            }
        }
    }
    
    void GLESStateCacheManagerImp::deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        GLuint hash = target | (attach << 16);
        GLBindingMap::iterator i = mActiveBufferMap.find(hash);
        
        if (i != mActiveBufferMap.end())
        {
            if((*i).second == buffer || force)
            {
                if(target == GL_FRAMEBUFFER_OES)
                    glDeleteFramebuffersOES(1, &buffer);
                else if(target == GL_RENDERBUFFER_OES)
                    glDeleteRenderbuffersOES(1, &buffer);
                else
                    glDeleteBuffers(1, &buffer);
                
                GL_CHECK_ERROR
            }
        }
    }
    
#pragma mark Texture settings and bindings
    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GLESStateCacheManagerImp::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        // Check if we have a map entry for this texture id. If not, create a blank one and insert it.
        TexUnitsMap::iterator it = mTexUnitsMap.find(mLastBoundedTexID);
        if (it == mTexUnitsMap.end())
        {
            TextureUnitParams unit;
            mTexUnitsMap[mLastBoundedTexID] = unit;
            
            // Update the iterator
            it = mTexUnitsMap.find(mLastBoundedTexID);
        }
        
        // Get a local copy of the parameter map and search for this parameter
        TexParameteriMap &myMap = (*it).second.mTexParameteriMap;
        TexParameteriMap::iterator i = myMap.find(pname);
        
        if (i == myMap.end())
        {
            // Haven't cached this state yet.  Insert it into the map
            myMap.insert(TexParameteriMap::value_type(pname, param));
            
            // Update GL
            glTexParameteri(target, pname, param);
            GL_CHECK_ERROR
        }
        else
        {
            // Update the cached value if needed
            if((*i).second != param)
            {
                (*i).second = param;
                
                // Update GL
                glTexParameteri(target, pname, param);
                GL_CHECK_ERROR
            }
        }
    }
    
    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GLESStateCacheManagerImp::bindGLTexture(GLenum target, GLuint texture)
    {
        mLastBoundedTexID = texture;
        
        GLBindingMap::iterator i = mActiveTextureMap.find(target);
        if (i == mActiveTextureMap.end())
        {
            // Haven't cached this state yet. Insert it into the map
            mActiveTextureMap.insert(GLBindingMap::value_type(target, texture));
            
            // Update GL
            glBindTexture(target, texture);
            GL_CHECK_ERROR
        }
        else
        {
            (*i).second = texture;
                
            // Update GL
            glBindTexture(target, texture);
            GL_CHECK_ERROR
        }
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
    
#pragma mark Blending settings
    // TODO: Store as high/low bits of a GLuint
    void GLESStateCacheManagerImp::setBlendFunc(GLenum source, GLenum dest)
    {
        if(mBlendFuncSrc != source ||
           mBlendFuncDest != dest)
        {
            mBlendFuncSrc = source;
            mBlendFuncDest = dest;
            
            glBlendFunc(source, dest);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setBlendEquation(GLenum eq)
    {
        if(mBlendEquation != eq)
        {
            mBlendEquation = eq;
            
            glBlendEquationOES(eq);
            GL_CHECK_ERROR
        }
    }
    
#pragma mark Depth settings
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
    
#pragma mark Clear settings
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
    
#pragma mark Masks
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
    
#pragma mark Enable/Disable
    void GLESStateCacheManagerImp::setEnabled(GLenum flag)
    {
        bool found = std::find(mEnableVector.begin(), mEnableVector.end(), flag) != mEnableVector.end();
        if(!found)
        {
            mEnableVector.push_back(flag);
            
            glEnable(flag);
            GL_CHECK_ERROR
        }
    }
    
    void GLESStateCacheManagerImp::setDisabled(GLenum flag)
    {
        vector<GLenum>::iterator iter = std::find(mEnableVector.begin(), mEnableVector.end(), flag);
        if(iter != mEnableVector.end())
        {
            mEnableVector.erase(iter);
            
            glDisable(flag);
            GL_CHECK_ERROR
        }
    }
    
#pragma mark Other
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
