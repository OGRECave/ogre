
#include "EmptyProjectGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreHlmsPbs.h"

using namespace Demo;

namespace Demo
{
	EmptyProjectGameState::EmptyProjectGameState( const Ogre::String &helpDescription ) :
		TutorialGameState( helpDescription )
	{
    }
    //-----------------------------------------------------------------------------------
	void EmptyProjectGameState::createScene01(void)
    {
        mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
	void EmptyProjectGameState::update( float timeSinceLast )
	{
        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
	void EmptyProjectGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
	{
		TutorialGameState::generateDebugText( timeSinceLast, outText );
	}
    //-----------------------------------------------------------------------------------
	void EmptyProjectGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        {
            TutorialGameState::keyReleased( arg );
            return;
        }

		TutorialGameState::keyReleased( arg );
    }
}
