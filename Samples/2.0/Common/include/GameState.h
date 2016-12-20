
#ifndef _Demo_GameState_H_
#define _Demo_GameState_H_

#include "InputListeners.h"

namespace Demo
{
    class GameState : public MouseListener, public KeyboardListener, public JoystickListener
    {
    public:
        virtual ~GameState() {}

        virtual void initialize(void) {}
        virtual void deinitialize(void) {}

        virtual void createScene01(void) {}
        virtual void createScene02(void) {}

        virtual void destroyScene(void) {}

        virtual void update( float timeSinceLast ) {}
        virtual void finishFrameParallel(void) {}
        virtual void finishFrame(void) {}
    };
}

#endif
