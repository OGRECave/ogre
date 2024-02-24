// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreBitesConfigDialog.h"
#include "OgreConfigDialogImp.h"

namespace OgreBites {
    Ogre::ConfigDialog* getNativeConfigDialog() {
#ifndef DISABLE_NATIVE_DIALOG
        static ConfigDialog dialog;
        return &dialog;
#else
        return NULL;
#endif
    }
} /* namespace OgreBites */
