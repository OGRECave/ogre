/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

namespace Ogre {

//------------------------------------------------------------------
/**
  * Default object constructor
  * 
  */
GLES2HardwareOcclusionQuery::GLES2HardwareOcclusionQuery() 
{ 
	// Check for hardware occlusion support
#ifdef GL_EXT_occlusion_query_boolean
    glGenQueriesEXT(1, &mQueryID );
    GL_CHECK_ERROR;
#else
    OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Cannot allocate a Hardware query. This video card doesn't support it, sorry.", 
                "GLES2HardwareOcclusionQuery::GLES2HardwareOcclusionQuery" );
#endif
}
//------------------------------------------------------------------
/**
  * Object destructor
  */
GLES2HardwareOcclusionQuery::~GLES2HardwareOcclusionQuery() 
{ 
#ifdef GL_EXT_occlusion_query_boolean
    glDeleteQueriesEXT(1, &mQueryID);
    GL_CHECK_ERROR;
#endif
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::beginOcclusionQuery() 
{ 
#ifdef GL_EXT_occlusion_query_boolean
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, mQueryID);
    GL_CHECK_ERROR;
#endif
}
//------------------------------------------------------------------
void GLES2HardwareOcclusionQuery::endOcclusionQuery() 
{ 
#ifdef GL_EXT_occlusion_query_boolean
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    GL_CHECK_ERROR;
#endif
}
//------------------------------------------------------------------
bool GLES2HardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
{
#ifdef GL_EXT_occlusion_query_boolean
    glGetQueryObjectuivEXT(mQueryID, GL_QUERY_RESULT_EXT, (GLuint*)NumOfFragments);
    GL_CHECK_ERROR;
    mPixelCount = *NumOfFragments;
    return true;
#else
	return false;
#endif
}
//------------------------------------------------------------------
bool GLES2HardwareOcclusionQuery::isStillOutstanding(void)
{    
    GLuint available = GL_FALSE;

#ifdef GL_EXT_occlusion_query_boolean
    glGetQueryObjectuivEXT(mQueryID, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
    GL_CHECK_ERROR;
#endif

	// GL_TRUE means a wait would occur
    return !(available == GL_TRUE);  
} 

}


