/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreOSXCocoaWindow.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLRenderSystem.h"
#include "OgreOSXCGLContext.h"

namespace Ogre {

    OSXCocoaWindow::OSXCocoaWindow() : mWindow(nil), mView(nil), mGLContext(nil), mActive(false),
        mClosed(false), mHasResized(false), mIsExternal(false), mWindowTitle(""), mUseNSView(false)
    {
		mContext = nil;
    }

    OSXCocoaWindow::~OSXCocoaWindow()
    {
		[mGLContext clearDrawable];
		CGReleaseAllDisplays();

        destroy();

        if(mWindow)
        {
            [mWindow release];
            mWindow = nil;
        }
    }
	
	void OSXCocoaWindow::create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams)
    {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSApplicationLoad();

		/*
        ***Key: "title" Description: The title of the window that will appear in the title bar Values: string Default: RenderTarget name

        ***Key: "colourDepth" Description: Colour depth of the resulting rendering window; only applies if fullScreen is set. Values: 16 or 32 Default: desktop depth Notes: [W32 specific]

        ***Key: "left" Description: screen x coordinate from left Values: positive integers Default: 'center window on screen' Notes: Ignored in case of full screen

        ***Key: "top" Description: screen y coordinate from top Values: positive integers Default: 'center window on screen' Notes: Ignored in case of full screen

        ***Key: "depthBuffer" [DX9 specific] Description: Use depth buffer Values: false or true Default: true

        ***Key: "externalWindowHandle" [API specific] Description: External window handle, for embedding the OGRE context Values: positive integer for W32 (HWND handle) poslong:posint:poslong (display*:screen:windowHandle) or poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*) for GLX Default: 0 (None)

        ***Key: "FSAA" Description: Full screen antialiasing factor Values: 0,2,4,6,... Default: 0

        ***Key: "displayFrequency" Description: Display frequency rate, for fullscreen mode Values: 60...? Default: Desktop vsync rate

        ***Key: "vsync" Description: Synchronize buffer swaps to vsync Values: true, false Default: 0
        */

		BOOL hasDepthBuffer = YES;
		int fsaa_samples = 0;
		NSString *windowTitle = [NSString stringWithCString:name.c_str() encoding:NSUTF8StringEncoding];
		int winx = 0, winy = 0;
		int depth = 32;
        NameValuePairList::const_iterator opt(NULL);
		
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
				winy = NSHeight([[NSScreen mainScreen] frame]) - StringConverter::parseUnsignedInt(opt->second) - height;

			opt = miscParams->find("depthBuffer");
			if(opt != miscParams->end())
				hasDepthBuffer = StringConverter::parseBool(opt->second);
				
			opt = miscParams->find("FSAA");
			if(opt != miscParams->end())
				fsaa_samples = StringConverter::parseUnsignedInt(opt->second);
			
			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				depth = StringConverter::parseUnsignedInt(opt->second);

            opt = miscParams->find("Full Screen");
            if(opt != miscParams->end())
                fullScreen = StringConverter::parseBool(opt->second);
        }		

        if(fullScreen)
        {
            GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
            OSXContext *mainContext = (OSXContext*)rs->_getMainContext();
    
            CGLContextObj share = NULL;
            if(mainContext == 0)
            {
                share = NULL;
            }
            else if(mainContext->getContextType() == "NSOpenGL")
            {
                OSXCocoaContext* cocoaContext = static_cast<OSXCocoaContext*>(mainContext);
                NSOpenGLContext* nsShare = cocoaContext->getContext();
                share = (CGLContextObj)[nsShare CGLContextObj];
            }
            else if(mainContext->getContextType() == "CGL")
            {
                OSXCGLContext* cglShare = static_cast<OSXCGLContext*>(mainContext);
                share = cglShare->getContext();
            }
    
            // create the context
            createCGLFullscreen(width, height, depth, fsaa_samples, share);	
        }
        else
        {
            NSOpenGLPixelFormat* openglFormat = nil;
            NSOpenGLPixelFormatAttribute attribs[30];
            int i = 0;
            
            // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.  This makes the View-based context a compatible with the fullscreen context, enabling us to use the "shareContext" feature to share textures, display lists, and other OpenGL objects between the two.
            attribs[i++] = NSOpenGLPFANoRecovery;
            
            attribs[i++] = NSOpenGLPFAAccelerated;
            attribs[i++] = NSOpenGLPFADoubleBuffer;
            
            attribs[i++] = NSOpenGLPFAAlphaSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;
            
            attribs[i++] = NSOpenGLPFAStencilSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;
            
            attribs[i++] = NSOpenGLPFAAccumSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;
            
            attribs[i++] = NSOpenGLPFADepthSize;
            attribs[i++] = (NSOpenGLPixelFormatAttribute) depth;
            
            if(fsaa_samples > 0)
            {
                attribs[i++] = NSOpenGLPFASampleBuffers;
                attribs[i++] = (NSOpenGLPixelFormatAttribute) 1;
                
                attribs[i++] = NSOpenGLPFASamples;
                attribs[i++] = (NSOpenGLPixelFormatAttribute) fsaa_samples;
            }
            
            attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;

            openglFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];

			GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
			OSXCocoaContext *mainContext = (OSXCocoaContext*)rs->_getMainContext();
			NSOpenGLContext *shareContext = mainContext == 0? nil : mainContext->getContext();
            mGLContext = [[NSOpenGLContext alloc] initWithFormat:openglFormat shareContext:shareContext];

            if(!miscParams || opt == miscParams->end())
            {
                createNewWindow(width, height, [windowTitle cStringUsingEncoding:NSUTF8StringEncoding]);
            }
            else
            {
                NameValuePairList::const_iterator param_useNSView_pair(NULL);
                if(miscParams)
                {
                    opt = miscParams->find("externalWindowHandle");
                    param_useNSView_pair = miscParams->find("macAPICocoaUseNSView");
                }

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
                    mWidth = b.size.width;
                    mHeight = b.size.height;
                }

                createWindowFromExternal(mView);
            }

            // Create register the context with the rendersystem and associate it with this window
            mContext = OGRE_NEW OSXCocoaContext(mGLContext, openglFormat);
        }
		// make active
		mActive = true;
        mClosed = false;
        mName = [windowTitle cStringUsingEncoding:NSUTF8StringEncoding];
        mWidth = width;
        mHeight = height;
        mColourDepth = depth;
        mFSAA = fsaa_samples;
        mIsFullScreen = fullScreen;

        [pool drain];
    }

    void OSXCocoaWindow::destroy(void)
    {
        if(mIsFullScreen)
        {
            // Handle fullscreen destruction
			destroyCGLFullscreen();
        }
        else
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
                [mWindow close];
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

    void OSXCocoaWindow::reposition(int left, int top)
    {
		if(!mWindow) return;
		
		NSRect frame = [mWindow frame];
		frame.origin.x = left;
		frame.origin.y = top-frame.size.height;
		[mWindow setFrame:frame display:YES];
    }

    void OSXCocoaWindow::resize(unsigned int width, unsigned int height)
    {
		if(!mWindow) return;
		
		NSRect frame = [mWindow frame];
		frame.size.width = width;
		frame.size.height = height;
		[mWindow setFrame:frame display:YES];
    }

    void OSXCocoaWindow::windowMovedOrResized()
    {
		[mGLContext update];
		NSRect frame = [mView frame];
		mWidth = (unsigned int)frame.size.width;
		mHeight = (unsigned int)frame.size.height;
        mLeft = (int)frame.origin.x;
        mTop = (int)frame.origin.y+(unsigned int)frame.size.height;
		
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
    }

    void OSXCocoaWindow::swapBuffers(bool waitForVSync)
    {
		if(!mIsFullScreen)
			[mGLContext flushBuffer];
		else
			swapCGLBuffers();
    }
	
	//-------------------------------------------------------------------------------------------------//
	void OSXCocoaWindow::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "GLCONTEXT" ) 
		{
			*static_cast<OSXContext**>(pData) = mContext;
			return;
		} 
		if( name == "WINDOW" ) 
		{
			*(void**)pData = mWindow;
			return;
		} 
		if( name == "VIEW" ) 
		{
			*(void**)(pData) = mView;
			return;
		} 
	}

    void OSXCocoaWindow::createNewWindow(unsigned int width, unsigned int height, String title)
    {
        mWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
                                              styleMask:NSResizableWindowMask
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
        [mWindow setTitle:[NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding]];
        mWindowTitle = title;

        [mWindow center];
        
        mView = [[OgreView alloc] initWithGLOSXWindow:this];
        [mWindow setContentView:mView];
        [mWindow makeFirstResponder:mView];

        [mGLContext setView:mView];

        // Show window
        if(mWindow)
            [mWindow makeKeyAndOrderFront:nil];

        // Add our window to the window event listener class
        WindowEventUtilities::_addRenderWindow(this);
    }

    void OSXCocoaWindow::createWindowFromExternal(NSView *viewRef)
    {
        mWindow = [viewRef window];

        [mWindow center];
        
        mView = viewRef;
        [mWindow setContentView:mView];
        [mWindow makeFirstResponder:mView];

        [mGLContext setView:mView];

        // Show window
        if(mWindow)
            [mWindow makeKeyAndOrderFront:nil];

        // Add our window to the window event listener class
        WindowEventUtilities::_addRenderWindow(this);
    }

    void OSXCocoaWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
    {
        if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
        {
            // Set the full screen flag
			mIsFullScreen = fullScreen;

            if(mIsFullScreen) {
                GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
                OSXContext *mainContext = (OSXContext *)rs->_getMainContext();

                CGLContextObj share = NULL;
                if(mainContext == 0)
                {
                    share = NULL;
                }
                else if(mainContext->getContextType() == "NSOpenGL")
                {
                    OSXCocoaContext* cocoaContext = static_cast<OSXCocoaContext*>(mainContext);
                    NSOpenGLContext* nsShare = cocoaContext->getContext();
                    share = (CGLContextObj)[nsShare CGLContextObj];
                }
                else if(mainContext->getContextType() == "CGL")
                {
                    OSXCGLContext* cglShare = static_cast<OSXCGLContext*>(mainContext);
                    share = cglShare->getContext();
                }
                
                // create the context
                createCGLFullscreen(width, height, getColourDepth(), getFSAA(), share);

                WindowEventUtilities::_removeRenderWindow(this);
            }
            else
            {
                // Create a window if we haven't already, existence check is done within the functions
                if(mIsExternal)
                    createWindowFromExternal(mView);
                else
                    createNewWindow(width, height, mWindowTitle);

                // Destroy the current CGL context, we will create a new one when/if we go back to full screen
                destroyCGLFullscreen();

                // Set the drawable, and current context
                // If you do this last, there is a moment before the rendering window pops-up
                [mGLContext setView:mView];

                // Show window
                if(mWindow)
                    [mWindow performSelectorOnMainThread:@selector(makeKeyAndOrderFront:) withObject:NULL waitUntilDone:NO];
            }
            mWidth = width;
            mHeight = height;
        }
    }
}
