/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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
#define NOMINMAX
#include "OgreGLESRenderSystem.h"
#include "OgreGLESTextureManager.h"
#include "OgreGLESDefaultHardwareBufferManager.h"
#include "OgreGLESDepthBuffer.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESHardwareBufferManager.h"
#include "OgreGLESHardwareIndexBuffer.h"
#include "OgreGLESHardwareVertexBuffer.h"
#include "OgreGLESGpuProgramManager.h"
#include "OgreGLESUtil.h"
#include "OgreGLESPBRenderTexture.h"
#include "OgreGLESFBORenderTexture.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#   include "OgreEAGLWindow.h"
#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
#	include "OgreAndroidEGLWindow.h"
#	include "OgreAndroidEGLContext.h"
#   include "OgreAndroidResourceManager.h"
Ogre::AndroidResourceManager* Ogre::GLESRenderSystem::mResourceManager = NULL;
#else
#   include "OgreEGLWindow.h"

#	ifndef GL_GLEXT_PROTOTYPES
    // Function pointers for FBO extension
    PFNGLISRENDERBUFFEROESPROC glIsRenderbufferOES;
    PFNGLBINDRENDERBUFFEROESPROC glBindRenderbufferOES;
    PFNGLDELETERENDERBUFFERSOESPROC glDeleteRenderbuffersOES;
    PFNGLGENRENDERBUFFERSOESPROC glGenRenderbuffersOES;
    PFNGLRENDERBUFFERSTORAGEOESPROC glRenderbufferStorageOES;
    PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glGetRenderbufferParameterivOES;
    PFNGLISFRAMEBUFFEROESPROC glIsFramebufferOES;
    PFNGLBINDFRAMEBUFFEROESPROC glBindFramebufferOES;
    PFNGLDELETEFRAMEBUFFERSOESPROC glDeleteFramebuffersOES;
    PFNGLGENFRAMEBUFFERSOESPROC glGenFramebuffersOES;
    PFNGLCHECKFRAMEBUFFERSTATUSOESPROC glCheckFramebufferStatusOES;
    PFNGLFRAMEBUFFERRENDERBUFFEROESPROC glFramebufferRenderbufferOES;
    PFNGLFRAMEBUFFERTEXTURE2DOESPROC glFramebufferTexture2DOES;
    PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC glGetFramebufferAttachmentParameterivOES;
    PFNGLGENERATEMIPMAPOESPROC glGenerateMipmapOES;
	PFNGLBLENDEQUATIONOESPROC glBlendEquationOES;
	PFNGLBLENDFUNCSEPARATEOESPROC glBlendFuncSeparateOES;
	PFNGLBLENDEQUATIONSEPARATEOESPROC glBlendEquationSeparateOES;
    PFNGLMAPBUFFEROESPROC glMapBufferOES;
    PFNGLUNMAPBUFFEROESPROC glUnmapBufferOES;
#	endif

#endif

#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreGLESStateCacheManager.h"

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace Ogre {
    GLESRenderSystem::GLESRenderSystem()
        : mGpuProgramManager(0),
          mHardwareBufferManager(0),
          mRTTManager(0),
		  mCurTexMipCount(0)
    {
            // Get function pointers on platforms that doesn't have prototypes
#ifndef GL_GLEXT_PROTOTYPES
            ::glIsRenderbufferOES = (PFNGLISRENDERBUFFEROESPROC)eglGetProcAddress("glIsRenderbufferOES");
            ::glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEROESPROC)eglGetProcAddress("glBindRenderbufferOES");
            ::glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSOESPROC)eglGetProcAddress("glDeleteRenderbuffersOES");
            ::glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSOESPROC)eglGetProcAddress("glGenRenderbuffersOES");
            ::glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEOESPROC)eglGetProcAddress("glRenderbufferStorageOES");
            ::glGetRenderbufferParameterivOES = (PFNGLGETRENDERBUFFERPARAMETERIVOESPROC)eglGetProcAddress("glGetRenderbufferParameterivOES");
            ::glIsFramebufferOES = (PFNGLISFRAMEBUFFEROESPROC)eglGetProcAddress("glIsFramebufferOES");
            ::glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEROESPROC)eglGetProcAddress("glBindFramebufferOES");
            ::glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSOESPROC)eglGetProcAddress("glDeleteFramebuffersOES");
            ::glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSOESPROC)eglGetProcAddress("glGenFramebuffersOES");
            ::glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSOESPROC)eglGetProcAddress("glCheckFramebufferStatusOES");
            ::glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEROESPROC)eglGetProcAddress("glFramebufferRenderbufferOES");
            ::glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DOESPROC)eglGetProcAddress("glFramebufferTexture2DOES");
            ::glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC)eglGetProcAddress("glGetFramebufferAttachmentParameterivOES");
            ::glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOESPROC)eglGetProcAddress("glGenerateMipmapOES");
			::glBlendEquationOES = (PFNGLBLENDEQUATIONOESPROC)eglGetProcAddress("glBlendEquationOES");
			::glBlendFuncSeparateOES = (PFNGLBLENDFUNCSEPARATEOESPROC)eglGetProcAddress("glBlendFuncSeparateOES");
			::glBlendEquationSeparateOES = (PFNGLBLENDEQUATIONSEPARATEOESPROC)eglGetProcAddress("glBlendEquationSeparateOES");
            ::glMapBufferOES = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");
            ::glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress("glUnmapBufferOES");
#endif
        GL_CHECK_ERROR;
		LogManager::getSingleton().logMessage(getName() + " created.");

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        mResourceManager = OGRE_NEW AndroidResourceManager();
#endif
      
        mStateCacheManager = new GLESStateCacheManager();
        mGLSupport = getGLSupport();
        mGLSupport->setStateCacheManager(mStateCacheManager);
        
        for (size_t i = 0; i < MAX_LIGHTS; i++)
        {
            mLights[i] = NULL;
        }

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;
        mGLSupport->addConfig();

        for (size_t i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            mTextureCoordIndex[i] = 99; // Dummy value
        }

        mTextureCount = 0;
        mActiveRenderTarget = 0;
        mCurrentContext = 0;
        mMainContext = 0;
        mGLInitialised = false;
        mCurrentLights = 0;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
    }

    GLESRenderSystem::~GLESRenderSystem()
    {
        shutdown();

		// Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            OGRE_DELETE i->second;
        }

        mRenderTargets.clear();
        OGRE_DELETE mGLSupport;
        OGRE_DELETE mStateCacheManager;
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        if (mResourceManager != NULL)
		{
			OGRE_DELETE mResourceManager;
			mResourceManager = NULL;
		}
