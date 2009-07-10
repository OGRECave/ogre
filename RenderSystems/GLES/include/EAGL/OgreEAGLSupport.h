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

#ifndef __EAGLSupport_H__
#define __EAGLSupport_H__

#include "OgreGLESSupport.h"

#ifdef __OBJC__
// Forward declarations
@class CAEAGLLayer;
@class UIWindow;
typedef UIWindow *NativeWindowType;
#endif

namespace Ogre {
    class EAGLWindow;
    class EAGLESContext;

	template<class C> void removeDuplicates(C& c)
	{
		std::sort(c.begin(), c.end());
		typename C::iterator p = std::unique(c.begin(), c.end());
		c.erase(p, c.end());
	}

    class _OgrePrivate EAGLSupport : public GLESSupport
    {
        protected:

            typedef std::pair<uint, uint> ScreenSize;
            typedef short Rate;
            typedef std::pair<ScreenSize, Rate> VideoMode;
            typedef std::vector<VideoMode> VideoModes;
            VideoModes mVideoModes;
            VideoMode mOriginalMode;
            VideoMode mCurrentMode;

        public:
            EAGLSupport();
            virtual ~EAGLSupport();

            void start(void);
            void stop(void);
            void addConfig(void);
            String validateConfig(void);
            virtual String getDisplayName(void);
			CFDictionaryRef chooseGLConfig(const GLint *attribList, GLint *nElements);
            GLint getGLConfigAttrib(CFDictionaryRef fbConfig, GLint attribute, GLint *value);
            void * getProcAddress(const Ogre::String& name);
            RenderWindow * createWindow(bool autoCreateWindow,
                                           GLESRenderSystem *renderSystem,
                                           const String& windowTitle);

            RenderWindow * newWindow(const String& name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams = 0);
            virtual GLESPBuffer * createPBuffer(PixelComponentType format,
                                                   size_t width, size_t height);

#ifdef __OBJC__
            EAGLESContext * createNewContext(CFDictionaryRef &glconfig, CAEAGLLayer *drawable) const;
            CFDictionaryRef getGLConfigFromContext(EAGLESContext context);
            CFDictionaryRef getGLConfigFromDrawable(CAEAGLLayer *drawable, unsigned int *w, unsigned int *h);
#endif
			CFDictionaryRef selectGLConfig(const int* minAttribs, const int *maxAttribs);
            void switchMode(void);
	};
}

#endif
