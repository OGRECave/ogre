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

/*
The nVidia occlusion query extension is defined in glext.h so you don't 
need anything else. You do need to look up the function, we provide a 
GLSupport class to do this, which has platform implementations for 
getProcAddress. Check the way that extensions like glActiveTextureARB are 
initialised and used in glRenderSystem and copy what is done there.



  To do: fix so dx7 and DX9 checks and flags if HW Occlusion is supported
  See the openGl dito for ideas what to do.

*/



//GL_ActiveTextureARB_Func* glActiveTextureARB_ptr = (GL_ActiveTextureARB_Func)mGLSupport->getProcAddress("glActiveTextureARB");

#ifndef __GLHARDWAREOCCLUSIONQUERY_H__
#define __GLHARDWAREOCCLUSIONQUERY_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwareOcclusionQuery.h"


namespace Ogre { 


// If you use multiple rendering passes you can test only the first pass and all other passes don't have to be rendered 
// if the first pass result has too few pixels visible.

// Be sure to render all occluder first and whats out so the RenderQue don't switch places on 
// the occluding objects and the tested objects because it thinks it's more effective..


/**
  * This is a class that is the base class of the query class for 
  * hardware occlusion.
  *
  * @author Lee Sandberg email: lee@abcmedia.se
  * Updated on 13/9/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
  */

class _OgrePrivate GLHardwareOcclusionQuery : public HardwareOcclusionQuery
{
//----------------------------------------------------------------------
// Public methods
//--
public:
	/**
	  * Default object constructor
	  * 
	  */
	GLHardwareOcclusionQuery();
	/**
	  * Object destructor
	  */
	~GLHardwareOcclusionQuery();

	//------------------------------------------------------------------
	// Occlusion query functions (see base class documentation for this)
	//--
	void beginOcclusionQuery();
	void endOcclusionQuery();
	bool pullOcclusionQuery( unsigned int* NumOfFragments); 
	bool isStillOutstanding(void);


    //----------------------------------------------------------------------
    // private members
    //--
    private:

	    GLuint			mQueryID;
};

}

#endif 

