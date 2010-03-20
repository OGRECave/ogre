/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreRoot.h"

#include "OgreGLES2RenderSystem.h"
#include "OgreEAGLES2Context.h"

#include "OgreEAGL2Support.h"
#include "OgreEAGL2Window.h"

#include "macUtils.h"

namespace Ogre {

    EAGL2Support::EAGL2Support()
    {
    }

    EAGL2Support::~EAGL2Support()
    {
    }

    void EAGL2Support::addConfig(void)
    {
        ConfigOption optFullScreen;
        ConfigOption optOrientation;
        ConfigOption optVideoMode;
        ConfigOption optDisplayFrequency;
        ConfigOption optFSAA;
        ConfigOption optRTTMode;

        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back("Yes");
        optFullScreen.possibleValues.push_back("No");
        optFullScreen.currentValue = "Yes";
        optFullScreen.immutable = false;

        CGRect appFrame = [[UIScreen mainScreen] applicationFrame];

        optVideoMode.name = "Video Mode";
        optVideoMode.possibleValues.push_back("320 x 480");
        optVideoMode.possibleValues.push_back("768 x 1024");
        optVideoMode.currentValue = StringConverter::toString(appFrame.size.width) + " x " + 
                                    StringConverter::toString(appFrame.size.height);
        optVideoMode.immutable = false;

        optOrientation.name = "Orientation";
        optOrientation.possibleValues.push_back("Landscape Left");
        optOrientation.possibleValues.push_back("Landscape Right");
        optOrientation.possibleValues.push_back("Portrait");
        optOrientation.currentValue = "Landscape Left";
        optOrientation.immutable = false;
        
        optDisplayFrequency.name = "Display Frequency";
        optDisplayFrequency.possibleValues.push_back("0 Hz");
        optDisplayFrequency.currentValue = "0 Hz";
        optDisplayFrequency.immutable = false;

        optFSAA.name = "FSAA";
        optFSAA.possibleValues.push_back( "0" );
        
        // TODO: DJR - figure out how to get the number of samples
        switch( 0 )
        {
            case 6:
                optFSAA.possibleValues.push_back( "2" );
                optFSAA.possibleValues.push_back( "4" );
                optFSAA.possibleValues.push_back( "6" );
                break;
            case 4:
                optFSAA.possibleValues.push_back( "2" );
                optFSAA.possibleValues.push_back( "4" );
                break;
            case 2:
                optFSAA.possibleValues.push_back( "2" );
                break;
            default: break;
        }
        
        optFSAA.currentValue = "0";
        optFSAA.immutable = false;

        optRTTMode.name = "RTT Preferred Mode";
        optRTTMode.possibleValues.push_back("Copy");
        optRTTMode.possibleValues.push_back("FBO");
        optRTTMode.currentValue = "FBO";
        optRTTMode.immutable = false;

        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optDisplayFrequency.name] = optDisplayFrequency;
        mOptions[optFSAA.name] = optFSAA;
        mOptions[optRTTMode.name] = optRTTMode;
        mOptions[optOrientation.name] = optOrientation;
        
        // Set the shader cache path
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString *cachesDirectory = [paths objectAtIndex:0];

        setShaderCachePath([[cachesDirectory stringByAppendingString:@"/"] cStringUsingEncoding:NSASCIIStringEncoding]);
        
        // Set the shader library path
        setShaderLibraryPath(Ogre::macBundlePath() + "/Contents/Media/RTShaderLib");
    }

    String EAGL2Support::validateConfig(void)
    {
        // TODO - DJR
        return StringUtil::BLANK;
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
    
    RenderWindow * EAGL2Support::createWindow(bool autoCreateWindow,
                                           GLES2RenderSystem* renderSystem,
                                           const String& windowTitle)
    {
        RenderWindow *window = 0;

        if (autoCreateWindow)
        {
            ConfigOptionMap::iterator opt;
            ConfigOptionMap::iterator end = mOptions.end();
            NameValuePairList miscParams;

            bool fullscreen = false;
            uint w = 320, h = 480;

            if ((opt = mOptions.find("Full Screen")) != end)
            {
                fullscreen = (opt->second.currentValue == "Yes");
            }

            if ((opt = mOptions.find("Display Frequency")) != end)
            {
                miscParams["displayFrequency"] = opt->second.currentValue;
            }

            if ((opt = mOptions.find("Orientation")) != end)
            {
                miscParams["orientation"] = opt->second.currentValue;
            }

            if ((opt = mOptions.find("Video Mode")) != end)
            {
                String val = opt->second.currentValue;
                String::size_type pos = val.find('x');

                if (pos != String::npos)
                {
                    w = StringConverter::parseUnsignedInt(val.substr(0, pos));
                    h = StringConverter::parseUnsignedInt(val.substr(pos + 1));
                }
            }

            if ((opt = mOptions.find("FSAA")) != end)
            {
                miscParams["FSAA"] = opt->second.currentValue;
            }

            window = renderSystem->_createRenderWindow(windowTitle, w, h, fullscreen, &miscParams);
        }

        return window;
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

    EAGLES2Context * EAGL2Support::createNewContext(CFDictionaryRef &glconfig, CAEAGLLayer *drawable) const //, EAGLES2Context shareList) const
    {
        EAGLES2Context *context = OGRE_NEW EAGLES2Context(drawable);
        if (context == NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create new context",
                        __FUNCTION__);
            return context;
        }
        
        return context;
    }

    void * EAGL2Support::getProcAddress(const Ogre::String& name)
    {
        return NULL;
    }
    
    void EAGL2Support::start()
    {
    }
    
    void EAGL2Support::stop()
    {
    }
}
