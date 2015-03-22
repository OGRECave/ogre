
#include "MyGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreTextAreaOverlayElement.h"

using namespace Demo;

extern int gCurrentFrameTimeIdx;
extern bool gFakeSlowmo;

namespace Demo
{
    MyGameState::MyGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mSceneNode( 0 ),
        mDisplacement( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void MyGameState::createScene01(void)
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
    void MyGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to toggle fixed framerate.";
        outText += "\nPress F3 to fake slow motion. ";
        outText += gFakeSlowmo ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void MyGameState::update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast * 4.0f;
        mDisplacement = fmodf( mDisplacement, 10.0f );

        mSceneNode->setPosition( origin + Ogre::Vector3::UNIT_X * mDisplacement );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void MyGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

        if( arg.keysym.sym == SDLK_F2 )
        {
            gCurrentFrameTimeIdx = !gCurrentFrameTimeIdx;
        }
        if( arg.keysym.sym == SDLK_F3 )
        {
            gFakeSlowmo = !gFakeSlowmo;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }
}
