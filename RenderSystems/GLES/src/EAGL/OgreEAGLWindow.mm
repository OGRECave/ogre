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
--------------------------------------------------------------------------*/

#include "OgreEAGLWindow.h"

#include "OgreRoot.h"
#include "OgreWindowEventUtilities.h"

#include "OgreGLESRenderSystem.h"
#include "OgreGLESPixelFormat.h"

@implementation EAGLView

- (id)initWithFrame:(CGRect)frame
{
	if((self = [super initWithFrame:frame]))
	{
	}
	return self;
}

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

@end

// Constant to limit framerate to 60 FPS
#define kSwapInterval 1.0f / 60.0f

namespace Ogre {
    EAGLWindow::EAGLWindow(EAGLSupport *glsupport)
        :   mClosed(false),
            mVisible(false),
            mIsTopLevel(true),
            mIsExternalGLControl(false),
            mCurrentOrientation(Viewport::OR_LANDSCAPELEFT),
            mGLSupport(glsupport)
    {
        mIsFullScreen = true;
        mActive = true;
        mWindow = nil;
        mContext = NULL;
        mAnimationTimer = new Ogre::Timer();
    }

    EAGLWindow::~EAGLWindow()
    {
        destroy();

        if (mContext == NULL)
        {
            delete mContext;
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
        
        delete mAnimationTimer;

        if (!mIsExternal)
        {
            WindowEventUtilities::_removeRenderWindow(this);
        }

        if (mIsFullScreen)
        {
            mGLSupport->switchMode();
            switchFullScreen(false);
        }
        
        [mWindow release];
    }

    void EAGLWindow::setFullscreen(bool fullscreen, uint width, uint height)
    {
#pragma unused(fullscreen, width, height)
    }

    void EAGLWindow::reposition(int left, int top)
	{
#pragma unused(left, top)
	}
    
	void EAGLWindow::resize(unsigned int width, unsigned int height)
	{
        if(!mWindow) return;

		CGRect frame = [mWindow frame];
		frame.size.width = width;
		frame.size.height = height;
		[mWindow setFrame:frame];
	}
    
	void EAGLWindow::windowMovedOrResized()
	{
		CGRect frame = [mView frame];
		mWidth = (unsigned int)frame.size.width;
		mHeight = (unsigned int)frame.size.height;
        mLeft = (int)frame.origin.x;
        mTop = (int)frame.origin.y+(unsigned int)frame.size.height;

        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
	}

	void EAGLWindow::switchFullScreen( bool fullscreen )
	{
#pragma unused(fullscreen)
	}
    
    void EAGLWindow::initNativeCreatedWindow(const NameValuePairList *miscParams)
    {
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        CAEAGLLayer *eaglLayer = nil;
        
        mWindow = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] retain];
        if(mWindow == nil)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Failed to create native window",
                        __FUNCTION__);
        }
        
        mView = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        if(mView == nil)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Failed to create view",
                        __FUNCTION__);
        }
        
        mView.opaque = YES;

        eaglLayer = (CAEAGLLayer *)mView.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                            [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                            kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        CFDictionaryRef dict;   // TODO: Dummy dictionary for now
        if(eaglLayer)
            mContext = mGLSupport->createNewContext(dict, eaglLayer);
        
        if(mContext == nil)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Fail to create OpenGL ES context",
                        __FUNCTION__);
        }

        [mWindow addSubview:mView];
        [mWindow makeKeyAndVisible];
        mContext->createFramebuffer();

        [pool release];
    }
    
    void EAGLWindow::create(const String& name, uint width, uint height,
                                bool fullScreen, const NameValuePairList *miscParams)
    {
        String title = name;
        String orientation = "Landscape Left";
        uint samples = 0;
        int gamma;
        short frequency = 0;
        bool vsync = false;
		int left = 0;
		int top  = 0;
        
        mIsFullScreen = fullScreen;

        if (miscParams)
        {
            NameValuePairList::const_iterator opt;
            NameValuePairList::const_iterator end = miscParams->end();

            // Note: Some platforms support AA inside ordinary windows
            if ((opt = miscParams->find("FSAA")) != end)
            {
                samples = StringConverter::parseUnsignedInt(opt->second);
            }
            
            if ((opt = miscParams->find("displayFrequency")) != end)
            {
                frequency = (short)StringConverter::parseInt(opt->second);
            }
            
            if ((opt = miscParams->find("vsync")) != end)
            {
                vsync = StringConverter::parseBool(opt->second);
            }
            
            if ((opt = miscParams->find("gamma")) != end)
            {
                gamma = StringConverter::parseBool(opt->second);
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
                title = opt->second;
            }

            if ((opt = miscParams->find("orientation")) != end)
            {
                orientation = opt->second;
            }
            
            if ((opt = miscParams->find("externalGLControl")) != end)
            {
                mIsExternalGLControl = StringConverter::parseBool(opt->second);
            }
		}
        
        initNativeCreatedWindow(miscParams);

        if(orientation == "Landscape Left")
        {
            top = 320;
            left = 0;
            width = 320;
            height = 480;
            mCurrentOrientation = Viewport::OR_LANDSCAPELEFT;
        }
        else if(orientation == "Landscape Right")
        {
            left = 480;
            width = 320;
            height = 480;
            mCurrentOrientation = Viewport::OR_LANDSCAPERIGHT;
        }
        else
        {
            left = top = 0;
            mCurrentOrientation = Viewport::OR_PORTRAIT;
        };
        mIsExternal = false;    // Cannot use external displays on iPhone

        mHwGamma = false;
        
        if (!mIsTopLevel)
        {
            mIsFullScreen = false;
            left = top = 0;
        }

		mName = name;
		mWidth = width;
		mHeight = height;
		mLeft = left;
		mTop = top;
		mActive = true;
		mVisible = true;
		mClosed = false;
    }
    
    bool EAGLWindow::isClosed() const
    {
        return mClosed;
    }

    bool EAGLWindow::isVisible() const
    {
        return mVisible;
    }

    void EAGLWindow::setVisible(bool visible)
    {
        mVisible = visible;
    }

    void EAGLWindow::setClosed(bool closed)
    {
        mClosed = closed;
    }

    void EAGLWindow::swapBuffers(bool waitForVSync)
    {
        if (mClosed || mIsExternalGLControl)
        {
            return;
        }
        
        if (mAnimationTimer->getMilliseconds() < kSwapInterval)
        {
            return;
        }

        mAnimationTimer->reset();

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

    void EAGLWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
    {
        if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Invalid box.",
				__FUNCTION__ );
		}

		if (buffer == FB_AUTO)
		{
			buffer = mIsFullScreen? FB_FRONT : FB_BACK;
		}

		GLenum format = GLESPixelUtil::getGLOriginFormat(dst.format);
		GLenum type = GLESPixelUtil::getGLOriginDataType(dst.format);

		if ((format == 0) || (type == 0))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Unsupported format.",
				__FUNCTION__ );
		}

		// Switch context if different from current one
		RenderSystem* rsys = Root::getSingleton().getRenderSystem();
		rsys->_setViewport(this->getViewport(0));

        if((dst.getWidth() * Ogre::PixelUtil::getNumElemBytes(dst.format)) & 3)
        {
            // Standard alignment of 4 is not right
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
        }

		glReadPixels((GLint)dst.left, (GLint)dst.top,
			(GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
			format, type, dst.data);

		// Restore default alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		// Vertical flip
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

	bool EAGLWindow::requiresTextureFlipping() const
	{
        return false;
	}
}
