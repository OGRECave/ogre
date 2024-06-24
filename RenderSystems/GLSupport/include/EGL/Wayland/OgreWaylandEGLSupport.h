// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __WaylandEGLSupport_H__
#define __WaylandEGLSupport_H__

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h> // include before egl

#include "OgreEGLSupport.h"
#include <OgreGLNativeSupport.h>

namespace Ogre
{
class _OgrePrivate WaylandEGLSupport : public EGLSupport
{
public:
    wl_surface* mWlSurface;
    wl_compositor* mWlCompositor;
    bool mIsExternalDisplay;

protected:
    VideoModes mCbVideoModes;

public:
    WaylandEGLSupport(int profile);
    virtual ~WaylandEGLSupport();

    inline void setNativeDisplay(NativeDisplayType t)
    {
        mIsExternalDisplay = true;
        mNativeDisplay = t;
    }
    NativeDisplayType getNativeDisplay(void);
    // This just sets the native variables needed by EGLSupport::getGLDisplay
    // Then it calls EGLSupport::getGLDisplay to do the rest of the work.
    EGLDisplay getGLDisplay();
    void doInit();

    RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                            const NameValuePairList* miscParams = 0) override;

    static void globalRegistryHandler(void* data, wl_registry* registry, uint32_t id, const char* interface,
                                      uint32_t version);

    static void globalRegistryRemover(void* data, wl_registry* registry, uint32_t id);

    static const wl_registry_listener mRegistryListener;
};

} // namespace Ogre

#endif
