
#ifndef _BaseSystem_H_
#define _BaseSystem_H_

#include "Threading/MessageQueueSystem.h"

namespace Demo
{
    class GameState;

    class BaseSystem : public Mq::MessageQueueSystem
    {
    protected:
        GameState   *mCurrentGameState;

    public:
        BaseSystem( GameState *gameState );
        virtual ~BaseSystem();

        void initialize(void);
        void deinitialize(void);

        void createScene01(void);
        void createScene02(void);

        void destroyScene(void);

        void beginFrameParallel(void);
        void update( float timeSinceLast );
        void finishFrameParallel(void);
        void finishFrame(void);
    };
}

#endif
