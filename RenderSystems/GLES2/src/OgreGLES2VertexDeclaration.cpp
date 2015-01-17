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

#include "OgreGLES2VertexDeclaration.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace v1 {
    //-----------------------------------------------------------------------
    GLES2VertexDeclaration::GLES2VertexDeclaration()
        :
        mVAO(0),
        mIsInitialised(false)
    {
#if OGRE_NO_GLES2_VAO_SUPPORT == 0
#   if defined(GL_OES_vertex_array_object) || (OGRE_NO_GLES3_SUPPORT == 0)
        OGRE_CHECK_GL_ERROR(glGenVertexArraysOES(1, &mVAO));
//        LogManager::getSingleton().logMessage("Created VAO " + StringConverter::toString(mVAO));

        if (!mVAO)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                    "Cannot create GL ES Vertex Array Object",
                    "GLES2VertexDeclaration::GLES2VertexDeclaration");
        }
#   endif
#endif
    }

    //-----------------------------------------------------------------------
    GLES2VertexDeclaration::~GLES2VertexDeclaration()
    {
#if OGRE_NO_GLES2_VAO_SUPPORT == 0
#   if defined(GL_OES_vertex_array_object) || (OGRE_NO_GLES3_SUPPORT == 0)
//        LogManager::getSingleton().logMessage("Deleting VAO " + StringConverter::toString(mVAO));
        OGRE_CHECK_GL_ERROR(glDeleteVertexArraysOES(1, &mVAO));
#   endif
#endif
    }

    //-----------------------------------------------------------------------
    void GLES2VertexDeclaration::bind(void)
    {
#if OGRE_NO_GLES2_VAO_SUPPORT == 0
#   if defined(GL_OES_vertex_array_object) || (OGRE_NO_GLES3_SUPPORT == 0)
//        LogManager::getSingleton().logMessage("Binding VAO " + StringConverter::toString(mVAO));
        OGRE_CHECK_GL_ERROR(glBindVertexArrayOES(mVAO));
#   endif
#endif
    }
}
}
