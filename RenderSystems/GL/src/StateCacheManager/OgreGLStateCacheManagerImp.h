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

#ifndef __GLStateCacheManagerImp_H__
#define __GLStateCacheManagerImp_H__

#include "OgreGLPrerequisites.h"

typedef Ogre::GeneralAllocatedObject StateCacheAlloc;

namespace Ogre
{
    /** An in memory cache of the OpenGL state.
     @see GLStateCacheManager
     */
    class _OgreGLExport GLStateCacheManagerImp : public StateCacheAlloc
    {
    private:
        typedef HashMap<GLenum, GLuint> BindBufferMap;
        typedef HashMap<GLenum, GLint> TexParameteriMap;
        typedef HashMap<GLenum, bool> GLbooleanStateMap;

        struct TextureUnitParams
        {
            ~TextureUnitParams()
            {
                mTexParameteriMap.clear();
            }

            TexParameteriMap mTexParameteriMap;
        };

        typedef HashMap<GLuint, TextureUnitParams> TexUnitsMap;

        /* These variables are used for caching OpenGL state.
         They are cached because state changes can be quite expensive,
         which is especially important on mobile or embedded systems.
         */

        /// Stores textures currently bound to each texture stage
        HashMap <GLenum, GLuint> mBoundTextures;

        struct TexGenParams
        {
            std::set<GLenum> mEnabled;
        };
        /// Stores the currently enabled texcoord generation types per texture unit
        HashMap <GLenum, TexGenParams> mTextureCoordGen;

        /// A map of different buffer types and the currently bound buffer for each type
        BindBufferMap mActiveBufferMap;
        /// A map of texture parameters for each texture unit
        TexUnitsMap mTexUnitsMap;
        /// Array of each OpenGL feature that is enabled i.e. blending, depth test, etc.
        GLbooleanStateMap mBoolStateMap;
        /// Stores the current clear colour
        vector<GLclampf>::type mClearColour;
        /// Stores the current colour write mask
        vector<GLboolean>::type mColourMask;
        /// Stores the current depth write mask
        GLboolean mDepthMask;
        /// Stores the current polygon rendering mode
        GLenum mPolygonMode;
        /// Stores the current blend equation
        GLenum mBlendEquation;
        /// Stores the current blend source function
        GLenum mBlendFuncSource;
        /// Stores the current blend destination function
        GLenum mBlendFuncDest;
        /// Stores the current face culling setting
        GLenum mCullFace;
        /// Stores the current depth test function
        GLenum mDepthFunc;
        /// Stores the current stencil mask
        GLuint mStencilMask;
		/// Stores the last bound texture id
        GLuint mLastBoundTexID;
        /// Stores the currently active texture unit
        size_t mActiveTextureUnit;
        /// Mask of buffers who contents can be discarded if GL_EXT_discard_framebuffer is supported
        unsigned int mDiscardBuffers;
        /// Stores the current depth clearing colour
        GLclampf mClearDepth;
        /// Viewport origin and size
        int mViewport[4];

        GLenum mBlendEquationRGB;
        GLenum mBlendEquationAlpha;
        GLenum mShadeModel;

        GLfloat mAmbient[4];
        GLfloat mDiffuse[4];
        GLfloat mSpecular[4];
        GLfloat mEmissive[4];
        GLfloat mLightAmbient[4];
        GLfloat mShininess;

        GLfloat mPointAttenuation[3];
        GLfloat mPointSize;
        GLfloat mPointSizeMin;
        GLfloat mPointSizeMax;

    public:
        GLStateCacheManagerImp(void);
        ~GLStateCacheManagerImp(void);
        
        /// See GLStateCacheManager.initializeCache.
        void initializeCache();
        
        /// See GLStateCacheManager.clearCache.
        void clearCache();
		
        /// See GLStateCacheManager.bindGLBuffer.
        void bindGLBuffer(GLenum target, GLuint buffer, GLenum attach = 0, bool force = false);
        
        /// See GLStateCacheManager.deleteGLBuffer.
        void deleteGLBuffer(GLenum target, GLuint buffer, GLenum attach = 0, bool force = false);
        
