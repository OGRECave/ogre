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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreOSXGLSupport.h"
#include "OgreOSXCarbonWindow.h"
#include "OgreOSXCocoaWindow.h"

#include "OgreGLTexture.h"
#include "OgreOSXRenderTexture.h"

#include "macUtils.h"
#include <dlfcn.h>

#include <OpenGL/OpenGL.h>

namespace Ogre {

OSXGLSupport::OSXGLSupport() : mAPI(""), mContextType("")
{
}

OSXGLSupport::~OSXGLSupport()
{
}

void OSXGLSupport::addConfig( void )
{
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optBitDepth;
    ConfigOption optFSAA;
	ConfigOption optRTTMode;

	// FS setting possiblities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back( "Yes" );
	optFullScreen.possibleValues.push_back( "No" );
	optFullScreen.currentValue = "No";
	optFullScreen.immutable = false;

	optBitDepth.name = "Colour Depth";
	optBitDepth.possibleValues.push_back( "32" );
	optBitDepth.possibleValues.push_back( "16" );
	optBitDepth.currentValue = "32";
	optBitDepth.immutable = false;

    mOptions[ optFullScreen.name ] = optFullScreen;
	mOptions[ optBitDepth.name ] = optBitDepth;

	CGLRendererInfoObj rend;
    
#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
	GLint nrend;
	CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
#else
    long nrend;
	CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), &rend, &nrend);
#endif

#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
    GLint maxSamples;
	CGLDescribeRenderer(rend, 0, kCGLRPMaxSamples, &maxSamples);
#else
	long maxSamples;
	CGLDescribeRenderer(rend, 0, kCGLRPMaxSamples, &maxSamples);
#endif

    CGLDestroyRendererInfo(rend);

    // FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back( "0" );

	switch( maxSamples )
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

    mOptions[ optFSAA.name ] = optFSAA;

	// Video mode possiblities
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
#else
	CFArrayRef displayModes = CGDisplayAvailableModes(CGMainDisplayID());
#endif
	CFIndex numModes = CFArrayGetCount(displayModes);
	CFMutableArrayRef goodModes = NULL;
	goodModes = CFArrayCreateMutable(kCFAllocatorDefault, numModes, NULL);
	
	// Grab all the available display modes, then weed out duplicates...
	for(int i = 0; i < numModes; ++i)
	{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
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
#else
		CFDictionaryRef modeInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(displayModes, i);
		
		Boolean safeForHardware = _getDictionaryBoolean(modeInfo, kCGDisplayModeIsSafeForHardware);
		Boolean stretched = _getDictionaryBoolean(modeInfo, kCGDisplayModeIsStretched);
		Boolean skipped = false;
		
		if((safeForHardware) || (!stretched))
		{
			long width  = _getDictionaryLong(modeInfo, kCGDisplayWidth);
			long height = _getDictionaryLong(modeInfo, kCGDisplayHeight); 
			
			for(int j = 0; j < CFArrayGetCount(goodModes); ++j)
			{
				CFDictionaryRef otherMode = (CFDictionaryRef)CFArrayGetValueAtIndex(goodModes, j);

				long otherWidth  = _getDictionaryLong(otherMode, kCGDisplayWidth);
				long otherHeight = _getDictionaryLong(otherMode, kCGDisplayHeight);
				
				// If we find a duplicate then skip this mode
				if((otherWidth == width) && (otherHeight == height))
					skipped = true;
			}
			
			// This is a new mode, so add it to our goodModes array
			if(!skipped)
				CFArrayAppendValue(goodModes, modeInfo);
#endif
		}
	}
	
	// Sort the modes...
	CFArraySortValues(goodModes, CFRangeMake(0, CFArrayGetCount(goodModes)), 
					  (CFComparatorFunction)_compareModes, NULL);
					  
	// Now pull the modes out and put them into optVideoModes
	for(int i = 0; i < CFArrayGetCount(goodModes); ++i)
	{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
		CGDisplayModeRef resolution = (CGDisplayModeRef)CFArrayGetValueAtIndex(goodModes, i);
		
		size_t fWidth  = CGDisplayModeGetWidth(resolution);
		size_t fHeight = CGDisplayModeGetHeight(resolution);
#else
		CFDictionaryRef resolution = (CFDictionaryRef)CFArrayGetValueAtIndex(goodModes, i);
		
		long fWidth  = _getDictionaryLong(resolution, kCGDisplayWidth);
		long fHeight = _getDictionaryLong(resolution, kCGDisplayHeight);
#endif
		String resoString = StringConverter::toString(fWidth) + " x " + StringConverter::toString(fHeight);
		optVideoMode.possibleValues.push_back(resoString);
		
//		LogManager::getSingleton().logMessage( "Added resolution: " + resoString);
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
}

String OSXGLSupport::validateConfig( void )
{
	return String( "" );
}

