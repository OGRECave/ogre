// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __WaylandEGLWindow_H__
#define __WaylandEGLWindow_H__

#include "OgreEGLWindow.h"
#include "OgreWaylandEGLSupport.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include "xdg-shell-client-protocol.h"
#pragma GCC diagnostic pop

namespace Ogre
{
class _OgrePrivate WaylandEGLWindow : public EGLWindow
{
protected:
    WaylandEGLSupport* mGLSupport;

    xdg_surface *mXdgSurface;
    xdg_toplevel *mXdgToplevel;
    void initNativeCreatedWindow(const NameValuePairList* miscParams);
    void createNativeWindow(uint& width, uint& height, String& title);
    void resize(unsigned int width, unsigned int height) override;
    void windowMovedOrResized() override;
    void switchFullScreen(bool fullscreen) override;

public:
    WaylandEGLWindow(WaylandEGLSupport* glsupport);
    virtual ~WaylandEGLWindow();

    /**

       Get custom attribute

       The following attributes are valid:
       | Attribute | Description |
       |---|---|
       | TBD | TBD |

       @param[in] name Attribute name
       @param[out] pData The requested attribute

    */
    void getCustomAttribute(const String& name, void* pData) override;

    void setFullscreen(bool fullscreen, uint width, uint height) override;

    void create(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                const NameValuePairList* miscParams) override;
};
} // namespace Ogre

#endif