        /// See GLStateCacheManager.bindGLTexture.
        void bindGLTexture(GLenum target, GLuint texture);
        
        /// See GLStateCacheManager.setTexParameteri.
        void setTexParameteri(GLenum target, GLenum pname, GLint param);

        /// See GLStateCacheManager.invalidateStateForTexture.
        void invalidateStateForTexture(GLuint texture);

        /// See GLStateCacheManager.activateGLTextureUnit.
        bool activateGLTextureUnit(size_t unit);
        
        /// See GLStateCacheManager.getBlendEquation.
        GLenum getBlendEquation(void) const { return mBlendEquation; }
        
        /// See GLStateCacheManager.setBlendEquation.
        void setBlendEquation(GLenum eq);

        /// Set the blend equation for RGB and alpha separately.
        void setBlendEquation(GLenum eqRGB, GLenum eqA);

        /// See GLStateCacheManager.setBlendFunc.
        void setBlendFunc(GLenum source, GLenum dest);

        /// See GLStateCacheManager.setShadeModel.
        void setShadeModel(GLenum model);

        /// See GLStateCacheManager.setLightAmbient.
        void setLightAmbient(GLfloat r, GLfloat g, GLfloat b);

        /// See GLStateCacheManager.getDepthMask.
        GLboolean getDepthMask(void) const { return mDepthMask; }
        
        /// See GLStateCacheManager.setDepthMask.
        void setDepthMask(GLboolean mask);
        
        /// See GLStateCacheManager.getDepthFunc.
        GLenum getDepthFunc(void) const { return mDepthFunc; }
        
        /// See GLStateCacheManager.setDepthFunc.
        void setDepthFunc(GLenum func);
        
        /// See GLStateCacheManager.getClearDepth.
        GLclampf getClearDepth(void) const { return mClearDepth; }
        
        /// See GLStateCacheManager.setClearDepth.
        void setClearDepth(GLclampf depth);
        
        /// See GLStateCacheManager.setClearColour.
        void setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
        
        /// See GLStateCacheManager.getColourMask.
        vector<GLboolean>::type & getColourMask(void) { return mColourMask; }
        
        /// See GLStateCacheManager.setColourMask.
        void setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
        
        /// See GLStateCacheManager.getStencilMask.
        GLuint getStencilMask(void) const { return mStencilMask; }
        
        /// See GLStateCacheManager.setStencilMask.
        void setStencilMask(GLuint mask);
        
        /// See GLStateCacheManager.setEnabled.
        void setEnabled(GLenum flag, bool enabled);

        /// See GLStateCacheManager.getDiscardBuffers.
        unsigned int getDiscardBuffers(void) const { return mDiscardBuffers; }
        
        /// See GLStateCacheManager.setDiscardBuffers.
        void setDiscardBuffers(unsigned int flags) { mDiscardBuffers = flags; }
        
        /// See GLStateCacheManager.getPolygonMode.
        GLenum getPolygonMode(void) const { return mPolygonMode; }
        
        /// See GLStateCacheManager.setPolygonMode.
        void setPolygonMode(GLenum mode);
        
        /// See GLStateCacheManager.getCullFace.
        GLenum getCullFace(void) const { return mCullFace; }
        
        /// See GLStateCacheManager.setCullFace.
        void setCullFace(GLenum face);

        /// Enable the specified texture coordinate generation option for the currently active texture unit
        void enableTextureCoordGen(GLenum type);
        /// Disable the specified texture coordinate generation option for the currently active texture unit
        void disableTextureCoordGen(GLenum type);

        // Set material lighting parameters
        void setMaterialAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
        void setMaterialDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
        void setMaterialEmissive(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
        void setMaterialSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
        void setMaterialShininess(GLfloat shininess);
        void setPointSize(GLfloat size);
        void setPointParameters(GLfloat* attenuation, float minSize, float maxSize);

        /// Set viewport parameters
        void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);

        /// Get viewport parameters
        void getViewport(int* array);
    };
}

#endif
