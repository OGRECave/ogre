#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"

#include "OgreOSXRenderTexture.h"
#include "OgreOSXCarbonContext.h"

#include <OpenGL/gl.h>
#define GL_EXT_texture_env_combine 1
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>

namespace Ogre
{
	OSXPBuffer::OSXPBuffer( PixelComponentType format, size_t width, size_t height ) : GLPBuffer( format, width, height ), mContext( NULL )
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::OSXPBuffer()" );
		createPBuffer();
		// Create context
		//mContext = new OSXCarbonContext(mAGLContext);
    }
	
	OSXPBuffer::~OSXPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::~OSXPBuffer()" );
		delete mContext;
		destroyPBuffer();
	}

	GLContext* OSXPBuffer::getContext()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::getContext()" );
		return mContext;
	}
	
	void OSXPBuffer::createPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::createPBuffer()" );
		
		GLint attrib[] = { AGL_NO_RECOVERY, GL_TRUE, AGL_ACCELERATED, GL_TRUE, AGL_RGBA, AGL_NONE };
		AGLPixelFormat pixelFormat = aglChoosePixelFormat(NULL, 0, attrib);
		mAGLContext = aglCreateContext(pixelFormat, NULL);
		
		//mAGLContext = aglGetCurrentContext();
		aglCreatePBuffer( mWidth, mHeight, GL_TEXTURE_2D, GL_RGBA, 0, &mPBuffer );
		
		GLint vs = aglGetVirtualScreen( mAGLContext );
		aglSetPBuffer( mAGLContext, mPBuffer, 0, 0, vs );
        mContext = new OSXCarbonContext(mAGLContext, pixelFormat);
	}
	
	void OSXPBuffer::destroyPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::destroyPBuffer()" );
		aglDestroyPBuffer( mPBuffer );
	}
}

