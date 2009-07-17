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

#include "OgreGLRenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLXContext.h"
#include "OgreGLXUtils.h"
#include "OgreGLXGLSupport.h"

namespace Ogre 
{
	GLXContext::GLXContext(GLXGLSupport* glsupport, ::GLXFBConfig fbconfig, ::GLXDrawable drawable, ::GLXContext context) :
		mGLSupport(glsupport), mDrawable(drawable), mContext(0), mFBConfig(fbconfig), mExternalContext(false)
	{
		GLRenderSystem *renderSystem = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
		GLXContext* mainContext = static_cast<GLXContext*>(renderSystem->_getMainContext());
		::GLXContext shareContext = 0;
		
		if (mainContext)
		{
			shareContext = mainContext->mContext;
		}
		
		if (context)
		{
			mContext = context;
			mExternalContext = true;
		}
		else
		{
			mContext = mGLSupport->createNewContext(mFBConfig, GLX_RGBA_TYPE, shareContext, GL_TRUE);
		}

		if (! mContext)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to create a suitable GLXContext", "GLXContext::GLXContext");
		}
	}
	
	GLXContext::~GLXContext() 	
	{
		GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
		
		if (!mExternalContext)
			glXDestroyContext(mGLSupport->getGLDisplay(), mContext);
		
		rs->_unregisterContext(this);
	}
	
	void GLXContext::setCurrent() 
	{
		glXMakeCurrent(mGLSupport->getGLDisplay(), mDrawable, mContext);
	}
	
	void GLXContext::endCurrent() 
	{
		glXMakeCurrent(mGLSupport->getGLDisplay(), None, None);
	}
	
	GLContext* GLXContext::clone() const
	{
		return new GLXContext(mGLSupport, mFBConfig, mDrawable);
	} 
}
