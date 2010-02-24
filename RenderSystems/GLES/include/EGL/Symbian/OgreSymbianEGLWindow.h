/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __SymbianEGLWindow_H__
#define __SymbianEGLWindow_H__

#include <coecntrl.h>
#include "OgreEGLWindow.h"

namespace Ogre {
    class _OgrePrivate SymbianEGLWindow : public EGLWindow
    {
	protected:
		CCoeControl *  mNativeControl;

		virtual EGLContext * createEGLContext() const;
		virtual void getLeftAndTopFromNativeWindow(int & left, int & top, uint width, uint height);
		virtual void initNativeCreatedWindow(const NameValuePairList *miscParams);
		virtual void createNativeWindow( int &left, int &top, uint &width, uint &height, String &title );
		virtual void reposition(int left, int top);
		virtual void resize(unsigned int width, unsigned int height);
		virtual void windowMovedOrResized();
		virtual void switchFullScreen(bool fullscreen);

    public:
		SymbianEGLWindow(EGLSupport* glsupport);
		virtual ~SymbianEGLWindow();

		void create(const String& name, unsigned int width, unsigned int height,
		                        bool fullScreen, const NameValuePairList *miscParams);

		void getCustomAttribute( const String& name, void *pData );


    };
}

#endif
