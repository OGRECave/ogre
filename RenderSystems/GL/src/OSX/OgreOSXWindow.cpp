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

#include "OgreOSXWindow.h"
#include "OgreOSXCGLContext.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLPixelFormat.h"

#include <OpenGL/gl.h>
#define GL_EXT_texture_env_combine 1
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

namespace Ogre
{

//-------------------------------------------------------------------------------------------------//
OSXWindow::OSXWindow() : mCGLContext(NULL)
{
}

//-------------------------------------------------------------------------------------------------//
OSXWindow::~OSXWindow()
{
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
{
	if ((dst.left < 0) || (dst.right > mWidth) ||
		(dst.top < 0) || (dst.bottom > mHeight) ||
		(dst.front != 0) || (dst.back != 1))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Invalid box.",
					"OSXWindow::copyContentsToMemory" );
	}

	if (buffer == FB_AUTO)
	{
		buffer = mIsFullScreen? FB_FRONT : FB_BACK;
	}

	GLenum format = Ogre::GLPixelUtil::getGLOriginFormat(dst.format);
	GLenum type = Ogre::GLPixelUtil::getGLOriginDataType(dst.format);

	if ((format == GL_NONE) || (type == 0))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Unsupported format.",
					"OSXWindow::copyContentsToMemory" );
	}

	if((dst.getWidth()*Ogre::PixelUtil::getNumElemBytes(dst.format)) & 3)
	{
		// Standard alignment of 4 is not right
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}
	
	glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
	glReadPixels((GLint)dst.left, (GLint)dst.top,
				 (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
				 format, type, dst.data);

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	
	//vertical flip
	{
		size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
		size_t height = dst.getHeight();
		uchar *tmpData = new uchar[rowSpan * height];
		uchar *srcRow = (uchar *)dst.data, *tmpRow = tmpData + (height - 1) * rowSpan;

		while (tmpRow >= tmpData)
		{
			memcpy(tmpRow, srcRow, rowSpan);
			srcRow += rowSpan;
			tmpRow -= rowSpan;
		}
		memcpy(dst.data, tmpData, rowSpan * height);

		delete [] tmpData;
	}
}

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
uint32 OSXWindow::bitDepthFromDisplayMode(CGDisplayModeRef mode)
{
    uint32 depth = 0;
    CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
    if(CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 32;
    else if(CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 16;
    else if(CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        depth = 8;
    
    return depth;
}
#endif

//-------------------------------------------------------------------------------------------------//
void OSXWindow::createCGLFullscreen(unsigned int width, unsigned int height, unsigned int depth, unsigned int fsaa, CGLContextObj sharedContext)
{
    // Find the best match to what was requested
    boolean_t exactMatch = 0;
    int reqWidth, reqHeight, reqDepth;
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    
    // Get a copy of the current display mode
    CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(kCGDirectMainDisplay);

    // Loop through all display modes to determine the closest match.
    // CGDisplayBestModeForParameters is deprecated on 10.6 so we will emulate it's behavior
    // Try to find a mode with the requested depth and equal or greater dimensions first.
    // If no match is found, try to find a mode with greater depth and same or greater dimensions.
    // If still no match is found, just use the current mode.
    CFArrayRef allModes = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
    for(int i = 0; i < CFArrayGetCount(allModes); i++)
    {
        CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
        String modeString = StringConverter::toString(CGDisplayModeGetWidth(mode)) + String(" x ") +
            StringConverter::toString(CGDisplayModeGetHeight(mode)) + String(" @ ") +
            StringConverter::toString(bitDepthFromDisplayMode(mode)) + "bpp.";

        LogManager::getSingleton().logMessage(modeString);
        if(bitDepthFromDisplayMode(mode) != depth)
            continue;

        if((CGDisplayModeGetWidth(mode) >= width) && (CGDisplayModeGetHeight(mode) >= height))
        {
            displayMode = mode;
            exactMatch = 1;
            break;
        }
    }

    // No depth match was found
    if(!exactMatch)
    {
        for(int i = 0; i < CFArrayGetCount(allModes); i++)
        {
            CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
            if(bitDepthFromDisplayMode(mode) >= depth)
                continue;

            if((CGDisplayModeGetWidth(mode) >= width) && (CGDisplayModeGetHeight(mode) >= height))
            {
                displayMode = mode;
                exactMatch = 1;
                break;
            }
        }
    }
    
    reqWidth = CGDisplayModeGetWidth(displayMode);
    reqHeight = CGDisplayModeGetHeight(displayMode);
    reqDepth = bitDepthFromDisplayMode(displayMode);
#else
    CFDictionaryRef displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, depth, width, height, &exactMatch);
    const void *value = NULL;

    value = CFDictionaryGetValue(displayMode, kCGDisplayWidth);
    CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqWidth);
    
    value = CFDictionaryGetValue(displayMode, kCGDisplayHeight);
    CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqHeight);
    
    value = CFDictionaryGetValue(displayMode, kCGDisplayBitsPerPixel);
    CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqDepth);
