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
    glGenQueriesARB(1, &mQueryID );
}
//------------------------------------------------------------------
/**
  * Object destructor
  */
GLHardwareOcclusionQuery::~GLHardwareOcclusionQuery() 
{ 
    glDeleteQueriesARB(1, &mQueryID);
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::beginOcclusionQuery() 
{ 
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, mQueryID);
}
//------------------------------------------------------------------
void GLHardwareOcclusionQuery::endOcclusionQuery() 
{ 
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::pullOcclusionQuery( unsigned int* NumOfFragments ) 
{
    glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_ARB, (GLuint*)NumOfFragments);
    mPixelCount = *NumOfFragments;
    return true;
}
//------------------------------------------------------------------
bool GLHardwareOcclusionQuery::isStillOutstanding(void)
{    
    GLuint available = GL_FALSE;

    glGetQueryObjectuivARB(mQueryID, GL_QUERY_RESULT_AVAILABLE_ARB, &available);

    // GL_TRUE means a wait would occur
    return !(available == GL_TRUE);  
} 

}


