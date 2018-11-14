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

#ifndef __GLES2StateCacheManager_H__
#define __GLES2StateCacheManager_H__

#include "OgreGLES2Prerequisites.h"
#include "OgreGLStateCacheManagerCommon.h"

namespace Ogre
{
    class _OgreGLES2Export GLES2StateCacheManager : public GLStateCacheManagerCommon
    {
    protected:
        struct TextureUnitParams
        {
            TexParameteriMap mTexParameteriMap;
            TexParameterfMap mTexParameterfMap;
        };

        typedef std::unordered_map<GLuint, TextureUnitParams> TexUnitsMap;

        /// Stores the currently bound vertex array object
        GLuint mActiveVertexArray;
        /// A map of texture parameters for each texture unit
        TexUnitsMap mTexUnitsMap;
        /// Stores the currently enabled vertex attributes
        std::vector<GLuint> mEnabledVertexAttribs;
        /// Stores the last bound texture id
        GLuint mLastBoundTexID;
    public:
        GLES2StateCacheManager(void);

        /** Initialize our cache variables and sets the
            GL states on the current context.
        */
        void initializeCache();
        
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

        /** Sets a float parameter value per texture target.
         @param target The texture target.
         @param pname The parameter name.
         @param param The parameter value.
         */
        void setTexParameterf(GLenum target, GLenum pname, GLfloat param);

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

        /** Sets the current depth test function.
         @param func The depth test function to use.
         */
        void setDepthFunc(GLenum func);

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
        void setEnabled(GLenum flag);

        /** Disables a piece of OpenGL functionality.
         @param flag The function to disable.
         */
        void setDisabled(GLenum flag);

        /** Sets the face culling setting.
         @param face The face culling mode to use.
         */
        void setCullFace(GLenum face);

        void setViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    };
}

#endif
