
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

    /// Base game state for the tutorials. All it does is show a little text on screen :)
    class TutorialGameState : public GameState
    {
    protected:
        GraphicsSystem      *mGraphicsSystem;

        Ogre::String        mHelpDescription;
        bool                mDisplayHelp;

        Ogre::v1::TextAreaOverlayElement *mDebugText;

        virtual void createDebugTextOverlay(void);
        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        TutorialGameState( const Ogre::String &helpDescription );

        void _notifyGraphicsSystem( GraphicsSystem *graphicsSystem );

        virtual void createScene01(void);

        virtual void update( float timeSinceLast );

        virtual void keyReleased (const SDL_KeyboardEvent &arg );
    };
}

#endif
