// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreBitesConfigDialog.h"
#include "OgreConfigDialogImp.h"

namespace OgreBites {
    Ogre::ConfigDialog* getNativeConfigDialog() {
#if defined(HAVE_XAW) || (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined(__MINGW32__)) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        static ConfigDialog dialog;
        return &dialog;
#else
        return NULL;
#endif
    }
} /* namespace OgreBites */