#endif
    }

    const String& GLESRenderSystem::getName(void) const
    {
        static String strName("OpenGL ES 1.x Rendering Subsystem");
        return strName;
    }

    ConfigOptionMap& GLESRenderSystem::getConfigOptions(void)
    {
        return mGLSupport->getConfigOptions();
    }

    void GLESRenderSystem::setConfigOption(const String &name, const String &value)
    {
        mGLSupport->setConfigOption(name, value);
    }

    String GLESRenderSystem::validateConfigOptions(void)
    {
		// XXX Return an error string if something is invalid
        return mGLSupport->validateConfig();
    }

    RenderWindow* GLESRenderSystem::_initialise(bool autoCreateWindow,
                                                const String& windowTitle)
    {
		mGLSupport->start();

        // Create the texture manager        
		mTextureManager = OGRE_NEW GLESTextureManager(*mGLSupport); 

        RenderWindow *autoWindow = mGLSupport->createWindow(autoCreateWindow,
                                                            this, windowTitle);
        RenderSystem::_initialise(autoCreateWindow, windowTitle);
        return autoWindow;
    }

    RenderSystemCapabilities* GLESRenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = OGRE_NEW RenderSystemCapabilities();

        rsc->setCategoryRelevant(CAPS_CATEGORY_GL, true);
        rsc->setDriverVersion(mDriverVersion);

        const char* deviceName = (const char*)glGetString(GL_RENDERER);
		const char* vendorName = (const char*)glGetString(GL_VENDOR);        
        if (deviceName)
        {
            rsc->setDeviceName(deviceName);
        }

        rsc->setRenderSystemName(getName());

		// Determine vendor
		if (strstr(vendorName, "Imagination Technologies"))
			rsc->setVendor(GPU_IMAGINATION_TECHNOLOGIES);
		else if (strstr(vendorName, "Apple Computer, Inc."))
			rsc->setVendor(GPU_APPLE);  // iOS Simulator
		else if (strstr(vendorName, "NVIDIA"))
			rsc->setVendor(GPU_NVIDIA);
		else if (strstr(vendorName, "Nokia"))
			rsc->setVendor(GPU_NOKIA);
        else
            rsc->setVendor(GPU_UNKNOWN);

        // GL ES 1.x is fixed function only
        rsc->setCapability(RSC_FIXED_FUNCTION);

        // Multitexturing support and set number of texture units
        GLint units;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &units);
        rsc->setNumTextureUnits(std::min<ushort>(16, units));

        // Check for hardware stencil support and set bit depth
        GLint stencil;
        glGetIntegerv(GL_STENCIL_BITS, &stencil);
        GL_CHECK_ERROR;

        if(stencil)
        {
            rsc->setCapability(RSC_HWSTENCIL);
            rsc->setStencilBufferBitDepth(stencil);
        }

        // Scissor test is standard
        rsc->setCapability(RSC_SCISSOR_TEST);

        // Vertex Buffer Objects are always supported by OpenGL ES
        rsc->setCapability(RSC_VBO);
        if(mGLSupport->checkExtension("GL_OES_element_index_uint"))
            rsc->setCapability(RSC_32BIT_INDEX);

        // OpenGL ES - Check for these extensions too
        // For 1.1, http://www.khronos.org/registry/gles/api/1.1/glext.h

        if (mGLSupport->checkExtension("GL_IMG_texture_compression_pvrtc") ||
            mGLSupport->checkExtension("GL_EXT_texture_compression_dxt1") ||
            mGLSupport->checkExtension("GL_EXT_texture_compression_s3tc") ||
            mGLSupport->checkExtension("GL_OES_compressed_ETC1_RGB8_texture") ||
            mGLSupport->checkExtension("GL_AMD_compressed_ATC_texture") ||
            mGLSupport->checkExtension("GL_OES_compressed_paletted_texture"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION);
			
            if(mGLSupport->checkExtension("GL_IMG_texture_compression_pvrtc") ||
               mGLSupport->checkExtension("GL_IMG_texture_compression_pvrtc2"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_PVRTC);
				
            if(mGLSupport->checkExtension("GL_EXT_texture_compression_dxt1") && 
               mGLSupport->checkExtension("GL_EXT_texture_compression_s3tc"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

            if(mGLSupport->checkExtension("GL_OES_compressed_ETC1_RGB8_texture"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ETC1);
				
			if(mGLSupport->checkExtension("GL_AMD_compressed_ATC_texture"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ATC);
        }

        if (mGLSupport->checkExtension("GL_EXT_texture_filter_anisotropic"))
            rsc->setCapability(RSC_ANISOTROPY);

        // FIXME: DJR - causes GL errors on 3GS
//        if (mGLSupport->checkExtension("GL_APPLE_texture_2D_limited_npot"))
//            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);

        if (mGLSupport->checkExtension("GL_OES_framebuffer_object")) {
            rsc->setCapability(RSC_FBO);
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        } else {
            rsc->setCapability(RSC_PBUFFER);
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        }

        // Cube map
        if (mGLSupport->checkExtension("GL_OES_texture_cube_map"))
            rsc->setCapability(RSC_CUBEMAPPING);

        if (mGLSupport->checkExtension("GL_OES_stencil_wrap"))
            rsc->setCapability(RSC_STENCIL_WRAP);

        if (mGLSupport->checkExtension("GL_OES_blend_subtract"))
            rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);

//        if (mGLSupport->checkExtension("GL_IMG_user_clip_plane"))
            rsc->setCapability(RSC_USER_CLIP_PLANES);

		rsc->setCapability(RSC_TEXTURE_1D);			

        if (mGLSupport->checkExtension("GL_OES_texture3D"))
            rsc->setCapability(RSC_TEXTURE_3D);

        // GL always shares vertex and fragment texture units (for now?)
        rsc->setVertexTextureUnitsShared(true);

        // Hardware support mipmapping
        rsc->setCapability(RSC_AUTOMIPMAP);

        if (mGLSupport->checkExtension("GL_EXT_texture_lod_bias"))
            rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // Blending support
        rsc->setCapability(RSC_BLENDING);

        // DOT3 support is standard
        rsc->setCapability(RSC_DOT3);
        
        // Point size
        float ps = 0;
        glGetFloatv(GL_POINT_SIZE_MAX, &ps);
        GL_CHECK_ERROR;
        rsc->setMaxPointSize(ps);

        // Point sprites
        if (mGLSupport->checkExtension("GL_OES_point_sprite"))
            rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);

        // UBYTE4 always supported
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

        // Infinite far plane always supported
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);

        // Alpha to coverage always 'supported' when MSAA is available
        // although card may ignore it if it doesn't specifically support A2C
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
        
        return rsc;
    }

    void GLESRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if(caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to initialize GLESRenderSystem from RenderSystemCapabilities that do not support OpenGL ES",
                        "GLESRenderSystem::initialiseFromRenderSystemCapabilities");
        }

        mGpuProgramManager = OGRE_NEW GLESGpuProgramManager();

        // Set texture the number of texture units
        mFixedFunctionTextureUnits = caps->getNumTextureUnits();

        if(caps->hasCapability(RSC_VBO))
        {
            mHardwareBufferManager = OGRE_NEW GLESHardwareBufferManager;
        }
        else
        {
            mHardwareBufferManager = OGRE_NEW GLESDefaultHardwareBufferManager;
        }

        // Check for framebuffer object extension
		if(caps->hasCapability(RSC_FBO))
		{
			if(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE))
			{
				// Create FBO manager
				LogManager::getSingleton().logMessage("GL ES: Using GL_OES_framebuffer_object for rendering to textures (best)");
				mRTTManager = new GLESFBOManager();
				caps->setCapability(RSC_RTT_SEPARATE_DEPTHBUFFER);
			}
		}
		else
		{
			// Check GLSupport for PBuffer support
			if(caps->hasCapability(RSC_PBUFFER))
			{
				if(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE))
				{
					// Use PBuffers
					mRTTManager = new GLESPBRTTManager(mGLSupport, primary);
					LogManager::getSingleton().logMessage("GL ES: Using PBuffers for rendering to textures");
				}
			}
            
			// Downgrade number of simultaneous targets
			caps->setNumMultiRenderTargets(1);
		}
        
        
		Log* defaultLog = LogManager::getSingleton().getDefaultLog();
		if (defaultLog)
		{
			caps->log(defaultLog);
		}

        GL_CHECK_ERROR;
        mGLInitialised = true;
    }

    void GLESRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->_initialise(true);
    }

    void GLESRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        OGRE_DELETE mGpuProgramManager;
        mGpuProgramManager = 0;

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        delete mRTTManager;
        mRTTManager = 0;

        mGLSupport->stop();

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        mGLInitialised = 0;
    }

    void GLESRenderSystem::setAmbientLight(float r, float g, float b)
    {
        GLfloat lmodel_ambient[] = { r, g, b, 1.0 };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::setShadingType(ShadeOptions so)
    {
        switch (so)
        {
            case SO_FLAT:
                glShadeModel(GL_FLAT);
                GL_CHECK_ERROR;
                break;
            default:
                glShadeModel(GL_SMOOTH);
                GL_CHECK_ERROR;
                break;
        }
    }

    void GLESRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_LIGHTING);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_LIGHTING);
            GL_CHECK_ERROR;
        }
    }

    RenderWindow* GLESRenderSystem::_createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                                        bool fullScreen, const NameValuePairList *miscParams)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "NativeWindowType with name '" + name + "' already exists",
                        "GLESRenderSystem::_createRenderWindow");
        }

		// Log a message
        StringStream ss;
        ss << "GLESRenderSystem::_createRenderWindow \"" << name << "\", " <<
            width << "x" << height << " ";
        if (fullScreen)
            ss << "fullscreen ";
        else
            ss << "windowed ";

        if (miscParams)
        {
            ss << " miscParams: ";
            NameValuePairList::const_iterator it;
            for (it = miscParams->begin(); it != miscParams->end(); ++it)
            {
                ss << it->first << "=" << it->second << " ";
            }

            LogManager::getSingleton().logMessage(ss.str());
        }

		// Create the window
        RenderWindow* win = mGLSupport->newWindow(name, width, height, fullScreen, miscParams);
        attachRenderTarget((Ogre::RenderTarget&) *win);

        if (!mGLInitialised)
        {
            initialiseContext(win);

            StringVector tokens = StringUtil::split(mGLSupport->getGLVersion(), ".");
            if (!tokens.empty())
            {
                mDriverVersion.major = StringConverter::parseInt(tokens[0]);
                if (tokens.size() > 1)
                    mDriverVersion.minor = StringConverter::parseInt(tokens[1]);
                if (tokens.size() > 2)
                    mDriverVersion.release = StringConverter::parseInt(tokens[2]);
            }
            mDriverVersion.build = 0;
			// Initialise GL after the first window has been created
			// TODO: fire this from emulation options, and don't duplicate Real and Current capabilities
            mRealCapabilities = createRenderSystemCapabilities();

			// use real capabilities if custom capabilities are not available
            if (!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities(mCurrentCapabilities, (RenderTarget *) win);

			// Initialise the main context
            _oneTimeContextInitialization();
            if (mCurrentContext)
                mCurrentContext->setInitialized();
        }

        if( win->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH )
		{
			//Unlike D3D9, OGL doesn't allow sharing the main depth buffer, so keep them separate.
			//Only Copy does, but Copy means only one depth buffer...
			GLESDepthBuffer *depthBuffer = OGRE_NEW GLESDepthBuffer( DepthBuffer::POOL_DEFAULT, this,
															mCurrentContext, 0, 0,
															win->getWidth(), win->getHeight(),
															win->getFSAA(), 0, true );

			mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

			win->attachDepthBuffer( depthBuffer );
		}

        return win;
    }

	//---------------------------------------------------------------------
	DepthBuffer* GLESRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
	{
		GLESDepthBuffer *retVal = 0;

		// Only FBO & pbuffer support different depth buffers, so everything
		// else creates dummy (empty) containers
		// retVal = mRTTManager->_createDepthBufferFor( renderTarget );
		GLESFrameBufferObject *fbo = 0;
        renderTarget->getCustomAttribute("FBO", &fbo);

		if( fbo )
		{
			// Presence of an FBO means the manager is an FBO Manager, that's why it's safe to downcast
			// Find best depth & stencil format suited for the RT's format
			GLuint depthFormat, stencilFormat;
			static_cast<GLESFBOManager*>(mRTTManager)->getBestDepthStencil( fbo->getFormat(),
																		&depthFormat, &stencilFormat );

			GLESRenderBuffer *depthBuffer = OGRE_NEW GLESRenderBuffer( depthFormat, fbo->getWidth(),
																fbo->getHeight(), fbo->getFSAA() );

			GLESRenderBuffer *stencilBuffer = depthBuffer;
			if( 
               // not supported on AMD emulation for now...
#ifdef GL_DEPTH24_STENCIL8_OES
               depthFormat != GL_DEPTH24_STENCIL8_OES && 
#endif
               stencilFormat )
			{
				stencilBuffer = OGRE_NEW GLESRenderBuffer( stencilFormat, fbo->getWidth(),
													fbo->getHeight(), fbo->getFSAA() );
			}

			//No "custom-quality" multisample for now in GL
			retVal = OGRE_NEW GLESDepthBuffer( 0, this, mCurrentContext, depthBuffer, stencilBuffer,
										fbo->getWidth(), fbo->getHeight(), fbo->getFSAA(), 0, false );
		}

		return retVal;
	}
	//---------------------------------------------------------------------
	void GLESRenderSystem::_getDepthStencilFormatFor( GLenum internalColourFormat, GLenum *depthFormat,
													GLenum *stencilFormat )
	{
		mRTTManager->getBestDepthStencil( internalColourFormat, depthFormat, stencilFormat );
	}

    MultiRenderTarget* GLESRenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget *retval = mRTTManager->createMultiRenderTarget(name);
        attachRenderTarget(*retval);
        return retval;
    }

    void GLESRenderSystem::destroyRenderWindow(RenderWindow* pWin)
    {
		// Find it to remove from list
        RenderTargetMap::iterator i = mRenderTargets.begin();

        while (i != mRenderTargets.end())
        {
            if (i->second == pWin)
            {
                _destroyDepthBuffer(pWin);
                mRenderTargets.erase(i);
                OGRE_DELETE pWin;
                break;
            }
        }
    }

    void GLESRenderSystem::_destroyDepthBuffer(RenderWindow* pWin)
    {
        GLESContext *windowContext = 0;
        pWin->getCustomAttribute("GLCONTEXT", &windowContext);
        
        //1 Window <-> 1 Context, should be always true
        assert( windowContext );
        
        bool bFound = false;
        //Find the depth buffer from this window and remove it.
        DepthBufferMap::iterator itMap = mDepthBufferPool.begin();
        DepthBufferMap::iterator enMap = mDepthBufferPool.end();
        
        while( itMap != enMap && !bFound )
        {
            DepthBufferVec::iterator itor = itMap->second.begin();
            DepthBufferVec::iterator end  = itMap->second.end();
            
            while( itor != end )
            {
                // A DepthBuffer with no depth & stencil pointers is a dummy one,
                // look for the one that matches the same GL context
                GLESDepthBuffer *depthBuffer = static_cast<GLESDepthBuffer*>(*itor);
                GLESContext *glContext = depthBuffer->getGLContext();
                
                if( glContext == windowContext &&
                   (depthBuffer->getDepthBuffer() || depthBuffer->getStencilBuffer()) )
                {
                    bFound = true;
                    
                    delete *itor;
                    itMap->second.erase( itor );
                    break;
                }
                ++itor;
            }
            
            ++itMap;
        }
    }
    
    String GLESRenderSystem::getErrorDescription(long errorNumber) const
    {
        // TODO find a way to get error string
//        const GLubyte *errString = gluErrorString (errCode);
//        return (errString != 0) ? String((const char*) errString) : StringUtil::BLANK;

        return StringUtil::BLANK;
    }

    VertexElementType GLESRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ABGR;
    }

    void GLESRenderSystem::setNormaliseNormals(bool normalise)
    {
        if (normalise)
            mStateCacheManager->setEnabled(GL_NORMALIZE);
        else
            mStateCacheManager->setDisabled(GL_NORMALIZE);
    }

    void GLESRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glPushMatrix();
        GL_CHECK_ERROR;

        // Just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        LightList::const_iterator i, iend;
        iend = lights.end();
        unsigned short num = 0;
        for (i = lights.begin(); i != iend && num < limit; ++i, ++num)
        {
            setGLLight(num, *i);
            mLights[num] = *i;
        }
        // Disable extra lights
        for (; num < mCurrentLights; ++num)
        {
            setGLLight(num, NULL);
            mLights[num] = NULL;
        }
        mCurrentLights = std::min(limit, static_cast<unsigned short>(lights.size()));

        setLights();

        // restore previous
        glPopMatrix();
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];
        mWorldMatrix = m;
        makeGLMatrix(mat, mViewMatrix * mWorldMatrix);
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
        mViewMatrix = m;

        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix * mWorldMatrix);
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        // Also mark clip planes dirty
        if (!mClipPlanes.empty())
        {
            mClipPlanesDirty = true;
        }
    }

    void GLESRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];
        makeGLMatrix(mat, m);
        if (mActiveRenderTarget->requiresTextureFlipping())
        {
			// Invert transformed y
            mat[1] = -mat[1];
            mat[5] = -mat[5];
            mat[9] = -mat[9];
            mat[13] = -mat[13];
        }
        glMatrixMode(GL_PROJECTION);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;

        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;
    }

    void GLESRenderSystem::_setSurfaceParams(const ColourValue &ambient,
                                             const ColourValue &diffuse,
                                             const ColourValue &specular,
                                             const ColourValue &emissive,
                                             Real shininess,
                                             TrackVertexColourType tracking)
    {
        // Track vertex colour
        if(tracking != TVC_NONE)
        {
            GLenum gt = GL_DIFFUSE;
            // There are actually 15 different combinations for tracking, of which
            // GL only supports the most used 5. This means that we have to do some
            // magic to find the best match. NOTE:
            // GL_AMBIENT_AND_DIFFUSE != GL_AMBIENT | GL__DIFFUSE
            if (tracking & TVC_AMBIENT)
            {
                if (tracking & TVC_DIFFUSE)
                {
                    gt = GL_AMBIENT_AND_DIFFUSE;
                }
                else
                {
                    gt = GL_AMBIENT;
                }
            }
            else if (tracking & TVC_DIFFUSE)
            {
                gt = GL_DIFFUSE;
            }
            else if (tracking & TVC_SPECULAR)
            {
                gt = GL_SPECULAR;
            }
            else if (tracking & TVC_EMISSIVE)
            {
                gt = GL_EMISSION;
            }
            mStateCacheManager->setEnabled(gt);
            mStateCacheManager->setEnabled(GL_COLOR_MATERIAL);
        }
        else
        {
             mStateCacheManager->setDisabled(GL_COLOR_MATERIAL);
        }

        // XXX Cache previous values?
        // XXX Front or Front and Back?

        GLfloat f4val[4] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f4val);
        GL_CHECK_ERROR;
        f4val[0] = ambient.r;
        f4val[1] = ambient.g;
        f4val[2] = ambient.b;
        f4val[3] = ambient.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f4val);
        GL_CHECK_ERROR;
        f4val[0] = specular.r;
        f4val[1] = specular.g;
        f4val[2] = specular.b;
        f4val[3] = specular.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f4val);
        GL_CHECK_ERROR;
        f4val[0] = emissive.r;
        f4val[1] = emissive.g;
        f4val[2] = emissive.b;
        f4val[3] = emissive.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f4val);
        GL_CHECK_ERROR;
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setPointParameters(Real size,
                                               bool attenuationEnabled,
                                               Real constant,
                                               Real linear,
                                               Real quadratic,
                                               Real minSize,
                                               Real maxSize)
    {
        GL_CHECK_ERROR;
        if (attenuationEnabled &&
            mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
        {
            // Point size is still calculated in pixels even when attenuation is
            // enabled, which is pretty awkward, since you typically want a viewport
            // independent size if you're looking for attenuation.
            // So, scale the point size up by viewport size (this is equivalent to
            // what D3D does as standard)
            Real adjSize = size * mActiveViewport->getActualHeight();
            Real adjMinSize = minSize * mActiveViewport->getActualHeight();
            Real adjMaxSize;
            if (maxSize == 0.0f)
                adjMaxSize = mCurrentCapabilities->getMaxPointSize(); // pixels
            else
                adjMaxSize = maxSize * mActiveViewport->getActualHeight();

            glPointSize(adjSize);
            GL_CHECK_ERROR;

            // XXX: why do I need this for results to be consistent with D3D?
            // Equations are supposedly the same once you factor in vp height
            Real correction = 0.005;
            // scaling required
            float val[4] = { static_cast<float>(constant), static_cast<float>(linear * correction),
                             static_cast<float>(quadratic * correction), 1};
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
            GL_CHECK_ERROR;
            glPointParameterf(GL_POINT_SIZE_MIN, adjMinSize);
            GL_CHECK_ERROR;
            glPointParameterf(GL_POINT_SIZE_MAX, adjMaxSize);
            GL_CHECK_ERROR;
        }
        else
        {
            // no scaling required
            // GL has no disabled flag for this so just set to constant
            glPointSize(size);
            GL_CHECK_ERROR;

            if (mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
            {
                float val[4] = { 1, 0, 0, 1 };
                glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
                GL_CHECK_ERROR;
                glPointParameterf(GL_POINT_SIZE_MIN, minSize);
                GL_CHECK_ERROR;
                if (maxSize == 0.0f)
                {
                    maxSize = mCurrentCapabilities->getMaxPointSize();
                }
                glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
                GL_CHECK_ERROR;
            }
        }
    }

    void GLESRenderSystem::_setPointSpritesEnabled(bool enabled)
    {
		if (!getCapabilities()->hasCapability(RSC_POINT_SPRITES))
			return;

        GL_CHECK_ERROR;
        if (enabled)
        {
            mStateCacheManager->setEnabled(GL_POINT_SPRITE_OES);
            GL_CHECK_ERROR;
        }
        else
        {
            mStateCacheManager->setDisabled(GL_POINT_SPRITE_OES);
            GL_CHECK_ERROR;
        }

		// Set sprite texture coord generation
		// Don't offer this as an option since D3D links it to sprite enabled
		for (ushort i = 0; i < mFixedFunctionTextureUnits; ++i)
		{
			mStateCacheManager->activateGLTextureUnit(i);
			glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, 
                      enabled ? GL_TRUE : GL_FALSE);
		}
		mStateCacheManager->activateGLTextureUnit(0);
    }

    void GLESRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
		GLESTexturePtr tex = texPtr.staticCast<GLESTexture>();

        if (!mStateCacheManager->activateGLTextureUnit(stage))
			return;

        if (enabled)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			mCurTexMipCount = 0;
#endif
            if (!tex.isNull())
            {
                // Note used
                tex->touch();
                glEnable(tex->getGLESTextureTarget());
                mStateCacheManager->bindGLTexture(tex->getGLESTextureTarget(), tex->getGLID());
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				mCurTexMipCount = tex->getNumMipmaps();
#endif
            }
            else
            {
                glEnable(GL_TEXTURE_2D);
                mStateCacheManager->bindGLTexture(GL_TEXTURE_2D, static_cast<GLESTextureManager*>(mTextureManager)->getWarningTextureID());
            }
        }
        else
        {
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_2D);
            GL_CHECK_ERROR;

            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            GL_CHECK_ERROR;

            // bind zero texture
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        mStateCacheManager->activateGLTextureUnit(0);
    }

    void GLESRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
        mTextureCoordIndex[stage] = index;
    }

    void GLESRenderSystem::_setTextureCoordCalculation(size_t stage,
                                                       TexCoordCalcMethod m,
                                                       const Frustum* frustum)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

        GLfloat M[16];
        Matrix4 projectionBias;

        // Default to no extra auto texture matrix
        mUseAutoTextureMatrix = false;

        if (!mStateCacheManager->activateGLTextureUnit(stage))
            return;
        
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        bool hasCubeMapExt = mCurrentCapabilities->hasCapability(RSC_CUBEMAPPING);
#endif
        
        switch(m)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            case TEXCALC_NONE:
                if(hasCubeMapExt)
                {
                    glDisable(GL_TEXTURE_GEN_STR_OES);
                    GL_CHECK_ERROR
                }
                break;
