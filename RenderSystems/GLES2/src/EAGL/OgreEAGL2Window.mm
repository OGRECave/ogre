/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreEAGL2Window.h"

#include "OgreEAGL2Support.h"
#include "OgreEAGLES2Context.h"

#include "OgreRoot.h"
#include "OgreWindowEventUtilities.h"
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2PixelFormat.h"

#import <UIKit/UIWindow.h>
#import <UIKit/UIGraphics.h>

namespace Ogre {
    EAGL2Window::EAGL2Window(EAGL2Support *glsupport)
        :   mClosed(false),
            mVisible(false),
            mIsExternal(false),
            mUsingExternalView(false),
            mUsingExternalViewController(false),
            mIsContentScalingSupported(false),
            mContentScalingFactor(1.0),
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
        if(mGLSupport->getCurrentOSVersion() >= 4.0)
            mIsContentScalingSupported = true;
    }

    EAGL2Window::~EAGL2Window()
    {
        destroy();

        if (mContext != NULL)
        {
            OGRE_DELETE mContext;
        }

        mContext = NULL;
    }

    void EAGL2Window::destroy(void)
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

        if(!mUsingExternalViewController)
            [mViewController release];
    }

    void EAGL2Window::setFullscreen(bool fullscreen, uint width, uint height)
    {
    }

    void EAGL2Window::reposition(int left, int top)
	{
	}
    
	void EAGL2Window::resize(unsigned int width, unsigned int height)
	{
        if(!mWindow) return;

        Real w = mContentScalingFactor, h = mContentScalingFactor;

        // Check the orientation of the view controller and adjust dimensions
        if (UIInterfaceOrientationIsPortrait(mViewController.interfaceOrientation))
        {
            h *= std::max(width, height);
            w *= std::min(width, height);
        }
        else
        {
            w *= std::max(width, height);
            h *= std::min(width, height);
        }

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
    
	void EAGL2Window::windowMovedOrResized()
	{
		CGRect frame = [mView frame];
		mWidth = (unsigned int)frame.size.width;
		mHeight = (unsigned int)frame.size.height;
        mLeft = (int)frame.origin.x;
        mTop = (int)frame.origin.y+(int)frame.size.height;

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
	}

    void EAGL2Window::_beginUpdate(void)
    {
        // Call the base class method first
        RenderTarget::_beginUpdate();
        
        if(mContext->mIsMultiSampleSupported && mContext->mNumSamples > 0)
        {
            // Bind the FSAA buffer if we're doing multisampling
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mContext->mFSAAFramebuffer));
        }
    }

    void EAGL2Window::initNativeCreatedWindow(const NameValuePairList *miscParams)
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
        
        OgreAssert(mWindow != nil, "EAGL2Window: Failed to create native window");
        
        // Set up the view
        if(!mUsingExternalView)
        {
            mView = [[EAGL2View alloc] initWithFrame:CGRectMake(0, 0, w, h)];
            mView.opaque = YES;

            // Use the default scale factor of the screen
            // See Apple's documentation on supporting high resolution devices for more info
            mView.contentScaleFactor = mContentScalingFactor;
        }
    
        OgreAssert(mView != nil, "EAGL2Window: Failed to create view");
        
        [mView setMWindowName:mName];

        OgreAssert([mView.layer isKindOfClass:[CAEAGLLayer class]], "EAGL2Window: View's Core Animation layer is not a CAEAGLLayer. This is a requirement for using OpenGL ES for drawing.");
        
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)mView.layer;
        OgreAssert(eaglLayer != nil, "EAGL2Window: Failed to retrieve a pointer to the view's Core Animation layer");

        BOOL retainedBacking = NO;
        NameValuePairList::const_iterator option;
        if ((option = miscParams->find("retainedBacking")) != miscParams->end())
        {
            retainedBacking = StringConverter::parseBool(option->second);
        }

        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:retainedBacking], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        // Set up the view controller
        if(!mUsingExternalViewController)
        {
            mViewController = [[EAGL2ViewController alloc] init];
        }
        
        OgreAssert(mViewController != nil, "EAGL2Window: Failed to create view controller");
        
        if(mViewController.view != mView)
            mViewController.view = mView;

        CFDictionaryRef dict;   // TODO: Dummy dictionary for now
        if(eaglLayer)
        {
            EAGLSharegroup *group = nil;
            
            if ((option = miscParams->find("externalSharegroup")) != miscParams->end())
            {
                group = (EAGLSharegroup *)StringConverter::parseUnsignedLong(option->second);
                LogManager::getSingleton().logMessage("iOS: Using an external EAGLSharegroup");
            }
            
            mContext = mGLSupport->createNewContext(dict, eaglLayer, group);

            mContext->mIsMultiSampleSupported = true;
            mContext->mNumSamples = mFSAA;
        }
        
        OgreAssert(mContext != nil, "EAGL2Window: Failed to create OpenGL ES context");

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
    
    void EAGL2Window::create(const String& name, uint width, uint height,
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
                mView = (EAGL2View *)StringConverter::parseUnsignedLong(opt->second);
                CGRect b = [mView bounds];
                mWidth = b.size.width;
                mHeight = b.size.height;
                mUsingExternalView = true;
                LogManager::getSingleton().logMessage("iOS: Using an external view handle");
            }
        
            if ((opt = miscParams->find("externalViewControllerHandle")) != end)
            {
                mViewController = (EAGL2ViewController *)StringConverter::parseUnsignedLong(opt->second);
                if(mViewController.view != nil)
                    mView = (EAGL2View *)mViewController.view;
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

    void EAGL2Window::swapBuffers()
    {
        if (mClosed)
        {
            return;
        }

        unsigned int attachmentCount = 0;
        GLenum attachments[3];
        GLES2RenderSystem *rs =
            static_cast<GLES2RenderSystem*>(Root::getSingleton().getRenderSystem());
        unsigned int buffers = rs->getDiscardBuffers();
        
        if(buffers & FBT_COLOUR)
        {
            attachments[attachmentCount++] = GL_COLOR_ATTACHMENT0;
        }
        if(buffers & FBT_DEPTH)
        {
            attachments[attachmentCount++] = GL_DEPTH_ATTACHMENT;
        }
        if(buffers & FBT_STENCIL)
        {
            attachments[attachmentCount++] = GL_STENCIL_ATTACHMENT;
        }
        if(mContext->mIsMultiSampleSupported && mContext->mNumSamples > 0)
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_SCISSOR_TEST));
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, mContext->mFSAAFramebuffer));
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, mContext->mViewFramebuffer));
            OGRE_CHECK_GL_ERROR(glResolveMultisampleFramebufferAPPLE());
