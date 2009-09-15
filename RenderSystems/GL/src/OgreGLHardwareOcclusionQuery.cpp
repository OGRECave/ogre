/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLHardwareOcclusionQuery.h"
#include "OgreException.h"

namespace Ogre {

/**
  * This is a class that is the base class of the query class for 
  * hardware occlusion testing.
  *
  * @author Lee Sandberg email: lee@abcmedia.se
  *
  * Updated on 12/7/2004 by Chris McGuirk
  * - Implemented ARB_occlusion_query
  * Updated on 13/9/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
  */
//------------------------------------------------------------------
/**
  * Default object constructor
  * 
  */
GLHardwareOcclusionQuery::GLHardwareOcclusionQuery() 
{ 
	// Check for hardware occlusion support
    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
	    glGenQueriesARB(1, &mQueryID );	
	}
	else if (GLEW_NV_occlusion_query)
	{
		glGenOcclusionQueriesNV(1, &mQueryID);
	}
	else
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                    "Cannot allocate a Hardware query. This video card doesn't supports it, sorry.", 
                    "GLHardwareOcclusionQuery::GLHardwareOcclusionQuery" );
    }
	
}
//------------------------------------------------------------------
/**
  * Object destructor
  */
GLHardwareOcclusionQuery::~GLHardwareOcclusionQuery() 
{ 
    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
		glDeleteQueriesARB(1, &mQueryID);  
	}
	else if (GLEW_NV_occlusion_query)
	{
		glDeleteOcclusionQueriesNV(1, &mQueryID);  
	}
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::beginOcclusionQuery() 
{ 
    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
		glBeginQueryARB(GL_SAMPLES_PASSED_ARB, mQueryID);
	}
	else if (GLEW_NV_occlusion_query)
	{
		glBeginOcclusionQueryNV(mQueryID);
	}
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::endOcclusionQuery() 
{ 
    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
	    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	}
	else if (GLEW_NV_occlusion_query)
	{
		glEndOcclusionQueryNV();
	}
	
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
{
    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
		glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_ARB, (GLuint*)NumOfFragments);
		mPixelCount = *NumOfFragments;
		return true;
	}
	else if (GLEW_NV_occlusion_query)
	{
		glGetOcclusionQueryuivNV(mQueryID, GL_PIXEL_COUNT_NV, (GLuint*)NumOfFragments);
		mPixelCount = *NumOfFragments;
		return true;
	}

	return false;
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::isStillOutstanding(void)
{    
      GLuint available;

    if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
	{
	    glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_AVAILABLE_ARB, &available);
	}
	else if (GLEW_NV_occlusion_query)
	{
	    glGetOcclusionQueryuivNV(mQueryID, GL_PIXEL_COUNT_AVAILABLE_NV, &available);
	}

	// GL_TRUE means a wait would occur
    return !(available == GL_TRUE);  
} 

}


