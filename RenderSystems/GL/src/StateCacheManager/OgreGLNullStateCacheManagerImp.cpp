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
#include "OgreGLNullStateCacheManagerImp.h"
#include "OgreGLRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre {
    
    GLStateCacheManagerImp::GLStateCacheManagerImp(void) 
    {
        clearCache();
    }
    
    GLStateCacheManagerImp::~GLStateCacheManagerImp(void)
    {
    }
    
    void GLStateCacheManagerImp::initializeCache()
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

        glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

        glBindRenderbufferEXT(GL_RENDERBUFFER, 0);

        glActiveTexture(GL_TEXTURE0);

        glClearColor(mClearColour[0], mClearColour[1], mClearColour[2], mClearColour[3]);

        glColorMask(mColourMask[0], mColourMask[1], mColourMask[2], mColourMask[3]);
    }
    
    void GLStateCacheManagerImp::clearCache()
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
    
    void GLStateCacheManagerImp::bindGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        // Update GL
        if(target == GL_FRAMEBUFFER)
        {
            glBindFramebufferEXT(target, buffer);
        }
        else if(target == GL_RENDERBUFFER)
        {
            glBindRenderbufferEXT(target, buffer);
        }
        else
        {
            glBindBuffer(target, buffer);
        }
    }
    
    void GLStateCacheManagerImp::deleteGLBuffer(GLenum target, GLuint buffer, bool force)
    {
        // Buffer name 0 is reserved and we should never try to delete it
        if(buffer == 0)
            return;

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
    
    void GLStateCacheManagerImp::setTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        // Update GL
        glTexParameteri(target, pname, param);
    }
    
    void GLStateCacheManagerImp::invalidateStateForTexture(GLuint texture) { }

    void GLStateCacheManagerImp::bindGLTexture(GLenum target, GLuint texture)
    {
        // Update GL
        glBindTexture(target, texture);
    }
    
    bool GLStateCacheManagerImp::activateGLTextureUnit(size_t unit)
	{
        // Always return true for the currently bound texture unit
        if (mActiveTextureUnit == unit)
            return true;
        
        if (unit < dynamic_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem())->getCapabilities()->getNumTextureUnits())
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
    
    void GLStateCacheManagerImp::setBlendFunc(GLenum source, GLenum dest)
    {
        glBlendFunc(source, dest);
    }
    
    void GLStateCacheManagerImp::setBlendEquation(GLenum eq)
    {
        mBlendEquation = eq;
        glBlendEquation(eq);
    }
    
    void GLStateCacheManagerImp::setDepthMask(GLboolean mask)
    {
        if(mDepthMask != mask)
        {
            mDepthMask = mask;
            
            glDepthMask(mask);
        }
    }
    
    void GLStateCacheManagerImp::setDepthFunc(GLenum func)
    {
        if(mDepthFunc != func)
        {
            mDepthFunc = func;
            
            glDepthFunc(func);
        }
    }
    
    void GLStateCacheManagerImp::setClearDepth(GLclampf depth)
    {
        if(mClearDepth != depth)
        {
            mClearDepth = depth;
            
            glClearDepth(depth);
        }
    }
    
    void GLStateCacheManagerImp::setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
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
    
    void GLStateCacheManagerImp::setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
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
    
    void GLStateCacheManagerImp::setStencilMask(GLuint mask)
    {
        if(mStencilMask != mask)
        {
            mStencilMask = mask;
            
            glStencilMask(mask);
        }
    }
    
    void GLStateCacheManagerImp::setEnabled(GLenum flag, bool enabled)
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

    void GLStateCacheManagerImp::setCullFace(GLenum face)
    {
        if(mCullFace != face)
        {
            mCullFace = face;
            
            glCullFace(face);
        }
    }

    void GLStateCacheManagerImp::setViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        mViewport[0] = x;
        mViewport[1] = y;
        mViewport[2] = width;
        mViewport[3] = height;
        glViewport(x, y, width, height);
    }

    void GLStateCacheManagerImp::getViewport(int *array)
    {
        for (int i = 0; i < 4; ++i)
            array[i] = mViewport[i];
    }

    void GLStateCacheManagerImp::setBlendEquation(GLenum eqRGB, GLenum eqAlpha)
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

    void GLStateCacheManagerImp::setMaterialDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &mDiffuse[0]);
        }
    }

    void GLStateCacheManagerImp::setMaterialAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &mAmbient[0]);
        }
    }

    void GLStateCacheManagerImp::setMaterialEmissive(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &mEmissive[0]);
        }
    }

    void GLStateCacheManagerImp::setMaterialSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
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

            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &mSpecular[0]);
        }
    }

    void GLStateCacheManagerImp::setMaterialShininess(GLfloat shininess)
    {
        if (shininess != mShininess)
        {
            mShininess = shininess;
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
        }
    }

    void GLStateCacheManagerImp::setShadeModel(GLenum model)
    {
        if (mShadeModel != model)
        {
            mShadeModel = model;
            glShadeModel(model);
        }
    }

    void GLStateCacheManagerImp::setLightAmbient(GLfloat r, GLfloat g, GLfloat b)
    {
        if((mLightAmbient[0] != r) ||
           (mLightAmbient[1] != g) ||
           (mLightAmbient[2] != b))
        {
            mLightAmbient[0] = r;
            mLightAmbient[1] = g;
            mLightAmbient[2] = b;

            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &mLightAmbient[0]);
        }
    }

    void GLStateCacheManagerImp::setPointSize(GLfloat size)
    {
        if (mPointSize != size)
        {
            mPointSize = size;
            glPointSize(mPointSize);
        }
    }

    void GLStateCacheManagerImp::setPointParameters(GLfloat *attenuation, float minSize, float maxSize)
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

    void GLStateCacheManagerImp::enableTextureCoordGen(GLenum type)
    {
        glEnable(type);
    }

    void GLStateCacheManagerImp::disableTextureCoordGen(GLenum type)
    {
        glDisable(type);
    }

    void GLStateCacheManagerImp::setPolygonMode(GLenum mode)
    {
        mPolygonMode = mode;
        glPolygonMode(GL_FRONT_AND_BACK, mPolygonMode);
    }
}
