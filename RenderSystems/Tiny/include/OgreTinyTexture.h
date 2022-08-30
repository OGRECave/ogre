// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TinyTexture_H__
#define __TinyTexture_H__

#include "OgreTexture.h"

namespace Ogre {
    class TinyTexture : public Texture
    {
    public:
        // Constructor
        TinyTexture(ResourceManager* creator, const String& name, ResourceHandle handle,
                       const String& group, bool isManual, ManualResourceLoader* loader);

        Image* getImage() { return &mBuffer; }

        virtual ~TinyTexture();

    protected:
        Image mBuffer;
        void createInternalResourcesImpl(void) override;
        void freeInternalResourcesImpl(void) override {}
    };
}

#endif
