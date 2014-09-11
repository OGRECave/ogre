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


#include "OgreGL3PlusRenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreGL3PlusTextureManager.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreGL3PlusHardwareIndexBuffer.h"
#include "OgreGL3PlusDefaultHardwareBufferManager.h"
#include "OgreGL3PlusUtil.h"
#include "OgreGL3PlusGpuProgram.h"
#include "OgreGL3PlusGpuProgramManager.h"
#include "OgreException.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGL3PlusHardwareOcclusionQuery.h"
#include "OgreGL3PlusDepthBuffer.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusContext.h"
#include "OgreGLSLProgramFactory.h"
#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreGLSLProgramPipeline.h"
#include "OgreGLSLLinkProgramManager.h"
#include "OgreGL3PlusVertexArrayObject.h"
#include "OgreRoot.h"
#include "OgreConfig.h"

#if OGRE_DEBUG_MODE
static void APIENTRY GLDebugCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* message,
                            GLvoid* userParam)
{
    char debSource[32], debType[32], debSev[32];

    if(source == GL_DEBUG_SOURCE_API)
        strcpy(debSource, "OpenGL");
    else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
        strcpy(debSource, "Windows");
    else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER)
        strcpy(debSource, "Shader Compiler");
    else if(source == GL_DEBUG_SOURCE_THIRD_PARTY)
        strcpy(debSource, "Third Party");
    else if(source == GL_DEBUG_SOURCE_APPLICATION)
        strcpy(debSource, "Application");
    else if(source == GL_DEBUG_SOURCE_OTHER)
        strcpy(debSource, "Other");

    if(type == GL_DEBUG_TYPE_ERROR)
        strcpy(debType, "error");
    else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
        strcpy(debType, "deprecated behavior");
    else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
        strcpy(debType, "undefined behavior");
    else if(type == GL_DEBUG_TYPE_PORTABILITY)
        strcpy(debType, "portability");
    else if(type == GL_DEBUG_TYPE_PERFORMANCE)
        strcpy(debType, "performance");
    else if(type == GL_DEBUG_TYPE_OTHER)
        strcpy(debType, "message");

    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        strcpy(debSev, "high");
    }
    else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
        strcpy(debSev, "medium");
    else if(severity == GL_DEBUG_SEVERITY_LOW)
        strcpy(debSev, "low");

    Ogre::LogManager::getSingleton().stream() << debSource << ":" << debType << "(" << debSev << ") " << id << ": " << message;
}
#endif

namespace Ogre {

    GL3PlusRenderSystem::GL3PlusRenderSystem()
        : mDepthWrite(true),
          mStencilWriteMask(0xFFFFFFFF),
          mGpuProgramManager(0),
          mGLSLProgramFactory(0),
          mHardwareBufferManager(0),
          mRTTManager(0),
		mActiveTextureUnit(0)
    {
        size_t i;

		LogManager::getSingleton().logMessage(getName() + " created.");

        mRenderAttribsBound.reserve(100);
        mRenderInstanceAttribsBound.reserve(100);

		// Get our GLSupport
        mGLSupport = getGLSupport();

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;

		initConfigOptions();

        mColourWrite[0] = mColourWrite[1] = mColourWrite[2] = mColourWrite[3] = true;

        for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            // Dummy value
            mTextureCoordIndex[i] = 99;
            mTextureTypes[i] = 0;
        }

        mActiveRenderTarget = 0;
        mCurrentContext = 0;
        mMainContext = 0;
        mGLInitialised = false;
        mTextureMipmapCount = 0;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
        mCurrentVertexProgram = 0;
		mCurrentGeometryProgram = 0;
        mCurrentFragmentProgram = 0;
		mCurrentHullProgram = 0;
        mCurrentDomainProgram = 0;
        mCurrentComputeProgram = 0;
        mPolygonMode = GL_FILL;
        mEnableFixedPipeline = false;
    }

    GL3PlusRenderSystem::~GL3PlusRenderSystem()
    {
        shutdown();

		// Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            delete i->second;
        }

        mRenderTargets.clear();
        if(mGLSupport)
            delete mGLSupport;
    }

    const String& GL3PlusRenderSystem::getName(void) const
    {
        static String strName("OpenGL 3+ Rendering Subsystem (EXPERIMENTAL)");
        return strName;
    }

	void GL3PlusRenderSystem::initConfigOptions(void)
	{
		mGLSupport->addConfig();
	}

    ConfigOptionMap& GL3PlusRenderSystem::getConfigOptions(void)
    {
        return mGLSupport->getConfigOptions();
    }

    void GL3PlusRenderSystem::setConfigOption(const String &name, const String &value)
    {
        mGLSupport->setConfigOption(name, value);
    }

    String GL3PlusRenderSystem::validateConfigOptions(void)
    {
		// XXX Return an error string if something is invalid
        return mGLSupport->validateConfig();
    }

    RenderWindow* GL3PlusRenderSystem::_initialise(bool autoCreateWindow,
                                                 const String& windowTitle)
    {
		mGLSupport->start();

        RenderWindow *autoWindow = mGLSupport->createWindow(autoCreateWindow,
                                                            this, windowTitle);
        RenderSystem::_initialise(autoCreateWindow, windowTitle);
        return autoWindow;
    }

    RenderSystemCapabilities* GL3PlusRenderSystem::createRenderSystemCapabilities() const
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
		if (strstr(vendorName, "NVIDIA"))
			rsc->setVendor(GPU_NVIDIA);
		else if (strstr(vendorName, "ATI"))
			rsc->setVendor(GPU_AMD);
		else if (strstr(vendorName, "AMD"))
			rsc->setVendor(GPU_AMD);
		else if (strstr(vendorName, "Intel"))
			rsc->setVendor(GPU_INTEL);
        else
            rsc->setVendor(GPU_UNKNOWN);

        // Check for hardware mipmapping support.
        bool disableAutoMip = false;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        // Apple & Linux ATI drivers have faults in hardware mipmap generation
        // TODO: Test this with GL3+
        if (rsc->getVendor() == GPU_AMD)
            disableAutoMip = true;
