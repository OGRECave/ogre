/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
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

        CFRelease(pixEnc);
        return depth;
    }
#endif
    //-------------------------------------------------------------------------------------------------//
    OSXWindow::OSXWindow() : mContext(NULL), mCGLContextObj(NULL), mOriginalDisplayMode(NULL)
    {
    }
    
    //-------------------------------------------------------------------------------------------------//
    OSXWindow::~OSXWindow()
    {
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        if (mOriginalDisplayMode != NULL)
            CGDisplayModeRelease(mOriginalDisplayMode);
#endif
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
        if ((dst.right > mWidth) ||
            (dst.bottom > mHeight) ||
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
        
        // Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

        if(dst.getWidth() != dst.rowPitch)
        {
            glPixelStorei(GL_PACK_ROW_LENGTH, static_cast<GLint>(dst.rowPitch));
        }
        if((dst.getWidth()*Ogre::PixelUtil::getNumElemBytes(dst.format)) & 3)
        {
            // Standard alignment of 4 is not right
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
        }
        
        glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
        glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());
        
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        
        PixelUtil::bulkPixelVerticalFlip(dst);
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXWindow::createCGLFullscreen(unsigned int width, unsigned int height, unsigned int depth, unsigned int fsaa, CGLContextObj sharedContext)
    {
        // Find the best match to what was requested
        boolean_t exactMatch = 0;
        size_t reqWidth = 0, reqHeight = 0, reqDepth = 0;
        CGLError cglErr = kCGLNoError;
        CGError cgErr = kCGErrorSuccess;
        
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
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
            
            //        LogManager::getSingleton().logMessage(modeString);
            if(bitDepthFromDisplayMode(mode) != depth)
                continue;
            
            if((CGDisplayModeGetWidth(mode) == width) && (CGDisplayModeGetHeight(mode) == height))
            {
                // Release memory
                CGDisplayModeRelease(displayMode);
                displayMode = mode;
                CGDisplayModeRetain(displayMode);
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
                    // Release memory
                    CGDisplayModeRelease(displayMode);
                    displayMode = mode;
                    CGDisplayModeRetain(displayMode);
                    exactMatch = 1;
                    break;
                }
            }
        }

        // Release memory
        CFRelease(allModes);

        reqWidth = CGDisplayModeGetWidth(displayMode);
        reqHeight = CGDisplayModeGetHeight(displayMode);
        reqDepth = bitDepthFromDisplayMode(displayMode);
#else
        const void *value = NULL;
        CFDictionaryRef displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, depth, width, height, &exactMatch);
        
        value = CFDictionaryGetValue(displayMode, kCGDisplayWidth);
        CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqWidth);
        
        value = CFDictionaryGetValue(displayMode, kCGDisplayHeight);
        CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqHeight);
        
        value = CFDictionaryGetValue(displayMode, kCGDisplayBitsPerPixel);
        CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &reqDepth);
#endif

        if(!exactMatch)
        {
            // That mode is not available, using the closest match
            String request = StringConverter::toString(width) + String(" x ") + StringConverter::toString(height) + String(" @ ") + 
            StringConverter::toString(depth) + "bpp. ";

            String received = StringConverter::toString(reqWidth) + String(" x ") +
            StringConverter::toString(reqHeight) + String(" @ ") + 
            StringConverter::toString(reqDepth) + "bpp. "; 
            
            LogManager::getSingleton().logMessage(String("RenderSystem Warning: You requested a fullscreen mode of ") + request +
                                                  String(" This mode is not available and you will receive the closest match.  The best display mode for the parameters requested is: ")
                                                  + received);
        }

        // Do the fancy display fading
        CGDisplayFadeReservationToken reservationToken = 0;
        cgErr = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
                                                &reservationToken);
        if(reservationToken)
        {
            cgErr = CGDisplayFade(reservationToken,
                                  0.5f,
                                  kCGDisplayBlendNormal,
                                  kCGDisplayBlendSolidColor,
                                  0.0f, 0.0f, 0.0f,
                                  true);
            CG_CHECK_ERROR(cgErr)
            
            cgErr = CGReleaseDisplayFadeReservation(reservationToken);
            reservationToken = 0;
            CG_CHECK_ERROR(cgErr)
        }
        
        // Grab the main display and save it for later.
        // You could render to any display, but picking what display
        // to render to could be interesting.
        cgErr = CGDisplayCapture(kCGDirectMainDisplay);
        CG_CHECK_ERROR(cgErr)
        
        // Switch to the correct resolution
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        mOriginalDisplayMode = CGDisplayCopyDisplayMode(kCGDirectMainDisplay);
        cgErr = CGDisplaySetDisplayMode(kCGDirectMainDisplay, displayMode, NULL);

        // Release memory
        CFRelease(displayMode);
#else
        mOriginalDisplayMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
        cgErr = CGDisplaySwitchToMode(kCGDirectMainDisplay, displayMode);
#endif
        CG_CHECK_ERROR(cgErr)
        
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
            // Turn on kCGLPFASampleBuffers
            attribs[8] = (CGLPixelFormatAttribute)1;
            // Set the samples for kCGLPFASamples
            attribs[10] = (CGLPixelFormatAttribute)fsaa;
        }
        
        CGLPixelFormatObj pixelFormatObj = NULL;
