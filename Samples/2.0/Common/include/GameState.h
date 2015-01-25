
#ifndef _GameState_H_
#define _GameState_H_

#include "InputListeners.h"

namespace Demo
{
    class GameState : public MouseListener, public KeyboardListener, public JoystickListener
    {
    public:
        ~GameState() {}

        void initialize(void) {}
        void deinitialize(void) {}

        virtual void createScene01(void) {}
        virtual void createScene02(void) {}

        void destroyScene(void) {}

        void update( float timeSinceLast ) {}
        void finishFrameParallel(void) {}
        void finishFrame(void) {}
    };
}

#endif
