/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreGLES2HardwareOcclusionQuery.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLUtil.h"

namespace Ogre {

//------------------------------------------------------------------
/**
  * Default object constructor
  * 
  */
GLES2HardwareOcclusionQuery::GLES2HardwareOcclusionQuery() 
{ 
    createQuery();
}
//------------------------------------------------------------------
/**
  * Object destructor
  */
GLES2HardwareOcclusionQuery::~GLES2HardwareOcclusionQuery() 
{ 
    destroyQuery();
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::createQuery()
{
    OGRE_CHECK_GL_ERROR(glGenQueriesEXT(1, &mQueryID));
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::destroyQuery()
{
    OGRE_CHECK_GL_ERROR(glDeleteQueriesEXT(1, &mQueryID));
}
//------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
void GLES2HardwareOcclusionQuery::notifyOnContextLost()
{
    destroyQuery();
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::notifyOnContextReset()
{
    createQuery();
}
#endif
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::beginOcclusionQuery() 
{
    OGRE_CHECK_GL_ERROR(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, mQueryID));
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::endOcclusionQuery() 
{
    OGRE_CHECK_GL_ERROR(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
}
//------------------------------------------------------------------
bool GLES2HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
{
    OGRE_CHECK_GL_ERROR(glGetQueryObjectuivEXT(mQueryID, GL_QUERY_RESULT_EXT, (GLuint*)NumOfFragments));
    mPixelCount = *NumOfFragments;
    return true;
}
//------------------------------------------------------------------
bool GLES2HardwareOcclusionQuery::isStillOutstanding(void)
{    
    GLuint available = GL_FALSE;

    OGRE_CHECK_GL_ERROR(glGetQueryObjectuivEXT(mQueryID, GL_QUERY_RESULT_AVAILABLE_EXT, &available));

    // GL_TRUE means a wait would occur
    return !(available == GL_TRUE);
} 

}
