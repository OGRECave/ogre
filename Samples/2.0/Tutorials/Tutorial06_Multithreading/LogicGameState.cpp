
#include "LogicGameState.h"
#include "LogicSystem.h"
#include "GameEntityManager.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"

using namespace Demo;

namespace Demo
{
    LogicGameState::LogicGameState() :
        mDisplacement( 0 ),
        mCubeEntity( 0 ),
        mCubeMoDef( 0 ),
        mLogicSystem( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    LogicGameState::~LogicGameState()
    {
        delete mCubeMoDef;
        mCubeMoDef = 0;
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::createScene01(void)
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        GameEntityManager *geMgr = mLogicSystem->getGameEntityManager();

        mCubeMoDef = new MovableObjectDefinition();
        mCubeMoDef->meshName        = "Cube_d.mesh";
        mCubeMoDef->resourceGroup   = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
        mCubeMoDef->moType          = MoTypeItem;

        mCubeEntity = geMgr->addGameEntity( Ogre::SCENE_DYNAMIC, mCubeMoDef, origin,
                                            Ogre::Quaternion::IDENTITY,
                                            Ogre::Vector3::UNIT_SCALE );
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast * 4.0f;
        mDisplacement = fmodf( mDisplacement, 10.0f );

        const size_t currIdx = mLogicSystem->getCurrentTransformIdx();
        //const size_t prevIdx = (currIdx + NUM_GAME_ENTITY_BUFFERS - 1) % NUM_GAME_ENTITY_BUFFERS;

        mCubeEntity->mTransform[currIdx]->vPos = origin + Ogre::Vector3::UNIT_X * mDisplacement;

        //This code will read our last position we set and update it to the the new buffer.
        //Graphics will be reading mCubeEntity->mTransform[prevIdx]; as long as we don't
        //write to it, we're ok.
        //mCubeEntity->mTransform[currIdx]->vPos = mCubeEntity->mTransform[prevIdx]->vPos +
        //                                              Ogre::Vector3::UNIT_X * timeSinceLast;

        GameState::update( timeSinceLast );
    }
}
