/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#ifndef __OSXWindow_H__
#define __OSXWindow_H__

#include <Carbon/Carbon.h>
#include "OgreRenderWindow.h"
#include "OgreOSXContext.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>

namespace Ogre 
{
	class OSXWindow : public RenderWindow
	{
	public:
		OSXWindow();
		virtual ~OSXWindow();
		
		/** Overridden - see RenderWindow */
		void create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams ) = 0;
        /** Overridden - see RenderWindow */
        virtual void destroy( void ) = 0;
        /** Overridden - see RenderWindow */
        virtual bool isActive( void ) const = 0;
        /** Overridden - see RenderWindow */
        virtual bool isClosed( void ) const = 0;
        /** Overridden - see RenderWindow */
        virtual void reposition( int left, int top ) = 0;
        /** Overridden - see RenderWindow */
        virtual void resize( unsigned int width, unsigned int height ) = 0;
        /** Overridden - see RenderWindow */
        virtual void swapBuffers( bool waitForVSync ) = 0;
        /** Overridden - see RenderTarget */
        virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		/** Overridden - see RenderTarget */
		virtual void windowMovedOrResized() {};

	protected:
		OSXContext* mContext;
		CGLContextObj mCGLContext;
		/** Switch to full screen mode using CGL */
		void createCGLFullscreen(unsigned int width, unsigned int height, unsigned int depth, unsigned int fsaa, CGLContextObj sharedContext);
		/** Kill full screen mode, and return to default windowed mode */
		void destroyCGLFullscreen(void);
		/** Update the full screen context */
		void swapCGLBuffers(void);
	};
}

#endif
