
#include "TutorialGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"

#include "OgreOverlayManager.h"
#include "OgreOverlay.h"
#include "OgreOverlayContainer.h"
#include "OgreTextAreaOverlayElement.h"

#include "OgreRoot.h"
#include "OgreFrameStats.h"

using namespace Demo;

namespace Demo
{
    TutorialGameState::TutorialGameState( const Ogre::String &helpDescription ) :
        mGraphicsSystem( 0 ),
        mCameraController( 0 ),
        mHelpDescription( helpDescription ),
        mDisplayHelp( false ),
        mDebugText( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::_notifyGraphicsSystem( GraphicsSystem *graphicsSystem )
    {
        mGraphicsSystem = graphicsSystem;
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::createScene01(void)
    {
        createDebugTextOverlay();
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::createDebugTextOverlay(void)
    {
        Ogre::v1::OverlayManager &overlayManager = Ogre::v1::OverlayManager::getSingleton();
        Ogre::v1::Overlay *overlay = overlayManager.create( "DebugText" );

        Ogre::v1::OverlayContainer *panel = static_cast<Ogre::v1::OverlayContainer*>(
            overlayManager.createOverlayElement("Panel", "DebugPanel"));
        mDebugText = static_cast<Ogre::v1::TextAreaOverlayElement*>(
                    overlayManager.createOverlayElement( "TextArea", "DebugText" ) );
        mDebugText->setFontName( "DebugFont" );
        mDebugText->setCharHeight( 0.025f );
        panel->addChild( mDebugText );
        overlay->add2D( panel );
        overlay->show();
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        if( mDisplayHelp )
        {
            outText = mHelpDescription;
            outText += "\n\nPress F1 to toggle help";
            return;
        }

        const Ogre::FrameStats *frameStats = mGraphicsSystem->getRoot()->getFrameStats();

        Ogre::String finalText;
        finalText.reserve( 128 );
        finalText  = "Frame time:\t";
        finalText += Ogre::StringConverter::toString( timeSinceLast * 1000.0f );
        finalText += " ms\n";
        finalText += "Frame FPS:\t";
        finalText += Ogre::StringConverter::toString( 1.0f / timeSinceLast );
        finalText += "\nAvg time:\t";
        finalText += Ogre::StringConverter::toString( frameStats->getAvgTime() );
        finalText += " ms\n";
        finalText += "Avg FPS:\t";
        finalText += Ogre::StringConverter::toString( 1000.0f / frameStats->getAvgTime() );
        finalText += "\n\nPress F1 to toggle help";

        outText.swap( finalText );

        mDebugText->setCaption( finalText );
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::update( float timeSinceLast )
    {
        if( !mDisplayHelp )
        {
            //Show FPS
            Ogre::String finalText;
            generateDebugText( timeSinceLast, finalText );
            mDebugText->setCaption( finalText );
        }

        if( mCameraController )
            mCameraController->update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        bool handledEvent = false;

        if( mCameraController )
            handledEvent = mCameraController->keyPressed( arg );

        if( !handledEvent )
            GameState::keyPressed( arg );
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F1 )
        {
            mDisplayHelp = !mDisplayHelp;

            Ogre::String finalText;
            generateDebugText( 0, finalText );
            mDebugText->setCaption( finalText );
        }
        else
        {
            bool handledEvent = false;

            if( mCameraController )
                handledEvent = mCameraController->keyReleased( arg );

            if( !handledEvent )
                GameState::keyReleased( arg );
        }
    }
    //-----------------------------------------------------------------------------------
    void TutorialGameState::mouseMoved( const SDL_Event &arg )
    {
        if( mCameraController )
            mCameraController->mouseMoved( arg );

        GameState::mouseMoved( arg );
    }
}
