/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreEGLSupport.h"
#include "OgreEGLWindow.h"
#include "OgreEGLRenderTexture.h"


namespace Ogre {


    EGLSupport::EGLSupport()
        : mGLDisplay(0),
          mNativeDisplay(0),
		  mRandr(false)
    {
        mGLDisplay = getGLDisplay();
		mNativeDisplay = getNativeDisplay();

    }

    EGLSupport::~EGLSupport()
    {
	}

    void EGLSupport::addConfig(void)
    {
        ConfigOption optFullScreen;
        ConfigOption optVideoMode;
        ConfigOption optDisplayFrequency;
        ConfigOption optFSAA;
        ConfigOption optRTTMode;

        optFullScreen.name = "Full Screen";
        optFullScreen.immutable = false;

        optVideoMode.name = "Video Mode";
        optVideoMode.immutable = false;

        optDisplayFrequency.name = "Display Frequency";
        optDisplayFrequency.immutable = false;

        optFSAA.name = "FSAA";
        optFSAA.immutable = false;

        optRTTMode.name = "RTT Preferred Mode";
        optRTTMode.immutable = false;

        optFullScreen.possibleValues.push_back("No");
        optFullScreen.possibleValues.push_back("Yes");

        optFullScreen.currentValue = optFullScreen.possibleValues[1];

        VideoModes::const_iterator value = mVideoModes.begin();
        VideoModes::const_iterator end = mVideoModes.end();

        for (; value != end; value++)
        {
            String mode = StringConverter::toString(value->first.first,4) + " x " + StringConverter::toString(value->first.second,4);
            optVideoMode.possibleValues.push_back(mode);
        }
        removeDuplicates(optVideoMode.possibleValues);

        optVideoMode.currentValue = StringConverter::toString(mCurrentMode.first.first,4) + " x " + StringConverter::toString(mCurrentMode.first.second,4);

        refreshConfig();
        if (!mSampleLevels.empty())
        {
            StringVector::const_iterator value = mSampleLevels.begin();
            StringVector::const_iterator end = mSampleLevels.end();

            for (; value != end; value++)
            {
                optFSAA.possibleValues.push_back(*value);
            }

            optFSAA.currentValue = optFSAA.possibleValues[0];
        }

        optRTTMode.possibleValues.push_back("Copy");
        optRTTMode.currentValue = optRTTMode.possibleValues[0];

        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optDisplayFrequency.name] = optDisplayFrequency;
        mOptions[optFSAA.name] = optFSAA;
        mOptions[optRTTMode.name] = optRTTMode;

