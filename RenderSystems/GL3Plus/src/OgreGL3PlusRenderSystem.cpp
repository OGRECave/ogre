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

#include "OgreGLUtil.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreGL3PlusTextureManager.h"
#include "OgreGL3PlusHardwareUniformBuffer.h"
#include "OgreGL3PlusHardwareVertexBuffer.h"
#include "OgreGL3PlusHardwareIndexBuffer.h"
#include "OgreGLSLShader.h"
#include "OgreGpuProgramManager.h"
#include "OgreException.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGL3PlusHardwareOcclusionQuery.h"
#include "OgreGL3PlusDepthBuffer.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGLContext.h"
#include "OgreGLSLShaderFactory.h"
#include "OgreGL3PlusFBORenderTexture.h"
#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGLSLProgramManager.h"
#include "OgreGLSLSeparableProgram.h"
#include "OgreGLVertexArrayObject.h"
#include "OgreRoot.h"
#include "OgreConfig.h"
#include "OgreViewport.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusStateCacheManager.h"
#include "OgreGLSLProgramCommon.h"
#include "OgreGL3PlusFBOMultiRenderTarget.h"
#include "OgreSPIRVShaderFactory.h"


#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
extern "C" void glFlushRenderAPPLE();
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#if ENABLE_GL_DEBUG_OUTPUT
static void APIENTRY GLDebugCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* message,
                                     const GLvoid* userParam)
{
    char debSource[32] = {0}, debType[32] = {0}, debSev[32] = {0};

    if (source == GL_DEBUG_SOURCE_API)
        strcpy(debSource, "OpenGL");
    else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
        strcpy(debSource, "Windows");
    else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
        strcpy(debSource, "Shader Compiler");
    else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
        strcpy(debSource, "Third Party");
    else if (source == GL_DEBUG_SOURCE_APPLICATION)
        strcpy(debSource, "Application");
    else if (source == GL_DEBUG_SOURCE_OTHER)
        strcpy(debSource, "Other");

    if (type == GL_DEBUG_TYPE_ERROR)
        strcpy(debType, "error");
    else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
        strcpy(debType, "deprecated behavior");
    else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
        strcpy(debType, "undefined behavior");
    else if (type == GL_DEBUG_TYPE_PORTABILITY)
        strcpy(debType, "portability");
    else if (type == GL_DEBUG_TYPE_PERFORMANCE)
        strcpy(debType, "performance");
    else if (type == GL_DEBUG_TYPE_OTHER)
        strcpy(debType, "message");

    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        strcpy(debSev, "high");
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
        strcpy(debSev, "medium");
    else if (severity == GL_DEBUG_SEVERITY_LOW)
        strcpy(debSev, "low");

    Ogre::LogManager::getSingleton().stream() << debSource << ":" << debType << "(" << debSev << ") " << id << ": " << message;
}
#endif

namespace Ogre {

    static GLNativeSupport* glsupport;
    static GL3WglProc get_proc(const char* proc) {
        return (GL3WglProc)glsupport->getProcAddress(proc);
    }

    GL3PlusRenderSystem::GL3PlusRenderSystem()
        : mDepthWrite(true),
          mScissorsEnabled(false),
          mStencilWriteMask(0xFFFFFFFF),
          mStateCacheManager(0),
          mShaderManager(0),
          mGLSLShaderFactory(0),
          mSPIRVShaderFactory(0),
          mHardwareBufferManager(0),
          mActiveTextureUnit(0)
    {
        size_t i;

        LogManager::getSingleton().logMessage(getName() + " created.");

        // Get our GLSupport
        mGLSupport = getGLSupport();
        glsupport = mGLSupport;

        initConfigOptions();

        mColourWrite[0] = mColourWrite[1] = mColourWrite[2] = mColourWrite[3] = true;

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
        mCurrentShader.fill(NULL);
        mLargestSupportedAnisotropy = 1;
        mRTTManager = NULL;
        mSeparateShaderObjectsEnabled = false;
    }

    GL3PlusRenderSystem::~GL3PlusRenderSystem()
    {
        shutdown();

        if (mGLSupport)
            OGRE_DELETE mGLSupport;
    }

    const String& GL3PlusRenderSystem::getName(void) const
    {
        static String strName("OpenGL 3+ Rendering Subsystem");
        return strName;
    }

    void GL3PlusRenderSystem::_initialise()
    {
        RenderSystem::_initialise();
        mGLSupport->start();
    }

    void GL3PlusRenderSystem::initConfigOptions()
    {
        GLRenderSystemCommon::initConfigOptions();

        ConfigOption opt;
        opt.name = "Reversed Z-Buffer";
        opt.possibleValues = {"No", "Yes"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;

        opt.name = "Separate Shader Objects";
        opt.possibleValues = {"No", "Yes"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;
    }

    RenderSystemCapabilities* GL3PlusRenderSystem::createRenderSystemCapabilities() const
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
        rsc->parseVendorFromString(mVendor);

        // Check for hardware mipmapping support.
        rsc->setCapability(RSC_AUTOMIPMAP_COMPRESSED);

        // Multitexturing support and set number of texture units
        GLint units;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units));
        rsc->setNumTextureUnits(std::min(OGRE_MAX_TEXTURE_LAYERS, units));

        glGetIntegerv( GL_MAX_VERTEX_ATTRIBS , &units);
        rsc->setNumVertexAttributes(units);

