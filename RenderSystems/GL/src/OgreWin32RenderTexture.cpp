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

#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreWin32RenderTexture.h"
#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"

namespace Ogre {

	 Win32PBuffer::Win32PBuffer(PixelComponentType format, size_t width, size_t height):
		GLPBuffer(format, width, height),
        mContext(0)
	{
		createPBuffer();

        // Create context
        mContext = new Win32Context(mHDC, mGlrc);
#if 0
		if(mUseBind)
		{
			// Bind texture
			glBindTextureEXT(GL_TEXTURE_2D, static_cast<GLTexture*>(mTexture.get())->getGLID());
			wglBindTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);
		}
#endif
	}
	 Win32PBuffer::~Win32PBuffer() 
	{
#if 0
		if(mUseBind)
		{
			// Unbind texture
			glBindTextureEXT(GL_TEXTURE_2D,
				static_cast<GLTexture*>(mTexture.get())->getGLID());
			glBindTextureEXT(GL_TEXTURE_2D,
				static_cast<GLTexture*>(mTexture.get())->getGLID());
			wglReleaseTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);
		}
#endif
        // Unregister and destroy mContext
        delete mContext;        
           
		// Destroy pbuffer
		destroyPBuffer();
	}
	
	void Win32PBuffer::createPBuffer() 
	{

        // Process format
        int bits=0;
        bool isFloat=false;
#if 0
		bool hasAlpha=true;
#endif
        switch(mFormat)
        {
            case PCT_BYTE:
                bits=8; isFloat=false;
                break;
            case PCT_SHORT:
                bits=16; isFloat=false;
                break;
            case PCT_FLOAT16:
                bits=16; isFloat=true;
                break;
            case PCT_FLOAT32:
                bits=32; isFloat=true;
                break;
            default: break;
        };
		LogManager::getSingleton().logMessage(
			" Win32PBuffer::Creating PBuffer of format bits="+
			StringConverter::toString(bits)+
			" float="+StringConverter::toString(isFloat)
	    );


		HDC old_hdc = wglGetCurrentDC();
		HGLRC old_context = wglGetCurrentContext();

		// Bind to RGB or RGBA texture
		int bttype = 0;
#if 0
		if(mUseBind)
		{
			// Only provide bind type when actually binding
			bttype = PixelUtil::hasAlpha(mInternalFormat)?
				WGL_BIND_TO_TEXTURE_RGBA_ARB : WGL_BIND_TO_TEXTURE_RGB_ARB;
		}
		int texformat = hasAlpha?
			WGL_TEXTURE_RGBA_ARB : WGL_TEXTURE_RGB_ARB;
#endif
		// Make a float buffer?
        int pixeltype = isFloat?
			WGL_TYPE_RGBA_FLOAT_ARB: WGL_TYPE_RGBA_ARB;
		
		int attrib[] = {
			WGL_RED_BITS_ARB,bits,
			WGL_GREEN_BITS_ARB,bits,
			WGL_BLUE_BITS_ARB,bits,
			WGL_ALPHA_BITS_ARB,bits,
			WGL_STENCIL_BITS_ARB,1,
			WGL_DEPTH_BITS_ARB,15,
			WGL_DRAW_TO_PBUFFER_ARB,true,
			WGL_SUPPORT_OPENGL_ARB,true,
			WGL_PIXEL_TYPE_ARB,pixeltype,
			//WGL_DOUBLE_BUFFER_ARB,true,
			//WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB, // Make sure it is accelerated
			bttype,true, // must be last, as bttype can be zero
			0
		};
		int pattrib_default[] = { 
			0
		};
#if 0
		int pattrib_bind[] = { 
			WGL_TEXTURE_FORMAT_ARB, texformat, 
			WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
			WGL_PBUFFER_LARGEST_ARB, true,
			0 
		};
#endif
		int format;
		unsigned int count;

		// Choose suitable pixel format
		wglChoosePixelFormatARB(old_hdc,attrib,NULL,1,&format,&count);
		if(count == 0)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglChoosePixelFormatARB() failed", " Win32PBuffer::createPBuffer");

		// Analyse pixel format
		const int piAttributes[]={
				WGL_RED_BITS_ARB,WGL_GREEN_BITS_ARB,WGL_BLUE_BITS_ARB,WGL_ALPHA_BITS_ARB,
				WGL_DEPTH_BITS_ARB,WGL_STENCIL_BITS_ARB
		};
		int piValues[sizeof(piAttributes)/sizeof(const int)];
		wglGetPixelFormatAttribivARB(old_hdc,format,0,sizeof(piAttributes)/sizeof(const int),piAttributes,piValues);

        LogManager::getSingleton().stream()
			<< " Win32PBuffer::PBuffer -- Chosen pixel format rgba="
            << piValues[0] << ","  
            << piValues[1] << ","  
            << piValues[2] << ","  
            << piValues[3] 
            << " depth=" << piValues[4]
            << " stencil=" << piValues[5];

		mPBuffer = wglCreatePbufferARB(old_hdc,format,mWidth,mHeight,pattrib_default);
		if(!mPBuffer)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglCreatePbufferARB() failed", " Win32PBuffer::createPBuffer");

		mHDC = wglGetPbufferDCARB(mPBuffer);
		if(!mHDC) {
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglGetPbufferDCARB() failed", " Win32PBuffer::createPBuffer");
		}
			
		mGlrc = wglCreateContext(mHDC);
		if(!mGlrc) {
			wglReleasePbufferDCARB(mPBuffer,mHDC);
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglCreateContext() failed", " Win32PBuffer::createPBuffer");
		}

		if(!wglShareLists(old_context,mGlrc)) {
			wglDeleteContext(mGlrc);
			wglReleasePbufferDCARB(mPBuffer,mHDC);
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglShareLists() failed", " Win32PBuffer::createPBuffer");
		}
				
		// Query real width and height
		int iWidth, iHeight;
		wglQueryPbufferARB(mPBuffer, WGL_PBUFFER_WIDTH_ARB, &iWidth);
		wglQueryPbufferARB(mPBuffer, WGL_PBUFFER_HEIGHT_ARB, &iHeight);
		mWidth = iWidth;  
		mHeight = iHeight;
		LogManager::getSingleton().stream()
			<< "Win32RenderTexture::PBuffer created -- Real dimensions "
            << mWidth << "x" << mHeight;
	}
	void Win32PBuffer::destroyPBuffer() 
	{
		wglDeleteContext(mGlrc);
		wglReleasePbufferDCARB(mPBuffer,mHDC);
		wglDestroyPbufferARB(mPBuffer);
	}


}
