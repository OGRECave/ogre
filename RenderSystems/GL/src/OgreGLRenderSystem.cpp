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


#include "OgreGLRenderSystem.h"
#include "OgreGLNativeSupport.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreFrustum.h"
#include "OgreGLTextureManager.h"
#include "OgreGLHardwareBuffer.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreGLUtil.h"
#include "OgreGLGpuProgram.h"
#include "OgreGLGpuNvparseProgram.h"
#include "ATI_FS_GLGpuProgram.h"
#include "OgreGLGpuProgramManager.h"
#include "OgreException.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLHardwareOcclusionQuery.h"
#include "OgreGLDepthBufferCommon.h"
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLContext.h"
#include "OgreGLSLProgramFactory.h"
#include "OgreGLStateCacheManager.h"

#include "OgreGLFBORenderTexture.h"
#include "OgreGLPBRenderTexture.h"
#include "OgreConfig.h"
#include "OgreViewport.h"

#include "OgreGLPixelFormat.h"

#include "OgreGLSLProgramCommon.h"
#include "OgreGLFBOMultiRenderTarget.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
extern "C" void glFlushRenderAPPLE();
#endif

namespace Ogre {

    static GLNativeSupport* glsupport;
    static void* get_proc(const char* proc) {
        return glsupport->getProcAddress(proc);
    }

    typedef TransformBase<4, float> Matrix4f;

    // Callback function used when registering GLGpuPrograms
    static GpuProgram* createGLArbGpuProgram(ResourceManager* creator,
                                      const String& name, ResourceHandle handle,
                                      const String& group, bool isManual, ManualResourceLoader* loader,
                                      GpuProgramType gptype, const String& syntaxCode)
    {
        GLArbGpuProgram* ret = new GLArbGpuProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
    }

    static GpuProgram* createGLGpuNvparseProgram(ResourceManager* creator,
                                          const String& name, ResourceHandle handle,
                                          const String& group, bool isManual, ManualResourceLoader* loader,
                                          GpuProgramType gptype, const String& syntaxCode)
    {
        GLGpuNvparseProgram* ret = new GLGpuNvparseProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
    }

    static GpuProgram* createGL_ATI_FS_GpuProgram(ResourceManager* creator,
                                           const String& name, ResourceHandle handle,
                                           const String& group, bool isManual, ManualResourceLoader* loader,
                                           GpuProgramType gptype, const String& syntaxCode)
    {

        ATI_FS_GLGpuProgram* ret = new ATI_FS_GLGpuProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
    }

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

    GLRenderSystem::GLRenderSystem()
    :   mFixedFunctionTextureUnits(0),
        mStencilWriteMask(0xFFFFFFFF),
        mDepthWrite(true),
        mUseAutoTextureMatrix(false),
        mHardwareBufferManager(0),
        mGpuProgramManager(0),
        mGLSLProgramFactory(0),
        mStateCacheManager(0),
        mActiveTextureUnit(0),
        mMaxBuiltInTextureAttribIndex(0)
    {
        size_t i;

        LogManager::getSingleton().logMessage(getName() + " created.");

        mRenderAttribsBound.reserve(100);
        mRenderInstanceAttribsBound.reserve(100);

        // Get our GLSupport
        mGLSupport = getGLSupport(GLNativeSupport::CONTEXT_COMPATIBILITY);
        glsupport = mGLSupport;

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;

        initConfigOptions();

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
        mEnableFixedPipeline = true;

        mCurrentLights = 0;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
        mCurrentVertexProgram = 0;
        mCurrentGeometryProgram = 0;
        mCurrentFragmentProgram = 0;
        mRTTManager = NULL;
    }

    GLRenderSystem::~GLRenderSystem()
    {
        shutdown();

        delete mGLSupport;
    }

    const GpuProgramParametersPtr& GLRenderSystem::getFixedFunctionParams(TrackVertexColourType tracking,
                                                                          FogMode fog)
    {
        _setSurfaceTracking(tracking);
        _setFog(fog);

        return mFixedFunctionParams;
    }

