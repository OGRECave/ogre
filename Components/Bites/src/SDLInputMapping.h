// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef BITES_SDL_INPUTMAPPING
#define BITES_SDL_INPUTMAPPING

namespace {
    OgreBites::Event convert(const SDL_Event& in)
    {
        OgreBites::Event out;

        out.type = 0;

        switch(in.type)
        {
        case SDL_KEYDOWN:
            out.type = OgreBites::KEYDOWN;
            OGRE_FALLTHROUGH;
        case SDL_KEYUP:
            if(!out.type)
                out.type = OgreBites::KEYUP;
            out.key.repeat = in.key.repeat;
            out.key.keysym.sym = in.key.keysym.sym;
            out.key.keysym.mod = in.key.keysym.mod;
            break;
        case SDL_MOUSEBUTTONUP:
            out.type = OgreBites::MOUSEBUTTONUP;
            OGRE_FALLTHROUGH;
        case SDL_MOUSEBUTTONDOWN:
            if(!out.type)
                out.type = OgreBites::MOUSEBUTTONDOWN;
            out.button.x = in.button.x;
            out.button.y = in.button.y;
            out.button.button = in.button.button;
            out.button.clicks = in.button.clicks;
            break;
        case SDL_MOUSEWHEEL:
            out.type = OgreBites::MOUSEWHEEL;
            out.wheel.y = in.wheel.y;
            break;
        case SDL_MOUSEMOTION:
            out.type = OgreBites::MOUSEMOTION;
            out.motion.x = in.motion.x;
            out.motion.y = in.motion.y;
            out.motion.xrel = in.motion.xrel;
            out.motion.yrel = in.motion.yrel;
            out.motion.windowID = in.motion.windowID;
            break;
        case SDL_FINGERDOWN:
            out.type = OgreBites::FINGERDOWN;
            OGRE_FALLTHROUGH;
        case SDL_FINGERUP:
            if(!out.type)
                out.type = OgreBites::FINGERUP;
            OGRE_FALLTHROUGH;
        case SDL_FINGERMOTION:
            if(!out.type)
                out.type = OgreBites::FINGERMOTION;
            out.tfinger.x = in.tfinger.x;
            out.tfinger.y = in.tfinger.y;
            out.tfinger.dx = in.tfinger.dx;
            out.tfinger.dy = in.tfinger.dy;
            out.tfinger.fingerId = in.tfinger.fingerId;
            break;
        case SDL_TEXTINPUT:
            out.type = OgreBites::TEXTINPUT;
            out.text.chars = in.text.text;
            break;
        case SDL_JOYAXISMOTION:
            out.type = OgreBites::JOYAXISMOTION;
            out.axis.which = in.jaxis.which;
            out.axis.axis = in.jaxis.axis;
            out.axis.value = in.jaxis.value;
            break;
        case SDL_CONTROLLERAXISMOTION:
            out.type = OgreBites::CONTROLLERAXISMOTION;
            out.axis.which = in.caxis.which;
            out.axis.axis = in.caxis.axis;
            out.axis.value = in.caxis.value;
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            out.type = OgreBites::CONTROLLERBUTTONDOWN;
            OGRE_FALLTHROUGH;
        case SDL_CONTROLLERBUTTONUP:
            if(!out.type)
                out.type = OgreBites::CONTROLLERBUTTONUP;
            out.cbutton.which = in.cbutton.which;
            out.cbutton.button = in.cbutton.button;
            break;
        }

        return out;
    }
}

#endif
