/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreGLESPrerequisites.h"
#include "OgreGLESRenderSystem.h"

#include "OgreSymbianEGLSupport.h"
#include "OgreSymbianEGLWindow.h"
#include "OgreSymbianEGLRenderTexture.h"
#include "OgreSymbianEGLContext.h"

#include <e32base.h> // for Symbian classes.
#include <coemain.h> // for CCoeEnv.


namespace Ogre {


	SymbianEGLSupport::SymbianEGLSupport()
	{

		TSizeMode sizeMode = CCoeEnv::Static()->ScreenDevice()->GetCurrentScreenModeAttributes();

		//RECT windowRect;
		//GetClientRect(mNativeDisplay, &windowRect);
		mCurrentMode.first.first = sizeMode.iScreenSize.iWidth; 
		mCurrentMode.first.second = sizeMode.iScreenSize.iHeight; 
		mCurrentMode.second = 0;
		mOriginalMode = mCurrentMode;
		mVideoModes.push_back(mCurrentMode);
/*
		EGLConfig *glConfigs;
		int config, nConfigs = 0;

		glConfigs = chooseGLConfig(NULL, &nConfigs);

		for (config = 0; config < nConfigs; config++)
		{
			int caveat, samples;

			getGLConfigAttrib(glConfigs[config], EGL_CONFIG_CAVEAT, &caveat);

			if (caveat != EGL_SLOW_CONFIG)
			{
				getGLConfigAttrib(glConfigs[config], EGL_SAMPLES, &samples);
				mSampleLevels.push_back(StringConverter::toString(samples));
			}
		}

		free(glConfigs);

		removeDuplicates(mSampleLevels);
*/




		mNativeDisplay = EGL_DEFAULT_DISPLAY;
		mGLDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
		if( mGLDisplay != NULL )
		{            
			// Initialize EGL.
			if( eglInitialize( mGLDisplay, NULL, NULL ) == EGL_FALSE )
			{      
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"EGL Initialize failed!",
					"SymbianEGLSupport::SymbianEGLSupport");

			}  
		}



	}

	SymbianEGLSupport::~SymbianEGLSupport()
	{


	}

	EGLWindow* SymbianEGLSupport::createEGLWindow(  EGLSupport * support )
	{
		return new SymbianEGLWindow(support);
	}

	GLESPBuffer* SymbianEGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
	{
		return new SymbianEGLPBuffer(this, format, width, height);
	}

	void SymbianEGLSupport::switchMode( uint& width, uint& height, short& frequency )
	{
		if (!mRandr)
			return;

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
				if (!newMode ||
					mode->first.first < newMode->first.first ||
					mode->first.second < newMode->first.second)
				{
					newSize = size;
					newMode = &(*mode);
				}
			}

			VideoMode* lastMode = &(*mode);

			while (++mode != end && mode->first == lastMode->first)
			{
				if (lastMode == newMode && mode->second == frequency)
				{
					newMode = &(*mode);
				}
			}
		}

		//todo
	}

	RenderWindow* SymbianEGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, bool fullScreen, const NameValuePairList *miscParams /*= 0*/ )
	{
		SymbianEGLWindow* window = new SymbianEGLWindow(this);
		window->create(name, width, height, fullScreen, miscParams);

		return window;
	}
}
