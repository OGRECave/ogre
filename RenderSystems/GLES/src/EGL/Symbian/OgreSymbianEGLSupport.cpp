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

#include "OgreSymbianEGLSupport.h"
#include "OgreSymbianEGLWindow.h"
#include "OgreSymbianEGLRenderTexture.h"
#include "OgreSymbianEGLContext.h"

#ifdef __GCCE__
#include <staticlibinit_gcce.h>
#endif

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
		//eglInitialize( mGLDisplay, NULL, NULL ) ; - not possible here




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
