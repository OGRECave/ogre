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

#import "OgreGLRenderSystemCommon.h"
#import "OgreGLNativeSupport.h"
#import <AppKit/AppKit.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSOpenGLView.h>
#import <QuartzCore/CVDisplayLink.h>
#import "OgreViewport.h"
#import <iomanip>

@implementation OgreGLWindow

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
    
    struct NSOpenGLContextGuard
    {
        NSOpenGLContextGuard(NSOpenGLContext* ctx) : mPrevContext([NSOpenGLContext currentContext]) { if(ctx != mPrevContext) [ctx makeCurrentContext]; }
        ~NSOpenGLContextGuard() { [mPrevContext makeCurrentContext]; }
    private:
         NSOpenGLContext *mPrevContext;
    };


    CocoaWindow::CocoaWindow() : GLWindow(), mWindow(nil), mView(nil), mGLContext(nil), mGLPixelFormat(nil), mWindowOriginPt(NSZeroPoint),
        mHasResized(false), mWindowTitle(""),
        mUseOgreGLView(true), mContentScalingFactor(1.0), mStyleMask(NSResizableWindowMask|NSTitledWindowMask)
    {
        // Set vsync by default to save battery and reduce tearing
    }

    CocoaWindow::~CocoaWindow()
    {
		[mGLContext clearDrawable];

        destroy();

        if(mView && mUseOgreGLView)
        {
            [(OgreGLView*)mView setOgreWindow:NULL];
        }
        
        if(mWindow && !mIsExternal)
        {
            [mWindow release];
            mWindow = nil;
        }
    }
	
	void CocoaWindow::create(const String& name, unsigned int widthPt, unsigned int heightPt,
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

        ***Key: "currentGLContext" Description: use an externally created OpenGL context (must be current)
         Values: true, false Default: false
        */

		BOOL hasDepthBuffer = YES;
		int fsaa_samples = 0;
        bool hidden = false;
        NSString *windowTitle = [NSString stringWithCString:name.c_str() encoding:NSUTF8StringEncoding];
		int winxPt = 0, winyPt = 0;
		int colourDepth = 32;
        int surfaceOrder = 1;
        int contextProfile = GLNativeSupport::CONTEXT_COMPATIBILITY;
        bool currentGLContext = false;
        NSOpenGLContext *externalGLContext = nil;
        NSObject* externalWindowHandle = nil; // NSOpenGLView, NSView or NSWindow
        NameValuePairList::const_iterator opt;
		
        mIsFullScreen = fullScreen;

		if(miscParams)
		{
			opt = miscParams->find("title");
			if(opt != miscParams->end())
				windowTitle = [NSString stringWithCString:opt->second.c_str() encoding:NSUTF8StringEncoding];
				
			opt = miscParams->find("left");
			if(opt != miscParams->end())
				winxPt = StringConverter::parseUnsignedInt(opt->second);
				
			opt = miscParams->find("top");
			if(opt != miscParams->end())
				winyPt = (int)NSHeight([[NSScreen mainScreen] frame]) - StringConverter::parseUnsignedInt(opt->second) - heightPt;

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

            opt = miscParams->find("vsync");
			if(opt != miscParams->end())
				mVSync = StringConverter::parseBool(opt->second);

			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				colourDepth = StringConverter::parseUnsignedInt(opt->second);

			opt = miscParams->find("Full Screen");
			if(opt != miscParams->end())
				fullScreen = StringConverter::parseBool(opt->second);

            opt = miscParams->find("contentScalingFactor");
            if(opt != miscParams->end())
                mContentScalingFactor = StringConverter::parseReal(opt->second);
            
            opt = miscParams->find("contextProfile");
            if(opt != miscParams->end())
                contextProfile = StringConverter::parseInt(opt->second);

            opt = miscParams->find("currentGLContext");
            if (opt != miscParams->end())
                currentGLContext = StringConverter::parseBool(opt->second);

            opt = miscParams->find("externalGLControl");
            if (opt != miscParams->end())
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            
            opt = miscParams->find("externalGLContext");
            if(opt != miscParams->end())
                externalGLContext = (NSOpenGLContext*)StringConverter::parseSizeT(opt->second);
            
            opt = miscParams->find("externalWindowHandle");
            if(opt != miscParams->end())
                externalWindowHandle = (NSObject*)StringConverter::parseSizeT(opt->second);
            
            opt = miscParams->find("border");
            if(opt != miscParams->end())
            {
                String border = opt->second;
                if (border == "none")
                {
                    mStyleMask = NSBorderlessWindowMask;
                }
                else if (border == "fixed")
                {
                    mStyleMask = NSTitledWindowMask;
                }
                // Default case set in initializer.
            }

            opt = miscParams->find("NSOpenGLCPSurfaceOrder");
            if(opt != miscParams->end())
                surfaceOrder = StringConverter::parseInt(opt->second);

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			opt = miscParams->find("stereoMode");
			if (opt != miscParams->end())
			{
				mStereoEnabled = StringConverter::parseBool(opt->second);
			}
#endif
        }

        if(externalGLContext)
        {
            mGLContext = [externalGLContext retain];
            mGLPixelFormat = [externalGLContext.pixelFormat retain];
        }
        else if(currentGLContext)
        {
            mGLContext = [[NSOpenGLContext currentContext] retain];
            mGLPixelFormat = [mGLContext.pixelFormat retain];
        }
        else if(!mIsExternalGLControl)
        {
            NSOpenGLPixelFormatAttribute attribs[30];
            int i = 0;
            
            // Specify the display ID to associate the GL context with (main display for now)
            // Useful if there is ambiguity
            attribs[i++] = NSOpenGLPFAScreenMask;
            attribs[i++] = (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(CGMainDisplayID());

            // Specify that we want to use the OpenGL 3.2 Core profile
            attribs[i++] = NSOpenGLPFAOpenGLProfile;
            attribs[i++] = contextProfile == GLNativeSupport::CONTEXT_CORE ? NSOpenGLProfileVersion3_2Core : NSOpenGLProfileVersionLegacy;

            // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.
            // This makes the View-based context a compatible with the fullscreen context, enabling us to use
            // the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
            attribs[i++] = NSOpenGLPFANoRecovery;
            
            attribs[i++] = NSOpenGLPFAAccelerated;
            attribs[i++] = NSOpenGLPFADoubleBuffer;

            attribs[i++] = NSOpenGLPFAColorSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) colourDepth;

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

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			if (mStereoEnabled)
				attribs[i++] = NSOpenGLPFAStereo;
#endif

            attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;

            mGLPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
            
            GLRenderSystemCommon *rs = static_cast<GLRenderSystemCommon*>(Root::getSingleton().getRenderSystem());
            CocoaContext *mainContext = (CocoaContext*)rs->_getMainContext();
            NSOpenGLContext *shareContext = mainContext == 0 ? nil : mainContext->getContext();
            mGLContext = [[NSOpenGLContext alloc] initWithFormat:mGLPixelFormat shareContext:shareContext];
        }

        if(!mIsExternalGLControl)
        {
            // Set vsync
            GLint swapInterval = (GLint)mVSync;
            [mGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
            GLint order = (GLint)surfaceOrder;
            [mGLContext setValues:&order forParameter:NSOpenGLCPSurfaceOrder];
        }
        
        // Make active
        setHidden(hidden);
		mActive = true;
        mVisible = true;
        mClosed = false;
        mName = [windowTitle cStringUsingEncoding:NSUTF8StringEncoding];
        mWidth = _getPixelFromPoint(widthPt);
        mHeight = _getPixelFromPoint(heightPt);
        mFSAA = fsaa_samples;

        if(!externalWindowHandle)
        {
            createNewWindow(widthPt, heightPt, [windowTitle cStringUsingEncoding:NSUTF8StringEncoding]);
        }
        else
        {
            if([externalWindowHandle isKindOfClass:[NSWindow class]])
            {
                mView = [(NSWindow*)externalWindowHandle contentView];
                mUseOgreGLView = [mView isKindOfClass:[OgreGLView class]];
                LogManager::getSingleton().logMessage(mUseOgreGLView ?
                    "Mac Cocoa Window: Rendering on an external NSWindow with nested OgreGLView" :
                    "Mac Cocoa Window: Rendering on an external NSWindow with nested NSView");
            }
            else
            {
                assert([externalWindowHandle isKindOfClass:[NSView class]]);
                mView = (NSView*)externalWindowHandle;
                mUseOgreGLView = [mView isKindOfClass:[OgreGLView class]];
                LogManager::getSingleton().logMessage(mUseOgreGLView ?
                    "Mac Cocoa Window: Rendering on an external OgreGLView" :
                    "Mac Cocoa Window: Rendering on an external NSView");
            }
            
            if(mUseOgreGLView)
            {
                [(OgreGLView*)mView setOgreWindow:this];
            }

            NSRect b = [mView bounds];
            mWidth = _getPixelFromPoint((int)b.size.width);
            mHeight = _getPixelFromPoint((int)b.size.height);

            mWindow = [mView window];
            mIsExternal = true;
        }

        // Create register the context with the rendersystem and associate it with this window
        mContext = OGRE_NEW CocoaContext(mGLContext, mGLPixelFormat);

        if(mContentScalingFactor > 1.0)
            [mView setWantsBestResolutionOpenGLSurface:YES];

        CGLLockContext((CGLContextObj)[mGLContext CGLContextObj]);

        [mView setNeedsDisplay:YES];

        if([mGLContext view] != mView)
            [mGLContext setView:mView];
        [mGLContext makeCurrentContext];

#if OGRE_DEBUG_MODE
        // Crash on functions that have been removed from the API
        CGLEnable((CGLContextObj)[mGLContext CGLContextObj], kCGLCECrashOnRemovedFunctions);
#endif

        // Enable GL multithreading
        CGLEnable((CGLContextObj)[mGLContext CGLContextObj], kCGLCEMPEngine);

        [mGLContext update];

//        rs->clearFrameBuffer(FBT_COLOUR);

        [mGLContext flushBuffer];
        CGLUnlockContext((CGLContextObj)[mGLContext CGLContextObj]);

        [pool drain];

        StringStream ss;
        ss  << "Cocoa: Window created " << widthPt << " x " << heightPt
        << " with backing store size " << mWidth << " x " << mHeight
        << " using content scaling factor " << std::fixed << std::setprecision(1) << getViewPointToPixelScale();
        LogManager::getSingleton().logMessage(ss.str());
    }

    unsigned int CocoaWindow::getWidth() const
    {
        // keep mWidth in sync with reality
        OgreAssertDbg(mView == nil || int(mWidth) == _getPixelFromPoint([mView frame].size.width),
                      "Window dimension mismatch. Did you call windowMovedOrResized?");

        return mWidth;
    }

    unsigned int CocoaWindow::getHeight() const
    {
        // keep mHeight in sync with reality
        OgreAssertDbg(mView == nil || int(mHeight) == _getPixelFromPoint([mView frame].size.height),
                      "Window dimension mismatch. Did you call windowMovedOrResized?");

        return mHeight;
    }

    void CocoaWindow::destroy(void)
    {
        if(!mIsFullScreen)
        {
            // Unregister and destroy OGRE GLContext
            OGRE_DELETE mContext;

            if(mWindow && !mIsExternal)
            {
                [mWindow performClose:nil];
            }
        }
        
        if(mGLContext)
        {
            [mGLContext release];
            mGLContext = nil;
        }
        
        if(mGLPixelFormat)
        {
            [mGLPixelFormat release];
            mGLPixelFormat = nil;
        }
        
        mActive = false;
        mClosed = true;
    }

    void CocoaWindow::setHidden(bool hidden)
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

	void CocoaWindow::setVSyncEnabled(bool vsync)
	{
        mVSync = vsync;
        mContext->setCurrent();
        
        GLint vsyncInterval = mVSync ? 1 : 0;
        [mGLContext setValues:&vsyncInterval forParameter:NSOpenGLCPSwapInterval];

        mContext->endCurrent();
        
        if(mGLContext != [NSOpenGLContext currentContext])
            [mGLContext makeCurrentContext];
	}

    float CocoaWindow::getViewPointToPixelScale()
    {
        return mContentScalingFactor > 1.0f ? mContentScalingFactor : 1.0f;
    }
    
    int CocoaWindow::_getPixelFromPoint(int viewPt) const
    {
        return mContentScalingFactor > 1.0 ? viewPt * mContentScalingFactor : viewPt;
    }
    
    void CocoaWindow::reposition(int leftPt, int topPt)
    {
		if(!mWindow)
            return;

        if(mIsFullScreen)
            return;

        NSRect frame = mIsExternal ? [[mView window] frame] : [mWindow frame];
        NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
		frame.origin.x = leftPt;
		frame.origin.y = screenFrame.size.height - frame.size.height - topPt;
        mWindowOriginPt = frame.origin;

        if(mIsExternal)
            [[mView window] setFrame:frame display:YES];
        else
            [mWindow setFrame:frame display:YES];

        // Keep our size up to date
        NSRect b = [mView bounds];
        mWidth = _getPixelFromPoint((int)b.size.width);
        mHeight = _getPixelFromPoint((int)b.size.height);
    }

    void CocoaWindow::resize(unsigned int widthPt, unsigned int heightPt)
    {
		if(!mWindow)
            return;

        if(mIsFullScreen)
            return;

        unsigned widthPx = _getPixelFromPoint(widthPt);
        unsigned heightPx = _getPixelFromPoint(heightPt);
        
        // Check if the window size really changed
        if(mWidth == widthPx && mHeight == heightPx)
            return;

        mWidth = widthPx;
        mHeight = heightPx;

        if(mIsExternal)
        {
            NSRect viewFrame = [mView frame];
            viewFrame.size.width = widthPt;
            viewFrame.size.height = heightPt;

            NSRect windowFrame = [[mView window] frame];

            CGFloat leftPt = viewFrame.origin.x;
            CGFloat topPt = windowFrame.size.height - (viewFrame.origin.y + viewFrame.size.height);
            mLeft = _getPixelFromPoint((int)leftPt);
            mTop = _getPixelFromPoint((int)topPt);
            mWindowOriginPt = NSMakePoint(leftPt, topPt);

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
            mWindowOriginPt = [mWindow frame].origin;
            [mWindow setContentSize:NSMakeSize(widthPt, heightPt)];
        }
        //make sure the context is current
        NSOpenGLContextGuard ctx_guard(mGLContext);
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
		[mGLContext update];
    }

    void CocoaWindow::windowHasResized()
    {
        // Ensure the context is current
        [mGLContext flushBuffer];
    }
    
    void CocoaWindow::windowMovedOrResized()
    {
        mContentScalingFactor =
            ([mView respondsToSelector:@selector(wantsBestResolutionOpenGLSurface)] && [(id)mView wantsBestResolutionOpenGLSurface]) ?
            (mView.window.screen ?: [NSScreen mainScreen]).backingScaleFactor : 1.0f;

        NSRect winFrame = mIsExternal ? [[mView window] frame] : [mWindow frame];
        NSRect viewFrame = [mView frame];
        NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];
        CGFloat leftPt = winFrame.origin.x;
        CGFloat topPt = screenFrame.size.height - winFrame.size.height;
        mWidth = _getPixelFromPoint((unsigned int)viewFrame.size.width);
        mHeight = _getPixelFromPoint((unsigned int)viewFrame.size.height);
        mLeft = _getPixelFromPoint((int)leftPt);
        mTop = _getPixelFromPoint((int)topPt);

        mWindowOriginPt = NSMakePoint(leftPt, topPt);

        //make sure the context is current
        NSOpenGLContextGuard ctx_guard(mGLContext);

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
		[mGLContext update];
    }

    void CocoaWindow::swapBuffers()
    {
        if(!mIsExternalGLControl)
        {
            CGLLockContext((CGLContextObj)[mGLContext CGLContextObj]);
            [mGLContext makeCurrentContext];

            if([mGLContext view] != mView)
                [mGLContext setView:mView];

            [mGLContext flushBuffer];
            CGLUnlockContext((CGLContextObj)[mGLContext CGLContextObj]);
        }
    }
	
	//-------------------------------------------------------------------------------------------------//
	void CocoaWindow::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "GLCONTEXT" ) 
		{
			*static_cast<GLContext**>(pData) = mContext;
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

    void CocoaWindow::createNewWindow(unsigned int widthPt, unsigned int heightPt, String title)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Builtin Window creation broken. Use an external Window (e.g SDL2) or fix this");

        // Get the dimensions of the display. We will use it for the window size but not context resolution
        NSRect windowRect = NSZeroRect;
        if(mIsFullScreen)
        {
            NSRect mainDisplayRect = [[NSScreen mainScreen] visibleFrame];
            windowRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
        }
        else
            windowRect = NSMakeRect(0.0, 0.0, widthPt, heightPt);

        mWindow = [[OgreGLWindow alloc] initWithContentRect:windowRect
                                              styleMask:mIsFullScreen ? NSBorderlessWindowMask : mStyleMask
                                                backing:NSBackingStoreBuffered
                                                  defer:YES];
        [mWindow setTitle:[NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding]];
        mWindowTitle = title;

        mView = [[OgreGLView alloc] initWithGLOSXWindow:this];

        _setWindowParameters(widthPt, heightPt);

//        GL3PlusRenderSystem *rs = static_cast<GL3PlusRenderSystem*>(Root::getSingleton().getRenderSystem());
//        rs->clearFrameBuffer(FBT_COLOUR);

        // Show window
        setHidden(mHidden);
    }

    void CocoaWindow::createWindowFromExternal(NSView *viewRef)
    {
        LogManager::getSingleton().logMessage("Creating external window");

        NSRect viewBounds = [mView convertRectToBacking:[mView bounds]];

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

    void CocoaWindow::_setWindowParameters(unsigned int widthPt, unsigned int heightPt)
    {
        if(mWindow)
        {
            if(mIsFullScreen)
            {
                // Set the backing store size to the viewport dimensions
                // This ensures that it will scale to the full screen size
                NSRect mainDisplayRect = [[NSScreen mainScreen] frame];
                NSRect backingRect = NSZeroRect;
                if(mContentScalingFactor > 1.0)
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
                
                mWindowOriginPt = mWindow.frame.origin;
                mLeft = mTop = 0;
            }
            else
            {
                // Reset and disable the backing store in windowed mode
                GLint backingStoreDimensions[2] = { 0, 0 };
                CGLSetParameter((CGLContextObj)[mGLContext CGLContextObj], kCGLCPSurfaceBackingSize, backingStoreDimensions);
                CGLDisable((CGLContextObj)[mGLContext CGLContextObj], kCGLCESurfaceBackingSize);

                NSRect viewRect = NSMakeRect(mWindowOriginPt.x, mWindowOriginPt.y, widthPt, heightPt);
                [mWindow setFrame:viewRect display:YES];
                [mView setFrame:viewRect];
                [mWindow setStyleMask:mStyleMask];
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

    void CocoaWindow::setFullscreen(bool fullScreen, unsigned int widthPt, unsigned int heightPt)
    {
        unsigned widthPx = _getPixelFromPoint(widthPt);
        unsigned heightPx = _getPixelFromPoint(heightPt);
        if (mIsFullScreen != fullScreen || widthPx != mWidth || heightPx != mHeight)
        {
            // Set the full screen flag
			mIsFullScreen = fullScreen;

            // Create a window if we haven't already, existence check is done within the functions
            if(!mWindow)
            {
                if(mIsExternal)
                    createWindowFromExternal(mView);
                else
                    createNewWindow(widthPt, heightPt, mWindowTitle);
            }

            _setWindowParameters(widthPt, heightPt);

            mWidth = widthPx;
            mHeight = heightPx;
        }
    }
}
