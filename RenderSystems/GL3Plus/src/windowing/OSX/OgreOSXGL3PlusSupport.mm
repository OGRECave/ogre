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

#import "OgreGL3PlusSupport.h"
#import "OgreOSXGL3PlusSupport.h"
#import "OgreOSXCocoaWindow.h"
#import "OgreGL3PlusTexture.h"
#import "OgreGL3PlusRenderSystem.h"

#import "macUtils.h"
#import <dlfcn.h>

#import <OpenGL/OpenGL.h>
#import <AppKit/NSScreen.h>

namespace Ogre {

OSXGL3PlusSupport::OSXGL3PlusSupport()
{
}

OSXGL3PlusSupport::~OSXGL3PlusSupport()
{
}

void OSXGL3PlusSupport::addConfig( void )
{
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optBitDepth;
	ConfigOption optFSAA;
	ConfigOption optRTTMode;
    ConfigOption optMacAPI;
	ConfigOption optHiddenWindow;
	ConfigOption optVsync;
	ConfigOption optSRGB;
    ConfigOption optContentScalingFactor;

	// FS setting possibilities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back( "Yes" );
	optFullScreen.possibleValues.push_back( "No" );
	optFullScreen.currentValue = "No";
	optFullScreen.immutable = false;

    // Hidden window setting possibilities
	optHiddenWindow.name = "hidden";
	optHiddenWindow.possibleValues.push_back( "Yes" );
	optHiddenWindow.possibleValues.push_back( "No" );
	optHiddenWindow.currentValue = "No";
	optHiddenWindow.immutable = false;

    // FS setting possibilities
	optVsync.name = "vsync";
	optVsync.possibleValues.push_back( "Yes" );
	optVsync.possibleValues.push_back( "No" );
	optVsync.currentValue = "No";
	optVsync.immutable = false;

	optBitDepth.name = "Colour Depth";
	optBitDepth.possibleValues.push_back( "32" );
	optBitDepth.possibleValues.push_back( "16" );
	optBitDepth.currentValue = "32";
	optBitDepth.immutable = false;

    optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back( "FBO" );
	optRTTMode.possibleValues.push_back( "PBuffer" );
	optRTTMode.possibleValues.push_back( "Copy" );
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;

	// SRGB on auto window
	optSRGB.name = "sRGB Gamma Conversion";
	optSRGB.possibleValues.push_back("Yes");
	optSRGB.possibleValues.push_back("No");
	optSRGB.currentValue = "No";
	optSRGB.immutable = false;

    optContentScalingFactor.name = "Content Scaling Factor";
    optContentScalingFactor.possibleValues.push_back( "1.0" );
    optContentScalingFactor.possibleValues.push_back( "1.33" );
    optContentScalingFactor.possibleValues.push_back( "1.5" );
    optContentScalingFactor.possibleValues.push_back( "2.0" );
    optContentScalingFactor.currentValue = StringConverter::toString((float)[NSScreen mainScreen].backingScaleFactor);
    optContentScalingFactor.immutable = false;

    optMacAPI.name = "macAPI";
    optMacAPI.possibleValues.push_back( "cocoa" );
	optMacAPI.currentValue = "cocoa";
    optMacAPI.immutable = false;

    mOptions[ optMacAPI.name ] = optMacAPI;
    mOptions[ optFullScreen.name ] = optFullScreen;
	mOptions[ optBitDepth.name ] = optBitDepth;
    mOptions[ optContentScalingFactor.name ] = optContentScalingFactor;

	CGLRendererInfoObj rend;
	GLint nrend = 0, maxSamples = 0;

	CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
	CGLDescribeRenderer(rend, 0, kCGLRPMaxSamples, &maxSamples);
    CGLDestroyRendererInfo(rend);

    // FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back( "0" );

    for(int i = 0; i <= maxSamples; i += 2)
        optFSAA.possibleValues.push_back( StringConverter::toString(i) );

    optFSAA.currentValue = "0";
    optFSAA.immutable = false;

    mOptions[ optFSAA.name ] = optFSAA;

	// Video mode possibilities
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;
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
		String resoString = StringConverter::toString(fWidth) + " x " + StringConverter::toString(fHeight);
		optVideoMode.possibleValues.push_back(resoString);
    }
	
    // Release memory
    CFRelease(goodModes);

    optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back( "FBO" );
	optRTTMode.possibleValues.push_back( "PBuffer" );
	optRTTMode.possibleValues.push_back( "Copy" );
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;

	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optVideoMode.name] = optVideoMode;
    mOptions[optFSAA.name] = optFSAA;
	mOptions[optRTTMode.name] = optRTTMode;
	mOptions[optHiddenWindow.name] = optHiddenWindow;
	mOptions[optVsync.name] = optVsync;
	mOptions[optSRGB.name] = optSRGB;

    setShaderCachePath(Ogre::macCachePath() + "/org.ogre3d.RTShaderCache");
    
    // Set the shader library path
    setShaderLibraryPath(Ogre::macBundlePath() + "/Contents/Resources/RTShaderLib/GLSL150");
}

