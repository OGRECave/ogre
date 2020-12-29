/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreRoot.h"

#include "OgreRenderSystem.h"
#include "OgreEAGLES2Context.h"

#include "OgreEAGL2Support.h"
#include "OgreEAGL2Window.h"

#import <UIKit/UIScreen.h>

namespace Ogre {
    float EAGLCurrentOSVersion;
    
    GLNativeSupport* getGLSupport(int = 0)
    {
        return new EAGL2Support();
    }

    EAGL2Support::EAGL2Support() : GLNativeSupport(CONTEXT_ES)
    {
        EAGLCurrentOSVersion = [[[UIDevice currentDevice] systemVersion] floatValue];
    }

    EAGL2Support::~EAGL2Support()
    {
    }

    ConfigOptionMap EAGL2Support::getConfigOptions(void)
    {
        ConfigOptionMap mOptions;
        ConfigOption optContentScalingFactor;
        
        // Get the application frame size.  On all iPhones(including iPhone 4) this will be 320 x 480
        // The iPad, at least with iPhone OS 3.2 will report 768 x 1024
        CGSize screenSize = [[UIScreen mainScreen] bounds].size;
        mVideoModes.push_back({uint32(screenSize.width), uint32(screenSize.height), 0});
        mVideoModes.push_back({320, 480, 0});
        mVideoModes.push_back({320, 568, 0});
        mVideoModes.push_back({768, 1024, 0});

        optContentScalingFactor.name = "Content Scaling Factor";
        optContentScalingFactor.possibleValues.push_back( "1.0" );
        optContentScalingFactor.possibleValues.push_back( "1.33" );
        optContentScalingFactor.possibleValues.push_back( "1.5" );
        optContentScalingFactor.possibleValues.push_back( "2.0" );
        optContentScalingFactor.currentValue = StringConverter::toString([UIScreen mainScreen].scale);
        optContentScalingFactor.immutable = false;
        
        mFSAALevels.push_back(0);
        mFSAALevels.push_back(2);
        mFSAALevels.push_back(4);
        mFSAALevels.push_back(8);

        mOptions[optContentScalingFactor.name] = optContentScalingFactor;
        
#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
        ConfigOption optOrientation;
        optOrientation.name = "Orientation";
        optOrientation.possibleValues.push_back("Portrait");
        optOrientation.possibleValues.push_back("Landscape Left");
        optOrientation.possibleValues.push_back("Landscape Right");
        optOrientation.currentValue = "Portrait";
        optOrientation.immutable = false;
        mOptions[optOrientation.name] = optOrientation;
#endif

        return mOptions;
    }

    String EAGL2Support::getDisplayName(void)
    {
        return "todo";
	}

    CFDictionaryRef EAGL2Support::chooseGLConfig(const GLint *attribList, GLint *nElements)
    {
        // TODO: DJR - implement
        CFDictionaryRef configs = NULL;

        return configs;
    }

    GLint EAGL2Support::getGLConfigAttrib(CFDictionaryRef glConfig, GLint attribute, GLint *value)
    {
        // TODO: DJR - implement
        GLint status = 0;

        return status;
    }

    CFDictionaryRef EAGL2Support::getGLConfigFromContext(EAGLES2Context context)
    {
        // TODO: DJR - implement
        CFDictionaryRef glConfig = 0;

        return glConfig;
    }

    CFDictionaryRef EAGL2Support::getGLConfigFromDrawable(CAEAGLLayer *drawable,
                                                    unsigned int *w, unsigned int *h)
    {
        // TODO: DJR - implement
        CFDictionaryRef glConfig = 0;

        return glConfig;
    }

    //------------------------------------------------------------------------
    // A helper class for the implementation of selectFBConfig
    //------------------------------------------------------------------------
    class GLConfigAttribs
    {
        public:
            GLConfigAttribs(const int* attribs)
            {
                for (int i = 0; attribs[2*i] != 0; i++)
                {
                    fields[attribs[2*i]] = attribs[2*i+1];
                }
            }

            void load(EAGL2Support* const glSupport, CFDictionaryRef glConfig)
            {
                std::map<int,int>::iterator it;

                for (it = fields.begin(); it != fields.end(); it++)
                {
                    it->second = 0;

                    glSupport->getGLConfigAttrib(glConfig, it->first, &it->second);
                }
            }

            bool operator>(GLConfigAttribs& alternative)
            {
                return false;
            }

            std::map<int,int> fields;
    };

    CFDictionaryRef EAGL2Support::selectGLConfig(const int* minAttribs, const int *maxAttribs)
    {
        // TODO: DJR - implement
        CFDictionaryRef glConfig = 0;
        int nConfigs = 0;//, config;

        glConfig = chooseGLConfig(minAttribs, &nConfigs);

        if (!nConfigs)
        {
            return 0;
        }

        return glConfig;
    }
    
    RenderWindow * EAGL2Support::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
        EAGL2Window *window = OGRE_NEW EAGL2Window(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    EAGLES2Context * EAGL2Support::createNewContext(CAEAGLLayer *drawable, EAGLSharegroup *group) const
    {
        EAGLES2Context *context = new EAGLES2Context(drawable, group);
        if (context == NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Fail to create new context");
            return context;
        }

        // Initialise GL3W
        if (glGetError == NULL && gleswInit())
            LogManager::getSingleton().logMessage("Failed to initialize GL3W");

        return context;
    }

    void * EAGL2Support::getProcAddress(const char* name) const
    {
        return (void*)gleswGetProcAddress(name);
    }
    
    void EAGL2Support::start()
    {
    }
    
    void EAGL2Support::stop()
    {
    }

    bool EAGL2Support::interfaceOrientationIsSupported(NSString *orientation)
    {
        NSArray *supportedOrientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];

        return [supportedOrientations containsObject:orientation];
    }

    bool EAGL2Support::portraitIsSupported()
    {
        NSArray *supportedOrientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];

        return ([supportedOrientations containsObject:@"UIInterfaceOrientationPortrait"] || 
                [supportedOrientations containsObject:@"UIInterfaceOrientationPortraitUpsideDown"]);
    }
}
