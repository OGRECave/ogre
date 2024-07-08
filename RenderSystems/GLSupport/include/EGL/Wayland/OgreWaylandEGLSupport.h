// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __WaylandEGLSupport_H__
#define __WaylandEGLSupport_H__

#include <wayland-client.h>
#include <wayland-egl.h> // include before egl

#include "OgreEGLSupport.h"
#include <OgreGLNativeSupport.h>

namespace Ogre
{
class _OgrePrivate WaylandEGLSupport : public EGLSupport
{
public:
    bool mIsExternalDisplay;

protected:
    VideoModes mCbVideoModes;
    EGLWindow* mInitialWindow = nullptr;

    void start() override;

    void doInit();

public:
    WaylandEGLSupport(int profile);
    virtual ~WaylandEGLSupport();

    NativeDisplayType getNativeDisplay(void);
    // This just sets the native variables needed by EGLSupport::getGLDisplay
    // Then it calls EGLSupport::getGLDisplay to do the rest of the work.
    EGLDisplay getGLDisplay();

    RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                            const NameValuePairList* miscParams = 0) override;

};

} // namespace Ogre

#endif
