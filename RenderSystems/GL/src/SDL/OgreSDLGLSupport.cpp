#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreSDLGLSupport.h"

#include "OgreSDLWindow.h"

using namespace Ogre;

SDLGLSupport::SDLGLSupport()
{

    SDL_Init(SDL_INIT_VIDEO);
}

SDLGLSupport::~SDLGLSupport()
{
}

void SDLGLSupport::addConfig(void)
{
    mVideoModes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
    
    if (mVideoModes == (SDL_Rect **)0)
    {
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to load video modes",
                "SDLRenderSystem::initConfigOptions");
    }

    ConfigOption optFullScreen;
    ConfigOption optVideoMode;
    ConfigOption optFSAA;
	ConfigOption optRTTMode;
	ConfigOption optEnableFixedPipeline;

    // FS setting possibilities
    optFullScreen.name = "Full Screen";
    optFullScreen.possibleValues.push_back("Yes");
    optFullScreen.possibleValues.push_back("No");
    optFullScreen.currentValue = "Yes";
    optFullScreen.immutable = false;

    // Video mode possibilities
    optVideoMode.name = "Video Mode";
    optVideoMode.immutable = false;
    for (size_t i = 0; mVideoModes[i]; i++)
    {
        char szBuf[16];
		snprintf(szBuf, 16, "%d x %d", mVideoModes[i]->w, mVideoModes[i]->h);
        optVideoMode.possibleValues.push_back(szBuf);
        // Make the first one default
        if (i == 0)
        {
            optVideoMode.currentValue = szBuf;
        }
    }
    
    //FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back("0");
    optFSAA.possibleValues.push_back("2");
    optFSAA.possibleValues.push_back("4");
    optFSAA.possibleValues.push_back("6");
    optFSAA.currentValue = "0";
    optFSAA.immutable = false;

	optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back("FBO");
	optRTTMode.possibleValues.push_back("PBuffer");
	optRTTMode.possibleValues.push_back("Copy");
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;

		optEnableFixedPipeline.name = "Fixed Pipeline Enabled";
		optEnableFixedPipeline.possibleValues.push_back( "Yes" );
		optEnableFixedPipeline.possibleValues.push_back( "No" );
		optEnableFixedPipeline.currentValue = "Yes";
		optEnableFixedPipeline.immutable = false;

    mOptions[optFullScreen.name] = optFullScreen;
    mOptions[optVideoMode.name] = optVideoMode;
    mOptions[optFSAA.name] = optFSAA;
	mOptions[optRTTMode.name] = optRTTMode;
		mOptions[optEnableFixedPipeline.name] = optEnableFixedPipeline;

}

String SDLGLSupport::validateConfig(void)
{
    return String("");
}

RenderWindow* SDLGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle)
{
	if (autoCreateWindow)
    {
        ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "SDLGLSupport::createWindow");
        bool fullscreen = (opt->second.currentValue == "Yes");

        opt = mOptions.find("Video Mode");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "SDLGLSupport::createWindow");
        String val = opt->second.currentValue;
        String::size_type pos = val.find('x');
        if (pos == String::npos)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "SDLGLSupport::createWindow");

		// Parse FSAA config
		NameValuePairList winOptions;
		winOptions["title"] = windowTitle;
        int fsaa_x_samples = 0;
        opt = mOptions.find("FSAA");
        if(opt != mOptions.end())
        {
			winOptions["FSAA"] = opt->second.currentValue;
        }

			opt = mOptions.find("Fixed Pipeline Enabled");
			if (opt == mOptions.end())
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find Fixed Pipeline enabled options!", "Win32GLSupport::createWindow");
			bool enableFixedPipeline = (opt->second.currentValue == "Yes");
			renderSystem->setFixedPipelineEnabled(enableFixedPipeline);

        unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
        unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

        const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
        return renderSystem->createRenderWindow(windowTitle, w, h, fullscreen, &winOptions);
    }
    else
    {
        // XXX What is the else?
		return NULL;
    }

}


RenderWindow* SDLGLSupport::newWindow(const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams)
{
	SDLWindow* window = new SDLWindow();
	window->create(name, width, height, fullScreen, miscParams);
	return window;
}

void SDLGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting SDL Subsystem ***\n"
        "******************************");

    SDL_Init(SDL_INIT_VIDEO);
}

void SDLGLSupport::stop()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Stopping SDL Subsystem ***\n"
        "******************************");

    SDL_Quit();
}

void* SDLGLSupport::getProcAddress(const String& procname)
{
    return SDL_GL_GetProcAddress(procname.c_str());
}
