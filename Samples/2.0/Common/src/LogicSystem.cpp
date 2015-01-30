
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
            this->queueSendMessage( mGraphicsSystem, Mq::LOGICFRAME_FINISHED, 0 );

        BaseSystem::finishFrameParallel();
    }
    //-----------------------------------------------------------------------------------
    void LogicSystem::processIncomingMessage( Mq::MessageId messageId, const void *data )
    {
        switch( messageId )
        {
        case Mq::SDL_EVENT:
            //TODO
            break;
        default:
            break;
        }
    }
}
