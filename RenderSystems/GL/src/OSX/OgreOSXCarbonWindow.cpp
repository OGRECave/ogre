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

// 64-bit Mac OS X doesn't support Carbon UI API
#ifndef __LP64__

#include "OgreOSXCarbonWindow.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreWindowEventUtilities.h"

namespace Ogre
{

    //-------------------------------------------------------------------------------------------------//
    OSXCarbonWindow::OSXCarbonWindow() : mWindow(NULL), mEventHandlerRef(NULL), mView(NULL), mAGLContext(NULL), mAGLPixelFormat(NULL),
        mWindowTitle(""), mCGLContext(NULL), mCarbonContext(NULL), mActive(false), mClosed(false), mCreated(false), mHasResized(false),
        mIsExternal(false), mVisible(false)
    {
        mIsFullScreen = false;
        mContext = NULL;
        mColourDepth = 32;
        mVSync = false;
    }

    //-------------------------------------------------------------------------------------------------//
    OSXCarbonWindow::~OSXCarbonWindow()
    {
        destroy();
    }

    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::create( const String& name, unsigned int width, unsigned int height,
                    bool fullScreen, const NameValuePairList *miscParams )
    {
        bool hasDepthBuffer = false;
        String title = name;
        size_t fsaa_samples = 0;
        int left = 0;
        int top = 0;
        bool vsync = false;
        bool hidden = false;
        int depth = 32;

        if( miscParams )
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            // Full screen anti aliasing
            if((opt = miscParams->find("FSAA")) != end)
                fsaa_samples = StringConverter::parseUnsignedInt( opt->second );

            if((opt = miscParams->find("left")) != end) 
                left = StringConverter::parseUnsignedInt( opt->second );

            if((opt = miscParams->find("top")) != end) 
                top = StringConverter::parseUnsignedInt( opt->second );

            if((opt = miscParams->find("title")) != end) 
                title = opt->second;

            if((opt = miscParams->find("vsync")) != end) 
                vsync = StringConverter::parseBool(opt->second);

            if((opt = miscParams->find("hidden")) != end) 
                hidden = StringConverter::parseBool(opt->second);

            if((opt = miscParams->find("gamma")) != end) 
				mHwGamma = StringConverter::parseBool(opt->second);

            if((opt = miscParams->find("depthBuffer")) != end) 
                hasDepthBuffer = StringConverter::parseBool( opt->second );
            
            if((opt = miscParams->find("colourDepth")) != end) 
                depth = StringConverter::parseUnsignedInt( opt->second );

            if((opt = miscParams->find("Full Screen")) != end) 
                fullScreen = StringConverter::parseBool( opt->second );
        }

        if(fullScreen)
        {
            setFullscreen(fullScreen, width, height);
        }
        else
        {
            createAGLContext(fsaa_samples, depth);

            NameValuePairList::const_iterator opt;
            if(miscParams)
                opt = miscParams->find("externalWindowHandle");

            if(!miscParams || opt == miscParams->end())
                createNewWindow(width, height, title.c_str());
            else
                createWindowFromExternal((HIViewRef)StringConverter::parseUnsignedLong(opt->second));

            // Set the drawable, and current context
            // If you do this last, there is a moment before the rendering window pops-up
            // This could go once inside each case above, before the window is displayed,
            // if desired.
    #if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
            aglSetDrawable(mAGLContext, GetWindowPort(mWindow));
    #else
            aglSetWindowRef(mAGLContext, mWindow);
    #endif
            aglSetCurrentContext(mAGLContext);

            // Give a copy of our context to the render system
            if(!mCarbonContext)
            {
                mCarbonContext = OGRE_NEW OSXCarbonContext(mAGLContext, mAGLPixelFormat);
                mContext = mCarbonContext;
            }
        }

        // Apply vsync settings. call setVSyncInterval first to avoid 
		// setting vsync more than once.
        setVSyncEnabled(vsync);
        setHidden(hidden);

