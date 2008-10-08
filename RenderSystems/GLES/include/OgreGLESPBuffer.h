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

#ifndef __GLESPBuffer_H__
#define __GLESPBuffer_H__

#include "OgreGLESPrerequisites.h"

namespace Ogre {
    class GLContext;

    /** An off-screen rendering context. These contexts are always RGBA for simplicity, speed and
        convience, but the component format is configurable.
    */
    class _OgrePrivate GLESPBuffer
    {
        protected:
            PixelComponentType mFormat;
            size_t mWidth, mHeight;

        public:
            GLESPBuffer(PixelComponentType format, size_t width, size_t height);
            virtual ~GLESPBuffer();

            /** Get the GL context that needs to be active to render to this PBuffer.
            */
            virtual GLESContext *getContext() = 0;

            PixelComponentType getFormat() { return mFormat; }
            size_t getWidth() { return mWidth; }
            size_t getHeight() { return mHeight; }

            /** Get PBuffer component format for an OGRE pixel format.
             */
            static PixelComponentType getPixelComponentType(PixelFormat fmt);
    };
}

#endif
