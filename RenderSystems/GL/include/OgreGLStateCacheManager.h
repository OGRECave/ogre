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

#ifndef __GLStateCacheManager_H__
#define __GLStateCacheManager_H__

#include "OgreGLPrerequisites.h"
#include "OgreStdHeaders.h"
#include "OgreIteratorWrappers.h"

typedef Ogre::GeneralAllocatedObject StateCacheAlloc;

namespace Ogre
{
	class GLStateCacheManagerImp;

    /** An in memory cache of the OpenGL state.
     @remarks
     State changes can be particularly expensive time wise. This is because
     a change requires OpenGL to re-evaluate and update the state machine.
     Because of the general purpose nature of OGRE we often set the state for
     a specific texture, material, buffer, etc. But this may be the same as the
     current status of the state machine and is therefore redundant and causes
     unnecessary work to be performed by OpenGL.
     @par
     Instead we are caching the state so that we can check whether it actually
     does need to be updated. This leads to improved performance all around and 
     can be somewhat dramatic in some cases.
     */
    class _OgreGLExport GLStateCacheManager : public StateCacheAlloc
    {
    private:
		GLStateCacheManagerImp* mImp;
        typedef map<intptr_t, GLStateCacheManagerImp*>::type CachesMap;
        typedef MapIterator<CachesMap> CachesMapIterator;

		CachesMap mCaches;

    public:
        GLStateCacheManager(void);
        ~GLStateCacheManager(void);

        /**
         * GL state is tracked per context, so call this function to drop all
         * recorded state for a given context before you destroy it.
         */
        void unregisterContext (intptr_t id);

        /**
         * @param id new context to switch to for state tracking
         */
        void switchContext (intptr_t id);
        
        /** Clears all cached values
        */
        void clearCache();
		
		/** Bind an OpenGL buffer of any type.
         @param target The buffer target.
         @param buffer The buffer ID.
         @param force Optional parameter to force an update.
         */
        void bindGLBuffer(GLenum target, GLuint buffer, bool force = false);

		/** Delete an OpenGL buffer of any type.
         @param target The buffer target.
         @param buffer The buffer ID.
         @param force Optional parameter to force an update.
         */
        void deleteGLBuffer(GLenum target, GLuint buffer, bool force = false);

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

        /** Gets the current blend equation setting.
         @return The blend equation.
         */
        GLenum getBlendEquation(void) const;

        /** Sets the current blend equation setting.
         @param eq The blend equation to use.
         */
        void setBlendEquation(GLenum eq);

        /// Set the blend equation for RGB and alpha separately.
        void setBlendEquation(GLenum eqRGB, GLenum eqA);

        /** Sets the blending function.
         @param source The blend mode for the source.
         @param dest The blend mode for the destination
         */
        void setBlendFunc(GLenum source, GLenum dest);

        void setShadeModel(GLenum model);

        void setLightAmbient(GLfloat r, GLfloat g, GLfloat b);

        /** Gets the current depth mask setting.
         @return The current depth mask.
         */
        GLboolean getDepthMask(void) const;

        /** Sets the current depth mask setting.
         @param mask The depth mask to use.
         */
        void setDepthMask(GLboolean mask);

        /** Gets the current depth test function.
         @return The current depth test function.
         */
        GLenum getDepthFunc(void) const;

        /** Sets the current depth test function.
         @param func The depth test function to use.
         */
        void setDepthFunc(GLenum func);

		/** Gets the clear depth in the range from [0..1].
         @return The current clearing depth.
         */
        GLclampf getClearDepth(void) const;

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

        /** Gets the current colour mask setting.
         @return An array containing the mask in RGBA order.
         */
        vector<GLboolean>::type & getColourMask(void) const;

        /** Sets the current colour mask.
         @param red The red component.
         @param green The green component.
         @param blue The blue component.
         @param alpha The alpha component.
         */
        void setColourMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

        /** Gets the current stencil mask.
         @return The stencil mask.
         */
        GLuint getStencilMask(void) const;

        /** Sets the stencil mask.
         @param mask The stencil mask to use
         */
        void setStencilMask(GLuint mask);

        /** Enables a piece of OpenGL functionality.
         @param flag The function to enable.
         */
        void setEnabled(GLenum flag, bool enabled);

        /** Gets the mask of buffers to be discarded if GL_EXT_discard_framebuffer is supported
         @return The buffer mask.
         */
        unsigned int getDiscardBuffers(void) const;

        /** Sets the mask of buffers to be discarded if GL_EXT_discard_framebuffer is supported
         @param flags The bit mask of buffers to be discarded. Stored as Ogre::FrameBufferType.
         */
        void setDiscardBuffers(unsigned int flags);

        /** Gets the current polygon rendering mode, fill, wireframe, points, etc.
         @return The current polygon rendering mode.
         */
        GLenum getPolygonMode(void) const;

        /** Sets the current polygon rendering mode.
         @param mode The polygon mode to use.
         */
        void setPolygonMode(GLenum mode);

        /** Sets the face culling mode.
         @return The current face culling mode
         */
        GLenum getCullFace(void) const;

        /** Sets the face culling setting.
         @param face The face culling mode to use.
         */
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

        void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);
        void getViewport(int* array);
    };
}

#endif