        // Check for Anisotropy support
        if (checkExtension("GL_EXT_texture_filter_anisotropic"))
        {
            GLfloat maxAnisotropy = 0;
            OGRE_CHECK_GL_ERROR(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy));
            rsc->setMaxSupportedAnisotropy(maxAnisotropy);
            rsc->setCapability(RSC_ANISOTROPY);
        }

        // DOT3 support is standard
        rsc->setCapability(RSC_DOT3);

        // Point sprites
        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);

        // Check for hardware stencil support and set bit depth
        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        rsc->setStencilBufferBitDepth(8);

        rsc->setCapability(RSC_HW_GAMMA);

        // Vertex Buffer Objects are always supported
        rsc->setCapability(RSC_MAPBUFFER);
        rsc->setCapability(RSC_32BIT_INDEX);

        // Vertex Array Objects are supported in 3.0
        rsc->setCapability(RSC_VAO);

        // Check for texture compression
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);

        // Check for dxt compression
        if (checkExtension("GL_EXT_texture_compression_s3tc"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        }

        // Check for etc compression
        if (hasMinGLVersion(4, 3) || checkExtension("GL_ARB_ES3_compatibility"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_ETC2);
        }

        // Check for vtc compression
        if (checkExtension("GL_NV_texture_compression_vtc"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_VTC);
        }

        // RGTC(BC4/BC5) is supported by the 3.0 spec
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);

        // BPTC(BC6H/BC7) is supported by the extension or OpenGL 4.2 or higher
        if (hasMinGLVersion(4, 2) || checkExtension("GL_ARB_texture_compression_bptc"))
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
        }

        if (checkExtension("WEBGL_compressed_texture_astc") ||
            checkExtension("GL_KHR_texture_compression_astc_ldr"))
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_ASTC);

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
        rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);

        // Check for non-power-of-2 texture support
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);

        // Check for SSBO support
        if (hasMinGLVersion(4, 3) || checkExtension("GL_ARB_shader_storage_buffer_object"))
            rsc->setCapability(RSC_READ_WRITE_BUFFERS);

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
        bool limitedOSXCoreProfile = OGRE_PLATFORM == OGRE_PLATFORM_APPLE && hasMinGLVersion(3, 2);

        for (uint16 ver = getNativeShadingLanguageVersion(); ver >= 400; ver -= 10)
            rsc->addShaderProfile("glsl" + StringConverter::toString(ver));

        if (getNativeShadingLanguageVersion() >= 330)
            rsc->addShaderProfile("glsl330");
        if (getNativeShadingLanguageVersion() >= 150)
            rsc->addShaderProfile("glsl150");
        if (getNativeShadingLanguageVersion() >= 140 && !limitedOSXCoreProfile)
            rsc->addShaderProfile("glsl140");
        if (getNativeShadingLanguageVersion() >= 130 && !limitedOSXCoreProfile)
            rsc->addShaderProfile("glsl130");

        if (mSeparateShaderObjectsEnabled &&
            (hasMinGLVersion(4, 3) ||
             (checkExtension("GL_ARB_separate_shader_objects") && checkExtension("GL_ARB_program_interface_query"))))
        {
            rsc->setCapability(RSC_SEPARATE_SHADER_OBJECTS);
            rsc->setCapability(RSC_GLSL_SSO_REDECLARE);
        }

        if (checkExtension("GL_ARB_gl_spirv") && rsc->hasCapability(RSC_SEPARATE_SHADER_OBJECTS))
            rsc->addShaderProfile("spirv");

        // Mesa 11.2 does not behave according to spec and throws a "gl_Position redefined"
        if(rsc->getDeviceName().find("Mesa") != String::npos) {
            rsc->unsetCapability(RSC_GLSL_SSO_REDECLARE);
        }

        // Vertex/Fragment Programs
        rsc->setCapability(RSC_VERTEX_PROGRAM);
        rsc->setCapability(RSC_FRAGMENT_PROGRAM);

        GLint constantCount = 0;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &constantCount));
        rsc->setVertexProgramConstantFloatCount((Ogre::ushort)constantCount);
        rsc->setVertexProgramConstantBoolCount((Ogre::ushort)constantCount);
        rsc->setVertexProgramConstantIntCount((Ogre::ushort)constantCount);

        // Fragment Program Properties
        constantCount = 0;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &constantCount));
        rsc->setFragmentProgramConstantFloatCount((Ogre::ushort)constantCount);
        rsc->setFragmentProgramConstantBoolCount((Ogre::ushort)constantCount);
        rsc->setFragmentProgramConstantIntCount((Ogre::ushort)constantCount);

        // Geometry Program Properties
        rsc->setCapability(RSC_GEOMETRY_PROGRAM);

        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &constantCount));
        rsc->setGeometryProgramConstantFloatCount(constantCount);

        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &constantCount));
        rsc->setGeometryProgramNumOutputVertices(constantCount);

        //FIXME Is this correct?
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &constantCount));
        rsc->setGeometryProgramConstantFloatCount(constantCount);
        rsc->setGeometryProgramConstantBoolCount(constantCount);
        rsc->setGeometryProgramConstantIntCount(constantCount);

        // Tessellation Program Properties
        if (hasMinGLVersion(4, 0) || checkExtension("GL_ARB_tessellation_shader"))
        {
            rsc->setCapability(RSC_TESSELLATION_HULL_PROGRAM);
            rsc->setCapability(RSC_TESSELLATION_DOMAIN_PROGRAM);

            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, &constantCount));
            // 16 boolean params allowed
            rsc->setTessellationHullProgramConstantBoolCount(constantCount);
            // 16 integer params allowed, 4D
            rsc->setTessellationHullProgramConstantIntCount(constantCount);
            // float params, always 4D
            rsc->setTessellationHullProgramConstantFloatCount(constantCount);

            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, &constantCount));
            // 16 boolean params allowed
            rsc->setTessellationDomainProgramConstantBoolCount(constantCount);
            // 16 integer params allowed, 4D
            rsc->setTessellationDomainProgramConstantIntCount(constantCount);
            // float params, always 4D
            rsc->setTessellationDomainProgramConstantFloatCount(constantCount);
        }

        // Compute Program Properties
        if (hasMinGLVersion(4, 3) || checkExtension("GL_ARB_compute_shader"))
        {
            rsc->setCapability(RSC_COMPUTE_PROGRAM);

            //FIXME Is this correct?
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, &constantCount));
            rsc->setComputeProgramConstantFloatCount(constantCount);
            rsc->setComputeProgramConstantBoolCount(constantCount);
            rsc->setComputeProgramConstantIntCount(constantCount);

            //TODO we should also check max workgroup count & size
            // OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_SIZE, &workgroupCount));
            // OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroupInvocations));
        }

        if (hasMinGLVersion(4, 1) || checkExtension("GL_ARB_get_program_binary"))
        {
            GLint formats = 0;
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats));

            if (formats > 0)
                rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);
        }

        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);

        // Check for Float textures
        rsc->setCapability(RSC_TEXTURE_FLOAT);

        // OpenGL 3.0 requires a minimum of 16 texture image units
        units = std::max<GLint>(16, units);

        rsc->setNumVertexTextureUnits(static_cast<ushort>(units));
        rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);

        // Mipmap LOD biasing?
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // Alpha to coverage always 'supported' when MSAA is available
        // although card may ignore it if it doesn't specifically support A2C
        rsc->setCapability(RSC_ALPHA_TO_COVERAGE);

        // Check if render to vertex buffer (transform feedback in OpenGL)
        rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);

        if (hasMinGLVersion(4, 3) || checkExtension("GL_KHR_debug"))
            rsc->setCapability(RSC_DEBUG);

        if( hasMinGLVersion(4, 3) || checkExtension("GL_ARB_ES3_compatibility"))
            rsc->setCapability(RSC_PRIMITIVE_RESTART);

        GLfloat lineWidth[2] = {1, 1};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidth);
        if(lineWidth[1] != 1 && lineWidth[1] != lineWidth[0])
            rsc->setCapability(RSC_WIDE_LINES);

        return rsc;
    }

    void GL3PlusRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if (caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to initialize GL3PlusRenderSystem from RenderSystemCapabilities that do not support OpenGL 3+",
                        "GL3PlusRenderSystem::initialiseFromRenderSystemCapabilities");
        }

        mShaderManager = new GpuProgramManager();
        ResourceGroupManager::getSingleton()._registerResourceManager(mShaderManager->getResourceType(),
                                                                      mShaderManager);

        // Create GLSL shader factory
        mGLSLShaderFactory = new GLSLShaderFactory(this);
        HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLShaderFactory);
        mSPIRVShaderFactory = new SPIRVShaderFactory();
        HighLevelGpuProgramManager::getSingleton().addFactory(mSPIRVShaderFactory);

        // Use VBO's by default
        mHardwareBufferManager = new GL3PlusHardwareBufferManager();

        // Use FBO's for RTT, PBuffers and Copy are no longer supported
        // Create FBO manager
        mRTTManager = new GL3PlusFBOManager(this);
        caps->setCapability(RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL);

        Log* defaultLog = LogManager::getSingleton().getDefaultLog();
        if (defaultLog)
        {
            caps->log(defaultLog);
        }

        // Create the texture manager
        mTextureManager = new GL3PlusTextureManager(this);

        mGLInitialised = true;
    }

    void GL3PlusRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        // Remove from manager safely
        if (auto progMgr = HighLevelGpuProgramManager::getSingletonPtr())
        {
            if(mGLSLShaderFactory)
                progMgr->removeFactory(mGLSLShaderFactory);

            if(mSPIRVShaderFactory)
                progMgr->removeFactory(mSPIRVShaderFactory);
        }

        OGRE_DELETE mGLSLShaderFactory;
        mGLSLShaderFactory = 0;

        OGRE_DELETE mSPIRVShaderFactory;
        mSPIRVShaderFactory = 0;

        // Delete extra threads contexts
        for (auto pCurContext : mBackgroundContextList)
        {
            pCurContext->releaseContext();
            OGRE_DELETE pCurContext;
        }
        mBackgroundContextList.clear();

        // Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
        if(mShaderManager)
        {
            ResourceGroupManager::getSingleton()._unregisterResourceManager(mShaderManager->getResourceType());
            OGRE_DELETE mShaderManager;
            mShaderManager = 0;
        }

        OGRE_DELETE mHardwareBufferManager;
        mHardwareBufferManager = 0;

        OGRE_DELETE mRTTManager;
        mRTTManager = 0;

        OGRE_DELETE mTextureManager;
        mTextureManager = 0;


        mGLSupport->stop();
        mStopRendering = true;

        // delete mTextureManager;
        // mTextureManager = 0;

        mGLInitialised = 0;

        // RenderSystem::shutdown();
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

            const char* shadingLangVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
            StringVector tokens = StringUtil::split(shadingLangVersion, ". ");
            mNativeShadingLanguageVersion = (StringConverter::parseUnsignedInt(tokens[0]) * 100) + StringConverter::parseUnsignedInt(tokens[1]);

            auto it = mOptions.find("Reversed Z-Buffer");
            if (it != mOptions.end())
            {
                mIsReverseDepthBufferEnabled = StringConverter::parseBool(it->second.currentValue);

                if(mIsReverseDepthBufferEnabled && !hasMinGLVersion(4, 5) && !checkExtension("GL_ARB_clip_control"))
                {
                    mIsReverseDepthBufferEnabled = false;
                    LogManager::getSingleton().logWarning("Reversed Z-Buffer was requested, but it is not supported. Disabling.");
                }
            }

            it = mOptions.find("Separate Shader Objects");
            if (it != mOptions.end())
            {
                mSeparateShaderObjectsEnabled = StringConverter::parseBool(it->second.currentValue);
            }

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

        if ( win->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH )
        {
            // Unlike D3D9, OGL doesn't allow sharing the main depth buffer, so keep them separate.
            // Only Copy does, but Copy means only one depth buffer...
            GL3PlusContext *windowContext = dynamic_cast<GLRenderTarget*>(win)->getContext();
            GL3PlusDepthBuffer *depthBuffer = new GL3PlusDepthBuffer( DepthBuffer::POOL_DEFAULT, this,
                                                                      windowContext, 0, 0,
                                                                      win->getWidth(), win->getHeight(),
                                                                      win->getFSAA(), true );

            mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

            win->attachDepthBuffer( depthBuffer );
        }

        return win;
    }


    DepthBuffer* GL3PlusRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        GL3PlusDepthBuffer *retVal = 0;

        if ( auto fbo = dynamic_cast<GLRenderTarget*>(renderTarget)->getFBO() )
        {
            // Presence of an FBO means the manager is an FBO Manager, that's why it's safe to downcast.
            // Find best depth & stencil format suited for the RT's format.
            GLuint depthFormat, stencilFormat;
            _getDepthStencilFormatFor(fbo->getFormat(), &depthFormat, &stencilFormat);

            GL3PlusRenderBuffer *depthBuffer = new GL3PlusRenderBuffer( depthFormat, fbo->getWidth(),
                                                                        fbo->getHeight(), fbo->getFSAA() );

            GL3PlusRenderBuffer *stencilBuffer = NULL;
            if ( depthFormat == GL_DEPTH24_STENCIL8 || depthFormat == GL_DEPTH32F_STENCIL8)
            {
                // If we have a packed format, the stencilBuffer is the same as the depthBuffer
                stencilBuffer = depthBuffer;
            }
            else if(stencilFormat)
            {
                stencilBuffer = new GL3PlusRenderBuffer( stencilFormat, fbo->getWidth(),
                                                         fbo->getHeight(), fbo->getFSAA() );
            }

            // No "custom-quality" multisample for now in GL
            retVal = new GL3PlusDepthBuffer( 0, this, mCurrentContext, depthBuffer, stencilBuffer,
                                             fbo->getWidth(), fbo->getHeight(), fbo->getFSAA(), false );
        }

        return retVal;
    }

    MultiRenderTarget* GL3PlusRenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget* retval =
            new GL3PlusFBOMultiRenderTarget(static_cast<GL3PlusFBOManager*>(mRTTManager), name);
        attachRenderTarget(*retval);
        return retval;
    }

    void GL3PlusRenderSystem::destroyRenderWindow(const String& name)
    {
        // Find it to remove from list.
        RenderTarget* pWin = detachRenderTarget(name);
        OgreAssert(pWin, "unknown RenderWindow name");

        GL3PlusContext *windowContext = dynamic_cast<GLRenderTarget*>(pWin)->getContext();

        // 1 Window <-> 1 Context, should be always true.
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
                // look for the one that matches the same GL context.
                GL3PlusDepthBuffer *depthBuffer = static_cast<GL3PlusDepthBuffer*>(*itor);
                GL3PlusContext *glContext = depthBuffer->getGLContext();

                if ( glContext == windowContext &&
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

        delete pWin;
    }

    void GL3PlusRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
        if (!mStateCacheManager->activateGLTextureUnit(stage))
            return;

        if (enabled)
        {
            GL3PlusTexturePtr tex = static_pointer_cast<GL3PlusTexture>(texPtr);

            // Note used
            tex->touch();
            mTextureTypes[stage] = tex->getGL3PlusTextureTarget();

            mStateCacheManager->bindGLTexture( mTextureTypes[stage], tex->getGLID() );
        }
        else
        {
            // Bind zero texture.
            mStateCacheManager->bindGLTexture(GL_TEXTURE_2D, 0);
        }
    }

    void GL3PlusRenderSystem::_setSampler(size_t unit, Sampler& sampler)
    {
        static_cast<GL3PlusSampler&>(sampler).bind(unit);
    }

    void GL3PlusRenderSystem::_setTextureAddressingMode(size_t stage, const Sampler::UVWAddressingMode& uvw)
    {
        if (!mStateCacheManager->activateGLTextureUnit(stage))
            return;
        mStateCacheManager->setTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_S,
                                             GL3PlusSampler::getTextureAddressingMode(uvw.u));
        mStateCacheManager->setTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_T,
                                             GL3PlusSampler::getTextureAddressingMode(uvw.v));
        mStateCacheManager->setTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_R,
                                             GL3PlusSampler::getTextureAddressingMode(uvw.w));
    }

    void GL3PlusRenderSystem::_setLineWidth(float width)
    {
        OGRE_CHECK_GL_ERROR(glLineWidth(width));
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

        // To keep compiler happy.
        return GL_ONE;
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

        if (sourceFactor == SBF_ONE && destFactor == SBF_ZERO &&
            sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
        {
            mStateCacheManager->setEnabled(GL_BLEND, false);
        }
        else
        {
            mStateCacheManager->setEnabled(GL_BLEND, true);
            mStateCacheManager->setBlendFunc(sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha);
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

        mStateCacheManager->setBlendEquation(func, alphaFunc);
    }

    void GL3PlusRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        mStateCacheManager->setEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE, (func != CMPF_ALWAYS_PASS) && alphaToCoverage);
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

            mStateCacheManager->setViewport(x, y, w, h);

            // Configure the viewport clipping
            glScissor(x, y, w, h);
            mScissorBox[0] = x;
            mScissorBox[1] = y;
            mScissorBox[2] = w;
            mScissorBox[3] = h;

            vp->_clearUpdatedFlag();
        }
    }

    void GL3PlusRenderSystem::_beginFrame(void)
    {
        if (!mActiveViewport)
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Cannot begin frame - no viewport selected.",
                        "GL3PlusRenderSystem::_beginFrame");

        mScissorsEnabled = true;
        mStateCacheManager->setEnabled(GL_SCISSOR_TEST, true);
    }

    void GL3PlusRenderSystem::_endFrame(void)
    {
        // Deactivate the viewport clipping.
        mScissorsEnabled = false;
        mStateCacheManager->setEnabled(GL_SCISSOR_TEST, false);

        // unbind GPU programs at end of frame
        // this is mostly to avoid holding bound programs that might get deleted
        // outside via the resource manager
        unbindGpuProgram(GPT_VERTEX_PROGRAM);
        unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
        unbindGpuProgram(GPT_GEOMETRY_PROGRAM);

        if (mDriverVersion.major >= 4)
        {
            unbindGpuProgram(GPT_HULL_PROGRAM);
            unbindGpuProgram(GPT_DOMAIN_PROGRAM);
            if (mDriverVersion.minor >= 3)
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
            mStateCacheManager->setEnabled( GL_CULL_FACE, false );
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

        mStateCacheManager->setEnabled( GL_CULL_FACE, true );
        mStateCacheManager->setCullFace( cullMode );
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
            mStateCacheManager->setClearDepth(isReverseDepthBufferEnabled() ? 0.0f : 1.0f);
        }
        mStateCacheManager->setEnabled(GL_DEPTH_TEST, enabled);
    }

    void GL3PlusRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
        mStateCacheManager->setDepthMask( flag );

        // Store for reference in _beginFrame
        mDepthWrite = enabled;
    }

    void GL3PlusRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        if(isReverseDepthBufferEnabled())
            func = reverseCompareFunction(func);
        mStateCacheManager->setDepthFunc(convertCompareFunction(func));
    }

    void GL3PlusRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        bool enable = constantBias != 0 || slopeScaleBias != 0;
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_FILL, enable);
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_POINT, enable);
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_LINE, enable);

        if (enable)
        {
            if(isReverseDepthBufferEnabled())
            {
                slopeScaleBias *= -1;
                constantBias *= -1;
            }

            glPolygonOffset(-slopeScaleBias, -constantBias);
        }
    }

    void GL3PlusRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        mStateCacheManager->setColourMask(red, green, blue, alpha);

        // record this
        mColourWrite[0] = red;
        mColourWrite[1] = green;
        mColourWrite[2] = blue;
        mColourWrite[3] = alpha;
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
            mStateCacheManager->setPolygonMode(GL_POINT);
            break;
        case PM_WIREFRAME:
            mStateCacheManager->setPolygonMode(GL_LINE);
            break;
        case PM_SOLID:
            mStateCacheManager->setPolygonMode(GL_FILL);
            break;
        }
    }

    void GL3PlusRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        mStateCacheManager->setEnabled(GL_STENCIL_TEST, enabled);
    }

    void GL3PlusRenderSystem::setStencilBufferParams(CompareFunction func,
                                                     uint32 refValue, uint32 compareMask, uint32 writeMask,
                                                     StencilOperation stencilFailOp,
                                                     StencilOperation depthFailOp,
                                                     StencilOperation passOp,
                                                     bool twoSidedOperation,
                                                     bool readBackAsTexture)
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
            mStateCacheManager->setStencilMask(writeMask);
            OGRE_CHECK_GL_ERROR(glStencilFunc(convertCompareFunction(func), refValue, compareMask));
            OGRE_CHECK_GL_ERROR(glStencilOp(
                convertStencilOp(stencilFailOp, flip),
                convertStencilOp(depthFailOp, flip),
                convertStencilOp(passOp, flip)));
        }
    }

    void GL3PlusRenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions fo)
    {
        if (!mStateCacheManager->activateGLTextureUnit(unit))
            return;

        switch (ftype)
        {
        case FT_MIN:
            mMinFilter = fo;

            // Combine with existing mip filter
            mStateCacheManager->setTexParameteri(
                mTextureTypes[unit], GL_TEXTURE_MIN_FILTER,
                GL3PlusSampler::getCombinedMinMipFilter(mMinFilter, mMipFilter));
            break;

        case FT_MAG:
            switch (fo)
            {
            case FO_ANISOTROPIC: // GL treats linear and aniso the same
            case FO_LINEAR:
                mStateCacheManager->setTexParameteri(mTextureTypes[unit],
                                                    GL_TEXTURE_MAG_FILTER,
                                                    GL_LINEAR);
                break;
            case FO_POINT:
            case FO_NONE:
                mStateCacheManager->setTexParameteri(mTextureTypes[unit],
                                                    GL_TEXTURE_MAG_FILTER,
                                                    GL_NEAREST);
                break;
            }
            break;
        case FT_MIP:
            mMipFilter = fo;

            // Combine with existing min filter
            mStateCacheManager->setTexParameteri(
                mTextureTypes[unit], GL_TEXTURE_MIN_FILTER,
                GL3PlusSampler::getCombinedMinMipFilter(mMinFilter, mMipFilter));
            break;
        }
    }

    void GL3PlusRenderSystem::_dispatchCompute(const Vector3i& workgroupDim)
    {
        // if(mComputeProgramExecutions <= compute_execution_cap)

        //FIXME give user control over when and what memory barriers are created
        // if (mPreComputeMemoryBarrier)
        OGRE_CHECK_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));
        OGRE_CHECK_GL_ERROR(glDispatchCompute(workgroupDim[0], workgroupDim[1], workgroupDim[2]));
        // if (mPostComputeMemoryBarrier)
        //     OGRE_CHECK_GL_ERROR(glMemoryBarrier(toGL(MB_TEXTURE)));
        // if (compute_execution_cap > 0)
        //     mComputeProgramExecutions++;
    }

    void GL3PlusRenderSystem::_render(const RenderOperation& op)
    {
        // Call super class.
        RenderSystem::_render(op);

        // Create variables related to instancing.
        HardwareVertexBufferSharedPtr globalInstanceVertexBuffer = getGlobalInstanceVertexBuffer();
        VertexDeclaration* globalVertexDeclaration = getGlobalInstanceVertexBufferVertexDeclaration();
        bool hasInstanceData = (op.useGlobalInstancingVertexBufferIsAvailable &&
                                globalInstanceVertexBuffer && globalVertexDeclaration) ||
                               op.vertexData->vertexBufferBinding->hasInstanceData();

        size_t numberOfInstances = op.numberOfInstances;

        if (op.useGlobalInstancingVertexBufferIsAvailable)
        {
            numberOfInstances *= getGlobalNumberOfInstances();
        }

        GLSLProgram* program = GLSLProgramManager::getSingleton().getActiveProgram();

        if (!program)
        {
            LogManager::getSingleton().logError("Failed to create shader program.");
        }

        GLVertexArrayObject* vao =
            static_cast<GLVertexArrayObject*>(op.vertexData->vertexDeclaration);
        // Bind VAO (set of per-vertex attributes: position, normal, etc.).
        vao->bind(this);
        bool updateVAO = vao->needsUpdate(op.vertexData->vertexBufferBinding,
                                          op.vertexData->vertexStart);

        if (updateVAO)
            vao->bindToGpu(this, op.vertexData->vertexBufferBinding, op.vertexData->vertexStart);

        // We treat index buffer binding inside VAO as volatile, always updating and never relying onto it,
        // as one shared vertex buffer could be rendered with several index buffers, from submeshes and/or LODs
        if (op.useIndexes)
            mStateCacheManager->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GL3PlusHardwareIndexBuffer*>(op.indexData->indexBuffer.get())->getGLBufferId());

        // unconditionally modify VAO for global instance data (FIXME bad API)
        VertexDeclaration::VertexElementList::const_iterator elemIter, elemEnd;
        if ( globalInstanceVertexBuffer && globalVertexDeclaration )
        {
            elemEnd = globalVertexDeclaration->getElements().end();
            for (elemIter = globalVertexDeclaration->getElements().begin(); elemIter != elemEnd; ++elemIter)
            {
                const VertexElement & elem = *elemIter;
                bindVertexElementToGpu(elem, globalInstanceVertexBuffer, 0);
            }
        }

        int operationType = op.operationType;
        // Use adjacency if there is a geometry program and it requested adjacency info
        auto currentGeometryShader = mCurrentShader[GPT_GEOMETRY_PROGRAM];
        if(mGeometryProgramBound && currentGeometryShader && currentGeometryShader->isAdjacencyInfoRequired())
            operationType |= RenderOperation::OT_DETAIL_ADJACENCY_BIT;

        // Determine the correct primitive type to render.
        GLint primType;
        switch (operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            primType = GL_POINTS;
            break;
        case RenderOperation::OT_LINE_LIST:
            primType = GL_LINES;
            break;
        case RenderOperation::OT_LINE_LIST_ADJ:
            primType = GL_LINES_ADJACENCY;
            break;
        case RenderOperation::OT_LINE_STRIP:
            primType = GL_LINE_STRIP;
            break;
        case RenderOperation::OT_LINE_STRIP_ADJ:
            primType = GL_LINE_STRIP_ADJACENCY;
            break;
        default:
        case RenderOperation::OT_TRIANGLE_LIST:
            primType = GL_TRIANGLES;
            break;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
            primType = GL_TRIANGLES_ADJACENCY;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = GL_TRIANGLE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            primType = GL_TRIANGLE_STRIP_ADJACENCY;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            primType = GL_TRIANGLE_FAN;
            break;
        }


        // Bind atomic counter buffers.
        // if (Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_ATOMIC_COUNTERS))
        // {
        //     GLuint atomicsBuffer = 0;

        //     glGenBuffers(1, &atomicsBuffer);
        //     glBindBuffer(GL_ATOMIC_COUNTER_BUFFER,
        //                  static_cast<GL3PlusHardwareCounterBuffer*>(HardwareBufferManager::getSingleton().getCounterBuffer().getGLBufferId()));
        //                  //static_cast<GL3PlusHardwareCounterBuffer*>(op..getCounterBuffer().getGLBufferId()));
        //     // glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 3, NULL, GL_DYNAMIC_DRAW);
        //     // glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        // }
        //TODO: Reset atomic counters somewhere


        // Render to screen!
        if (mCurrentShader[GPT_DOMAIN_PROGRAM])
        {
            // Tessellation shader special case.
            // Note: Only evaluation (domain) shaders are required.

            // GLuint primCount = 0;
            // // Useful primitives for tessellation
            // switch( op.operationType )
            // {
            // case RenderOperation::OT_LINE_LIST:
            //     primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
            //     break;

            // case RenderOperation::OT_LINE_STRIP:
            //     primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
            //     break;

            // case RenderOperation::OT_TRIANGLE_LIST:
            //     primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
            //     //primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
            //     break;

            // case RenderOperation::OT_TRIANGLE_STRIP:
            //     primCount = (GLuint)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
            //     break;
            // default:
            //     break;
            // }

            // These are set via shader in DX11, SV_InsideTessFactor and SV_OutsideTessFactor
            // Hardcoding for the sample
            // float patchLevel(1.f);
            // OGRE_CHECK_GL_ERROR(glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &patchLevel));
            // OGRE_CHECK_GL_ERROR(glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &patchLevel));
            // OGRE_CHECK_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, op.vertexData->vertexCount));

            if (op.useIndexes)
            {
                void *pBufferData = GL_BUFFER_OFFSET(op.indexData->indexStart *
                                                     op.indexData->indexBuffer->getIndexSize());
                GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
                OGRE_CHECK_GL_ERROR(glDrawElements(GL_PATCHES, op.indexData->indexCount, indexType, pBufferData));
                //OGRE_CHECK_GL_ERROR(glDrawElements(GL_PATCHES, op.indexData->indexCount, indexType, pBufferData));
                //                OGRE_CHECK_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, primCount, 1));
            }
            else
            {
                OGRE_CHECK_GL_ERROR(glDrawArrays(GL_PATCHES, 0, op.vertexData->vertexCount));
                //OGRE_CHECK_GL_ERROR(glDrawArrays(GL_PATCHES, 0, primCount));
                //                OGRE_CHECK_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, primCount, 1));
            }
        }
        else if (op.useIndexes)
        {
            void *pBufferData = GL_BUFFER_OFFSET(op.indexData->indexStart *
                                                 op.indexData->indexBuffer->getIndexSize());

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            do
            {
                // Update derived depth bias.
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }

                if (hasInstanceData)
                {
                    OGRE_CHECK_GL_ERROR(glDrawElementsInstanced(primType, op.indexData->indexCount, indexType, pBufferData, numberOfInstances));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glDrawElements(primType, op.indexData->indexCount, indexType, pBufferData));
                }
            } while (updatePassIterationRenderState());
        }
        else
        {
            do
            {
                // Update derived depth bias.
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }

                if (hasInstanceData)
                {
                    OGRE_CHECK_GL_ERROR(glDrawArraysInstanced(primType, 0, op.vertexData->vertexCount, numberOfInstances));
                }
                else
                {
                    OGRE_CHECK_GL_ERROR(glDrawArrays(primType, 0, op.vertexData->vertexCount));
                }
            } while (updatePassIterationRenderState());
        }

        // Do not unbind the vertex array object
        // VAOs > 0 are selected each time before usage
        // VAO #0 is not supported in Core profiles, and WOULD NOT be used by Ogre even in compatibility profiles
    }

    void GL3PlusRenderSystem::_getDepthStencilFormatFor(PixelFormat internalColourFormat,
                                                        uint32* depthFormat,
                                                        uint32* stencilFormat)
    {
        if (isReverseDepthBufferEnabled())
        {
            *depthFormat = GL_DEPTH_COMPONENT32F;
            *stencilFormat = GL_NONE;
        }
        else
        {
            static_cast<GL3PlusFBOManager*>(mRTTManager)->getBestDepthStencil(
                    internalColourFormat, depthFormat, stencilFormat);
        }
    }

    void GL3PlusRenderSystem::setScissorTest(bool enabled, size_t left,
                                             size_t top, size_t right,
                                             size_t bottom)
    {
        mScissorsEnabled = enabled;
        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();
        //  GL measures from the bottom, not the top
        size_t targetHeight = mActiveRenderTarget->getHeight();
        // Calculate the "lower-left" corner of the viewport
        int x = 0, y = 0, w = 0, h = 0;

        if (enabled)
        {
            mStateCacheManager->setEnabled(GL_SCISSOR_TEST, true);
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

            mScissorBox[0] = x;
            mScissorBox[1] = y;
            mScissorBox[2] = w;
            mScissorBox[3] = h;
        }
        else
        {
            mStateCacheManager->setEnabled(GL_SCISSOR_TEST, false);
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

            mScissorBox[0] = x;
            mScissorBox[1] = y;
            mScissorBox[2] = w;
            mScissorBox[3] = h;
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
                mStateCacheManager->setColourMask(true, true, true, true);
            }
            mStateCacheManager->setClearColour(colour.r, colour.g, colour.b, colour.a);
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
            // Enable buffer for writing if it isn't
            if (!mDepthWrite)
            {
                mStateCacheManager->setDepthMask( GL_TRUE );
            }

            if (isReverseDepthBufferEnabled())
            {
                depth = 1.0f - 0.5f * (depth + 1.0f);
            }

            mStateCacheManager->setClearDepth(depth);
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
            // Enable buffer for writing if it isn't
            mStateCacheManager->setStencilMask(0xFFFFFFFF);
            OGRE_CHECK_GL_ERROR(glClearStencil(stencil));
        }

        // Should be enable scissor test due the clear region is
        // relied on scissor box bounds.
        if (!mScissorsEnabled)
        {
            mStateCacheManager->setEnabled(GL_SCISSOR_TEST, true);
        }

        // Sets the scissor box as same as viewport
        GLint viewport[4];
        mStateCacheManager->getViewport(viewport);
        bool scissorBoxDifference =
            viewport[0] != mScissorBox[0] || viewport[1] != mScissorBox[1] ||
            viewport[2] != mScissorBox[2] || viewport[3] != mScissorBox[3];
        if (scissorBoxDifference)
        {
            OGRE_CHECK_GL_ERROR(glScissor(viewport[0], viewport[1], viewport[2], viewport[3]));
        }

        // Clear buffers
        OGRE_CHECK_GL_ERROR(glClear(flags));

        // Restore scissor box
        if (scissorBoxDifference)
        {
            OGRE_CHECK_GL_ERROR(glScissor(mScissorBox[0], mScissorBox[1], mScissorBox[2], mScissorBox[3]));
        }

        // Restore scissor test
        if (!mScissorsEnabled)
        {
            mStateCacheManager->setEnabled(GL_SCISSOR_TEST, false);
        }

        // Reset buffer write state
        if (!mDepthWrite && (buffers & FBT_DEPTH))
        {
            mStateCacheManager->setDepthMask( GL_FALSE );
        }

        if (colourMask && (buffers & FBT_COLOUR))
        {
            mStateCacheManager->setColourMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
        }

        if (buffers & FBT_STENCIL)
        {
            mStateCacheManager->setStencilMask(mStencilWriteMask);
        }
    }

    void GL3PlusRenderSystem::_switchContext(GL3PlusContext *context)
    {
        // Unbind GPU programs and rebind to new context later, because
        // scene manager treat render system as ONE 'context' ONLY, and it
        // cached the GPU programs using state.
        for(auto shader : mCurrentShader)
        {
            if(!shader) continue;
            GLSLProgramManager::getSingleton().setActiveShader(shader->getType(), NULL);
        }

        // Disable textures
        _disableTextureUnitsFrom(0);

        // It's ready for switching
        if (mCurrentContext!=context)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            // NSGLContext::makeCurrentContext does not flush automatically. everybody else does.
            glFlushRenderAPPLE();
#endif
            mCurrentContext->endCurrent();
            mCurrentContext = context;
        }
        mCurrentContext->setCurrent();

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GL3PlusStateCacheManager>();
        _completeDeferredVaoFboDestruction();

        // Check if the context has already done one-time initialisation
        if (!mCurrentContext->getInitialized())
        {
            _oneTimeContextInitialization();
            mCurrentContext->setInitialized();
        }

        // Rebind GPU programs to new context
        for(auto shader : mCurrentShader)
        {
            if(!shader) continue;
            GLSLProgramManager::getSingleton().setActiveShader(shader->getType(), shader);
        }

        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        mStateCacheManager->setDepthMask(mDepthWrite);
        mStateCacheManager->setColourMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
        mStateCacheManager->setStencilMask(mStencilWriteMask);
    }

    void GL3PlusRenderSystem::_unregisterContext(GL3PlusContext *context)
    {
        static_cast<GL3PlusHardwareBufferManager*>(HardwareBufferManager::getSingletonPtr())->notifyContextDestroyed(context);

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
                /// No contexts remain
                mCurrentContext->endCurrent();
                mCurrentContext = 0;
                mMainContext = 0;
                mStateCacheManager = 0;
            }
        }
    }

    uint32 GL3PlusRenderSystem::_createVao()
    {
        uint32 vao = 0;
        OGRE_CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
        return vao;
    }

    void GL3PlusRenderSystem::_destroyVao(GLContext* context, uint32 vao)
    {
        if(context != mCurrentContext)
            context->_getVaoDeferredForDestruction().push_back(vao);
        else
            OGRE_CHECK_GL_ERROR(glDeleteVertexArrays(1, &vao));
    }

    void GL3PlusRenderSystem::_destroyFbo(GLContext* context, uint32 fbo)
    {
        if(context != mCurrentContext)
            context->_getFboDeferredForDestruction().push_back(fbo);
        else
            _getStateCacheManager()->deleteGLFrameBuffer(GL_FRAMEBUFFER, fbo);
    }

    void GL3PlusRenderSystem::_bindVao(GLContext* context, uint32 vao)
    {
        OgreAssert(context == mCurrentContext, "VAO used in wrong OpenGL context");
        _getStateCacheManager()->bindGLVertexArray(vao);
    }

    void GL3PlusRenderSystem::_oneTimeContextInitialization()
    {
        OGRE_CHECK_GL_ERROR(glDisable(GL_DITHER));

        // Check for FSAA
        // Enable the extension if it was enabled by the GL3PlusSupport
        int fsaa_active = false;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_SAMPLE_BUFFERS, (GLint*)&fsaa_active));
        if (fsaa_active)
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
        }

        if (checkExtension("GL_EXT_texture_filter_anisotropic"))
        {
            OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mLargestSupportedAnisotropy));
        }

        // Enable seamless cube maps
        OGRE_CHECK_GL_ERROR(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
        // Set provoking vertex convention
        OGRE_CHECK_GL_ERROR(glProvokingVertex(GL_FIRST_VERTEX_CONVENTION));

        if (getCapabilities()->hasCapability(RSC_DEBUG))
        {
#if ENABLE_GL_DEBUG_OUTPUT
            OGRE_CHECK_GL_ERROR(glEnable(GL_DEBUG_OUTPUT));
            OGRE_CHECK_GL_ERROR(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
            OGRE_CHECK_GL_ERROR(glDebugMessageCallbackARB(&GLDebugCallback, NULL));
            OGRE_CHECK_GL_ERROR(glDebugMessageControlARB(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, GL_TRUE));
#endif
        }

        if(getCapabilities()->hasCapability(RSC_PRIMITIVE_RESTART))
        {
            OGRE_CHECK_GL_ERROR(glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
        }

        glEnable(GL_PROGRAM_POINT_SIZE);

        if(getCapabilities()->getVendor() == GPU_NVIDIA)
        {
            // bug in NVIDIA driver, see e.g.
            // https://www.opengl.org/discussion_boards/showthread.php/168217-gl_PointCoord-and-OpenGL-3-1-GLSL-1-4
            glEnable(0x8861); // GL_POINT_SPRITE
            glGetError();     // clear the error that it generates nevertheless..
        }

        if (isReverseDepthBufferEnabled())
        {
            // We want depth to range from 0 to 1 to increase precision.
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        }
    }

    void GL3PlusRenderSystem::initialiseContext(RenderWindow* primary)
    {
        // Set main and current context
        mMainContext = dynamic_cast<GLRenderTarget*>(primary)->getContext();
        mCurrentContext = mMainContext;

        // Set primary context as active
        if (mCurrentContext)
            mCurrentContext->setCurrent();

        // Initialise GL3W
        if (gl3wInit2(get_proc)) { // gl3wInit() fails if GL3.0 is not supported
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "OpenGL 3.0 is not supported");
        }

        // Setup GL3PlusSupport
        initialiseExtensions();

        OgreAssert(hasMinGLVersion(3, 3), "OpenGL 3.3 is not supported");

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GL3PlusStateCacheManager>();

        LogManager::getSingleton().logMessage("**************************************");
        LogManager::getSingleton().logMessage("***   OpenGL 3+ Renderer Started   ***");
        LogManager::getSingleton().logMessage("**************************************");
    }

    void GL3PlusRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;
        if (auto gltarget = dynamic_cast<GLRenderTarget*>(target))
        {
            // Switch context if different from current one
            GL3PlusContext *newContext = gltarget->getContext();
            if (newContext && mCurrentContext != newContext)
            {
                _switchContext(newContext);
            }

            // Check the FBO's depth buffer status
            GL3PlusDepthBuffer *depthBuffer = static_cast<GL3PlusDepthBuffer*>(target->getDepthBuffer());

            if ( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
                 (!depthBuffer || depthBuffer->getGLContext() != mCurrentContext ) )
            {
                // Depth is automatically managed and there is no depth buffer attached to this RT
                // or the Current context doesn't match the one this Depth buffer was created with
                setDepthBufferFor( target );
            }

            /* Bind a certain render target if it is a FBO. If it is not a FBO, bind the
               main frame buffer.
            */
            if(auto fbo = gltarget->getFBO())
                fbo->bind(true);
            else
                _getStateCacheManager()->bindGLFrameBuffer( GL_FRAMEBUFFER, 0 );

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

    GLint GL3PlusRenderSystem::convertCompareFunction(CompareFunction func)
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
        }
        // To keep compiler happy
        return GL_ALWAYS;
    }

    GLint GL3PlusRenderSystem::convertStencilOp(StencilOperation op, bool invert)
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


    void GL3PlusRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        GLSLShader* glprg = static_cast<GLSLShader*>(prg);

        mCurrentShader[glprg->getType()] = glprg;
        // Bind the program
        GLSLProgramManager::getSingleton().setActiveShader(glprg->getType(), glprg);

        RenderSystem::bindGpuProgram(prg);

        // TextureManager::ResourceMapIterator resource = TextureManager::getSingletonPtr()->getResourceIterator();

        // while(resource.hasMoreElements())
        // {
        //     TextureManager::ResourceMapPtr resource_map = resource.getNext();
        //     resource_map.getResourceType();
        // }

        // //FIXME Either a new TextureShaderUsage enum needs to be introduced,
        // // or additional TextureUsages must be created.  See OgreTexture.h
        // if (tex->getUsage() == TU_DYNAMIC_SHADER)
        // {
        //     // OGRE_CHECK_GL_ERROR(glBindImageTexture(0, mTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
        //     OGRE_CHECK_GL_ERROR(glBindImageTexture(0, tex->getGLID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8));
        // }
    }

    void GL3PlusRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        GLSLProgramManager::getSingleton().setActiveShader(gptype, NULL);

        if (gptype == GPT_VERTEX_PROGRAM && mCurrentShader[gptype])
        {
            mActiveVertexGpuProgramParameters.reset();
        }
        else if (gptype == GPT_GEOMETRY_PROGRAM && mCurrentShader[gptype])
        {
            mActiveGeometryGpuProgramParameters.reset();
        }
        else if (gptype == GPT_FRAGMENT_PROGRAM && mCurrentShader[gptype])
        {
            mActiveFragmentGpuProgramParameters.reset();
        }
        else if (gptype == GPT_HULL_PROGRAM && mCurrentShader[gptype])
        {
            mActiveTessellationHullGpuProgramParameters.reset();
        }
        else if (gptype == GPT_DOMAIN_PROGRAM && mCurrentShader[gptype])
        {
            mActiveTessellationDomainGpuProgramParameters.reset();
        }
        else if (gptype == GPT_COMPUTE_PROGRAM && mCurrentShader[gptype])
        {
            mActiveComputeGpuProgramParameters.reset();
        }

        mCurrentShader[gptype] = NULL;

        RenderSystem::unbindGpuProgram(gptype);
    }

    void GL3PlusRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask)
    {
        switch (gptype)
        {
        case GPT_VERTEX_PROGRAM:
            mActiveVertexGpuProgramParameters = params;
            break;
        case GPT_FRAGMENT_PROGRAM:
            mActiveFragmentGpuProgramParameters = params;
            break;
        case GPT_GEOMETRY_PROGRAM:
            mActiveGeometryGpuProgramParameters = params;
            break;
        case GPT_HULL_PROGRAM:
            mActiveTessellationHullGpuProgramParameters = params;
            break;
        case GPT_DOMAIN_PROGRAM:
            mActiveTessellationDomainGpuProgramParameters = params;
            break;
        case GPT_COMPUTE_PROGRAM:
            mActiveComputeGpuProgramParameters = params;
            break;
        default:
            break;
        }

        GLSLProgram* program = NULL;

        // Link can throw exceptions, ignore them at this point.
        try
        {
            program = GLSLProgramManager::getSingleton().getActiveProgram();
        }
        catch (InvalidParametersException& e)
        {
            LogManager::getSingleton().logError("binding shared parameters failed: " + e.getDescription());
            return;
        }
        catch (Exception&)
        {
            return;
        }

        if (mask & (uint16)GPV_GLOBAL)
        {
            params->_updateSharedParams();
        }

        // Pass on parameters from params to program object uniforms.
        program->updateUniforms(params, mask, gptype);
        program->updateAtomicCounters(params, mask, gptype);

        // FIXME This needs to be moved somewhere texture specific.
        // Update image bindings for image load/store
        // static_cast<GL3PlusTextureManager*>(mTextureManager)->bindImages();
    }

    unsigned int GL3PlusRenderSystem::getDisplayMonitorCount() const
    {
        return mGLSupport->getDisplayMonitorCount();
    }


    void GL3PlusRenderSystem::beginProfileEvent( const String &eventName )
    {
        if (getCapabilities()->hasCapability(RSC_DEBUG))
            OGRE_CHECK_GL_ERROR(glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, static_cast<GLint>(eventName.length()), eventName.c_str()));
    }


    void GL3PlusRenderSystem::endProfileEvent( void )
    {
        if (getCapabilities()->hasCapability(RSC_DEBUG))
            OGRE_CHECK_GL_ERROR(glPopDebugGroup());
    }


    void GL3PlusRenderSystem::markProfileEvent( const String &eventName )
    {
        if ( eventName.empty() )
            return;

        if (getCapabilities()->hasCapability(RSC_DEBUG))
            glDebugMessageInsert(GL_DEBUG_SOURCE_THIRD_PARTY,
                                 GL_DEBUG_TYPE_PERFORMANCE,
                                 0,
                                 GL_DEBUG_SEVERITY_LOW,
                                 static_cast<GLint>(eventName.length()),
                                 eventName.c_str());
    }

    void GL3PlusRenderSystem::bindVertexElementToGpu(const VertexElement& elem,
                                                     const HardwareVertexBufferSharedPtr& vertexBuffer,
                                                     const size_t vertexStart)
    {
        VertexElementSemantic sem = elem.getSemantic();
        unsigned short elemIndex = elem.getIndex();

        GLuint attrib = (GLuint)GLSLProgramCommon::getFixedAttributeIndex(sem, elemIndex);

        const GL3PlusHardwareVertexBuffer* hwGlBuffer = static_cast<const GL3PlusHardwareVertexBuffer*>(vertexBuffer.get());
        mStateCacheManager->bindGLBuffer(GL_ARRAY_BUFFER, hwGlBuffer->getGLBufferId());
        void* pBufferData = GL_BUFFER_OFFSET(elem.getOffset() + vertexStart * vertexBuffer->getVertexSize());

        if (hwGlBuffer->isInstanceData())
        {
            OGRE_CHECK_GL_ERROR(glVertexAttribDivisor(attrib, hwGlBuffer->getInstanceDataStepRate()));
        }

        unsigned short typeCount = VertexElement::getTypeCount(elem.getType());
        GLboolean normalised = GL_FALSE;
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
        case VET_UBYTE4_NORM:
        case VET_SHORT2_NORM:
        case VET_USHORT2_NORM:
        case VET_SHORT4_NORM:
        case VET_USHORT4_NORM:
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
    }
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
	bool GL3PlusRenderSystem::setDrawBuffer(ColourBufferType colourBuffer)
	{
		bool result = true;

		switch (colourBuffer)
		{
            case CBT_BACK:
                OGRE_CHECK_GL_ERROR(glDrawBuffer(GL_BACK));
                break;
            case CBT_BACK_LEFT:
                OGRE_CHECK_GL_ERROR(glDrawBuffer(GL_BACK_LEFT));
                break;
            case CBT_BACK_RIGHT:
                OGRE_CHECK_GL_ERROR(glDrawBuffer(GL_BACK_RIGHT));
//                break;
            default:
                result = false;
		}

		return result;
	}
