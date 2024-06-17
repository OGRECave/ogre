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
#define NOMINMAX
#include "OgreGLES2RenderSystem.h"
#include "OgreGLES2TextureManager.h"
#include "OgreGLDepthBufferCommon.h"
#include "OgreGLES2HardwarePixelBuffer.h"
#include "OgreGLES2HardwareBufferManager.h"
#include "OgreGLES2HardwareBuffer.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLUtil.h"
#include "OgreGLES2FBORenderTexture.h"
#include "OgreGLES2HardwareOcclusionQuery.h"
#include "OgreGLVertexArrayObject.h"
#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#if !OGRE_NO_GLES2_CG_SUPPORT
#include "OgreGLSLESCgProgramFactory.h"
#endif
#include "OgreGLSLESLinkProgram.h"
#include "OgreGLSLESProgramManager.h"
#include "OgreGLSLESProgramPipeline.h"
#include "OgreGLES2StateCacheManager.h"
#include "OgreRenderWindow.h"
#include "OgreGLES2PixelFormat.h"
#include "OgreGLES2FBOMultiRenderTarget.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "OgreEAGLES2Context.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#   include "OgreGLES2ManagedResourceManager.h"
Ogre::GLES2ManagedResourceManager* Ogre::GLES2RenderSystem::mResourceManager = NULL;
#endif

#ifndef GL_PACK_ROW_LENGTH_NV
#define GL_PACK_ROW_LENGTH_NV             0x0D02
#endif

using namespace std;

static void gl2ext_to_gl3core() {
    glUnmapBufferOES = glUnmapBuffer;
    glRenderbufferStorageMultisampleAPPLE = glRenderbufferStorageMultisample;

    glGenQueriesEXT = glGenQueries;
    glDeleteQueriesEXT = glDeleteQueries;
    glBeginQueryEXT = glBeginQuery;
    glEndQueryEXT = glEndQuery;
    glGetQueryObjectuivEXT = glGetQueryObjectuiv;

    glMapBufferRangeEXT = glMapBufferRange;
    glFlushMappedBufferRangeEXT = glFlushMappedBufferRange;

    glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC)glTexImage3D;
    glCompressedTexImage3DOES = glCompressedTexImage3D;
    glTexSubImage3DOES = glTexSubImage3D;
    glCompressedTexSubImage3DOES = glCompressedTexSubImage3D;

    glFenceSyncAPPLE = glFenceSync;
    glClientWaitSyncAPPLE = glClientWaitSync;
    glDeleteSyncAPPLE = glDeleteSync;

    glProgramBinaryOES = glProgramBinary;
    glGetProgramBinaryOES = glGetProgramBinary;

    glDrawElementsInstancedEXT = glDrawElementsInstanced;
    glDrawArraysInstancedEXT = glDrawArraysInstanced;
    glVertexAttribDivisorEXT = glVertexAttribDivisor;
    glBindVertexArrayOES = glBindVertexArray;
    glGenVertexArraysOES = glGenVertexArrays;
    glDeleteVertexArraysOES = glDeleteVertexArrays;
}

namespace Ogre {

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS && OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    static GLNativeSupport* glsupport;
    static GLESWglProc get_proc(const char* proc) {
        return (GLESWglProc)glsupport->getProcAddress(proc);
    }
#endif