        mName = name;
        mWidth = width;
        mHeight = height;
        mColourDepth = depth;
        mFSAA = fsaa_samples;
        mIsFullScreen = fullScreen;
        mActive = true;
        mClosed = false;
        mCreated = true;
    }

    void OSXCarbonWindow::createAGLContext(size_t fsaa_samples, int depth)
    {
        if(!mAGLContext)
        {
            int i = 0;
            GLint attribs[ 20 ];
            
            attribs[ i++ ] = AGL_NO_RECOVERY;
            attribs[ i++ ] = GL_TRUE;
            attribs[ i++ ] = AGL_ACCELERATED;
            attribs[ i++ ] = GL_TRUE;
            attribs[ i++ ] = AGL_RGBA;
            attribs[ i++ ] = AGL_DOUBLEBUFFER;
            attribs[ i++ ] = AGL_ALPHA_SIZE;
            attribs[ i++ ] = 8;
            attribs[ i++ ] = AGL_STENCIL_SIZE;
            attribs[ i++ ] = 8;
            attribs[ i++ ] = AGL_DEPTH_SIZE;
            attribs[ i++ ] = depth;
            
            if(fsaa_samples > 1)
            {
                attribs[ i++ ] = AGL_MULTISAMPLE;
                attribs[ i++ ] = AGL_SAMPLE_BUFFERS_ARB;
                attribs[ i++ ] = 1;
                attribs[ i++ ] = AGL_SAMPLES_ARB;
                attribs[ i++ ] = fsaa_samples;
            }
            
            attribs[ i++ ] = AGL_NONE;
            
            mAGLPixelFormat = aglChoosePixelFormat( NULL, 0, attribs );

            if(!mAGLPixelFormat)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unable to create a valid pixel format with selected attributes.",
                            "OSXCarbonWindow::createAGLContext" );
            }
            
            // Create the AGLContext from our pixel format
            // Share it with main
            GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
            OSXContext* mainContext = static_cast<OSXContext*>( rs->_getMainContext() );
            if(mainContext == 0 || !(mainContext->getContextType() == "AGL"))
            {
                mAGLContext = aglCreateContext(mAGLPixelFormat, NULL);
            }
            else
            {
                OSXCarbonContext* context = static_cast<OSXCarbonContext*>( rs->_getMainContext() );
                mAGLContext = aglCreateContext(mAGLPixelFormat, context->getContext());
            }
        }
    }

    void OSXCarbonWindow::createNewWindow(unsigned int width, unsigned int height, String title)
    {
        if(!mWindow)
        {
            // Create the window rect in global coords
            ::Rect windowRect;
            windowRect.left = 0;
            windowRect.top = 0;
            windowRect.right = width;
            windowRect.bottom = height;
            
            // Set the default attributes for the window
            WindowAttributes windowAttrs = kWindowStandardDocumentAttributes; // default: "resize"

            windowAttrs |= kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute | kWindowHideOnFullScreenAttribute | kWindowNoShadowAttribute;

            // Create the window
            CreateNewWindow(kDocumentWindowClass, windowAttrs, &windowRect, &mWindow);

            // Color the window background black
            SetThemeWindowBackground(mWindow, kThemeBrushBlack, true);
            
            // Set the title of our window
            CFStringRef titleRef = CFStringCreateWithCString( kCFAllocatorDefault, title.c_str(), kCFStringEncodingASCII );
            SetWindowTitleWithCFString( mWindow, titleRef );
            CFRelease(titleRef);
            mWindowTitle = title;
            
            // Center our window on the screen
            RepositionWindow( mWindow, NULL, kWindowCenterOnMainScreen );
            
            // Get our view
            HIViewFindByID( HIViewGetRoot( mWindow ), kHIViewWindowContentID, &mView );
            
            // Set up our UPP for Window Events
            EventTypeSpec eventSpecs[] = {
                {kEventClassWindow, kEventWindowActivated},
                {kEventClassWindow, kEventWindowDeactivated},
                {kEventClassWindow, kEventWindowShown},
                {kEventClassWindow, kEventWindowHidden},
                {kEventClassWindow, kEventWindowDragCompleted},
                {kEventClassWindow, kEventWindowBoundsChanged},
                {kEventClassWindow, kEventWindowExpanded},
                {kEventClassWindow, kEventWindowCollapsed},
                {kEventClassWindow, kEventWindowClosed},
                {kEventClassWindow, kEventWindowClose}
            };

            EventHandlerUPP handlerUPP = NewEventHandlerUPP(WindowEventUtilities::_CarbonWindowHandler);
            
            // Install the standard event handler for the window
            EventTargetRef target = GetWindowEventTarget(mWindow);
            InstallStandardEventHandler(target);
            
            // We also need to install the WindowEvent Handler, we pass along the window with our requests
            InstallEventHandler(target, handlerUPP, 10, eventSpecs, (void*)this, &mEventHandlerRef);
        }
        HIRect winBounds = CGRectZero;
        HIViewRef root = HIViewGetRoot(HIViewGetWindow(mView)); 
        HIViewGetBounds(root, &winBounds); 

        HIRect viewBounds = CGRectZero;
        HIViewGetBounds(mView, &viewBounds);

        // Display and select our window
        if(!mHidden && mVisible)
        {
            ShowWindow(mWindow);
            SelectWindow(mWindow);
        }

        // Add our window to the window event listener class
        WindowEventUtilities::_addRenderWindow(this);
    }
    
    void OSXCarbonWindow::createWindowFromExternal(HIViewRef viewRef)
    {
        // TODO: The Control is going to report the incorrect location with a
        // Metallic / Textured window.  The default windows work just fine.
        
        // First get the HIViewRef / ControlRef
        mView = viewRef;
        mWindow = GetControlOwner(mView);
        // Lets try hiding the HIView
        //HIViewSetVisible(mView, false);
        // Get the rect bounds
        ::Rect ctrlBounds;
        GetControlBounds(mView, &ctrlBounds);
        GLint bufferRect[4];
        
        bufferRect[0] = ctrlBounds.left;					// left edge
        bufferRect[1] = ctrlBounds.bottom;					// bottom edge
        bufferRect[2] =	ctrlBounds.right - ctrlBounds.left; // width of buffer rect
        bufferRect[3] = ctrlBounds.bottom - ctrlBounds.top; // height of buffer rect
        
        aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect);
        aglEnable(mAGLContext, AGL_BUFFER_RECT);
        
        mIsExternal = true;
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::destroy(void)
    {
        if(!mCreated)
            return;

        if(mIsFullScreen)
        {
            // Handle fullscreen destruction
            destroyCGLFullscreen();
        }
        else
        {
            // Handle windowed destruction
            
            // Destroy the Ogre context
            if(mCGLContext)
            {
                OGRE_DELETE mCGLContext;
                mCGLContext = NULL;
            }
            
            if(mCarbonContext)
            {
                OGRE_DELETE mCarbonContext;
                mCarbonContext = NULL;
            }

            if(mContext)
            {
                // mContext is a reference to either the AGL or CGL context, already deleted.
                // Just clear the variable
                mContext = NULL;
            }

            if(!mIsExternal)
            {
                // Remove the window from the Window listener
                WindowEventUtilities::_removeRenderWindow( this );
                
                // Remove our event handler
                if(mEventHandlerRef)
                    RemoveEventHandler(mEventHandlerRef);        
            }

            if(mAGLContext)
                aglDestroyContext(mAGLContext);
            
            if(mWindow)
                DisposeWindow(mWindow);
        }

        mActive = false;
        mClosed = true;
        mCreated = false;
    }
    
    //-------------------------------------------------------------------------------------------------//
    bool OSXCarbonWindow::isActive() const
    {
        return mActive;
    }
    
    //-------------------------------------------------------------------------------------------------//
    bool OSXCarbonWindow::isClosed() const
    {
        return mClosed;
    }

    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::setHidden(bool hidden)
    {
        mHidden = hidden;
        if (!mIsExternal)
        {
            if (hidden)
                HideWindow(mWindow);
            else
                ShowWindow(mWindow);
        }
    }

    //-------------------------------------------------------------------------------------------------//
	void OSXCarbonWindow::setVSyncEnabled(bool vsync)
	{
        mVSync = vsync;
        mContext->setCurrent();

        CGLContextObj share = NULL;
        aglGetCGLContext(mAGLContext, (void**)&share);

        GLint vsyncInterval = mVSync ? 1 : 0;
        aglSetInteger(mAGLContext, AGL_SWAP_INTERVAL, &vsyncInterval);

        mContext->endCurrent();
    
        if(!mIsFullScreen)
        {
            if(mAGLContext != aglGetCurrentContext())
                aglSetCurrentContext(mAGLContext);
        }
        else
        {
            if(share != CGLGetCurrentContext())
                CGLSetCurrentContext(share);
        }
	}

    //-------------------------------------------------------------------------------------------------//
	bool OSXCarbonWindow::isVSyncEnabled() const
	{
        return mVSync;
	}

    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::reposition(int left, int top)
    {
        if(mWindow)
            MoveWindow(mWindow, left, top, true);
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::resize(unsigned int width, unsigned int height)
    {
        if(!mWindow)
            return;
        
        // Check if the window size really changed
        if(mWidth == width && mHeight == height)
            return;
        mWidth = width;
        mHeight = height;
        if (mIsExternal)
        {
            HIRect viewBounds = CGRectZero, winBounds = CGRectZero;
            HIViewGetBounds(mView, &viewBounds);
            HIViewRef root = HIViewGetRoot(HIViewGetWindow(mView));
            HIViewGetBounds(root, &winBounds);
            HIViewConvertRect(&viewBounds, mView, root);
            mLeft = viewBounds.origin.x;
            mTop = winBounds.size.height - (viewBounds.origin.y + viewBounds.size.height);
            
            // Set the AGL buffer rectangle (i.e. the bounds that we will use) 
            GLint bufferRect[4];      
            bufferRect[0] = mLeft; // 0 = left edge 
            bufferRect[1] = mTop; // 0 = bottom edge 
            bufferRect[2] = mWidth; // width of buffer rect 
            bufferRect[3] = mHeight; // height of buffer rect 
            aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect);
            for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it) 
            { 
                (*it).second->_updateDimensions(); 
            }
        }
        else
        {
            SizeWindow(mWindow, width, height, true);
        }
    }
    
    void OSXCarbonWindow::setVisible( bool visible )
    {
        mVisible = visible;
        if(mIsExternal && !visible) {
            GLint bufferRect[4] = {0, 0, 0, 0};
            aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect);
        }
    }
    
    bool OSXCarbonWindow::isVisible( void ) const
    {
        return mVisible;
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::windowHasResized()
    {
        mHasResized = true;
        
        // Ensure the context is current
        if(!mIsFullScreen)
            aglSwapBuffers(mAGLContext);
        else
            swapCGLBuffers();
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::windowResized()
    {
        // Ensure the context is current
        if(!mIsFullScreen)
        {
            // Determine the AGL_BUFFER_RECT for the view. The coordinate 
            // system for this rectangle is relative to the owning window, with 
            // the origin at the bottom left corner and the y-axis inverted. 
            HIRect newFrame = CGRectMake(mLeft, mTop+22, mWidth, mHeight);
            HIRect viewBounds = CGRectZero, winBounds = CGRectZero;

            SizeWindow(mWindow, mWidth, mHeight, true);
            HIViewSetFrame(mView, &newFrame);

            HIViewGetBounds(mView, &viewBounds);
            HIViewRef root = HIViewGetRoot(HIViewGetWindow(mView));

            HIViewGetBounds(root, &winBounds);
            HIViewConvertRect(&viewBounds, mView, root);

            // Set the AGL buffer rectangle (i.e. the bounds that we will use) 
            GLint bufferRect[4]; 
            bufferRect[0] = viewBounds.origin.x; // 0 = left edge 
            bufferRect[1] = winBounds.size.height - (viewBounds.origin.y + viewBounds.size.height); // 0 = bottom edge 
            bufferRect[2] = viewBounds.size.width; // width of buffer rect 
            bufferRect[3] = viewBounds.size.height; // height of buffer rect 

            aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect); 
            aglEnable(mAGLContext, AGL_BUFFER_RECT); 
            aglUpdateContext(mAGLContext);

            mLeft = viewBounds.origin.x; 
            mTop = bufferRect[1]; 
        }
        else
        {
            swapCGLBuffers();
        }
        
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it) 
        { 
            (*it).second->_updateDimensions(); 
        }
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::windowMovedOrResized()
    {
        // External windows will call this method.
        if(mView != NULL)
        {
            // Determine the AGL_BUFFER_RECT for the view. The coordinate 
            // system for this rectangle is relative to the owning window, with 
            // the origin at the bottom left corner and the y-axis inverted.
            
            // Also, when leaving fullscreen, the display properties are not guaranteed to be
            // the same as when we were windowed previously.  So resize the window and views back
            // to their original dimensions.
            HIRect newFrame = CGRectMake(mLeft, mTop+22, mWidth, mHeight);
            HIRect viewBounds = CGRectZero, winBounds = CGRectZero;

            SizeWindow(mWindow, mWidth, mHeight, true);
            HIViewSetFrame(mView, &newFrame);

            HIViewGetBounds(mView, &viewBounds);
            HIViewRef root = HIViewGetRoot(HIViewGetWindow(mView));

            HIViewGetBounds(root, &winBounds);
            HIViewConvertRect(&viewBounds, mView, root);

            // Set the AGL buffer rectangle (i.e. the bounds that we will use) 
            GLint bufferRect[4]; 
            bufferRect[0] = viewBounds.origin.x; // 0 = left edge 
            bufferRect[1] = winBounds.size.height - (viewBounds.origin.y + viewBounds.size.height); // 0 = bottom edge 
            bufferRect[2] = viewBounds.size.width; // width of buffer rect 
            bufferRect[3] = viewBounds.size.height; // height of buffer rect 
            
            aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect); 
            aglEnable(mAGLContext, AGL_BUFFER_RECT); 
            aglUpdateContext(mAGLContext);
            
            mLeft = viewBounds.origin.x; 
            mTop = bufferRect[1];
        }
        
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it) 
        { 
            (*it).second->_updateDimensions(); 
        }
    }
    
    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::swapBuffers( )
    {
        if(!mIsFullScreen)
        {
            if(mAGLContext != aglGetCurrentContext())
                aglSetCurrentContext(mAGLContext);
            
            aglSwapBuffers(mAGLContext);
        }
        else
        {
            swapCGLBuffers();
        }
        
        if(mHasResized)
        {
            windowResized();
            mHasResized = false;
        }
    }

    //-------------------------------------------------------------------------------------------------//
    void OSXCarbonWindow::getCustomAttribute( const String& name, void* pData )
    {
        if( name == "GLCONTEXT" ) 
        {
            *static_cast<OSXContext**>(pData) = mContext;
            return;
        }
        else if( name == "WINDOW" )
        {
            if(mIsFullScreen)
            {
                // A fullscreen application uses CGL and thus has no window.
                pData = 0;
                return;
            }
            else
            {
                *static_cast<WindowRef*>(pData) = mWindow;
            }
            return;
        }
    }
    
    void OSXCarbonWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
    {
        if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
        {
            // Set the full screen flag
            mIsFullScreen = fullScreen;

            createAGLContext(mFSAA, mColourDepth);

            if (mIsFullScreen)
            {
                CGLContextObj share = NULL;
                aglGetCGLContext(mAGLContext, (void**)&share);

                // Create the CGL context object if it doesn't already exist, sharing the AGL context.
                if(!mCGLContext)
                {
                    void *cglPixFormat;
                    aglGetCGLPixelFormat(mAGLPixelFormat, (void **)&cglPixFormat);
                    mCGLContext = OGRE_NEW OSXCGLContext(mCGLContextObj, (CGLPixelFormatObj) cglPixFormat);
                }

                // Create the context, keeping the current colour depth and FSAA settings
                createCGLFullscreen(width, height, getColourDepth(), getFSAA(), share);

                // Hide the Carbon window
                HideWindow(mWindow);

                // And tell the rendersystem to stop rendering to it too
                WindowEventUtilities::_removeRenderWindow(this);
            }
            else
            {
                // Create a new AGL context and pixel format if necessary
                createAGLContext(mFSAA, mColourDepth);

                // Create a window if we haven't already, existence check is done within the functions
                if(!mWindow)
                {
                    if(mIsExternal)
                        createWindowFromExternal(mView);
                    else
                        createNewWindow(width, height, mWindowTitle);
                }

                // Destroy the current CGL context, we will create a new one when/if we go back to full screen
                destroyCGLFullscreen();

                // Set the drawable, and current context
                // If you do this last, there is a moment before the rendering window pops-up
                #if defined(MAC_OS_X_VERSION_10_4) && MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
                    aglSetDrawable(mAGLContext, GetWindowPort(mWindow));
                #else
                    aglSetWindowRef(mAGLContext, mWindow);
                #endif
                aglSetCurrentContext(mAGLContext);

                if(!mCarbonContext)
                {
                    mCarbonContext = OGRE_NEW OSXCarbonContext(mAGLContext, mAGLPixelFormat);
                }
                
                GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
                mContext = mCarbonContext;
                rs->_switchContext(mContext);

                WindowEventUtilities::_addRenderWindow(this);

                ShowWindow(mWindow);
                SelectWindow(mWindow);
                RepositionWindow(mWindow, NULL, kWindowCenterOnMainScreen);
            }
            mWidth = width;
            mHeight = height;
        }
    }
}

#endif // __LP64__