#endif

    void GL3PlusRenderSystem::_copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox &dst, RenderWindow::FrameBuffer buffer)
    {
        GLenum format = GL3PlusPixelUtil::getGLOriginFormat(dst.format);
        GLenum type = GL3PlusPixelUtil::getGLOriginDataType(dst.format);

        if ((format == GL_NONE) || (type == 0))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported format", "GL3PlusRenderSystem::_copyContentsToMemory");
        }

        // Switch context if different from current one
        _setViewport(vp);

        if(dst.getWidth() != dst.rowPitch)
            glPixelStorei(GL_PACK_ROW_LENGTH, dst.rowPitch);
        // Must change the packing to ensure no overruns!
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        uint32_t height = vp->getTarget()->getHeight();

        glReadBuffer((buffer == RenderWindow::FB_FRONT)? GL_FRONT : GL_BACK);
        glReadPixels((GLint)src.left, (GLint)(height - src.bottom),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());

        // restore default alignment
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);

        PixelUtil::bulkPixelVerticalFlip(dst);
    }

    void GL3PlusRenderSystem::initialiseExtensions(void)
    {
        // get driver version.
        // this is the recommended way for GL3 see: https://www.opengl.org/wiki/Get_Context_Info
        glGetIntegerv(GL_MAJOR_VERSION, &mDriverVersion.major);
        glGetIntegerv(GL_MINOR_VERSION, &mDriverVersion.minor);

        LogManager::getSingleton().logMessage("GL_VERSION = " + mDriverVersion.toString());

        // Get vendor
        const GLubyte* pcVendor = glGetString(GL_VENDOR);
        String tmpStr = (const char*)pcVendor;
        LogManager::getSingleton().logMessage("GL_VENDOR = " + tmpStr);
        mVendor = tmpStr.substr(0, tmpStr.find(' '));

        // Get renderer
        const GLubyte* pcRenderer = glGetString(GL_RENDERER);
        tmpStr = (const char*)pcRenderer;
        LogManager::getSingleton().logMessage("GL_RENDERER = " + tmpStr);

        // Set extension list
        Log::Stream log = LogManager::getSingleton().stream();
        String str;

        GLint numExt;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

        log << "GL_EXTENSIONS = ";
        for(int i = 0; i < numExt; i++)
        {
            const GLubyte* pcExt = glGetStringi(GL_EXTENSIONS, i);
            assert(pcExt && "Problems getting GL extension string using glGetString");
            str = String((const char*)pcExt);
            log << str << " ";
            mExtensionList.insert(str);
        }
    }
}
