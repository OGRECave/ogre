// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __WaylandEGLWindow_H__
#define __WaylandEGLWindow_H__

#include "OgreEGLWindow.h"
#include "OgreWaylandEGLSupport.h"

namespace Ogre
{
class _OgrePrivate WaylandEGLWindow : public EGLWindow
{
protected:
    WaylandEGLSupport* mGLSupport;

    void initNativeCreatedWindow(const NameValuePairList* miscParams);
    void createNativeWindow(uint& width, uint& height, String& title);
    void resize(unsigned int width, unsigned int height) override;
    void windowMovedOrResized() override;

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

    void create(const String& name, unsigned int width, unsigned int height, bool fullScreen,
                const NameValuePairList* miscParams) override;
};
} // namespace Ogre

#endif
