
#include "BaseSystem.h"
#include "GameState.h"

namespace Demo
{
    BaseSystem::BaseSystem( GameState *gameState ) :
        mCurrentGameState( gameState )
    {
    }
    //-----------------------------------------------------------------------------------
    BaseSystem::~BaseSystem()
    {
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::initialize(void)
    {
        mCurrentGameState->initialize();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::deinitialize(void)
    {
        mCurrentGameState->deinitialize();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::createScene01(void)
    {
        mCurrentGameState->createScene01();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::createScene02(void)
    {
        mCurrentGameState->createScene02();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::destroyScene(void)
    {
        mCurrentGameState->destroyScene();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::beginFrameParallel(void)
    {
        this->processIncomingMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::update( float timeSinceLast )
    {
        mCurrentGameState->update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::finishFrameParallel(void)
    {
        mCurrentGameState->finishFrameParallel();

        this->flushQueuedMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::finishFrame(void)
    {
        mCurrentGameState->finishFrame();
    }
}