        refreshConfig();
    }

    void EGLSupport::refreshConfig(void) 
    {
        ConfigOptionMap::iterator optVideoMode = mOptions.find("Video Mode");
        ConfigOptionMap::iterator optDisplayFrequency = mOptions.find("Display Frequency");

        if (optVideoMode != mOptions.end() && optDisplayFrequency != mOptions.end())
        {
            optDisplayFrequency->second.possibleValues.clear();

            VideoModes::const_iterator value = mVideoModes.begin();
            VideoModes::const_iterator end = mVideoModes.end();

            for (; value != end; value++)
            {
                String mode = StringConverter::toString(value->first.first,4) + " x " + StringConverter::toString(value->first.second,4);

                if (mode == optVideoMode->second.currentValue)
                {
                    String frequency = StringConverter::toString(value->second) + " MHz";

                    optDisplayFrequency->second.possibleValues.push_back(frequency);
                }
            }

            if (!optDisplayFrequency->second.possibleValues.empty())
            {
                optDisplayFrequency->second.currentValue = optDisplayFrequency->second.possibleValues[0];
            }
            else
            {
                optVideoMode->second.currentValue = StringConverter::toString(mVideoModes[0].first.first,4) + " x " + StringConverter::toString(mVideoModes[0].first.second,4);
                optDisplayFrequency->second.currentValue = StringConverter::toString(mVideoModes[0].second) + " MHz";
            }
        }
    }

    void EGLSupport::setConfigOption(const String &name, const String &value)
    {
        GLESSupport::setConfigOption(name, value);
        if (name == "Video Mode")
        {
            refreshConfig();
        }
    }

    String EGLSupport::validateConfig(void)
    {
        // TODO
        return StringUtil::BLANK;
    }

	NativeDisplayType EGLSupport::getNativeDisplay()
	{
		return EGL_DEFAULT_DISPLAY; // TODO
	}

    EGLDisplay EGLSupport::getGLDisplay(void)
    {
        if (!mGLDisplay)
        {
            EGLint major, minor;

            mNativeDisplay = getNativeDisplay();

			mGLDisplay = eglGetDisplay(mNativeDisplay);

            if(mGLDisplay == EGL_NO_DISPLAY)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Couldn`t open X display " + getDisplayName(),
                            "EGLSupport::getGLDisplay");
            }

            if (eglInitialize(mGLDisplay, &major, &minor) == EGL_FALSE)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Couldn`t initialize EGLDispaly ",
                            "EGLSupport::getGLDisplay");
            }
        }
		return mGLDisplay;
    }


    String EGLSupport::getDisplayName(void)
    {
		return "todo";
	}

    EGLConfig* EGLSupport::chooseGLConfig(const GLint *attribList, GLint *nElements)
    {
        EGLConfig *configs;

        if (eglChooseConfig(mGLDisplay, attribList, NULL, 0, nElements) == EGL_FALSE)
        {
            assert(false);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to chosse config",
                        __FUNCTION__);

            *nElements = 0;
            return None;
		}

        configs = (EGLConfig*) malloc(*nElements * sizeof(EGLConfig));
        if (eglChooseConfig(mGLDisplay, attribList, configs, *nElements, nElements) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to chosse config",
                        __FUNCTION__);

            *nElements = 0;
            free(configs);
			return None;
        }

        return configs;
    }

    EGLBoolean EGLSupport::getGLConfigAttrib(EGLConfig glConfig, GLint attribute, GLint *value)
    {
        EGLBoolean status;

        status = eglGetConfigAttrib(mGLDisplay, glConfig, attribute, value);

        return status;
    }

    void* EGLSupport::getProcAddress(const Ogre::String& name)
    {
        return (void*)eglGetProcAddress((const char*) name.c_str());
    }

    ::EGLConfig EGLSupport::getGLConfigFromContext(::EGLContext context)
    {
        ::EGLConfig glConfig = 0;

        if (eglQueryContext(mGLDisplay, context, EGL_CONFIG_ID, (EGLint *) &glConfig) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to get config from context",
                        __FUNCTION__);
            return 0;
        }

        return glConfig;
    }

    ::EGLConfig EGLSupport::getGLConfigFromDrawable(::EGLSurface drawable,
                                                    unsigned int *w, unsigned int *h)
    {
        ::EGLConfig glConfig = 0;

        if (eglQuerySurface(mGLDisplay, drawable, EGL_CONFIG_ID, (EGLint *) &glConfig) == EGL_FALSE)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to get config from drawable",
                        __FUNCTION__);
            return 0;
        }

        eglQuerySurface(mGLDisplay, drawable, EGL_WIDTH, (EGLint *) w);
        eglQuerySurface(mGLDisplay, drawable, EGL_HEIGHT, (EGLint *) h);

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
                fields[EGL_CONFIG_CAVEAT] = EGL_NONE;

                for (int i = 0; attribs[2*i] != EGL_NONE; i++)
                {
                    fields[attribs[2*i]] = attribs[2*i+1];
                }
            }

            void load(EGLSupport* const glSupport, EGLConfig glConfig)
            {
                std::map<int,int>::iterator it;

                for (it = fields.begin(); it != fields.end(); it++)
                {
                    it->second = EGL_NONE;

                    glSupport->getGLConfigAttrib(glConfig, it->first, &it->second);
                }
            }

            bool operator>(GLConfigAttribs& alternative)
            {
                // Caveats are best avoided, but might be needed for anti-aliasing
                if (fields[EGL_CONFIG_CAVEAT] != alternative.fields[EGL_CONFIG_CAVEAT])
                {
                    if (fields[EGL_CONFIG_CAVEAT] == EGL_SLOW_CONFIG)
                    {
                        return false;
                    }

                    if (fields.find(EGL_SAMPLES) != fields.end() &&
                        fields[EGL_SAMPLES] < alternative.fields[EGL_SAMPLES])
                    {
                        return false;
                    }
                }

                std::map<int,int>::iterator it;

                for (it = fields.begin(); it != fields.end(); it++)
                {
                    if (it->first != EGL_CONFIG_CAVEAT &&
                        fields[it->first] > alternative.fields[it->first])
                    {
                        return true;
                    }
                }

                return false;
            }

            std::map<int,int> fields;
    };

    ::EGLConfig EGLSupport::selectGLConfig(const int* minAttribs, const int *maxAttribs)
    {
        EGLConfig *glConfigs;
        EGLConfig glConfig = 0;
        int config, nConfigs = 0;

        glConfigs = chooseGLConfig(minAttribs, &nConfigs);

        if (!nConfigs)
        {
            return 0;
        }

        glConfig = glConfigs[0];

        if (maxAttribs)
        {
            GLConfigAttribs maximum(maxAttribs);
            GLConfigAttribs best(maxAttribs);
            GLConfigAttribs candidate(maxAttribs);

            best.load(this, glConfig);

            for (config = 1; config < nConfigs; config++)
            {
                candidate.load(this, glConfigs[config]);

                if (candidate > maximum)
                {
                    continue;
                }

                if (candidate > best)
                {
                    glConfig = glConfigs[config];

                    best.load(this, glConfig);
                }
            }
        }

        free(glConfigs);
        return glConfig;
    }

    void EGLSupport::switchMode(void)
    {
        return switchMode(mOriginalMode.first.first,
                          mOriginalMode.first.second, mOriginalMode.second);
    }

    RenderWindow* EGLSupport::createWindow(bool autoCreateWindow,
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
            uint w = 640, h = 480;

            if ((opt = mOptions.find("Full Screen")) != end)
            {
                fullscreen = (opt->second.currentValue == "Yes");
            }

            if ((opt = mOptions.find("Display Frequency")) != end)
            {
                miscParams["displayFrequency"] = opt->second.currentValue;
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

    RenderWindow* EGLSupport::newWindow(const String &name,
                                        unsigned int width, unsigned int height,
                                        bool fullScreen,
                                        const NameValuePairList *miscParams)
    {
        EGLWindow* window = createEGLWindow(this);

        window->create(name, width, height, fullScreen, miscParams);

        return window;
    }

    ::EGLContext EGLSupport::createNewContext(EGLDisplay eglDisplay,
											  ::EGLConfig glconfig,
                                              ::EGLContext shareList) const 
    {
		::EGLContext context = ((::EGLContext) 0);
		if (eglDisplay == ((EGLDisplay) 0))
		{
			context = eglCreateContext(mGLDisplay, glconfig, shareList, 0);
		}
		else
		{
			context = eglCreateContext(eglDisplay, glconfig, 0, 0);
		}

		if (context == ((::EGLContext) 0))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Fail to create New context",
                        __FUNCTION__);
            return 0;
        }

        return context;
    }

    void EGLSupport::start()
    {
    }

    void EGLSupport::stop()
    {
    }

	void EGLSupport::setGLDisplay( EGLDisplay val )
	{
		mGLDisplay = val;
	}
}
