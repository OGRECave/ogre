#ifndef OGRE_GLXGLSupport_H
#define OGRE_GLXGLSupport_H

#include "OgreGLSupport.h"

#include <GL/glew.h>
#include <GL/glxew.h>

namespace Ogre {

	class _OgrePrivate GLXGLSupport : public GLSupport
	{
	public:
		GLXGLSupport();
		~GLXGLSupport();
		
		Atom mAtomDeleteWindow;
		Atom mAtomFullScreen;
		Atom mAtomState;
		
		/** @copydoc see GLSupport::addConfig */
		void addConfig(void);
		
		/** @copydoc see GLSupport::validateConfig */
		String validateConfig(void);
		
		/** @copydoc see GLSupport::setConfigOption */
		void setConfigOption(const String &name, const String &value);
		
		/// @copydoc GLSupport::createWindow
		RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle);
		
		/// @copydoc RenderSystem::createRenderWindow
		RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
								bool fullScreen, const NameValuePairList *miscParams = 0);
		
		/// @copydoc GLSupport::createPBuffer
		GLPBuffer* createPBuffer(PixelComponentType format, size_t width, size_t height);
		
		/** @copydoc see GLSupport::start */
		void start();
		
		/** @copydoc see GLSupport::stop */
		void stop();
		
		/** @copydoc see GLSupport::initialiseExtensions */
		void initialiseExtensions();
		
		/** @copydoc see GLSupport::getProcAddress */
		void* getProcAddress(const String& procname);
		
		// The remaining functions are internal to the GLX Rendersystem:
		
		/**
		 * Get the name of the display and screen used for rendering
		 *
		 * Ogre normally opens its own connection to the X server 
		 * and renders onto the screen where the user logged in
		 *
		 * However, if Ogre is passed a current GL context when the first
		 * RenderTarget is created, then it will connect to the X server
		 * using the same connection as that GL context and direct all 
		 * subsequent rendering to the screen targeted by that GL context.
		 * 
		 * @returns		 Display name.
		 */
		String getDisplayName (void);
		
		/**
		 * Get the Display connection used for rendering
		 *
		 * This function establishes the initial connection when necessary.
		 * 
		 * @returns		 Display connection
		 */
		Display* getGLDisplay(void);
		
		/**
		 * Get the Display connection used for window management & events
		 *
		 * @returns		 Display connection
		 */
		Display* getXDisplay(void);
		
		/**
		 * Switch video modes
		 *
		 * @param width	  Receiver for requested and final width
		 * @param height	 Receiver for requested and final drawable height
		 * @param height	 Receiver for requested and final drawable frequency
		 */
		void switchMode (uint& width, uint& height, short& frequency);
		
		/**
		 * Switch back to original video mode
		 */
		void switchMode (void);
		
		/**
		 * Loads an icon from an Ogre resource into the X Server. This currently only
		 * works for 24 and 32 bit displays. The image must be findable by the Ogre
		 * resource system, and of format PF_A8R8G8B8.
		 *
		 * @param display	X display
		 * @param name	   Name of image to load
		 * @param pix		Receiver for the output pixmap
		 * @param mask	   Receiver for the output mask (alpha bitmap)
		 * @returns		  true on success
		 */		
		bool loadIcon(const std::string &name, Pixmap *pix, Pixmap *mask);
		
		/**
		 * Get the GLXFBConfig used to create a ::GLXContext
		 *
		 * @param drawable   GLXContext 
		 * @returns		  GLXFBConfig used to create the context
		 */
		GLXFBConfig getFBConfigFromContext (::GLXContext context);
		
		/**
		 * Get the GLXFBConfig used to create a GLXDrawable.
		 * Caveat: GLX version 1.3 is needed when the drawable is a GLXPixmap
		 *
		 * @param drawable   GLXDrawable 
		 * @param width	  Receiver for the drawable width
		 * @param height	 Receiver for the drawable height
		 * @returns		  GLXFBConfig used to create the drawable
		 */
		GLXFBConfig getFBConfigFromDrawable (GLXDrawable drawable, unsigned int *width, unsigned int *height);
		
		/**
		 * Select an FBConfig given a list of required and a list of desired properties
		 *
		 * @param display	X Display
		 * @param minAttribs FBConfig attributes that must be provided with minimum values
		 * @param maxAttribs FBConfig attributes that are desirable with maximum values
		 * @returns		  GLXFBConfig with attributes or 0 when unsupported. 
		 */
		GLXFBConfig selectFBConfig(const int *minAttribs, const int *maxAttribs);
		
		/**
		 * Gets a GLXFBConfig compatible with a VisualID
		 * 
		 * Some platforms fail to implement glXGetFBconfigFromVisualSGIX as
		 * part of the GLX_SGIX_fbconfig extension, but this portable
		 * alternative suffices for the creation of compatible contexts.
		 *
		 * @param visualid   VisualID 
		 * @returns		  FBConfig for VisualID
		 */
		GLXFBConfig getFBConfigFromVisualID(VisualID visualid);
		
		/**
		 * Portable replacement for glXChooseFBConfig 
		 */
		GLXFBConfig* chooseFBConfig(const GLint *attribList, GLint *nElements);
		
		/**
		 * Portable replacement for glXCreateNewContext
		 */
		::GLXContext createNewContext(GLXFBConfig fbConfig, GLint renderType, ::GLXContext shareList, GLboolean direct) const;
		
		/**
		 * Portable replacement for glXGetFBConfigAttrib
		 */
		GLint getFBConfigAttrib(GLXFBConfig fbConfig, GLint attribute, GLint *value);
		
		/**
		 * Portable replacement for glXGetVisualFromFBConfig
		 */
		XVisualInfo* getVisualFromFBConfig(GLXFBConfig fbConfig);
		
		private:
		/**
		 * Initialise GLXEW without requiring a current GL context
		 */
		void initialiseGLXEW(void);
		
		/**
		 * Refresh config options to reflect dependencies
		 */
		void refreshConfig(void);
		
		Display* mGLDisplay; // used for GL/GLX commands
		Display* mXDisplay;  // used for other X commands and events
		bool mIsExternalDisplay;
		
		typedef std::pair<uint, uint>	   ScreenSize;
		typedef short					   Rate;
		typedef std::pair<ScreenSize, Rate> VideoMode;
		typedef std::vector<VideoMode>	  VideoModes;
		
		VideoModes mVideoModes;
		VideoMode  mOriginalMode;
		VideoMode  mCurrentMode;
		
		StringVector mSampleLevels;
	};
}

#endif // OGRE_GLXGLSupport_H
