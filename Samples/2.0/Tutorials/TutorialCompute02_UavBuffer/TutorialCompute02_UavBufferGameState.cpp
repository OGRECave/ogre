
#include "TutorialCompute02_UavBufferGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreTextAreaOverlayElement.h"

using namespace Demo;

namespace Demo
{
    TutorialCompute02_UavBufferGameState::TutorialCompute02_UavBufferGameState(
            const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mSceneNode( 0 ),
        mDisplacement( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void TutorialCompute02_UavBufferGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                     Ogre::ResourceGroupManager::
                                                     AUTODETECT_RESOURCE_GROUP_NAME,
                                                     Ogre::SCENE_DYNAMIC );

        mSceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );

        mSceneNode->attachObject( item );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void TutorialCompute02_UavBufferGameState::update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast * 4.0f;
        mDisplacement = fmodf( mDisplacement, 10.0f );

        mSceneNode->setPosition( origin + Ogre::Vector3::UNIT_X * mDisplacement );

        TutorialGameState::update( timeSinceLast );
    }
}
