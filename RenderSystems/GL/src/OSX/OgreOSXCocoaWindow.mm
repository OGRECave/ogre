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

#import "OgreOSXCocoaWindow.h"
#import "OgreRoot.h"
#import "OgreLogManager.h"
#import "OgreStringConverter.h"
#import "OgreWindowEventUtilities.h"
#import "OgreGLPixelFormat.h"
#import "OgreGLUtil.h"
#import "OgreGLRenderSystem.h"

#import <AppKit/NSScreen.h>
#import <AppKit/NSOpenGLView.h>
#import <QuartzCore/CVDisplayLink.h>

@implementation OgreWindow

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

@end

namespace Ogre {

    OSXCocoaWindow::OSXCocoaWindow() : mWindow(nil), mView(nil), mGLContext(nil), mGLPixelFormat(nil), mWindowOrigin(NSZeroPoint),
        mWindowDelegate(NULL), mActive(false), mClosed(false), mHasResized(false), mIsExternal(false), mWindowTitle(""),
        mUseNSView(false), mContentScalingFactor(1.0), mContentScalingSupported(false)
    {
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        mContentScalingSupported = dynamic_cast<OSXGLSupport*>(rs->getGLSupportRef())->OSVersionIsAtLeast("10.7");
    }

    OSXCocoaWindow::~OSXCocoaWindow()
    {
		[mGLContext clearDrawable];

        destroy();

        if(mWindow && !mIsExternal)
        {
            [mWindow release];
            mWindow = nil;
        }

        if(mWindowDelegate)
        {
            [mWindowDelegate release];
            mWindowDelegate = nil;
        }
    }
	