#endif

    if(!exactMatch)
    {
        // TODO: Report the size difference
        // That mode is not available, using the closest match
        String request = StringConverter::toString(width) + String(" x ") + StringConverter::toString(height) + String(" @ ") + 
            StringConverter::toString(depth) + "bpp. ";

        String received = StringConverter::toString(reqWidth) + String(" x ") +
            StringConverter::toString(reqHeight) + String(" @ ") + 
            StringConverter::toString(reqDepth) + "bpp. "; 
            
        LogManager::getSingleton().logMessage(String("RenderSystem Warning:  You requested a fullscreen mode of ") + request +
            String(" This mode is not available and you will receive the closest match.  The best display mode for the parameters requested is: ")
            + received);
    }
    
    // Do the fancy display fading
    CGDisplayFadeReservationToken reservationToken;
    CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
                                        &reservationToken);
    CGDisplayFade(reservationToken,
                  0.5,
                  kCGDisplayBlendNormal,
                  kCGDisplayBlendSolidColor,
                  0.0, 0.0, 0.0,
                  true);
    
    // Grab the main display and save it for later.
    // You could render to any display, but picking what display
    // to render to could be interesting.
    CGDisplayCapture(kCGDirectMainDisplay);
    
    // Switch to the correct resolution
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    CGDisplaySetDisplayMode(kCGDirectMainDisplay, displayMode, NULL);
#else
    CGDisplaySwitchToMode(kCGDirectMainDisplay, displayMode);
#endif
    
    // Get a pixel format that best matches what we are looking for
    CGLPixelFormatAttribute attribs[] = { 
        kCGLPFADoubleBuffer,
        kCGLPFAAlphaSize,     (CGLPixelFormatAttribute)8,
        kCGLPFADepthSize,     (CGLPixelFormatAttribute)reqDepth,
        kCGLPFAStencilSize,   (CGLPixelFormatAttribute)8,
        kCGLPFASampleBuffers, (CGLPixelFormatAttribute)0,
        kCGLPFASamples,       (CGLPixelFormatAttribute)0,
        kCGLPFAFullScreen,
        kCGLPFASingleRenderer,
        kCGLPFAAccelerated,
        kCGLPFADisplayMask,   (CGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        (CGLPixelFormatAttribute)0
    };
    
    // Set up FSAA if it was requested
    if(fsaa > 1)
    {
            // turn on kCGLPFASampleBuffers
            attribs[8] = (CGLPixelFormatAttribute)1;
            // set the samples for kCGLPFASamples
            attribs[10] = (CGLPixelFormatAttribute)fsaa;
    }
    
    
    CGLError err;
    CGLPixelFormatObj pixelFormatObj;
#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
    GLint numPixelFormats = 0;
    err = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#else
    long numPixelFormats = 0;
    err = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#endif
    if(err != 0)
    {
        CGReleaseAllDisplays();
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CGL Error: " + String(CGLErrorString(err))), "OSXWindow::createCGLFullscreen");
    }

    // Create the CGLcontext from our pixel format, share it with the sharedContext passed in
    err = CGLCreateContext(pixelFormatObj, sharedContext, &mCGLContext);
    if(err != 0)
    {
        CGReleaseAllDisplays();
        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CGL Error: " + String(CGLErrorString(err))), "OSXWindow::createCGLFullscreen");
    }
            
    // Once we have the context we can destroy the pixel format
    // In order to share contexts you must keep a pointer to the context object around
    // Our context class will now manage the life of the pixelFormatObj
    //CGLDestroyPixelFormat(pixelFormatObj); 
            
    // Set the context to full screen
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    CGLSetFullScreenOnDisplay(mCGLContext, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
#else
    CGLSetFullScreen(mCGLContext);
#endif
    
    // Set the context as current
    CGLSetCurrentContext(mCGLContext);
    
    // This synchronizes CGL with the vertical retrace
    // Apple docs suggest that OpenGL blocks rendering calls when waiting for
    // a vertical retrace anyhow.
#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
    GLint swapInterval = 1;
    CGLSetParameter(mCGLContext, kCGLCPSwapInterval, &swapInterval);
#else
    long swapInterval = 1;
    CGLSetParameter(mCGLContext, kCGLCPSwapInterval, &swapInterval);
#endif
    
    // Give a copy of our context to the rendersystem
    mContext = new OSXCGLContext(mCGLContext, pixelFormatObj);
    
    // Let everyone know we are fullscreen now
    mIsFullScreen = true;

    // Set some other variables.  Just in case we got a different value from CGDisplayBestModeForParameters than we requested
    mWidth = reqWidth;
    mHeight = reqHeight;
    mColourDepth = reqDepth;

    CGDisplayFade(reservationToken,
              2.0,
              kCGDisplayBlendSolidColor,
              kCGDisplayBlendNormal,
              0.0, 0.0, 0.0,
              false);
    CGReleaseDisplayFadeReservation(reservationToken);
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::destroyCGLFullscreen(void)
{
	CGReleaseAllDisplays();
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::swapCGLBuffers(void)
{
	CGLFlushDrawable(mCGLContext);
	CGLContextObj curCtx = CGLGetCurrentContext();
	if(curCtx != mCGLContext)
	{
		CGLSetCurrentContext(mCGLContext);
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
        CGLSetFullScreenOnDisplay(mCGLContext, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
#else
        CGLSetFullScreen(mCGLContext);
#endif
	}
}

}
