/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2011 Torus Knot Software Ltd
 
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

#ifndef __SampleBrowser_NACL_H__
#define __SampleBrowser_NACL_H__

#include "OgrePlatform.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_NACL
#error This header is for use with NaCl only
#endif

#include <pthread.h>
#include <map>
#include <vector>

//#include "SDL.h"
//#include "SDL_nacl.h"
#include "ppapi/cpp/instance.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/gles2/gl2ext_ppapi.h"

namespace Ogre {

    class AppDelegate : public pp::Instance {
    public:

        explicit AppDelegate(PP_Instance instance)
            :pp::Instance(instance)
        {
            //SDL_NACL_SetInstance(instance, 1, 1);
        }

        virtual ~AppDelegate()
        {

        }

        bool HandleInputEvent(const pp::InputEvent& event)
        {
            //SDL_NACL_PushEvent(event);
            return false;//todo true
        }
        std::string error;
        // Called by the browser when the NaCl module is loaded and all ready to go.
        virtual bool Init(uint32_t argc, const char* argn[], const char* argv[])
        {
	        /* Initialize SDL */
	        /*if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		        fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		        return(1);
	        }*/

        
            try {
                  sb.initAppForNaCl(this);
                  sb.initApp();

            //    Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();

                // Clear event times
		      //  Ogre::Root::getSingleton().clearEventTimes();
            } catch( Ogre::Exception& e ) {
                error = e.getFullDescription().c_str();
            }

            return true;
        }

        // Called whenever the in-browser window changes size.
        virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip)
        {
            try {
                int width = sb.getRenderWindow() ? sb.getRenderWindow()->getWidth() : 0;
                int height = sb.getRenderWindow() ? sb.getRenderWindow()->getHeight() : 0;
                if (position.size().width() == width &&
                    position.size().height() == height)
                return;  // Size didn't change, no need to update anything.

                sb.getRenderWindow()->resize(position.size().width(), position.size().height());
                DrawSelf();
            } catch( Ogre::Exception& e ) {
                error = e.getFullDescription().c_str();
            }
        }

        // Called by the browser to handle the postMessage() call in Javascript.
        virtual void HandleMessage(const pp::Var& message)
        {
            // handle js messages here
            PostMessage(pp::Var(error.c_str()));
        }

        // Called to draw the contents of the module's browser area.
        void DrawSelf()
        {
            PostMessage(pp::Var("DrawSelf"));
            // call render here
        }

    private:
        OgreBites::SampleBrowser sb;
    };

}  // namespace Ogre


/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class AppDelegateModule : public pp::Module {
public:
    AppDelegateModule() : pp::Module() {}
    virtual ~AppDelegateModule() {
        glTerminatePPAPI();
    }

    /// Called by the browser when the module is first loaded and ready to run.
    /// This is called once per module, not once per instance of the module on
    /// the page.
    virtual bool Init() {
        return glInitializePPAPI(get_browser_interface()) == GL_TRUE;
        return true;
    }

    /// Create and return a Tumbler instance object.
    /// @param[in] instance The browser-side instance.
    /// @return the plugin-side instance.
    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new Ogre::AppDelegate(instance);
    }
};

namespace pp {
    /// Factory function called by the browser when the module is first loaded.
    /// The browser keeps a singleton of this module.  It calls the
    /// CreateInstance() method on the object you return to make instances.  There
    /// is one instance per <embed> tag on the page.  This is the main binding
    /// point for your NaCl module with the browser.
    Module* CreateModule() {
        return new AppDelegateModule();
    }
}  // namespace pp



#endif
