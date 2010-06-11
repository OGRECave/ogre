#ifndef __ogrewrapper_H__
#define __ogrewrapper_H__

#include <Ogre.h>

// Initialize the root and load the GLES2 render system
bool initOgreRoot();

// Initializes the render window
// Returns null if it fails
Ogre::RenderWindow *initRenderWindow(unsigned int windowHandle, unsigned int width, 
	unsigned int height, unsigned int contextHandle);

// Destroy Ogre's core systems
void destroyOgreRoot();

// Destroys the previously created RenderWindow 
void destroyRenderWindow();

// Returns the render window created
Ogre::RenderWindow *getRenderWindow();

// Renders one frame
void renderOneFrame();

#endif