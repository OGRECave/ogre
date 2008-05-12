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

#ifndef __SDLWindow_H__
#define __SDLWindow_H__

#include "OgreSDLPrerequisites.h"
#include "OgreRenderWindow.h"

namespace Ogre {
    class _OgrePrivate SDLWindow : public RenderWindow
    {
    private:
        SDL_Surface* mScreen;
        bool mActive;
        bool mClosed;
            
        // Process pending events
        void processEvents(void);

        int (*glXGetVideoSyncSGI) (unsigned int *);
        int (*glXWaitVideoSyncSGI) (int, int, unsigned int *);

    public:
        SDLWindow();
        ~SDLWindow();

		void create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams);
        /** Overridden - see RenderWindow */
        void destroy(void);
        /** Overridden - see RenderWindow */
        bool isActive(void) const;
        /** Overridden - see RenderWindow */
        bool isClosed(void) const;
        /** Overridden - see RenderWindow */
        void reposition(int left, int top);
        /** Overridden - see RenderWindow */
        void resize(unsigned int width, unsigned int height);
        /** Overridden - see RenderWindow */
        void swapBuffers(bool waitForVSync);

        /** Overridden - see RenderTarget. */
        void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
        
        /** Overridden - see RenderTarget.
        */
        void getCustomAttribute( const String& name, void* pData )
        {
            // NOOP
        }

        bool requiresTextureFlipping() const { return false; }

        bool isFullScreen(void) const
        {
            return ( mScreen->flags & SDL_FULLSCREEN ) == SDL_FULLSCREEN;
        }
    };
}

#endif