#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
        long numPixelFormats = 0;
        cglErr = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#else
        GLint numPixelFormats = 0;
        cglErr = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#endif
        CGL_CHECK_ERROR(cglErr)
        
        if(pixelFormatObj && !mCGLContextObj)
        {
            // Create the CGL context from our pixel format, share it with the sharedContext passed in
            cglErr = CGLCreateContext(pixelFormatObj, sharedContext, &mCGLContextObj);
            CGL_CHECK_ERROR(cglErr)
        }

        if(mCGLContextObj)
        {
            // Set the context as current
            cglErr = CGLSetCurrentContext(mCGLContextObj);
            CGL_CHECK_ERROR(cglErr)
            
            // Set the context to full screen
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            cglErr = CGLSetFullScreenOnDisplay(mCGLContextObj, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
#else
            cglErr = CGLSetFullScreen(mCGLContextObj);
#endif
            CGL_CHECK_ERROR(cglErr)
            
            // This synchronizes CGL with the vertical retrace
            // Apple docs suggest that OpenGL blocks rendering calls when waiting for
            // a vertical retrace anyhow.
#if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
            long swapInterval = 1;
            cgErr = CGLSetParameter(mCGLContextObj, kCGLCPSwapInterval, &swapInterval);
#else
            GLint swapInterval = 1;
            cgErr = CGLSetParameter(mCGLContextObj, kCGLCPSwapInterval, &swapInterval);
#endif
            CG_CHECK_ERROR(cgErr)
            
            // Give a copy of our context to the rendersystem
            if(!mContext)
                mContext = OGRE_NEW OSXCGLContext(mCGLContextObj, pixelFormatObj);

            // Once we have the context we can destroy the pixel format
            // In order to share contexts you must keep a pointer to the context object around
            // Our context class will now manage the life of the pixelFormatObj
            cglErr = CGLDestroyPixelFormat(pixelFormatObj); 
            CGL_CHECK_ERROR(cglErr)
            
            // Let everyone know we are fullscreen now
            mIsFullScreen = true;
        }

        // Set some other variables.  Just in case we got a different value from CGDisplayBestModeForParameters than we requested
        mWidth = static_cast<uint32>(reqWidth);
        mHeight = static_cast<uint32>(reqHeight);
        mColourDepth = static_cast<uint32>(reqDepth);

        cgErr = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
                                                &reservationToken);
        if(cgErr == kCGErrorSuccess)
        {
            cgErr = CGDisplayFade(reservationToken,
                                  2.0f,
                                  kCGDisplayBlendSolidColor,
                                  kCGDisplayBlendNormal,
                                  0.0f, 0.0f, 0.0f,
                                  false);
            CG_CHECK_ERROR(cgErr)

            cgErr = CGReleaseDisplayFadeReservation(reservationToken);
            CG_CHECK_ERROR(cgErr)
        }
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXWindow::destroyCGLFullscreen(void)
    {
        CGError cgErr = kCGErrorSuccess;
        // Do the fancy display fading
        CGDisplayFadeReservationToken reservationToken = 0;
        cgErr = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
                                                &reservationToken);
        if(cgErr == kCGErrorSuccess)
        {
            cgErr = CGDisplayFade(reservationToken,
                                  2.0f,
                                  kCGDisplayBlendSolidColor,
                                  kCGDisplayBlendNormal,
                                  0.0f, 0.0f, 0.0f,
                                  false);
            CG_CHECK_ERROR(cgErr)
            
            cgErr = CGReleaseDisplayFadeReservation(reservationToken);
            reservationToken = 0;
            CG_CHECK_ERROR(cgErr)
        }
        
        // Release the main display
        cgErr = CGDisplayRelease( kCGDirectMainDisplay );
        CG_CHECK_ERROR(cgErr)
        
        cgErr = CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
                                                &reservationToken);
        if(cgErr == kCGErrorSuccess)
        {
            cgErr = CGDisplayFade(reservationToken,
                                  0.5f,
                                  kCGDisplayBlendNormal,
                                  kCGDisplayBlendSolidColor,
                                  0.0f, 0.0f, 0.0f,
                                  true);
            CG_CHECK_ERROR(cgErr)
            
            cgErr = CGReleaseDisplayFadeReservation(reservationToken);
            CG_CHECK_ERROR(cgErr)
        }

        if(mCGLContextObj)
        {
            CGLDestroyContext(mCGLContextObj);
            mCGLContextObj = 0;
        }

        // Switch back to the original screen resolution
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        CGDisplaySetDisplayMode(kCGDirectMainDisplay, mOriginalDisplayMode, NULL);
#else
        CGDisplaySwitchToMode(kCGDirectMainDisplay, mOriginalDisplayMode);
#endif
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXWindow::swapCGLBuffers(void)
    {
        CGLFlushDrawable(mCGLContextObj);
        CGLContextObj curCtx = CGLGetCurrentContext();
        if(curCtx != mCGLContextObj)
        {
            CGLSetCurrentContext(mCGLContextObj);
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            CGLSetFullScreenOnDisplay(mCGLContextObj, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
#else
            CGLSetFullScreen(mCGLContextObj);
#endif
        }
    }
    
}
