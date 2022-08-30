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
                    bool fullScreen, const NameValuePairList *miscParams) override;

        void destroy(void) override {}

        void resize(unsigned int width, unsigned int height) override;

        void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer) override;
        bool requiresTextureFlipping() const override { return true; }

        Image* getImage() { return &mBuffer; }

        void swapBuffers() override;

    protected:
        Image mBuffer;
        SDL_Window* mParentWindow;
    };
}

#endif