#endif
            case TEXCALC_ENVIRONMENT_MAP:
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
                if(hasCubeMapExt)
                {
                    glEnable(GL_TEXTURE_GEN_STR_OES);
                    glTexGeniOES( GL_TEXTURE_GEN_STR_OES, GL_TEXTURE_GEN_MODE_OES, GL_REFLECTION_MAP_OES );
                    GL_CHECK_ERROR
                }
#endif
                mUseAutoTextureMatrix = true;
                memset(mAutoTextureMatrix, 0, sizeof(GLfloat)*16);
                mAutoTextureMatrix[0] = mAutoTextureMatrix[10] = mAutoTextureMatrix[15] = 1.0f;
                mAutoTextureMatrix[5] = -1.0f;
                break;

            case TEXCALC_ENVIRONMENT_MAP_PLANAR:
                break;

            case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
                if(hasCubeMapExt)
                {
                    glEnable(GL_TEXTURE_GEN_STR_OES);
                    glTexGeniOES( GL_TEXTURE_GEN_STR_OES, GL_TEXTURE_GEN_MODE_OES, GL_REFLECTION_MAP_OES );
                    GL_CHECK_ERROR
                }
#endif
                // We need an extra texture matrix here
                // This sets the texture matrix to be the inverse of the view matrix
                mUseAutoTextureMatrix = true;
                makeGLMatrix(M, mViewMatrix);

                // Transpose 3x3 in order to invert matrix (rotation)
                // Note that we need to invert the Z _before_ the rotation
                // No idea why we have to invert the Z at all, but reflection is wrong without it
                mAutoTextureMatrix[0] = M[0]; mAutoTextureMatrix[1] = M[4]; mAutoTextureMatrix[2] = -M[8];
                mAutoTextureMatrix[4] = M[1]; mAutoTextureMatrix[5] = M[5]; mAutoTextureMatrix[6] = -M[9];
                mAutoTextureMatrix[8] = M[2]; mAutoTextureMatrix[9] = M[6]; mAutoTextureMatrix[10] = -M[10];
                mAutoTextureMatrix[3] = mAutoTextureMatrix[7] = mAutoTextureMatrix[11] = 0.0f;
                mAutoTextureMatrix[12] = mAutoTextureMatrix[13] = mAutoTextureMatrix[14] = 0.0f;
                mAutoTextureMatrix[15] = 1.0f;
                break;

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
            case TEXCALC_ENVIRONMENT_MAP_NORMAL:
                if(hasCubeMapExt)
                {
                    glEnable(GL_TEXTURE_GEN_STR_OES);
                    glTexGeniOES( GL_TEXTURE_GEN_STR_OES, GL_TEXTURE_GEN_MODE_OES, GL_NORMAL_MAP_OES );
                    GL_CHECK_ERROR
                }
                break;