    static GLint getCombinedMinMipFilter(FilterOptions min, FilterOptions mip)
    {
        switch(min)
        {
        case FO_ANISOTROPIC:
        case FO_LINEAR:
            switch (mip)
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
            switch (mip)
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

    GLES2RenderSystem::GLES2RenderSystem()
        : mStateCacheManager(0),
          mProgramManager(0),
          mGLSLESProgramFactory(0),
#if !OGRE_NO_GLES2_CG_SUPPORT
          mGLSLESCgProgramFactory(0),
#endif
          mHardwareBufferManager(0),
          mCurTexMipCount(0)
    {
        size_t i;

        LogManager::getSingleton().logMessage(getName() + " created.");

        mRenderAttribsBound.reserve(100);
        mRenderInstanceAttribsBound.reserve(100);

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        mResourceManager = OGRE_NEW GLES2ManagedResourceManager();
#endif
        
        mGLSupport = getGLSupport(GLNativeSupport::CONTEXT_ES);
        
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS && OGRE_PLATFORM != OGRE_PLATFORM_ANDROID && OGRE_PLATFORM != OGRE_PLATFORM_WIN32
        glsupport = mGLSupport;
#endif

        initConfigOptions();

        for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            // Dummy value
            mTextureTypes[i] = 0;
        }

        mActiveRenderTarget = 0;
        mCurrentContext = 0;
        mMainContext = 0;
        mGLInitialised = false;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
        mPolygonMode = GL_FILL;
        mCurrentVertexProgram = 0;
        mCurrentFragmentProgram = 0;
        mRTTManager = NULL;
    }

    GLES2RenderSystem::~GLES2RenderSystem()
    {
        shutdown();

        OGRE_DELETE mGLSupport;
        mGLSupport = 0;

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
        if (mResourceManager != NULL)
        {
            OGRE_DELETE mResourceManager;
            mResourceManager = NULL;
        }
#endif
    }

    const String& GLES2RenderSystem::getName(void) const
    {
        static String strName("OpenGL ES 2.x Rendering Subsystem");
        return strName;
    }

    void GLES2RenderSystem::_initialise()
    {
        RenderSystem::_initialise();

        mGLSupport->start();
        // Create the texture manager
        mTextureManager = OGRE_NEW GLES2TextureManager(this);
    }

    RenderSystemCapabilities* GLES2RenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = OGRE_NEW RenderSystemCapabilities();

        rsc->setCategoryRelevant(CAPS_CATEGORY_GL, true);
        rsc->setDriverVersion(mDriverVersion);

        const char* deviceName = (const char*)glGetString(GL_RENDERER);
        if (deviceName)
        {
            rsc->setDeviceName(deviceName);
        }

        rsc->setRenderSystemName(getName());
        rsc->setVendor(mVendor);

        // Multitexturing support and set number of texture units
        GLint units;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units));
        rsc->setNumTextureUnits(std::min(OGRE_MAX_TEXTURE_LAYERS, units));

        glGetIntegerv( GL_MAX_VERTEX_ATTRIBS , &units);
        rsc->setNumVertexAttributes(units);

        // Check for hardware stencil support and set bit depth
        GLint stencil;

        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_STENCIL_BITS, &stencil));

        if(stencil)
        {
            rsc->setCapability(RSC_HWSTENCIL);
            rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        }

        if(hasMinGLVersion(3, 0) ||
                (checkExtension("GL_EXT_sRGB") && checkExtension("GL_NV_sRGB_formats")))
            rsc->setCapability(RSC_HW_GAMMA);

        // Vertex Buffer Objects are always supported by OpenGL ES
        if(hasMinGLVersion(3, 0) || checkExtension("GL_OES_element_index_uint"))
            rsc->setCapability(RSC_32BIT_INDEX);

        // Check for hardware occlusion support
        if(hasMinGLVersion(3, 0) || checkExtension("GL_EXT_occlusion_query_boolean"))
        {
            rsc->setCapability(RSC_HWOCCLUSION);
        }

        // OpenGL ES - Check for these extensions too
        // For 2.0, http://www.khronos.org/registry/gles/api/2.0/gl2ext.h

        if (checkExtension("GL_IMG_texture_compression_pvrtc") ||
            checkExtension("GL_EXT_texture_compression_dxt1") ||
            checkExtension("GL_EXT_texture_compression_s3tc") ||
            checkExtension("GL_OES_compressed_ETC1_RGB8_texture") ||
            checkExtension("GL_AMD_compressed_ATC_texture") ||
            checkExtension("WEBGL_compressed_texture_s3tc") ||
            checkExtension("WEBGL_compressed_texture_atc") ||
            checkExtension("WEBGL_compressed_texture_pvrtc") ||
            checkExtension("WEBGL_compressed_texture_etc1") ||
            checkExtension("WEBGL_compressed_texture_astc") ||
            checkExtension("GL_KHR_texture_compression_astc_ldr"))

        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION);

            if(checkExtension("GL_IMG_texture_compression_pvrtc") ||
               checkExtension("GL_IMG_texture_compression_pvrtc2") ||
               checkExtension("WEBGL_compressed_texture_pvrtc"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_PVRTC);
                
            if((checkExtension("GL_EXT_texture_compression_dxt1") &&
               checkExtension("GL_EXT_texture_compression_s3tc")) ||
               checkExtension("WEBGL_compressed_texture_s3tc"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

            if(checkExtension("GL_OES_compressed_ETC1_RGB8_texture") ||
               checkExtension("WEBGL_compressed_texture_etc1"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ETC1);

            if(hasMinGLVersion(3, 0))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ETC2);

            if(checkExtension("GL_AMD_compressed_ATC_texture") ||
               checkExtension("WEBGL_compressed_texture_atc"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ATC);

            if(checkExtension("WEBGL_compressed_texture_astc") ||
               checkExtension("GL_KHR_texture_compression_astc_ldr"))
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);
        }

        // Check for Anisotropy support
        if (checkExtension("GL_EXT_texture_filter_anisotropic"))
        {
            GLfloat maxAnisotropy = 0;
            OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy));
            rsc->setMaxSupportedAnisotropy(maxAnisotropy);
            rsc->setCapability(RSC_ANISOTROPY);
        }

        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        if (hasMinGLVersion(3, 0))
        {
            // Probe number of draw buffers
            // Only makes sense with FBO support, so probe here
            GLint buffers;
            glGetIntegerv(GL_MAX_DRAW_BUFFERS, &buffers);
            rsc->setNumMultiRenderTargets(
                std::min<int>(buffers, (GLint)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
        }
        else
        {
            rsc->setNumMultiRenderTargets(1);
        }

        // Stencil wrapping
        rsc->setCapability(RSC_STENCIL_WRAP);
        
        // Point size
        GLfloat psRange[2] = {0.0, 0.0};
        OGRE_CHECK_GL_ERROR(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, psRange));
        rsc->setMaxPointSize(psRange[1]);

        // Point sprites
        rsc->setCapability(RSC_POINT_SPRITES);
        
        // GLSL ES is always supported in GL ES 2
        rsc->addShaderProfile("glsles");
        if (getNativeShadingLanguageVersion() >= 320)
            rsc->addShaderProfile("glsl320es");
        if (getNativeShadingLanguageVersion() >= 310)
            rsc->addShaderProfile("glsl310es");
        if (getNativeShadingLanguageVersion() >= 300)
            rsc->addShaderProfile("glsl300es");

#if !OGRE_NO_GLES2_CG_SUPPORT
        rsc->addShaderProfile("cg");
        rsc->addShaderProfile("ps_2_0");
        rsc->addShaderProfile("vs_2_0");
#endif

        // Vertex/Fragment Programs
        rsc->setCapability(RSC_VERTEX_PROGRAM);

        // Separate shader objects
        OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
        if(checkExtension("GL_EXT_separate_shader_objects"))
        {
            // this relaxes shader matching rules and requires slightly different GLSL declaration
            // however our usage pattern does not benefit from this and driver support is quite poor
            // so disable it for now (see below)
            /*rsc->setCapability(RSC_SEPARATE_SHADER_OBJECTS);
            rsc->setCapability(RSC_GLSL_SSO_REDECLARE);*/
        }

        // Mesa 11.2 does not behave according to spec and throws a "gl_Position redefined"
        if(rsc->getDeviceName().find("Mesa") != String::npos) {
            rsc->unsetCapability(RSC_GLSL_SSO_REDECLARE);
        }

        GLfloat floatConstantCount = 0;
        glGetFloatv(GL_MAX_VERTEX_UNIFORM_VECTORS, &floatConstantCount);
        rsc->setVertexProgramConstantFloatCount((Ogre::ushort)floatConstantCount);

        // Fragment Program Properties
        floatConstantCount = 0;
        glGetFloatv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &floatConstantCount);
        rsc->setFragmentProgramConstantFloatCount((Ogre::ushort)floatConstantCount);

        // Check for Float textures
        if(hasMinGLVersion(3, 0) || checkExtension("GL_OES_texture_float") || checkExtension("GL_OES_texture_half_float"))
            rsc->setCapability(RSC_TEXTURE_FLOAT);

        if(hasMinGLVersion(3, 0) || checkExtension("GL_OES_texture_3D"))
            rsc->setCapability(RSC_TEXTURE_3D);

        if(hasMinGLVersion(3, 0))
            rsc->setCapability(RSC_TEXTURE_2D_ARRAY);

        // ES 3 always supports NPOT textures
        if(hasMinGLVersion(3, 0) || checkExtension("GL_OES_texture_npot") || checkExtension("GL_ARB_texture_non_power_of_two"))
        {
            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
            rsc->setNonPOW2TexturesLimited(false);
        }
        else if(checkExtension("GL_APPLE_texture_2D_limited_npot"))
        {
            rsc->setNonPOW2TexturesLimited(true);
        }

        // Alpha to coverage always 'supported' when MSAA is available
        // although card may ignore it if it doesn't specifically support A2C
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
        
        // No point sprites, so no size
        rsc->setMaxPointSize(0.f);

        if (hasMinGLVersion(3, 0) ||
            (checkExtension("GL_OES_vertex_array_object") && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN))
            rsc->setCapability(RSC_VAO);

        if (hasMinGLVersion(3, 0) || checkExtension("GL_OES_get_program_binary"))
        {
            // http://www.khronos.org/registry/gles/extensions/OES/OES_get_program_binary.txt
            GLint formats;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &formats));

            if(formats > 0)
                rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);
        }

        if(hasMinGLVersion(3, 0))
        {
            rsc->setCapability(RSC_VERTEX_FORMAT_INT_10_10_10_2);
            rsc->setCapability(RSC_VERTEX_FORMAT_16X3);
        }

        if (hasMinGLVersion(3, 0) || checkExtension("GL_EXT_instanced_arrays"))
        {
            rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        }
        else if(checkExtension("GL_ANGLE_instanced_arrays"))
        {
            rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
            glDrawElementsInstancedEXT = glDrawElementsInstancedANGLE;
            glDrawArraysInstancedEXT = glDrawArraysInstancedANGLE;
            glVertexAttribDivisorEXT = glVertexAttribDivisorANGLE;
        }

        if (checkExtension("GL_EXT_debug_marker") &&
            checkExtension("GL_EXT_debug_label"))
        {
            OGRE_IF_IOS_VERSION_IS_GREATER_THAN(5.0)
            rsc->setCapability(RSC_DEBUG);
        }

        if ((hasMinGLVersion(3, 0) && OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN) ||
            checkExtension("GL_EXT_map_buffer_range"))
        {
            rsc->setCapability(RSC_MAPBUFFER);
        }

        if(hasMinGLVersion(3, 0))
        {
            // Check if render to vertex buffer (transform feedback in OpenGL)
            rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);

            rsc->setCapability(RSC_PRIMITIVE_RESTART);
        }

        GLfloat lineWidth[2] = {1, 1};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidth);
        if(lineWidth[1] != 1 && lineWidth[1] != lineWidth[0])
            rsc->setCapability(RSC_WIDE_LINES);

        return rsc;
    }

    void GLES2RenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if(caps->getNumVertexAttributes() < 16)
            GLSLProgramCommon::useTightAttributeLayout();

        mProgramManager = new GLSLESProgramManager();

        mGLSLESProgramFactory = OGRE_NEW GLSLESProgramFactory();
        HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLESProgramFactory);

