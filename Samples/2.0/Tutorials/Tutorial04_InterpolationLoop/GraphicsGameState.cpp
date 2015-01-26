
#include "GraphicsGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreTextAreaOverlayElement.h"

using namespace Demo;

extern const double cFrametime;
extern bool gFakeSlowmo;
extern bool gFakeFrameskip;

namespace Demo
{
    GraphicsGameState::GraphicsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mLastPosition( Ogre::Vector3::ZERO ),
        mCurrentPosition( Ogre::Vector3::ZERO )
    {
    }
    //-----------------------------------------------------------------------------------
    void GraphicsGameState::createScene01(void)
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
    void GraphicsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
        outText += "\nPress F2 to fake a GPU bottleneck (frame skip). ";
        outText += gFakeFrameskip ? "[On]" : "[Off]";
        outText += "\nPress F3 to fake a CPU Logic bottleneck. ";
        outText += gFakeSlowmo ? "[On]" : "[Off]";
    }
    //-----------------------------------------------------------------------------------
    void GraphicsGameState::update( float timeSinceLast )
    {
        float weight = mGraphicsSystem->getAccumTimeSinceLastLogicFrame() / cFrametime;
        weight = std::min( 1.0f, weight );

        Ogre::Vector3 interpPosition = Ogre::Math::lerp( mLastPosition, mCurrentPosition, weight );

        mSceneNode->setPosition( interpPosition );

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F2 )
        {
            gFakeFrameskip = !gFakeFrameskip;
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
