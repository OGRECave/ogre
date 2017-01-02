
#ifndef _Demo_TutorialGameState_H_
#define _Demo_TutorialGameState_H_

#include "OgrePrerequisites.h"
#include "GameState.h"

namespace Ogre
{
    namespace v1
    {
        class TextAreaOverlayElement;
    }
}

namespace Demo
{
    class GraphicsSystem;
    class CameraController;

    /// Base game state for the tutorials. All it does is show a little text on screen :)
    class TutorialGameState : public GameState
    {
    protected:
        GraphicsSystem      *mGraphicsSystem;

        /// Optional, for controlling the camera with WASD and the mouse
        CameraController    *mCameraController;

        Ogre::String        mHelpDescription;
        Ogre::uint16        mDisplayHelpMode;
        Ogre::uint16        mNumDisplayHelpModes;

        Ogre::v1::TextAreaOverlayElement *mDebugText;
        Ogre::v1::TextAreaOverlayElement *mDebugTextShadow;

        virtual void createDebugTextOverlay(void);
        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        TutorialGameState( const Ogre::String &helpDescription );
        virtual ~TutorialGameState();

        void _notifyGraphicsSystem( GraphicsSystem *graphicsSystem );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyPressed( const SDL_KeyboardEvent &arg );
        virtual void keyReleased( const SDL_KeyboardEvent &arg );

        virtual void mouseMoved( const SDL_Event &arg );
    };
}

#endif