#if !OGRE_NO_GLES2_CG_SUPPORT
        mGLSLESCgProgramFactory = OGRE_NEW GLSLESCgProgramFactory();
        HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLESCgProgramFactory);
#endif

        // Use VBO's by default
        mHardwareBufferManager = OGRE_NEW GLES2HardwareBufferManager();

        // Create FBO manager
        mRTTManager = new GLES2FBOManager();

        mGLInitialised = true;
    }

    void GLES2RenderSystem::shutdown(void)
    {

        // Deleting the GLSL program factory
        if (mGLSLESProgramFactory)
        {
            // Remove from manager safely
            if (HighLevelGpuProgramManager::getSingletonPtr())
                HighLevelGpuProgramManager::getSingleton().removeFactory(mGLSLESProgramFactory);
            OGRE_DELETE mGLSLESProgramFactory;
            mGLSLESProgramFactory = 0;
        }

#if !OGRE_NO_GLES2_CG_SUPPORT
        // Deleting the GLSL program factory
        if (mGLSLESCgProgramFactory)
        {
            // Remove from manager safely
            if (HighLevelGpuProgramManager::getSingletonPtr())
                HighLevelGpuProgramManager::getSingleton().removeFactory(mGLSLESCgProgramFactory);
            OGRE_DELETE mGLSLESCgProgramFactory;
            mGLSLESCgProgramFactory = 0;
        }
#endif
        // Delete extra threads contexts
        for (auto pCurContext : mBackgroundContextList)
        {
            pCurContext->releaseContext();
            OGRE_DELETE pCurContext;
        }
        mBackgroundContextList.clear();

        // Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
        delete mProgramManager;
        mProgramManager = NULL;

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        delete mRTTManager;
        mRTTManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;

        RenderSystem::shutdown();

        mGLSupport->stop();

        mGLInitialised = 0;
    }

    RenderWindow* GLES2RenderSystem::_createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                                        bool fullScreen, const NameValuePairList *miscParams)
    {
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);

        // Create the window
        RenderWindow* win = mGLSupport->newWindow(name, width, height, fullScreen, miscParams);
        attachRenderTarget((Ogre::RenderTarget&) *win);

        if (!mGLInitialised)
        {
            initialiseContext(win);

            // Get the shader language version
            const char* shadingLangVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
            LogManager::getSingleton().logMessage("Shading language version: " + String(shadingLangVersion));
            StringVector tokens = StringUtil::split(shadingLangVersion, ". ");
            size_t i = 0;

            // iOS reports the GLSL version with a whole bunch of non-digit characters so we have to find where the version starts.
            for(; i < tokens.size(); i++)
            {
                if (isdigit(*tokens[i].c_str()))
                    break;
            }
            mNativeShadingLanguageVersion = (StringConverter::parseUnsignedInt(tokens[i]) * 100) + StringConverter::parseUnsignedInt(tokens[i+1]);
            if (mNativeShadingLanguageVersion < 100) // Emscripten + MS IE/Edge reports an experimental WebGL version (e.g. 0.96) which causes a compile error
                mNativeShadingLanguageVersion = 100;

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
            // Unlike D3D9, OGL doesn't allow sharing the main depth buffer, so keep them separate.
            GLContext *windowContext = dynamic_cast<GLRenderTarget*>(win)->getContext();
            auto depthBuffer =
                new GLDepthBufferCommon(DepthBuffer::POOL_DEFAULT, this, windowContext, 0, 0, win, true);

            mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

            win->attachDepthBuffer( depthBuffer );
        }

        return win;
    }

    //---------------------------------------------------------------------
    DepthBuffer* GLES2RenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        if( auto fbo = dynamic_cast<GLRenderTarget*>(renderTarget)->getFBO() )
        {
            // Find best depth & stencil format suited for the RT's format
            GLuint depthFormat, stencilFormat;
            mRTTManager->getBestDepthStencil(fbo->getFormat(), &depthFormat, &stencilFormat);

            GLES2RenderBuffer *depthBuffer = OGRE_NEW GLES2RenderBuffer( depthFormat, fbo->getWidth(),
                                                                fbo->getHeight(), fbo->getFSAA() );

            GLES2RenderBuffer *stencilBuffer = NULL;
            if (depthFormat == GL_DEPTH32F_STENCIL8 || depthFormat == GL_DEPTH24_STENCIL8_OES)
            {
                // If we have a packed format, the stencilBuffer is the same as the depthBuffer
                stencilBuffer = depthBuffer;
            }
            else if(stencilFormat)
            {
                stencilBuffer = new GLES2RenderBuffer( stencilFormat, fbo->getWidth(),
                                                       fbo->getHeight(), fbo->getFSAA() );
            }

            return new GLDepthBufferCommon(0, this, mCurrentContext, depthBuffer, stencilBuffer,
                                           renderTarget, false);
        }

        return NULL;
    }

    MultiRenderTarget* GLES2RenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget* retval =
            new GLES2FBOMultiRenderTarget(static_cast<GLES2FBOManager*>(mRTTManager), name);
        attachRenderTarget(*retval);
        return retval;
    }

    void GLES2RenderSystem::destroyRenderWindow(const String& name)
    {
        // Find it to remove from list.
        RenderTarget* pWin = detachRenderTarget(name);
        OgreAssert(pWin, "unknown RenderWindow name");

        _destroyDepthBuffer(pWin);
        OGRE_DELETE pWin;
    }

    void GLES2RenderSystem::_destroyDepthBuffer(RenderTarget* pWin)
    {
        GLContext *windowContext = dynamic_cast<GLRenderTarget*>(pWin)->getContext();
        
        // 1 Window <-> 1 Context, should be always true
        assert( windowContext );
        
        bool bFound = false;
        // Find the depth buffer from this window and remove it.
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
                auto depthBuffer = static_cast<GLDepthBufferCommon*>(*itor);
                GLContext *glContext = depthBuffer->getGLContext();
                
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

    void GLES2RenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
        mStateCacheManager->activateGLTextureUnit(stage);
        if (enabled)
        {
            GLES2TexturePtr tex = static_pointer_cast<GLES2Texture>(texPtr);

            mCurTexMipCount = 0;

            // Note used
            tex->touch();
            mTextureTypes[stage] = tex->getGLES2TextureTarget();
            mCurTexMipCount = tex->getNumMipmaps();

            mStateCacheManager->bindGLTexture(mTextureTypes[stage], tex->getGLID());
        }
        else
        {
            // Bind zero texture
            mStateCacheManager->bindGLTexture(GL_TEXTURE_2D, 0);
        }
    }

    static int getTextureAddressingMode(TextureAddressingMode tam, bool hasBorderClamp = true)
    {
        switch (tam)
        {
        case TAM_BORDER:
            if (hasBorderClamp)
                return GL_CLAMP_TO_BORDER_EXT;
            OGRE_FALLTHROUGH;
        case TAM_CLAMP:
            return GL_CLAMP_TO_EDGE;
        case TAM_MIRROR:
            return GL_MIRRORED_REPEAT;
        case TAM_WRAP:
        default:
            return GL_REPEAT;
        }
    }

    void GLES2RenderSystem::_setSampler(size_t unit, Sampler& sampler)
    {
        mStateCacheManager->activateGLTextureUnit(unit);

        GLenum target = mTextureTypes[unit];

        const Sampler::UVWAddressingMode& uvw = sampler.getAddressingMode();

        bool hasBorderClamp = hasMinGLVersion(3, 2) || checkExtension("GL_EXT_texture_border_clamp") ||
                              checkExtension("GL_OES_texture_border_clamp");

        mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u, hasBorderClamp));
        mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v, hasBorderClamp));
        if(getCapabilities()->hasCapability(RSC_TEXTURE_3D))
            mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_R, getTextureAddressingMode(uvw.w, hasBorderClamp));

        if ((uvw.u == TAM_BORDER || uvw.v == TAM_BORDER || uvw.w == TAM_BORDER) && hasBorderClamp)
            OGRE_CHECK_GL_ERROR(glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR_EXT, sampler.getBorderColour().ptr()));

        // only via shader..
        // OGRE_CHECK_GL_ERROR(glTexParameterf(target, GL_TEXTURE_LOD_BIAS, sampler.getTextureMipmapBias()));

        if (mCurrentCapabilities->hasCapability(RSC_ANISOTROPY))
            mStateCacheManager->setTexParameteri(
                target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min<uint>(getCapabilities()->getMaxSupportedAnisotropy(), sampler.getAnisotropy()));

        if(hasMinGLVersion(3, 0))
        {
            mStateCacheManager->setTexParameteri(target, GL_TEXTURE_COMPARE_MODE,
                                                 sampler.getCompareEnabled() ? GL_COMPARE_REF_TO_TEXTURE
                                                                                    : GL_NONE);
            if(sampler.getCompareEnabled())
                mStateCacheManager->setTexParameteri(target, GL_TEXTURE_COMPARE_FUNC,
                                                 convertCompareFunction(sampler.getCompareFunction()));
        }

        // Combine with existing mip filter
        mStateCacheManager->setTexParameteri(
            target, GL_TEXTURE_MIN_FILTER,
            getCombinedMinMipFilter(sampler.getFiltering(FT_MIN), sampler.getFiltering(FT_MIP)));

        switch (sampler.getFiltering(FT_MAG))
        {
        case FO_ANISOTROPIC: // GL treats linear and aniso the same
        case FO_LINEAR:
            mStateCacheManager->setTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case FO_POINT:
        case FO_NONE:
            mStateCacheManager->setTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }
    }

    void GLES2RenderSystem::_setLineWidth(float width)
    {
        OGRE_CHECK_GL_ERROR(glLineWidth(width));
    }

    GLenum GLES2RenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
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

    void GLES2RenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        if (getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
        {
            if ((func != CMPF_ALWAYS_PASS) && alphaToCoverage)
                mStateCacheManager->setEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
            else
                mStateCacheManager->setDisabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
    }

    void GLES2RenderSystem::_setViewport(Viewport *vp)
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
            
            // Calculate the "lower-left" corner of the viewport
            Rect vpRect = vp->getActualDimensions();
            if (!target->requiresTextureFlipping())
            {
                // Convert "upper-left" corner to "lower-left"
                std::swap(vpRect.top, vpRect.bottom);
                vpRect.top = target->getHeight() - vpRect.top;
                vpRect.bottom = target->getHeight() - vpRect.bottom;
            }

            mStateCacheManager->setViewport(vpRect);

            vp->_clearUpdatedFlag();
        }
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        else
        {
            // On iOS RenderWindow is FBO based, renders to multisampled FBO and then resolves
            // to non-multisampled FBO, therefore we need to restore FBO binding even when
            // rendering to the same viewport.
            RenderTarget* target = vp->getTarget();
            mRTTManager->bind(target);
        }
