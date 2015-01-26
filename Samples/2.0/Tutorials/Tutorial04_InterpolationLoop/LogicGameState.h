
#ifndef _Demo_MyGameState_H_
#define _Demo_MyGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

namespace Demo
{
    class GraphicsGameState;

    class LogicGameState : public GameState
    {
        float               mDisplacement;
        GraphicsGameState   *mGraphicsGameState;

    public:
        LogicGameState();

        void _notifyGraphicsGameState( GraphicsGameState *graphicsGameState );

        virtual void update( float timeSinceLast );
    };
}

#endif
