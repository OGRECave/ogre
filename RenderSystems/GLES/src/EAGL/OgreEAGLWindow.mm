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
--------------------------------------------------------------------------*/

#include "OgreEAGLWindow.h"

#include "OgreEAGLSupport.h"
#include "OgreEAGLESContext.h"

#include "OgreRoot.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESPixelFormat.h"
#include "OgreGLESRenderSystem.h"

namespace Ogre {
    EAGLWindow::EAGLWindow(EAGLSupport *glsupport)
        :   mClosed(false),
            mVisible(false),
            mIsExternal(false),
            mUsingExternalView(false),
            mUsingExternalViewController(false),
            mIsContentScalingSupported(false),
            mContentScalingFactor(1.0),
            mCurrentOSVersion(0.0),
            mGLSupport(glsupport),
            mContext(NULL),
            mWindow(nil),
            mView(nil),
            mViewController(nil)
    {
        mIsFullScreen = true;
        mActive = true;
        mHwGamma = false;

        // Check for content scaling.  iOS 4 or later
        mCurrentOSVersion = [[[UIDevice currentDevice] systemVersion] floatValue];
        if(mCurrentOSVersion >= 4.0)
            mIsContentScalingSupported = true;
    }

    EAGLWindow::~EAGLWindow()
    {
        destroy();

        if (mContext != NULL)
        {
            OGRE_DELETE mContext;
        }

        mContext = NULL;
    }

    void EAGLWindow::destroy(void)
    {
        if (mClosed)
        {
            return;
        }

        mClosed = true;
        mActive = false;

        if (!mIsExternal)
        {
            WindowEventUtilities::_removeRenderWindow(this);

            [mWindow release];
            mWindow = nil;
        }

        if (mIsFullScreen)
        {
            switchFullScreen(false);
        }
        
        if(!mUsingExternalView)
            [mView release];

        if(!mUsingExternalViewController)
            [mViewController release];
    }

    void EAGLWindow::setFullscreen(bool fullscreen, uint width, uint height)
    {
    }

    void EAGLWindow::reposition(int left, int top)
	{
	}
    
	void EAGLWindow::resize(unsigned int width, unsigned int height)
	{
        if(!mWindow) return;
        
        Real w = width * mContentScalingFactor;
        Real h = height * mContentScalingFactor;
        
        // Check if the window size really changed
        if(mWidth == w && mHeight == h)
            return;
        
        // Destroy and recreate the framebuffer with new dimensions 
        mContext->destroyFramebuffer();
        
        mWidth = w;
        mHeight = h;
        
        mContext->createFramebuffer();
        
        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
	}

	void EAGLWindow::windowMovedOrResized()
	{
		CGRect frame = [mView frame];
        mWidth = (unsigned int)frame.size.width * mContentScalingFactor;
        mHeight = (unsigned int)frame.size.height * mContentScalingFactor;
        mLeft = (int)frame.origin.x * mContentScalingFactor;
        mTop = ((int)frame.origin.y + (int)frame.size.height) * mContentScalingFactor;

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
	}

    void EAGLWindow::_beginUpdate(void)
    {
        // Call the base class method first
        RenderTarget::_beginUpdate();

        if(mContext->mIsMultiSampleSupported && mContext->mNumSamples > 0)
        {
            // Bind the FSAA buffer if we're doing multisampling
            glBindFramebufferOES(GL_FRAMEBUFFER_OES, mContext->mFSAAFramebuffer);
            GL_CHECK_ERROR
        }
    }

    void EAGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {
        // This method is called from within create() and after parameters have been parsed.
        // If the window, view or view controller objects are nil at this point, it is safe
        // to assume that external handles are either not being used or are invalid and
        // we can create our own.
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

        uint w = 0, h = 0;

        ConfigOptionMap::const_iterator opt;
        ConfigOptionMap::const_iterator end = mGLSupport->getConfigOptions().end();
        NameValuePairList::const_iterator param;

        if ((opt = mGLSupport->getConfigOptions().find("Video Mode")) != end)
        {
            String val = opt->second.currentValue;
            String::size_type pos = val.find('x');

            if (pos != String::npos)
            {
                w = StringConverter::parseUnsignedInt(val.substr(0, pos));
                h = StringConverter::parseUnsignedInt(val.substr(pos + 1));
            }
        }

        // Set us up with an external window, or create our own.
        if(!mIsExternal)
        {
            mWindow = [[[UIWindow alloc] initWithFrame:CGRectMake(0, 0, w, h)] retain];
        }

        OgreAssert(mWindow != nil, "EAGLWindow: Failed to create native window");

        // Set up the view
        if(!mUsingExternalView)
        {
            mView = [[EAGLView alloc] initWithFrame:CGRectMake(0, 0, w, h)];
            mView.opaque = YES;

            // Use the default scale factor of the screen
            // See Apple's documentation on supporting high resolution devices for more info
            mView.contentScaleFactor = mContentScalingFactor;
        }

        OgreAssert(mView != nil, "EAGLWindow: Failed to create view");

        [mView setMWindowName:mName];

        OgreAssert([mView.layer isKindOfClass:[CAEAGLLayer class]], "EAGLWindow: View's Core Animation layer is not a CAEAGLLayer. This is a requirement for using OpenGL ES for drawing.");
        
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)mView.layer;
        OgreAssert(eaglLayer != nil, "EAGLWindow: Failed to retrieve a pointer to the view's Core Animation layer");

        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        // Set up the view controller
        if(!mUsingExternalViewController)
        {
            mViewController = [[EAGLViewController alloc] init];
        }