#endif
                
            case TEXCALC_PROJECTIVE_TEXTURE:
                mUseAutoTextureMatrix = true;

                // Set scale and translation matrix for projective textures
                projectionBias = Matrix4::CLIPSPACE2DTOIMAGESPACE;

                projectionBias = projectionBias * frustum->getProjectionMatrix();
                projectionBias = projectionBias * frustum->getViewMatrix();
                projectionBias = projectionBias * mWorldMatrix;

                makeGLMatrix(mAutoTextureMatrix, projectionBias);
                break;

            default:
                break;
        }
		mStateCacheManager->activateGLTextureUnit(0);
    }

    void GLESRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

        // Check to see if blending is supported
        if (!mCurrentCapabilities->hasCapability(RSC_BLENDING))
            return;

        GLenum src1op, src2op, cmd;
        GLfloat cv1[4], cv2[4];

        if (bm.blendType == LBT_COLOUR)
        {
            cv1[0] = bm.colourArg1.r;
            cv1[1] = bm.colourArg1.g;
            cv1[2] = bm.colourArg1.b;
            cv1[3] = bm.colourArg1.a;
            mManualBlendColours[stage][0] = bm.colourArg1;

            cv2[0] = bm.colourArg2.r;
            cv2[1] = bm.colourArg2.g;
            cv2[2] = bm.colourArg2.b;
            cv2[3] = bm.colourArg2.a;
            mManualBlendColours[stage][1] = bm.colourArg2;
        }

        if (bm.blendType == LBT_ALPHA)
        {
            cv1[0] = mManualBlendColours[stage][0].r;
            cv1[1] = mManualBlendColours[stage][0].g;
            cv1[2] = mManualBlendColours[stage][0].b;
            cv1[3] = bm.alphaArg1;

            cv2[0] = mManualBlendColours[stage][1].r;
            cv2[1] = mManualBlendColours[stage][1].g;
            cv2[2] = mManualBlendColours[stage][1].b;
            cv2[3] = bm.alphaArg2;
        }

        switch (bm.source1)
        {
            case LBS_CURRENT:
                src1op = GL_PREVIOUS;
                break;
            case LBS_TEXTURE:
                src1op = GL_TEXTURE;
                break;
            case LBS_MANUAL:
                src1op = GL_CONSTANT;
                break;
            case LBS_DIFFUSE:
                src1op = GL_PRIMARY_COLOR;
                break;
            case LBS_SPECULAR:
                src1op = GL_PRIMARY_COLOR;
                break;
            default:
                src1op = 0;
        }

        switch (bm.source2)
        {
            case LBS_CURRENT:
                src2op = GL_PREVIOUS;
                break;
            case LBS_TEXTURE:
                src2op = GL_TEXTURE;
                break;
            case LBS_MANUAL:
                src2op = GL_CONSTANT;
                break;
            case LBS_DIFFUSE:
                src2op = GL_PRIMARY_COLOR;
                break;
            case LBS_SPECULAR:
                src2op = GL_PRIMARY_COLOR;
                break;
            default:
                src2op = 0;
        }

        switch (bm.operation)
        {
            case LBX_SOURCE1:
                cmd = GL_REPLACE;
                break;
            case LBX_SOURCE2:
                cmd = GL_REPLACE;
                break;
            case LBX_MODULATE:
                cmd = GL_MODULATE;
                break;
            case LBX_MODULATE_X2:
                cmd = GL_MODULATE;
                break;
            case LBX_MODULATE_X4:
                cmd = GL_MODULATE;
                break;
            case LBX_ADD:
                cmd = GL_ADD;
                break;
            case LBX_ADD_SIGNED:
                cmd = GL_ADD_SIGNED;
                break;
            case LBX_ADD_SMOOTH:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_SUBTRACT:
                cmd = GL_SUBTRACT;
                break;
            case LBX_BLEND_DIFFUSE_COLOUR:
                cmd = GL_INTERPOLATE;
                break; 
            case LBX_BLEND_DIFFUSE_ALPHA:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_BLEND_TEXTURE_ALPHA:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_BLEND_CURRENT_ALPHA:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_BLEND_MANUAL:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_DOTPRODUCT:
                cmd = mCurrentCapabilities->hasCapability(RSC_DOT3)
                    ? GL_DOT3_RGB : GL_MODULATE;
                break;
            default:
                cmd = 0;
        }

		if (!mStateCacheManager->activateGLTextureUnit(stage))
			return;
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        GL_CHECK_ERROR;

        if (bm.blendType == LBT_COLOUR)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, cmd);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, src1op);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, src2op);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
            GL_CHECK_ERROR;
        }
        else
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, cmd);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, src1op);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, src2op);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_CONSTANT);
            GL_CHECK_ERROR;
        }

        float blendValue[4] = {0, 0, 0, static_cast<float>(bm.factor)};
        switch (bm.operation)
        {
            case LBX_BLEND_DIFFUSE_COLOUR:
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PRIMARY_COLOR);
                GL_CHECK_ERROR;
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PRIMARY_COLOR);
                GL_CHECK_ERROR;
                break;
            case LBX_BLEND_DIFFUSE_ALPHA:
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PRIMARY_COLOR);
                GL_CHECK_ERROR;
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PRIMARY_COLOR);
                GL_CHECK_ERROR;
                break;
            case LBX_BLEND_TEXTURE_ALPHA:
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_TEXTURE);
                GL_CHECK_ERROR;
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_TEXTURE);
                GL_CHECK_ERROR;
                break;
            case LBX_BLEND_CURRENT_ALPHA:
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PREVIOUS);
                GL_CHECK_ERROR;
                glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, GL_PREVIOUS);
                GL_CHECK_ERROR;
                break;
            case LBX_BLEND_MANUAL:
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendValue);
                GL_CHECK_ERROR;
                break;
            default:
                break;
        };

        switch (bm.operation)
        {
            case LBX_MODULATE_X2:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 2);
                GL_CHECK_ERROR;
                break;
            case LBX_MODULATE_X4:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 4);
                GL_CHECK_ERROR;

                break;
            default:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 1);
                GL_CHECK_ERROR;
                break;
        }

        if (bm.blendType == LBT_COLOUR)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
            GL_CHECK_ERROR;
            if (bm.operation == LBX_BLEND_DIFFUSE_COLOUR)
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
                GL_CHECK_ERROR;
            }
            else
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
                GL_CHECK_ERROR;
            }
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        if (bm.source1 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv1);
        if (bm.source2 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv2);

		mStateCacheManager->activateGLTextureUnit(0);
    }

    GLint GLESRenderSystem::getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const
    {
        switch (tam)
        {
            case TextureUnitState::TAM_CLAMP:
            case TextureUnitState::TAM_BORDER:
                return GL_CLAMP_TO_EDGE;
            case TextureUnitState::TAM_MIRROR:
#if GL_OES_texture_mirrored_repeat
                return GL_MIRRORED_REPEAT_OES;
#endif
            case TextureUnitState::TAM_WRAP:
            default:
                return GL_REPEAT;
        }
    }

    void GLESRenderSystem::_setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw)
    {
		if (!mStateCacheManager->activateGLTextureUnit(stage))
			return;
        mStateCacheManager->setTexParameteri(GL_TEXTURE_2D,
                                             GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u));
        mStateCacheManager->setTexParameteri(GL_TEXTURE_2D,
                                             GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v));
		mStateCacheManager->activateGLTextureUnit(0);
    }

    void GLESRenderSystem::_setTextureBorderColour(size_t stage, const ColourValue& colour)
    {
        // Not supported
    }

    void GLESRenderSystem::_setTextureMipmapBias(size_t unit, float bias)
    {
        if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
        {
#if GL_EXT_texture_lod_bias	// This extension only seems to be supported on iOS OS, block it out to fix Linux build
            if (mStateCacheManager->activateGLTextureUnit(unit))
            {
                glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
                    mStateCacheManager->activateGLTextureUnit(0);
            }
#endif
        }
    }

    void GLESRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xform)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

		if (!mStateCacheManager->activateGLTextureUnit(stage))
			return;

        GLfloat mat[16];
        makeGLMatrix(mat, xform);

        glMatrixMode(GL_TEXTURE);
        GL_CHECK_ERROR;

        // Load this matrix in
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        if (mUseAutoTextureMatrix)
        {
            // Concat auto matrix
            glMultMatrixf(mAutoTextureMatrix);
        }

        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
		mStateCacheManager->activateGLTextureUnit(0);
    }

    GLint GLESRenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
    {
        switch (ogreBlend)
        {
            case SBF_ONE:
                return GL_ONE;
            case SBF_ZERO:
                return GL_ZERO;
            case SBF_DEST_COLOUR:
                return GL_DST_COLOR;
            case SBF_SOURCE_COLOUR:
                return GL_SRC_COLOR;
            case SBF_ONE_MINUS_DEST_COLOUR:
                return GL_ONE_MINUS_DST_COLOR;
            case SBF_ONE_MINUS_SOURCE_COLOUR:
                return GL_ONE_MINUS_SRC_COLOR;
            case SBF_DEST_ALPHA:
                return GL_DST_ALPHA;
            case SBF_SOURCE_ALPHA:
                return GL_SRC_ALPHA;
            case SBF_ONE_MINUS_DEST_ALPHA:
                return GL_ONE_MINUS_DST_ALPHA;
            case SBF_ONE_MINUS_SOURCE_ALPHA:
                return GL_ONE_MINUS_SRC_ALPHA;
        };

        // To keep compiler happy
        return GL_ONE;
    }

	void GLESRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op)
	{
        GL_CHECK_ERROR;
		GLenum sourceBlend = getBlendMode(sourceFactor);
		GLenum destBlend = getBlendMode(destFactor);
		if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
		{
			mStateCacheManager->setDisabled(GL_BLEND);
		}
		else
		{
			// SBF_SOURCE_COLOUR - not allowed for source - http://www.khronos.org/opengles/sdk/1.1/docs/man/
			if(sourceFactor == SBF_SOURCE_COLOUR)
			{
				sourceBlend = getBlendMode(SBF_SOURCE_ALPHA);
			}
            mStateCacheManager->setEnabled(GL_BLEND);
            mStateCacheManager->setBlendFunc(sourceBlend, destBlend);
		}
        
#if GL_OES_blend_subtract
        GLint func = GL_FUNC_ADD_OES;
		switch(op)
		{
		case SBO_ADD:
			func = GL_FUNC_ADD_OES;
			break;
		case SBO_SUBTRACT:
			func = GL_FUNC_SUBTRACT_OES;
			break;
		case SBO_REVERSE_SUBTRACT:
			func = GL_FUNC_REVERSE_SUBTRACT_OES;
			break;
		case SBO_MIN:
#if GL_EXT_blend_minmax
            func = GL_MIN_EXT;
#endif
            break;
		case SBO_MAX:
#if GL_EXT_blend_minmax
            func = GL_MAX_EXT;
#endif
			break;
		}
        
        mStateCacheManager->setBlendEquation(func);
#endif
	}

	void GLESRenderSystem::_setSeparateSceneBlending(
        SceneBlendFactor sourceFactor, SceneBlendFactor destFactor,
        SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha,
        SceneBlendOperation op, SceneBlendOperation alphaOp )
	{
        // Kinda hacky way to prevent this from compiling if the extensions aren't available
#if defined(GL_OES_blend_func_separate) && defined(GL_OES_blend_equation_separate) && defined(GL_EXT_blend_minmax)
        if (mGLSupport->checkExtension("GL_OES_blend_equation_separate") &&
            mGLSupport->checkExtension("GL_OES_blend_func_separate"))
        {
            GLenum sourceBlend = getBlendMode(sourceFactor);
            GLenum destBlend = getBlendMode(destFactor);
            GLenum sourceBlendAlpha = getBlendMode(sourceFactorAlpha);
            GLenum destBlendAlpha = getBlendMode(destFactorAlpha);
            
            if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO && 
               sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
            {
                mStateCacheManager->setDisabled(GL_BLEND);
            }
            else
            {
                mStateCacheManager->setEnabled(GL_BLEND);
                
                glBlendFuncSeparateOES(sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha);
                GL_CHECK_ERROR;
            }
            
            GLint func = GL_FUNC_ADD_OES, alphaFunc = GL_FUNC_ADD_OES;
            
            switch(op)
            {
                case SBO_ADD:
                    func = GL_FUNC_ADD_OES;
                    break;
                case SBO_SUBTRACT:
                    func = GL_FUNC_SUBTRACT_OES;
                    break;
                case SBO_REVERSE_SUBTRACT:
                    func = GL_FUNC_REVERSE_SUBTRACT_OES;
                    break;
                case SBO_MIN:
                    func = GL_MIN_EXT;
                    break;
                case SBO_MAX:
                    func = GL_MAX_EXT;
                    break;
            }
            
            switch(alphaOp)
            {
                case SBO_ADD:
                    alphaFunc = GL_FUNC_ADD_OES;
                    break;
                case SBO_SUBTRACT:
                    alphaFunc = GL_FUNC_SUBTRACT_OES;
                    break;
                case SBO_REVERSE_SUBTRACT:
                    alphaFunc = GL_FUNC_REVERSE_SUBTRACT_OES;
                    break;
                case SBO_MIN:
                    alphaFunc = GL_MIN_EXT;
                    break;
                case SBO_MAX:
                    alphaFunc = GL_MAX_EXT;
                    break;
            }
            
            glBlendEquationSeparateOES(func, alphaFunc);
            GL_CHECK_ERROR;
        }
#endif
	}

    void GLESRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
		bool a2c = false;
		static bool lasta2c = false;
		if (func == CMPF_ALWAYS_PASS)
		{
            mStateCacheManager->setDisabled(GL_ALPHA_TEST);
		}
		else
		{
            mStateCacheManager->setEnabled(GL_ALPHA_TEST);

			a2c = alphaToCoverage;

			glAlphaFunc(convertCompareFunction(func), value / 255.0f);
			GL_CHECK_ERROR;
		}
		if (a2c != lasta2c && getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
		{
			if (a2c)
                mStateCacheManager->setEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
			else
                mStateCacheManager->setDisabled(GL_SAMPLE_ALPHA_TO_COVERAGE);

			lasta2c = a2c;
		} 
	}

    void GLESRenderSystem::_setViewport(Viewport *vp)
    {
		// Check if viewport is different
		if (!vp)
		{
			mActiveViewport = NULL;
			_setRenderTarget(NULL);
		}
		else if (vp != mActiveViewport || vp->_isUpdated())
        {
            RenderTarget* target;

            target = vp->getTarget();
            _setRenderTarget(target);
            mActiveViewport = vp;

            GLsizei x, y, w, h;

			// Calculate the "lower-left" corner of the viewport
            w = vp->getActualWidth();
            h = vp->getActualHeight();
            x = vp->getActualLeft();
            y = vp->getActualTop();

            if (!target->requiresTextureFlipping())
            {
                // Convert "upper-left" corner to "lower-left"
                y = target->getHeight() - h - y;
            }

#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
            ConfigOptionMap::const_iterator opt;
            ConfigOptionMap::const_iterator end = mGLSupport->getConfigOptions().end();
            
            if ((opt = mGLSupport->getConfigOptions().find("Orientation")) != end)
            {
                String val = opt->second.currentValue;
                String::size_type pos = val.find("Landscape");
                
                if (pos != String::npos)
                {
					// Convert x,y since glViewport is expecting the x,y to be lower-left of the portrait orientation
					if (val.find("Left") != String::npos)
					{
						GLsizei temp = x;
						x = y;
						y = target->getWidth() - w - temp;
					}
					else
					{
						GLsizei temp = x;
						x = target->getHeight() - h - y;
						y = temp;
					}
					
                    GLsizei temp = h;
                    h = w;
                    w = temp;
                }
            }
#endif
            glViewport(x, y, w, h);
            GL_CHECK_ERROR;

			// Configure the viewport clipping
            glScissor(x, y, w, h);
            GL_CHECK_ERROR;

            vp->_clearUpdatedFlag();
        }
    }

    void GLESRenderSystem::_beginFrame(void)
    {
        if (!mActiveViewport)
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Cannot begin frame - no viewport selected.",
                        "GLESRenderSystem::_beginFrame");

        if(mCurrentCapabilities->hasCapability(RSC_SCISSOR_TEST)) {
            glEnable(GL_SCISSOR_TEST);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_endFrame(void)
    {
        // Deactivate the viewport clipping.
        if(mCurrentCapabilities->hasCapability(RSC_SCISSOR_TEST)) {
            glDisable(GL_SCISSOR_TEST);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_setCullingMode(CullingMode mode)
    {
        mCullingMode = mode;
        // NB: Because two-sided stencil API dependence of the front face, we must
        // use the same 'winding' for the front face everywhere. As the OGRE default
        // culling mode is clockwise, we also treat anticlockwise winding as front
        // face for consistently. On the assumption that, we can't change the front
        // face by glFrontFace anywhere.

        GLenum cullMode;

        switch( mode )
        {
            case CULL_NONE:
                mStateCacheManager->setDisabled(GL_CULL_FACE);
                return;

            default:
            case CULL_CLOCKWISE:
                if (mActiveRenderTarget &&
                    ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                     (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding)))
                {
                    cullMode = GL_FRONT;
                }
                else
                {
                    cullMode = GL_BACK;
                }
                break;
            case CULL_ANTICLOCKWISE:
                if (mActiveRenderTarget && 
                    ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                    (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding)))
                {
                    cullMode = GL_BACK;
                }
                else
                {
                    cullMode = GL_FRONT;
                }
                break;
        }

        mStateCacheManager->setEnabled(GL_CULL_FACE);
        mStateCacheManager->setCullFace(cullMode);
    }

    void GLESRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);
    }

    void GLESRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            mStateCacheManager->setClearDepth(1.0f);
            mStateCacheManager->setEnabled(GL_DEPTH_TEST);
        }
        else
        {
            mStateCacheManager->setDisabled(GL_DEPTH_TEST);
        }
    }

    void GLESRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        mStateCacheManager->setDepthMask(enabled ? GL_TRUE : GL_FALSE);
    }

    void GLESRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        mStateCacheManager->setDepthFunc(convertCompareFunction(func));
    }

    void GLESRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        if (constantBias != 0 || slopeScaleBias != 0)
        {
            mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-slopeScaleBias, -constantBias);
            GL_CHECK_ERROR;
        }
        else
        {
            mStateCacheManager->setDisabled(GL_POLYGON_OFFSET_FILL);
        }
    }

    void GLESRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        mStateCacheManager->setColourMask(red, green, blue, alpha);
    }

    void GLESRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
        GLint fogMode;
        switch (mode)
        {
            case FOG_EXP:
                fogMode = GL_EXP;
                break;
            case FOG_EXP2:
                fogMode = GL_EXP2;
                break;
            case FOG_LINEAR:
                fogMode = GL_LINEAR;
                break;
            default:
                // Give up on it
                mStateCacheManager->setDisabled(GL_FOG);
                return;
        }

        mStateCacheManager->setEnabled(GL_FOG);
        glFogf(GL_FOG_MODE, fogMode);
        GLfloat fogColor[4] = { colour.r, colour.g, colour.b, colour.a };
        glFogfv(GL_FOG_COLOR, fogColor);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_DENSITY, density);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_START, start);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_END, end);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
                                                  Matrix4& dest,
                                                  bool forGpuProgram)
    {
		// no any conversion request for OpenGL
        dest = matrix;
    }

    void GLESRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect,
                                               Real nearPlane, Real farPlane,
                                               Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

		// Calc matrix elements
        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q, qn;
        if (farPlane == 0)
        {
			// Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

		// NB This creates Z in range [-1,1]
		//
		// [ w   0   0   0  ]
		// [ 0   h   0   0  ]
		// [ 0   0   q   qn ]
		// [ 0   0   -1  0  ]

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    void GLESRenderSystem::_makeProjectionMatrix(Real left, Real right,
                                               Real bottom, Real top,
                                               Real nearPlane, Real farPlane,
                                               Matrix4& dest, bool forGpuProgram)
    {
        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
			// Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    void GLESRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect,
                                          Real nearPlane, Real farPlane,
                                          Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        // Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; // Math::Tan(thetaX);
        Real half_w = tanThetaX * nearPlane;
        Real half_h = tanThetaY * nearPlane;
        Real iw = 1.0 / half_w;
        Real ih = 1.0 / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 2.0 / (farPlane - nearPlane);
        }
        dest = Matrix4::ZERO;
        dest[0][0] = iw;
        dest[1][1] = ih;
        dest[2][2] = -q;
        dest[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        dest[3][3] = 1;
    }

    void GLESRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix,
                                                      const Plane& plane,
                                                      bool forGpuProgram)
    {
		// Thanks to Eric Lenyel for posting this calculation at www.terathon.com
        
		// Calculate the clip-space corner point opposite the clipping plane
		// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
		// transform it into camera space by multiplying it
		// by the inverse of the projection matrix

        Vector4 q;
        q.x = (Math::Sign(plane.normal.x) + matrix[0][2]) / matrix[0][0];
        q.y = (Math::Sign(plane.normal.y) + matrix[1][2]) / matrix[1][1];
        q.z = -1.0F;
        q.w = (1.0F + matrix[2][2]) / matrix[2][3];

        // Calculate the scaled plane vector
        Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
        Vector4 c = clipPlane4d * (2.0F / (clipPlane4d.dotProduct(q)));

        // Replace the third row of the projection matrix
        matrix[2][0] = c.x;
        matrix[2][1] = c.y;
        matrix[2][2] = c.z + 1.0F;
        matrix[2][3] = c.w; 
    }

    void GLESRenderSystem::_setPolygonMode(PolygonMode level)
    {
        switch(level)
        {
        case PM_POINTS:
            mStateCacheManager->setPolygonMode(GL_POINTS);
            break;
        case PM_WIREFRAME:
            mStateCacheManager->setPolygonMode(GL_LINE_STRIP);
            break;
        default:
        case PM_SOLID:
            mStateCacheManager->setPolygonMode(GL_FILL);
            break;
        }
    }

    void GLESRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            mStateCacheManager->setEnabled(GL_STENCIL_TEST);
        }
        else
        {
            mStateCacheManager->setDisabled(GL_STENCIL_TEST);
        }
    }

    void GLESRenderSystem::setStencilBufferParams(CompareFunction func,
                                                uint32 refValue, uint32 compareMask, uint32 writeMask, 
                                                StencilOperation stencilFailOp,
                                                StencilOperation depthFailOp,
                                                StencilOperation passOp,
                                                bool twoSidedOperation)
    {
        mStateCacheManager->setStencilMask(writeMask);
        glStencilFunc(convertCompareFunction(func), refValue, compareMask);
        glStencilOp(
            convertStencilOp(stencilFailOp, false),
            convertStencilOp(depthFailOp, false), 
            convertStencilOp(passOp, false));
    }

    GLuint GLESRenderSystem::getCombinedMinMipFilter(void) const
    {
        switch(mMinFilter)
        {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                switch (mMipFilter)
                {
                    case FO_ANISOTROPIC:
                    case FO_LINEAR:
                        // linear min, linear mip
                        return GL_LINEAR_MIPMAP_LINEAR;
                    case FO_POINT:
                        // linear min, point mip
                        return GL_LINEAR_MIPMAP_NEAREST;
                    case FO_NONE:
                        // linear min, no mip
                        return GL_LINEAR;
                }
                break;
            case FO_POINT:
            case FO_NONE:
                switch (mMipFilter)
                {
                    case FO_ANISOTROPIC:
                    case FO_LINEAR:
                        // nearest min, linear mip
                        return GL_NEAREST_MIPMAP_LINEAR;
                    case FO_POINT:
                        // nearest min, point mip
                        return GL_NEAREST_MIPMAP_NEAREST;
                    case FO_NONE:
                        // nearest min, no mip
                        return GL_NEAREST;
                }
                break;
        }

        // should never get here
        return 0;
    }

	void GLESRenderSystem::_setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
				FilterOptions magFilter, FilterOptions mipFilter)
	{ 		
		mMipFilter = mipFilter;
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		if(mCurTexMipCount == 0 && mMipFilter != FO_NONE)
		{
			mMipFilter = FO_NONE;			
		}
#endif
		_setTextureUnitFiltering(unit, FT_MAG, magFilter);
		_setTextureUnitFiltering(unit, FT_MIN, minFilter);
	}
	
    void GLESRenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions fo)
    {
		if (!mStateCacheManager->activateGLTextureUnit(unit))
			return;

        switch (ftype)
        {
            case FT_MIN:
                mMinFilter = fo;

                // Combine with existing mip filter
                mStateCacheManager->setTexParameteri(GL_TEXTURE_2D,
                                                     GL_TEXTURE_MIN_FILTER,
                                                     getCombinedMinMipFilter());
                break;

            case FT_MAG:
                switch (fo)
                {
                    case FO_ANISOTROPIC: // GL treats linear and aniso the same
                    case FO_LINEAR:
                        mStateCacheManager->setTexParameteri(GL_TEXTURE_2D,
                                                             GL_TEXTURE_MAG_FILTER,
                                                             GL_LINEAR);
                        break;
                    case FO_POINT:
                    case FO_NONE:
                        mStateCacheManager->setTexParameteri(GL_TEXTURE_2D,
                                                             GL_TEXTURE_MAG_FILTER,
                                                             GL_NEAREST);
                        break;
                }
                break;
            case FT_MIP:
                mMipFilter = fo;
                break;
        }

		mStateCacheManager->activateGLTextureUnit(0);
    }

    GLfloat GLESRenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		GLfloat curAniso = 0;
		glGetTexParameterfv(GL_TEXTURE_2D, 
                            GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso);
		return curAniso ? curAniso : 1;
	}
    
    void GLESRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
    {
		if (!mCurrentCapabilities->hasCapability(RSC_ANISOTROPY))
			return;

		if (!mStateCacheManager->activateGLTextureUnit(unit))
			return;

		GLfloat largest_supported_anisotropy = 0;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
		if (maxAnisotropy > largest_supported_anisotropy)
			maxAnisotropy = largest_supported_anisotropy ? 
			static_cast<uint>(largest_supported_anisotropy) : 1;
		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

		mStateCacheManager->activateGLTextureUnit(0);
    }

    void GLESRenderSystem::setVertexDeclaration(VertexDeclaration* decl)
    {
    }

    void GLESRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
    {
    }

    void GLESRenderSystem::_render(const RenderOperation& op)
    {
        GL_CHECK_ERROR;
        // Call super class
        RenderSystem::_render(op);

        void* pBufferData = 0;
		bool multitexturing = (getCapabilities()->getNumTextureUnits() > 1);

        const VertexDeclaration::VertexElementList& decl =
            op.vertexData->vertexDeclaration->getElements();

        VertexDeclaration::VertexElementList::const_iterator elem, elemEnd;

        elemEnd = decl.end();
        vector<GLuint>::type attribsBound;

        for (elem = decl.begin(); elem != elemEnd; ++elem)
        {
            if (!op.vertexData->vertexBufferBinding->isBufferBound(elem->getSource()))
                continue; // skip unbound elements

            HardwareVertexBufferSharedPtr vertexBuffer =
                op.vertexData->vertexBufferBinding->getBuffer(elem->getSource());
            if (mCurrentCapabilities->hasCapability(RSC_VBO))
            {
                mStateCacheManager->bindGLBuffer(GL_ARRAY_BUFFER,
                    static_cast<const GLESHardwareVertexBuffer*>(vertexBuffer.get())->getGLBufferId(), true);
                pBufferData = VBO_BUFFER_OFFSET(elem->getOffset());
            }
            else
            {
                pBufferData = static_cast<const GLESDefaultHardwareVertexBuffer*>(vertexBuffer.get())->getDataPtr(elem->getOffset());
            }

            if (op.vertexData->vertexStart)
            {
                pBufferData = static_cast<char*>(pBufferData) + op.vertexData->vertexStart * vertexBuffer->getVertexSize();
            }

            VertexElementSemantic sem = elem->getSemantic();
            {
                GL_CHECK_ERROR;
                // fixed-function & builtin attribute support
                switch (sem)
                {
                    case VES_POSITION:
                        glVertexPointer(VertexElement::getTypeCount(elem->getType()),
                                        GLESHardwareBufferManager::getGLType(elem->getType()),
                                        static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                        pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_VERTEX_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_NORMAL:
                        glNormalPointer(GLESHardwareBufferManager::getGLType(elem->getType()),
                                        static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                        pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_NORMAL_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_DIFFUSE:
                        glColorPointer(4, GLESHardwareBufferManager::getGLType(elem->getType()),
                                       static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                       pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_COLOR_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_SPECULAR:
                       break;
                    case VES_TEXTURE_COORDINATES:
                        {
                            // fixed function matching to units based on tex_coord_set
                            for (unsigned int i = 0; i < mDisabledTexUnitsFrom; i++)
                            {
                                // Only set this texture unit's texcoord pointer if it
                                // is supposed to be using this element's index
                                if (mTextureCoordIndex[i] == elem->getIndex() && i < mFixedFunctionTextureUnits)
                                {
                                    GL_CHECK_ERROR;
                                    if (multitexturing)
                                        glClientActiveTexture(GL_TEXTURE0 + i);
                                    GL_CHECK_ERROR;
                                    glTexCoordPointer(VertexElement::getTypeCount(elem->getType()),
                                                      GLESHardwareBufferManager::getGLType(elem->getType()),
                                                      static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                                      pBufferData);
                                    GL_CHECK_ERROR;
                                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                                    GL_CHECK_ERROR;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                };
            }
        }

		if (multitexturing)
            glClientActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;

        // Find the correct type to render
        GLint primType;
        switch (op.operationType)
        {
            case RenderOperation::OT_POINT_LIST:
                primType = GL_POINTS;
                break;
            case RenderOperation::OT_LINE_LIST:
                primType = GL_LINES;
                break;
            case RenderOperation::OT_LINE_STRIP:
                primType = GL_LINE_STRIP;
                break;
            default:
            case RenderOperation::OT_TRIANGLE_LIST:
                primType = GL_TRIANGLES;
                break;
            case RenderOperation::OT_TRIANGLE_STRIP:
                primType = GL_TRIANGLE_STRIP;
                break;
            case RenderOperation::OT_TRIANGLE_FAN:
                primType = GL_TRIANGLE_FAN;
                break;
        }

        GLenum polyMode = mStateCacheManager->getPolygonMode();
        if (op.useIndexes)
        {
            if(mCurrentCapabilities->hasCapability(RSC_VBO))
            {
                mStateCacheManager->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             static_cast<GLESHardwareIndexBuffer*>(op.indexData->indexBuffer.get())->getGLBufferId(), true);

                pBufferData = VBO_BUFFER_OFFSET(op.indexData->indexStart *
                                                op.indexData->indexBuffer->getIndexSize());
            }
            else
            {
                pBufferData = static_cast<GLESDefaultHardwareIndexBuffer*>(
                                op.indexData->indexBuffer.get())->getDataPtr(
                                op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
            }

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;

            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }
                
                
                glDrawElements((polyMode == GL_FILL) ? primType : polyMode, op.indexData->indexCount, indexType, pBufferData);
                GL_CHECK_ERROR;
            } while (updatePassIterationRenderState());
        }
        else
        {
            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }
                glDrawArrays((polyMode == GL_FILL) ? primType : polyMode, 0, op.vertexData->vertexCount);
                GL_CHECK_ERROR;
            } while (updatePassIterationRenderState());
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        GL_CHECK_ERROR;

		// Only valid up to GL_MAX_TEXTURE_UNITS, which is recorded in mFixedFunctionTextureUnits
		if (multitexturing)
        {
            for (int i = 0; i < mFixedFunctionTextureUnits; i++)
            {
                glClientActiveTexture(GL_TEXTURE0 + i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            glClientActiveTexture(GL_TEXTURE0);
        }
        else
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        GL_CHECK_ERROR;
        glDisableClientState(GL_NORMAL_ARRAY);
        GL_CHECK_ERROR;
        glDisableClientState(GL_COLOR_ARRAY);
        GL_CHECK_ERROR;

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::setScissorTest(bool enabled, size_t left,
                                        size_t top, size_t right,
                                        size_t bottom)
    {
        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();
        //  GL measures from the bottom, not the top
        size_t targetHeight = mActiveRenderTarget->getHeight();
        // Calculate the "lower-left" corner of the viewport
        GLsizei w, h, x, y;

        if (enabled)
        {
            mStateCacheManager->setEnabled(GL_SCISSOR_TEST);
            // NB GL uses width / height rather than right / bottom
            x = left;
            if (flipping)
                y = top;
            else
                y = targetHeight - bottom;
            w = right - left;
            h = bottom - top;
            glScissor(x, y, w, h);
        }
        else
        {
            mStateCacheManager->setDisabled(GL_SCISSOR_TEST);
            // GL requires you to reset the scissor when disabling
            w = mActiveViewport->getActualWidth();
            h = mActiveViewport->getActualHeight();
            x = mActiveViewport->getActualLeft();
            if (flipping)
                y = mActiveViewport->getActualTop();
            else
                y = targetHeight - mActiveViewport->getActualTop() - h;
            glScissor(x, y, w, h);
        }
    }

    void GLESRenderSystem::clearFrameBuffer(unsigned int buffers,
                                            const ColourValue& colour,
                                            Real depth, unsigned short stencil)
    {
        GLboolean* colourWrite = &mStateCacheManager->getColourMask()[0];
        bool colourMask = !colourWrite[0] || !colourWrite[1] ||
                          !colourWrite[2] || !colourWrite[3];
        GLuint stencilMask = mStateCacheManager->getStencilMask();

        GLbitfield flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= GL_COLOR_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            if (colourMask)
            {
                mStateCacheManager->setColourMask(true, true, true, true);
            }
            mStateCacheManager->setClearColour(colour.r, colour.g, colour.b, colour.a);
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            if (!mStateCacheManager->getDepthMask())
            {
                mStateCacheManager->setDepthMask(GL_TRUE);
            }
            mStateCacheManager->setClearDepth(depth);
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            mStateCacheManager->setStencilMask(0xFFFFFFFF);
            glClearStencil(stencil);
            GL_CHECK_ERROR;
        }

		// Should be enable scissor test due the clear region is
		// relied on scissor box bounds.
        mStateCacheManager->setEnabled(GL_SCISSOR_TEST);

		// Sets the scissor box as same as viewport
        GLint viewport[4], scissor[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        GL_CHECK_ERROR;
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        GL_CHECK_ERROR;
        bool scissorBoxDifference =
            viewport[0] != scissor[0] || viewport[1] != scissor[1] ||
            viewport[2] != scissor[2] || viewport[3] != scissor[3];
        if (scissorBoxDifference)
        {
            glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
            GL_CHECK_ERROR;
        }

        mStateCacheManager->setDiscardBuffers(buffers);

		// Clear buffers
        glClear(flags);
        GL_CHECK_ERROR;

        // Restore scissor box
        if (scissorBoxDifference)
        {
            glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
            GL_CHECK_ERROR;
        }

        // Restore scissor test
        mStateCacheManager->setDisabled(GL_SCISSOR_TEST);

        // Reset buffer write state
        if (!mStateCacheManager->getDepthMask() && (buffers & FBT_DEPTH))
        {
            mStateCacheManager->setDepthMask(GL_FALSE);
        }

        if (colourMask && (buffers & FBT_COLOUR))
        {
            mStateCacheManager->setColourMask(colourWrite[0], colourWrite[1], colourWrite[2], colourWrite[3]);
        }

        if (buffers & FBT_STENCIL)
        {
            mStateCacheManager->setStencilMask(stencilMask);
        }
    }

    HardwareOcclusionQuery* GLESRenderSystem::createHardwareOcclusionQuery(void)
    {
        // Not supported
        return 0;
    }

    Real GLESRenderSystem::getHorizontalTexelOffset(void)
    {
		// No offset in GL
        return 0.0;
    }

    Real GLESRenderSystem::getVerticalTexelOffset(void)
    {
		// No offset in GL
        return 0.0;
    }

    Real GLESRenderSystem::getMinimumDepthInputValue(void)
    {
		// Range [-1.0f, 1.0f]
        return -1.0f;
    }

    Real GLESRenderSystem::getMaximumDepthInputValue(void)
    {
		// Range [-1.0f, 1.0f]
        return 1.0f;
    }

    void GLESRenderSystem::registerThread()
    {
        // Not implemented
    }

    void GLESRenderSystem::unregisterThread()
    {
        // Not implemented
    }

    void GLESRenderSystem::preExtraThreadsStarted()
    {
        // Not implemented
    }

    void GLESRenderSystem::postExtraThreadsStarted()
    {
        // Not implemented
    }

    void GLESRenderSystem::setClipPlanesImpl(const Ogre::PlaneList& clipPlanes)
    {
        // A note on GL user clipping:
        // When an ARB vertex program is enabled in GL, user clipping is completely
        // disabled. There is no way around this, it's just turned off.
        // When using GLSL, user clipping can work but you have to include a
        // glClipVertex command in your vertex shader.
        // Thus the planes set here may not actually be respected.
        int i = 0;
        int numClipPlanes;
        GLfloat clipPlane[4];

        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glPushMatrix();
        GL_CHECK_ERROR;
        // just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        numClipPlanes = clipPlanes.size();

        GLint maxClip;
        glGetIntegerv(GL_MAX_CLIP_PLANES, &maxClip);

        for (i = 0; i < numClipPlanes; ++i)
        {
            GLenum clipPlaneId = static_cast<GLenum>(GL_CLIP_PLANE0 + i);
            const Plane& plane = clipPlanes[i];

            if (i >= maxClip)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Unable to set clip plane",
                            "GLESRenderSystem::setClipPlanes");
            }
            
            clipPlane[0] = plane.normal.x;
            clipPlane[1] = plane.normal.y;
            clipPlane[2] = plane.normal.z;
            clipPlane[3] = plane.d;

            glClipPlanef(clipPlaneId, clipPlane);
            GL_CHECK_ERROR;
            glEnable(clipPlaneId);
            GL_CHECK_ERROR;
        }

        // disable remaining clip planes
        for ( ; i < maxClip; ++i)
        {
            glDisable(static_cast<GLenum>(GL_CLIP_PLANE0 + i));
            GL_CHECK_ERROR;
        }

        // restore matrices
        glPopMatrix();
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_switchContext(GLESContext *context)
    {
        // Disable lights
		for (unsigned short i = 0; i < mCurrentLights; ++i)
		{
			setGLLight(i, NULL);
			mLights[i] = NULL;
		}
		mCurrentLights = 0;

		// Disable textures
		_disableTextureUnitsFrom(0);

        // It's ready for switching
        if(mCurrentContext)
            mCurrentContext->endCurrent();
        mCurrentContext = context;
        mCurrentContext->setCurrent();

        // Check if the context has already done one-time initialisation
        if (!mCurrentContext->getInitialized())
        {
            _oneTimeContextInitialization();
            mCurrentContext->setInitialized();
        }

        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        glDepthMask(mStateCacheManager->getDepthMask());
        GLboolean* colourWrite = &mStateCacheManager->getColourMask()[0];
        glColorMask(colourWrite[0], colourWrite[1], colourWrite[2], colourWrite[3]);
        glStencilMask(mStateCacheManager->getStencilMask());
    }

    void GLESRenderSystem::_unregisterContext(GLESContext *context)
    {
        if (mCurrentContext == context)
        {
			// Change the context to something else so that a valid context
			// remains active. When this is the main context being unregistered,
			// we set the main context to 0.
            if (mCurrentContext != mMainContext)
            {
                _switchContext(mMainContext);
            }
            else
            {
				/// No contexts remain
                mCurrentContext->endCurrent();
                mCurrentContext = 0;
                mMainContext = 0;
            }
        }
    }

    void GLESRenderSystem::_oneTimeContextInitialization()
    {
        mStateCacheManager->initializeCache();
        mStateCacheManager->setDisabled(GL_DITHER);

        int fsaa_active = false;
        glGetIntegerv(GL_SAMPLE_BUFFERS,(GLint*)&fsaa_active);
        GL_CHECK_ERROR;
        if (fsaa_active)
        {
            glEnable(GL_MULTISAMPLE);
            GL_CHECK_ERROR;
            LogManager::getSingleton().logMessage("Using FSAA OpenGL ES.");
        }
        
        static_cast<GLESTextureManager*>(mTextureManager)->createWarningTexture();
    }

    void GLESRenderSystem::initialiseContext(RenderWindow* primary)
    {
		// Set main and current context
        mMainContext = 0;
        primary->getCustomAttribute("GLCONTEXT", &mMainContext);
        mCurrentContext = mMainContext;

		// Set primary context as active
        if (mCurrentContext)
            mCurrentContext->setCurrent();

		// Setup GLSupport
        mGLSupport->initialiseExtensions();

        LogManager::getSingleton().logMessage("**************************************");
        LogManager::getSingleton().logMessage("*** OpenGL ES 1.x Renderer Started ***");
        LogManager::getSingleton().logMessage("**************************************");
    }

    void GLESRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        // Unbind frame buffer object
        if(mActiveRenderTarget)
            mRTTManager->unbind(mActiveRenderTarget);

        mActiveRenderTarget = target;
		if (target)
		{
			// Switch context if different from current one
			GLESContext *newContext = 0;
			target->getCustomAttribute("GLCONTEXT", &newContext);
			if (newContext && mCurrentContext != newContext)
			{
				_switchContext(newContext);
			}

			// Check the FBO's depth buffer status
			GLESDepthBuffer *depthBuffer = static_cast<GLESDepthBuffer*>(target->getDepthBuffer());

			if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
				(!depthBuffer || depthBuffer->getGLContext() != mCurrentContext ) )
			{
				// Depth is automatically managed and there is no depth buffer attached to this RT
				// or the Current context doesn't match the one this Depth buffer was created with
				setDepthBufferFor( target );
			}

			// Bind frame buffer object
			mRTTManager->bind(target);
		}
    }

    void GLESRenderSystem::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
    {
        size_t x = 0;

        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                gl_matrix[x] = m[j][i];
                x++;
            }
        }
    }

    GLint GLESRenderSystem::convertCompareFunction(CompareFunction func) const
    {
        switch(func)
        {
            case CMPF_ALWAYS_FAIL:
                return GL_NEVER;
            case CMPF_ALWAYS_PASS:
                return GL_ALWAYS;
            case CMPF_LESS:
                return GL_LESS;
            case CMPF_LESS_EQUAL:
                return GL_LEQUAL;
            case CMPF_EQUAL:
                return GL_EQUAL;
            case CMPF_NOT_EQUAL:
                return GL_NOTEQUAL;
            case CMPF_GREATER_EQUAL:
                return GL_GEQUAL;
            case CMPF_GREATER:
                return GL_GREATER;
        };
        // To keep compiler happy
        return GL_ALWAYS;
    }

	GLint GLESRenderSystem::convertStencilOp(StencilOperation op, bool invert) const
	{
		switch(op)
		{
		case SOP_KEEP:
			return GL_KEEP;
		case SOP_ZERO:
			return GL_ZERO;
		case SOP_REPLACE:
			return GL_REPLACE;
		case SOP_INCREMENT_WRAP:
        case SOP_INCREMENT:
			return invert ? GL_DECR : GL_INCR;
		case SOP_DECREMENT_WRAP:
		case SOP_DECREMENT:
			return invert ? GL_INCR : GL_DECR;
		case SOP_INVERT:
			return GL_INVERT;
		};
		// to keep compiler happy
		return SOP_KEEP;
	}
    
    void GLESRenderSystem::setLights()
    {
        for (size_t i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i] != NULL)
            {
                Light* lt = mLights[i];
                setGLLightPositionDirection(lt, GL_LIGHT0 + i);
            }
        }
    }

    void GLESRenderSystem::setGLLightPositionDirection(Light* lt, GLenum lightindex)
    {
        // Set position / direction
        Vector4 vec;
        // Use general 4D vector which is the same as GL's approach
        vec = lt->getAs4DVector();

#if OGRE_DOUBLE_PRECISION
		// Must convert to float*
		float tmp[4] = {static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z), static_cast<float>(vec.w)};
		glLightfv(lightindex, GL_POSITION, tmp);
