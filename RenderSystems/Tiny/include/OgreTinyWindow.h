// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __GLWindow_H__
#define __GLWindow_H__

#include "OgreRenderWindow.h"
#include "OgreImage.h"

struct SDL_Window;

namespace Ogre
{
    class _OgrePrivate TinyWindow : public RenderWindow
    {
    public:
        TinyWindow();

        void create(const String& name, unsigned int width, unsigned int height,
                    bool fullScreen, const NameValuePairList *miscParams);

        void destroy(void) {}

        void resize(unsigned int width, unsigned int height);

        void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer);
        bool requiresTextureFlipping() const { return true; }

        Image* getImage() { return &mBuffer; }

        void swapBuffers();

    protected:
        Image mBuffer;
        SDL_Window* mParentWindow;
    };
}

#endif
