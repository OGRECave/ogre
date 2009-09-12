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

#ifndef __OSXCarbonWindow_H__
#define __OSXCarbonWindow_H__

#include "OgreOSXWindow.h"
#include "OgreOSXCarbonContext.h"
#include "OgreOSXCGLContext.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>

namespace Ogre 
{
	class OSXCarbonWindow : public OSXWindow
	{
	public:
		OSXCarbonWindow();
		virtual ~OSXCarbonWindow();
		
		virtual void create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams );
        /** Overridden - see RenderWindow */
		virtual void destroy( void );
        /** Overridden - see RenderWindow */
		virtual bool isActive( void ) const;
        /** Overridden - see RenderWindow */
		virtual bool isClosed( void ) const;
        /** Overridden - see RenderWindow */
		virtual void reposition( int left, int top );
        /** Overridden - see RenderWindow */
		virtual void resize( unsigned int width, unsigned int height );
        /** Overridden - see RenderWindow */
    	virtual void setVisible( bool visible );
        /** Overridden - see RenderWindow */
    	virtual bool isVisible(void) const;
        /** Overridden - see RenderWindow */
		virtual void swapBuffers( bool waitForVSync );
        /** Overridden - see RenderWindow */
        virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
   	    /** Overridden - see RenderTarget */
		virtual void windowMovedOrResized();

		bool requiresTextureFlipping() const { return false; }
		
		void windowResized();
		void windowHasResized();
		
		void getCustomAttribute( const String& name, void* pData );

	private:
		void processEvents();
		
	private:
		WindowRef mWindow;
        EventHandlerRef mEventHandlerRef;
		HIViewRef mView;
		AGLContext mAGLContext;
		
    bool mActive;
    bool mClosed;
    bool mCreated;
    bool mHasResized;
    bool mIsExternal;
    bool mVisible;
	};
}

#endif