#if OGRE_NO_GLES3_SUPPORT == 0
            OGRE_CHECK_GL_ERROR(glInvalidateFramebuffer(GL_READ_FRAMEBUFFER_APPLE, attachmentCount, attachments));
#else
            OGRE_CHECK_GL_ERROR(glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, attachmentCount, attachments));
#endif
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mContext->mViewFramebuffer));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, mContext->mViewFramebuffer));
#if OGRE_NO_GLES3_SUPPORT == 0
            OGRE_CHECK_GL_ERROR(glInvalidateFramebuffer(GL_FRAMEBUFFER, attachmentCount, attachments));
#else
            OGRE_CHECK_GL_ERROR(glDiscardFramebufferEXT(GL_FRAMEBUFFER, attachmentCount, attachments));
#endif
        }
        
        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mContext->mViewRenderbuffer));
        if ([mContext->getContext() presentRenderbuffer:GL_RENDERBUFFER] == NO)
        {
            glGetError();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Failed to swap buffers in ",
                        __FUNCTION__);
        }
    }

    void EAGL2Window::getCustomAttribute( const String& name, void* pData )
    {
		if( name == "GLCONTEXT" )
		{
			*static_cast<EAGLES2Context**>(pData) = mContext;
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

    void EAGL2Window::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
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

        OGRE_CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, mContext->mViewRenderbuffer));

        // The following code is adapted from Apple Technical Q & A QA1704
        // http://developer.apple.com/library/ios/#qa/qa1704/_index.html
        NSInteger width = dst.getWidth(), height = dst.getHeight();
        NSInteger dataLength = width * height * PixelUtil::getComponentCount(dst.format);
        GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));
        GLenum format = GLES2PixelUtil::getGLOriginFormat(dst.format);
        GLenum type = GLES2PixelUtil::getGLOriginDataType(dst.format);

        // Read pixel data from the framebuffer
        OGRE_CHECK_GL_ERROR(glReadPixels((GLint)0, (GLint)(mHeight - dst.getHeight()),
                                         (GLsizei)width, (GLsizei)height,
                                         format, type, data));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_PACK_ALIGNMENT, 4));

        // Create a CGImage with the pixel data
        // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
        // otherwise, use kCGImageAlphaPremultipliedLast
        CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGImageRef iref = CGImageCreate(width, height, 8, PixelUtil::getNumElemBits(dst.format),
                                        width * PixelUtil::getComponentCount(dst.format), colorspace,
                                        kCGBitmapByteOrderDefault,
                                        ref, NULL, YES, kCGRenderingIntentDefault);

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
        CGContextSetBlendMode(context, kCGBlendModeCopy);
        CGContextDrawImage(context, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);

        // Retrieve the UIImage from the current context
        memcpy(dst.data, CGBitmapContextGetData(context), CGBitmapContextGetBytesPerRow(context) * height); // TODO: support dst.rowPitch != dst.getWidth() case
        UIGraphicsEndImageContext();

        // Clean up
        free(data);
        CFRelease(ref);
        CFRelease(colorspace);
        CGImageRelease(iref);
    }
}
