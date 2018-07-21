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
	ConfigOption optHiddenWindow;
    ConfigOption optContentScalingFactor;

    // Hidden window setting possibilities
	optHiddenWindow.name = "hidden";
	optHiddenWindow.possibleValues.push_back( "Yes" );
	optHiddenWindow.possibleValues.push_back( "No" );
	optHiddenWindow.currentValue = "No";
	optHiddenWindow.immutable = false;
	mOptions[optHiddenWindow.name] = optHiddenWindow;

	optBitDepth.name = "Colour Depth";
	optBitDepth.currentValue = "32";
	optBitDepth.immutable = false;

    optContentScalingFactor.name = "Content Scaling Factor";
    optContentScalingFactor.possibleValues.push_back( "1.0" );
    optContentScalingFactor.possibleValues.push_back( "1.33" );
    optContentScalingFactor.possibleValues.push_back( "1.5" );
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

	// Video mode possibilities
	CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
	CFIndex numModes = CFArrayGetCount(displayModes);
	CFMutableArrayRef goodModes = NULL;
	goodModes = CFArrayCreateMutable(kCFAllocatorDefault, numModes, NULL);
	
	// Grab all the available display modes, then weed out duplicates...
	for(int i = 0; i < numModes; ++i)
	{
		CGDisplayModeRef modeInfo = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);

        // Get IOKit flags for the display mode
        uint32_t ioFlags = CGDisplayModeGetIOFlags(modeInfo);

        bool safeForHardware = ioFlags & kDisplayModeSafetyFlags ? true : false;
		bool stretched = ioFlags & kDisplayModeStretchedFlag ? true : false;
		bool skipped = false;
		
		if((safeForHardware) || (!stretched))
		{
			size_t width  = CGDisplayModeGetWidth(modeInfo);
			size_t height = CGDisplayModeGetHeight(modeInfo); 
			
			for(CFIndex j = 0; j < CFArrayGetCount(goodModes); ++j)
			{
				CGDisplayModeRef otherMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(goodModes, j);
                
				size_t otherWidth  = CGDisplayModeGetWidth(otherMode);
				size_t otherHeight = CGDisplayModeGetHeight(otherMode);
				
				// If we find a duplicate then skip this mode
				if((otherWidth == width) && (otherHeight == height))
					skipped = true;
			}
			
			// This is a new mode, so add it to our goodModes array
			if(!skipped)
				CFArrayAppendValue(goodModes, modeInfo);
		}
	}
	
    // Release memory
    CFRelease(displayModes);

	// Sort the modes...
	CFArraySortValues(goodModes, CFRangeMake(0, CFArrayGetCount(goodModes)), 
					  (CFComparatorFunction)_compareModes, NULL);
					  
	// Now pull the modes out and put them into optVideoModes
	for(int i = 0; i < CFArrayGetCount(goodModes); ++i)
	{
		CGDisplayModeRef resolution = (CGDisplayModeRef)CFArrayGetValueAtIndex(goodModes, i);
		
		size_t fWidth  = CGDisplayModeGetWidth(resolution);
		size_t fHeight = CGDisplayModeGetHeight(resolution);
		// allow 16 and 32 bpp
		mVideoModes.push_back({uint32(fWidth), uint32(fHeight),0, 16});
		mVideoModes.push_back({uint32(fWidth), uint32(fHeight),0, 32});
    }
	
    // Release memory
    CFRelease(goodModes);

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

CFComparisonResult OSXGLSupport::_compareModes (const void *val1, const void *val2, void *context)
{
	// These are the values we will be interested in...
	/*
	CGDisplayModeGetWidth
	CGDisplayModeGetHeight
	CGDisplayModeGetRefreshRate
	_getDictionaryLong((mode), kCGDisplayBitsPerPixel)
	CGDisplayModeGetIOFlags((mode), kDisplayModeStretchedFlag)
	CGDisplayModeGetIOFlags((mode), kDisplayModeSafetyFlags)
	*/
	
	// CFArray comparison callback for sorting display modes.
	#pragma unused(context)
	CGDisplayModeRef thisMode = (CGDisplayModeRef)val1;
	CGDisplayModeRef otherMode = (CGDisplayModeRef)val2;
	
	size_t width = CGDisplayModeGetWidth(thisMode);
	size_t otherWidth = CGDisplayModeGetWidth(otherMode);
	
	size_t height = CGDisplayModeGetHeight(thisMode);
	size_t otherHeight = CGDisplayModeGetHeight(otherMode);

	// Sort modes in screen size order
	if (width * height < otherWidth * otherHeight)
	{
		return kCFCompareLessThan;
	}
	else if (width * height > otherWidth * otherHeight)
	{
		return kCFCompareGreaterThan;
	}

	// Sort modes by refresh rate.
	double refreshRate = CGDisplayModeGetRefreshRate(thisMode);
	double otherRefreshRate = CGDisplayModeGetRefreshRate(otherMode);

	if (refreshRate < otherRefreshRate)
	{
		return kCFCompareLessThan;
	}
	else if (refreshRate > otherRefreshRate)
	{
		return kCFCompareGreaterThan;
	}

	return kCFCompareEqualTo;
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