#endif
    }

    void GLES2RenderSystem::_endFrame(void)
    {
        // unbind GPU programs at end of frame
        // this is mostly to avoid holding bound programs that might get deleted
        // outside via the resource manager
        unbindGpuProgram(GPT_VERTEX_PROGRAM);
		unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
    }

    void GLES2RenderSystem::_setCullingMode(CullingMode mode)
    {
        mCullingMode = mode;
        // NB: Because two-sided stencil API dependence of the front face, we must
        // use the same 'winding' for the front face everywhere. As the OGRE default
        // culling mode is clockwise, we also treat anticlockwise winding as front
        // face for consistently. On the assumption that, we can't change the front
        // face by glFrontFace anywhere.

        GLenum cullMode;
        bool flip = flipFrontFace();
        OGRE_CHECK_GL_ERROR(glFrontFace(flip ? GL_CW : GL_CCW));

        switch (mode)
        {
        case CULL_NONE:
            mStateCacheManager->setDisabled(GL_CULL_FACE);
            return;
        case CULL_CLOCKWISE:
            cullMode = GL_BACK;
            break;
        case CULL_ANTICLOCKWISE:
            cullMode = GL_FRONT;
            break;
        }

        mStateCacheManager->setEnabled(GL_CULL_FACE);
        mStateCacheManager->setCullFace(cullMode);
    }

    void GLES2RenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        if (depthTest)
        {
            mStateCacheManager->setClearDepth(1.0f);
            mStateCacheManager->setEnabled(GL_DEPTH_TEST);
        }
        else
        {
            mStateCacheManager->setDisabled(GL_DEPTH_TEST);
        }
        mStateCacheManager->setDepthMask(depthWrite);
        mStateCacheManager->setDepthFunc(convertCompareFunction(depthFunction));
    }

    void GLES2RenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        if (constantBias != 0 || slopeScaleBias != 0)
        {
            mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_FILL);
            OGRE_CHECK_GL_ERROR(glPolygonOffset(-slopeScaleBias, -constantBias));
        }
        else
        {
            mStateCacheManager->setDisabled(GL_POLYGON_OFFSET_FILL);
        }
    }
    static GLenum getBlendOp(SceneBlendOperation op, bool hasMinMax)
    {
        switch (op)
        {
        case SBO_ADD:
            return GL_FUNC_ADD;
        case SBO_SUBTRACT:
            return GL_FUNC_SUBTRACT;
        case SBO_REVERSE_SUBTRACT:
            return GL_FUNC_REVERSE_SUBTRACT;
        case SBO_MIN:
            return hasMinMax ? GL_MIN : GL_FUNC_ADD;
        case SBO_MAX:
            return hasMinMax ? GL_MAX : GL_FUNC_ADD;
        }
        return GL_FUNC_ADD;
    }
    void GLES2RenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        // record this
        mCurrentBlend = state;

        if (state.blendingEnabled())
        {
            mStateCacheManager->setEnabled(GL_BLEND);
            mStateCacheManager->setBlendFunc(
                getBlendMode(state.sourceFactor), getBlendMode(state.destFactor),
                getBlendMode(state.sourceFactorAlpha), getBlendMode(state.destFactorAlpha));
        }
        else
        {
            mStateCacheManager->setDisabled(GL_BLEND);
        }

        bool hasMinMax = hasMinGLVersion(3, 0) || checkExtension("GL_EXT_blend_minmax");
        mStateCacheManager->setBlendEquation(getBlendOp(state.operation, hasMinMax),
                                             getBlendOp(state.alphaOperation, hasMinMax));
        mStateCacheManager->setColourMask(state.writeR, state.writeG, state.writeB, state.writeA);
    }

    //---------------------------------------------------------------------
    HardwareOcclusionQuery* GLES2RenderSystem::createHardwareOcclusionQuery(void)
    {
        if(hasMinGLVersion(3, 0) || checkExtension("GL_EXT_occlusion_query_boolean"))
        {
            GLES2HardwareOcclusionQuery* ret = new GLES2HardwareOcclusionQuery(); 
            mHwOcclusionQueries.push_back(ret);
            return ret;
        }
        else
        {
            return NULL;
        }
    }

    void GLES2RenderSystem::_setPolygonMode(PolygonMode level)
    {
        switch(level)
        {
        case PM_POINTS:
            mPolygonMode = GL_POINTS;
            break;
        case PM_WIREFRAME:
            mPolygonMode = GL_LINE_STRIP;
            break;
        default:
        case PM_SOLID:
            mPolygonMode = GL_FILL;
            break;
        }
    }

    void GLES2RenderSystem::setStencilState(const StencilState& state)
    {
        if (state.enabled)
        {
            mStateCacheManager->setEnabled(GL_STENCIL_TEST);
        }
        else
        {
            mStateCacheManager->setDisabled(GL_STENCIL_TEST);
            return;
        }

        bool flip = false;

        auto compareOp = convertCompareFunction(state.compareOp);

        if (state.twoSidedOperation)
        {
            // Back
            OGRE_CHECK_GL_ERROR(glStencilMaskSeparate(GL_BACK, state.writeMask));
            OGRE_CHECK_GL_ERROR(glStencilFuncSeparate(GL_BACK, compareOp, state.referenceValue, state.compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOpSeparate(GL_BACK,
                                                    convertStencilOp(state.stencilFailOp, !flip),
                                                    convertStencilOp(state.depthFailOp, !flip),
                                                    convertStencilOp(state.depthStencilPassOp, !flip)));

            // Front
            OGRE_CHECK_GL_ERROR(glStencilMaskSeparate(GL_FRONT, state.writeMask));
            OGRE_CHECK_GL_ERROR(glStencilFuncSeparate(GL_FRONT, compareOp, state.referenceValue, state.compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOpSeparate(GL_FRONT,
                                                    convertStencilOp(state.stencilFailOp, flip),
                                                    convertStencilOp(state.depthFailOp, flip),
                                                    convertStencilOp(state.depthStencilPassOp, flip)));
        }
        else
        {
            flip = false;
            mStateCacheManager->setStencilMask(state.writeMask);
            OGRE_CHECK_GL_ERROR(glStencilFunc(compareOp, state.referenceValue, state.compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOp(
                convertStencilOp(state.stencilFailOp, flip),
                convertStencilOp(state.depthFailOp, flip),
                convertStencilOp(state.depthStencilPassOp, flip)));
        }
    }

    void GLES2RenderSystem::_render(const RenderOperation& op)
    {
        // Call super class
        RenderSystem::_render(op);

        void* pBufferData = 0;

        GLVertexArrayObject* vao = static_cast<GLVertexArrayObject*>(op.vertexData->vertexDeclaration);

        bool updateVAO = true;
        if(getCapabilities()->hasCapability(RSC_VAO))
        {
            vao->bind(this);
            updateVAO = vao->needsUpdate(op.vertexData->vertexBufferBinding,
                                         op.vertexData->vertexStart);
        }

        if (updateVAO)
            vao->bindToGpu(this, op.vertexData->vertexBufferBinding, op.vertexData->vertexStart);

        // We treat index buffer binding inside VAO as volatile, always updating and never relying onto it,
        // as one shared vertex buffer could be rendered with several index buffers, from submeshes and/or LODs
        if (op.useIndexes)
            mStateCacheManager->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER,
                op.indexData->indexBuffer->_getImpl<GLES2HardwareBuffer>()->getGLBufferId());


        size_t numberOfInstances = 0;
        if (getCapabilities()->hasCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA))
        {
            numberOfInstances = op.numberOfInstances;
        }

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

        GLenum polyMode = mPolygonMode;
        if (op.useIndexes)
        {
            pBufferData = VBO_BUFFER_OFFSET(op.indexData->indexStart *
                                            op.indexData->indexBuffer->getIndexSize());

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            do
            {
                if(numberOfInstances > 1)
                {
                    OGRE_CHECK_GL_ERROR(glDrawElementsInstancedEXT((polyMode == GL_FILL) ? primType : polyMode, static_cast<GLsizei>(op.indexData->indexCount), indexType, pBufferData, static_cast<GLsizei>(numberOfInstances)));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glDrawElements((polyMode == GL_FILL) ? primType : polyMode, static_cast<GLsizei>(op.indexData->indexCount), indexType, pBufferData));
                }

            } while (updatePassIterationRenderState());
        }
        else
        {
            do
            {
                if(numberOfInstances > 1)
                {
                    OGRE_CHECK_GL_ERROR(glDrawArraysInstancedEXT((polyMode == GL_FILL) ? primType : polyMode, 0, static_cast<GLsizei>(op.vertexData->vertexCount), static_cast<GLsizei>(numberOfInstances)));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glDrawArrays((polyMode == GL_FILL) ? primType : polyMode, 0, static_cast<GLsizei>(op.vertexData->vertexCount)));
                }
            } while (updatePassIterationRenderState());
        }

        if(getCapabilities()->hasCapability(RSC_VAO))
        {
            // Do not unbind the vertex array object if VAOs are supported
            // VAOs > 0 are selected each time before usage
            // VAO #0 WOULD BE explicitly selected by Ogre before usage
        }
        else // VAOs are not supported, we should clear scratch 'VAO #0'
        {
            // Unbind all attributes
            for(std::vector<GLuint>::iterator ai = mRenderAttribsBound.begin(); ai != mRenderAttribsBound.end(); ++ai)
                OGRE_CHECK_GL_ERROR(glDisableVertexAttribArray(*ai));

            // Unbind any instance attributes
            for (std::vector<GLuint>::iterator ai = mRenderInstanceAttribsBound.begin(); ai != mRenderInstanceAttribsBound.end(); ++ai)
                OGRE_CHECK_GL_ERROR(glVertexAttribDivisorEXT(*ai, 0));
        }
        mRenderAttribsBound.clear();
        mRenderInstanceAttribsBound.clear();
    }

    void GLES2RenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {
        if (!enabled)
        {
            mStateCacheManager->setDisabled(GL_SCISSOR_TEST);
            return;
        }

        mStateCacheManager->setEnabled(GL_SCISSOR_TEST);

        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();

        //  GL measures from the bottom, not the top
        long targetHeight = mActiveRenderTarget->getHeight();
        long top = flipping ? rect.top : targetHeight - rect.bottom;
        // NB GL uses width / height rather than right / bottom
        OGRE_CHECK_GL_ERROR(glScissor(rect.left, top, rect.width(), rect.height()));
    }

    void GLES2RenderSystem::clearFrameBuffer(unsigned int buffers,
                                            const ColourValue& colour,
                                            float depth, unsigned short stencil)
    {
        uchar* colourWrite = mStateCacheManager->getColourMask();
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
                mStateCacheManager->setColourMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            }
            mStateCacheManager->setClearColour(colour.r, colour.g, colour.b, colour.a);
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
            // Enable buffer for writing if it isn't
            mStateCacheManager->setDepthMask(GL_TRUE);
            mStateCacheManager->setClearDepth(depth);
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
            // Enable buffer for writing if it isn't
            mStateCacheManager->setStencilMask(0xFFFFFFFF);
            OGRE_CHECK_GL_ERROR(glClearStencil(stencil));
        }

        Rect vpRect = mActiveViewport->getActualDimensions();
        bool needScissorBox =
            vpRect != Rect(0, 0, mActiveRenderTarget->getWidth(), mActiveRenderTarget->getHeight());
        if (needScissorBox)
        {
            // Should be enable scissor test due the clear region is
            // relied on scissor box bounds.
            setScissorTest(true, vpRect);
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        static_cast<EAGLES2Context*>(mCurrentContext)->mDiscardBuffers = buffers;
#endif

        // Clear buffers
        OGRE_CHECK_GL_ERROR(glClear(flags));

        // Restore scissor test
        if (needScissorBox)
        {
            setScissorTest(false);
        }

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

    void GLES2RenderSystem::_switchContext(GLContext *context)
    {
        // Unbind GPU programs and rebind to new context later, because
        // scene manager treat render system as ONE 'context' ONLY, and it
        // cached the GPU programs using state.
        if (mCurrentVertexProgram)
            mProgramManager->setActiveShader(GPT_VERTEX_PROGRAM, NULL);
        if (mCurrentFragmentProgram)
            mProgramManager->setActiveShader(GPT_FRAGMENT_PROGRAM, NULL);
        
        // Disable textures
        _disableTextureUnitsFrom(0);

        // It's ready for switching
        if (mCurrentContext!=context)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
            // EAGLContext::setCurrentContext does not flush automatically. everybody else does.
            // see https://developer.apple.com/library/content/qa/qa1612/_index.html
            glFlush();
#endif
            mCurrentContext->endCurrent();
            mCurrentContext = context;
        }
        mCurrentContext->setCurrent();

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GLES2StateCacheManager>();
        _completeDeferredVaoFboDestruction();

        // Check if the context has already done one-time initialisation
        if (!mCurrentContext->getInitialized())
        {
            _oneTimeContextInitialization();
            mCurrentContext->setInitialized();
        }

        // Rebind GPU programs to new context
        if (mCurrentVertexProgram)
            mProgramManager->setActiveShader(GPT_VERTEX_PROGRAM, mCurrentVertexProgram);
        if (mCurrentFragmentProgram)
            mProgramManager->setActiveShader(GPT_FRAGMENT_PROGRAM, mCurrentFragmentProgram);
        
        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        uchar* colourWrite = mStateCacheManager->getColourMask();
        GLuint stencilMask = mStateCacheManager->getStencilMask();
        GLboolean depthMask = mStateCacheManager->getDepthMask();
        mStateCacheManager->setStencilMask(stencilMask);
        mStateCacheManager->setColourMask(colourWrite[0], colourWrite[1], colourWrite[2], colourWrite[3]);
        mStateCacheManager->setDepthMask(depthMask);
    }

    void GLES2RenderSystem::_unregisterContext(GLContext *context)
    {
        if(HardwareBufferManager::getSingletonPtr())
            static_cast<GLES2HardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->notifyContextDestroyed(context);
        
        for(RenderTargetMap::iterator it = mRenderTargets.begin(); it!=mRenderTargets.end(); ++it)
        {
            if(auto target = dynamic_cast<GLRenderTarget*>(it->second))
            {
                if(auto fbo = target->getFBO())
                    fbo->notifyContextDestroyed(context);
            }
        }
        
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
                // No contexts remain
                mCurrentContext->endCurrent();
                mCurrentContext = 0;
                mMainContext = 0;
                mStateCacheManager = 0;
            }
        }
    }

    uint32 GLES2RenderSystem::_createVao()
    {
        uint32 vao = 0;
        if(getCapabilities()->hasCapability(RSC_VAO))
            OGRE_CHECK_GL_ERROR(glGenVertexArraysOES(1, &vao));
        return vao;
    }

    void GLES2RenderSystem::_destroyVao(GLContext* context, uint32 vao)
    {
        if(context != mCurrentContext)
            context->_getVaoDeferredForDestruction().push_back(vao);
        else
            OGRE_CHECK_GL_ERROR(glDeleteVertexArraysOES(1, &vao));
    }
    
    void GLES2RenderSystem::_destroyFbo(GLContext* context, uint32 fbo)
    {
        if(context != mCurrentContext)
            context->_getFboDeferredForDestruction().push_back(fbo);
        else
            OGRE_CHECK_GL_ERROR(glDeleteFramebuffers(1, &fbo));
    }

    void GLES2RenderSystem::_bindVao(GLContext* context, uint32 vao)
    {
        OgreAssert(context == mCurrentContext, "VAO used in wrong OpenGL context");
        _getStateCacheManager()->bindGLVertexArray(vao);
    }

    void GLES2RenderSystem::_oneTimeContextInitialization()
    {
        mStateCacheManager->setDisabled(GL_DITHER);

        if(!getCapabilities()->hasCapability(RSC_PRIMITIVE_RESTART))
            return;

        // Enable primitive restarting with fixed indices depending upon the data type
        // https://www.khronos.org/registry/webgl/specs/latest/2.0/#NO_PRIMITIVE_RESTART_FIXED_INDEX
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        OGRE_CHECK_GL_ERROR(glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
#endif
    }

    void GLES2RenderSystem::initialiseContext(RenderWindow* primary)
    {
        // Set main and current context
        mMainContext = dynamic_cast<GLRenderTarget*>(primary)->getContext();
        mCurrentContext = mMainContext;

        // Set primary context as active
        if (mCurrentContext)
            mCurrentContext->setCurrent();

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        // ios: EAGL2Support redirects to glesw for get_proc. Overwriting it there would create an infinite loop
        // android: eglGetProcAddress fails in some cases (e.g. Virtual Device), whereas dlsym always works.
        if (glGetError == NULL && gleswInit())
#else
        if (gleswInit2(get_proc))
#endif
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Could not initialize glesw",
                        "GLES2RenderSystem::initialiseContext");
        }

        // Setup GLSupport
        initialiseExtensions();

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GLES2StateCacheManager>();

        if(hasMinGLVersion(3, 0)) {
            gl2ext_to_gl3core();
            GLES2PixelUtil::useSizedFormats();
        }

        LogManager::getSingleton().logMessage("**************************************");
        LogManager::getSingleton().logMessage("*** OpenGL ES 2.x Renderer Started ***");
        LogManager::getSingleton().logMessage("**************************************");
    }

    void GLES2RenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;
        if (target && mRTTManager)
        {
            // Switch context if different from current one
            GLContext *newContext = dynamic_cast<GLRenderTarget*>(target)->getContext();
            if (newContext && mCurrentContext != newContext)
            {
                _switchContext(newContext);
            }

            // Check the FBO's depth buffer status
            auto depthBuffer = static_cast<GLDepthBufferCommon*>(target->getDepthBuffer());

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

    GLint GLES2RenderSystem::convertCompareFunction(CompareFunction func) const
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

    GLint GLES2RenderSystem::convertStencilOp(StencilOperation op, bool invert) const
    {
        switch(op)
        {
        case SOP_KEEP:
            return GL_KEEP;
        case SOP_ZERO:
            return GL_ZERO;
        case SOP_REPLACE:
            return GL_REPLACE;
        case SOP_INCREMENT:
            return invert ? GL_DECR : GL_INCR;
        case SOP_DECREMENT:
            return invert ? GL_INCR : GL_DECR;
        case SOP_INCREMENT_WRAP:
            return invert ? GL_DECR_WRAP : GL_INCR_WRAP;
        case SOP_DECREMENT_WRAP:
            return invert ? GL_INCR_WRAP : GL_DECR_WRAP;
        case SOP_INVERT:
            return GL_INVERT;
        };
        // to keep compiler happy
        return SOP_KEEP;
    }

    //---------------------------------------------------------------------
    void GLES2RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "Null program bound.",
                        "GLES2RenderSystem::bindGpuProgram");
        }
        
        GLSLESProgram* glprg = static_cast<GLSLESProgram*>(prg);

        switch (glprg->getType())
        {
            case GPT_VERTEX_PROGRAM:
                mCurrentVertexProgram = glprg;
                break;
                
            case GPT_FRAGMENT_PROGRAM:
                mCurrentFragmentProgram = glprg;
                break;
            default:
                break;
        }
        
        // Bind the program
        mProgramManager->setActiveShader(glprg->getType(), glprg);

        RenderSystem::bindGpuProgram(prg);
    }

    void GLES2RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        mProgramManager->setActiveShader(gptype, NULL);
        mActiveParameters[gptype].reset();

        if (gptype == GPT_VERTEX_PROGRAM && mCurrentVertexProgram)
        {
            mCurrentVertexProgram = 0;
        }
        else if (gptype == GPT_FRAGMENT_PROGRAM && mCurrentFragmentProgram)
        {
            mCurrentFragmentProgram = 0;
        }
        RenderSystem::unbindGpuProgram(gptype);
    }

    void GLES2RenderSystem::bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask)
    {
        mActiveParameters[gptype] = params;

        GLSLESProgramCommon* program = NULL;

        // Link can throw exceptions, ignore them at this point
        try
        {
            program = mProgramManager->getActiveProgram();
            // Pass on parameters from params to program object uniforms
            program->updateUniforms(params, mask, gptype);
        }
        catch (Exception& e)
        {
            return;
        }

        if (mask & (uint16)GPV_GLOBAL)
        {
            params->_updateSharedParams();
        }
    }

    //---------------------------------------------------------------------
    void GLES2RenderSystem::beginProfileEvent( const String &eventName )
    {
        if(getCapabilities()->hasCapability(RSC_DEBUG))
            glPushGroupMarkerEXT(0, eventName.c_str());
    }
    //---------------------------------------------------------------------
    void GLES2RenderSystem::endProfileEvent( void )
    {
        if(getCapabilities()->hasCapability(RSC_DEBUG))
            glPopGroupMarkerEXT();
    }
    //---------------------------------------------------------------------
    void GLES2RenderSystem::markProfileEvent( const String &eventName )
    {
        if( eventName.empty() )
            return;

        if(getCapabilities()->hasCapability(RSC_DEBUG))
           glInsertEventMarkerEXT(0, eventName.c_str());
    }
    //---------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLES2RenderSystem::notifyOnContextLost() {
        static_cast<GLES2HardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->notifyContextDestroyed(mCurrentContext);
        GLES2RenderSystem::mResourceManager->notifyOnContextLost();
    }

    void GLES2RenderSystem::resetRenderer(RenderWindow* win)
    {
        LogManager::getSingleton().logMessage("********************************************");
        LogManager::getSingleton().logMessage("*** OpenGL ES 2.x Reset Renderer Started ***");
        LogManager::getSingleton().logMessage("********************************************");
                
        initialiseContext(win);
        
        static_cast<GLES2FBOManager*>(mRTTManager)->_reload();
        
        _destroyDepthBuffer(win);

        auto depthBuffer =
            new GLDepthBufferCommon(DepthBuffer::POOL_DEFAULT, this, mMainContext, 0, 0, win, true);

        mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );
        win->attachDepthBuffer( depthBuffer );
        
        GLES2RenderSystem::mResourceManager->notifyOnContextReset();
        
        mStateCacheManager->clearCache();
        _setViewport(NULL);
        _setRenderTarget(win);
    }
    
    GLES2ManagedResourceManager* GLES2RenderSystem::getResourceManager()
    {
        return GLES2RenderSystem::mResourceManager;
    }