    void GLRenderSystem::applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 mask)
    {
        bool updateLightPos = false;

        // Autoconstant index is not a physical index
        for (const auto& ac : params->getAutoConstants())
        {
            // Only update needed slots
            if (ac.variability & mask)
            {
                const float* ptr = params->getFloatPointer(ac.physicalIndex);
                switch(ac.paramType)
                {
                case GpuProgramParameters::ACT_WORLD_MATRIX:
                    setWorldMatrix(Matrix4(ptr));
                    break;
                case GpuProgramParameters::ACT_VIEW_MATRIX:
                    // force light update
                    updateLightPos = true;
                    mask |= GPV_LIGHTS;
                    setViewMatrix(Matrix4(ptr));
                    break;
                case GpuProgramParameters::ACT_PROJECTION_MATRIX:
                    setProjectionMatrix(Matrix4(ptr));
                    break;
                case GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR:
                    mStateCacheManager->setMaterialAmbient(ptr[0], ptr[1], ptr[2], ptr[3]);
                    break;
                case GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR:
                    mStateCacheManager->setMaterialDiffuse(ptr[0], ptr[1], ptr[2], ptr[3]);
                    break;
                case GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR:
                    mStateCacheManager->setMaterialSpecular(ptr[0], ptr[1], ptr[2], ptr[3]);
                    break;
                case GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR:
                    mStateCacheManager->setMaterialEmissive(ptr[0], ptr[1], ptr[2], ptr[3]);
                    break;
                case GpuProgramParameters::ACT_SURFACE_SHININESS:
                    mStateCacheManager->setMaterialShininess(ptr[0]);
                    break;
                case GpuProgramParameters::ACT_POINT_PARAMS:
                    mStateCacheManager->setPointSize(ptr[0]);
                    mStateCacheManager->setPointParameters(ptr + 1);
                    break;
                case GpuProgramParameters::ACT_FOG_PARAMS:
                    glFogf(GL_FOG_DENSITY, ptr[0]);
                    glFogf(GL_FOG_START, ptr[1]);
                    glFogf(GL_FOG_END, ptr[2]);
                    break;
                case GpuProgramParameters::ACT_FOG_COLOUR:
                    glFogfv(GL_FOG_COLOR, ptr);
                    break;
                case GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR:
                    mStateCacheManager->setLightAmbient(ptr[0], ptr[1], ptr[2]);
                    break;
                case GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR:
                    glLightfv(GL_LIGHT0 + ac.data, GL_DIFFUSE, ptr);
                    break;
                case GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR:
                    glLightfv(GL_LIGHT0 + ac.data, GL_SPECULAR, ptr);
                    break;
                case GpuProgramParameters::ACT_LIGHT_ATTENUATION:
                    glLightf(GL_LIGHT0 + ac.data, GL_CONSTANT_ATTENUATION, ptr[1]);
                    glLightf(GL_LIGHT0 + ac.data, GL_LINEAR_ATTENUATION, ptr[2]);
                    glLightf(GL_LIGHT0 + ac.data, GL_QUADRATIC_ATTENUATION, ptr[3]);
                    break;
                case GpuProgramParameters::ACT_SPOTLIGHT_PARAMS:
                {
                    float cutoff = ptr[3] ? Math::RadiansToDegrees(std::acos(ptr[1])) : 180;
                    glLightf(GL_LIGHT0 + ac.data, GL_SPOT_CUTOFF, cutoff);
                    glLightf(GL_LIGHT0 + ac.data, GL_SPOT_EXPONENT, ptr[2]);
                    break;
                }
                case GpuProgramParameters::ACT_LIGHT_POSITION:
                case GpuProgramParameters::ACT_LIGHT_DIRECTION:
                    // handled below
                    updateLightPos = true;
                    break;
                default:
                    OgreAssert(false, "unknown autoconstant");
                    break;
                }
            }
        }

        if(!updateLightPos) return;

        // GL lights use eye coordinates, which we only know now

        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(Matrix4f(mViewMatrix.transpose())[0]);

        for (const auto& ac : params->getAutoConstants())
        {
            // Only update needed slots
            if ((GPV_GLOBAL | GPV_LIGHTS) & mask)
            {
                const float* ptr = params->getFloatPointer(ac.physicalIndex);
                switch(ac.paramType)
                {
                case GpuProgramParameters::ACT_LIGHT_POSITION:
                    glLightfv(GL_LIGHT0 + ac.data, GL_POSITION, ptr);
                    break;
                case GpuProgramParameters::ACT_LIGHT_DIRECTION:
                    glLightfv(GL_LIGHT0 + ac.data, GL_SPOT_DIRECTION, ptr);
                    break;
                default:
                    break;
                }
            }
        }
        glPopMatrix();
    }

    const String& GLRenderSystem::getName(void) const
    {
        static String strName("OpenGL Rendering Subsystem");
        return strName;
    }

    void GLRenderSystem::_initialise()
    {
        RenderSystem::_initialise();

        mGLSupport->start();

        // Create the texture manager
        mTextureManager = new GLTextureManager(this);
    }

    void GLRenderSystem::initConfigOptions()
    {
        GLRenderSystemCommon::initConfigOptions();

        ConfigOption optRTTMode;
        optRTTMode.name = "RTT Preferred Mode";
        optRTTMode.possibleValues = {"FBO", "PBuffer", "Copy"};
        optRTTMode.currentValue = optRTTMode.possibleValues[0];
        optRTTMode.immutable = true;
        mOptions[optRTTMode.name] = optRTTMode;

        ConfigOption opt;
        opt.name = "Fixed Pipeline Enabled";
        opt.possibleValues = {"Yes", "No"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;
    }

    RenderSystemCapabilities* GLRenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();

        rsc->setCategoryRelevant(CAPS_CATEGORY_GL, true);
        rsc->setDriverVersion(mDriverVersion);
        const char* deviceName = (const char*)glGetString(GL_RENDERER);
        rsc->setDeviceName(deviceName);
        rsc->setRenderSystemName(getName());
        rsc->setVendor(mVendor);

        if (mEnableFixedPipeline)
        {
            // Supports fixed-function
            rsc->setCapability(RSC_FIXED_FUNCTION);
        }

        rsc->setCapability(RSC_AUTOMIPMAP_COMPRESSED);

        // Check for Multitexturing support and set number of texture units
        GLint units;
        glGetIntegerv( GL_MAX_TEXTURE_UNITS, &units );

        if (GLAD_GL_ARB_fragment_program)
        {
            // Also check GL_MAX_TEXTURE_IMAGE_UNITS_ARB since NV at least
            // only increased this on the FX/6x00 series
            GLint arbUnits;
            glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &arbUnits );
            if (arbUnits > units)
                units = arbUnits;
        }
        rsc->setNumTextureUnits(std::min(OGRE_MAX_TEXTURE_LAYERS, units));

        // Check for Anisotropy support
        if(GLAD_GL_EXT_texture_filter_anisotropic)
        {
            GLfloat maxAnisotropy = 0;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            rsc->setMaxSupportedAnisotropy(maxAnisotropy);
            rsc->setCapability(RSC_ANISOTROPY);
        }

        // Point sprites
        if (GLAD_GL_VERSION_2_0 || GLAD_GL_ARB_point_sprite)
        {
            rsc->setCapability(RSC_POINT_SPRITES);
        }

        if(GLAD_GL_ARB_point_parameters)
        {
            glPointParameterf = glPointParameterfARB;
            glPointParameterfv = glPointParameterfvARB;
        }
        else if(GLAD_GL_EXT_point_parameters)
        {
            glPointParameterf = glPointParameterfEXT;
            glPointParameterfv = glPointParameterfvEXT;
        }

        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);


        // Check for hardware stencil support and set bit depth
        GLint stencil;
        glGetIntegerv(GL_STENCIL_BITS,&stencil);

        if(stencil)
        {
            rsc->setCapability(RSC_HWSTENCIL);
        }

        rsc->setCapability(RSC_HW_GAMMA);

        rsc->setCapability(RSC_MAPBUFFER);
        rsc->setCapability(RSC_32BIT_INDEX);

        if(GLAD_GL_ARB_vertex_program)
        {
            rsc->setCapability(RSC_VERTEX_PROGRAM);

            // Vertex Program Properties
            GLint floatConstantCount;
            glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &floatConstantCount);
            rsc->setVertexProgramConstantFloatCount(floatConstantCount);

            GLint attrs;
            glGetIntegerv( GL_MAX_VERTEX_ATTRIBS_ARB, &attrs);
            rsc->setNumVertexAttributes(attrs);

            rsc->addShaderProfile("arbvp1");
            if (GLAD_GL_NV_vertex_program2_option)
            {
                rsc->addShaderProfile("vp30");
            }

            if (GLAD_GL_NV_vertex_program3)
            {
                rsc->addShaderProfile("vp40");
            }

            if (GLAD_GL_NV_gpu_program4)
            {
                rsc->addShaderProfile("gp4vp");
                rsc->addShaderProfile("gpu_vp");
            }
        }

        if (GLAD_GL_NV_register_combiners2 &&
            GLAD_GL_NV_texture_shader)
        {
            rsc->addShaderProfile("fp20");
        }

        // NFZ - check for ATI fragment shader support
        if (GLAD_GL_ATI_fragment_shader)
        {
            // only 8 Vector4 constant floats supported
            rsc->setFragmentProgramConstantFloatCount(8);

            rsc->addShaderProfile("ps_1_4");
            rsc->addShaderProfile("ps_1_3");
            rsc->addShaderProfile("ps_1_2");
            rsc->addShaderProfile("ps_1_1");
        }

        if (GLAD_GL_ARB_fragment_program)
        {
            // Fragment Program Properties
            GLint floatConstantCount;
            glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &floatConstantCount);
            rsc->setFragmentProgramConstantFloatCount(floatConstantCount);

            rsc->addShaderProfile("arbfp1");
            if (GLAD_GL_NV_fragment_program_option)
            {
                rsc->addShaderProfile("fp30");
            }

            if (GLAD_GL_NV_fragment_program2)
            {
                rsc->addShaderProfile("fp40");
            }

            if (GLAD_GL_NV_gpu_program4)
            {
                rsc->addShaderProfile("gp4fp");
                rsc->addShaderProfile("gpu_fp");
            }
        }

        // NFZ - Check if GLSL is supported
        if ( GLAD_GL_VERSION_2_0 ||
             (GLAD_GL_ARB_shading_language_100 &&
              GLAD_GL_ARB_shader_objects &&
              GLAD_GL_ARB_fragment_shader &&
              GLAD_GL_ARB_vertex_shader) )
        {
            rsc->addShaderProfile("glsl");
            if(getNativeShadingLanguageVersion() >= 120)
                rsc->addShaderProfile("glsl120");
            if(getNativeShadingLanguageVersion() >= 110)
                rsc->addShaderProfile("glsl110");
            if(getNativeShadingLanguageVersion() >= 100)
                rsc->addShaderProfile("glsl100");
        }

        // Check if geometry shaders are supported
        if (hasMinGLVersion(3, 2) || (GLAD_GL_VERSION_2_0 && GLAD_GL_EXT_geometry_shader4))
        {
            rsc->setCapability(RSC_GEOMETRY_PROGRAM);
            GLint floatConstantCount = 0;
            glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT, &floatConstantCount);
            rsc->setGeometryProgramConstantFloatCount(floatConstantCount/4);

            GLint maxOutputVertices;
            glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&maxOutputVertices);
            rsc->setGeometryProgramNumOutputVertices(maxOutputVertices);
        }

        if(GLAD_GL_NV_gpu_program4)
        {
            rsc->setCapability(RSC_GEOMETRY_PROGRAM);
            rsc->addShaderProfile("nvgp4");

            //Also add the CG profiles
            rsc->addShaderProfile("gpu_gp");
            rsc->addShaderProfile("gp4gp");
        }

        if (checkExtension("GL_ARB_get_program_binary"))
        {
            // states 3.0 here: http://developer.download.nvidia.com/opengl/specs/GL_ARB_get_program_binary.txt
            // but not here: http://www.opengl.org/sdk/docs/man4/xhtml/glGetProgramBinary.xml
            // and here states 4.1: http://www.geeks3d.com/20100727/opengl-4-1-allows-the-use-of-binary-shaders/
            GLint formats;
            glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);

            if(formats > 0)
                rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);
        }

        if (hasMinGLVersion(3, 3) || GLAD_GL_ARB_instanced_arrays)
        {
            // states 3.3 here: http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribDivisor.xml
            rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        }

        //Check if render to vertex buffer (transform feedback in OpenGL)
        if (GLAD_GL_VERSION_2_0 &&
            GLAD_GL_NV_transform_feedback)
        {
            rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);
        }

        // Check for texture compression
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);

        // Check for dxt compression
        if(GLAD_GL_EXT_texture_compression_s3tc)
        {
#if defined(__APPLE__) && defined(__PPC__)
            // Apple on ATI & PPC has errors in DXT
            if (mGLSupport->getGLVendor().find("ATI") == std::string::npos)
#endif
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
        }
        // Check for vtc compression
        if(GLAD_GL_NV_texture_compression_vtc)
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_VTC);
        }

        // As are user clipping planes
        rsc->setCapability(RSC_USER_CLIP_PLANES);

        // 2-sided stencil?
        if (GLAD_GL_VERSION_2_0 || GLAD_GL_EXT_stencil_two_side)
        {
            rsc->setCapability(RSC_TWO_SIDED_STENCIL);
        }
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_HWOCCLUSION);

        // Check for non-power-of-2 texture support
        if(GLAD_GL_ARB_texture_non_power_of_two)
        {
            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        }

        // Check for Float textures
        if(GLAD_GL_ATI_texture_float || GLAD_GL_ARB_texture_float)
        {
            rsc->setCapability(RSC_TEXTURE_FLOAT);
        }

        // 3D textures should be supported by GL 1.2, which is our minimum version
        rsc->setCapability(RSC_TEXTURE_1D);
        rsc->setCapability(RSC_TEXTURE_3D);

        if(hasMinGLVersion(3, 0) || GLAD_GL_EXT_texture_array)
            rsc->setCapability(RSC_TEXTURE_2D_ARRAY);

        // Check for framebuffer object extension
        if(GLAD_GL_EXT_framebuffer_object)
        {
            // Probe number of draw buffers
            // Only makes sense with FBO support, so probe here
            if(GLAD_GL_VERSION_2_0 ||
               GLAD_GL_ARB_draw_buffers ||
               GLAD_GL_ATI_draw_buffers)
            {
                GLint buffers;
                glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &buffers);
                rsc->setNumMultiRenderTargets(std::min<int>(buffers, (GLint)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
                rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);

            }
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        }

        // Check GLSupport for PBuffer support
        if(GLAD_GL_ARB_pixel_buffer_object || GLAD_GL_EXT_pixel_buffer_object)
        {
            // Use PBuffers
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
            rsc->setCapability(RSC_PBUFFER);
        }

        // Point size
        float ps;
        glGetFloatv(GL_POINT_SIZE_MAX, &ps);
        rsc->setMaxPointSize(ps);

        // Vertex texture fetching
        if (checkExtension("GL_ARB_vertex_shader"))
        {
            GLint vUnits;
            glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &vUnits);
            rsc->setNumVertexTextureUnits(static_cast<ushort>(vUnits));
            if (vUnits > 0)
            {
                rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
            }
        }

        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // Alpha to coverage?
        if (checkExtension("GL_ARB_multisample"))
        {
            // Alpha to coverage always 'supported' when MSAA is available
            // although card may ignore it if it doesn't specifically support A2C
            rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
        }

        GLfloat lineWidth[2] = {1, 1};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidth);
        if(lineWidth[1] != 1 && lineWidth[1] != lineWidth[0])
            rsc->setCapability(RSC_WIDE_LINES);

        return rsc;
    }

    void GLRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        // set texture the number of texture units
        mFixedFunctionTextureUnits = caps->getNumTextureUnits();

        //In GL there can be less fixed function texture units than general
        //texture units. Get the minimum of the two.
        if (caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            GLint maxTexCoords = 0;
            glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &maxTexCoords);
            if (mFixedFunctionTextureUnits > maxTexCoords)
            {
                mFixedFunctionTextureUnits = maxTexCoords;
            }
        }

        if(!GLAD_GL_ARB_vertex_buffer_object)
        {
            // Assign ARB functions same to GL 1.5 version since
            // interface identical
            glBindBufferARB = glBindBuffer;
            glBufferDataARB = glBufferData;
            glBufferSubDataARB = glBufferSubData;
            glDeleteBuffersARB = glDeleteBuffers;
            glGenBuffersARB = glGenBuffers;
            glGetBufferParameterivARB = glGetBufferParameteriv;
            glGetBufferPointervARB = glGetBufferPointerv;
            glGetBufferSubDataARB = glGetBufferSubData;
            glIsBufferARB = glIsBuffer;
            glMapBufferARB = glMapBuffer;
            glUnmapBufferARB = glUnmapBuffer;
        }

        mHardwareBufferManager = new GLHardwareBufferManager;

        // XXX Need to check for nv2 support and make a program manager for it
        // XXX Probably nv1 as well for older cards
        // GPU Program Manager setup
        mGpuProgramManager = new GLGpuProgramManager();

        if(caps->hasCapability(RSC_VERTEX_PROGRAM))
        {
            if(caps->isShaderProfileSupported("arbvp1"))
            {
                mGpuProgramManager->registerProgramFactory("arbvp1", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("vp30"))
            {
                mGpuProgramManager->registerProgramFactory("vp30", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("vp40"))
            {
                mGpuProgramManager->registerProgramFactory("vp40", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("gp4vp"))
            {
                mGpuProgramManager->registerProgramFactory("gp4vp", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("gpu_vp"))
            {
                mGpuProgramManager->registerProgramFactory("gpu_vp", createGLArbGpuProgram);
            }
        }

        if(caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            //TODO : Should these be createGLArbGpuProgram or createGLGpuNVparseProgram?
            if(caps->isShaderProfileSupported("nvgp4"))
            {
                mGpuProgramManager->registerProgramFactory("nvgp4", createGLArbGpuProgram);
            }
            if(caps->isShaderProfileSupported("gp4gp"))
            {
                mGpuProgramManager->registerProgramFactory("gp4gp", createGLArbGpuProgram);
            }
            if(caps->isShaderProfileSupported("gpu_gp"))
            {
                mGpuProgramManager->registerProgramFactory("gpu_gp", createGLArbGpuProgram);
            }
        }

        if(caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {

            if(caps->isShaderProfileSupported("fp20"))
            {
                mGpuProgramManager->registerProgramFactory("fp20", createGLGpuNvparseProgram);
            }

            if(caps->isShaderProfileSupported("ps_1_4"))
            {
                mGpuProgramManager->registerProgramFactory("ps_1_4", createGL_ATI_FS_GpuProgram);
            }

            if(caps->isShaderProfileSupported("ps_1_3"))
            {
                mGpuProgramManager->registerProgramFactory("ps_1_3", createGL_ATI_FS_GpuProgram);
            }

            if(caps->isShaderProfileSupported("ps_1_2"))
            {
                mGpuProgramManager->registerProgramFactory("ps_1_2", createGL_ATI_FS_GpuProgram);
            }

            if(caps->isShaderProfileSupported("ps_1_1"))
            {
                mGpuProgramManager->registerProgramFactory("ps_1_1", createGL_ATI_FS_GpuProgram);
            }

            if(caps->isShaderProfileSupported("arbfp1"))
            {
                mGpuProgramManager->registerProgramFactory("arbfp1", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("fp40"))
            {
                mGpuProgramManager->registerProgramFactory("fp40", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("fp30"))
            {
                mGpuProgramManager->registerProgramFactory("fp30", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("gp4fp"))
            {
                mGpuProgramManager->registerProgramFactory("gp4fp", createGLArbGpuProgram);
            }

            if(caps->isShaderProfileSupported("gpu_fp"))
            {
                mGpuProgramManager->registerProgramFactory("gpu_fp", createGLArbGpuProgram);
            }

        }

        if(caps->isShaderProfileSupported("glsl"))
        {
            // NFZ - check for GLSL vertex and fragment shader support successful
            mGLSLProgramFactory = new GLSL::GLSLProgramFactory();
            HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLProgramFactory);
            LogManager::getSingleton().logMessage("GLSL support detected");
        }

        if(caps->hasCapability(RSC_HWOCCLUSION) && !GLAD_GL_ARB_occlusion_query)
        {
            // Assign ARB functions same to GL 1.5 version since
            // interface identical
            glBeginQueryARB = glBeginQuery;
            glDeleteQueriesARB = glDeleteQueries;
            glEndQueryARB = glEndQuery;
            glGenQueriesARB = glGenQueries;
            glGetQueryObjectivARB = glGetQueryObjectiv;
            glGetQueryObjectuivARB = glGetQueryObjectuiv;
            glGetQueryivARB = glGetQueryiv;
            glIsQueryARB = glIsQuery;
        }


        /// Do this after extension function pointers are initialised as the extension
        /// is used to probe further capabilities.
        auto cfi = getConfigOptions().find("RTT Preferred Mode");
        // RTT Mode: 0 use whatever available, 1 use PBuffers, 2 force use copying
        int rttMode = 0;
        if (cfi != getConfigOptions().end())
        {
            if (cfi->second.currentValue == "PBuffer")
            {
                rttMode = 1;
            }
            else if (cfi->second.currentValue == "Copy")
            {
                rttMode = 2;
            }
        }




        // Check for framebuffer object extension
        if(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE) && rttMode < 1)
        {
            // Before GL version 2.0, we need to get one of the extensions
            if(GLAD_GL_ARB_draw_buffers)
                glDrawBuffers = glDrawBuffersARB;
            else if(GLAD_GL_ATI_draw_buffers)
                glDrawBuffers = glDrawBuffersATI;

            // Create FBO manager
            LogManager::getSingleton().logMessage("GL: Using GL_EXT_framebuffer_object for rendering to textures (best)");
            mRTTManager = new GLFBOManager(false);
            //TODO: Check if we're using OpenGL 3.0 and add RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL flag
        }
        else
        {
            // Check GLSupport for PBuffer support
            if(caps->hasCapability(RSC_PBUFFER) && rttMode < 2)
            {
                if(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE))
                {
                    // Use PBuffers
                    mRTTManager = new GLPBRTTManager(mGLSupport, primary);
                    LogManager::getSingleton().logWarning("GL: Using PBuffers for rendering to textures");

                    //TODO: Depth buffer sharing in pbuffer is left unsupported
                }
            }
            else
            {
                // No pbuffer support either -- fallback to simplest copying from framebuffer
                mRTTManager = new GLCopyingRTTManager();
                LogManager::getSingleton().logWarning("GL: Using framebuffer copy for rendering to textures (worst)");
                LogManager::getSingleton().logWarning("GL: RenderTexture size is restricted to size of framebuffer. If you are on Linux, consider using GLX instead of SDL.");

                //Copy method uses the main depth buffer but no other depth buffer
                caps->setCapability(RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE);
                caps->setCapability(RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL);
            }

            // Downgrade number of simultaneous targets
            caps->setNumMultiRenderTargets(1);
        }

        mGLInitialised = true;
    }

    void GLRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        // Deleting the GLSL program factory
        if (mGLSLProgramFactory)
        {
            // Remove from manager safely
            if (HighLevelGpuProgramManager::getSingletonPtr())
                HighLevelGpuProgramManager::getSingleton().removeFactory(mGLSLProgramFactory);
            delete mGLSLProgramFactory;
            mGLSLProgramFactory = 0;
        }

        // Delete extra threads contexts
        for (auto pCurContext : mBackgroundContextList)
        {
            pCurContext->releaseContext();
            OGRE_DELETE pCurContext;
        }
        mBackgroundContextList.clear();

        // Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
        delete mGpuProgramManager;
        mGpuProgramManager = 0;

        delete mHardwareBufferManager;
        mHardwareBufferManager = 0;

        delete mRTTManager;
        mRTTManager = 0;

        mGLSupport->stop();

        delete mTextureManager;
        mTextureManager = 0;

        // There will be a new initial window and so forth, thus any call to test
        //  some params will access an invalid pointer, so it is best to reset
        //  the whole state.
        mGLInitialised = 0;
    }

    void GLRenderSystem::setShadingType(ShadeOptions so)
    {
        // XXX Don't do this when using shader
        switch(so)
        {
        case SO_FLAT:
            mStateCacheManager->setShadeModel(GL_FLAT);
            break;
        default:
            mStateCacheManager->setShadeModel(GL_SMOOTH);
            break;
        }
    }
    //---------------------------------------------------------------------
    RenderWindow* GLRenderSystem::_createRenderWindow(const String &name,
                                                      unsigned int width, unsigned int height, bool fullScreen,
                                                      const NameValuePairList *miscParams)
    {
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);

        // Create the window
        RenderWindow* win = mGLSupport->newWindow(name, width, height,
                                                  fullScreen, miscParams);

        attachRenderTarget( *win );

        if (!mGLInitialised)
        {
            // set up glew and GLSupport
            initialiseContext(win);

            const char* shadingLangVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
            StringVector tokens = StringUtil::split(shadingLangVersion, ". ");
            mNativeShadingLanguageVersion = (StringConverter::parseUnsignedInt(tokens[0]) * 100) + StringConverter::parseUnsignedInt(tokens[1]);

            auto it = mOptions.find("Fixed Pipeline Enabled");
            if (it != mOptions.end())
            {
                mEnableFixedPipeline = StringConverter::parseBool(it->second.currentValue);
            }

            // Initialise GL after the first window has been created
            // TODO: fire this from emulation options, and don't duplicate Real and Current capabilities
            mRealCapabilities = createRenderSystemCapabilities();
            initFixedFunctionParams(); // create params

            // use real capabilities if custom capabilities are not available
            if(!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities(mCurrentCapabilities, win);

            // Initialise the main context
            _oneTimeContextInitialization();
            if(mCurrentContext)
                mCurrentContext->setInitialized();
        }

        if( win->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH )
        {
            //Unlike D3D9, OGL doesn't allow sharing the main depth buffer, so keep them separate.
            //Only Copy does, but Copy means only one depth buffer...
            GLContext *windowContext = dynamic_cast<GLRenderTarget*>(win)->getContext();;

            auto depthBuffer =
                new GLDepthBufferCommon(DepthBuffer::POOL_DEFAULT, this, windowContext, 0, 0, win, true);

            mDepthBufferPool[depthBuffer->getPoolId()].push_back( depthBuffer );

            win->attachDepthBuffer( depthBuffer );
        }

        return win;
    }
    //---------------------------------------------------------------------
    DepthBuffer* GLRenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        if( auto fbo = dynamic_cast<GLRenderTarget*>(renderTarget)->getFBO() )
        {
            //Find best depth & stencil format suited for the RT's format
            GLuint depthFormat, stencilFormat;
            mRTTManager->getBestDepthStencil(fbo->getFormat(), &depthFormat, &stencilFormat);

            GLRenderBuffer *depthBuffer = new GLRenderBuffer( depthFormat, fbo->getWidth(),
                                                              fbo->getHeight(), fbo->getFSAA() );

            GLRenderBuffer *stencilBuffer = NULL;
            if ( depthFormat == GL_DEPTH24_STENCIL8_EXT)
            {
                // If we have a packed format, the stencilBuffer is the same as the depthBuffer
                stencilBuffer = depthBuffer;
            }
            else if(stencilFormat)
            {
                stencilBuffer = new GLRenderBuffer( stencilFormat, fbo->getWidth(),
                                                    fbo->getHeight(), fbo->getFSAA() );
            }

            return new GLDepthBufferCommon(0, this, mCurrentContext, depthBuffer, stencilBuffer,
                                           renderTarget, false);
        }

        return NULL;
    }

    void GLRenderSystem::initialiseContext(RenderWindow* primary)
    {
        // Set main and current context
        mMainContext = dynamic_cast<GLRenderTarget*>(primary)->getContext();
        mCurrentContext = mMainContext;

        // Set primary context as active
        if(mCurrentContext)
            mCurrentContext->setCurrent();

        gladLoadGLLoader(get_proc);

        if (!GLAD_GL_VERSION_1_5) {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "OpenGL 1.5 is not supported",
                        "GLRenderSystem::initialiseContext");
        }

        // Get extension function pointers
        initialiseExtensions();

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GLStateCacheManager>();

        LogManager::getSingleton().logMessage("***************************");
        LogManager::getSingleton().logMessage("*** GL Renderer Started ***");
        LogManager::getSingleton().logMessage("***************************");
    }



    //-----------------------------------------------------------------------
    MultiRenderTarget * GLRenderSystem::createMultiRenderTarget(const String & name)
    {
        auto fboMgr = dynamic_cast<GLFBOManager*>(mRTTManager);
        if (!fboMgr)
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "MultiRenderTarget is not supported");

        MultiRenderTarget *retval = new GLFBOMultiRenderTarget(fboMgr, name);
        attachRenderTarget( *retval );
        return retval;
    }

    //-----------------------------------------------------------------------
    void GLRenderSystem::destroyRenderWindow(const String& name)
    {
        // Find it to remove from list.
        RenderTarget* pWin = detachRenderTarget(name);
        OgreAssert(pWin, "unknown RenderWindow name");

        GLContext *windowContext = dynamic_cast<GLRenderTarget*>(pWin)->getContext();
    
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
                //A DepthBuffer with no depth & stencil pointers is a dummy one,
                //look for the one that matches the same GL context
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

        delete pWin;
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::_useLights(unsigned short limit)
    {
        if(limit == mCurrentLights)
            return;

        unsigned short num = 0;
        for (;num < limit; ++num)
        {
            setGLLight(num, true);
        }
        // Disable extra lights
        for (; num < mCurrentLights; ++num)
        {
            setGLLight(num, false);
        }
        mCurrentLights = limit;
    }

    void GLRenderSystem::setGLLight(size_t index, bool lt)
    {
        setFFPLightParams(index, lt);

        GLenum gl_index = GL_LIGHT0 + index;

        if (!lt)
        {
            // Disable in the scene
            mStateCacheManager->setEnabled(gl_index, false);
        }
        else
        {
            GLfloat f4vals[4] = {0, 0, 0, 1};
            // Disable ambient light for movables;
            glLightfv(gl_index, GL_AMBIENT, f4vals);

            // Enable in the scene
            mStateCacheManager->setEnabled(gl_index, true);
        }
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
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
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setWorldMatrix( const Matrix4 &m )
    {
        mWorldMatrix = m;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(Matrix4f((mViewMatrix * mWorldMatrix).transpose())[0]);
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::setViewMatrix( const Matrix4 &m )
    {
        mViewMatrix = m;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(Matrix4f((mViewMatrix * mWorldMatrix).transpose())[0]);

        // also mark clip planes dirty
        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setProjectionMatrix(const Matrix4 &m)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(Matrix4f(m.transpose())[0]);
        glMatrixMode(GL_MODELVIEW);

        // also mark clip planes dirty
        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setSurfaceTracking(TrackVertexColourType tracking)
    {

        // Track vertex colour
        if(tracking != TVC_NONE)
        {
            GLenum gt = GL_DIFFUSE;
            // There are actually 15 different combinations for tracking, of which
            // GL only supports the most used 5. This means that we have to do some
            // magic to find the best match. NOTE:
            //  GL_AMBIENT_AND_DIFFUSE != GL_AMBIENT | GL__DIFFUSE
            if(tracking & TVC_AMBIENT)
            {
                if(tracking & TVC_DIFFUSE)
                {
                    gt = GL_AMBIENT_AND_DIFFUSE;
                }
                else
                {
                    gt = GL_AMBIENT;
                }
            }
            else if(tracking & TVC_DIFFUSE)
            {
                gt = GL_DIFFUSE;
            }
            else if(tracking & TVC_SPECULAR)
            {
                gt = GL_SPECULAR;
            }
            else if(tracking & TVC_EMISSIVE)
            {
                gt = GL_EMISSION;
            }
            glColorMaterial(GL_FRONT_AND_BACK, gt);

            mStateCacheManager->setEnabled(GL_COLOR_MATERIAL, true);
        }
        else
        {
            mStateCacheManager->setEnabled(GL_COLOR_MATERIAL, false);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setPointParameters(bool attenuationEnabled, Real minSize, Real maxSize)
    {
        if(attenuationEnabled)
        {
            // Point size is still calculated in pixels even when attenuation is
            // enabled, which is pretty awkward, since you typically want a viewport
            // independent size if you're looking for attenuation.
            // So, scale the point size up by viewport size (this is equivalent to
            // what D3D does as standard)
            minSize = minSize * mActiveViewport->getActualHeight();
            if (maxSize == 0.0f)
                maxSize = mCurrentCapabilities->getMaxPointSize(); // pixels
            else
                maxSize = maxSize * mActiveViewport->getActualHeight();

            if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
                mStateCacheManager->setEnabled(GL_VERTEX_PROGRAM_POINT_SIZE, true);
        }
        else
        {
            if (maxSize == 0.0f)
                maxSize = mCurrentCapabilities->getMaxPointSize();
            if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
                mStateCacheManager->setEnabled(GL_VERTEX_PROGRAM_POINT_SIZE, false);
        }

        mStateCacheManager->setPointParameters(NULL, minSize, maxSize);
    }

    void GLRenderSystem::_setLineWidth(float width)
    {
        glLineWidth(width);
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::_setPointSpritesEnabled(bool enabled)
    {
        if (!getCapabilities()->hasCapability(RSC_POINT_SPRITES))
            return;

        mStateCacheManager->setEnabled(GL_POINT_SPRITE, enabled);

        // Set sprite texture coord generation
        // Don't offer this as an option since D3D links it to sprite enabled
        for (ushort i = 0; i < mFixedFunctionTextureUnits; ++i)
        {
            mStateCacheManager->activateGLTextureUnit(i);
            glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE,
                      enabled ? GL_TRUE : GL_FALSE);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
        GLenum lastTextureType = mTextureTypes[stage];

        mStateCacheManager->activateGLTextureUnit(stage);

        if (enabled)
        {
            GLTexturePtr tex = static_pointer_cast<GLTexture>(texPtr);

            // note used
            tex->touch();
            mTextureTypes[stage] = tex->getGLTextureTarget();

            if(lastTextureType != mTextureTypes[stage] && lastTextureType != 0)
            {
                if (stage < mFixedFunctionTextureUnits)
                {
                    if(lastTextureType != GL_TEXTURE_2D_ARRAY_EXT)
                        glDisable( lastTextureType );
                }
            }

            if (stage < mFixedFunctionTextureUnits)
            {
                if(mTextureTypes[stage] != GL_TEXTURE_2D_ARRAY_EXT)
                    glEnable( mTextureTypes[stage] );
            }

            mStateCacheManager->bindGLTexture( mTextureTypes[stage], tex->getGLID() );
        }
        else
        {
            if (stage < mFixedFunctionTextureUnits)
            {
                if (lastTextureType != 0)
                {
                    if(mTextureTypes[stage] != GL_TEXTURE_2D_ARRAY_EXT)
                        glDisable( mTextureTypes[stage] );
                }
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }
            // bind zero texture
            mStateCacheManager->bindGLTexture(GL_TEXTURE_2D, 0);
        }
    }

    void GLRenderSystem::_setSampler(size_t unit, Sampler& sampler)
    {
        mStateCacheManager->activateGLTextureUnit(unit);

        GLenum target = mTextureTypes[unit];

        const Sampler::UVWAddressingMode& uvw = sampler.getAddressingMode();
        mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u));
        mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v));
        mStateCacheManager->setTexParameteri(target, GL_TEXTURE_WRAP_R, getTextureAddressingMode(uvw.w));

        if (uvw.u == TAM_BORDER || uvw.v == TAM_BORDER || uvw.w == TAM_BORDER)
            glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, sampler.getBorderColour().ptr());

        if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
        {
            glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, sampler.getMipmapBias());
        }

        if (mCurrentCapabilities->hasCapability(RSC_ANISOTROPY))
            mStateCacheManager->setTexParameteri(
                target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                std::min<uint>(mCurrentCapabilities->getMaxSupportedAnisotropy(), sampler.getAnisotropy()));

        if(GLAD_GL_VERSION_2_0)
        {
            mStateCacheManager->setTexParameteri(target, GL_TEXTURE_COMPARE_MODE,
                                                 sampler.getCompareEnabled() ? GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT
                                                                             : GL_NONE);
            if (sampler.getCompareEnabled())
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

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
        mTextureCoordIndex[stage] = index;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m,
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

        GLfloat eyePlaneS[] = {1.0, 0.0, 0.0, 0.0};
        GLfloat eyePlaneT[] = {0.0, 1.0, 0.0, 0.0};
        GLfloat eyePlaneR[] = {0.0, 0.0, 1.0, 0.0};
        GLfloat eyePlaneQ[] = {0.0, 0.0, 0.0, 1.0};

        mStateCacheManager->activateGLTextureUnit(stage);

        switch( m )
        {
        case TEXCALC_NONE:
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_S );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_T );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_R );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_Q );
            break;

        case TEXCALC_ENVIRONMENT_MAP_PLANAR:
            // should be view position .xy, according to doc, but we want to match OGRE D3D9 behaviour here
        case TEXCALC_ENVIRONMENT_MAP:
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_S );
            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_T );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_R );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_Q );

            // Need to use a texture matrix to flip the spheremap
            mUseAutoTextureMatrix = true;
            memset(mAutoTextureMatrix, 0, sizeof(GLfloat)*16);
            mAutoTextureMatrix[0] = mAutoTextureMatrix[10] = mAutoTextureMatrix[15] = 1.0f;
            mAutoTextureMatrix[5] = -1.0f;

            break;

        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_S );
            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_T );
            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_R );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_Q );

            // We need an extra texture matrix here
            // This sets the texture matrix to be the inverse of the view matrix
            mUseAutoTextureMatrix = true;
            makeGLMatrix( M, mViewMatrix);

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
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );

            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_S );
            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_T );
            mStateCacheManager->enableTextureCoordGen( GL_TEXTURE_GEN_R );
            mStateCacheManager->disableTextureCoordGen( GL_TEXTURE_GEN_Q );
            break;
        case TEXCALC_PROJECTIVE_TEXTURE:
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGenfv(GL_S, GL_EYE_PLANE, eyePlaneS);
            glTexGenfv(GL_T, GL_EYE_PLANE, eyePlaneT);
            glTexGenfv(GL_R, GL_EYE_PLANE, eyePlaneR);
            glTexGenfv(GL_Q, GL_EYE_PLANE, eyePlaneQ);
            mStateCacheManager->enableTextureCoordGen(GL_TEXTURE_GEN_S);
            mStateCacheManager->enableTextureCoordGen(GL_TEXTURE_GEN_T);
            mStateCacheManager->enableTextureCoordGen(GL_TEXTURE_GEN_R);
            mStateCacheManager->enableTextureCoordGen(GL_TEXTURE_GEN_Q);

            mUseAutoTextureMatrix = true;

            // Set scale and translation matrix for projective textures
            projectionBias = Matrix4::CLIPSPACE2DTOIMAGESPACE;

            projectionBias = projectionBias * frustum->getProjectionMatrix();
            if(mTexProjRelative)
            {
                Matrix4 viewMatrix;
                frustum->calcViewMatrixRelative(mTexProjRelativeOrigin, viewMatrix);
                projectionBias = projectionBias * viewMatrix;
            }
            else
            {
                projectionBias = projectionBias * frustum->getViewMatrix();
            }
            projectionBias = projectionBias * mWorldMatrix;

            makeGLMatrix(mAutoTextureMatrix, projectionBias);
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------------
    GLint GLRenderSystem::getTextureAddressingMode(
        TextureAddressingMode tam) const
    {
        switch(tam)
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
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xform)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

        mStateCacheManager->activateGLTextureUnit(stage);
        glMatrixMode(GL_TEXTURE);

        // Load this matrix in
        glLoadMatrixf(Matrix4f(xform.transpose())[0]);

        if (mUseAutoTextureMatrix)
        {
            // Concat auto matrix
            glMultMatrixf(mAutoTextureMatrix);
        }

        glMatrixMode(GL_MODELVIEW);
    }
    //-----------------------------------------------------------------------------
    GLint GLRenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
    {
        switch(ogreBlend)
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
        // to keep compiler happy
        return GL_ONE;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
    {
        bool enable = func != CMPF_ALWAYS_PASS;

        mStateCacheManager->setEnabled(GL_ALPHA_TEST, enable);

        if(enable)
        {
            glAlphaFunc(convertCompareFunction(func), value / 255.0f);
        }

        if (getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
        {
            mStateCacheManager->setEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE, alphaToCoverage && enable);
        }

    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setViewport(Viewport *vp)
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
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_endFrame(void)
    {
        // unbind GPU programs at end of frame
        // this is mostly to avoid holding bound programs that might get deleted
        // outside via the resource manager
        unbindGpuProgram(GPT_VERTEX_PROGRAM);
        unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setCullingMode(CullingMode mode)
    {
        mCullingMode = mode;

        GLenum cullMode;
        bool flip = flipFrontFace();
        glFrontFace(flip ? GL_CW : GL_CCW);

        switch( mode )
        {
        case CULL_NONE:
            mStateCacheManager->setEnabled( GL_CULL_FACE, false );
            return;
        case CULL_CLOCKWISE:
            cullMode = GL_BACK;
            break;
        case CULL_ANTICLOCKWISE:
            cullMode = GL_FRONT;
            break;
        }

        mStateCacheManager->setEnabled( GL_CULL_FACE, true );
        mStateCacheManager->setCullFace( cullMode );
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        if (depthTest)
        {
            mStateCacheManager->setClearDepth(1.0f);
        }
        mStateCacheManager->setEnabled(GL_DEPTH_TEST, depthTest);
        mStateCacheManager->setDepthMask( depthWrite );
        // Store for reference in _beginFrame
        mDepthWrite = depthWrite;
        mStateCacheManager->setDepthFunc(convertCompareFunction(depthFunction));
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        bool enable = constantBias != 0 || slopeScaleBias != 0;
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_FILL, enable);
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_POINT, enable);
        mStateCacheManager->setEnabled(GL_POLYGON_OFFSET_LINE, enable);

        if (enable)
        {
            glPolygonOffset(-slopeScaleBias, -constantBias);
        }
    }
    //-----------------------------------------------------------------------------
    static GLenum getBlendOp(SceneBlendOperation op)
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
            return GL_MIN;
        case SBO_MAX:
            return GL_MAX;
        }
        return GL_FUNC_ADD;
    }
    void GLRenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        // record this
        mCurrentBlend = state;

        if (state.blendingEnabled())
        {
            mStateCacheManager->setEnabled(GL_BLEND, true);
            mStateCacheManager->setBlendFunc(
                getBlendMode(state.sourceFactor), getBlendMode(state.destFactor),
                getBlendMode(state.sourceFactorAlpha), getBlendMode(state.destFactorAlpha));
        }
        else
        {
            mStateCacheManager->setEnabled(GL_BLEND, false);
        }

        mStateCacheManager->setBlendEquation(getBlendOp(state.operation), getBlendOp(state.alphaOperation));
        mStateCacheManager->setColourMask(state.writeR, state.writeG, state.writeB, state.writeA);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setLightingEnabled(bool enabled)
    {
        mStateCacheManager->setEnabled(GL_LIGHTING, enabled);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setFog(FogMode mode)
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
            mStateCacheManager->setEnabled(GL_FOG, false);
            mFixedFunctionParams->clearAutoConstant(18);
            mFixedFunctionParams->clearAutoConstant(19);
            return;
        }

        mFixedFunctionParams->setAutoConstant(18, GpuProgramParameters::ACT_FOG_PARAMS);
        mFixedFunctionParams->setAutoConstant(19, GpuProgramParameters::ACT_FOG_COLOUR);
        mStateCacheManager->setEnabled(GL_FOG, true);
        glFogi(GL_FOG_MODE, fogMode);
    }

    void GLRenderSystem::_setPolygonMode(PolygonMode level)
    {
        GLenum glmode;
        switch(level)
        {
        case PM_POINTS:
            glmode = GL_POINT;
            break;
        case PM_WIREFRAME:
            glmode = GL_LINE;
            break;
        default:
        case PM_SOLID:
            glmode = GL_FILL;
            break;
        }
        mStateCacheManager->setPolygonMode(glmode);
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setStencilState(const StencilState& state)
    {
        mStateCacheManager->setEnabled(GL_STENCIL_TEST, state.enabled);

        if(!state.enabled)
            return;

        bool flip = false;
        mStencilWriteMask = state.writeMask;

        auto compareOp = convertCompareFunction(state.compareOp);

        if (state.twoSidedOperation)
        {
            if (!mCurrentCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported");

            if(GLAD_GL_VERSION_2_0) // New GL2 commands
            {
                // Back
                glStencilMaskSeparate(GL_BACK, state.writeMask);
                glStencilFuncSeparate(GL_BACK, compareOp, state.referenceValue, state.compareMask);
                glStencilOpSeparate(GL_BACK, 
                    convertStencilOp(state.stencilFailOp, !flip),
                    convertStencilOp(state.depthFailOp, !flip),
                    convertStencilOp(state.depthStencilPassOp, !flip));
                // Front
                glStencilMaskSeparate(GL_FRONT, state.writeMask);
                glStencilFuncSeparate(GL_FRONT, compareOp, state.referenceValue, state.compareMask);
                glStencilOpSeparate(GL_FRONT, 
                    convertStencilOp(state.stencilFailOp, flip),
                    convertStencilOp(state.depthFailOp, flip),
                    convertStencilOp(state.depthStencilPassOp, flip));
            }
            else // EXT_stencil_two_side
            {
                mStateCacheManager->setEnabled(GL_STENCIL_TEST_TWO_SIDE_EXT, true);
                // Back
                glActiveStencilFaceEXT(GL_BACK);
                mStateCacheManager->setStencilMask(state.writeMask);
                glStencilFunc(compareOp, state.referenceValue, state.compareMask);
                glStencilOp(
                    convertStencilOp(state.stencilFailOp, !flip),
                    convertStencilOp(state.depthFailOp, !flip),
                    convertStencilOp(state.depthStencilPassOp, !flip));
                // Front
                glActiveStencilFaceEXT(GL_FRONT);
                mStateCacheManager->setStencilMask(state.writeMask);
                glStencilFunc(compareOp, state.referenceValue, state.compareMask);
                glStencilOp(
                    convertStencilOp(state.stencilFailOp, flip),
                    convertStencilOp(state.depthFailOp, flip),
                    convertStencilOp(state.depthStencilPassOp, flip));
            }
        }
        else
        {
            if(!GLAD_GL_VERSION_2_0)
                mStateCacheManager->setEnabled(GL_STENCIL_TEST_TWO_SIDE_EXT, false);

            flip = false;
            mStateCacheManager->setStencilMask(state.writeMask);
            glStencilFunc(compareOp, state.referenceValue, state.compareMask);
            glStencilOp(
                convertStencilOp(state.stencilFailOp, flip),
                convertStencilOp(state.depthFailOp, flip),
                convertStencilOp(state.depthStencilPassOp, flip));
        }
    }
    //---------------------------------------------------------------------
    GLint GLRenderSystem::convertCompareFunction(CompareFunction func) const
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
        // to keep compiler happy
        return GL_ALWAYS;
    }
    //---------------------------------------------------------------------
    GLint GLRenderSystem::convertStencilOp(StencilOperation op, bool invert) const
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
            return invert ? GL_DECR_WRAP_EXT : GL_INCR_WRAP_EXT;
        case SOP_DECREMENT_WRAP:
            return invert ? GL_INCR_WRAP_EXT : GL_DECR_WRAP_EXT;
        case SOP_INVERT:
            return GL_INVERT;
        };
        // to keep compiler happy
        return SOP_KEEP;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

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
            // XXX
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
            // XXX
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
            cmd = GL_DOT3_RGB;
            break;
        default:
            cmd = 0;
        }

        mStateCacheManager->activateGLTextureUnit(stage);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

        if (bm.blendType == LBT_COLOUR)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, cmd);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, src1op);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, src2op);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
        }
        else
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, cmd);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, src1op);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, src2op);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
        }

        float blendValue[4] = {0, 0, 0, static_cast<float>(bm.factor)};
        switch (bm.operation)
        {
        case LBX_BLEND_DIFFUSE_COLOUR:
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PRIMARY_COLOR);
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PRIMARY_COLOR);
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PREVIOUS);
            break;
        case LBX_BLEND_MANUAL:
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendValue);
            break;
        default:
            break;
        };

        switch (bm.operation)
        {
        case LBX_MODULATE_X2:
            glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                      GL_RGB_SCALE : GL_ALPHA_SCALE, 2);
            break;
        case LBX_MODULATE_X4:
            glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                      GL_RGB_SCALE : GL_ALPHA_SCALE, 4);
            break;
        default:
            glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                      GL_RGB_SCALE : GL_ALPHA_SCALE, 1);
            break;
        }

        if (bm.blendType == LBT_COLOUR){
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
            if (bm.operation == LBX_BLEND_DIFFUSE_COLOUR){
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
            } else {
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
            }
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
        if(bm.source1 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv1);
        if (bm.source2 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv2);
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_render(const RenderOperation& op)
    {
        // Call super class
        RenderSystem::_render(op);

        mMaxBuiltInTextureAttribIndex = 0;

        const VertexDeclaration::VertexElementList& decl =
            op.vertexData->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator elemIter, elemEnd;
        elemEnd = decl.end();

        for (elemIter = decl.begin(); elemIter != elemEnd; ++elemIter)
        {
            const VertexElement & elem = *elemIter;
            size_t source = elem.getSource();

            if (!op.vertexData->vertexBufferBinding->isBufferBound(source))
                continue; // skip unbound elements

            HardwareVertexBufferSharedPtr vertexBuffer =
                op.vertexData->vertexBufferBinding->getBuffer(source);

            bindVertexElementToGpu(elem, vertexBuffer, op.vertexData->vertexStart);
        }

        auto numberOfInstances = op.numberOfInstances;

        bool multitexturing = (getCapabilities()->getNumTextureUnits() > 1);
        if (multitexturing)
            glClientActiveTextureARB(GL_TEXTURE0);

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
        case RenderOperation::OT_LINE_LIST_ADJ:
            primType = GL_LINES_ADJACENCY_EXT;
            break;
        case RenderOperation::OT_LINE_STRIP:
            primType = GL_LINE_STRIP;
            break;
        case RenderOperation::OT_LINE_STRIP_ADJ:
            primType = GL_LINE_STRIP_ADJACENCY_EXT;
            break;
        default:
        case RenderOperation::OT_TRIANGLE_LIST:
            primType = GL_TRIANGLES;
            break;
        case RenderOperation::OT_TRIANGLE_LIST_ADJ:
            primType = GL_TRIANGLES_ADJACENCY_EXT;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = GL_TRIANGLE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
            primType = GL_TRIANGLE_STRIP_ADJACENCY_EXT;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            primType = GL_TRIANGLE_FAN;
            break;
        }

        if (op.useIndexes)
        {
            void* pBufferData = 0;
            mStateCacheManager->bindGLBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                op.indexData->indexBuffer->_getImpl<GLHardwareBuffer>()->getGLBufferId());

            pBufferData = VBO_BUFFER_OFFSET(
                op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            do
            {
                if(numberOfInstances > 1)
                {
                    glDrawElementsInstancedARB(primType, op.indexData->indexCount, indexType, pBufferData, numberOfInstances);
                }
                else
                {
                    glDrawElements(primType, op.indexData->indexCount, indexType, pBufferData);
                }
            } while (updatePassIterationRenderState());

        }
        else
        {
            do
            {
                if(numberOfInstances > 1)
                {
                    glDrawArraysInstancedARB(primType, 0, op.vertexData->vertexCount, numberOfInstances);
                }
                else
                {
                    glDrawArrays(primType, 0, op.vertexData->vertexCount);
                }
            } while (updatePassIterationRenderState());
        }

        glDisableClientState( GL_VERTEX_ARRAY );
        // only valid up to GL_MAX_TEXTURE_UNITS, which is recorded in mFixedFunctionTextureUnits
        if (multitexturing)
        {
            unsigned short mNumEnabledTextures = std::max(std::min((unsigned short)mDisabledTexUnitsFrom, mFixedFunctionTextureUnits), (unsigned short)(mMaxBuiltInTextureAttribIndex + 1));		
            for (unsigned short i = 0; i < mNumEnabledTextures; i++)
            {
                // No need to disable for texture units that weren't used
                glClientActiveTextureARB(GL_TEXTURE0 + i);
                glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            }
            glClientActiveTextureARB(GL_TEXTURE0);
        }
        else
        {
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        }
        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY );
        if (GLAD_GL_EXT_secondary_color)
        {
            glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
        }
        // unbind any custom attributes
        for (unsigned int & ai : mRenderAttribsBound)
        {
            glDisableVertexAttribArrayARB(ai);
        }

        // unbind any instance attributes
        for (unsigned int & ai : mRenderInstanceAttribsBound)
        {
            glVertexAttribDivisorARB(ai, 0);
        }

        mRenderAttribsBound.clear();
        mRenderInstanceAttribsBound.clear();

    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setNormaliseNormals(bool normalise)
    {
        mStateCacheManager->setEnabled(GL_NORMALIZE, normalise);
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Null program bound.",
                        "GLRenderSystem::bindGpuProgram");
        }

        GLGpuProgramBase* glprg = dynamic_cast<GLGpuProgramBase*>(prg);

        // Unbind previous gpu program first.
        //
        // Note:
        //  1. Even if both previous and current are the same object, we can't
        //     bypass re-bind completely since the object itself maybe modified.
        //     But we can bypass unbind based on the assumption that object
        //     internally GL program type shouldn't be changed after it has
        //     been created. The behavior of bind to a GL program type twice
        //     should be same as unbind and rebind that GL program type, even
        //     for difference objects.
        //  2. We also assumed that the program's type (vertex or fragment) should
        //     not be changed during it's in using. If not, the following switch
        //     statement will confuse GL state completely, and we can't fix it
        //     here. To fix this case, we must coding the program implementation
        //     itself, if type is changing (during load/unload, etc), and it's inuse,
        //     unbind and notify render system to correct for its state.
        //
        switch (prg->getType())
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
        case GPT_COMPUTE_PROGRAM:
        case GPT_DOMAIN_PROGRAM:
        case GPT_HULL_PROGRAM:
            break;
        }

        // Bind the program
        glprg->bindProgram();

        RenderSystem::bindGpuProgram(prg);
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        mActiveParameters[gptype].reset();
        if (gptype == GPT_VERTEX_PROGRAM && mCurrentVertexProgram)
        {
            mCurrentVertexProgram->unbindProgram();
            mCurrentVertexProgram = 0;
        }
        else if (gptype == GPT_GEOMETRY_PROGRAM && mCurrentGeometryProgram)
        {
            mCurrentGeometryProgram->unbindProgram();
            mCurrentGeometryProgram = 0;
        }
        else if (gptype == GPT_FRAGMENT_PROGRAM && mCurrentFragmentProgram)
        {
            mCurrentFragmentProgram->unbindProgram();
            mCurrentFragmentProgram = 0;
        }
        RenderSystem::unbindGpuProgram(gptype);

    }
    //---------------------------------------------------------------------
    void GLRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask)
    {
        if (mask & (uint16)GPV_GLOBAL)
        {
            // We could maybe use GL_EXT_bindable_uniform here to produce Dx10-style
            // shared constant buffers, but GPU support seems fairly weak?
            // for now, just copy
            params->_copySharedParams();
        }
        mActiveParameters[gptype] = params;
        switch (gptype)
        {
        case GPT_VERTEX_PROGRAM:
            mCurrentVertexProgram->bindProgramParameters(params, mask);
            break;
        case GPT_GEOMETRY_PROGRAM:
            mCurrentGeometryProgram->bindProgramParameters(params, mask);
            break;
        case GPT_FRAGMENT_PROGRAM:
            mCurrentFragmentProgram->bindProgramParameters(params, mask);
            break;
        case GPT_COMPUTE_PROGRAM:
        case GPT_DOMAIN_PROGRAM:
        case GPT_HULL_PROGRAM:
            break;
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
        // A note on GL user clipping:
        // When an ARB vertex program is enabled in GL, user clipping is completely
        // disabled. There is no way around this, it's just turned off.
        // When using GLSL, user clipping can work but you have to include a
        // glClipVertex command in your vertex shader.
        // Thus the planes set here may not actually be respected.


        size_t i = 0;
        size_t numClipPlanes;
        GLdouble clipPlane[4];

        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        // just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);

        numClipPlanes = clipPlanes.size();
        for (i = 0; i < numClipPlanes; ++i)
        {
            GLenum clipPlaneId = static_cast<GLenum>(GL_CLIP_PLANE0 + i);
            const Plane& plane = clipPlanes[i];

            if (i >= 6/*GL_MAX_CLIP_PLANES*/)
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set clip plane",
                            "GLRenderSystem::setClipPlanes");
            }

            clipPlane[0] = plane.normal.x;
            clipPlane[1] = plane.normal.y;
            clipPlane[2] = plane.normal.z;
            clipPlane[3] = plane.d;

            glClipPlane(clipPlaneId, clipPlane);
            mStateCacheManager->setEnabled(clipPlaneId, true);
        }

        // disable remaining clip planes
        for ( ; i < 6/*GL_MAX_CLIP_PLANES*/; ++i)
        {
            mStateCacheManager->setEnabled(static_cast<GLenum>(GL_CLIP_PLANE0 + i), false);
        }

        // restore matrices
        glPopMatrix();
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {
        mStateCacheManager->setEnabled(GL_SCISSOR_TEST, enabled);

        if (!enabled)
            return;

        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();
        //  GL measures from the bottom, not the top
        long targetHeight = mActiveRenderTarget->getHeight();
        long top = flipping ? rect.top : targetHeight - rect.bottom;
        // NB GL uses width / height rather than right / bottom
        glScissor(rect.left, top, rect.width(), rect.height());
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::clearFrameBuffer(unsigned int buffers,
                                          const ColourValue& colour, float depth, unsigned short stencil)
    {
        bool colourMask =
            !(mCurrentBlend.writeR && mCurrentBlend.writeG && mCurrentBlend.writeB && mCurrentBlend.writeA);

        if(mCurrentContext)
			mCurrentContext->setCurrent();

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
            mStateCacheManager->setClearDepth(depth);
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
            // Enable buffer for writing if it isn't
            mStateCacheManager->setStencilMask(0xFFFFFFFF);

            glClearStencil(stencil);
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

        // Clear buffers
        glClear(flags);

        // Restore scissor test
        if (needScissorBox)
        {
            setScissorTest(false);
        }

        // Reset buffer write state
        if (!mDepthWrite && (buffers & FBT_DEPTH))
        {
            mStateCacheManager->setDepthMask( GL_FALSE );
        }
        if (colourMask && (buffers & FBT_COLOUR))
        {
            mStateCacheManager->setColourMask(mCurrentBlend.writeR, mCurrentBlend.writeG,
                                              mCurrentBlend.writeB, mCurrentBlend.writeA);
        }
        if (buffers & FBT_STENCIL)
        {
            mStateCacheManager->setStencilMask(mStencilWriteMask);
        }
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* GLRenderSystem::createHardwareOcclusionQuery(void)
    {
        GLHardwareOcclusionQuery* ret = new GLHardwareOcclusionQuery();
        mHwOcclusionQueries.push_back(ret);
        return ret;
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_oneTimeContextInitialization()
    {
        // Set nicer lighting model -- d3d9 has this by default
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
        mStateCacheManager->setEnabled(GL_COLOR_SUM, true);
        mStateCacheManager->setEnabled(GL_DITHER, false);

        // Check for FSAA
        // Enable the extension if it was enabled by the GLSupport
        if (checkExtension("GL_ARB_multisample"))
        {
            int fsaa_active = false;
            glGetIntegerv(GL_SAMPLE_BUFFERS_ARB,(GLint*)&fsaa_active);
            if(fsaa_active)
            {
                mStateCacheManager->setEnabled(GL_MULTISAMPLE_ARB, true);
                LogManager::getSingleton().logMessage("Using FSAA from GL_ARB_multisample extension.");
            }            
        }

		if (checkExtension("GL_ARB_seamless_cube_map"))
		{
            // Enable seamless cube maps
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::_switchContext(GLContext *context)
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

        // Disable lights
        for (unsigned short i = 0; i < mCurrentLights; ++i)
        {
            setGLLight(i, false);
        }
        mCurrentLights = 0;

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

        mStateCacheManager = mCurrentContext->createOrRetrieveStateCacheManager<GLStateCacheManager>();

        // Check if the context has already done one-time initialisation
        if(!mCurrentContext->getInitialized())
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
        mStateCacheManager->setDepthMask(mDepthWrite);
        mStateCacheManager->setColourMask(mCurrentBlend.writeR, mCurrentBlend.writeG,
                                          mCurrentBlend.writeB, mCurrentBlend.writeA);
        mStateCacheManager->setStencilMask(mStencilWriteMask);

    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        // Unbind frame buffer object
        if(mActiveRenderTarget)
            mRTTManager->unbind(mActiveRenderTarget);

        mActiveRenderTarget = target;
        if (target)
        {
            // Switch context if different from current one
            GLContext *newContext = dynamic_cast<GLRenderTarget*>(target)->getContext();
            if(newContext && mCurrentContext != newContext)
            {
                _switchContext(newContext);
            }

            //Check the FBO's depth buffer status
            auto depthBuffer = static_cast<GLDepthBufferCommon*>(target->getDepthBuffer());

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
                (!depthBuffer || depthBuffer->getGLContext() != mCurrentContext ) )
            {
                //Depth is automatically managed and there is no depth buffer attached to this RT
                //or the Current context doesn't match the one this Depth buffer was created with
                setDepthBufferFor( target );
            }

            // Bind frame buffer object
            mRTTManager->bind(target);

            if (GLAD_GL_EXT_framebuffer_sRGB)
            {
                // Enable / disable sRGB states
                mStateCacheManager->setEnabled(GL_FRAMEBUFFER_SRGB_EXT, target->isHardwareGammaEnabled());
                // Note: could test GL_FRAMEBUFFER_SRGB_CAPABLE_EXT here before
                // enabling, but GL spec says incapable surfaces ignore the setting
                // anyway. We test the capability to enable isHardwareGammaEnabled.
            }
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_unregisterContext(GLContext *context)
    {
        if(mCurrentContext == context) {
            // Change the context to something else so that a valid context
            // remains active. When this is the main context being unregistered,
            // we set the main context to 0.
            if(mCurrentContext != mMainContext) {
                _switchContext(mMainContext);
            } else {
                /// No contexts remain
                mCurrentContext->endCurrent();
                mCurrentContext = 0;
                mMainContext = 0;
                mStateCacheManager = 0;
            }
        }
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::beginProfileEvent( const String &eventName )
    {
        markProfileEvent("Begin Event: " + eventName);
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::endProfileEvent( void )
    {
        markProfileEvent("End Event");
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::markProfileEvent( const String &eventName )
    {
        if( eventName.empty() )
            return;

        if(GLAD_GL_GREMEDY_string_marker)
            glStringMarkerGREMEDY(eventName.length(), eventName.c_str());
    }

    //---------------------------------------------------------------------
    void GLRenderSystem::bindVertexElementToGpu(const VertexElement& elem,
                                                const HardwareVertexBufferSharedPtr& vertexBuffer,
                                                const size_t vertexStart)
    {
        void* pBufferData = 0;
        const GLHardwareBuffer* hwGlBuffer = vertexBuffer->_getImpl<GLHardwareBuffer>();

        mStateCacheManager->bindGLBuffer(GL_ARRAY_BUFFER_ARB, 
                        hwGlBuffer->getGLBufferId());
        pBufferData = VBO_BUFFER_OFFSET(elem.getOffset());

        if (vertexStart)
        {
            pBufferData = static_cast<char*>(pBufferData) + vertexStart * vertexBuffer->getVertexSize();
        }

        VertexElementSemantic sem = elem.getSemantic();
        bool multitexturing = (getCapabilities()->getNumTextureUnits() > 1);

        bool isCustomAttrib = false;
        if (mCurrentVertexProgram)
        {
            isCustomAttrib = !mEnableFixedPipeline || mCurrentVertexProgram->isAttributeValid(sem, elem.getIndex());

            if (vertexBuffer->isInstanceData())
            {
                GLint attrib = GLSLProgramCommon::getFixedAttributeIndex(sem, elem.getIndex());
                glVertexAttribDivisorARB(attrib, vertexBuffer->getInstanceDataStepRate() );
                mRenderInstanceAttribsBound.push_back(attrib);
            }
        }


        // Custom attribute support
        // tangents, binormals, blendweights etc always via this route
        // builtins may be done this way too
        if (isCustomAttrib)
        {
            GLint attrib = GLSLProgramCommon::getFixedAttributeIndex(sem, elem.getIndex());
            unsigned short typeCount = VertexElement::getTypeCount(elem.getType());
            GLboolean normalised = GL_FALSE;
            switch(elem.getType())
            {
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

            glVertexAttribPointerARB(
                attrib,
                typeCount,
                GLHardwareBufferManager::getGLType(elem.getType()),
                normalised,
                static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                pBufferData);
            glEnableVertexAttribArrayARB(attrib);

            mRenderAttribsBound.push_back(attrib);
        }
        else
        {
            // fixed-function & builtin attribute support
            switch(sem)
            {
            case VES_POSITION:
                glVertexPointer(VertexElement::getTypeCount(
                    elem.getType()),
                                GLHardwareBufferManager::getGLType(elem.getType()),
                                static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                pBufferData);
                glEnableClientState( GL_VERTEX_ARRAY );
                break;
            case VES_NORMAL:
                glNormalPointer(
                    GLHardwareBufferManager::getGLType(elem.getType()),
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                    pBufferData);
                glEnableClientState( GL_NORMAL_ARRAY );
                break;
            case VES_DIFFUSE:
                glColorPointer(4,
                               GLHardwareBufferManager::getGLType(elem.getType()),
                               static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                               pBufferData);
                glEnableClientState( GL_COLOR_ARRAY );
                break;
            case VES_SPECULAR:
                if (GLAD_GL_EXT_secondary_color)
                {
                    glSecondaryColorPointerEXT(4,
                                               GLHardwareBufferManager::getGLType(elem.getType()),
                                               static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                               pBufferData);
                    glEnableClientState( GL_SECONDARY_COLOR_ARRAY );
                }
                break;
            case VES_TEXTURE_COORDINATES:

                if (mCurrentVertexProgram)
                {
                    // Programmable pipeline - direct UV assignment
                    glClientActiveTextureARB(GL_TEXTURE0 + elem.getIndex());
                    glTexCoordPointer(
                        VertexElement::getTypeCount(elem.getType()),
                        GLHardwareBufferManager::getGLType(elem.getType()),
                        static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                        pBufferData);
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
                    if (elem.getIndex() > mMaxBuiltInTextureAttribIndex)
                        mMaxBuiltInTextureAttribIndex = elem.getIndex();
                }
                else
                {
                    // fixed function matching to units based on tex_coord_set
                    for (unsigned int i = 0; i < mDisabledTexUnitsFrom; i++)
                    {
                        // Only set this texture unit's texcoord pointer if it
                        // is supposed to be using this element's index
                        if (mTextureCoordIndex[i] == elem.getIndex() && i < mFixedFunctionTextureUnits)
                        {
                            if (multitexturing)
                                glClientActiveTextureARB(GL_TEXTURE0 + i);
                            glTexCoordPointer(
                                VertexElement::getTypeCount(elem.getType()),
                                GLHardwareBufferManager::getGLType(elem.getType()),
                                static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                pBufferData);
                            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
                        }
                    }
                }
                break;
            default:
                break;
            };
        } // isCustomAttrib
    }
	
	//---------------------------------------------------------------------
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
	bool GLRenderSystem::setDrawBuffer(ColourBufferType colourBuffer)
	{
		bool result = true;

		switch (colourBuffer)
		{
		case CBT_BACK:
			glDrawBuffer(GL_BACK);
			break;
		case CBT_BACK_LEFT:
			glDrawBuffer(GL_BACK_LEFT);
			break;
		case CBT_BACK_RIGHT:
			glDrawBuffer(GL_BACK_RIGHT);
			break;
		default:
			result = false;
		}

		// Check for any errors
		GLenum error = glGetError();
		if (result && GL_NO_ERROR != error)
		{		
			const char* errorCode = glErrorToString(error);
			String errorString = "GLRenderSystem::setDrawBuffer(" 
				+ Ogre::StringConverter::toString(colourBuffer) + "): " + errorCode;

			Ogre::LogManager::getSingleton().logMessage(errorString);			
			result = false;
		}

		return result;
	}
#endif

    void GLRenderSystem::_copyContentsToMemory(Viewport* vp, const Box& src, const PixelBox &dst, RenderWindow::FrameBuffer buffer)
    {
        GLenum format = GLPixelUtil::getGLOriginFormat(dst.format);
        GLenum type = GLPixelUtil::getGLOriginDataType(dst.format);

        if ((format == GL_NONE) || (type == 0))
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unsupported format.", "GLRenderSystem::copyContentsToMemory" );
        }

        // Switch context if different from current one
        _setViewport(vp);

        if(dst.getWidth() != dst.rowPitch)
            glPixelStorei(GL_PACK_ROW_LENGTH, dst.rowPitch);
        // Must change the packing to ensure no overruns!
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        glReadBuffer((buffer == RenderWindow::FB_FRONT)? GL_FRONT : GL_BACK);

        uint32_t height = vp->getTarget()->getHeight();

        glReadPixels((GLint)src.left, (GLint)(height - src.bottom),
                     (GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
                     format, type, dst.getTopLeftFrontPixelPtr());

        // restore default alignment
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);

        PixelUtil::bulkPixelVerticalFlip(dst);
    }
	//---------------------------------------------------------------------
    void GLRenderSystem::initialiseExtensions(void)
    {
        // Set version string
        const GLubyte* pcVer = glGetString(GL_VERSION);
        assert(pcVer && "Problems getting GL version string using glGetString");

        String tmpStr = (const char*)pcVer;
        mDriverVersion.fromString(tmpStr.substr(0, tmpStr.find(' ')));
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
        assert(pcExt && "Problems getting GL extension string using glGetString");
        LogManager::getSingleton().logMessage("GL_EXTENSIONS = " + String((const char*)pcExt));

        ext << pcExt;

        while(ext >> str)
        {
            mExtensionList.insert(str);
        }
    }
}
