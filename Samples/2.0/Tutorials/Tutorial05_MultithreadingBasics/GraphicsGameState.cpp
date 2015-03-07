
#include "GraphicsGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreTextAreaOverlayElement.h"

using namespace Demo;

extern const double cFrametime;

namespace Demo
{
    GraphicsGameState::GraphicsGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {
    }
    //-----------------------------------------------------------------------------------
    void GraphicsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        //Show the current weight.
        //The text doesn't get updated every frame while displaying
        //help, so don't show the weight as it is inaccurate.
        if( mDisplayHelpMode != 0 )
        {
            outText += "\nSEE HELP DESCRIPTION!";

            float weight = mGraphicsSystem->getAccumTimeSinceLastLogicFrame() / cFrametime;
            weight = std::min( 1.0f, weight );

            outText += "\nBlend weight: ";
            outText += Ogre::StringConverter::toString( weight );
        }
    }
}
