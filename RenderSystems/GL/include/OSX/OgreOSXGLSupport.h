#ifndef OGRE_OSXGLSupport_H
#define OGRE_OSXGLSupport_H

#include "OgreGLSupport.h"

namespace Ogre
{

class OSXGLSupport : public GLSupport
{
public:
	OSXGLSupport();
	~OSXGLSupport();

	/**
	* Add any special config values to the system.
	* Must have a "Full Screen" value that is a bool and a "Video Mode" value
	* that is a string in the form of wxh
	*/
	void addConfig( void );

	/**
	* Make sure all the extra options are valid
	*/
	String validateConfig( void );

	/// @copydoc GLSupport::createWindow
	RenderWindow* createWindow( bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle );
	
	/// @copydoc RenderSystem::createRenderWindow
	virtual RenderWindow* newWindow( const String &name, unsigned int width, unsigned int height, 
		bool fullScreen, const NameValuePairList *miscParams = 0 );
	
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
	void* getProcAddress( const char *name );
	void* getProcAddress( const String& procname );

	virtual bool supportsPBuffers();
	virtual GLPBuffer* createPBuffer( PixelComponentType format, size_t width, size_t height );
	
	// Core Foundation Array callback function for sorting, must be static for the function ptr
	static CFComparisonResult _compareModes (const void *val1, const void *val2, void *context);
	// Core Fondation Dictionary helper functions, also static for ease of use in above static
	static Boolean _getDictionaryBoolean(CFDictionaryRef dict, const void* key);
	static long _getDictionaryLong(CFDictionaryRef dict, const void* key);
	
protected:
	String mAPI;
	String mContextType;
	
}; // class OSXGLSupport

}; // namespace Ogre

#endif // OGRE_OSXGLSupport_H