        OgreAssert(mViewController != nil, "EAGLWindow: Failed to create view controller");
        
        if(mViewController.view != mView)
            mViewController.view = mView;

        CFDictionaryRef dict;   // TODO: Dummy dictionary for now
        if(eaglLayer)
        {
            EAGLSharegroup *group = nil;
            NameValuePairList::const_iterator option;

            if ((option = miscParams->find("externalSharegroup")) != miscParams->end())
            {
                group = (EAGLSharegroup *)StringConverter::parseUnsignedLong(option->second);
                LogManager::getSingleton().logMessage("iOS: Using an external EAGLSharegroup");
            }

            mContext = mGLSupport->createNewContext(dict, eaglLayer, group);

            mContext->mIsMultiSampleSupported = true;
            mContext->mNumSamples = mFSAA;
        }
        
        OgreAssert(mContext != nil, "EAGLWindow: Failed to create OpenGL ES context");

        if(!mUsingExternalViewController)
            [mWindow addSubview:mViewController.view];

        mViewController.mGLSupport = mGLSupport;

        if(!mUsingExternalViewController)
            mWindow.rootViewController = mViewController;

        if(!mUsingExternalView)
            [mView release];

        if(!mUsingExternalViewController)
            [mWindow makeKeyAndVisible];

        mContext->createFramebuffer();

        // If content scaling is supported, the window size will be smaller than the GL pixel buffer
        // used to render.  Report the buffer size for reference.
        StringStream ss;
    
        ss  << "iOS: Window created " << w << " x " << h
            << " with backing store size " << mContext->mBackingWidth << " x " << mContext->mBackingHeight;
        if(mIsContentScalingSupported)
        {
            ss << " using content scaling factor " << std::fixed << std::setprecision(1) << mContentScalingFactor;
        }
        LogManager::getSingleton().logMessage(ss.str());

