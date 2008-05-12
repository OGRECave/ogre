/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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