#endif
        // The Intel 915G frequently corrupts textures when using hardware mip generation
        // I'm not currently sure how many generations of hardware this affects, 
        // so for now, be safe.
        if (rsc->getVendor() == GPU_INTEL)
            disableAutoMip = true;

        if (!disableAutoMip)
            rsc->setCapability(RSC_AUTOMIPMAP);

		// Check for blending support
        rsc->setCapability(RSC_BLENDING);

        // Multitexturing support and set number of texture units
        GLint units;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units));
        rsc->setNumTextureUnits(std::min<ushort>(16, units));

        // Check for Anisotropy support
        if (mGLSupport->checkExtension("GL_EXT_texture_filter_anisotropic"))
            rsc->setCapability(RSC_ANISOTROPY);

        // DOT3 support is standard
        rsc->setCapability(RSC_DOT3);

        // Cube map
        rsc->setCapability(RSC_CUBEMAPPING);

        // Point sprites
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);

        // Check for hardware stencil support and set bit depth
        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setStencilBufferBitDepth(8);

        // Vertex Buffer Objects are always supported
        rsc->setCapability(RSC_VBO);
        rsc->setCapability(RSC_32BIT_INDEX);

        // Vertex Array Objects are supported in 3.0
        rsc->setCapability(RSC_VAO);

        // Check for texture compression
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);

        // Check for dxt compression
        if(mGLSupport->checkExtension("GL_EXT_texture_compression_s3tc"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        }

        // Check for etc compression
        if(mGLSupport->checkExtension("GL_ARB_ES3_compatibility") || gl3wIsSupported(4, 3))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_ETC2);
        }

        // Check for vtc compression
        if(mGLSupport->checkExtension("GL_NV_texture_compression_vtc"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_VTC);
        }

        // RGTC(BC4/BC5) is supported by the 3.0 spec
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);

        // BPTC(BC6H/BC7) is supported by the extension or OpenGL 4.2 or higher
        if(mGLSupport->checkExtension("GL_ARB_texture_compression_bptc") || gl3wIsSupported(4, 2))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
        }

        rsc->setCapability(RSC_FBO);
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        // Probe number of draw buffers
        // Only makes sense with FBO support, so probe here
        GLint buffers;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_DRAW_BUFFERS, &buffers));
        rsc->setNumMultiRenderTargets(std::min<int>(buffers, (GLint)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
        rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);

        // Stencil wrapping
        rsc->setCapability(RSC_STENCIL_WRAP);

        // GL always shares vertex and fragment texture units (for now?)
        rsc->setVertexTextureUnitsShared(true);

        // Blending support
        rsc->setCapability(RSC_BLENDING);
        rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);

        // Check for non-power-of-2 texture support
        if(mGLSupport->checkExtension("GL_ARB_texture_rectangle") || mGLSupport->checkExtension("GL_ARB_texture_non_power_of_two") ||
           gl3wIsSupported(3, 1))
            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);

        // Check for atomic counter support
        if(mGLSupport->checkExtension("GL_ARB_shader_atomic_counters") || gl3wIsSupported(4, 2))
            rsc->setCapability(RSC_ATOMIC_COUNTERS);

        // Scissor test is standard
        rsc->setCapability(RSC_SCISSOR_TEST);

		// As are user clipping planes
		rsc->setCapability(RSC_USER_CLIP_PLANES);

        // So are 1D & 3D textures
        rsc->setCapability(RSC_TEXTURE_1D);
        rsc->setCapability(RSC_TEXTURE_3D);

        // UBYTE4 always supported
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

        // Infinite far plane always supported
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);

        // Check for hardware occlusion support
        rsc->setCapability(RSC_HWOCCLUSION);

        // Point size
        GLfloat psRange[2] = {0.0, 0.0};
        OGRE_CHECK_GL_ERROR(glGetFloatv(GL_POINT_SIZE_RANGE, psRange));
        rsc->setMaxPointSize(psRange[1]);

        // GLSL is always supported in GL
        // TODO: Deprecate this profile name in favor of versioned names
        rsc->addShaderProfile("glsl");

        // Support for specific shader profiles
        if(getNativeShadingLanguageVersion() >= 430)
            rsc->addShaderProfile("glsl430");
        if(getNativeShadingLanguageVersion() >= 420)
            rsc->addShaderProfile("glsl420");
        if(getNativeShadingLanguageVersion() >= 410)
            rsc->addShaderProfile("glsl410");
        if(getNativeShadingLanguageVersion() >= 400)
            rsc->addShaderProfile("glsl400");
        if(getNativeShadingLanguageVersion() >= 330)
            rsc->addShaderProfile("glsl330");
        if(getNativeShadingLanguageVersion() >= 150)
            rsc->addShaderProfile("glsl150");
        if(getNativeShadingLanguageVersion() >= 140)
            rsc->addShaderProfile("glsl140");
        if(getNativeShadingLanguageVersion() >= 130)
            rsc->addShaderProfile("glsl130");

        // FIXME: This isn't working right yet in some rarer cases
        if(mGLSupport->checkExtension("GL_ARB_separate_shader_objects") || gl3wIsSupported(4, 1))
            rsc->setCapability(RSC_SEPARATE_SHADER_OBJECTS);

        // Vertex/Fragment Programs
        rsc->setCapability(RSC_VERTEX_PROGRAM);
        rsc->setCapability(RSC_FRAGMENT_PROGRAM);

        GLfloat floatConstantCount = 0;
        OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &floatConstantCount));
        rsc->setVertexProgramConstantFloatCount((Ogre::ushort)floatConstantCount);
        rsc->setVertexProgramConstantBoolCount((Ogre::ushort)floatConstantCount);
        rsc->setVertexProgramConstantIntCount((Ogre::ushort)floatConstantCount);

        // Fragment Program Properties
        floatConstantCount = 0;
        OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &floatConstantCount));
        rsc->setFragmentProgramConstantFloatCount((Ogre::ushort)floatConstantCount);
        rsc->setFragmentProgramConstantBoolCount((Ogre::ushort)floatConstantCount);
        rsc->setFragmentProgramConstantIntCount((Ogre::ushort)floatConstantCount);

        // Geometry Program Properties
        rsc->setCapability(RSC_GEOMETRY_PROGRAM);

        OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &floatConstantCount));
        rsc->setGeometryProgramConstantFloatCount(floatConstantCount);
        
        GLint maxOutputVertices;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxOutputVertices));
        rsc->setGeometryProgramNumOutputVertices(maxOutputVertices);

        rsc->setGeometryProgramConstantBoolCount(0);
        rsc->setGeometryProgramConstantIntCount(0);

        // Tessellation Program Properties
        if(mGLSupport->checkExtension("GL_ARB_tessellation_shader") || gl3wIsSupported(4, 0))
        {
            rsc->setCapability(RSC_TESSELATION_HULL_PROGRAM);
            rsc->setCapability(RSC_TESSELATION_DOMAIN_PROGRAM);
        
            OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, &floatConstantCount));

            // 16 boolean params allowed
            rsc->setTesselationHullProgramConstantBoolCount(floatConstantCount);
            // 16 integer params allowed, 4D
            rsc->setTesselationHullProgramConstantIntCount(floatConstantCount);
            // float params, always 4D
            rsc->setTesselationHullProgramConstantFloatCount(floatConstantCount);

            OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, &floatConstantCount));
            // 16 boolean params allowed
            rsc->setTesselationDomainProgramConstantBoolCount(floatConstantCount);
            // 16 integer params allowed, 4D
            rsc->setTesselationDomainProgramConstantIntCount(floatConstantCount);
            // float params, always 4D
            rsc->setTesselationDomainProgramConstantFloatCount(floatConstantCount);
        }

        if (mGLSupport->checkExtension("GL_ARB_get_program_binary") || gl3wIsSupported(4, 1))
		{
            GLint formats;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats));

            if(formats > 0)
                rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);
		}

        if (mGLSupport->checkExtension("GL_ARB_instanced_arrays") || gl3wIsSupported(3, 3))
		{
			rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
		}

        // Check for Float textures
        rsc->setCapability(RSC_TEXTURE_FLOAT);

        // OpenGL 3.0 requires a minimum of 16 texture image units
        units = std::max<GLint>(16, units);
        
		rsc->setNumVertexTextureUnits(static_cast<ushort>(units));
		if (units > 0)
		{
			rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
		}

		// Mipmap LOD biasing?
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // Alpha to coverage always 'supported' when MSAA is available
        // although card may ignore it if it doesn't specifically support A2C
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
		
        // Check if render to vertex buffer (transform feedback in OpenGL)
        rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);

        return rsc;
    }

    void GL3PlusRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if(caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to initialize GL3PlusRenderSystem from RenderSystemCapabilities that do not support OpenGL 3+",
                        "GL3PlusRenderSystem::initialiseFromRenderSystemCapabilities");
        }

        mGpuProgramManager = OGRE_NEW GL3PlusGpuProgramManager();

        // Create GLSL program factory
        mGLSLProgramFactory = new GLSLProgramFactory();
        HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLProgramFactory);

        // Set texture the number of texture units
        mFixedFunctionTextureUnits = caps->getNumTextureUnits();

        // Use VBO's by default
        mHardwareBufferManager = new GL3PlusHardwareBufferManager();

		// Use FBO's for RTT, PBuffers and Copy are no longer supported
        // Create FBO manager
        LogManager::getSingleton().logMessage("GL3+: Using FBOs for rendering to textures");
        mRTTManager = new GL3PlusFBOManager();
        caps->setCapability(RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL);

		Log* defaultLog = LogManager::getSingleton().getDefaultLog();
		if (defaultLog)
		{
			caps->log(defaultLog);
		}

        // Create the texture manager        
        mTextureManager = new GL3PlusTextureManager(*mGLSupport);

        if (caps->hasCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER))
		{
            // Enable microcache
            mGpuProgramManager->setSaveMicrocodesToCache(true);
		}

        mGLInitialised = true;
    }

    void GL3PlusRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->_initialise(true);
    }

    void GL3PlusRenderSystem::shutdown(void)
    {
        // Deleting the GLSL program factory
		if (mGLSLProgramFactory)
		{
			// Remove from manager safely
			if (HighLevelGpuProgramManager::getSingletonPtr())
				HighLevelGpuProgramManager::getSingleton().removeFactory(mGLSLProgramFactory);
			delete mGLSLProgramFactory;
			mGLSLProgramFactory = 0;
		}

		// Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
        delete mGpuProgramManager;
        mGpuProgramManager = 0;

        delete mHardwareBufferManager;
        mHardwareBufferManager = 0;

        delete mRTTManager;
        mRTTManager = 0;

        delete mTextureManager;
        mTextureManager = 0;

        // Delete extra threads contexts
		for (GL3PlusContextList::iterator i = mBackgroundContextList.begin(); 
             i != mBackgroundContextList.end(); ++i)
		{
			GL3PlusContext* pCurContext = *i;
            
			pCurContext->releaseContext();
            
			delete pCurContext;
		}
		mBackgroundContextList.clear();

        mGLSupport->stop();
		mStopRendering = true;

        mGLInitialised = 0;
		
		RenderSystem::shutdown();
    }

	bool GL3PlusRenderSystem::_createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
                                               RenderWindowList& createdWindows)
	{		
		// Call base render system method.
		if (false == RenderSystem::_createRenderWindows(renderWindowDescriptions, createdWindows))
			return false;
        
		// Simply call _createRenderWindow in a loop.
		for (size_t i = 0; i < renderWindowDescriptions.size(); ++i)
		{
			const RenderWindowDescription& curRenderWindowDescription = renderWindowDescriptions[i];			
			RenderWindow* curWindow = NULL;
            
			curWindow = _createRenderWindow(curRenderWindowDescription.name, 
                                            curRenderWindowDescription.width, 
                                            curRenderWindowDescription.height, 
                                            curRenderWindowDescription.useFullScreen, 
                                            &curRenderWindowDescription.miscParams);
            
			createdWindows.push_back(curWindow);											
		}
        
		return true;
	}

    RenderWindow* GL3PlusRenderSystem::_createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                                        bool fullScreen, const NameValuePairList *miscParams)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Window with name '" + name + "' already exists",
                        "GL3PlusRenderSystem::_createRenderWindow");
        }

		// Log a message
        StringStream ss;
        ss << "GL3PlusRenderSystem::_createRenderWindow \"" << name << "\", " <<
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
            
            if(mDriverVersion.major < 3)
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                            "Driver does not support at least OpenGL 3.0.",
                            "GL3PlusRenderSystem::_createRenderWindow");

            mDriverVersion.build = 0;

            const char* shadingLangVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
            tokens = StringUtil::split(shadingLangVersion, ". ");
            mNativeShadingLanguageVersion = (StringConverter::parseUnsignedInt(tokens[0]) * 100) + StringConverter::parseUnsignedInt(tokens[1]);

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
			// Only Copy does, but Copy means only one depth buffer...
			GL3PlusContext *windowContext = 0;
			win->getCustomAttribute( GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT, &windowContext );
			GL3PlusDepthBuffer *depthBuffer = new GL3PlusDepthBuffer( DepthBuffer::POOL_DEFAULT, this,
															windowContext, 0, 0,
															win->getWidth(), win->getHeight(),
															win->getFSAA(), 0, true );

			mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

			win->attachDepthBuffer( depthBuffer );
		}

        return win;
    }

	//---------------------------------------------------------------------
	DepthBuffer* GL3PlusRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
	{
		GL3PlusDepthBuffer *retVal = 0;

		// Only FBOs support different depth buffers, so everything
		// else creates dummy (empty) containers
		// retVal = mRTTManager->_createDepthBufferFor( renderTarget );
		GL3PlusFrameBufferObject *fbo = 0;
        renderTarget->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_FBO, &fbo);

		if( fbo )
		{
			// Presence of an FBO means the manager is an FBO Manager, that's why it's safe to downcast
			// Find best depth & stencil format suited for the RT's format
			GLuint depthFormat, stencilFormat;
			static_cast<GL3PlusFBOManager*>(mRTTManager)->getBestDepthStencil( fbo->getFormat(),
																		&depthFormat, &stencilFormat );

			GL3PlusRenderBuffer *depthBuffer = new GL3PlusRenderBuffer( depthFormat, fbo->getWidth(),
																fbo->getHeight(), fbo->getFSAA() );

			GL3PlusRenderBuffer *stencilBuffer = fbo->getFormat() != PF_DEPTH ? depthBuffer : 0;
			if( depthFormat != GL_DEPTH24_STENCIL8 && depthFormat != GL_DEPTH32F_STENCIL8 && stencilFormat != GL_NONE )
			{
				stencilBuffer = new GL3PlusRenderBuffer( stencilFormat, fbo->getWidth(),
													fbo->getHeight(), fbo->getFSAA() );
			}

			// No "custom-quality" multisample for now in GL
			retVal = new GL3PlusDepthBuffer( 0, this, mCurrentContext, depthBuffer, stencilBuffer,
										fbo->getWidth(), fbo->getHeight(), fbo->getFSAA(), 0, false );
		}

		return retVal;
	}
	//---------------------------------------------------------------------
	void GL3PlusRenderSystem::_getDepthStencilFormatFor( GLenum internalColourFormat, GLenum *depthFormat,
                                                      GLenum *stencilFormat )
	{
		mRTTManager->getBestDepthStencil( internalColourFormat, depthFormat, stencilFormat );
	}

    MultiRenderTarget* GL3PlusRenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget *retval = mRTTManager->createMultiRenderTarget(name);
        attachRenderTarget(*retval);
        return retval;
    }

    void GL3PlusRenderSystem::destroyRenderWindow(RenderWindow* pWin)
    {
		// Find it to remove from list
        RenderTargetMap::iterator i = mRenderTargets.begin();

        while (i != mRenderTargets.end())
        {
            if (i->second == pWin)
            {
				GL3PlusContext *windowContext = 0;
				pWin->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT, &windowContext);

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
						GL3PlusDepthBuffer *depthBuffer = static_cast<GL3PlusDepthBuffer*>(*itor);
						GL3PlusContext *glContext = depthBuffer->getGLContext();

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

                mRenderTargets.erase(i);
                delete pWin;
                break;
            }
        }
    }

    String GL3PlusRenderSystem::getErrorDescription(long errorNumber) const
    {
        return StringUtil::BLANK;
    }

    VertexElementType GL3PlusRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ABGR;
    }

    void GL3PlusRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
        mWorldMatrix = m;
    }

    void GL3PlusRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
        mViewMatrix = m;

        // Also mark clip planes dirty
        if (!mClipPlanes.empty())
        {
            mClipPlanesDirty = true;
        }
    }

    void GL3PlusRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
		// Nothing to do but mark clip planes dirty
        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;
    }

    void GL3PlusRenderSystem::_setPointParameters(Real size, 
                                              bool attenuationEnabled, Real constant, Real linear, Real quadratic,
                                              Real minSize, Real maxSize)
	{
		float val[4] = {1, 0, 0, 1};
		
		if(attenuationEnabled) 
		{
			// Point size is still calculated in pixels even when attenuation is
			// enabled, which is pretty awkward, since you typically want a viewport
			// independent size if you're looking for attenuation.
			// So, scale the point size up by viewport size (this is equivalent to
			// what D3D does as standard)
			size = size * mActiveViewport->getActualHeight();

			// XXX: why do I need this for results to be consistent with D3D?
			// Equations are supposedly the same once you factor in vp height
			Real correction = 0.005;
			// scaling required
			val[0] = constant;
			val[1] = linear * correction;
			val[2] = quadratic * correction;
			val[3] = 1;
			
			if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
            {
                if(gl3wIsSupported(3, 2))
                {
                    OGRE_CHECK_GL_ERROR(glEnable(GL_PROGRAM_POINT_SIZE));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
                }
            }
		}
		else
		{
			if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
            {
                if(gl3wIsSupported(3, 2))
                {
                    OGRE_CHECK_GL_ERROR(glDisable(GL_PROGRAM_POINT_SIZE));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
                }
            }
		}
		
		// no scaling required
		// GL has no disabled flag for this so just set to constant
		OGRE_CHECK_GL_ERROR(glPointSize(size));
    }

    void GL3PlusRenderSystem::_setPointSpritesEnabled(bool enabled)
	{
	}

    void GL3PlusRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
		GL3PlusTexturePtr tex = texPtr.staticCast<GL3PlusTexture>();

		if (!activateGLTextureUnit(stage))
			return;

		if (enabled)
		{
			if (!tex.isNull())
			{
				// Note used
				tex->touch();
				mTextureTypes[stage] = tex->getGL3PlusTextureTarget();

                // Store the number of mipmaps
                mTextureMipmapCount = tex->getNumMipmaps();
			}
			else
				// Assume 2D
				mTextureTypes[stage] = GL_TEXTURE_2D;

			if(!tex.isNull())
            {
				OGRE_CHECK_GL_ERROR(glBindTexture( mTextureTypes[stage], tex->getGLID() ));
            }
			else
            {
				OGRE_CHECK_GL_ERROR(glBindTexture( mTextureTypes[stage], static_cast<GL3PlusTextureManager*>(mTextureManager)->getWarningTextureID() ));
            }
		}
		else
		{
			// Bind zero texture
			OGRE_CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
		}

		activateGLTextureUnit(0);
    }

    void GL3PlusRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
		mTextureCoordIndex[stage] = index;
    }

    GLint GL3PlusRenderSystem::getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const
    {
        switch (tam)
        {
            default:
            case TextureUnitState::TAM_WRAP:
                return GL_REPEAT;
            case TextureUnitState::TAM_MIRROR:
                return GL_MIRRORED_REPEAT;
            case TextureUnitState::TAM_CLAMP:
                return GL_CLAMP_TO_EDGE;
            case TextureUnitState::TAM_BORDER:
                return GL_CLAMP_TO_BORDER;
        }
    }

    void GL3PlusRenderSystem::_setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw)
    {
		if (!activateGLTextureUnit(stage))
			return;
		OGRE_CHECK_GL_ERROR(glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u)));
		OGRE_CHECK_GL_ERROR(glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v)));
		OGRE_CHECK_GL_ERROR(glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_R, getTextureAddressingMode(uvw.w)));

        activateGLTextureUnit(0);
    }

    void GL3PlusRenderSystem::_setTextureBorderColour(size_t stage, const ColourValue& colour)
	{
		GLfloat border[4] = { colour.r, colour.g, colour.b, colour.a };
		if (activateGLTextureUnit(stage))
		{
            OGRE_CHECK_GL_ERROR(glTexParameterfv( mTextureTypes[stage], GL_TEXTURE_BORDER_COLOR, border));
            activateGLTextureUnit(0);
        }
	}

    void GL3PlusRenderSystem::_setTextureMipmapBias(size_t stage, float bias)
	{
		if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
		{
			if (activateGLTextureUnit(stage))
			{
                OGRE_CHECK_GL_ERROR(glTexParameterf(mTextureTypes[stage], GL_TEXTURE_LOD_BIAS, bias));
                activateGLTextureUnit(0);
            }
		}
	}

    GLenum GL3PlusRenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
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

	void GL3PlusRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op)
	{
		GLenum sourceBlend = getBlendMode(sourceFactor);
		GLenum destBlend = getBlendMode(destFactor);
		if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
		{
			OGRE_CHECK_GL_ERROR(glDisable(GL_BLEND));
		}
		else
		{
			OGRE_CHECK_GL_ERROR(glEnable(GL_BLEND));
			OGRE_CHECK_GL_ERROR(glBlendFunc(sourceBlend, destBlend));
		}
        
        GLint func = GL_FUNC_ADD;
		switch(op)
		{
		case SBO_ADD:
			func = GL_FUNC_ADD;
			break;
		case SBO_SUBTRACT:
			func = GL_FUNC_SUBTRACT;
			break;
		case SBO_REVERSE_SUBTRACT:
			func = GL_FUNC_REVERSE_SUBTRACT;
			break;
		case SBO_MIN:
            func = GL_MIN;
            break;
		case SBO_MAX:
            func = GL_MAX;
			break;
		}

        OGRE_CHECK_GL_ERROR(glBlendEquation(func));
	}

	void GL3PlusRenderSystem::_setSeparateSceneBlending(
        SceneBlendFactor sourceFactor, SceneBlendFactor destFactor,
        SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha,
        SceneBlendOperation op, SceneBlendOperation alphaOp )
	{
        GLenum sourceBlend = getBlendMode(sourceFactor);
        GLenum destBlend = getBlendMode(destFactor);
        GLenum sourceBlendAlpha = getBlendMode(sourceFactorAlpha);
        GLenum destBlendAlpha = getBlendMode(destFactorAlpha);
        
        if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO && 
           sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_BLEND));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_BLEND));
            OGRE_CHECK_GL_ERROR(glBlendFuncSeparate(sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha));
        }
        
        GLint func = GL_FUNC_ADD, alphaFunc = GL_FUNC_ADD;
        
        switch(op)
        {
            case SBO_ADD:
                func = GL_FUNC_ADD;
                break;
            case SBO_SUBTRACT:
                func = GL_FUNC_SUBTRACT;
                break;
            case SBO_REVERSE_SUBTRACT:
                func = GL_FUNC_REVERSE_SUBTRACT;
                break;
            case SBO_MIN:
                func = GL_MIN;
                break;
            case SBO_MAX:
                func = GL_MAX;
                break;
        }
        
        switch(alphaOp)
        {
            case SBO_ADD:
                alphaFunc = GL_FUNC_ADD;
                break;
            case SBO_SUBTRACT:
                alphaFunc = GL_FUNC_SUBTRACT;
                break;
            case SBO_REVERSE_SUBTRACT:
                alphaFunc = GL_FUNC_REVERSE_SUBTRACT;
                break;
            case SBO_MIN:
                alphaFunc = GL_MIN;
                break;
            case SBO_MAX:
                alphaFunc = GL_MAX;
                break;
        }
        
        OGRE_CHECK_GL_ERROR(glBlendEquationSeparate(func, alphaFunc));
	}

    void GL3PlusRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
		bool a2c = false;
		static bool lasta2c = false;

        if(func != CMPF_ALWAYS_PASS)
		{
			a2c = alphaToCoverage;
        }

		if (a2c != lasta2c && getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
		{
			if (a2c)
            {
				OGRE_CHECK_GL_ERROR(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
            }
			else
            {
				OGRE_CHECK_GL_ERROR(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
            }

			lasta2c = a2c;
		}
	}

    void GL3PlusRenderSystem::_setViewport(Viewport *vp)
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
            
            if (target && !target->requiresTextureFlipping())
            {
                // Convert "upper-left" corner to "lower-left"
                y = target->getHeight() - h - y;
            }
            
            OGRE_CHECK_GL_ERROR(glViewport(x, y, w, h));

			// Configure the viewport clipping
            OGRE_CHECK_GL_ERROR(glScissor(x, y, w, h));

            vp->_clearUpdatedFlag();
        }
    }

    void GL3PlusRenderSystem::_beginFrame(void)
    {
        if (!mActiveViewport)
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Cannot begin frame - no viewport selected.",
                        "GL3PlusRenderSystem::_beginFrame");

        OGRE_CHECK_GL_ERROR(glEnable(GL_SCISSOR_TEST));
    }

    void GL3PlusRenderSystem::_endFrame(void)
    {
        // Deactivate the viewport clipping.
        OGRE_CHECK_GL_ERROR(glDisable(GL_SCISSOR_TEST));

        OGRE_CHECK_GL_ERROR(glDisable(GL_DEPTH_CLAMP));

		// unbind GPU programs at end of frame
		// this is mostly to avoid holding bound programs that might get deleted
		// outside via the resource manager
		unbindGpuProgram(GPT_VERTEX_PROGRAM);
		unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
		unbindGpuProgram(GPT_GEOMETRY_PROGRAM);
        
        if(mDriverVersion.major >= 4)
        {
            unbindGpuProgram(GPT_HULL_PROGRAM);
            unbindGpuProgram(GPT_DOMAIN_PROGRAM);
            if(mDriverVersion.minor >= 3)
                unbindGpuProgram(GPT_COMPUTE_PROGRAM);
        }
    }

    void GL3PlusRenderSystem::_setCullingMode(CullingMode mode)
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
                OGRE_CHECK_GL_ERROR(glDisable(GL_CULL_FACE));
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

        OGRE_CHECK_GL_ERROR(glEnable(GL_CULL_FACE));
        OGRE_CHECK_GL_ERROR(glCullFace(cullMode));
    }

    void GL3PlusRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);
    }

    void GL3PlusRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            OGRE_CHECK_GL_ERROR(glClearDepth(1.0));
            OGRE_CHECK_GL_ERROR(glEnable(GL_DEPTH_TEST));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_DEPTH_TEST));
        }
    }

    void GL3PlusRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
        OGRE_CHECK_GL_ERROR(glDepthMask(flag));

        // Store for reference in _beginFrame
        mDepthWrite = enabled;
    }

    void GL3PlusRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        OGRE_CHECK_GL_ERROR(glDepthFunc(convertCompareFunction(func)));
    }

    void GL3PlusRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        if (constantBias != 0 || slopeScaleBias != 0)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_POLYGON_OFFSET_FILL));
			OGRE_CHECK_GL_ERROR(glEnable(GL_POLYGON_OFFSET_POINT));
			OGRE_CHECK_GL_ERROR(glEnable(GL_POLYGON_OFFSET_LINE));
            OGRE_CHECK_GL_ERROR(glPolygonOffset(-slopeScaleBias, -constantBias));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_POLYGON_OFFSET_FILL));
			OGRE_CHECK_GL_ERROR(glDisable(GL_POLYGON_OFFSET_POINT));
			OGRE_CHECK_GL_ERROR(glDisable(GL_POLYGON_OFFSET_LINE));
        }
    }

    void GL3PlusRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        OGRE_CHECK_GL_ERROR(glColorMask(red, green, blue, alpha));

        // record this
        mColourWrite[0] = red;
        mColourWrite[1] = blue;
        mColourWrite[2] = green;
        mColourWrite[3] = alpha;
    }

    void GL3PlusRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
    }

    void GL3PlusRenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
                                                  Matrix4& dest,
                                                  bool forGpuProgram)
    {
		// no any conversion request for OpenGL
        dest = matrix;
    }

    void GL3PlusRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect,
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

    void GL3PlusRenderSystem::_makeProjectionMatrix(Real left, Real right,
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

    void GL3PlusRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect,
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

    void GL3PlusRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix,
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

    HardwareOcclusionQuery* GL3PlusRenderSystem::createHardwareOcclusionQuery(void)
	{
		GL3PlusHardwareOcclusionQuery* ret = new GL3PlusHardwareOcclusionQuery(); 
		mHwOcclusionQueries.push_back(ret);
		return ret;
	}

    void GL3PlusRenderSystem::_setPolygonMode(PolygonMode level)
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

    void GL3PlusRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_STENCIL_TEST));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_STENCIL_TEST));
        }
    }

    void GL3PlusRenderSystem::setStencilBufferParams(CompareFunction func,
                                                     uint32 refValue, uint32 compareMask, uint32 writeMask,
                                                     StencilOperation stencilFailOp,
                                                     StencilOperation depthFailOp,
                                                     StencilOperation passOp,
                                                     bool twoSidedOperation)
    {
		bool flip;
		mStencilWriteMask = writeMask;

		if (twoSidedOperation)
		{
			if (!mCurrentCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported",
                            "GL3PlusRenderSystem::setStencilBufferParams");

			// NB: We should always treat CCW as front face for consistent with default
			// culling mode. Therefore, we must take care with two-sided stencil settings.
			flip = (mInvertVertexWinding && !mActiveRenderTarget->requiresTextureFlipping()) ||
            (!mInvertVertexWinding && mActiveRenderTarget->requiresTextureFlipping());
            // Back
            OGRE_CHECK_GL_ERROR(glStencilMaskSeparate(GL_BACK, writeMask));
            OGRE_CHECK_GL_ERROR(glStencilFuncSeparate(GL_BACK, convertCompareFunction(func), refValue, compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOpSeparate(GL_BACK,
                                convertStencilOp(stencilFailOp, !flip),
                                convertStencilOp(depthFailOp, !flip),
                                convertStencilOp(passOp, !flip)));

            // Front
            OGRE_CHECK_GL_ERROR(glStencilMaskSeparate(GL_FRONT, writeMask));
            OGRE_CHECK_GL_ERROR(glStencilFuncSeparate(GL_FRONT, convertCompareFunction(func), refValue, compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOpSeparate(GL_FRONT,
                                convertStencilOp(stencilFailOp, flip),
                                convertStencilOp(depthFailOp, flip),
                                convertStencilOp(passOp, flip)));
		}
		else
		{
			flip = false;
			OGRE_CHECK_GL_ERROR(glStencilMask(writeMask));
			OGRE_CHECK_GL_ERROR(glStencilFunc(convertCompareFunction(func), refValue, compareMask));
			OGRE_CHECK_GL_ERROR(glStencilOp(
                        convertStencilOp(stencilFailOp, flip),
                        convertStencilOp(depthFailOp, flip),
                        convertStencilOp(passOp, flip)));
		}
    }

    GLint GL3PlusRenderSystem::getCombinedMinMipFilter(void) const
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

    void GL3PlusRenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions fo)
    {
		if (!activateGLTextureUnit(unit))
			return;

        switch (ftype)
        {
            case FT_MIN:
                mMinFilter = fo;

                // Combine with existing mip filter
                OGRE_CHECK_GL_ERROR(glTexParameteri(mTextureTypes[unit],
                                GL_TEXTURE_MIN_FILTER,
                                getCombinedMinMipFilter()));
                break;

            case FT_MAG:
                switch (fo)
                {
                    case FO_ANISOTROPIC: // GL treats linear and aniso the same
                    case FO_LINEAR:
                        OGRE_CHECK_GL_ERROR(glTexParameteri(mTextureTypes[unit],
                                        GL_TEXTURE_MAG_FILTER,
                                        GL_LINEAR));
                        break;
                    case FO_POINT:
                    case FO_NONE:
                        OGRE_CHECK_GL_ERROR(glTexParameteri(mTextureTypes[unit],
                                        GL_TEXTURE_MAG_FILTER,
                                        GL_NEAREST));
                        break;
                }
                break;
            case FT_MIP:
                mMipFilter = fo;

                // Combine with existing min filter
                OGRE_CHECK_GL_ERROR(glTexParameteri(mTextureTypes[unit],
                                GL_TEXTURE_MIN_FILTER,
                                getCombinedMinMipFilter()));
                break;
        }

		activateGLTextureUnit(0);
    }

    GLfloat GL3PlusRenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		GLfloat curAniso = 0;
		OGRE_CHECK_GL_ERROR(glGetTexParameterfv(mTextureTypes[unit],
                                                GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso));

		return curAniso ? curAniso : 1;
	}

	void GL3PlusRenderSystem::_setTextureUnitCompareFunction(size_t unit, CompareFunction function)
	{
        // TODO: Sampler objects, GL 3.3 or GL_ARB_sampler_objects required. For example:
//        OGRE_CHECK_GL_ERROR(glSamplerParameteri(m_rt_ss, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
//        OGRE_CHECK_GL_ERROR(glSamplerParameteri(m_rt_ss, GL_TEXTURE_COMPARE_FUNC, GL_NEVER));
	}

	void GL3PlusRenderSystem::_setTextureUnitCompareEnabled(size_t unit, bool compare)
	{
        // TODO: GL 3.3 or later or GL_ARB_sampler_objects
        mTextureCompareEnabled = compare;
	}

    void GL3PlusRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
    {
		if (!mCurrentCapabilities->hasCapability(RSC_ANISOTROPY))
			return;

		if (!activateGLTextureUnit(unit))
			return;
        
		GLfloat largest_supported_anisotropy = 0;
		OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy));

		if (maxAnisotropy > largest_supported_anisotropy)
			maxAnisotropy = largest_supported_anisotropy ? 
			static_cast<uint>(largest_supported_anisotropy) : 1;
		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			OGRE_CHECK_GL_ERROR(glTexParameterf(mTextureTypes[unit], GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy));

		activateGLTextureUnit(0);
    }

    void GL3PlusRenderSystem::_render(const RenderOperation& op)
    {
        // Call super class
        RenderSystem::_render(op);

        HardwareVertexBufferSharedPtr globalInstanceVertexBuffer = getGlobalInstanceVertexBuffer();
        VertexDeclaration* globalVertexDeclaration = getGlobalInstanceVertexBufferVertexDeclaration();
        bool hasInstanceData = (op.useGlobalInstancingVertexBufferIsAvailable &&
            !globalInstanceVertexBuffer.isNull() && (globalVertexDeclaration != NULL)) 
            || op.vertexData->vertexBufferBinding->hasInstanceData();
        
		size_t numberOfInstances = op.numberOfInstances;
        
        if (op.useGlobalInstancingVertexBufferIsAvailable)
        {
            numberOfInstances *= getGlobalNumberOfInstances();
        }

        const VertexDeclaration::VertexElementList& decl =
            op.vertexData->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator elemIter, elemEnd;
        elemEnd = decl.end();

        bool updateVAO = true;
        if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
        {
            GLSLProgramPipeline* programPipeline =
                GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
            if(programPipeline)
            {
                updateVAO = !programPipeline->getVertexArrayObject()->isInitialised();

                // Bind VAO
                programPipeline->getVertexArrayObject()->bind();
            }
        }
        else
        {
            GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
            if(linkProgram)
            {
                updateVAO = !linkProgram->getVertexArrayObject()->isInitialised();

                // Bind VAO
                linkProgram->getVertexArrayObject()->bind();
            }
        }

        for (elemIter = decl.begin(); elemIter != elemEnd; ++elemIter)
        {
            const VertexElement & elem = *elemIter;
            size_t source = elem.getSource();

            if (!op.vertexData->vertexBufferBinding->isBufferBound(source))
                continue; // skip unbound elements

            HardwareVertexBufferSharedPtr vertexBuffer = 
                op.vertexData->vertexBufferBinding->getBuffer(source);
            
            bindVertexElementToGpu(elem, vertexBuffer, op.vertexData->vertexStart, 
                                   mRenderAttribsBound, mRenderInstanceAttribsBound, updateVAO);
        }
        
        if( !globalInstanceVertexBuffer.isNull() && globalVertexDeclaration != NULL )
        {
            elemEnd = globalVertexDeclaration->getElements().end();
            for (elemIter = globalVertexDeclaration->getElements().begin(); elemIter != elemEnd; ++elemIter)
            {
                const VertexElement & elem = *elemIter;
                bindVertexElementToGpu(elem, globalInstanceVertexBuffer, 0, 
                                       mRenderAttribsBound, mRenderInstanceAttribsBound, updateVAO);
            }
        }

        activateGLTextureUnit(0);

        // Find the correct type to render
        GLint primType;
		// Use adjacency if there is a geometry program and it requested adjacency info
		bool useAdjacency = (mGeometryProgramBound && mCurrentGeometryProgram && mCurrentGeometryProgram->isAdjacencyInfoRequired());
        switch (op.operationType)
        {
            case RenderOperation::OT_POINT_LIST:
                primType = GL_POINTS;
                break;
            case RenderOperation::OT_LINE_LIST:
                primType = useAdjacency ? GL_LINES_ADJACENCY : GL_LINES;
                break;
            case RenderOperation::OT_LINE_STRIP:
                primType = useAdjacency ? GL_LINE_STRIP_ADJACENCY : GL_LINE_STRIP;
                break;
            default:
            case RenderOperation::OT_TRIANGLE_LIST:
                primType = useAdjacency ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
                break;
            case RenderOperation::OT_TRIANGLE_STRIP:
                primType = useAdjacency ? GL_TRIANGLE_STRIP_ADJACENCY : GL_TRIANGLE_STRIP;
                break;
            case RenderOperation::OT_TRIANGLE_FAN:
                primType = GL_TRIANGLE_FAN;
                break;
        }

        // TODO: Bind atomic counter buffers here

        // Do tessellation rendering. Note: Only evaluation(domain) shaders are required.
        if(mCurrentDomainProgram)
		{
            GLuint primCount = 0;
			// Useful primitives for tessellation
			switch( op.operationType )
			{
                case RenderOperation::OT_LINE_LIST:
                    primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
                    break;

                case RenderOperation::OT_LINE_STRIP:
                    primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
                    break;

                case RenderOperation::OT_TRIANGLE_LIST:
                    primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
                    break;

                case RenderOperation::OT_TRIANGLE_STRIP:
                    primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                    break;
                default:
                    break;
			}

            // These are set via shader in DX11, SV_InsideTessFactor and SV_OutsideTessFactor
            // Hardcoding for the sample
            float patchLevel(16.f);
            OGRE_CHECK_GL_ERROR(glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &patchLevel));
            OGRE_CHECK_GL_ERROR(glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &patchLevel));
            OGRE_CHECK_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, op.vertexData->vertexCount));

            if(op.useIndexes)
            {
                OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                                 static_cast<GL3PlusHardwareIndexBuffer*>(op.indexData->indexBuffer.get())->getGLBufferId()));
                void *pBufferData = GL_BUFFER_OFFSET(op.indexData->indexStart *
                                                     op.indexData->indexBuffer->getIndexSize());
                GLuint indexEnd = op.indexData->indexCount - op.indexData->indexStart;
                GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
                OGRE_CHECK_GL_ERROR(glDrawRangeElements(GL_PATCHES, op.indexData->indexStart, indexEnd, op.indexData->indexCount, indexType, pBufferData));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glDrawArrays(GL_PATCHES, 0, primCount));
            }
		}
        else if (op.useIndexes)
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                             static_cast<GL3PlusHardwareIndexBuffer*>(op.indexData->indexBuffer.get())->getGLBufferId()));

            void *pBufferData = GL_BUFFER_OFFSET(op.indexData->indexStart *
                                            op.indexData->indexBuffer->getIndexSize());

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

                GLuint indexEnd = op.indexData->indexCount - op.indexData->indexStart;
				if(hasInstanceData)
				{
                    if(mGLSupport->checkExtension("GL_ARB_draw_elements_base_vertex") || gl3wIsSupported(3, 2))
                    {
                        OGRE_CHECK_GL_ERROR(glDrawElementsInstancedBaseVertex(primType, op.indexData->indexCount, indexType, pBufferData, numberOfInstances, op.vertexData->vertexStart));
                    }
                    else
                    {
                        OGRE_CHECK_GL_ERROR(glDrawElementsInstanced(primType, op.indexData->indexCount, indexType, pBufferData, numberOfInstances));
                    }
				}
				else
				{
                    if(mGLSupport->checkExtension("GL_ARB_draw_elements_base_vertex") || gl3wIsSupported(3, 2))
                    {
                        OGRE_CHECK_GL_ERROR(glDrawRangeElementsBaseVertex((_getPolygonMode() == GL_FILL) ? primType : _getPolygonMode(), op.indexData->indexStart, indexEnd, op.indexData->indexCount, indexType, pBufferData, op.vertexData->vertexStart));
                    }
                    else
                    {
                        OGRE_CHECK_GL_ERROR(glDrawRangeElements((_getPolygonMode() == GL_FILL) ? primType : _getPolygonMode(), op.indexData->indexStart, indexEnd, op.indexData->indexCount, indexType, pBufferData));
                    }
				}
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
                if(hasInstanceData)
				{
					OGRE_CHECK_GL_ERROR(glDrawArraysInstanced(primType, 0, op.vertexData->vertexCount, numberOfInstances));
				}
				else
				{
                    GLenum mode = (_getPolygonMode() == GL_FILL) ? primType : _getPolygonMode();
					OGRE_CHECK_GL_ERROR(glDrawArrays(mode, 0, op.vertexData->vertexCount));
				}
            } while (updatePassIterationRenderState());
        }

        if (updateVAO)
        {
            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                GLSLProgramPipeline* programPipeline =
                    GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                if(programPipeline)
                {
                    programPipeline->getVertexArrayObject()->setInitialised(true);
                }
            }
            else
            {
                GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
                if(linkProgram)
                {
                    linkProgram->getVertexArrayObject()->setInitialised(true);
                }
            }

            // Unbind the vertex array object.  Marks the end of what state will be included.
            OGRE_CHECK_GL_ERROR(glBindVertexArray(0));
        }

        mRenderAttribsBound.clear();
        mRenderInstanceAttribsBound.clear();
    }

    void GL3PlusRenderSystem::setScissorTest(bool enabled, size_t left,
                                        size_t top, size_t right,
                                        size_t bottom)
    {
        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();
        //  GL measures from the bottom, not the top
        size_t targetHeight = mActiveRenderTarget->getHeight();
        // Calculate the "lower-left" corner of the viewport
        uint64 x = 0, y = 0, w = 0, h = 0;

        if (enabled)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_SCISSOR_TEST));
            // NB GL uses width / height rather than right / bottom
            x = left;
            if (flipping)
                y = top;
            else
                y = targetHeight - bottom;
            w = right - left;
            h = bottom - top;
            OGRE_CHECK_GL_ERROR(glScissor(static_cast<GLsizei>(x),
                                          static_cast<GLsizei>(y),
                                          static_cast<GLsizei>(w),
                                          static_cast<GLsizei>(h)));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_SCISSOR_TEST));
            // GL requires you to reset the scissor when disabling
            w = mActiveViewport->getActualWidth();
            h = mActiveViewport->getActualHeight();
            x = mActiveViewport->getActualLeft();
            if (flipping)
                y = mActiveViewport->getActualTop();
            else
                y = targetHeight - mActiveViewport->getActualTop() - h;
            OGRE_CHECK_GL_ERROR(glScissor(static_cast<GLsizei>(x),
                                          static_cast<GLsizei>(y),
                                          static_cast<GLsizei>(w),
                                          static_cast<GLsizei>(h)));
        }
    }

    void GL3PlusRenderSystem::clearFrameBuffer(unsigned int buffers,
                                            const ColourValue& colour,
                                            Real depth, unsigned short stencil)
    {
        bool colourMask = !mColourWrite[0] || !mColourWrite[1] ||
                          !mColourWrite[2] || !mColourWrite[3];

        GLbitfield flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= GL_COLOR_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            if (colourMask)
            {
                OGRE_CHECK_GL_ERROR(glColorMask(true, true, true, true));
            }
            OGRE_CHECK_GL_ERROR(glClearColor(colour.r, colour.g, colour.b, colour.a));
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            if (!mDepthWrite)
            {
                OGRE_CHECK_GL_ERROR(glDepthMask(GL_TRUE));
            }
            OGRE_CHECK_GL_ERROR(glClearDepth(depth));
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
			// Enable buffer for writing if it isn't
            OGRE_CHECK_GL_ERROR(glStencilMask(0xFFFFFFFF));
            OGRE_CHECK_GL_ERROR(glClearStencil(stencil));
        }

		// Should be enable scissor test due the clear region is
		// relied on scissor box bounds.
        GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
        if (!scissorTestEnabled)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_SCISSOR_TEST));
        }

		// Sets the scissor box as same as viewport
        GLint viewport[4], scissor[4];
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_VIEWPORT, viewport));
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_SCISSOR_BOX, scissor));
        bool scissorBoxDifference =
            viewport[0] != scissor[0] || viewport[1] != scissor[1] ||
            viewport[2] != scissor[2] || viewport[3] != scissor[3];
        if (scissorBoxDifference)
        {
            OGRE_CHECK_GL_ERROR(glScissor(viewport[0], viewport[1], viewport[2], viewport[3]));
        }

		// Clear buffers
        OGRE_CHECK_GL_ERROR(glClear(flags));

        // Restore scissor box
        if (scissorBoxDifference)
        {
            OGRE_CHECK_GL_ERROR(glScissor(scissor[0], scissor[1], scissor[2], scissor[3]));
        }

        // Restore scissor test
        if (!scissorTestEnabled)
        {
            OGRE_CHECK_GL_ERROR(glDisable(GL_SCISSOR_TEST));
        }

        // Reset buffer write state
        if (!mDepthWrite && (buffers & FBT_DEPTH))
        {
            OGRE_CHECK_GL_ERROR(glDepthMask(GL_FALSE));
        }

        if (colourMask && (buffers & FBT_COLOUR))
        {
            OGRE_CHECK_GL_ERROR(glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]));
        }

        if (buffers & FBT_STENCIL)
        {
            OGRE_CHECK_GL_ERROR(glStencilMask(mStencilWriteMask));
        }
    }

    void GL3PlusRenderSystem::_switchContext(GL3PlusContext *context)
    {
		// Unbind GPU programs and rebind to new context later, because
		// scene manager treat render system as ONE 'context' ONLY, and it
		// cached the GPU programs using state.
		if (mCurrentVertexProgram)
			mCurrentVertexProgram->unbindProgram();
		if (mCurrentGeometryProgram)
			mCurrentGeometryProgram->unbindProgram();
		if (mCurrentFragmentProgram)
			mCurrentFragmentProgram->unbindProgram();
        
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

        // Rebind GPU programs to new context
		if (mCurrentVertexProgram)
			mCurrentVertexProgram->bindProgram();
		if (mCurrentGeometryProgram)
			mCurrentGeometryProgram->bindProgram();
		if (mCurrentFragmentProgram)
			mCurrentFragmentProgram->bindProgram();
        
        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        OGRE_CHECK_GL_ERROR(glDepthMask(mDepthWrite));
        OGRE_CHECK_GL_ERROR(glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]));
        OGRE_CHECK_GL_ERROR(glStencilMask(mStencilWriteMask));
    }

    void GL3PlusRenderSystem::_unregisterContext(GL3PlusContext *context)
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

    void GL3PlusRenderSystem::_oneTimeContextInitialization()
    {
		OGRE_CHECK_GL_ERROR(glDisable(GL_DITHER));

        // Check for FSAA
		// Enable the extension if it was enabled by the GL3PlusSupport
        int fsaa_active = false;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_SAMPLE_BUFFERS, (GLint*)&fsaa_active));
        if(fsaa_active)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
            LogManager::getSingleton().logMessage("Using FSAA.");
        }
        
        if (mGLSupport->checkExtension("GL_ARB_seamless_cube_map") || gl3wIsSupported(3, 2))
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            // Some Apple NVIDIA hardware can't handle seamless cubemaps
            if (mCurrentCapabilities->getVendor() != GPU_NVIDIA)
