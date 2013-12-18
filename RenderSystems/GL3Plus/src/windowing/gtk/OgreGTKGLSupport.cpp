/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreGTKGLSupport.h"
#include "OgreGTKWindow.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringConverter.h"



using namespace Ogre;

template<> GTKGLSupport* Singleton<GTKGLSupport>::ms_Singleton = 0;
GTKGLSupport* GTKGLSupport::getSingletonPtr(void)
{
    return ms_Singleton;
}
GTKGLSupport& GTKGLSupport::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

GTKGLSupport::GTKGLSupport() : 
    _kit(0, NULL),
    _context_ref(0)
{
    Gtk::GL::init(0, NULL);
    _main_context = 0;
    _main_window = 0;
    //_ogre_widget = 0;
}

void GTKGLSupport::addConfig()
{
    ConfigOption optFullScreen;
    ConfigOption optVideoMode;

     // FS setting possibilities
    optFullScreen.name = "Full Screen";
    optFullScreen.possibleValues.push_back("Yes");
    optFullScreen.possibleValues.push_back("No");
    optFullScreen.currentValue = "No";
    optFullScreen.immutable = false;
 
    // Video mode possibilities
    // XXX Actually do this
    optVideoMode.name = "Video Mode";
    optVideoMode.immutable = false;
    optVideoMode.possibleValues.push_back("640 x 480");
    optVideoMode.possibleValues.push_back("800 x 600");
    optVideoMode.possibleValues.push_back("1024 x 768");
    optVideoMode.possibleValues.push_back("1280 x 1024");

    optVideoMode.currentValue = "800 x 600";

    mOptions[optFullScreen.name] = optFullScreen;
    mOptions[optVideoMode.name] = optVideoMode;
}
    
String GTKGLSupport::validateConfig()
{
    return String("");
}

RenderWindow* GTKGLSupport::createWindow(bool autoCreateWindow, 
                                         GL3PlusRenderSystem* renderSystem, 
					 const String& windowTitle)
{
    if (autoCreateWindow)
    {
        ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "GTKGLSupport::createWindow");
        bool fullscreen = (opt->second.currentValue == "Yes");
 
        opt = mOptions.find("Video Mode");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "GTKGLSupport::createWindow");
        String val = opt->second.currentValue;
        String::size_type pos = val.find('x');
        if (pos == String::npos)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "GTKGLSupport::createWindow");
 
        unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
        unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));
 
        return renderSystem->createRenderWindow(windowTitle, w, h, 32,
fullscreen);
    }
    else
    {
        // XXX What is the else?
                return NULL;
    }
}

RenderWindow* GTKGLSupport::newWindow(const String& name, unsigned int width, 
        unsigned int height, unsigned int colourDepth, bool fullScreen, int left, int top,
        bool depthBuffer, RenderWindow* parentWindowHandle, bool vsync)
{
    GTKWindow* window = new GTKWindow();
    window->create(name, width, height, colourDepth, fullScreen, left, top,
                   depthBuffer, parentWindowHandle);

    //if(!_ogre_widget)
    //	_ogre_widget = window->get_ogre_widget();

    // Copy some important information for future reference, for example
    // for when the context is needed
    if(!_main_context)
        _main_context = window->get_ogre_widget()->get_gl_context();
    if(!_main_window)
        _main_window = window->get_ogre_widget()->get_gl_window();

    return window;
}

void GTKGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting GTK Subsystem ***\n"
        "******************************");

}
 
void GTKGLSupport::stop()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Stopping GTK Subsystem ***\n"
        "******************************");
}

void GTKGLSupport::begin_context(RenderTarget *_target)
{
	// Support nested contexts, in which case.. nothing happens
    	++_context_ref;
    	if (_context_ref == 1) {
		if(_target) {
			// Begin a specific context
			OGREWidget *_ogre_widget = static_cast<GTKWindow*>(_target)->get_ogre_widget();

	        	_ogre_widget->get_gl_window()->gl_begin(_ogre_widget->get_gl_context());
		} else {
			// Begin a generic main context
			_main_window->gl_begin(_main_context);
		}
    	}
}

void GTKGLSupport::end_context()
{
    	--_context_ref;
    	if(_context_ref < 0)
        	OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Too many contexts destroyed!", "GTKGLSupport::end_context");
    	if (_context_ref == 0)
    	{
		// XX is this enough? (_main_window might not be the current window,
 		// but we can never be sure the previous rendering window 
		// even still exists)
		_main_window->gl_end();
    	}
}
 
void GTKGLSupport::initialiseExtensions(void)
{
    // XXX anythign to actually do here?
}

bool GTKGLSupport::checkMinGLVersion(const String& v) const
{
    int major, minor;
    Gdk::GL::query_version(major, minor);

    std::string::size_type pos = v.find(".");
    int cmaj = atoi(v.substr(0, pos).c_str());
    int cmin = atoi(v.substr(pos + 1).c_str());

    return ( (major >= cmaj) && (minor >= cmin) );
}

bool GTKGLSupport::checkExtension(const String& ext) const
{
	// query_gl_extension needs an active context, doesn't matter which one
	if (_context_ref == 0)
		_main_window->gl_begin(_main_context);

	bool result = Gdk::GL::query_gl_extension(ext.c_str());

	if (_context_ref == 0)
		_main_window->gl_end();
}

void* GTKGLSupport::getProcAddress(const String& procname)
{
    return (void*)Gdk::GL::get_proc_address(procname.c_str());
}

Glib::RefPtr<const Gdk::GL::Context> GTKGLSupport::getMainContext() const {
	return _main_context;
}

