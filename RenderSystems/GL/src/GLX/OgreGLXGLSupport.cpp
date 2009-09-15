/*
-----------------------------------------------------------------------------
This source file is part of OGRE
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include "OgreGLXGLSupport.h"
#include "OgreGLXUtils.h"
#include "OgreGLXWindow.h"
#include "OgreGLXRenderTexture.h"

#include <X11/extensions/Xrandr.h>

GLenum glxewContextInit(Ogre::GLSupport *glSupport);

static Display *_currentDisplay;
static Display *_getCurrentDisplay(void) { return _currentDisplay; }

namespace Ogre 
{
	//-------------------------------------------------------------------------------------------------//
	template<class C> void remove_duplicates(C& c)
	{
		std::sort(c.begin(), c.end());
		typename C::iterator p = std::unique(c.begin(), c.end());
		c.erase(p, c.end());
	}

	//-------------------------------------------------------------------------------------------------//
	GLXGLSupport::GLXGLSupport() : mGLDisplay(0), mXDisplay(0)
	{
		// A connection that might be shared with the application for GL rendering:
		mGLDisplay = getGLDisplay();
		
		// A connection that is NOT shared to enable independent event processing:
		mXDisplay  = getXDisplay();
		
		int dummy;
		
		if (XQueryExtension(mXDisplay, "RANDR", &dummy, &dummy, &dummy))
		{
			XRRScreenConfiguration *screenConfig;
			
			screenConfig = XRRGetScreenInfo(mXDisplay, DefaultRootWindow(mXDisplay));
			
			if (screenConfig) 
			{
				XRRScreenSize *screenSizes;
				int nSizes = 0;
				Rotation currentRotation;
				int currentSizeID = XRRConfigCurrentConfiguration(screenConfig, &currentRotation);
				
				screenSizes = XRRConfigSizes(screenConfig, &nSizes);
				
				mCurrentMode.first.first = screenSizes[currentSizeID].width;
				mCurrentMode.first.second = screenSizes[currentSizeID].height;
				mCurrentMode.second = XRRConfigCurrentRate(screenConfig);
				
				mOriginalMode = mCurrentMode;
				
				for(int sizeID = 0; sizeID < nSizes; sizeID++) 
				{
					short *rates;
					int nRates = 0;
					
					rates = XRRConfigRates(screenConfig, sizeID, &nRates);
					
					for (int rate = 0; rate < nRates; rate++)
					{
						VideoMode mode;
						
						mode.first.first = screenSizes[sizeID].width;
						mode.first.second = screenSizes[sizeID].height;
						mode.second = rates[rate];
						
						mVideoModes.push_back(mode);
					}
				}
				XRRFreeScreenConfigInfo(screenConfig);
			}
		}
		else
		{
			mCurrentMode.first.first = DisplayWidth(mXDisplay, DefaultScreen(mXDisplay));
			mCurrentMode.first.second = DisplayHeight(mXDisplay, DefaultScreen(mXDisplay));
			mCurrentMode.second = 0;
			
			mOriginalMode = mCurrentMode;
			
			mVideoModes.push_back(mCurrentMode);
		}
		
		GLXFBConfig *fbConfigs;
		int config, nConfigs = 0;
		
		fbConfigs = chooseFBConfig(NULL, &nConfigs);
		
		for (config = 0; config < nConfigs; config++)
		{
			int caveat, samples;
			
			getFBConfigAttrib (fbConfigs[config], GLX_CONFIG_CAVEAT, &caveat);
			
			if (caveat != GLX_SLOW_CONFIG)
			{
				getFBConfigAttrib (fbConfigs[config], GLX_SAMPLES, &samples);
				mSampleLevels.push_back(StringConverter::toString(samples));
			}
		}
		
		XFree (fbConfigs);
		
		remove_duplicates(mSampleLevels);
	}

	//-------------------------------------------------------------------------------------------------//
	GLXGLSupport::~GLXGLSupport() 
	{
		if (mXDisplay)
			XCloseDisplay(mXDisplay);
		
		if (! mIsExternalDisplay && mGLDisplay)
			XCloseDisplay(mGLDisplay);
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::addConfig(void) 
	{
		ConfigOption optFullScreen;
		ConfigOption optVideoMode;
		ConfigOption optDisplayFrequency;
		ConfigOption optVSync;
		ConfigOption optFSAA;
		ConfigOption optRTTMode;
		ConfigOption optSRGB;
		
		optFullScreen.name = "Full Screen";
		optFullScreen.immutable = false;
		
		optVideoMode.name = "Video Mode";
		optVideoMode.immutable = false;
		
		optDisplayFrequency.name = "Display Frequency";
		optDisplayFrequency.immutable = false;
		
		optVSync.name = "VSync";
		optVSync.immutable = false;
		
		optFSAA.name = "FSAA";
		optFSAA.immutable = false;
		
		optRTTMode.name = "RTT Preferred Mode";
		optRTTMode.immutable = false;
		
		optSRGB.name = "sRGB Gamma Conversion";
		optSRGB.immutable = false;

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
		
		remove_duplicates(optVideoMode.possibleValues);
		
		optVideoMode.currentValue = StringConverter::toString(mCurrentMode.first.first,4) + " x " + StringConverter::toString(mCurrentMode.first.second,4);
		
		refreshConfig();
		
		if (GLXEW_SGI_swap_control)
		{
			optVSync.possibleValues.push_back("No");
			optVSync.possibleValues.push_back("Yes");
			
			optVSync.currentValue = optVSync.possibleValues[0];
		}
		
		// Is it worth checking for GL_EXT_framebuffer_object ?
		optRTTMode.possibleValues.push_back("FBO");
		
		if (GLXEW_VERSION_1_3)
		{
			optRTTMode.possibleValues.push_back("PBuffer");
		}
		
		optRTTMode.possibleValues.push_back("Copy");
		
		optRTTMode.currentValue = optRTTMode.possibleValues[0];
		
		if (! mSampleLevels.empty())
		{
			StringVector::const_iterator value = mSampleLevels.begin();
			StringVector::const_iterator end = mSampleLevels.end();
			
			for (; value != end; value++)
			{
				optFSAA.possibleValues.push_back(*value);
			}
			
			optFSAA.currentValue = optFSAA.possibleValues[0];
		}
		
		if (GLXEW_EXT_framebuffer_sRGB)
		{	
			optSRGB.possibleValues.push_back("No");
			optSRGB.possibleValues.push_back("Yes");

			optSRGB.currentValue = optSRGB.possibleValues[0];
		}

		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVideoMode.name] = optVideoMode;
		mOptions[optDisplayFrequency.name] = optDisplayFrequency;
		mOptions[optVSync.name] = optVSync;
		mOptions[optRTTMode.name] = optRTTMode;
		mOptions[optFSAA.name] = optFSAA;
		mOptions[optSRGB.name] = optSRGB;
		
		refreshConfig();
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::refreshConfig(void) 
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
			
			if (! optDisplayFrequency->second.possibleValues.empty())
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
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::setConfigOption(const String &name, const String &value)
	{
		ConfigOptionMap::iterator option = mOptions.find(name);
		
		if(option == mOptions.end())
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Option named " + name + " does not exist.", "GLXGLSupport::setConfigOption" );
		}
		else
		{
			option->second.currentValue = value;
		}
		
		if (name == "Video Mode")
		{
			ConfigOptionMap::iterator opt;
			if((opt = mOptions.find("Full Screen")) != mOptions.end())
			{
				if (opt->second.currentValue == "Yes")
					refreshConfig();
			}
		}
	}

	//-------------------------------------------------------------------------------------------------//
	String GLXGLSupport::validateConfig(void) 
	{
		// TO DO
		return StringUtil::BLANK;
	}

	//-------------------------------------------------------------------------------------------------//
	RenderWindow* GLXGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle) 
	{
		RenderWindow *window = 0;
		
		if (autoCreateWindow) 
		{
			ConfigOptionMap::iterator opt;
			ConfigOptionMap::iterator end = mOptions.end();
			NameValuePairList miscParams;
			
			bool fullscreen = false;
			uint w = 800, h = 600;
			
			if((opt = mOptions.find("Full Screen")) != end)
				fullscreen = (opt->second.currentValue == "Yes");
			
			if((opt = mOptions.find("Display Frequency")) != end)
				miscParams["displayFrequency"] = opt->second.currentValue;
			
			if((opt = mOptions.find("Video Mode")) != end)
			{
				String val = opt->second.currentValue;
				String::size_type pos = val.find('x');
				
				if (pos != String::npos)
				{
					w = StringConverter::parseUnsignedInt(val.substr(0, pos));
					h = StringConverter::parseUnsignedInt(val.substr(pos + 1));
				}
			}
			
			if((opt = mOptions.find("FSAA")) != end)
				miscParams["FSAA"] = opt->second.currentValue;
			
			if((opt = mOptions.find("VSync")) != end)
				miscParams["vsync"] = opt->second.currentValue;

			if((opt = mOptions.find("sRGB Gamma Conversion")) != end)
				miscParams["gamma"] = opt->second.currentValue;
			
			window = renderSystem->_createRenderWindow(windowTitle, w, h, fullscreen, &miscParams);
		} 
		
		return window;
	}
	
	//-------------------------------------------------------------------------------------------------//
	RenderWindow* GLXGLSupport::newWindow(const String &name, unsigned int width, unsigned int height, bool fullScreen, const NameValuePairList *miscParams)
	{
		GLXWindow* window = new GLXWindow(this);
		
		window->create(name, width, height, fullScreen, miscParams);
		
		return window;
	}

	//-------------------------------------------------------------------------------------------------//
	GLPBuffer *GLXGLSupport::createPBuffer(PixelComponentType format, size_t width, size_t height)
	{
		return new GLXPBuffer(this, format, width, height);
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::start() 
	{
		LogManager::getSingleton().logMessage(
			"******************************\n"
			"*** Starting GLX Subsystem ***\n"
			"******************************");
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::stop() 
	{
		LogManager::getSingleton().logMessage(
			"******************************\n"
			"*** Stopping GLX Subsystem ***\n"
			"******************************");
	}

	//-------------------------------------------------------------------------------------------------//
	void* GLXGLSupport::getProcAddress(const String& procname) {
		return (void*)glXGetProcAddressARB((const GLubyte*)procname.c_str());
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::initialiseExtensions(void)
	{
		assert (mGLDisplay);
		
		GLSupport::initialiseExtensions();
		
		const char* extensionsString;
		
		// This is more realistic than using glXGetClientString:
		extensionsString = glXQueryExtensionsString(mGLDisplay, DefaultScreen(mGLDisplay));
		
		LogManager::getSingleton().stream() << "Supported GLX extensions: " << extensionsString;
		
		std::stringstream ext;
		String instr;
		
		ext << extensionsString;
		
		while(ext >> instr)
		{
			extensionList.insert(instr);
		}
	}

	//-------------------------------------------------------------------------------------------------//
	// Returns the FBConfig behind a GLXContext
   
	GLXFBConfig GLXGLSupport::getFBConfigFromContext(::GLXContext context)
	{
		GLXFBConfig fbConfig = 0;
		
		if (GLXEW_VERSION_1_3)
		{
			int fbConfigAttrib[] = {
				GLX_FBCONFIG_ID, 0, 
				None
			};
			GLXFBConfig *fbConfigs;
			int nElements = 0;
			
			glXQueryContext(mGLDisplay, context, GLX_FBCONFIG_ID, &fbConfigAttrib[1]);
			fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), fbConfigAttrib, &nElements);
			
			if (nElements)
			{
				fbConfig = fbConfigs[0];
				XFree(fbConfigs);
			}
		}
		else if (GLXEW_EXT_import_context && GLXEW_SGIX_fbconfig)
		{
			VisualID visualid;
			
			if (glXQueryContextInfoEXT(mGLDisplay, context, GLX_VISUAL_ID, (int*)&visualid))
			{
				fbConfig = getFBConfigFromVisualID(visualid);
			}
		}
	
		return fbConfig;
	}
	
	//-------------------------------------------------------------------------------------------------//
	// Returns the FBConfig behind a GLXDrawable, or returns 0 when 
	//   missing GLX_SGIX_fbconfig and drawable is Window (unlikely), OR
	//   missing GLX_VERSION_1_3 and drawable is a GLXPixmap (possible).
	
	GLXFBConfig GLXGLSupport::getFBConfigFromDrawable(GLXDrawable drawable, unsigned int *width, unsigned int *height)
	{
		GLXFBConfig fbConfig = 0;
		
		if (GLXEW_VERSION_1_3)
		{
			int fbConfigAttrib[] = {
				GLX_FBCONFIG_ID, 0, 
				None
			};
			GLXFBConfig *fbConfigs;
			int nElements = 0;
			
			glXQueryDrawable (mGLDisplay, drawable, GLX_FBCONFIG_ID, (unsigned int*)&fbConfigAttrib[1]);
			
			fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), fbConfigAttrib, &nElements);
			
			if (nElements)
			{
				fbConfig = fbConfigs[0];
				XFree (fbConfigs);
				
				glXQueryDrawable(mGLDisplay, drawable, GLX_WIDTH, width);
				glXQueryDrawable(mGLDisplay, drawable, GLX_HEIGHT, height);
			}
		}
		
		if (! fbConfig && GLXEW_SGIX_fbconfig)
		{
			XWindowAttributes windowAttrib;
			
			if (XGetWindowAttributes(mGLDisplay, drawable, &windowAttrib))
			{
				VisualID visualid = XVisualIDFromVisual(windowAttrib.visual);
				
				fbConfig = getFBConfigFromVisualID(visualid);
				
				*width = windowAttrib.width;
				*height = windowAttrib.height;
			}
		}
		
		return fbConfig;
	}

	//-------------------------------------------------------------------------------------------------//
	// Finds a GLXFBConfig compatible with a given VisualID.

	GLXFBConfig GLXGLSupport::getFBConfigFromVisualID(VisualID visualid)
	{
		GLXFBConfig fbConfig = 0;
		
		if (GLXEW_SGIX_fbconfig && glXGetFBConfigFromVisualSGIX)
		{
			XVisualInfo visualInfo;
			
			visualInfo.screen = DefaultScreen(mGLDisplay);
			visualInfo.depth = DefaultDepth(mGLDisplay, DefaultScreen(mGLDisplay));
			visualInfo.visualid = visualid;
			
			fbConfig = glXGetFBConfigFromVisualSGIX(mGLDisplay, &visualInfo);
		}
		
		if (! fbConfig)
		{
			int minAttribs[] = {
				GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT || GLX_PIXMAP_BIT,
				GLX_RENDER_TYPE,	GLX_RGBA_BIT,
				GLX_RED_SIZE,	   1,
				GLX_BLUE_SIZE,	  1,
				GLX_GREEN_SIZE,	 1,
				None
			};
			int nConfigs = 0;
			
			GLXFBConfig *fbConfigs = chooseFBConfig(minAttribs, &nConfigs);
			
			for (int i = 0; i < nConfigs && ! fbConfig; i++)
			{
				XVisualInfo *visualInfo = getVisualFromFBConfig(fbConfigs[i]);
				
				if (visualInfo->visualid == visualid)
					fbConfig = fbConfigs[i];
				
				XFree(visualInfo);
			}

			XFree(fbConfigs);
		}
		
		return fbConfig;
	}

	//-------------------------------------------------------------------------------------------------//
	// A helper class for the implementation of selectFBConfig

	class FBConfigAttribs
	{
	public:
		FBConfigAttribs(const int* attribs)
		{
			fields[GLX_CONFIG_CAVEAT] = GLX_NONE;
			
			for (int i = 0; attribs[2*i]; i++)
			{
				fields[attribs[2*i]] = attribs[2*i+1];
			}
		}

		void load(GLXGLSupport* const glSupport, GLXFBConfig fbConfig)
		{
			std::map<int,int>::iterator it;
			
			for (it = fields.begin(); it != fields.end(); it++)
			{
				it->second = 0;

				glSupport->getFBConfigAttrib(fbConfig, it->first, &it->second);
			}
		}

		bool operator>(FBConfigAttribs& alternative)
		{
			// Caveats are best avoided, but might be needed for anti-aliasing
			
			if (fields[GLX_CONFIG_CAVEAT] != alternative.fields[GLX_CONFIG_CAVEAT])
			{
				if (fields[GLX_CONFIG_CAVEAT] == GLX_SLOW_CONFIG)
					return false;
				
				if (fields.find(GLX_SAMPLES) != fields.end() && 
					fields[GLX_SAMPLES] < alternative.fields[GLX_SAMPLES])
					return false;
			}
			
			std::map<int,int>::iterator it;
			
			for (it = fields.begin(); it != fields.end(); it++)
			{
				if (it->first != GLX_CONFIG_CAVEAT && fields[it->first] > alternative.fields[it->first])
					return true;
			}
			
			return false;
		}
		
		std::map<int,int> fields;
	};

	//-------------------------------------------------------------------------------------------------//
	// Finds an FBConfig that possesses each of minAttribs and gets as close 
	// as possible to each of the maxAttribs without exceeding them.
	// Resembles glXChooseFBConfig, but is forgiving to platforms
	// that do not support the attributes listed in the maxAttribs.
	
	GLXFBConfig GLXGLSupport::selectFBConfig (const int* minAttribs, const int *maxAttribs)
	{
		GLXFBConfig *fbConfigs;
		GLXFBConfig fbConfig = 0;
		int config, nConfigs = 0;
		
		fbConfigs = chooseFBConfig(minAttribs, &nConfigs);
		
		if (! nConfigs) 
			return 0;
		
		fbConfig = fbConfigs[0];
		
		if (maxAttribs)
		{
			FBConfigAttribs maximum(maxAttribs);
			FBConfigAttribs best(maxAttribs);
			FBConfigAttribs candidate(maxAttribs);
			
			best.load(this, fbConfig);
			
			for (config = 1; config < nConfigs; config++)
			{
				candidate.load(this, fbConfigs[config]);
				
				if (candidate > maximum)
					continue;
				
				if (candidate > best)
				{
					fbConfig = fbConfigs[config];		
					
					best.load(this, fbConfig);
				}
			}
		}

		XFree (fbConfigs);
		return fbConfig;
	}

	//-------------------------------------------------------------------------------------------------//
	bool GLXGLSupport::loadIcon(const std::string &name, Pixmap *pixmap, Pixmap *bitmap)
	{
		Image image;
		int width, height;
		char* imageData;
		
		try 
		{
			// Try to load image
			image.load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			
			if(image.getFormat() != PF_A8R8G8B8)
			{
				// Image format must be RGBA
				return false;
			}
			
			width  = image.getWidth();
			height = image.getHeight();
			imageData = (char*)image.getData();
		} 
		catch(Exception &e) 
		{
			// Could not find image; never mind
			return false;
		}
	
		int bitmapLineLength = (width + 7) / 8;
		int pixmapLineLength = 4 * width;
		
		char* bitmapData = (char*)malloc(bitmapLineLength * height);
		char* pixmapData = (char*)malloc(pixmapLineLength * height);
		
		int sptr = 0, dptr = 0;
		
		for(int y = 0; y < height; y++) 
		{
			for(int x = 0; x < width; x++) 
			{
				if (ImageByteOrder(mXDisplay) == MSBFirst)
				{
					pixmapData[dptr + 0] = 0;
					pixmapData[dptr + 1] = imageData[sptr + 0];
					pixmapData[dptr + 2] = imageData[sptr + 1];
					pixmapData[dptr + 3] = imageData[sptr + 2];
				}
				else
				{
					pixmapData[dptr + 3] = 0;
					pixmapData[dptr + 2] = imageData[sptr + 0];
					pixmapData[dptr + 1] = imageData[sptr + 1];
					pixmapData[dptr + 0] = imageData[sptr + 2];
				}
				
				if(((unsigned char)imageData[sptr + 3])<128) 
				{
					bitmapData[y*bitmapLineLength+(x>>3)] &= ~(1<<(x&7));
				} 
				else 
				{
					bitmapData[y*bitmapLineLength+(x>>3)] |= 1<<(x&7);
				}
				sptr += 4;
				dptr += 4;
			}
		}
		
		// Create bitmap on server and copy over bitmapData
		*bitmap = XCreateBitmapFromData(mXDisplay, DefaultRootWindow(mXDisplay), bitmapData, width, height);
		
		free(bitmapData);
		
		// Create pixmap on server and copy over pixmapData (via pixmapXImage)
		*pixmap = XCreatePixmap(mXDisplay, DefaultRootWindow(mXDisplay), width, height, 24);
		
		GC gc = XCreateGC (mXDisplay, DefaultRootWindow(mXDisplay), 0, NULL);
		XImage *pixmapXImage = XCreateImage(mXDisplay, NULL, 24, ZPixmap, 0, pixmapData, width, height, 8, width*4);
		XPutImage(mXDisplay, *pixmap, gc, pixmapXImage, 0, 0, 0, 0, width, height);
		XDestroyImage(pixmapXImage);
		XFreeGC(mXDisplay, gc);
		
		return true;
	}
	
	//-------------------------------------------------------------------------------------------------//
	Display* GLXGLSupport::getGLDisplay(void)
	{
		if (! mGLDisplay)
		{
			glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)getProcAddress("glXGetCurrentDisplay");
			
			mGLDisplay = glXGetCurrentDisplay();
			mIsExternalDisplay = true;
			
			if (! mGLDisplay)
			{
				mGLDisplay = XOpenDisplay(0);
				mIsExternalDisplay = false;
			}
			
			if(! mGLDisplay) 
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn`t open X display " + String((const char*)XDisplayName (0)), "GLXGLSupport::getGLDisplay");
			}
			
			initialiseGLXEW();
			
			if (! GLXEW_VERSION_1_3 && ! (GLXEW_SGIX_fbconfig && GLXEW_EXT_import_context))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "No GLX FBConfig support on your display", "GLXGLSupport::GLXGLSupport");
			}
		}
		
		return mGLDisplay;
	}

	//-------------------------------------------------------------------------------------------------//
	Display* GLXGLSupport::getXDisplay(void)
	{
		if (! mXDisplay)
		{
			char* displayString = mGLDisplay ? DisplayString(mGLDisplay) : 0;
			
			mXDisplay = XOpenDisplay(displayString);
			
			if (! mXDisplay)
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn`t open X display " + String((const char*)displayString), "GLXGLSupport::getXDisplay");
			}
			
			mAtomDeleteWindow = XInternAtom(mXDisplay, "WM_DELETE_WINDOW", True);
			mAtomFullScreen = XInternAtom(mXDisplay, "_NET_WM_STATE_FULLSCREEN", True);
			mAtomState = XInternAtom(mXDisplay, "_NET_WM_STATE", True); 
		}
		
		return mXDisplay;
	}

	//-------------------------------------------------------------------------------------------------//
	String GLXGLSupport::getDisplayName(void)
	{
		return String((const char*)XDisplayName(DisplayString(mGLDisplay)));
	}

	//-------------------------------------------------------------------------------------------------//
	GLXFBConfig* GLXGLSupport::chooseFBConfig(const GLint *attribList, GLint *nElements)
	{
		GLXFBConfig *fbConfigs;
		
		if (GLXEW_VERSION_1_3)
			fbConfigs = glXChooseFBConfig(mGLDisplay, DefaultScreen(mGLDisplay), attribList, nElements);
		else
			fbConfigs = glXChooseFBConfigSGIX(mGLDisplay, DefaultScreen(mGLDisplay), attribList, nElements);
		
		return fbConfigs;
	}
	
	//-------------------------------------------------------------------------------------------------//
	::GLXContext GLXGLSupport::createNewContext(GLXFBConfig fbConfig, GLint renderType, ::GLXContext shareList, GLboolean direct) const
	{
		::GLXContext glxContext;
		
		if (GLXEW_VERSION_1_3)
			glxContext = glXCreateNewContext(mGLDisplay, fbConfig, renderType, shareList, direct);
		else
			glxContext = glXCreateContextWithConfigSGIX(mGLDisplay, fbConfig, renderType, shareList, direct);
		
		return glxContext;
	}
	
	//-------------------------------------------------------------------------------------------------//
	GLint GLXGLSupport::getFBConfigAttrib(GLXFBConfig fbConfig, GLint attribute, GLint *value)
	{
		GLint status;
		
		if (GLXEW_VERSION_1_3)
			status = glXGetFBConfigAttrib(mGLDisplay, fbConfig, attribute, value);
		else
			status = glXGetFBConfigAttribSGIX(mGLDisplay, fbConfig, attribute, value);
		
		return status;
	}
	
	//-------------------------------------------------------------------------------------------------//
	XVisualInfo* GLXGLSupport::getVisualFromFBConfig(GLXFBConfig fbConfig)
	{
		XVisualInfo *visualInfo;
		
		if (GLXEW_VERSION_1_3)
			visualInfo = glXGetVisualFromFBConfig(mGLDisplay, fbConfig);
		else
			visualInfo = glXGetVisualFromFBConfigSGIX(mGLDisplay, fbConfig);
		
		return visualInfo;
	}

	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::switchMode(uint& width, uint& height, short& frequency)
	{
		int size = 0;
		int newSize = -1;
		
		VideoModes::iterator mode;
		VideoModes::iterator end = mVideoModes.end();
		VideoMode *newMode = 0;
		
		for(mode = mVideoModes.begin(); mode != end; size++)
		{
			if (mode->first.first >= static_cast<int>(width) &&
				mode->first.second >= static_cast<int>(height))
			{
				if (! newMode || 
					mode->first.first < newMode->first.first ||
					mode->first.second < newMode->first.second)
				{
					newSize = size;
					newMode = &(*mode);
				}
			}
			
			VideoMode *lastMode = &(*mode);
			
			while (++mode != end && mode->first == lastMode->first)
			{
				if (lastMode == newMode && mode->second == frequency)
				{
					newMode = &(*mode);
				}
			}
		}
		
		if (newMode && *newMode != mCurrentMode)
		{
			XRRScreenConfiguration *screenConfig = XRRGetScreenInfo (mXDisplay, DefaultRootWindow(mXDisplay)); 
			
			if (screenConfig)
			{
				Rotation currentRotation;
				
				XRRConfigCurrentConfiguration (screenConfig, &currentRotation);
				
				XRRSetScreenConfigAndRate(mXDisplay, screenConfig, DefaultRootWindow(mXDisplay), newSize, currentRotation, newMode->second, CurrentTime);
				
				XRRFreeScreenConfigInfo(screenConfig);
				
				mCurrentMode = *newMode;
				
				LogManager::getSingleton().logMessage("Entered video mode " + StringConverter::toString(mCurrentMode.first.first) + "x" + StringConverter::toString(mCurrentMode.first.second) + " @ " + StringConverter::toString(mCurrentMode.second) + "MHz");
			}
		}
	}
	
	//-------------------------------------------------------------------------------------------------//
	void GLXGLSupport::switchMode(void)
	{
		return switchMode(mOriginalMode.first.first, mOriginalMode.first.second, mOriginalMode.second);
	}

	//-------------------------------------------------------------------------------------------------//
	// Initialise GLXEW
	// 
	// Overloading glXGetCurrentDisplay allows us to call glxewContextInit
	// before establishing a GL context. This approach is a bit of a hack,
	// but it minimises the patches required between glew.c and glew.cpp.

	void GLXGLSupport::initialiseGLXEW(void)
	{
		_currentDisplay = mGLDisplay;
		
		glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)_getCurrentDisplay;
		
		if (glxewContextInit(this) != GLEW_OK)
		{
			XCloseDisplay (mGLDisplay);
			XCloseDisplay (mXDisplay);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "No GLX 1.1 support on your platform", "GLXGLSupport::initialiseGLXEW");
		}
		
		glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)getProcAddress("glXGetCurrentDisplay");
	}
}
