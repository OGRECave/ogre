/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreRoot.h"

#include "OgreGLESRenderSystem.h"
#include "OgreEAGLESContext.h"

#include "OgreEAGLRenderTexture.h"
#include "OgreEAGLSupport.h"
#include "OgreEAGLWindow.h"

namespace Ogre {

    EAGLSupport::EAGLSupport()
    {
    }

    EAGLSupport::~EAGLSupport()
    {
    }

    void EAGLSupport::addConfig(void)
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

        optVideoMode.name = "Video Mode";
        optVideoMode.possibleValues.push_back("480 x 320");
        optVideoMode.currentValue = "480 x 320";
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
    }

    String EAGLSupport::validateConfig(void)
    {
        // TODO - DJR
        return StringUtil::BLANK;
    }

    String EAGLSupport::getDisplayName(void)
    {
        return "todo";
	}

    CFDictionaryRef EAGLSupport::chooseGLConfig(const GLint *attribList, GLint *nElements)
    {
        // TODO: DJR - implement
        CFDictionaryRef configs = NULL;

        return configs;
    }

    GLint EAGLSupport::getGLConfigAttrib(CFDictionaryRef glConfig, GLint attribute, GLint *value)
    {
        // TODO: DJR - implement
        GLint status = 0;

        return status;
    }

    CFDictionaryRef EAGLSupport::getGLConfigFromContext(EAGLESContext context)
    {
        // TODO: DJR - implement
        CFDictionaryRef glConfig = 0;

        return glConfig;
    }

    CFDictionaryRef EAGLSupport::getGLConfigFromDrawable(CAEAGLLayer *drawable,
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

            void load(EAGLSupport* const glSupport, CFDictionaryRef glConfig)
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

    CFDictionaryRef EAGLSupport::selectGLConfig(const int* minAttribs, const int *maxAttribs)
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

    GLESPBuffer * EAGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
	{
		return new EAGLPBuffer(this, format, width, height);
	}
    
    void EAGLSupport::switchMode(void)
    {
// TODO: Support mode switch, and orientation change
//        return switchMode(mOriginalMode.first.first,
//                          mOriginalMode.first.second, mOriginalMode.second);
    }
    
    RenderWindow * EAGLSupport::createWindow(bool autoCreateWindow,
                                           GLESRenderSystem* renderSystem,
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

    RenderWindow * EAGLSupport::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
        EAGLWindow *window = new EAGLWindow(this);
        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    EAGLESContext * EAGLSupport::createNewContext(CFDictionaryRef &glconfig, CAEAGLLayer *drawable) const //, EAGLESContext shareList) const
    {
        EAGLESContext *context = new EAGLESContext(drawable);
        if (context == NULL)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create new context",
                        __FUNCTION__);
            return context;
        }
        
        return context;
    }

    void * EAGLSupport::getProcAddress(const Ogre::String& name)
    {
        return NULL;
    }
    
    void EAGLSupport::start()
    {
    }
    
    void EAGLSupport::stop()
    {
    }
}
