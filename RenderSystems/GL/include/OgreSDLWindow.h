/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

