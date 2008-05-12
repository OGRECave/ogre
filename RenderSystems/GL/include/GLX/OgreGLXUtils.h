/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __GLXUtils_H__
#define __GLXUtils_H__

#include "OgrePrerequisites.h"
#include <GL/glew.h>
#include <GL/glxew.h>

namespace Ogre 
{
	class _OgrePrivate GLXUtils
	{
	public:
		// Portable replacements for some GLX 1.3 function pointers
		
		static PFNGLXCHOOSEFBCONFIGPROC chooseFBConfig;
		static PFNGLXCREATENEWCONTEXTPROC createNewContext;
		static PFNGLXGETFBCONFIGATTRIBPROC getFBConfigAttrib;
		static PFNGLXGETVISUALFROMFBCONFIGPROC getVisualFromFBConfig;
		
		/**
		 * Get the GLXFBConfig used to create a ::GLXContext
		 *
		 * @param display	X Display
		 * @param drawable   GLXContext 
		 * @returns		  GLXFBConfig used to create the context
		 */
		static GLXFBConfig getFBConfigFromContext (Display *display, ::GLXContext context);
		
		/**
		 * Get the GLXFBConfig used to create a GLXDrawable.
		 * Caveat: GLX version 1.3 is needed when the drawable is a GLXPixmap
		 *
		 * @param display	X Display
		 * @param drawable   GLXDrawable 
		 * @param width	  Receiver for the drawable width
		 * @param height	 Receiver for the drawable height
		 * @returns		  GLXFBConfig used to create the drawable
		 */
		static GLXFBConfig getFBConfigFromDrawable (Display *display, GLXDrawable drawable, 
								unsigned int *width, unsigned int *height);
		
		/**
		 * Initialise the parts of GLXEW needed to create a GL Context
		 *
		 * @param display	X Display
		 */
		static void initialiseGLXEW (Display *display);
		
		/**
		 * Select an FBConfig given a list of required and a list of desired properties
		 *
		 * @param display	X Display
		 * @param minAttribs FBConfig attributes that must be provided with minimum values
		 * @param maxAttribs FBConfig attributes that are preferred with maximum values
		 * @returns		  GLXFBConfig with attributes or 0 when unsupported. 
		 */
		static GLXFBConfig selectFBConfig(Display *display, const int *minAttribs, const int *maxAttribs);
		
		/**
		 * Loads an icon from an Ogre resource into the X Server. This currently only
		 * works for 24 and 32 bit displays. The image must be findable by the Ogre
		 * resource system, and of format PF_A8R8G8B8.
		 *
		 * @param display	X display
		 * @param name	   Name of image to load
		 * @param pix		Receiver for the output pixmap
		 * @param mask	   Receiver for the output mask (alpha bitmap)
		 * @returns true on success
		 */		
		static bool loadIcon(Display *display, const std::string &name, Pixmap *pix, Pixmap *mask);
	};
   
};

#endif