RenderWindow* OSXGLSupport::createWindow( bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle ) 
{
	if( autoCreateWindow )
	{
		ConfigOptionMap::iterator opt = mOptions.find( "Full Screen" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "OSXGLSupport::createWindow" );
		bool fullscreen = ( opt->second.currentValue == "Yes" );

		opt = mOptions.find( "Video Mode" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "OSXGLSupport::createWindow" );
		String val = opt->second.currentValue;
		String::size_type pos = val.find( 'x' );
		if( pos == String::npos )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "OSXGLSupport::createWindow" );

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

		return renderSystem->_createRenderWindow( windowTitle, w, h, fullscreen, &winOptions );
	}
	else
	{
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* OSXGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams )
{
	//  Does the user want Cocoa or Carbon, default to carbon...
	mAPI = "carbon";
	mContextType = "AGL";
	
	if(miscParams)
	{
		NameValuePairList::const_iterator opt(NULL);
		
		// First we must determine if this is a carbon or a cocoa window
		// that we wish to create
		opt = miscParams->find("macAPI");
		if(opt != miscParams->end() && opt->second == "cocoa")
		{
			// Our user wants a cocoa compatable system
			mAPI = "cocoa";
			mContextType = "NSOpenGL";
		}
	}
	
	// Create the window, if cocoa return a cocoa window
	if(mAPI == "cocoa")
	{
		LogManager::getSingleton().logMessage("Creating a Cocoa Compatible Render System");
		OSXCocoaWindow* window = new OSXCocoaWindow();
		window->create(name, width, height, fullScreen, miscParams);
		return window;
	}
	
	// Otherwise default to carbon
	LogManager::getSingleton().logMessage("Creating a Carbon Compatible Render System");
	OSXCarbonWindow* window = new OSXCarbonWindow();
	window->create(name, width, height, fullScreen, miscParams);
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

void* OSXGLSupport::getProcAddress( const char* name )
{
    void *symbol;
    symbol = NULL;
    
    std::string fullPath = macPluginPath() + "RenderSystem_GL.dylib";
    void *handle = dlopen(fullPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if(handle) {
        symbol = dlsym (handle, name);
    }
    dlclose(handle);
    
    return symbol;
}

void* OSXGLSupport::getProcAddress( const String& procname )
{
	return getProcAddress( procname.c_str() );
}

bool OSXGLSupport::supportsPBuffers()
{
	return true;
}

GLPBuffer* OSXGLSupport::createPBuffer(PixelComponentType format, size_t width, size_t height)
{
//	if(mContextType == "NSOpenGL")
//		return new OSXCocoaPBuffer(format, width, height);
//	if(mContextType == "CGL")
//		return new OSXCGLPBuffer(format, width, height);
//	else
		return new OSXPBuffer(format, width, height);
}

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
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

	// sort modes in screen size order
	if (width * height < otherWidth * otherHeight)
	{
		return kCFCompareLessThan;
	}
	else if (width * height > otherWidth * otherHeight)
	{
		return kCFCompareGreaterThan;
	}

    // TODO: DJR - Find out DisplayMode API method for determining display depth
	// sort modes by bits per pixel
//    uint32_t ioFlags = CGDisplayModeGetIOFlags(modeInfo);

//	size_t bitsPerPixel = _getDictionaryLong(thisMode, kCGDisplayBitsPerPixel);
//	size_t otherBitsPerPixel = _getDictionaryLong(otherMode, kCGDisplayBitsPerPixel);
//
//	if (bitsPerPixel < otherBitsPerPixel)
//	{
//		return kCFCompareLessThan;
//	}
//	else if (bitsPerPixel > otherBitsPerPixel)
//	{
//		return kCFCompareGreaterThan;
//	}

	// sort modes by refresh rate.
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
#else
CFComparisonResult OSXGLSupport::_compareModes (const void *val1, const void *val2, void *context)
{
    // These are the values we will be interested in...
    /*
     _getDictionaryLong((mode), kCGDisplayWidth)
     _getDictionaryLong((mode), kCGDisplayHeight)
     _getDictionaryLong((mode), kCGDisplayRefreshRate)
     _getDictionaryLong((mode), kCGDisplayBitsPerPixel)
     _getDictionaryBoolean((mode), kCGDisplayModeIsSafeForHardware)
     _getDictionaryBoolean((mode), kCGDisplayModeIsStretched)
     */
    
    // CFArray comparison callback for sorting display modes.
#pragma unused(context)
    CFDictionaryRef thisMode = (CFDictionaryRef)val1;
    CFDictionaryRef otherMode = (CFDictionaryRef)val2;
    
    long width = _getDictionaryLong(thisMode, kCGDisplayWidth);
    long otherWidth = _getDictionaryLong(otherMode, kCGDisplayWidth);
    
    long height = _getDictionaryLong(thisMode, kCGDisplayHeight);
    long otherHeight = _getDictionaryLong(otherMode, kCGDisplayHeight);
    
    // sort modes in screen size order
    if (width * height < otherWidth * otherHeight)
    {
        return kCFCompareLessThan;
    }
    else if (width * height > otherWidth * otherHeight)
    {
        return kCFCompareGreaterThan;
    }
    
    // sort modes by bits per pixel
    long bitsPerPixel = _getDictionaryLong(thisMode, kCGDisplayBitsPerPixel);
    long otherBitsPerPixel = _getDictionaryLong(otherMode, kCGDisplayBitsPerPixel);
    
    if (bitsPerPixel < otherBitsPerPixel)
    {
        return kCFCompareLessThan;
    }
    else if (bitsPerPixel > otherBitsPerPixel)
    {
        return kCFCompareGreaterThan;
    }
    
    // sort modes by refresh rate.
    long refreshRate = _getDictionaryLong(thisMode, kCGDisplayRefreshRate);
    long otherRefreshRate = _getDictionaryLong(otherMode, kCGDisplayRefreshRate);
    
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
#endif

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
