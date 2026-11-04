/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#import "OgreException.h"
#import "OgreLogManager.h"
#import "OgreStringConverter.h"
#import "OgreRoot.h"

#import "OgreGLNativeSupport.h"
#import "OgreOSXGLSupport.h"
#import "OgreOSXCocoaWindow.h"
#import "OgreGLUtil.h"

#import <dlfcn.h>

#import <OpenGL/OpenGL.h>
#import <AppKit/NSScreen.h>

namespace Ogre {

GLNativeSupport* getGLSupport(int profile)
{
    return new OSXGLSupport(profile);
}

ConfigOptionMap OSXGLSupport::getConfigOptions()
{
	ConfigOptionMap mOptions;
	ConfigOption optBitDepth;
    ConfigOption optContentScalingFactor;

	optBitDepth.name = "Colour Depth";
	optBitDepth.currentValue = "32";
	optBitDepth.immutable = false;

    optContentScalingFactor.name = "Content Scaling Factor";
    optContentScalingFactor.possibleValues.push_back( "1.0" );
    optContentScalingFactor.possibleValues.push_back( "2.0" );
    optContentScalingFactor.currentValue = StringConverter::toString((float)[NSScreen mainScreen].backingScaleFactor);
    optContentScalingFactor.immutable = false;

	mOptions[ optBitDepth.name ] = optBitDepth;
    mOptions[ optContentScalingFactor.name ] = optContentScalingFactor;

	CGLRendererInfoObj rend;
	GLint nrend = 0, maxSamples = 0;

	CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
	CGLDescribeRenderer(rend, 0, kCGLRPMaxSamples, &maxSamples);
    CGLDestroyRendererInfo(rend);

    // FSAA possibilities
    for(int i = 0; i <= maxSamples; i += 2)
        mFSAALevels.push_back( i );

	return mOptions;
}

RenderWindow* OSXGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams )
{
    NameValuePairList params;
    if(miscParams)
        params = *miscParams;
    params["contextProfile"] = StringConverter::toString(int(mContextProfile));

	// Create the window, if Cocoa return a Cocoa window
    LogManager::getSingleton().logMessage("Creating a Cocoa Compatible Render System");
    CocoaWindow *window = OGRE_NEW CocoaWindow();
    window->create(name, width, height, fullScreen, &params);

    return window;
}

void OSXGLSupport::start()
{
	LogManager::getSingleton().logMessage(
			"********************************************\n"
			"***  Starting Mac OS X OpenGL Subsystem  ***\n"
			"********************************************");
}

void OSXGLSupport::stop()
{
	LogManager::getSingleton().logMessage(
			"********************************************\n"
			"***  Stopping Mac OS X OpenGL Subsystem  ***\n"
			"********************************************");
}

void* OSXGLSupport::getProcAddress( const char* name ) const
{
    return dlsym (RTLD_DEFAULT, name);
}

Boolean OSXGLSupport::_getDictionaryBoolean(CFDictionaryRef dict, const void* key)
{
	Boolean value = false;
	CFBooleanRef boolRef;
	boolRef = (CFBooleanRef)CFDictionaryGetValue(dict, key);
	
	if (boolRef != NULL)
		value = CFBooleanGetValue(boolRef); 	
		
	return value;
}

long OSXGLSupport::_getDictionaryLong(CFDictionaryRef dict, const void* key)
{
	long value = 0;
	CFNumberRef numRef;
	numRef = (CFNumberRef)CFDictionaryGetValue(dict, key);
	
	if (numRef != NULL)
		CFNumberGetValue(numRef, kCFNumberLongType, &value);	
		
	return value;
}

}