#else
		glLightfv(lightindex, GL_POSITION, vec.ptr());
#endif
		// Set spotlight direction
		if (lt->getType() == Light::LT_SPOTLIGHT)
		{
			vec = lt->getDerivedDirection();
			vec.w = 0.0; 
#if OGRE_DOUBLE_PRECISION
			// Must convert to float*
            float tmp2[4] = {static_cast<float>(vec.x), static_cast<float>(vec.y), static_cast<float>(vec.z), static_cast<float>(vec.w)};
			glLightfv(lightindex, GL_SPOT_DIRECTION, tmp2);
#else
			glLightfv(lightindex, GL_SPOT_DIRECTION, vec.ptr());
#endif
        }
    }

    void GLESRenderSystem::setGLLight(size_t index, Light* lt)
    {
        GLenum gl_index = GL_LIGHT0 + index;

        if (!lt)
        {
            // Disable in the scene
            glDisable(gl_index);
            GL_CHECK_ERROR;
        }
        else
        {
            switch (lt->getType())
            {
                case Light::LT_SPOTLIGHT:
                    glLightf(gl_index, GL_SPOT_CUTOFF, 0.5f * lt->getSpotlightOuterAngle().valueDegrees());
                    GL_CHECK_ERROR;
                    glLightf(gl_index, GL_SPOT_EXPONENT, lt->getSpotlightFalloff());
                    GL_CHECK_ERROR;
                    break;
                default:
                    glLightf(gl_index, GL_SPOT_CUTOFF, 180.0);
                    GL_CHECK_ERROR;
                    break;
            }

            // Color
            ColourValue col;
            col = lt->getDiffuseColour();

            GLfloat f4vals[4] = {col.r, col.g, col.b, col.a};
            glLightfv(gl_index, GL_DIFFUSE, f4vals);
            GL_CHECK_ERROR;

            col = lt->getSpecularColour();
            f4vals[0] = col.r;
            f4vals[1] = col.g;
            f4vals[2] = col.b;
            f4vals[3] = col.a;
            glLightfv(gl_index, GL_SPECULAR, f4vals);
            GL_CHECK_ERROR;

            // Disable ambient light for movables;
            f4vals[0] = 0;
            f4vals[1] = 0;
            f4vals[2] = 0;
            f4vals[3] = 1;
            glLightfv(gl_index, GL_AMBIENT, f4vals);
            GL_CHECK_ERROR;

            setGLLightPositionDirection(lt, gl_index);

            // Attenuation
            glLightf(gl_index, GL_CONSTANT_ATTENUATION, lt->getAttenuationConstant());
            GL_CHECK_ERROR;
            glLightf(gl_index, GL_LINEAR_ATTENUATION, lt->getAttenuationLinear());
            GL_CHECK_ERROR;
            glLightf(gl_index, GL_QUADRATIC_ATTENUATION, lt->getAttenuationQuadric());
            GL_CHECK_ERROR;
            // Enable in the scene
            glEnable(gl_index);
            GL_CHECK_ERROR;
        }
    }

    //---------------------------------------------------------------------
    void GLESRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        // Not implemented
    }

    void GLESRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        // Not implemented
    }

    void GLESRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Not implemented
    }

    void GLESRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {
        // Not implemented
    }

	unsigned int GLESRenderSystem::getDisplayMonitorCount() const
	{
		return 1;
	}
    
	//---------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    void GLESRenderSystem::resetRenderer(RenderWindow* win)
    {
        LogManager::getSingleton().logMessage("********************************************");
        LogManager::getSingleton().logMessage("*** OpenGL ES 1.x Reset Renderer Started ***");
        LogManager::getSingleton().logMessage("********************************************");        
        
        initialiseContext(win);
        
        mGLSupport->initialiseExtensions();
        
        static_cast<GLESFBOManager*>(mRTTManager)->_reload();
        
        _destroyDepthBuffer(win);
        
        GLESDepthBuffer *depthBuffer = OGRE_NEW GLESDepthBuffer( DepthBuffer::POOL_DEFAULT, this,
                                                                  mMainContext, 0, 0,
                                                                  win->getWidth(), win->getHeight(),
                                                                  win->getFSAA(), 0, true );
        
        mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );
        win->attachDepthBuffer( depthBuffer );
        
        GLESRenderSystem::mResourceManager->notifyOnContextReset();
        
		mStateCacheManager->clearCache();
        _setViewport(NULL);
        _setRenderTarget(win);
    }
    
    AndroidResourceManager* GLESRenderSystem::getResourceManager()
    {
        return GLESRenderSystem::mResourceManager;
    }
#endif

    //---------------------------------------------------------------------
    void GLESRenderSystem::_setTextureUnitCompareFunction(size_t unit, CompareFunction function)
    {
        //no effect in GLES rendersystem
    }
    //---------------------------------------------------------------------
    void GLESRenderSystem::_setTextureUnitCompareEnabled(size_t unit, bool compare)
    {
        //no effect in GLES rendersystem
    }
    //---------------------------------------------------------------------
}
