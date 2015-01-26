
#include "LogicSystem.h"
#include "GameState.h"
#include "SdlInputHandler.h"

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreConfigFile.h"

#include "OgreRenderWindow.h"
#include "OgreCamera.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

#include "Compositor/OgreCompositorManager2.h"

#include "OgreOverlaySystem.h"

#include <SDL_syswm.h>

namespace Demo
{
    LogicSystem::LogicSystem( GameState *gameState ) :
        BaseSystem( gameState ),
        mGraphicsSystem( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    LogicSystem::~LogicSystem()
    {
    }
    //-----------------------------------------------------------------------------------
    void LogicSystem::finishFrameParallel(void)
    {
        //Notify the GraphicsSystem we're done rendering this frame.
        if( mGraphicsSystem )
        {
            this->queueSendMessage( mGraphicsSystem, Mq::Message( Mq::LOGICFRAME_FINISHED,
                                                                  Mq::SendData(), false ) );
        }

        BaseSystem::finishFrameParallel();
    }
    //-----------------------------------------------------------------------------------
    void LogicSystem::processIncomingMessage( Mq::MessageId messageId, Mq::SendData data )
    {
        switch( messageId )
        {
        case Mq::SDL_EVENT:
            //TODO
            break;
        case Mq::SDL_EVENT_BUFFER_ID_USED:
            //Notify the GraphicsSystem we're done using this buffer (Important: The message
            //must be sent after we've processed all Mq::SDL_EVENT messages from this frame).
            this->queueSendMessage( mGraphicsSystem, Mq::Message( Mq::SDL_EVENT_BUFFER_ID_USED,
                                                                  data, false ) );
            break;
        default:
            break;
        }
    }
}
