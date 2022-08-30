// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_
#define COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_

#include "OgreBitesPrerequisites.h"
#include "OgreInput.h"

namespace OgreBites
{
    /** \addtogroup Optional
    *   \addtogroup Bites
    *   \addtogroup Input
    */
    struct _OgreBitesExport ImGuiInputListener : public InputListener
    {
        ImGuiInputListener();
        bool keyPressed(const KeyboardEvent& evt) override;
        bool keyReleased(const KeyboardEvent& evt) override;
        bool mouseMoved(const MouseMotionEvent& evt) override;
        bool mouseWheelRolled(const MouseWheelEvent& evt) override;
        bool mousePressed(const MouseButtonEvent& evt) override;
        bool mouseReleased(const MouseButtonEvent& evt) override;
        bool textInput (const TextInputEvent& evt) override;
        bool buttonPressed(const ButtonEvent& evt) override;
        bool buttonReleased(const ButtonEvent& evt) override;
    };
}

#endif /* COMPONENTS_BITES_INCLUDE_OGREIMGUIINPUTLISTENER_H_ */
