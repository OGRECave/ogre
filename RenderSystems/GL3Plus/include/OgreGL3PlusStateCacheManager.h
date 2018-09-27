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

#ifndef __GL3PlusStateCacheManager_H__
#define __GL3PlusStateCacheManager_H__

#include "OgreGL3PlusPrerequisites.h"
#include "OgreGLStateCacheManagerCommon.h"
#include "OgreStdHeaders.h"
#include "OgreIteratorWrappers.h"

namespace Ogre
{
    class _OgreGL3PlusExport GL3PlusStateCacheManager : public GLStateCacheManagerCommon
    {
    protected:
        struct TextureUnitParams
        {
            TexParameteriMap mTexParameteriMap;
        };

        typedef std::unordered_map<GLuint, TextureUnitParams> TexUnitsMap;

        /* These variables are used for caching OpenGL state.
         They are cached because state changes can be quite expensive,
         which is especially important on mobile or embedded systems.
         */

        /// Stores textures currently bound to each texture stage
        std::unordered_map <GLenum, GLuint> mBoundTextures;

        struct TexGenParams
        {
            std::set<GLenum> mEnabled;
        };
        /// Stores the currently enabled texcoord generation types per texture unit
        std::unordered_map <GLenum, TexGenParams> mTextureCoordGen;

        /// Stores the currently bound draw frame buffer value
        GLuint mActiveDrawFrameBuffer;
        /// Stores the currently bound read frame buffer value
        GLuint mActiveReadFrameBuffer;
        /// Stores the currently bound vertex array object
        GLuint mActiveVertexArray;
        /// A map of texture parameters for each texture unit
        TexUnitsMap mTexUnitsMap;
        /// Stores the current polygon rendering mode
        GLenum mPolygonMode;
        /// Stores the last bound texture id
        GLuint mLastBoundTexID;
        /// Stores the currently bound separable program pipeline
        GLuint mActiveProgramPipeline;

        GLfloat mPointSize;
    public:
        GL3PlusStateCacheManager(void);
        
        /// See GL3PlusStateCacheManager.initializeCache.
        void initializeCache();

        /** Clears all cached values
        */
        void clearCache();
        

        /** Bind an OpenGL frame buffer.
         @param target The buffer target.
         @param buffer The buffer ID.
         @param force Optional parameter to force an update.
         */
        void bindGLFrameBuffer(GLenum target,GLuint buffer, bool force = false);

        /** Bind an OpenGL frame buffer.
         @param buffer The buffer ID.
         @param force Optional parameter to force an update.
         */
        void bindGLRenderBuffer(GLuint buffer, bool force = false);

        /** Bind an OpenGL buffer of any type.
         @param target The buffer target.
         @param buffer The buffer ID.
         @param force Optional parameter to force an update.
         */
        void bindGLBuffer(GLenum target, GLuint buffer, bool force = false);

        /** Delete an OpenGL frame buffer.
         @param target The buffer target.
         @param buffer The buffer ID.
         */
        void deleteGLFrameBuffer(GLenum target, GLuint buffer);

        /** Delete an OpenGL render buffer.
         @param buffer The buffer ID.
         */
        void deleteGLRenderBuffer(GLuint buffer);
        /** Delete an OpenGL buffer of any type.
         @param target The buffer target.
         @param buffer The buffer ID.
         */
        void deleteGLBuffer(GLenum target, GLuint buffer);

        /** Bind an OpenGL Vertex array object.
         @param vao The vertex array object ID.
         */
        void bindGLVertexArray(GLuint vao);
        
        /** Bind an OpenGL texture of any type.
         @param target The texture target.
         @param texture The texture ID.
         */
        void bindGLTexture(GLenum target, GLuint texture);

        /** Invalidates the state associated with a particular texture ID.
         @param texture The texture ID.
         */
        void invalidateStateForTexture(GLuint texture);

        /** Sets an integer parameter value per texture target.
         @param target The texture target.
         @param pname The parameter name.
         @param param The parameter value.
         */
        void setTexParameteri(GLenum target, GLenum pname, GLint param);

        /** Activate an OpenGL texture unit.
         @param unit The texture unit to activate.
         @return Whether or not the texture unit was successfully activated.
         */
        bool activateGLTextureUnit(size_t unit);

        /// Set the blend equation for RGB and alpha separately.
        void setBlendEquation(GLenum eqRGB, GLenum eqA);

        /// Set the blend function for RGB and alpha separately.
        void setBlendFunc(GLenum source, GLenum dest, GLenum sourceA, GLenum destA);

        /** Sets the current depth mask setting.
         @param mask The depth mask to use.
         */
        void setDepthMask(GLboolean mask);

        /** Gets the current depth test function.
         @return The current depth test function.
         */
        GLenum getDepthFunc(void) const { return mDepthFunc; }

        /** Sets the current depth test function.
         @param func The depth test function to use.
         */
        void setDepthFunc(GLenum func);

        /** Gets the clear depth in the range from [0..1].
         @return The current clearing depth.
         */
        GLclampf getClearDepth(void) const { return mClearDepth; }

        /** Sets the clear depth in the range from [0..1].
         @param depth The clear depth to use.
         */
        void setClearDepth(GLclampf depth);

        /** Sets the color to clear to.
         @param red The red component.
         @param green The green component.
         @param blue The blue component.
         @param alpha The alpha component.
         */
        void setClearColour(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

        /** Sets the current colour mask.
         @param red The red component.
         @param green The green component.
         @param blue The blue component.
         @param alpha The alpha component.
         */
        void setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

        /** Sets the stencil mask.
         @param mask The stencil mask to use
         */
        void setStencilMask(GLuint mask);

        /** Enables a piece of OpenGL functionality.
         @param flag The function to enable.
         */
        void setEnabled(GLenum flag, bool enabled);

        /** Gets the current polygon rendering mode, fill, wireframe, points, etc.
         @return The current polygon rendering mode.
         */
        GLenum getPolygonMode(void) const { return mPolygonMode; }

        /** Sets the current polygon rendering mode.
         @param mode The polygon mode to use.
         */
        void setPolygonMode(GLenum mode);

        /** Sets the face culling mode.
         @return The current face culling mode
         */
        GLenum getCullFace(void) const { return mCullFace; }

        /** Sets the face culling setting.
         @param face The face culling mode to use.
         */
        void setCullFace(GLenum face);

         /** Bind an OpenGL separable program pipeline
         @param handle The handle to the program pipeline
         */
        void bindGLProgramPipeline(GLuint handle);

        /// Enable the specified texture coordinate generation option for the currently active texture unit
        void enableTextureCoordGen(GLenum type);
        /// Disable the specified texture coordinate generation option for the currently active texture unit
        void disableTextureCoordGen(GLenum type);

        void setPointSize(GLfloat size);

        void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    };
}

#endif
