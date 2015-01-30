
#ifndef _MqMessages_H_
#define _MqMessages_H_

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
        //Graphics  -> Logic
        SDL_EVENT,

        NUM_MESSAGE_IDS
    };
}
}

#endif