#endif
            // Enable seamless cube maps
            OGRE_CHECK_GL_ERROR(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
        }

        if (mGLSupport->checkExtension("GL_ARB_provoking_vertex") || gl3wIsSupported(3, 2))
        {
            // Set provoking vertex convention
            OGRE_CHECK_GL_ERROR(glProvokingVertex(GL_FIRST_VERTEX_CONVENTION));
        }

        if (mGLSupport->checkExtension("GL_KHR_debug") || gl3wIsSupported(4, 3))
        {
#if OGRE_DEBUG_MODE
            OGRE_CHECK_GL_ERROR(glEnable(GL_DEBUG_OUTPUT));
            OGRE_CHECK_GL_ERROR(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
            OGRE_CHECK_GL_ERROR(glDebugMessageCallbackARB(&GLDebugCallback, NULL));
            OGRE_CHECK_GL_ERROR(glDebugMessageControlARB(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, GL_TRUE));
#endif
        }
    }

    void GL3PlusRenderSystem::initialiseContext(RenderWindow* primary)
    {
		// Set main and current context
        mMainContext = 0;
        primary->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT, &mMainContext);
        mCurrentContext = mMainContext;

		// Set primary context as active
        if (mCurrentContext)
            mCurrentContext->setCurrent();

        // Initialise GL3W
        if (gl3wInit())
            LogManager::getSingleton().logMessage("Failed to initialize GL3W", LML_CRITICAL);

        // Make sure that OpenGL 3.0+ is supported in this context
        if (!gl3wIsSupported(3, 0))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "OpenGL 3.0 is not supported",
                        "GL3PlusRenderSystem::initialiseContext");
        }

		// Setup GL3PlusSupport
        mGLSupport->initialiseExtensions();

        LogManager::getSingleton().logMessage("**************************************");
        LogManager::getSingleton().logMessage("***   OpenGL 3+ Renderer Started   ***");
        LogManager::getSingleton().logMessage("***          EXPERIMENTAL          ***");
        LogManager::getSingleton().logMessage("**************************************");
    }

    void GL3PlusRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        // Unbind frame buffer object
        if(mActiveRenderTarget)
            mRTTManager->unbind(mActiveRenderTarget);

        mActiveRenderTarget = target;
		if (target)
		{
			// Switch context if different from current one
			GL3PlusContext *newContext = 0;
			target->getCustomAttribute(GL3PlusRenderTexture::CustomAttributeString_GLCONTEXT, &newContext);
			if (newContext && mCurrentContext != newContext)
			{
				_switchContext(newContext);
			}

			// Check the FBO's depth buffer status
			GL3PlusDepthBuffer *depthBuffer = static_cast<GL3PlusDepthBuffer*>(target->getDepthBuffer());

			if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
				(!depthBuffer || depthBuffer->getGLContext() != mCurrentContext ) )
			{
				// Depth is automatically managed and there is no depth buffer attached to this RT
				// or the Current context doesn't match the one this Depth buffer was created with
				setDepthBufferFor( target );
			}

			// Bind frame buffer object
			mRTTManager->bind(target);

            // Enable / disable sRGB states
            if (target->isHardwareGammaEnabled())
            {
                OGRE_CHECK_GL_ERROR(glEnable(GL_FRAMEBUFFER_SRGB));
                
                // Note: could test GL_FRAMEBUFFER_SRGB_CAPABLE here before
                // enabling, but GL spec says incapable surfaces ignore the setting
                // anyway. We test the capability to enable isHardwareGammaEnabled.
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glDisable(GL_FRAMEBUFFER_SRGB));
            }
        }
    }

    GLint GL3PlusRenderSystem::convertCompareFunction(CompareFunction func) const
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

    GLint GL3PlusRenderSystem::convertStencilOp(StencilOperation op, bool invert) const
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
    void GL3PlusRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
		GL3PlusGpuProgram* glprg = static_cast<GL3PlusGpuProgram*>(prg);
        
		// Unbind previous gpu program first.
		//
		// Note:
		//  1. Even if both previous and current are the same object, we can't
		//     bypass re-bind completely since the object itself may be modified.
		//     But we can bypass unbind based on the assumption that object
		//     internally GL program type shouldn't be changed after it has
		//     been created. The behavior of bind to a GL program type twice
		//     should be same as unbind and rebind that GL program type, even
		//     for different objects.
		//  2. We also assumed that the program's type (vertex or fragment) should
		//     not be changed during it's in using. If not, the following switch
		//     statement will confuse GL state completely, and we can't fix it
		//     here. To fix this case, we must coding the program implementation
		//     itself, if type is changing (during load/unload, etc), and it's in use,
		//     unbind and notify render system to correct for its state.
		//
		switch (glprg->getType())
		{
            case GPT_VERTEX_PROGRAM:
                if (mCurrentVertexProgram != glprg)
                {
                    if (mCurrentVertexProgram)
                        mCurrentVertexProgram->unbindProgram();
                    mCurrentVertexProgram = glprg;
                }
                break;
                
            case GPT_FRAGMENT_PROGRAM:
                if (mCurrentFragmentProgram != glprg)
                {
                    if (mCurrentFragmentProgram)
                        mCurrentFragmentProgram->unbindProgram();
                    mCurrentFragmentProgram = glprg;
                }
                break;
            case GPT_GEOMETRY_PROGRAM:
                if (mCurrentGeometryProgram != glprg)
                {
                    if (mCurrentGeometryProgram)
                        mCurrentGeometryProgram->unbindProgram();
                    mCurrentGeometryProgram = glprg;
                }
                break;
            case GPT_HULL_PROGRAM:
                if (mCurrentHullProgram != glprg)
                {
                    if (mCurrentHullProgram)
                        mCurrentHullProgram->unbindProgram();
                    mCurrentHullProgram = glprg;
                }
                break;
            case GPT_DOMAIN_PROGRAM:
                if (mCurrentDomainProgram != glprg)
                {
                    if (mCurrentDomainProgram)
                        mCurrentDomainProgram->unbindProgram();
                    mCurrentDomainProgram = glprg;
                }
                break;
            case GPT_COMPUTE_PROGRAM:
                if (mCurrentComputeProgram != glprg)
                {
                    if (mCurrentComputeProgram )
                        mCurrentComputeProgram ->unbindProgram();
                    mCurrentComputeProgram  = glprg;
                }
                break;
            default:
                break;
		}
        
		// Bind the program
		glprg->bindProgram();

		RenderSystem::bindGpuProgram(prg);
    }

    void GL3PlusRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
		if (gptype == GPT_VERTEX_PROGRAM && mCurrentVertexProgram)
		{
			mActiveVertexGpuProgramParameters.setNull();
			mCurrentVertexProgram->unbindProgram();
			mCurrentVertexProgram = 0;
		}
		else if (gptype == GPT_GEOMETRY_PROGRAM && mCurrentGeometryProgram)
		{
			mActiveGeometryGpuProgramParameters.setNull();
			mCurrentGeometryProgram->unbindProgram();
			mCurrentGeometryProgram = 0;
		}
		else if (gptype == GPT_FRAGMENT_PROGRAM && mCurrentFragmentProgram)
		{
			mActiveFragmentGpuProgramParameters.setNull();
			mCurrentFragmentProgram->unbindProgram();
			mCurrentFragmentProgram = 0;
		}
		else if (gptype == GPT_HULL_PROGRAM && mCurrentHullProgram)
		{
			mActiveTesselationHullGpuProgramParameters.setNull();
			mCurrentHullProgram->unbindProgram();
			mCurrentHullProgram = 0;
		}
		else if (gptype == GPT_DOMAIN_PROGRAM && mCurrentDomainProgram)
		{
			mActiveTesselationDomainGpuProgramParameters.setNull();
			mCurrentDomainProgram->unbindProgram();
			mCurrentDomainProgram = 0;
		}
		else if (gptype == GPT_COMPUTE_PROGRAM && mCurrentComputeProgram)
		{
			mActiveComputeGpuProgramParameters.setNull();
			mCurrentComputeProgram->unbindProgram();
			mCurrentComputeProgram = 0;
		}
		RenderSystem::unbindGpuProgram(gptype);
    }

    void GL3PlusRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask)
    {
//		if (mask & (uint16)GPV_GLOBAL)
//		{
			// We could maybe use GL_EXT_bindable_uniform here to produce Dx10-style
			// shared constant buffers, but GPU support seems fairly weak?
			// check the match to constant buffers & use rendersystem data hooks to store
			// for now, just copy
			params->_copySharedParams();

            switch (gptype)
            {
                case GPT_VERTEX_PROGRAM:
                    mActiveVertexGpuProgramParameters = params;
                    mCurrentVertexProgram->bindProgramSharedParameters(params, mask);
                    break;
                case GPT_FRAGMENT_PROGRAM:
                    mActiveFragmentGpuProgramParameters = params;
                    mCurrentFragmentProgram->bindProgramSharedParameters(params, mask);
                    break;
                case GPT_GEOMETRY_PROGRAM:
                    mActiveGeometryGpuProgramParameters = params;
                    mCurrentGeometryProgram->bindProgramSharedParameters(params, mask);
                    break;
                case GPT_HULL_PROGRAM:
                    mActiveTesselationHullGpuProgramParameters = params;
                    mCurrentHullProgram->bindProgramSharedParameters(params, mask);
                    break;
                case GPT_DOMAIN_PROGRAM:
                    mActiveTesselationDomainGpuProgramParameters = params;
                    mCurrentDomainProgram->bindProgramSharedParameters(params, mask);
                    break;
                case GPT_COMPUTE_PROGRAM:
                    mActiveComputeGpuProgramParameters = params;
                    mCurrentComputeProgram->bindProgramSharedParameters(params, mask);
                    break;
                default:
                    break;
            }
//		}
//        else
//        {
            switch (gptype)
            {
                case GPT_VERTEX_PROGRAM:
                    mActiveVertexGpuProgramParameters = params;
                    mCurrentVertexProgram->bindProgramParameters(params, mask);
                    break;
                case GPT_FRAGMENT_PROGRAM:
                    mActiveFragmentGpuProgramParameters = params;
                    mCurrentFragmentProgram->bindProgramParameters(params, mask);
                    break;
                case GPT_GEOMETRY_PROGRAM:
                    mActiveGeometryGpuProgramParameters = params;
                    mCurrentGeometryProgram->bindProgramParameters(params, mask);
                    break;
                case GPT_HULL_PROGRAM:
                    mActiveTesselationHullGpuProgramParameters = params;
                    mCurrentHullProgram->bindProgramParameters(params, mask);
                    break;
                case GPT_DOMAIN_PROGRAM:
                    mActiveTesselationDomainGpuProgramParameters = params;
                    mCurrentDomainProgram->bindProgramParameters(params, mask);
                    break;
                case GPT_COMPUTE_PROGRAM:
                    mActiveComputeGpuProgramParameters = params;
                    mCurrentComputeProgram->bindProgramParameters(params, mask);
                    break;
                default:
                    break;
            }
//        }
    }

    void GL3PlusRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {
		switch (gptype)
		{
            case GPT_VERTEX_PROGRAM:
                mCurrentVertexProgram->bindProgramPassIterationParameters(mActiveVertexGpuProgramParameters);
                break;
            case GPT_FRAGMENT_PROGRAM:
                mCurrentFragmentProgram->bindProgramPassIterationParameters(mActiveFragmentGpuProgramParameters);
                break;
            case GPT_GEOMETRY_PROGRAM:
                mCurrentGeometryProgram->bindProgramPassIterationParameters(mActiveGeometryGpuProgramParameters);
                break;
            case GPT_HULL_PROGRAM:
                mCurrentHullProgram->bindProgramPassIterationParameters(mActiveTesselationHullGpuProgramParameters);
                break;
            case GPT_DOMAIN_PROGRAM:
                mCurrentDomainProgram->bindProgramPassIterationParameters(mActiveTesselationDomainGpuProgramParameters);
                break;
            case GPT_COMPUTE_PROGRAM:
                mCurrentComputeProgram->bindProgramPassIterationParameters(mActiveComputeGpuProgramParameters);
                break;
             default:
                break;
		}
    }

    void GL3PlusRenderSystem::setClipPlanesImpl(const Ogre::PlaneList& planeList)
    {
        OGRE_CHECK_GL_ERROR(glEnable(GL_DEPTH_CLAMP));
    }

	void GL3PlusRenderSystem::registerThread()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex);
		// This is only valid once we've created the main context
		if (!mMainContext)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "Cannot register a background thread before the main context "
                        "has been created.", 
                        "GL3PlusRenderSystem::registerThread");
		}
        
		// Create a new context for this thread. Cloning from the main context
		// will ensure that resources are shared with the main context
		// We want a separate context so that we can safely create GL
		// objects in parallel with the main thread
		GL3PlusContext* newContext = mMainContext->clone();
		mBackgroundContextList.push_back(newContext);
        
		// Bind this new context to this thread. 
		newContext->setCurrent();
        
		_oneTimeContextInitialization();
		newContext->setInitialized();
	}

	void GL3PlusRenderSystem::unregisterThread()
	{
		// nothing to do here?
		// Don't need to worry about active context, just make sure we delete
		// on shutdown.
	}

	void GL3PlusRenderSystem::preExtraThreadsStarted()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex);
		// free context, we'll need this to share lists
        if(mCurrentContext)
		mCurrentContext->endCurrent();
	}

	void GL3PlusRenderSystem::postExtraThreadsStarted()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex);
		// reacquire context
        if(mCurrentContext)
		mCurrentContext->setCurrent();
	}

	unsigned int GL3PlusRenderSystem::getDisplayMonitorCount() const
	{
		return mGLSupport->getDisplayMonitorCount();
	}

    //---------------------------------------------------------------------
    void GL3PlusRenderSystem::beginProfileEvent( const String &eventName )
    {
        markProfileEvent("Begin Event: " + eventName);
        if (mGLSupport->checkExtension("ARB_debug_group") || gl3wIsSupported(4, 3))
            OGRE_CHECK_GL_ERROR(glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, static_cast<GLint>(eventName.length()), eventName.c_str()));
    }

    //---------------------------------------------------------------------
    void GL3PlusRenderSystem::endProfileEvent( void )
    {
        markProfileEvent("End Event");
        if (mGLSupport->checkExtension("ARB_debug_group") || gl3wIsSupported(4, 3))
            OGRE_CHECK_GL_ERROR(glPopDebugGroup());
    }

    //---------------------------------------------------------------------
    void GL3PlusRenderSystem::markProfileEvent( const String &eventName )
    {
        if( eventName.empty() )
            return;

        if (mGLSupport->checkExtension("GL_KHR_debug") || gl3wIsSupported(4, 3))
            glDebugMessageInsert(GL_DEBUG_SOURCE_THIRD_PARTY,
                                 GL_DEBUG_TYPE_PERFORMANCE,
                                 GL_DEBUG_SEVERITY_LOW,
                                 0,
                                 static_cast<GLint>(eventName.length()),
                                 eventName.c_str());
    }

    bool GL3PlusRenderSystem::activateGLTextureUnit(size_t unit)
	{
		if (mActiveTextureUnit != unit)
		{
			if (unit < getCapabilities()->getNumTextureUnits())
			{
				OGRE_CHECK_GL_ERROR(glActiveTexture(static_cast<uint32>(GL_TEXTURE0 + unit)));
				mActiveTextureUnit = static_cast<GLenum>(unit);
				return true;
			}
			else if (!unit)
			{
				// always ok to use the first unit
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}

    void GL3PlusRenderSystem::bindVertexElementToGpu( const VertexElement &elem,
                                                     HardwareVertexBufferSharedPtr vertexBuffer, const size_t vertexStart,
                                                     vector<GLuint>::type &attribsBound, 
                                                     vector<GLuint>::type &instanceAttribsBound,
                                                     bool updateVAO)
    {
        void* pBufferData = 0;
        const GL3PlusHardwareVertexBuffer* hwGlBuffer = static_cast<const GL3PlusHardwareVertexBuffer*>(vertexBuffer.get()); 

        // FIXME: Having this commented out fixes some rendering issues but leaves VAO's useless
//        if (updateVAO)
        {
            OGRE_CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                             hwGlBuffer->getGLBufferId()));
            pBufferData = GL_BUFFER_OFFSET(elem.getOffset());

            if (vertexStart)
            {
                pBufferData = static_cast<char*>(pBufferData) + vertexStart * vertexBuffer->getVertexSize();
            }

            VertexElementSemantic sem = elem.getSemantic();
            unsigned short typeCount = VertexElement::getTypeCount(elem.getType());
            GLboolean normalised = GL_FALSE;
            GLuint attrib = 0;
            unsigned short elemIndex = elem.getIndex();

            if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            {
                GLSLProgramPipeline* programPipeline =
                    GLSLProgramPipelineManager::getSingleton().getActiveProgramPipeline();
                if (!programPipeline || !programPipeline->isAttributeValid(sem, elemIndex))
                {
                    return;
                }

                attrib = (GLuint)programPipeline->getAttributeIndex(sem, elemIndex);
            }
            else
            {
                GLSLLinkProgram* linkProgram = GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
                if (!linkProgram || !linkProgram->isAttributeValid(sem, elemIndex))
                {
                    return;
                }

                attrib = (GLuint)linkProgram->getAttributeIndex(sem, elemIndex);
            }

            if (mCurrentVertexProgram)
            {
                if (hwGlBuffer->isInstanceData())
                {
                    OGRE_CHECK_GL_ERROR(glVertexAttribDivisor(attrib, hwGlBuffer->getInstanceDataStepRate()));
                    instanceAttribsBound.push_back(attrib);
                }
            }

            switch(elem.getType())
            {
                case VET_COLOUR:
                case VET_COLOUR_ABGR:
                case VET_COLOUR_ARGB:
                    // Because GL takes these as a sequence of single unsigned bytes, count needs to be 4
                    // VertexElement::getTypeCount treats them as 1 (RGBA)
                    // Also need to normalise the fixed-point data
                    typeCount = 4;
                    normalised = GL_TRUE;
                    break;
                default:
                    break;
            };

            switch(elem.getBaseType(elem.getType()))
            {
                default:
                case VET_FLOAT1:
                    OGRE_CHECK_GL_ERROR(glVertexAttribPointer(attrib,
                                          typeCount,
                                          GL3PlusHardwareBufferManager::getGLType(elem.getType()),
                                          normalised,
                                          static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                          pBufferData));
                    break;
                case VET_DOUBLE1:
                    OGRE_CHECK_GL_ERROR(glVertexAttribLPointer(attrib,
                                           typeCount,
                                           GL3PlusHardwareBufferManager::getGLType(elem.getType()),
                                           static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                           pBufferData));
                    break;
            }

            // If this attribute hasn't been enabled, do so and keep a record of it.
            OGRE_CHECK_GL_ERROR(glEnableVertexAttribArray(attrib));

            attribsBound.push_back(attrib);
        }
    }
}
