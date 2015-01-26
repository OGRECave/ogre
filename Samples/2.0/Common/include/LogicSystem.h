
#ifndef _LogicSystem_H_
#define _LogicSystem_H_

#include "BaseSystem.h"
#include "OgrePrerequisites.h"

namespace Demo
{
    class LogicSystem : public BaseSystem
    {
        BaseSystem          *mGraphicsSystem;

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void processIncomingMessage( Mq::MessageId messageId, Mq::SendData data );

    public:
        LogicSystem( GameState *gameState );
        virtual ~LogicSystem();

        void finishFrameParallel(void);
    };
}

#endif
