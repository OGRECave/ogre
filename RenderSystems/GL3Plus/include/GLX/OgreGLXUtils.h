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
#ifndef __GLXUtils_H__
#define __GLXUtils_H__

#include "OgrePrerequisites.h"
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>

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

}

#endif