#endif

    void GLES2RenderSystem::bindVertexElementToGpu(
        const VertexElement& elem, const HardwareVertexBufferSharedPtr& vertexBuffer,
        const size_t vertexStart)
    {
        VertexElementSemantic sem = elem.getSemantic();
        unsigned short elemIndex = elem.getIndex();

        const GLES2HardwareBuffer* hwGlBuffer = vertexBuffer->_getImpl<GLES2HardwareBuffer>();

        mStateCacheManager->bindGLBuffer(GL_ARRAY_BUFFER, hwGlBuffer->getGLBufferId());
        void* pBufferData = VBO_BUFFER_OFFSET(elem.getOffset() + vertexStart * vertexBuffer->getVertexSize());

        unsigned short typeCount = VertexElement::getTypeCount(elem.getType());
        GLboolean normalised = GL_FALSE;
        GLuint attrib = 0;

        attrib = (GLuint)GLSLProgramCommon::getFixedAttributeIndex(sem, elemIndex);

        if(getCapabilities()->hasCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA))
        {
            if (mCurrentVertexProgram)
            {
                if (vertexBuffer->isInstanceData())
                {
                    OGRE_CHECK_GL_ERROR(glVertexAttribDivisorEXT(attrib, static_cast<GLuint>(vertexBuffer->getInstanceDataStepRate())));
                    mRenderInstanceAttribsBound.push_back(attrib);
                }
            }
        }

        switch(elem.getType())
        {
            case VET_UBYTE4_NORM:
            case VET_SHORT2_NORM:
            case VET_USHORT2_NORM:
            case VET_SHORT4_NORM:
            case VET_USHORT4_NORM:
            case VET_INT_10_10_10_2_NORM:
                normalised = GL_TRUE;
                break;
            default:
                break;
        };

        OGRE_CHECK_GL_ERROR(glVertexAttribPointer(attrib,
                                                  typeCount,
                                                  GLES2HardwareBufferManager::getGLType(elem.getType()),
                                                  normalised,
                                                  static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                                  pBufferData));

        OGRE_CHECK_GL_ERROR(glEnableVertexAttribArray(attrib));
        mRenderAttribsBound.push_back(attrib);
    }

    void GLES2RenderSystem::_copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox& dst,
                                                  RenderWindow::FrameBuffer buffer) {
        GLenum format = GLES2PixelUtil::getGLOriginFormat(dst.format);
        GLenum type = GLES2PixelUtil::getGLOriginDataType(dst.format);

        if (dst.format != PF_BYTE_RGBA)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Unsupported format.",
                "GLES2RenderSystem::_copyContentsToMemory" );
        }

        bool hasPackImage = hasMinGLVersion(3, 0) || checkExtension("GL_NV_pack_subimage");
        OgreAssert(dst.getWidth() == dst.rowPitch || hasPackImage, "GL_PACK_ROW_LENGTH not supported");

        // Switch context if different from current one
        _setViewport(vp);

        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        if(dst.getWidth() != dst.rowPitch && hasPackImage)
            glPixelStorei(GL_PACK_ROW_LENGTH_NV, dst.rowPitch);

        // Must change the packing to ensure no overruns!
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        if(hasMinGLVersion(3, 0)) {
            glReadBuffer((buffer == RenderWindow::FB_FRONT) ? GL_FRONT : GL_BACK);
        }

        uint32_t height = vp->getTarget()->getHeight();

        glReadPixels((GLint)src.left, (GLint)(height - src.bottom),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());

        // restore default alignment
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH_NV, 0);

        PixelUtil::bulkPixelVerticalFlip(dst);
    }

    void GLES2RenderSystem::initialiseExtensions(void)
    {
        String tmpStr;
#if 1
        // Set version string
        const GLubyte* pcVer = glGetString(GL_VERSION);
        assert(pcVer && "Problems getting GL version string using glGetString");
        tmpStr = (const char*)pcVer;

        // format explained here:
        // https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetString.xml
        size_t offset = sizeof("OpenGL ES ") - 1;
        if(tmpStr.length() > offset) {
            mDriverVersion.fromString(tmpStr.substr(offset, tmpStr.find(' ', offset)));
        }
#else
        // GLES3 way, but should work with ES2 as well, so disabled for now
        glGetIntegerv(GL_MAJOR_VERSION, &mVersion.major);
        glGetIntegerv(GL_MINOR_VERSION, &mVersion.minor);
#endif

        LogManager::getSingleton().logMessage("GL_VERSION = " + mDriverVersion.toString());


        // Get vendor
        const GLubyte* pcVendor = glGetString(GL_VENDOR);
        tmpStr = (const char*)pcVendor;
        LogManager::getSingleton().logMessage("GL_VENDOR = " + tmpStr);
        mVendor = RenderSystemCapabilities::vendorFromString(tmpStr.substr(0, tmpStr.find(' ')));

        // Get renderer
        const GLubyte* pcRenderer = glGetString(GL_RENDERER);
        tmpStr = (const char*)pcRenderer;
        LogManager::getSingleton().logMessage("GL_RENDERER = " + tmpStr);

        // Set extension list
        StringStream ext;
        String str;

        const GLubyte* pcExt = glGetString(GL_EXTENSIONS);
        OgreAssert(pcExt, "Problems getting GL extension string using glGetString");
        ext << pcExt;

        Log::Stream log = LogManager::getSingleton().stream();
        log << "GL_EXTENSIONS = ";
        while (ext >> str)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
            // emscripten gives us both prefixed (GL_) and unprefixed (EXT_) forms
            // use prefixed form (GL_EXT) unless its a WebGL extension (WEBGL_)
            if ((str.substr(0, 3) != "GL_" && str.substr(0, 6) != "WEBGL_") || str.substr(0, 9) == "GL_WEBGL_")
                continue;
#endif
            log << str << " ";
            mExtensionList.insert(str);
        }
    }
}
