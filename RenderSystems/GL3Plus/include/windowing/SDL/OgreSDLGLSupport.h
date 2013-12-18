#ifndef OGRE_SDLGLSUPPORT_H
#define OGRE_SDLGLSUPPORT_H

#include "OgreSDLPrerequisites.h"
#include "OgreGL3PlusSupport.h"

namespace Ogre
{
    
class _OgrePrivate SDLGLSupport : public GL3PlusSupport
{
public:
    SDLGLSupport();
    ~SDLGLSupport();

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    void addConfig(void);
    /**
    * Make sure all the extra options are valid
    */
    String validateConfig(void);

    virtual RenderWindow* createWindow(bool autoCreateWindow, GL3PlusRenderSystem* renderSystem, const String& windowTitle);

	/// @copydoc RenderSystem::createRenderWindow
	virtual RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
		bool fullScreen, const NameValuePairList *miscParams = 0);

    /**
    * Start anything special
    */
    void start();
    /**
    * Stop anything special
    */
    void stop();

    /**
    * Get the address of a function
    */
    void* getProcAddress(const String& procname);
private:
    // Allowed video modes
    SDL_Rect** mVideoModes;


}; // class SDLGLSupport

}; // namespace Ogre

#endif // OGRE_SDLGLSUPPORT_H
