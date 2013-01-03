/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2013 Torus Knot Software Ltd
 
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
#include "OgreGLES2StateCacheManagerImp.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GLES2StateCacheManagerImp::GLES2StateCacheManagerImp(void)
    {
        clearCache();
    }
    
    void GLES2StateCacheManagerImp::initializeCache()
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
    
    void GLES2StateCacheManagerImp::clearCache()
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
        mLastBoundedTexID = 0;
        
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
    }
    
    GLES2StateCacheManagerImp::~GLES2StateCacheManagerImp(void)
    {
        mColourMask.clear();
        mClearColour.clear();
        mActiveBufferMap.clear();
        mEnableVector.clear();
        mTexUnitsMap.clear();
    }
    
//#pragma mark Buffer bindings
    void GLES2StateCacheManagerImp::bindGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
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
    
    void GLES2StateCacheManagerImp::deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
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
        }
    }
    
//#pragma mark Texture settings and bindings
    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GLES2StateCacheManagerImp::setTexParameteri(GLenum target, GLenum pname, GLint param)
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
    
    void GLES2StateCacheManagerImp::bindGLTexture(GLenum target, GLuint texture)
    {
        mLastBoundedTexID = texture;
        
        // Update GL
        OGRE_CHECK_GL_ERROR(glBindTexture(target, texture));
    }
    
    bool GLES2StateCacheManagerImp::activateGLTextureUnit(size_t unit)
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
    
//#pragma mark Blending settings
    // TODO: Store as high/low bits of a GLuint
    void GLES2StateCacheManagerImp::setBlendFunc(GLenum source, GLenum dest)
    {
        if(mBlendFuncSource != source || mBlendFuncDest != dest)
        {
            mBlendFuncSource = source;
            mBlendFuncDest = dest;
            
            OGRE_CHECK_GL_ERROR(glBlendFunc(source, dest));
        }
    }
    
    void GLES2StateCacheManagerImp::setBlendEquation(GLenum eq)
    {
        if(mBlendEquation != eq)
        {
            mBlendEquation = eq;
            
            OGRE_CHECK_GL_ERROR(glBlendEquation(eq));
        }
    }
    
//#pragma mark Depth settings
    void GLES2StateCacheManagerImp::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            OGRE_CHECK_GL_ERROR(glDepthMask(mask));
        }
    }
    
    void GLES2StateCacheManagerImp::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            OGRE_CHECK_GL_ERROR(glDepthFunc(func));
        }
    }
    
//#pragma mark Clear settings
    void GLES2StateCacheManagerImp::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            OGRE_CHECK_GL_ERROR(glClearDepthf(depth));
        }
    }
    
    void GLES2StateCacheManagerImp::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
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
    
//#pragma mark Masks
    void GLES2StateCacheManagerImp::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
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
    
    void GLES2StateCacheManagerImp::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            OGRE_CHECK_GL_ERROR(glStencilMask(mask));
        }
    }
    
//#pragma mark Enable/Disable
    void GLES2StateCacheManagerImp::setEnabled(GLenum flag)
    {
        bool found = std::find(mEnableVector.begin(), mEnableVector.end(), flag) != mEnableVector.end();
        if(!found)
        {
            mEnableVector.push_back(flag);
            
            OGRE_CHECK_GL_ERROR(glEnable(flag));
        }
    }
    
    void GLES2StateCacheManagerImp::setDisabled(GLenum flag)
    {
        vector<GLenum>::iterator iter = std::find(mEnableVector.begin(), mEnableVector.end(), flag);
        if(iter != mEnableVector.end())
        {
            mEnableVector.erase(iter);
            
            OGRE_CHECK_GL_ERROR(glDisable(flag));
        }
    }
    
//#pragma mark Other
    void GLES2StateCacheManagerImp::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            OGRE_CHECK_GL_ERROR(glCullFace(face));
        }
    }
}
