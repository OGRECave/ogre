// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
#ifndef __TinyPlugin_H__
#define __TinyPlugin_H__

#include "OgrePlugin.h"

namespace Ogre
{
/** Plugin instance for Tiny Manager */
class TinyPlugin : public Plugin
{
public:
    TinyPlugin();

    const String& getName() const;

    void install();
    void initialise();
    void shutdown();
    void uninstall();

protected:
    RenderSystem* mRenderSystem;
};
} // namespace Ogre

#endif
