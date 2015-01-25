
#ifndef _GraphicsSystem_H_
#define _GraphicsSystem_H_

#include "BaseSystem.h"
#include "OgrePrerequisites.h"
#include "OgreColourValue.h"

#include <SDL.h>

namespace Demo
{
    class GameState;
    class SdlInputHandler;

    class GraphicsSystem : public BaseSystem
    {
        BaseSystem          *mLogicSystem;

        SDL_Window          *mSdlWindow;
        SdlInputHandler     *mInputHandler;

        Ogre::Root                  *mRoot;
        Ogre::RenderWindow          *mRenderWindow;
        Ogre::SceneManager          *mSceneManager;
        Ogre::Camera                *mCamera;
        Ogre::CompositorWorkspace   *mWorkspace;
        Ogre::String                mResourcePath;

        bool                mQuit;

        Ogre::ColourValue   mBackgroundColour;

        void handleWindowEvent( const SDL_Event& evt );

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void processIncomingMessage( Mq::MessageId messageId, Mq::SendData data );

        virtual void setupResources(void);
        virtual void registerHlms(void);
        /// Optional override method where you can perform resource group loading
        /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        virtual void loadResources(void);
        virtual void chooseSceneManager(void);
        virtual void createCamera(void);
        /// Virtual so that advanced samples such as Sample_Compositor can override this
        /// method to change the default behavior if setupCompositor() is overridden, be
        /// aware @mBackgroundColour will be ignored
        virtual Ogre::CompositorWorkspace* setupCompositor();

        /// Optional override method where you can create resource listeners (e.g. for loading screens)
        virtual void createResourceListener(void) {}

    public:
        GraphicsSystem( GameState *gameState,
                        Ogre::ColourValue backgroundColour = Ogre::ColourValue( 0.2f, 0.4f, 0.6f ) );
        virtual ~GraphicsSystem();

        void initialize(void);
        void deinitialize(void);

        void update( float timeSinceLast );

        SdlInputHandler* getInputHandler(void)                  { return mInputHandler; }

        bool getQuit(void) const                                { return mQuit; }
    };
}

#endif
