/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESRenderSystem.h"

#include "OgreEGLSupport.h"
#include "OgreEGLContext.h"

#include "OgreRoot.h"

namespace Ogre {
    EGLContext::EGLContext(EGLSupport* glsupport,
                           ::EGLConfig glconfig,
                           ::EGLSurface drawable)
        : mGLSupport(glsupport),
          mDrawable(drawable),
          mContext(0),
          mConfig(glconfig)
    {
        GLESRenderSystem* renderSystem =
            static_cast<GLESRenderSystem*>(Root::getSingleton().getRenderSystem());
        EGLContext* mainContext =
            static_cast<EGLContext*>(renderSystem->_getMainContext());
        ::EGLContext shareContext = (::EGLContext) 0;

        if (mainContext)
        {
            shareContext = mainContext->mContext;
        }

        mContext = mGLSupport->createNewContext(mConfig, shareContext);

        if (!mContext)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unable to create a suitable EGLContext",
                        "EGLContext::EGLContext");
        }
    }

    EGLContext::~EGLContext()
    {
        GLESRenderSystem *rs =
            static_cast<GLESRenderSystem*>(Root::getSingleton().getRenderSystem());

        eglDestroyContext(mGLSupport->getGLDisplay(), mContext);
        rs->_unregisterContext(this);
    }

    void EGLContext::setCurrent()
    {
        EGLBoolean ret = eglMakeCurrent(mGLSupport->getGLDisplay(),
                                        mDrawable, mDrawable, mContext);
        if (!ret)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to make context current",
                        __FUNCTION__);
        }
    }

    void EGLContext::endCurrent()
    {
        eglMakeCurrent(mGLSupport->getGLDisplay(), None, None, None);
    }

    GLESContext* EGLContext::clone() const
    {
        return new EGLContext(mGLSupport, mConfig, mDrawable);
    }
}
