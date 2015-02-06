
#ifndef _Mq_MqMessages_H_
#define _Mq_MqMessages_H_

#include <vector>
#include <assert.h>

namespace Demo
{
namespace Mq
{
    enum MessageId
    {
        //Graphics <-  Logic
        LOGICFRAME_FINISHED,
        GAME_ENTITY_ADDED,
        GAME_ENTITY_REMOVED,
        //Graphics <-> Logic
        GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT,
        //Graphics  -> Logic
        SDL_EVENT,

        NUM_MESSAGE_IDS
    };
}
}

#endif
