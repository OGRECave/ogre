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

#include "OgreGLFBOMultiRenderTarget.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {

	GLFBOMultiRenderTarget::GLFBOMultiRenderTarget(GLFBOManager *manager, const String &name):
		MultiRenderTarget(name),
		fbo(manager, 0 /* TODO: multisampling on MRTs? */)
	{
	}
	GLFBOMultiRenderTarget::~GLFBOMultiRenderTarget()
	{
	}


	void GLFBOMultiRenderTarget::bindSurfaceImpl(size_t attachment, RenderTexture *target)

	{

		/// Check if the render target is in the rendertarget->FBO map
        GLFrameBufferObject *fbobj = 0;
        target->getCustomAttribute("FBO", &fbobj);
		assert(fbobj);
		fbo.bindSurface(attachment, fbobj->getSurface(0));



		// Initialise?

		

		// Set width and height

		mWidth = fbo.getWidth();

		mHeight = fbo.getHeight();

	}



	void GLFBOMultiRenderTarget::unbindSurfaceImpl(size_t attachment)
	{
		fbo.unbindSurface(attachment);

		// Set width and height

		mWidth = fbo.getWidth();

		mHeight = fbo.getHeight();
	}

	void GLFBOMultiRenderTarget::getCustomAttribute( const String& name, void *pData )
	{
		if(name=="FBO")
        {
            *static_cast<GLFrameBufferObject **>(pData) = &fbo;
        }
	}
}
