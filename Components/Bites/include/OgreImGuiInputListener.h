// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_
#define COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_

#include "OgreBitesPrerequisites.h"
#include "OgreInput.h"

namespace OgreBites
{
    struct _OgreBitesExport ImGuiInputListener : public InputListener
    {
        ImGuiInputListener();
        bool keyPressed(const KeyboardEvent& evt);
        bool keyReleased(const KeyboardEvent& evt);
        bool mouseMoved(const MouseMotionEvent& evt);
        bool mouseWheelRolled(const MouseWheelEvent& evt);
        bool mousePressed(const MouseButtonEvent& evt);
        bool mouseReleased(const MouseButtonEvent& evt);
        bool textInput (const TextInputEvent& evt);
    private:
        bool keyEvent (const KeyboardEvent& arg);
    };
}

#endif /* COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_ */
