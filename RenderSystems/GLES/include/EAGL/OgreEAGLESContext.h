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

#ifndef __EAGLESContext_H__
#define __EAGLESContext_H__

#include "OgreGLESContext.h"

#ifdef __OBJC__
#   import <QuartzCore/CAEAGLLayer.h>
#endif

namespace Ogre {
    class EAGLSupport;

    class _OgrePrivate EAGLESContext : public GLESContext
    {
        protected:
#ifdef __OBJC__
            CAEAGLLayer *mDrawable;
            EAGLContext *mContext;
#endif

        public:
#ifdef __OBJC__
            EAGLESContext(CAEAGLLayer *drawable);
            CAEAGLLayer * getDrawable() const;
            EAGLContext * getContext() const;
#endif
            virtual ~EAGLESContext();

            virtual void setCurrent();
            virtual void endCurrent();
            virtual GLESContext * clone() const;

            bool createFramebuffer();
            void destroyFramebuffer();

            /* The pixel dimensions of the backbuffer */
            GLint mBackingWidth;
            GLint mBackingHeight;

            /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
            GLuint mViewRenderbuffer;
            GLuint mViewFramebuffer;

            /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
            GLuint mDepthRenderbuffer;
    };
}

#endif
