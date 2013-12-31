/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#ifndef __WIN32EGLSupport_H__
#define __WIN32EGLSupport_H__


#include "OgreEGLSupport.h"

namespace Ogre {
    class _OgrePrivate Win32EGLSupport : public EGLSupport
    {
	protected:

		//Removed createEGLWindow because it was easier to just call
		//new Win32EGLWindow in the code to get the native version
		//virtual EGLWindow* createEGLWindow( EGLSupport * support);

        public:
            Win32EGLSupport();
            virtual ~Win32EGLSupport();

			virtual GLESPBuffer* createPBuffer(PixelComponentType format,
				size_t width, size_t height);
			virtual void switchMode(uint& width, uint& height, short& frequency);

            RenderWindow* newWindow(const String& name,
                                    unsigned int width, unsigned int height,
                                    bool fullScreen,
                                    const NameValuePairList *miscParams = 0);

			//Moved getNativeDisplay to native class
			NativeDisplayType getNativeDisplay();

			//This sets up the native variables then calls EGLSupport::getGLDisplay
			EGLDisplay getGLDisplay();

	};
}

#endif