String OSXGL3PlusSupport::validateConfig( void )
{
	return String( "" );
}

RenderWindow* OSXGL3PlusSupport::createWindow( bool autoCreateWindow, GL3PlusRenderSystem* renderSystem, const String& windowTitle ) 
{
	if( autoCreateWindow )
	{
		ConfigOptionMap::iterator opt = mOptions.find( "Full Screen" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "OSXGL3PlusSupport::createWindow" );
		bool fullscreen = ( opt->second.currentValue == "Yes" );
		opt = mOptions.find( "Video Mode" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "OSXGL3PlusSupport::createWindow" );
		String val = opt->second.currentValue;
		String::size_type pos = val.find( 'x' );
		if( pos == String::npos )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "OSXGL3PlusSupport::createWindow" );

		unsigned int w = StringConverter::parseUnsignedInt( val.substr( 0, pos ) );
		unsigned int h = StringConverter::parseUnsignedInt( val.substr( pos + 1 ) );

        // Parse FSAA config
		NameValuePairList winOptions;
		winOptions[ "title" ] = windowTitle;
        opt = mOptions.find( "FSAA" );
        if( opt != mOptions.end() )
        {
			winOptions[ "FSAA" ] = opt->second.currentValue;
        }

        opt = mOptions.find( "hidden" );
        if( opt != mOptions.end() )
        {
            winOptions[ "hidden" ] = opt->second.currentValue;
        }

        opt = mOptions.find( "vsync" );
        if( opt != mOptions.end() )
        {
            winOptions[ "vsync" ] = opt->second.currentValue;
        }

        opt = mOptions.find( "Content Scaling Factor" );
        if( opt != mOptions.end() )
        {
            winOptions["contentScalingFactor"] = opt->second.currentValue;
        }

        opt = mOptions.find( "sRGB Gamma Conversion" );
        if( opt != mOptions.end() )
            winOptions["gamma"] = opt->second.currentValue;

        opt = mOptions.find( "macAPI" );
        if( opt != mOptions.end() )
        {
			winOptions[ "macAPI" ] = opt->second.currentValue;
        }

		return renderSystem->_createRenderWindow( windowTitle, w, h, fullscreen, &winOptions );
	}
	else
	{
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* OSXGL3PlusSupport::newWindow( const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams )
{
	// Create the window, if Cocoa return a Cocoa window
    LogManager::getSingleton().logMessage("Creating a Cocoa Compatible Render System");
    CocoaWindow *window = OGRE_NEW CocoaWindow();
    window->create(name, width, height, fullScreen, miscParams);

    return window;
}

void OSXGL3PlusSupport::start()
{
	LogManager::getSingleton().logMessage(
			"***********************************************\n"
			"***  Starting Mac OS X OpenGL 3+ Subsystem  ***\n"
			"***********************************************");
}

void OSXGL3PlusSupport::stop()
{
	LogManager::getSingleton().logMessage(
			"***********************************************\n"
			"***  Stopping Mac OS X OpenGL 3+ Subsystem  ***\n"
			"***********************************************");
}

void* OSXGL3PlusSupport::getProcAddress( const char* name )
{
    void *symbol;
    symbol = NULL;
    
    String fullPath = macPluginPath() + "RenderSystem_GL3Plus.dylib";
    void *handle = dlopen(fullPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if(handle) {
        symbol = dlsym (handle, name);
    }
    dlclose(handle);
    
    return symbol;
}

void* OSXGL3PlusSupport::getProcAddress( const String& procname )
{
	return getProcAddress( procname.c_str() );
}

bool OSXGL3PlusSupport::supportsPBuffers()
{
	return false;
}

CFComparisonResult OSXGL3PlusSupport::_compareModes (const void *val1, const void *val2, void *context)
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

Boolean OSXGL3PlusSupport::_getDictionaryBoolean(CFDictionaryRef dict, const void* key)
{
	Boolean value = false;
	CFBooleanRef boolRef;
	boolRef = (CFBooleanRef)CFDictionaryGetValue(dict, key);
	
	if (boolRef != NULL)
		value = CFBooleanGetValue(boolRef); 	
		
	return value;
}

long OSXGL3PlusSupport::_getDictionaryLong(CFDictionaryRef dict, const void* key)
{
	long value = 0;
	CFNumberRef numRef;
	numRef = (CFNumberRef)CFDictionaryGetValue(dict, key);
	
	if (numRef != NULL)
		CFNumberGetValue(numRef, kCFNumberLongType, &value);	
		
	return value;
}

}