	void OSXCocoaWindow::create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams)
    {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSApplicationLoad();

		/*
        ***Key: "title" Description: The title of the window that will appear in the title bar
         Values: string Default: RenderTarget name

        ***Key: "colourDepth" Description: Colour depth of the resulting rendering window;
         only applies if fullScreen is set. Values: 16 or 32 Default: desktop depth Notes: [W32 specific]

        ***Key: "left" Description: screen x coordinate from left Values: positive integers
         Default: 'center window on screen' Notes: Ignored in case of full screen

        ***Key: "top" Description: screen y coordinate from top Values: positive integers
         Default: 'center window on screen' Notes: Ignored in case of full screen

        ***Key: "depthBuffer" [DX9 specific] Description: Use depth buffer Values: false or true Default: true

        ***Key: "externalWindowHandle" [API specific] Description: External window handle, for embedding the
         OGRE context Values: positive integer for W32 (HWND handle) poslong:posint:poslong (display*:screen:windowHandle)
         or poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*) for GLX Default: 0 (None)

        ***Key: "FSAA" Description: Full screen antialiasing factor Values: 0,2,4,6,... Default: 0

        ***Key: "displayFrequency" Description: Display frequency rate, for fullscreen mode Values: 60...?
         Default: Desktop vsync rate

        ***Key: "vsync" Description: Synchronize buffer swaps to vsync Values: true, false Default: 0
        */

		BOOL hasDepthBuffer = YES;
		int fsaa_samples = 0;
        bool hidden = false;
        NSString *windowTitle = [NSString stringWithCString:name.c_str() encoding:NSUTF8StringEncoding];
		int winx = 0, winy = 0;
		int depth = 32;
        NameValuePairList::const_iterator opt;
        mIsFullScreen = fullScreen;
		
		if(miscParams)
		{
			opt = miscParams->find("title");
			if(opt != miscParams->end())
				windowTitle = [NSString stringWithCString:opt->second.c_str() encoding:NSUTF8StringEncoding];
				
			opt = miscParams->find("left");
			if(opt != miscParams->end())
				winx = StringConverter::parseUnsignedInt(opt->second);
				
			opt = miscParams->find("top");
			if(opt != miscParams->end())
				winy = (int)NSHeight([[NSScreen mainScreen] frame]) - StringConverter::parseUnsignedInt(opt->second) - height;

            opt = miscParams->find("hidden");
            if (opt != miscParams->end())
                hidden = StringConverter::parseBool(opt->second);

			opt = miscParams->find("depthBuffer");
			if(opt != miscParams->end())
				hasDepthBuffer = StringConverter::parseBool(opt->second);
				
			opt = miscParams->find("FSAA");
			if(opt != miscParams->end())
				fsaa_samples = StringConverter::parseUnsignedInt(opt->second);
			
            opt = miscParams->find("gamma");
			if(opt != miscParams->end())
				mHwGamma = StringConverter::parseBool(opt->second);

			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				depth = StringConverter::parseUnsignedInt(opt->second);

            opt = miscParams->find("Full Screen");
            if(opt != miscParams->end())
                fullScreen = StringConverter::parseBool(opt->second);

            opt = miscParams->find("contentScalingFactor");
            if(opt != miscParams->end())
                mContentScalingFactor = StringConverter::parseReal(opt->second);
        }

        if(miscParams->find("externalGLContext") == miscParams->end())
        {
            NSOpenGLPixelFormatAttribute attribs[30];
            int i = 0;
            
            // Specify the display ID to associate the GL context with (main display for now)
            // Useful if there is ambiguity
            attribs[i++] = NSOpenGLPFAScreenMask;
            attribs[i++] = (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID());

            // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.
            // This makes the View-based context a compatible with the fullscreen context, enabling us to use
            // the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
            attribs[i++] = NSOpenGLPFANoRecovery;
            
            attribs[i++] = NSOpenGLPFAAccelerated;
            attribs[i++] = NSOpenGLPFADoubleBuffer;

            attribs[i++] = NSOpenGLPFAColorSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) depth;

            attribs[i++] = NSOpenGLPFAAlphaSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;
            
            attribs[i++] = NSOpenGLPFAStencilSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;

            attribs[i++] = NSOpenGLPFADepthSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) (hasDepthBuffer? 16 : 0);
            
            if(fsaa_samples > 0)
            {
                attribs[i++] = NSOpenGLPFAMultisample;
                attribs[i++] = NSOpenGLPFASampleBuffers;
                attribs[i++] = (NSOpenGLPixelFormatAttribute) 1;
                
                attribs[i++] = NSOpenGLPFASamples;
                attribs[i++] = (NSOpenGLPixelFormatAttribute) fsaa_samples;
            }
            
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;

            mGLPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
        }

        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        OSXCocoaContext *mainContext = (OSXCocoaContext*)rs->_getMainContext();
        NSOpenGLContext *shareContext = mainContext == 0 ? nil : mainContext->getContext();

        if(miscParams)
            opt = miscParams->find("externalGLContext");

        if(opt != miscParams->end())
        {
            NSOpenGLContext *openGLContext = (NSOpenGLContext*)StringConverter::parseUnsignedLong(opt->second);
            mGLContext = openGLContext;
        }
        else
        {
            mGLContext = [[NSOpenGLContext alloc] initWithFormat:mGLPixelFormat shareContext:shareContext];
        }

        // Set vsync by default to save battery and reduce tearing
        GLint swapInterval = 1;
        [mGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

        if(miscParams)
            opt = miscParams->find("externalWindowHandle");

        // Make active
        setHidden(hidden);
        mActive = true;
        mClosed = false;
        mName = [windowTitle cStringUsingEncoding:NSUTF8StringEncoding];
        mWidth = width;
        mHeight = height;
        mColourDepth = depth;
        mFSAA = fsaa_samples;

        if(!miscParams || opt == miscParams->end())
        {
            createNewWindow(width, height, [windowTitle cStringUsingEncoding:NSUTF8StringEncoding]);
        }
        else
        {
            NameValuePairList::const_iterator param_useNSView_pair;
            param_useNSView_pair = miscParams->find("macAPICocoaUseNSView");

            if(param_useNSView_pair != miscParams->end())
                if(param_useNSView_pair->second == "true")
                    mUseNSView = true;
            // If the macAPICocoaUseNSView parameter was set, use the winhandler as pointer to an NSView
            // Otherwise we assume the user created the interface with Interface Builder and instantiated an OgreView.
            
            if(mUseNSView) {
                LogManager::getSingleton().logMessage("Mac Cocoa Window: Rendering on an external plain NSView*");
                NSView *nsview = (NSView*)StringConverter::parseUnsignedLong(opt->second);
                mView = nsview;
            } else {
                LogManager::getSingleton().logMessage("Mac Cocoa Window: Rendering on an external OgreView*");
                OgreView *view = (OgreView*)StringConverter::parseUnsignedLong(opt->second);
                [view setOgreWindow:this];
                mView = view;
            
                NSRect b = [mView bounds];
                mWidth = (int)b.size.width;
                mHeight = (int)b.size.height;
            }

            mWindow = [mView window];
            mIsExternal = true;

            // Add our window to the window event listener class
            WindowEventUtilities::_addRenderWindow(this);
        }

        // Create register the context with the rendersystem and associate it with this window
        mContext = OGRE_NEW OSXCocoaContext(mGLContext, mGLPixelFormat);
        mContext->mBackingWidth = mWidth * mContentScalingFactor;
        mContext->mBackingHeight = mHeight * mContentScalingFactor;

		// Create the window delegate instance to handle window resizing and other window events
        mWindowDelegate = [[OSXCocoaWindowDelegate alloc] initWithNSWindow:mWindow ogreWindow:this];

        if(mContentScalingSupported && mContentScalingFactor > 1.0)
            [mView setWantsBestResolutionOpenGLSurface:YES];

        CGLLockContext((CGLContextObj)[mGLContext CGLContextObj]);

        [mView setNeedsDisplay:YES];

        if([mGLContext view] != mView)
            [mGLContext setView:mView];
        [mGLContext makeCurrentContext];

#if OGRE_DEBUG_MODE && defined(MAC_OS_X_VERSION_10_7)
        // Crash on functions that have been removed from the API
        CGLEnable((CGLContextObj)[mGLContext CGLContextObj], kCGLCECrashOnRemovedFunctions);
#endif

        // Enable GL multithreading
        CGLEnable((CGLContextObj)[mGLContext CGLContextObj], kCGLCEMPEngine);

        // Fix garbage screen
        glViewport(0, 0, mWidth, mHeight);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        [mGLContext update];

        [mGLContext flushBuffer];
        CGLUnlockContext((CGLContextObj)[mGLContext CGLContextObj]);

        [pool drain];

        StringStream ss;
        ss  << "Cocoa: Window created " << mWidth << " x " << mHeight
        << " with backing store size " << mContext->mBackingWidth << " x " << mContext->mBackingHeight
        << " using content scaling factor " << std::fixed << std::setprecision(1) << mContentScalingFactor;
        LogManager::getSingleton().logMessage(ss.str());
    }

    unsigned int OSXCocoaWindow::getWidth() const
    {
        NSRect winFrame;
        if(mContentScalingSupported && mContentScalingFactor > 1.0)
            winFrame = [mWindow convertRectToBacking:[mWindow contentRectForFrameRect:[mView frame]]];
        else
            winFrame = [mView frame];
        return (unsigned int) winFrame.size.width;
    }

    unsigned int OSXCocoaWindow::getHeight() const
    {
        NSRect winFrame;
        if(mContentScalingSupported && mContentScalingFactor > 1.0)
            winFrame = [mWindow convertRectToBacking:[mWindow contentRectForFrameRect:[mView frame]]];
        else
            winFrame = [mView frame];
        return (unsigned int) winFrame.size.height;
    }

    void OSXCocoaWindow::destroy(void)
    {
        if(!mIsFullScreen)
        {
            // Unregister and destroy OGRE GLContext
            OGRE_DELETE mContext;
            
            if(!mIsExternal)
            {
                // Remove the window from the Window listener
                WindowEventUtilities::_removeRenderWindow(this);
            }

            if(mGLContext)
            {
                [mGLContext release];
                mGLContext = nil;
            }

            if(mWindow)
            {
                if(!mIsExternal)
                    [mWindow performClose:nil];

                if(mGLPixelFormat)
                {
                    [mGLPixelFormat release];
                    mGLPixelFormat = nil;
                }

            }
		}
		
        mActive = false;
        mClosed = true;
    }

    bool OSXCocoaWindow::isActive() const
    {
        return mActive;
    }

    bool OSXCocoaWindow::isClosed() const
    {
        return false;
    }

    void OSXCocoaWindow::setHidden(bool hidden)
    {
        mHidden = hidden;
        if (!mIsExternal)
        {
            if (hidden)
                [mWindow orderOut:nil];
            else
                [mWindow makeKeyAndOrderFront:nil];
        }
    }

	void OSXCocoaWindow::setVSyncEnabled(bool vsync)
	{
        mVSync = vsync;
        mContext->setCurrent();
        
        GLint vsyncInterval = mVSync ? 1 : 0;
        [mGLContext setValues:&vsyncInterval forParameter:NSOpenGLCPSwapInterval];

        mContext->endCurrent();
        
        if(mGLContext != [NSOpenGLContext currentContext])
            [mGLContext makeCurrentContext];
	}
    
	bool OSXCocoaWindow::isVSyncEnabled() const
	{
        return mVSync;
	}

    void OSXCocoaWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
        if ((dst.right > mWidth) ||
            (dst.bottom > mHeight) ||
            (dst.front != 0) || (dst.back != 1))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Invalid box.",
                        "CocoaWindow::copyContentsToMemory" );
        }
        
        if (buffer == FB_AUTO)
        {
            buffer = mIsFullScreen? FB_FRONT : FB_BACK;
        }
        
        GLenum format = GLPixelUtil::getGLOriginFormat(dst.format);
        GLenum type = GLPixelUtil::getGLOriginDataType(dst.format);
        
        if ((format == GL_NONE) || (type == 0))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Unsupported format.",
                        "CocoaWindow::copyContentsToMemory" );
        }
        
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

    void OSXCocoaWindow::reposition(int left, int top)
    {
		if(!mWindow)
            return;

        if(mIsFullScreen)
            return;

		NSRect frame = [mWindow frame];
        NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
		frame.origin.x = left;
		frame.origin.y = screenFrame.size.height - frame.size.height - top;
        mWindowOrigin = frame.origin;
		[mWindow setFrame:frame display:YES];
    }

    void OSXCocoaWindow::resize(unsigned int width, unsigned int height)
    {
		if(!mWindow)
            return;
        
        if(mIsFullScreen)
            return;

        // Check if the window size really changed
        if(mWidth == width && mHeight == height)
            return;

        mWidth = width * mContentScalingFactor;
        mHeight = height * mContentScalingFactor;

        if(mIsExternal)
        {
            NSRect viewFrame = [mView frame];
            viewFrame.size.width = width;
            viewFrame.size.height = height;

            NSRect windowFrame = [[mView window] frame];

            mLeft = viewFrame.origin.x;
            mTop = windowFrame.size.height - (viewFrame.origin.y + viewFrame.size.height);
            mWindowOrigin = NSMakePoint(mLeft, mTop);

            GLint bufferRect[4];
            bufferRect[0] = mLeft;      // 0 = left edge 
            bufferRect[1] = mTop;       // 0 = bottom edge 
            bufferRect[2] = mWidth;     // width of buffer rect 
            bufferRect[3] = mHeight;    // height of buffer rect 
            CGLContextObj ctx = (CGLContextObj)[mGLContext CGLContextObj];
            CGLSetParameter(ctx, kCGLCPSwapRectangle, bufferRect);
        }
        else
        {
            NSRect frame = [mWindow frame];
            frame.size.width = width;
            frame.size.height = height;
            mWindowOrigin = frame.origin;
            [mWindow setFrame:frame display:YES];
        }
		[mGLContext update];
    }

    void OSXCocoaWindow::windowResized()
    {
        // Ensure the context is current
        if(!mIsFullScreen)
        {
            NSRect viewFrame = [mView frame];
            NSRect windowFrame = [[mView window] frame];
            NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];

            GLint bufferRect[4];
            bufferRect[0] = viewFrame.origin.x; // 0 = left edge 
            bufferRect[1] = windowFrame.size.height - (viewFrame.origin.y + viewFrame.size.height); // 0 = bottom edge 
            bufferRect[2] = viewFrame.size.width; // width of buffer rect 
            bufferRect[3] = viewFrame.size.height; // height of buffer rect 
            CGLContextObj ctx = (CGLContextObj)[mGLContext CGLContextObj];
            CGLSetParameter(ctx, kCGLCPSwapRectangle, bufferRect);
            [mGLContext update];

            mLeft = viewFrame.origin.x; 
            mTop = screenFrame.size.height - viewFrame.size.height;
            mWindowOrigin = NSMakePoint(mLeft, mTop);
        }
        
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it) 
        { 
            (*it).second->_updateDimensions(); 
        }
    }

    void OSXCocoaWindow::windowHasResized()
    {
        // Ensure the context is current
        [mGLContext flushBuffer];
    }
    
    void OSXCocoaWindow::windowMovedOrResized()
    {
        NSRect winFrame = [mWindow frame];
        NSRect viewFrame = [mView frame];
        NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
        mWidth = (unsigned int)viewFrame.size.width;
        mHeight = (unsigned int)viewFrame.size.height;
        mLeft = (int)winFrame.origin.x;
        mTop = screenFrame.size.height - winFrame.size.height;

        mWindowOrigin = NSMakePoint(mLeft, mTop);

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
		[mGLContext update];
    }

    void OSXCocoaWindow::swapBuffers()
    {
        CGLLockContext((CGLContextObj)[mGLContext CGLContextObj]);
        [mGLContext makeCurrentContext];

        if([mGLContext view] != mView)
            [mGLContext setView:mView];

        [mGLContext flushBuffer];
        CGLUnlockContext((CGLContextObj)[mGLContext CGLContextObj]);
    }
	
	//-------------------------------------------------------------------------------------------------//
	void OSXCocoaWindow::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "GLCONTEXT" ) 
		{
			*static_cast<OSXContext**>(pData) = mContext;
			return;
		} 
		else if( name == "WINDOW" ) 
		{
			*(void**)(pData) = mWindow;
			return;
		} 
		else if( name == "VIEW" ) 
		{
			*(void**)(pData) = mView;
			return;
		}
		else if( name == "NSOPENGLCONTEXT" ) 
		{
			*(void**)(pData) = mGLContext;
			return;
		}
		else if( name == "NSOPENGLPIXELFORMAT" ) 
		{
			*(void**)(pData) = mGLPixelFormat;
			return;
		}
		
	}

    void OSXCocoaWindow::createNewWindow(unsigned int width, unsigned int height, String title)
    {
        // Get the dimensions of the display. We will use it for the window size but not context resolution
        NSRect windowRect = NSZeroRect;
        if(mIsFullScreen)
        {
            NSRect mainDisplayRect = [[NSScreen mainScreen] visibleFrame];
            windowRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
        }
        else
            windowRect = NSMakeRect(0.0, 0.0, width, height);

        mWindow = [[OgreWindow alloc] initWithContentRect:windowRect
                                              styleMask:mIsFullScreen ? NSBorderlessWindowMask : NSResizableWindowMask|NSTitledWindowMask
                                                backing:NSBackingStoreBuffered
                                                  defer:YES];
        [mWindow setTitle:[NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding]];
        mWindowTitle = title;

        mView = [[OgreView alloc] initWithGLOSXWindow:this];

        _setWindowParameters();

//        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
//        rs->clearFrameBuffer(FBT_COLOUR);

        // Show window
        if(mWindow)
            [mWindow makeKeyAndOrderFront:nil];

        // Add our window to the window event listener class
        WindowEventUtilities::_addRenderWindow(this);
    }

    void OSXCocoaWindow::createWindowFromExternal(NSView *viewRef)
    {
        LogManager::getSingleton().logMessage("Creating external window");

        NSRect viewBounds;
        if(mContentScalingSupported)
            viewBounds = [mView convertRectToBacking:[mView bounds]];
        else
            viewBounds = [mView bounds];

        mWindow = [viewRef window];

        mView = viewRef;

        GLint bufferRect[4];
        bufferRect[0] = viewBounds.origin.x;      // 0 = left edge 
        bufferRect[1] = viewBounds.origin.y;      // 0 = bottom edge 
        bufferRect[2] = viewBounds.size.width;    // width of buffer rect 
        bufferRect[3] = viewBounds.size.height;   // height of buffer rect 
        CGLContextObj ctx = (CGLContextObj)[mGLContext CGLContextObj];
        CGLSetParameter(ctx, kCGLCPSwapRectangle, bufferRect);

        mIsExternal = true;
    }

    void OSXCocoaWindow::_setWindowParameters(void)
    {
        if(mWindow)
        {
            if(mIsFullScreen)
            {
                // Set the backing store size to the viewport dimensions
                // This ensures that it will scale to the full screen size
                NSRect mainDisplayRect = [[NSScreen mainScreen] frame];
                NSRect backingRect = NSZeroRect;

                if(mContentScalingSupported && mContentScalingFactor > 1.0)
                    backingRect = [[NSScreen mainScreen] convertRectToBacking:mainDisplayRect];
                else
                    backingRect = mainDisplayRect;

                GLint backingStoreDimensions[2] = { static_cast<GLint>(backingRect.size.width), static_cast<GLint>(backingRect.size.height) };
                CGLSetParameter((CGLContextObj)[mGLContext CGLContextObj], kCGLCPSurfaceBackingSize, backingStoreDimensions);
                CGLEnable((CGLContextObj)[mGLContext CGLContextObj], kCGLCESurfaceBackingSize);

                NSRect windowRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
                [mWindow setFrame:windowRect display:YES];
                [mView setFrame:windowRect];

                // Set window properties for full screen and save the origin in case the window has been moved
                [mWindow setStyleMask:NSBorderlessWindowMask];
                [mWindow setOpaque:YES];
                [mWindow setHidesOnDeactivate:YES];
                [mWindow setContentView:mView];
                [mWindow setFrameOrigin:NSZeroPoint];
                [mWindow setLevel:NSMainMenuWindowLevel+1];

                mWindowOrigin = mWindow.frame.origin;
                mLeft = mTop = 0;
            }
            else
            {
                // Reset and disable the backing store in windowed mode
                GLint backingStoreDimensions[2] = { 0, 0 };
                CGLSetParameter((CGLContextObj)[mGLContext CGLContextObj], kCGLCPSurfaceBackingSize, backingStoreDimensions);
                CGLDisable((CGLContextObj)[mGLContext CGLContextObj], kCGLCESurfaceBackingSize);

                NSRect viewRect = NSMakeRect(mWindowOrigin.x, mWindowOrigin.y, mWidth, mHeight);
                [mWindow setFrame:viewRect display:YES];
                [mView setFrame:viewRect];
                [mWindow setStyleMask:NSResizableWindowMask|NSTitledWindowMask];
                [mWindow setOpaque:YES];
                [mWindow setHidesOnDeactivate:NO];
                [mWindow setContentView:mView];
                [mWindow setLevel:NSNormalWindowLevel];
                [mWindow center];

                // Set the drawable, and current context
                // If you do this last, there is a moment before the rendering window pops-up
                [mGLContext makeCurrentContext];
            }
            
            [mGLContext update];
            
            // Even though OgreCocoaView doesn't accept first responder, it will get passed onto the next in the chain
            [mWindow makeFirstResponder:mView];
            [NSApp activateIgnoringOtherApps:YES];
        }
    }

    void OSXCocoaWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
    {
        if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
        {
            // Set the full screen flag
			mIsFullScreen = fullScreen;

                // Create a window if we haven't already, existence check is done within the functions
                if(!mWindow)
                {
                    if(mIsExternal)
                        createWindowFromExternal(mView);
                    else
                        createNewWindow(width, height, mWindowTitle);
                }

            _setWindowParameters();

            mWidth = width;
            mHeight = height;
        }
    }
}
