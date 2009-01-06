#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"
#include "OgreGLRenderSystem.h"

#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"
#include "OgreGLPBuffer.h"

namespace Ogre
{
    
class _OgreGLExport GLSupport
{
public:
    GLSupport() { }
    virtual ~GLSupport() { }

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    virtual void addConfig() = 0;

	virtual void setConfigOption(const String &name, const String &value);

    /**
    * Make sure all the extra options are valid
    * @return string with error message
    */
    virtual String validateConfig() = 0;

	virtual ConfigOptionMap& getConfigOptions(void);

	virtual RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle) = 0;

	/// @copydoc RenderSystem::_createRenderWindow
	virtual RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
		bool fullScreen, const NameValuePairList *miscParams = 0) = 0;

    virtual bool supportsPBuffers();
    virtual GLPBuffer *createPBuffer(PixelComponentType format, size_t width, size_t height);

    /**
    * Start anything special
    */
    virtual void start() = 0;
    /**
    * Stop anything special
    */
    virtual void stop() = 0;

    /**
    * get vendor information
    */
    const String& getGLVendor(void) const
    {
        return mVendor;
    }

    /**
    * get version information
    */
    const String& getGLVersion(void) const
    {
        return mVersion;
    }

    /**
    * compare GL version numbers
    */
    bool checkMinGLVersion(const String& v) const;

    /**
    * Check if an extension is available
    */
    virtual bool checkExtension(const String& ext) const;
    /**
    * Get the address of a function
    */
    virtual void* getProcAddress(const String& procname) = 0;

    /** Intialises GL extensions, must be done AFTER the GL context has been
        established.
    */
    virtual void initialiseExtensions();

protected:
	// Stored options
    ConfigOptionMap mOptions;

	// This contains the complete list of supported extensions
    set<String>::type extensionList;
private:
    String mVersion;
    String mVendor;

}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H