        [pool release];
    }
    
    void EAGLWindow::create(const String& name, uint width, uint height,
                                bool fullScreen, const NameValuePairList *miscParams)
    {
        short frequency = 0;
        bool vsync = false;
		int left = 0;
		int top  = 0;
        
        mIsFullScreen = fullScreen;
        mName = name;
        mWidth = width;
        mHeight = height;

        // Check the configuration. This may be overridden later by the value sent via miscParams
        ConfigOptionMap::const_iterator configOpt;
        ConfigOptionMap::const_iterator configEnd = mGLSupport->getConfigOptions().end();
        if ((configOpt = mGLSupport->getConfigOptions().find("Content Scaling Factor")) != configEnd)
        {
            mContentScalingFactor = StringConverter::parseReal(configOpt->second.currentValue);
        }

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            // Note: Some platforms support AA inside ordinary windows
            if ((opt = miscParams->find("FSAA")) != end)
            {
                mFSAA = StringConverter::parseUnsignedInt(opt->second);
            }
            
            if ((opt = miscParams->find("displayFrequency")) != end)
            {
                frequency = (short)StringConverter::parseInt(opt->second);
            }

            if ((opt = miscParams->find("contentScalingFactor")) != end)
            {
                mContentScalingFactor = StringConverter::parseReal(opt->second);
            }
            
            if ((opt = miscParams->find("vsync")) != end)
            {
                vsync = StringConverter::parseBool(opt->second);
            }

            if ((opt = miscParams->find("left")) != end)
            {
                left = StringConverter::parseInt(opt->second);
            }
            
            if ((opt = miscParams->find("top")) != end)
            {
                top = StringConverter::parseInt(opt->second);
            }
            
            if ((opt = miscParams->find("title")) != end)
            {
                mName = opt->second;
            }

            if ((opt = miscParams->find("externalWindowHandle")) != end)
            {
                mWindow = (UIWindow *)StringConverter::parseUnsignedLong(opt->second);
                mIsExternal = true;
                LogManager::getSingleton().logMessage("iOS: Using an external window handle");
            }

            if ((opt = miscParams->find("externalViewHandle")) != end)
            {
                mView = (EAGLView *)StringConverter::parseUnsignedLong(opt->second);
                CGRect b = [mView bounds];
                mWidth = b.size.width;
                mHeight = b.size.height;
                mUsingExternalView = true;
                LogManager::getSingleton().logMessage("iOS: Using an external view handle");
            }

            if ((opt = miscParams->find("externalViewControllerHandle")) != end)
            {
                mViewController = (EAGLViewController *)StringConverter::parseUnsignedLong(opt->second);
                if(mViewController.view != nil)
                    mView = (EAGLView *)mViewController.view;
                mUsingExternalViewController = true;
                LogManager::getSingleton().logMessage("iOS: Using an external view controller handle");
            }
		}

        initNativeCreatedWindow(miscParams);

        left = top = 0;
		mLeft = left;
		mTop = top;

        // Resize, taking content scaling factor into account
        resize(mWidth, mHeight);

		mActive = true;
		mVisible = true;
		mClosed = false;
    }

    void EAGLWindow::swapBuffers()
    {
        if (mClosed)
        {
            return;
        }

        unsigned int attachmentCount = 0;
        GLenum attachments[3];
        GLESRenderSystem *rs =
            static_cast<GLESRenderSystem*>(Root::getSingleton().getRenderSystem());
        unsigned int buffers = rs->getDiscardBuffers();
        
        if(buffers & FBT_COLOUR)
        {
            attachments[attachmentCount++] = GL_COLOR_ATTACHMENT0_OES;
        }
        if(buffers & FBT_DEPTH)
        {
            attachments[attachmentCount++] = GL_DEPTH_ATTACHMENT_OES;
        }
        if(buffers & FBT_STENCIL)
        {
            attachments[attachmentCount++] = GL_STENCIL_ATTACHMENT_OES;
        }
        
        if(mContext->mIsMultiSampleSupported && mContext->mNumSamples > 0)
        {
            glDisable(GL_SCISSOR_TEST);     
            glBindFramebufferOES(GL_READ_FRAMEBUFFER_APPLE, mContext->mFSAAFramebuffer);
            GL_CHECK_ERROR
            glBindFramebufferOES(GL_DRAW_FRAMEBUFFER_APPLE, mContext->mViewFramebuffer);
            GL_CHECK_ERROR
            glResolveMultisampleFramebufferAPPLE();
            GL_CHECK_ERROR
            glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, attachmentCount, attachments);
            GL_CHECK_ERROR
            
            glBindFramebufferOES(GL_FRAMEBUFFER_OES, mContext->mViewFramebuffer);
            GL_CHECK_ERROR
        }
        else
        {
            glBindFramebufferOES(GL_FRAMEBUFFER_OES, mContext->mViewFramebuffer);
            GL_CHECK_ERROR
            glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, attachmentCount, attachments);
            GL_CHECK_ERROR
        }
        
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, mContext->mViewRenderbuffer);
        GL_CHECK_ERROR
        if ([mContext->getContext() presentRenderbuffer:GL_RENDERBUFFER_OES] == NO)
        {
            GL_CHECK_ERROR
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to swap buffers in ",
                        __FUNCTION__);
        }
    }

    void EAGLWindow::getCustomAttribute( const String& name, void* pData )
    {
		if( name == "GLCONTEXT" )
		{
			*static_cast<EAGLESContext**>(pData) = mContext;
			return;
		}

        if( name == "SHAREGROUP" )
		{
            *(void**)(pData) = mContext->getContext().sharegroup;
            return;
		}

		if( name == "WINDOW" )
		{
			*(void**)(pData) = mWindow;
			return;
		}
        
		if( name == "VIEW" )
		{
			*(void**)(pData) = mViewController.view;
			return;
		}

        if( name == "VIEWCONTROLLER" )
		{
            *(void**)(pData) = mViewController;
            return;
		}
	}

    void EAGLWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
        if(dst.format != PF_A8R8G8B8)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Only PF_A8R8G8B8 is a supported format for OpenGL ES", __FUNCTION__);

        if ((dst.right > mWidth) ||
			(dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Invalid box.",
                        __FUNCTION__ );
		}

		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen ? FB_FRONT : FB_BACK;
		}

		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

        // The following code is adapted from Apple Technical Q & A QA1704
        // http://developer.apple.com/library/ios/#qa/qa1704/_index.html
        NSInteger width = dst.getWidth(), height = dst.getHeight();
        NSInteger dataLength = width * height * 4;
        GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));

        // Read pixel data from the framebuffer
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        GL_CHECK_ERROR
        glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        GL_CHECK_ERROR

        // Create a CGImage with the pixel data
        // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
        // otherwise, use kCGImageAlphaPremultipliedLast
        CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGImageRef iref = CGImageCreate(width, height, 8, 32, width * 4, colorspace,
                                        kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                        ref, NULL, true, kCGRenderingIntentDefault);

        // OpenGL ES measures data in PIXELS
        // Create a graphics context with the target size measured in POINTS
        NSInteger widthInPoints = 0, heightInPoints = 0;

        // Set the scale parameter to your OpenGL ES view's contentScaleFactor
        // so that you get a high-resolution snapshot when its value is greater than 1.0
        CGFloat scale = mView.contentScaleFactor;
        widthInPoints = width / scale;
        heightInPoints = height / scale;
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(widthInPoints, heightInPoints), NO, scale);

        CGContextRef context = UIGraphicsGetCurrentContext();
        CGContextDrawImage(context, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);

        // Retrieve the UIImage from the current context
        size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
        memcpy(dst.data, CGBitmapContextGetData(context), rowSpan * dst.getHeight()); // TODO: support dst.rowPitch != dst.getWidth() case
        UIGraphicsEndImageContext();

        // Clean up
        free(data);
        CFRelease(ref);
        CFRelease(colorspace);
        CGImageRelease(iref);
    }
}
