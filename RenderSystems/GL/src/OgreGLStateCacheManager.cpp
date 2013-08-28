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
#include "OgreGLStateCacheManager.h"
#include "OgreGLRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GLStateCacheManager::GLStateCacheManager(void)
    {
        clearCache();
    }
    
    void GLStateCacheManager::initializeCache()
    {
        // XXX: Initial state is guaranteed by the spec, this function should probably be removed

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
    
    void GLStateCacheManager::clearCache()
    {
        mDepthMask = GL_TRUE;
        mPolygonMode = GL_FILL;
        mBlendEquationRGB = GL_FUNC_ADD;
        mBlendEquationAlpha = GL_FUNC_ADD;
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
        
        mActiveBufferMap.clear();
        mTexUnitsMap.clear();

        mViewport[0] = 0;
        mViewport[1] = 0;
        mViewport[2] = 0;
        mViewport[3] = 0;

        mAmbient[0] = 0.2;
        mAmbient[1] = 0.2;
        mAmbient[2] = 0.2;
        mAmbient[3] = 1.0;

        mDiffuse[0] = 0.8;
        mDiffuse[1] = 0.8;
        mDiffuse[2] = 0.8;
        mDiffuse[3] = 1.0;

        mSpecular[0] = 0.0;
        mSpecular[1] = 0.0;
        mSpecular[2] = 0.0;
        mSpecular[3] = 1.0;

        mEmissive[0] = 0.0;
        mEmissive[1] = 0.0;
        mEmissive[2] = 0.0;
        mEmissive[3] = 1.0;

        mLightAmbient[0] = 0.2;
        mLightAmbient[1] = 0.2;
        mLightAmbient[2] = 0.2;
        mLightAmbient[3] = 1.0;

        mShininess = 0;

        mPolygonMode = GL_FILL;

        mShadeModel = GL_SMOOTH;

        mPointSize = 1;
        mPointSizeMin = 1;
        mPointSizeMax = 1;
        mPointAttenuation[0] = 1;
        mPointAttenuation[1] = 0;
        mPointAttenuation[2] = 0;
    }
    
    GLStateCacheManager::~GLStateCacheManager(void)
    {
        mColourMask.clear();
        mClearColour.clear();
        mActiveBufferMap.clear();
        mTexUnitsMap.clear();
    }
    
    void GLStateCacheManager::bindGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
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
                glBindFramebuffer(target, buffer);
            }
            else if(target == GL_RENDERBUFFER)
            {
                glBindRenderbuffer(target, buffer);
            }
            else
            {
                glBindBuffer(target, buffer);
            }
		}
    }
    
    void GLStateCacheManager::deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach, bool force)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;
        
        BindBufferMap::iterator i = mActiveBufferMap.find(target);
        
        if (i != mActiveBufferMap.end() && ((*i).second == buffer || force))
        {
			if(target == GL_FRAMEBUFFER)
            {
                glDeleteFramebuffers(1, &buffer);
            }
            else if(target == GL_RENDERBUFFER)
            {
                glDeleteRenderbuffers(1, &buffer);
            }
            else
            {
                glDeleteBuffers(1, &buffer);
            }
        }
    }
    
    // TODO: Store as high/low bits of a GLuint, use vector instead of map for TexParameteriMap
    void GLStateCacheManager::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        GLuint texId = mBoundTextures[mActiveTextureUnit];

        // Check if we have a map entry for this texture id. If not, create a blank one and insert it.
        TexUnitsMap::iterator it = mTexUnitsMap.find(texId);
        if (it == mTexUnitsMap.end())
        {
            TextureUnitParams unit;
            mTexUnitsMap[texId] = unit;
            
            // Update the iterator
            it = mTexUnitsMap.find(texId);
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
        }
        else
        {
            // Update the cached value if needed
            if((*i).second != param)
            {
                (*i).second = param;
                
                // Update GL
                glTexParameteri(target, pname, param);
            }
        }
    }
    
    void GLStateCacheManager::bindGLTexture(GLenum target, GLuint texture)
    {
        mBoundTextures[mActiveTextureUnit] = texture;
        
        // Update GL
        glBindTexture(target, texture);
    }
    
    bool GLStateCacheManager::activateGLTextureUnit(size_t unit)
	{
		if (mActiveTextureUnit != unit)
		{
			if (GLEW_VERSION_1_2 && unit < dynamic_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities()->getNumTextureUnits())
			{
				glActiveTexture(GL_TEXTURE0 + unit);
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
    void GLStateCacheManager::setBlendFunc(GLenum source, GLenum dest)
    {
        if(mBlendFuncSource != source || mBlendFuncDest != dest)
        {
            mBlendFuncSource = source;
            mBlendFuncDest = dest;
            
            glBlendFunc(source, dest);
        }
    }
    
    void GLStateCacheManager::setBlendEquation(GLenum eq)
    {
        if(mBlendEquationRGB != eq || mBlendEquationAlpha != eq)
        {
            mBlendEquationRGB = eq;
            mBlendEquationAlpha = eq;

            if(GLEW_VERSION_1_4 || GLEW_ARB_imaging)
            {
                glBlendEquation(eq);
            }
            else if(GLEW_EXT_blend_minmax && (eq == GL_MIN || eq == GL_MAX))
            {
                glBlendEquationEXT(eq);
            }
        }
    }

    void GLStateCacheManager::setBlendEquation(GLenum eqRGB, GLenum eqAlpha)
    {
        if(mBlendEquationRGB != eqRGB || mBlendEquationAlpha != eqAlpha)
        {
            mBlendEquationRGB = eqRGB;
            mBlendEquationAlpha = eqAlpha;

			if(GLEW_VERSION_2_0) {
				glBlendEquationSeparate(eqRGB, eqAlpha);
			}
			else if(GLEW_EXT_blend_equation_separate) {
				glBlendEquationSeparateEXT(eqRGB, eqAlpha);
			}
		}
	}
    
    void GLStateCacheManager::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            glDepthMask(mask);
        }
    }
    
    void GLStateCacheManager::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            glDepthFunc(func);
        }
    }
    
    void GLStateCacheManager::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            glClearDepth(depth);
        }
    }
    
    void GLStateCacheManager::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
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
    
    void GLStateCacheManager::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
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
    
    void GLStateCacheManager::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            glStencilMask(mask);
        }
    }

    void GLStateCacheManager::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        mViewport[0] = x;
        mViewport[1] = y;
        mViewport[2] = width;
        mViewport[3] = height;
        glViewport(x,y,width,height);
    }

    void GLStateCacheManager::getViewport(int *array)
    {
        for (int i=0; i<4; ++i)
            array[i] = mViewport[i];
    }

    void GLStateCacheManager::setFFPCurrentTextureType(GLenum type)
    {
        HashMap<GLenum, GLenum>::iterator it = mFFPCurrentTextureTypes.find(mActiveTextureUnit);
        if (it != mFFPCurrentTextureTypes.end())
        {
            glDisable (it->second);
            if (type != GL_TEXTURE_2D_ARRAY_EXT)
            {
                glEnable(type);
                it->second = type;
            }
            else
                mFFPCurrentTextureTypes.erase(it);
        }
        else
        {
            if (type != GL_TEXTURE_2D_ARRAY_EXT)
            {
                glEnable(type);
                mFFPCurrentTextureTypes[mActiveTextureUnit] = type;
            }
        }
    }

    void GLStateCacheManager::removeFFPCurrentTextureType()
    {
        HashMap<GLenum, GLenum>::iterator it = mFFPCurrentTextureTypes.find(mActiveTextureUnit);
        if (it != mFFPCurrentTextureTypes.end())
        {
            glDisable(it->second);
            mFFPCurrentTextureTypes.erase(it);
        }
    }

    void GLStateCacheManager::setEnabled(GLenum flag)
    {
        if (mEnabled.find(flag) == mEnabled.end())
        {
            mEnabled.insert(flag);
            glEnable(flag);
        }
    }

    bool GLStateCacheManager::getEnabled(GLenum flag)
    {
        return (mEnabled.find(flag) != mEnabled.end());
    }
    
    void GLStateCacheManager::setDisabled(GLenum flag)
    {
        std::set<GLenum>::iterator it = mEnabled.find(flag);
        if (it != mEnabled.end())
        {
            mEnabled.erase(it);
            glDisable(flag);
        }
    }
    
    void GLStateCacheManager::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            glCullFace(face);
        }
    }

    void GLStateCacheManager::setDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        if((mDiffuse[0] != r) ||
           (mDiffuse[1] != g) ||
           (mDiffuse[2] != b) ||
           (mDiffuse[3] != a))
        {
            mDiffuse[0] = r;
            mDiffuse[1] = g;
            mDiffuse[2] = b;
            mDiffuse[3] = a;

            // XXX GL_FRONT or GL_FRONT_AND_BACK?
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &mDiffuse[0]);
        }
    }

    void GLStateCacheManager::setAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        if((mAmbient[0] != r) ||
           (mAmbient[1] != g) ||
           (mAmbient[2] != b) ||
           (mAmbient[3] != a))
        {
            mAmbient[0] = r;
            mAmbient[1] = g;
            mAmbient[2] = b;
            mAmbient[3] = a;

            // XXX GL_FRONT or GL_FRONT_AND_BACK?
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &mAmbient[0]);
        }
    }

    void GLStateCacheManager::setEmissive(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        if((mEmissive[0] != r) ||
           (mEmissive[1] != g) ||
           (mEmissive[2] != b) ||
           (mEmissive[3] != a))
        {
            mEmissive[0] = r;
            mEmissive[1] = g;
            mEmissive[2] = b;
            mEmissive[3] = a;

            // XXX GL_FRONT or GL_FRONT_AND_BACK?
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &mEmissive[0]);
        }
    }

    void GLStateCacheManager::setSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    {
        if((mSpecular[0] != r) ||
           (mSpecular[1] != g) ||
           (mSpecular[2] != b) ||
           (mSpecular[3] != a))
        {
            mSpecular[0] = r;
            mSpecular[1] = g;
            mSpecular[2] = b;
            mSpecular[3] = a;

            // XXX GL_FRONT or GL_FRONT_AND_BACK?
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &mSpecular[0]);
        }
    }

    void GLStateCacheManager::setShininess(GLfloat shininess)
    {
        if (shininess != mShininess)
        {
            mShininess = shininess;
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
        }
    }

    void GLStateCacheManager::setPolygonMode(GLenum mode)
    {
        if (mPolygonMode != mode)
        {
            mPolygonMode = mode;
            glPolygonMode(GL_FRONT_AND_BACK, mPolygonMode);
        }
    }

    void GLStateCacheManager::setShadeModel(GLenum model)
    {
        if (mShadeModel != model)
        {
            mShadeModel = model;
            glShadeModel(model);
        }
    }

    void GLStateCacheManager::setLightAmbient(GLfloat r, GLfloat g, GLfloat b)
    {
        if((mLightAmbient[0] != r) ||
           (mLightAmbient[1] != g) ||
           (mLightAmbient[2] != b))
        {
            mLightAmbient[0] = r;
            mLightAmbient[1] = g;
            mLightAmbient[2] = b;

            // XXX GL_FRONT or GL_FRONT_AND_BACK?
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &mLightAmbient[0]);
        }
    }

    void GLStateCacheManager::setPointSize(GLfloat size)
    {
        if (mPointSize != size)
        {
            mPointSize = size;
            glPointSize(mPointSize);
        }
    }

    void GLStateCacheManager::setPointParameters(GLfloat *attenuation, float minSize, float maxSize)
    {
        if (minSize != mPointSizeMin)
        {
            mPointSizeMin = minSize;
            const Ogre::RenderSystemCapabilities* caps = dynamic_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities();
            if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
                glPointParameterf(GL_POINT_SIZE_MIN, mPointSizeMin);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_ARB))
                glPointParameterfARB(GL_POINT_SIZE_MIN, mPointSizeMin);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_EXT))
                glPointParameterfEXT(GL_POINT_SIZE_MIN, mPointSizeMin);
        }
        if (maxSize != mPointSizeMax)
        {
            mPointSizeMax = maxSize;
            const Ogre::RenderSystemCapabilities* caps = dynamic_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities();
            if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
                glPointParameterf(GL_POINT_SIZE_MAX, mPointSizeMax);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_ARB))
                glPointParameterfARB(GL_POINT_SIZE_MAX, mPointSizeMax);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_EXT))
                glPointParameterfEXT(GL_POINT_SIZE_MAX, mPointSizeMax);
        }
        if (attenuation[0] != mPointAttenuation[0] || attenuation[1] != mPointAttenuation[1] || attenuation[2] != mPointAttenuation[2])
        {
            mPointAttenuation[0] = attenuation[0];
            mPointAttenuation[1] = attenuation[1];
            mPointAttenuation[2] = attenuation[2];
            const Ogre::RenderSystemCapabilities* caps = dynamic_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities();
            if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
                glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, &mPointAttenuation[0]);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_ARB))
                glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION, &mPointAttenuation[0]);
            else if (caps->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_EXT))
                glPointParameterfvEXT(GL_POINT_DISTANCE_ATTENUATION, &mPointAttenuation[0]);
        }
    }
}
