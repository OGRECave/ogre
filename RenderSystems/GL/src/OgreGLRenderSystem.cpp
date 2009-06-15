/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.s
-----------------------------------------------------------------------------
*/


#include "OgreGLRenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreGLTextureManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreGLHardwareIndexBuffer.h"
#include "OgreGLDefaultHardwareBufferManager.h"
#include "OgreGLUtil.h"
#include "OgreGLGpuProgram.h"
#include "OgreGLGpuNvparseProgram.h"
#include "ATI_FS_GLGpuProgram.h"
#include "OgreGLGpuProgramManager.h"
#include "OgreException.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLHardwareOcclusionQuery.h"
#include "OgreGLContext.h"

#include "OgreGLFBORenderTexture.h"
#include "OgreGLPBRenderTexture.h"
#include "OgreConfig.h"

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

#if OGRE_THREAD_SUPPORT != 1
GLenum glewContextInit (Ogre::GLSupport *glSupport);
#endif

namespace Ogre {

	// Callback function used when registering GLGpuPrograms
	GpuProgram* createGLArbGpuProgram(ResourceManager* creator, 
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

	GpuProgram* createGLGpuNvparseProgram(ResourceManager* creator, 
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

	GpuProgram* createGL_ATI_FS_GpuProgram(ResourceManager* creator, 
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

	GLRenderSystem::GLRenderSystem()
		: mDepthWrite(true), mStencilMask(0xFFFFFFFF), mHardwareBufferManager(0),
		mGpuProgramManager(0),
		mGLSLProgramFactory(0),
		mRTTManager(0),
		mActiveTextureUnit(0)
	{
		size_t i;

		LogManager::getSingleton().logMessage(getName() + " created.");

		// Get our GLSupport
		mGLSupport = getGLSupport();

		for( i=0; i<MAX_LIGHTS; i++ )
			mLights[i] = NULL;

		mWorldMatrix = Matrix4::IDENTITY;
		mViewMatrix = Matrix4::IDENTITY;

		initConfigOptions();

		mColourWrite[0] = mColourWrite[1] = mColourWrite[2] = mColourWrite[3] = true;

		for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
		{
			// Dummy value
			mTextureCoordIndex[i] = 99;
		}

		for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
		{
			mTextureTypes[i] = 0;
		}

		mActiveRenderTarget = 0;
		mCurrentContext = 0;
		mMainContext = 0;

		mGLInitialized = false;

		mCurrentLights = 0;
		mMinFilter = FO_LINEAR;
		mMipFilter = FO_POINT;
		mCurrentVertexProgram = 0;
		mCurrentGeometryProgram = 0;
		mCurrentFragmentProgram = 0;

	}

	GLRenderSystem::~GLRenderSystem()
	{
		shutdown();

		// Destroy render windows
		RenderTargetMap::iterator i;
		for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
		{
			delete i->second;
		}
		mRenderTargets.clear();

		delete mGLSupport;
	}

	const String& GLRenderSystem::getName(void) const
	{
		static String strName("OpenGL Rendering Subsystem");
		return strName;
	}

	void GLRenderSystem::initConfigOptions(void)
	{
		mGLSupport->addConfig();
	}

	ConfigOptionMap& GLRenderSystem::getConfigOptions(void)
	{
		return mGLSupport->getConfigOptions();
	}

	void GLRenderSystem::setConfigOption(const String &name, const String &value)
	{
		mGLSupport->setConfigOption(name, value);
	}

	String GLRenderSystem::validateConfigOptions(void)
	{
		// XXX Return an error string if something is invalid
		return mGLSupport->validateConfig();
	}

	RenderWindow* GLRenderSystem::_initialise(bool autoCreateWindow, const String& windowTitle)
	{
		mGLSupport->start();

		RenderWindow* autoWindow = mGLSupport->createWindow(autoCreateWindow, this, windowTitle);

		RenderSystem::_initialise(autoCreateWindow, windowTitle);

		return autoWindow;
	}

	RenderSystemCapabilities* GLRenderSystem::createRenderSystemCapabilities() const
	{
		RenderSystemCapabilities* rsc = new RenderSystemCapabilities();

		rsc->setCategoryRelevant(CAPS_CATEGORY_GL, true);
		rsc->setDriverVersion(mDriverVersion);
		const char* deviceName = (const char*)glGetString(GL_RENDERER);
		const char* vendorName = (const char*)glGetString(GL_VENDOR);
		rsc->setDeviceName(deviceName);
		rsc->setRenderSystemName(getName());

		// determine vendor
		if (strstr(vendorName, "NVIDIA"))
			rsc->setVendor(GPU_NVIDIA);
		else if (strstr(vendorName, "ATI"))
			rsc->setVendor(GPU_ATI);
		else if (strstr(vendorName, "Intel"))
			rsc->setVendor(GPU_INTEL);
		else if (strstr(vendorName, "S3"))
			rsc->setVendor(GPU_S3);
		else if (strstr(vendorName, "Matrox"))
			rsc->setVendor(GPU_MATROX);
		else if (strstr(vendorName, "3DLabs"))
			rsc->setVendor(GPU_3DLABS);
		else if (strstr(vendorName, "SiS"))
			rsc->setVendor(GPU_SIS);
		else
			rsc->setVendor(GPU_UNKNOWN);

		// Supports fixed-function
		rsc->setCapability(RSC_FIXED_FUNCTION);

        // Check for hardware mipmapping support.
        if(GLEW_VERSION_1_4 || GLEW_SGIS_generate_mipmap)
        {
			bool disableAutoMip = false;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_LINUX
			// Apple & Linux ATI drivers have faults in hardware mipmap generation
			if (rsc->getVendor() == GPU_ATI)
				disableAutoMip = true;
#endif
			// The Intel 915G frequently corrupts textures when using hardware mip generation
			// I'm not currently sure how many generations of hardware this affects, 
			// so for now, be safe.
			if (rsc->getVendor() == GPU_INTEL)
				disableAutoMip = true;

			// SiS chipsets also seem to have problems with this
			if (rsc->getVendor() == GPU_SIS)
				disableAutoMip = true;

			if (!disableAutoMip)
				rsc->setCapability(RSC_AUTOMIPMAP);
        }

		// Check for blending support
		if(GLEW_VERSION_1_3 || 
			GLEW_ARB_texture_env_combine || 
			GLEW_EXT_texture_env_combine)
		{
			rsc->setCapability(RSC_BLENDING);
		}

		// Check for Multitexturing support and set number of texture units
		if(GLEW_VERSION_1_3 || 
			GLEW_ARB_multitexture)
		{
			GLint units;
			glGetIntegerv( GL_MAX_TEXTURE_UNITS, &units );

			if (GLEW_ARB_fragment_program)
			{
				// Also check GL_MAX_TEXTURE_IMAGE_UNITS_ARB since NV at least
				// only increased this on the FX/6x00 series
				GLint arbUnits;
				glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &arbUnits );
				if (arbUnits > units)
					units = arbUnits;
			}
			rsc->setNumTextureUnits(units);
		}
		else
		{
			// If no multitexture support then set one texture unit
			rsc->setNumTextureUnits(1);
		}

		// Check for Anisotropy support
		if(GLEW_EXT_texture_filter_anisotropic)
		{
			rsc->setCapability(RSC_ANISOTROPY);
		}

		// Check for DOT3 support
		if(GLEW_VERSION_1_3 ||
			GLEW_ARB_texture_env_dot3 ||
			GLEW_EXT_texture_env_dot3)
		{
			rsc->setCapability(RSC_DOT3);
		}

		// Check for cube mapping
		if(GLEW_VERSION_1_3 || 
			GLEW_ARB_texture_cube_map ||
			GLEW_EXT_texture_cube_map)
		{
			rsc->setCapability(RSC_CUBEMAPPING);
		}


		// Point sprites
		if (GLEW_VERSION_2_0 ||	GLEW_ARB_point_sprite)
		{
			rsc->setCapability(RSC_POINT_SPRITES);
		}
		// Check for point parameters
		if (GLEW_VERSION_1_4)
		{
			rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
		}
		if (GLEW_ARB_point_parameters)
		{
			rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS_ARB);
		}
		if (GLEW_EXT_point_parameters)
		{
			rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS_EXT);
		}

		// Check for hardware stencil support and set bit depth
		GLint stencil;
		glGetIntegerv(GL_STENCIL_BITS,&stencil);

		if(stencil)
		{
			rsc->setCapability(RSC_HWSTENCIL);
			rsc->setStencilBufferBitDepth(stencil);
		}


		if(GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object)
		{
			if (!GLEW_ARB_vertex_buffer_object)
			{
				rsc->setCapability(RSC_GL1_5_NOVBO);
			}
			rsc->setCapability(RSC_VBO);
		}

		if(GLEW_ARB_vertex_program)
		{
			rsc->setCapability(RSC_VERTEX_PROGRAM);

			// Vertex Program Properties
			rsc->setVertexProgramConstantBoolCount(0);
			rsc->setVertexProgramConstantIntCount(0);

			GLint floatConstantCount;
			glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &floatConstantCount);
			rsc->setVertexProgramConstantFloatCount(floatConstantCount);

			rsc->addShaderProfile("arbvp1");
			if (GLEW_NV_vertex_program2_option)
			{
				rsc->addShaderProfile("vp30");
			}

			if (GLEW_NV_vertex_program3)
			{
				rsc->addShaderProfile("vp40");
			}

			if (GLEW_NV_vertex_program4)
			{
				rsc->addShaderProfile("gp4vp");
				rsc->addShaderProfile("gpu_vp");
			}
		}

		if (GLEW_NV_register_combiners2 &&
			GLEW_NV_texture_shader)
		{
			rsc->setCapability(RSC_FRAGMENT_PROGRAM);
			rsc->addShaderProfile("fp20");
		}

		// NFZ - check for ATI fragment shader support
		if (GLEW_ATI_fragment_shader)
		{
			rsc->setCapability(RSC_FRAGMENT_PROGRAM);
			// no boolean params allowed
			rsc->setFragmentProgramConstantBoolCount(0);
			// no integer params allowed
			rsc->setFragmentProgramConstantIntCount(0);

			// only 8 Vector4 constant floats supported
			rsc->setFragmentProgramConstantFloatCount(8);

			rsc->addShaderProfile("ps_1_4");
			rsc->addShaderProfile("ps_1_3");
			rsc->addShaderProfile("ps_1_2");
			rsc->addShaderProfile("ps_1_1");
		}

		if (GLEW_ARB_fragment_program)
		{
			rsc->setCapability(RSC_FRAGMENT_PROGRAM);

			// Fragment Program Properties
			rsc->setFragmentProgramConstantBoolCount(0);
			rsc->setFragmentProgramConstantIntCount(0);

			GLint floatConstantCount;
			glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &floatConstantCount);
			rsc->setFragmentProgramConstantFloatCount(floatConstantCount);

			rsc->addShaderProfile("arbfp1");
			if (GLEW_NV_fragment_program_option)
			{
				rsc->addShaderProfile("fp30");
			}

			if (GLEW_NV_fragment_program2)
			{
				rsc->addShaderProfile("fp40");
			}        
		}

		// NFZ - Check if GLSL is supported
		if ( GLEW_VERSION_2_0 || 
			(GLEW_ARB_shading_language_100 &&
			GLEW_ARB_shader_objects &&
			GLEW_ARB_fragment_shader &&
			GLEW_ARB_vertex_shader) )
		{
			rsc->addShaderProfile("glsl");
		}

		// Check if geometry shaders are supported
		if (GLEW_VERSION_2_0 &&
			GLEW_EXT_geometry_shader4)
		{
			rsc->setCapability(RSC_GEOMETRY_PROGRAM);
			rsc->addShaderProfile("nvgp4");

			//Also add the CG profiles
			rsc->addShaderProfile("gpu_gp");
			rsc->addShaderProfile("gp4gp");

			rsc->setGeometryProgramConstantBoolCount(0);
			rsc->setGeometryProgramConstantIntCount(0);

			GLint floatConstantCount;
			glGetProgramivARB(GL_GEOMETRY_PROGRAM_NV, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &floatConstantCount);
			rsc->setGeometryProgramConstantFloatCount(floatConstantCount);

			GLint maxOutputVertices;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&maxOutputVertices);
			rsc->setGeometryProgramNumOutputVertices(maxOutputVertices);
		}
		
		//Check if render to vertex buffer (transform feedback in OpenGL)
		if (GLEW_VERSION_2_0 && 
			GLEW_NV_transform_feedback)
		{
			rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);
		}

		// Check for texture compression
        if(GLEW_VERSION_1_3 || GLEW_ARB_texture_compression)
        {   
			rsc->setCapability(RSC_TEXTURE_COMPRESSION);
         
            // Check for dxt compression
            if(GLEW_EXT_texture_compression_s3tc)
            {
#if defined(__APPLE__) && defined(__PPC__)
			// Apple on ATI & PPC has errors in DXT
			if (mGLSupport->getGLVendor().find("ATI") == std::string::npos)
#endif
					rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
            }
            // Check for vtc compression
            if(GLEW_NV_texture_compression_vtc)
            {
                rsc->setCapability(RSC_TEXTURE_COMPRESSION_VTC);
            }
        }

		// Scissor test is standard in GL 1.2 (is it emulated on some cards though?)
		rsc->setCapability(RSC_SCISSOR_TEST);
		// As are user clipping planes
		rsc->setCapability(RSC_USER_CLIP_PLANES);

		// 2-sided stencil?
		if (GLEW_VERSION_2_0 || GLEW_EXT_stencil_two_side)
		{
			rsc->setCapability(RSC_TWO_SIDED_STENCIL);
		}
		// stencil wrapping?
		if (GLEW_VERSION_1_4 || GLEW_EXT_stencil_wrap)
		{
			rsc->setCapability(RSC_STENCIL_WRAP);
		}

		// Check for hardware occlusion support
		if(GLEW_VERSION_1_5 || GLEW_ARB_occlusion_query)
		{
			// Some buggy driver claim that it is GL 1.5 compliant and
			// not support ARB_occlusion_query
			if (!GLEW_ARB_occlusion_query)
			{
				rsc->setCapability(RSC_GL1_5_NOHWOCCLUSION);
			}

			rsc->setCapability(RSC_HWOCCLUSION);
		}
		else if (GLEW_NV_occlusion_query)
		{
			// Support NV extension too for old hardware
			rsc->setCapability(RSC_HWOCCLUSION);
		}

		// UBYTE4 always supported
		rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

		// Inifinite far plane always supported
		rsc->setCapability(RSC_INFINITE_FAR_PLANE);

		// Check for non-power-of-2 texture support
		if(GLEW_ARB_texture_non_power_of_two)
		{
			rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		}

		// Check for Float textures
		if(GLEW_ATI_texture_float || GLEW_ARB_texture_float)
		{
			rsc->setCapability(RSC_TEXTURE_FLOAT);
		}

		// 3D textures should be supported by GL 1.2, which is our minimum version
		rsc->setCapability(RSC_TEXTURE_3D);

		// Check for framebuffer object extension
		if(GLEW_EXT_framebuffer_object)
		{
			// Probe number of draw buffers
			// Only makes sense with FBO support, so probe here
			if(GLEW_VERSION_2_0 || 
				GLEW_ARB_draw_buffers ||
				GLEW_ATI_draw_buffers)
			{
				GLint buffers;
				glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &buffers);
				rsc->setNumMultiRenderTargets(std::min<int>(buffers, (GLint)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
				rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);
				if(!GLEW_VERSION_2_0)
				{
					// Before GL version 2.0, we need to get one of the extensions
					if(GLEW_ARB_draw_buffers)
						rsc->setCapability(RSC_FBO_ARB);
					if(GLEW_ATI_draw_buffers)
						rsc->setCapability(RSC_FBO_ATI);
				}
				// Set FBO flag for all 3 'subtypes'
				rsc->setCapability(RSC_FBO);

			}
			rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
		}
		else
		{
			// Check GLSupport for PBuffer support
			if(mGLSupport->supportsPBuffers())
			{
				// Use PBuffers
				rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
				rsc->setCapability(RSC_PBUFFER);
			}
		}

		// Point size
		if (GLEW_VERSION_1_4)
		{
		float ps;
		glGetFloatv(GL_POINT_SIZE_MAX, &ps);
		rsc->setMaxPointSize(ps);
		}
		else
		{
			GLint vSize[2];
			glGetIntegerv(GL_POINT_SIZE_RANGE,vSize);
			rsc->setMaxPointSize(vSize[1]);
		}

		// Vertex texture fetching
		if (mGLSupport->checkExtension("GL_ARB_vertex_shader"))
		{
		GLint vUnits;
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &vUnits);
		rsc->setNumVertexTextureUnits(static_cast<ushort>(vUnits));
		if (vUnits > 0)
		{
			rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
		}
		// GL always shares vertex and fragment texture units (for now?)
		rsc->setVertexTextureUnitsShared(true);
		}

		// Mipmap LOD biasing?
		if (GLEW_VERSION_1_4 || GLEW_EXT_texture_lod_bias)
		{
			rsc->setCapability(RSC_MIPMAP_LOD_BIAS);
		}

		// Alpha to coverage?
		if (mGLSupport->checkExtension("GL_ARB_multisample"))
		{
			// Alpha to coverage always 'supported' when MSAA is available
			// although card may ignore it if it doesn't specifically support A2C
			rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
		}

		// Advanced blending operations
		if(GLEW_VERSION_2_0)
		{
			rsc->setCapability(RSC_ADVANCED_BLEND_OPERATIONS);
		}

		return rsc;
	}

	void GLRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
	{
		if(caps->getRenderSystemName() != getName())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Trying to initialize GLRenderSystem from RenderSystemCapabilities that do not support OpenGL",
				"GLRenderSystem::initialiseFromRenderSystemCapabilities");
		}

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
		
		if(caps->hasCapability(RSC_GL1_5_NOVBO))
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

		if(caps->hasCapability(RSC_VBO))
		{

			mHardwareBufferManager = new GLHardwareBufferManager;
		}
		else
		{
			mHardwareBufferManager = new GLDefaultHardwareBufferManager;
		}

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

		}

		if(caps->isShaderProfileSupported("glsl"))
		{
			// NFZ - check for GLSL vertex and fragment shader support successful
			mGLSLProgramFactory = new GLSLProgramFactory();
			HighLevelGpuProgramManager::getSingleton().addFactory(mGLSLProgramFactory);
			LogManager::getSingleton().logMessage("GLSL support detected");
		}

		if(caps->hasCapability(RSC_HWOCCLUSION))
		{
			if(caps->hasCapability(RSC_GL1_5_NOHWOCCLUSION))
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
		}


		/// Do this after extension function pointers are initialised as the extension
		/// is used to probe further capabilities.
		ConfigOptionMap::iterator cfi = getConfigOptions().find("RTT Preferred Mode");
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
		if(caps->hasCapability(RSC_FBO) && rttMode < 1)
		{
			// Before GL version 2.0, we need to get one of the extensions
			if(caps->hasCapability(RSC_FBO_ARB))
				GLEW_GET_FUN(__glewDrawBuffers) = glDrawBuffersARB;
			else if(caps->hasCapability(RSC_FBO_ATI))
				GLEW_GET_FUN(__glewDrawBuffers) = glDrawBuffersATI;

			if(caps->hasCapability(RSC_HWRENDER_TO_TEXTURE))
			{
				// Create FBO manager
				LogManager::getSingleton().logMessage("GL: Using GL_EXT_framebuffer_object for rendering to textures (best)");
				mRTTManager = new GLFBOManager(false);
			}

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
					LogManager::getSingleton().logMessage("GL: Using PBuffers for rendering to textures");
				}
			}
			else
			{
				// No pbuffer support either -- fallback to simplest copying from framebuffer
				mRTTManager = new GLCopyingRTTManager();
				LogManager::getSingleton().logMessage("GL: Using framebuffer copy for rendering to textures (worst)");
				LogManager::getSingleton().logMessage("GL: Warning: RenderTexture size is restricted to size of framebuffer. If you are on Linux, consider using GLX instead of SDL.");
			}

			// Downgrade number of simultaneous targets
			caps->setNumMultiRenderTargets(1);
		}


		Log* defaultLog = LogManager::getSingleton().getDefaultLog();
		if (defaultLog)
		{
			caps->log(defaultLog);
		}

		/// Create the texture manager        
		mTextureManager = new GLTextureManager(*mGLSupport); 

		mGLInitialized = true;
	}

	void GLRenderSystem::reinitialise(void)
	{
		this->shutdown();
		this->_initialise(true);
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

		// Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
		delete mGpuProgramManager;
		mGpuProgramManager = 0;

		delete mHardwareBufferManager;
		mHardwareBufferManager = 0;

		delete mRTTManager;
		mRTTManager = 0;

		// Delete extra threads contexts
		for (GLContextList::iterator i = mBackgroundContextList.begin(); 
			i != mBackgroundContextList.end(); ++i)
		{
			GLContext* pCurContext = *i;

			pCurContext->releaseContext();

			delete pCurContext;
		}
		mBackgroundContextList.clear();

		mGLSupport->stop();
		mStopRendering = true;

		delete mTextureManager;
		mTextureManager = 0;

		// There will be a new initial window and so forth, thus any call to test
		//  some params will access an invalid pointer, so it is best to reset
		//  the whole state.
		mGLInitialized = 0;
	}

	void GLRenderSystem::setAmbientLight(float r, float g, float b)
	{
		GLfloat lmodel_ambient[] = {r, g, b, 1.0};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	}

	void GLRenderSystem::setShadingType(ShadeOptions so)
	{
		switch(so)
		{
		case SO_FLAT:
			glShadeModel(GL_FLAT);
			break;
		default:
			glShadeModel(GL_SMOOTH);
			break;
		}
	}

	//---------------------------------------------------------------------
	bool GLRenderSystem::_createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
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
	//---------------------------------------------------------------------
	RenderWindow* GLRenderSystem::_createRenderWindow(const String &name, 
		unsigned int width, unsigned int height, bool fullScreen,
		const NameValuePairList *miscParams)
	{
		if (mRenderTargets.find(name) != mRenderTargets.end())
		{
			OGRE_EXCEPT(
				Exception::ERR_INVALIDPARAMS, 
				"Window with name '" + name + "' already exists",
				"GLRenderSystem::_createRenderWindow" );
		}
		// Log a message
		StringStream ss;
		ss << "GLRenderSystem::_createRenderWindow \"" << name << "\", " <<
			width << "x" << height << " ";
		if(fullScreen)
			ss << "fullscreen ";
		else
			ss << "windowed ";
		if(miscParams)
		{
			ss << " miscParams: ";
			NameValuePairList::const_iterator it;
			for(it=miscParams->begin(); it!=miscParams->end(); ++it)
			{
				ss << it->first << "=" << it->second << " ";
			}
			LogManager::getSingleton().logMessage(ss.str());
		}

		// Create the window
		RenderWindow* win = mGLSupport->newWindow(name, width, height, 
			fullScreen, miscParams);

		attachRenderTarget( *win );

		if (!mGLInitialized) 
		{                

			// set up glew and GLSupport
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
			if(!mUseCustomCapabilities)
				mCurrentCapabilities = mRealCapabilities;

			initialiseFromRenderSystemCapabilities(mCurrentCapabilities, win);

			// Initialise the main context
			_oneTimeContextInitialization();
			if(mCurrentContext)
				mCurrentContext->setInitialized();
		}

		return win;
	}

	void GLRenderSystem::initialiseContext(RenderWindow* primary)
	{
		// Set main and current context
		mMainContext = 0;
		primary->getCustomAttribute("GLCONTEXT", &mMainContext);
		mCurrentContext = mMainContext;

		// Set primary context as active
		if(mCurrentContext)
			mCurrentContext->setCurrent();

		// Setup GLSupport
		mGLSupport->initialiseExtensions();

		LogManager::getSingleton().logMessage("***************************");
		LogManager::getSingleton().logMessage("*** GL Renderer Started ***");
		LogManager::getSingleton().logMessage("***************************");

		// Get extension function pointers
#if OGRE_THREAD_SUPPORT != 1
		glewContextInit(mGLSupport);
#endif
	}



	//-----------------------------------------------------------------------
	MultiRenderTarget * GLRenderSystem::createMultiRenderTarget(const String & name)
	{
		MultiRenderTarget *retval = mRTTManager->createMultiRenderTarget(name);
		attachRenderTarget( *retval );
		return retval;
	}

	//-----------------------------------------------------------------------
	void GLRenderSystem::destroyRenderWindow(RenderWindow* pWin)
	{
		// Find it to remove from list
		RenderTargetMap::iterator i = mRenderTargets.begin();

		while (i != mRenderTargets.end())
		{
			if (i->second == pWin)
			{
				mRenderTargets.erase(i);
				delete pWin;
				break;
			}
		}
	}

	//---------------------------------------------------------------------
	void GLRenderSystem::_useLights(const LightList& lights, unsigned short limit)
	{
		// Save previous modelview
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		// just load view matrix (identity world)
		GLfloat mat[16];
		makeGLMatrix(mat, mViewMatrix);
		glLoadMatrixf(mat);

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

	}

	void GLRenderSystem::setGLLight(size_t index, Light* lt)
	{
		GLenum gl_index = GL_LIGHT0 + index;

		if (!lt)
		{
			// Disable in the scene
			glDisable(gl_index);
		}
		else
		{
			switch (lt->getType())
			{
			case Light::LT_SPOTLIGHT:
				glLightf( gl_index, GL_SPOT_CUTOFF, 0.5f * lt->getSpotlightOuterAngle().valueDegrees() );
				glLightf(gl_index, GL_SPOT_EXPONENT, lt->getSpotlightFalloff());
				break;
			default:
				glLightf( gl_index, GL_SPOT_CUTOFF, 180.0 );
				break;
			}

			// Color
			ColourValue col;
			col = lt->getDiffuseColour();


			GLfloat f4vals[4] = {col.r, col.g, col.b, col.a};
			glLightfv(gl_index, GL_DIFFUSE, f4vals);

			col = lt->getSpecularColour();
			f4vals[0] = col.r;
			f4vals[1] = col.g;
			f4vals[2] = col.b;
			f4vals[3] = col.a;
			glLightfv(gl_index, GL_SPECULAR, f4vals);


			// Disable ambient light for movables;
			f4vals[0] = 0;
			f4vals[1] = 0;
			f4vals[2] = 0;
			f4vals[3] = 1;
			glLightfv(gl_index, GL_AMBIENT, f4vals);

			setGLLightPositionDirection(lt, gl_index);


			// Attenuation
			glLightf(gl_index, GL_CONSTANT_ATTENUATION, lt->getAttenuationConstant());
			glLightf(gl_index, GL_LINEAR_ATTENUATION, lt->getAttenuationLinear());
			glLightf(gl_index, GL_QUADRATIC_ATTENUATION, lt->getAttenuationQuadric());
			// Enable in the scene
			glEnable(gl_index);

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
	void GLRenderSystem::_setWorldMatrix( const Matrix4 &m )
	{
		GLfloat mat[16];
		mWorldMatrix = m;
		makeGLMatrix( mat, mViewMatrix * mWorldMatrix );
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(mat);
	}

	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setViewMatrix( const Matrix4 &m )
	{
		mViewMatrix = m;

		GLfloat mat[16];
		makeGLMatrix( mat, mViewMatrix * mWorldMatrix );
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(mat);

		// also mark clip planes dirty
		if (!mClipPlanes.empty())
			mClipPlanesDirty = true;
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setProjectionMatrix(const Matrix4 &m)
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
		glLoadMatrixf(mat);
		glMatrixMode(GL_MODELVIEW);

		// also mark clip planes dirty
		if (!mClipPlanes.empty())
			mClipPlanesDirty = true;
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setSurfaceParams(const ColourValue &ambient,
		const ColourValue &diffuse, const ColourValue &specular,
		const ColourValue &emissive, Real shininess,
		TrackVertexColourType tracking)
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

			glEnable(GL_COLOR_MATERIAL);
		} 
		else 
		{
			glDisable(GL_COLOR_MATERIAL);          
		}

		// XXX Cache previous values?
		// XXX Front or Front and Back?

		GLfloat f4val[4] = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f4val);
		f4val[0] = ambient.r;
		f4val[1] = ambient.g;
		f4val[2] = ambient.b;
		f4val[3] = ambient.a;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f4val);
		f4val[0] = specular.r;
		f4val[1] = specular.g;
		f4val[2] = specular.b;
		f4val[3] = specular.a;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f4val);
		f4val[0] = emissive.r;
		f4val[1] = emissive.g;
		f4val[2] = emissive.b;
		f4val[3] = emissive.a;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f4val);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setPointParameters(Real size, 
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
			minSize = minSize * mActiveViewport->getActualHeight();
			if (maxSize == 0.0f)
				maxSize = mCurrentCapabilities->getMaxPointSize(); // pixels
			else
				maxSize = maxSize * mActiveViewport->getActualHeight();
			
			// XXX: why do I need this for results to be consistent with D3D?
			// Equations are supposedly the same once you factor in vp height
			Real correction = 0.005;
			// scaling required
			val[0] = constant;
			val[1] = linear * correction;
			val[2] = quadratic * correction;
			val[3] = 1;
			
			if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			
			
		} 
		else 
		{
			if (maxSize == 0.0f)
				maxSize = mCurrentCapabilities->getMaxPointSize();
			if (mCurrentCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
				glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		}
		
		// no scaling required
		// GL has no disabled flag for this so just set to constant
		glPointSize(size);
		
		if (mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
		{
			glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
			glPointParameterf(GL_POINT_SIZE_MIN, minSize);
			glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
		} 
		else if (mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_ARB))
		{
			glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION, val);
			glPointParameterfARB(GL_POINT_SIZE_MIN, minSize);
			glPointParameterfARB(GL_POINT_SIZE_MAX, maxSize);
		} 
		else if (mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS_EXT))
		{
			glPointParameterfvEXT(GL_POINT_DISTANCE_ATTENUATION, val);
			glPointParameterfEXT(GL_POINT_SIZE_MIN, minSize);
			glPointParameterfEXT(GL_POINT_SIZE_MAX, maxSize);
		}
		
		
		
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_setPointSpritesEnabled(bool enabled)
	{
		if (!getCapabilities()->hasCapability(RSC_POINT_SPRITES))
			return;

		if (enabled)
		{
			glEnable(GL_POINT_SPRITE);
		}
		else
		{
			glDisable(GL_POINT_SPRITE);
		}

		// Set sprite texture coord generation
		// Don't offer this as an option since D3D links it to sprite enabled
		for (ushort i = 0; i < mFixedFunctionTextureUnits; ++i)
		{
			activateGLTextureUnit(i);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, 
				enabled ? GL_TRUE : GL_FALSE);
		}
		activateGLTextureUnit(0);

	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
	{
		GLTexturePtr tex = texPtr;

		GLenum lastTextureType = mTextureTypes[stage];

		if (!activateGLTextureUnit(stage))
			return;

		if (enabled)
		{
			if (!tex.isNull())
			{
				// note used
				tex->touch();
				mTextureTypes[stage] = tex->getGLTextureTarget();
			}
			else
				// assume 2D
				mTextureTypes[stage] = GL_TEXTURE_2D;

			if(lastTextureType != mTextureTypes[stage] && lastTextureType != 0)
			{
				if (stage < mFixedFunctionTextureUnits)
				{
					glDisable( lastTextureType );
				}
			}

			if (stage < mFixedFunctionTextureUnits)
			{
				glEnable( mTextureTypes[stage] );
			}

			if(!tex.isNull())
				glBindTexture( mTextureTypes[stage], tex->getGLID() );
			else
				glBindTexture( mTextureTypes[stage], static_cast<GLTextureManager*>(mTextureManager)->getWarningTextureID() );
		}
		else
		{
			if (stage < mFixedFunctionTextureUnits)
			{
				if (lastTextureType != 0)
				{
					glDisable( mTextureTypes[stage] );
				}
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
			// bind zero texture
			glBindTexture(GL_TEXTURE_2D, 0); 
		}

		activateGLTextureUnit(0);
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

		if (!activateGLTextureUnit(stage))
			return;

		switch( m )
		{
		case TEXCALC_NONE:
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
			break;

		case TEXCALC_ENVIRONMENT_MAP:
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );

			// Need to use a texture matrix to flip the spheremap
			mUseAutoTextureMatrix = true;
			memset(mAutoTextureMatrix, 0, sizeof(GLfloat)*16);
			mAutoTextureMatrix[0] = mAutoTextureMatrix[10] = mAutoTextureMatrix[15] = 1.0f;
			mAutoTextureMatrix[5] = -1.0f;

			break;

		case TEXCALC_ENVIRONMENT_MAP_PLANAR:            
			// XXX This doesn't seem right?!
#ifdef GL_VERSION_1_3
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
			glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
#else
			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
#endif
			break;
		case TEXCALC_ENVIRONMENT_MAP_REFLECTION:

			glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
			glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
			glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );

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

			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
			glDisable( GL_TEXTURE_GEN_Q );
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
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glEnable(GL_TEXTURE_GEN_Q);

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
		activateGLTextureUnit(0);
	}
	//-----------------------------------------------------------------------------
	GLint GLRenderSystem::getTextureAddressingMode(
		TextureUnitState::TextureAddressingMode tam) const
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
	void GLRenderSystem::_setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw)
	{
		if (!activateGLTextureUnit(stage))
			return;
		glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_S, 
			getTextureAddressingMode(uvw.u));
		glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_T, 
			getTextureAddressingMode(uvw.v));
		glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_R, 
			getTextureAddressingMode(uvw.w));
		activateGLTextureUnit(0);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setTextureBorderColour(size_t stage, const ColourValue& colour)
	{
		GLfloat border[4] = { colour.r, colour.g, colour.b, colour.a };
		if (activateGLTextureUnit(stage))
		{
		glTexParameterfv( mTextureTypes[stage], GL_TEXTURE_BORDER_COLOR, border);
			activateGLTextureUnit(0);
	}
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setTextureMipmapBias(size_t stage, float bias)
	{
		if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
		{
			if (activateGLTextureUnit(stage))
			{
			glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);
				activateGLTextureUnit(0);
		}
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

		GLfloat mat[16];
		makeGLMatrix(mat, xform);

		if (!activateGLTextureUnit(stage))
			return;
		glMatrixMode(GL_TEXTURE);

		// Load this matrix in
		glLoadMatrixf(mat);

		if (mUseAutoTextureMatrix)
		{
			// Concat auto matrix
			glMultMatrixf(mAutoTextureMatrix);
		}

		glMatrixMode(GL_MODELVIEW);
		activateGLTextureUnit(0);
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

	void GLRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op )
	{
		GLint sourceBlend = getBlendMode(sourceFactor);
		GLint destBlend = getBlendMode(destFactor);
		if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendFunc(sourceBlend, destBlend);
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

		glBlendEquation(func);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setSeparateSceneBlending(
		SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, 
		SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha,
		SceneBlendOperation op, SceneBlendOperation alphaOp )
	{
		GLint sourceBlend = getBlendMode(sourceFactor);
		GLint destBlend = getBlendMode(destFactor);
		GLint sourceBlendAlpha = getBlendMode(sourceFactorAlpha);
		GLint destBlendAlpha = getBlendMode(destFactorAlpha);

		if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO && 
			sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			glBlendFuncSeparate(sourceBlend, destBlend, sourceBlendAlpha, destBlendAlpha);
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

		glBlendEquationSeparate(func, alphaFunc);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage)
	{
		bool a2c = false;
		static bool lasta2c = false;

		if(func == CMPF_ALWAYS_PASS)
		{
			glDisable(GL_ALPHA_TEST);
		}
		else
		{
			glEnable(GL_ALPHA_TEST);
			a2c = alphaToCoverage;
			glAlphaFunc(convertCompareFunction(func), value / 255.0f);
		}

		if (a2c != lasta2c && getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
		{
			if (a2c)
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			else
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

			lasta2c = a2c;
		}

	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setViewport(Viewport *vp)
	{
		// Check if viewport is different
		if (vp != mActiveViewport || vp->_isUpdated())
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
			glViewport(x, y, w, h);

			// Configure the viewport clipping
			glScissor(x, y, w, h);

			vp->_clearUpdatedFlag();
		}
	}

	void GLRenderSystem::setLights()
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

	//-----------------------------------------------------------------------------
	void GLRenderSystem::_beginFrame(void)
	{
		if (!mActiveViewport)
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
			"Cannot begin frame - no viewport selected.",
			"GLRenderSystem::_beginFrame");

		// Activate the viewport clipping
		glEnable(GL_SCISSOR_TEST);
	}

	//-----------------------------------------------------------------------------
	void GLRenderSystem::_endFrame(void)
	{
		// Deactivate the viewport clipping.
		glDisable(GL_SCISSOR_TEST);
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
		// NB: Because two-sided stencil API dependence of the front face, we must
		// use the same 'winding' for the front face everywhere. As the OGRE default
		// culling mode is clockwise, we also treat anticlockwise winding as front
		// face for consistently. On the assumption that, we can't change the front
		// face by glFrontFace anywhere.

		GLenum cullMode;

		switch( mode )
		{
		case CULL_NONE:
			glDisable( GL_CULL_FACE );
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

		glEnable( GL_CULL_FACE );
		glCullFace( cullMode );
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
	{
		_setDepthBufferCheckEnabled(depthTest);
		_setDepthBufferWriteEnabled(depthWrite);
		_setDepthBufferFunction(depthFunction);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			glClearDepth(1.0f);
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
	{
		GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
		glDepthMask( flag );  
		// Store for reference in _beginFrame
		mDepthWrite = enabled;
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setDepthBufferFunction(CompareFunction func)
	{
		glDepthFunc(convertCompareFunction(func));
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
	{
		if (constantBias != 0 || slopeScaleBias != 0)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_POLYGON_OFFSET_POINT);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(-slopeScaleBias, -constantBias);
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
			glDisable(GL_POLYGON_OFFSET_POINT);
			glDisable(GL_POLYGON_OFFSET_LINE);
		}
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
	{
		glColorMask(red, green, blue, alpha);
		// record this
		mColourWrite[0] = red;
		mColourWrite[1] = blue;
		mColourWrite[2] = green;
		mColourWrite[3] = alpha;
	}
	//-----------------------------------------------------------------------------
    String GLRenderSystem::getErrorDescription(long errCode) const
    {
        const GLubyte *errString = gluErrorString (errCode);
		return (errString != 0) ? String((const char*) errString) : StringUtil::BLANK;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled) 
        {      
            glEnable(GL_LIGHTING);
        } 
        else 
        {
            glDisable(GL_LIGHTING);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
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
            glDisable(GL_FOG);
            return;
        }

        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, fogMode);
        GLfloat fogColor[4] = {colour.r, colour.g, colour.b, colour.a};
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, density);
        glFogf(GL_FOG_START, start);
        glFogf(GL_FOG_END, end);
        // XXX Hint here?
    }

	VertexElementType GLRenderSystem::getColourVertexElementType(void) const
	{
		return VET_COLOUR_ABGR;
	}

	void GLRenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
		Matrix4& dest, bool forGpuProgram)
	{
		// no any convertion request for OpenGL
		dest = matrix;
	}

	void GLRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, 
		Real farPlane, Matrix4& dest, bool forGpuProgram)
	{
		Radian thetaY ( fovy / 2.0f );
		Real tanThetaY = Math::Tan(thetaY);
		//Real thetaX = thetaY * aspect;
		//Real tanThetaX = Math::Tan(thetaX);

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

	void GLRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, 
		Real farPlane, Matrix4& dest, bool forGpuProgram)
	{
		Radian thetaY (fovy / 2.0f);
		Real tanThetaY = Math::Tan(thetaY);

		//Real thetaX = thetaY * aspect;
		Real tanThetaX = tanThetaY * aspect; //Math::Tan(thetaX);
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
		dest[2][3] = - (farPlane + nearPlane)/(farPlane - nearPlane);
		dest[3][3] = 1;
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
		glPolygonMode(GL_FRONT_AND_BACK, glmode);
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setStencilCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setStencilBufferParams(CompareFunction func, 
		uint32 refValue, uint32 mask, StencilOperation stencilFailOp, 
		StencilOperation depthFailOp, StencilOperation passOp, 
		bool twoSidedOperation)
	{
		bool flip;
		mStencilMask = mask;

		if (twoSidedOperation)
		{
			if (!mCurrentCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported",
				"GLRenderSystem::setStencilBufferParams");

			// NB: We should always treat CCW as front face for consistent with default
			// culling mode. Therefore, we must take care with two-sided stencil settings.
			flip = (mInvertVertexWinding && !mActiveRenderTarget->requiresTextureFlipping()) ||
				(!mInvertVertexWinding && mActiveRenderTarget->requiresTextureFlipping());
			if(GLEW_VERSION_2_0) // New GL2 commands
			{
				// Back
				glStencilMaskSeparate(GL_BACK, mask);
				glStencilFuncSeparate(GL_BACK, convertCompareFunction(func), refValue, mask);
				glStencilOpSeparate(GL_BACK, 
					convertStencilOp(stencilFailOp, !flip), 
					convertStencilOp(depthFailOp, !flip), 
					convertStencilOp(passOp, !flip));
				// Front
				glStencilMaskSeparate(GL_FRONT, mask);
				glStencilFuncSeparate(GL_FRONT, convertCompareFunction(func), refValue, mask);
				glStencilOpSeparate(GL_FRONT, 
					convertStencilOp(stencilFailOp, flip),
					convertStencilOp(depthFailOp, flip), 
					convertStencilOp(passOp, flip));
			}
			else // EXT_stencil_two_side
			{
				glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
				// Back
				glActiveStencilFaceEXT(GL_BACK);
				glStencilMask(mask);
				glStencilFunc(convertCompareFunction(func), refValue, mask);
				glStencilOp(
					convertStencilOp(stencilFailOp, !flip), 
					convertStencilOp(depthFailOp, !flip), 
					convertStencilOp(passOp, !flip));
				// Front
				glActiveStencilFaceEXT(GL_FRONT);
				glStencilMask(mask);
				glStencilFunc(convertCompareFunction(func), refValue, mask);
				glStencilOp(
					convertStencilOp(stencilFailOp, flip),
					convertStencilOp(depthFailOp, flip), 
					convertStencilOp(passOp, flip));
			}
		}
		else
		{
			glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			flip = false;
			glStencilMask(mask);
			glStencilFunc(convertCompareFunction(func), refValue, mask);
			glStencilOp(
				convertStencilOp(stencilFailOp, flip),
				convertStencilOp(depthFailOp, flip), 
				convertStencilOp(passOp, flip));
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
	//---------------------------------------------------------------------
	GLuint GLRenderSystem::getCombinedMinMipFilter(void) const
	{
		switch(mMinFilter)
		{
		case FO_ANISOTROPIC:
		case FO_LINEAR:
			switch(mMipFilter)
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
			switch(mMipFilter)
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
	//---------------------------------------------------------------------
	void GLRenderSystem::_setTextureUnitFiltering(size_t unit, 
		FilterType ftype, FilterOptions fo)
	{
		if (!activateGLTextureUnit(unit))
			return;
		switch(ftype)
		{
		case FT_MIN:
			mMinFilter = fo;
			// Combine with existing mip filter
			glTexParameteri(
				mTextureTypes[unit],
				GL_TEXTURE_MIN_FILTER, 
				getCombinedMinMipFilter());
			break;
		case FT_MAG:
			switch (fo)
			{
			case FO_ANISOTROPIC: // GL treats linear and aniso the same
			case FO_LINEAR:
				glTexParameteri(
					mTextureTypes[unit],
					GL_TEXTURE_MAG_FILTER, 
					GL_LINEAR);
				break;
			case FO_POINT:
			case FO_NONE:
				glTexParameteri(
					mTextureTypes[unit],
					GL_TEXTURE_MAG_FILTER, 
					GL_NEAREST);
				break;
			}
			break;
		case FT_MIP:
			mMipFilter = fo;
			// Combine with existing min filter
			glTexParameteri(
				mTextureTypes[unit],
				GL_TEXTURE_MIN_FILTER, 
				getCombinedMinMipFilter());
			break;
		}

		activateGLTextureUnit(0);
	}
	//---------------------------------------------------------------------
	GLfloat GLRenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		GLfloat curAniso = 0;
		glGetTexParameterfv(mTextureTypes[unit], 
			GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso);
		return curAniso ? curAniso : 1;
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
	{
		if (!mCurrentCapabilities->hasCapability(RSC_ANISOTROPY))
			return;

		GLfloat largest_supported_anisotropy = 0;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
		if (maxAnisotropy > largest_supported_anisotropy)
			maxAnisotropy = largest_supported_anisotropy ? 
			static_cast<uint>(largest_supported_anisotropy) : 1;
		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			glTexParameterf(mTextureTypes[unit], GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	}
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
	{       
		if (stage >= mFixedFunctionTextureUnits)
		{
			// Can't do this
			return;
		}

		// Check to see if blending is supported
		if(!mCurrentCapabilities->hasCapability(RSC_BLENDING))
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
			cmd = mCurrentCapabilities->hasCapability(RSC_DOT3) 
				? GL_DOT3_RGB : GL_MODULATE;
			break;
		default:
			cmd = 0;
		}

		if (!activateGLTextureUnit(stage))
			return;
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

		float blendValue[4] = {0, 0, 0, bm.factor};
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

		activateGLTextureUnit(0);
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setGLLightPositionDirection(Light* lt, GLenum lightindex)
	{
		// Set position / direction
		Vector4 vec;
		// Use general 4D vector which is the same as GL's approach
		vec = lt->getAs4DVector(true);

#if OGRE_DOUBLE_PRECISION
		// Must convert to float*
		float tmp[4] = {vec.x, vec.y, vec.z, vec.w};
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
			float tmp2[4] = {vec.x, vec.y, vec.z, vec.w};
			glLightfv(lightindex, GL_SPOT_DIRECTION, tmp2);
#else
			glLightfv(lightindex, GL_SPOT_DIRECTION, vec.ptr());
#endif
		}
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setVertexDeclaration(VertexDeclaration* decl)
	{
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
	{
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_render(const RenderOperation& op)
	{
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
			if(mCurrentCapabilities->hasCapability(RSC_VBO))
			{
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, 
					static_cast<const GLHardwareVertexBuffer*>(vertexBuffer.get())->getGLBufferId());
				pBufferData = VBO_BUFFER_OFFSET(elem->getOffset());
			}
			else
			{
				pBufferData = static_cast<const GLDefaultHardwareVertexBuffer*>(vertexBuffer.get())->getDataPtr(elem->getOffset());
			}
			if (op.vertexData->vertexStart)
			{
				pBufferData = static_cast<char*>(pBufferData) + op.vertexData->vertexStart * vertexBuffer->getVertexSize();
			}

			unsigned int i = 0;
			VertexElementSemantic sem = elem->getSemantic();
 
 			bool isCustomAttrib = false;
 			if (mCurrentVertexProgram)
 				isCustomAttrib = mCurrentVertexProgram->isAttributeValid(sem, elem->getIndex());
 
 			// Custom attribute support
 			// tangents, binormals, blendweights etc always via this route
 			// builtins may be done this way too
 			if (isCustomAttrib)
 			{
 				GLint attrib = mCurrentVertexProgram->getAttributeIndex(sem, elem->getIndex());
 				glVertexAttribPointerARB(
 					attrib,
 					VertexElement::getTypeCount(elem->getType()), 
  					GLHardwareBufferManager::getGLType(elem->getType()), 
 					GL_FALSE, // normalisation disabled
  					static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
  					pBufferData);
 				glEnableVertexAttribArrayARB(attrib);
 
 				attribsBound.push_back(attrib);
 			}
 			else
 			{
 				// fixed-function & builtin attribute support
 				switch(sem)
  				{
 				case VES_POSITION:
 					glVertexPointer(VertexElement::getTypeCount(
 						elem->getType()), 
  						GLHardwareBufferManager::getGLType(elem->getType()), 
  						static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
  						pBufferData);
 					glEnableClientState( GL_VERTEX_ARRAY );
 					break;
 				case VES_NORMAL:
 					glNormalPointer(
 						GLHardwareBufferManager::getGLType(elem->getType()), 
  						static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
  						pBufferData);
 					glEnableClientState( GL_NORMAL_ARRAY );
 					break;
 				case VES_DIFFUSE:
 					glColorPointer(4, 
  						GLHardwareBufferManager::getGLType(elem->getType()), 
  						static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
  						pBufferData);
 					glEnableClientState( GL_COLOR_ARRAY );
 					break;
 				case VES_SPECULAR:
 					if (GLEW_EXT_secondary_color)
 					{
 						glSecondaryColorPointerEXT(4, 
 							GLHardwareBufferManager::getGLType(elem->getType()), 
 							static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
 							pBufferData);
 						glEnableClientState( GL_SECONDARY_COLOR_ARRAY );
 					}
 					break;
 				case VES_TEXTURE_COORDINATES:
  
 					if (mCurrentVertexProgram)
 					{
 						// Programmable pipeline - direct UV assignment
 						glClientActiveTextureARB(GL_TEXTURE0 + elem->getIndex());
 						glTexCoordPointer(
 							VertexElement::getTypeCount(elem->getType()), 
 							GLHardwareBufferManager::getGLType(elem->getType()),
 							static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
 							pBufferData);
 						glEnableClientState( GL_TEXTURE_COORD_ARRAY );
 					}
 					else
 					{
 						// fixed function matching to units based on tex_coord_set
 						for (i = 0; i < mDisabledTexUnitsFrom; i++)
 						{
 							// Only set this texture unit's texcoord pointer if it
 							// is supposed to be using this element's index
 							if (mTextureCoordIndex[i] == elem->getIndex() && i < mFixedFunctionTextureUnits)
 							{
								if (multitexturing)
 								glClientActiveTextureARB(GL_TEXTURE0 + i);
 								glTexCoordPointer(
 									VertexElement::getTypeCount(elem->getType()), 
 									GLHardwareBufferManager::getGLType(elem->getType()),
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

		if (multitexturing)
		glClientActiveTextureARB(GL_TEXTURE0);

		// Find the correct type to render
		GLint primType;
		//Use adjacency if there is a geometry program and it requested adjacency info
		bool useAdjacency = (mGeometryProgramBound && mCurrentGeometryProgram->isAdjacencyInfoRequired());
		switch (op.operationType)
		{
		case RenderOperation::OT_POINT_LIST:
			primType = GL_POINTS;
			break;
		case RenderOperation::OT_LINE_LIST:
			primType = useAdjacency ? GL_LINES_ADJACENCY_EXT : GL_LINES;
			break;
		case RenderOperation::OT_LINE_STRIP:
			primType = useAdjacency ? GL_LINE_STRIP_ADJACENCY_EXT : GL_LINE_STRIP;
			break;
		default:
		case RenderOperation::OT_TRIANGLE_LIST:
			primType = useAdjacency ? GL_TRIANGLES_ADJACENCY_EXT : GL_TRIANGLES;
			break;
		case RenderOperation::OT_TRIANGLE_STRIP:
			primType = useAdjacency ? GL_TRIANGLE_STRIP_ADJACENCY_EXT : GL_TRIANGLE_STRIP;
			break;
		case RenderOperation::OT_TRIANGLE_FAN:
			primType = GL_TRIANGLE_FAN;
			break;
		}

		if (op.useIndexes)
		{
			if(mCurrentCapabilities->hasCapability(RSC_VBO))
			{
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 
					static_cast<GLHardwareIndexBuffer*>(
					op.indexData->indexBuffer.get())->getGLBufferId());

				pBufferData = VBO_BUFFER_OFFSET(
					op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
			}
			else
			{
				pBufferData = static_cast<GLDefaultHardwareIndexBuffer*>(
					op.indexData->indexBuffer.get())->getDataPtr(
					op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
			}

			GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

			do
			{
				// Update derived depth bias
				if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
				{
					_setDepthBias(mDerivedDepthBiasBase + 
						mDerivedDepthBiasMultiplier * mCurrentPassIterationNum, 
						mDerivedDepthBiasSlopeScale);
				}
				glDrawElements(primType, op.indexData->indexCount, indexType, pBufferData);
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
				glDrawArrays(primType, 0, op.vertexData->vertexCount);
			} while (updatePassIterationRenderState());
		}

        glDisableClientState( GL_VERTEX_ARRAY );
		// only valid up to GL_MAX_TEXTURE_COORDS, which is recorded in mFixedFunctionTextureUnits
		if (multitexturing)
        {
			for (int i = 0; i < mFixedFunctionTextureUnits; i++)
			{
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
		if (GLEW_EXT_secondary_color)
		{
			glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
		}
 		// unbind any custom attributes
		for (vector<GLuint>::type::iterator ai = attribsBound.begin(); ai != attribsBound.end(); ++ai)
 		{
 			glDisableVertexAttribArrayARB(*ai); 
 
  		}
		
		glColor4f(1,1,1,1);
		if (GLEW_EXT_secondary_color)
		{
			glSecondaryColor3fEXT(0.0f, 0.0f, 0.0f);
		}

	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setNormaliseNormals(bool normalise)
	{
		if (normalise)
			glEnable(GL_NORMALIZE);
		else
			glDisable(GL_NORMALIZE);

	}
	//---------------------------------------------------------------------
	void GLRenderSystem::bindGpuProgram(GpuProgram* prg)
	{
		GLGpuProgram* glprg = static_cast<GLGpuProgram*>(prg);

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
		}

		// Bind the program
		glprg->bindProgram();

		RenderSystem::bindGpuProgram(prg);
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::unbindGpuProgram(GpuProgramType gptype)
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
		RenderSystem::unbindGpuProgram(gptype);

	}
	//---------------------------------------------------------------------
	void GLRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask)
	{
		if (mask & (uint16)GPV_GLOBAL)
		{
			// We could maybe use GL_EXT_bindable_uniform here to produce Dx10-style
			// shared constant buffers, but GPU support seems fairly weak?
			// for now, just copy
			params->_copySharedParams();
		}

		switch (gptype)
		{
		case GPT_VERTEX_PROGRAM:
			mActiveVertexGpuProgramParameters = params;
			mCurrentVertexProgram->bindProgramParameters(params, mask);
			break;
		case GPT_GEOMETRY_PROGRAM:
			mActiveGeometryGpuProgramParameters = params;
			mCurrentGeometryProgram->bindProgramParameters(params, mask);
			break;
		case GPT_FRAGMENT_PROGRAM:
			mActiveFragmentGpuProgramParameters = params;
			mCurrentFragmentProgram->bindProgramParameters(params, mask);
			break;
		}
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
	{
		switch (gptype)
		{
		case GPT_VERTEX_PROGRAM:
			mCurrentVertexProgram->bindProgramPassIterationParameters(mActiveVertexGpuProgramParameters);
			break;
		case GPT_GEOMETRY_PROGRAM:
			mCurrentGeometryProgram->bindProgramPassIterationParameters(mActiveGeometryGpuProgramParameters);
			break;
		case GPT_FRAGMENT_PROGRAM:
			mCurrentFragmentProgram->bindProgramPassIterationParameters(mActiveFragmentGpuProgramParameters);
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
			glEnable(clipPlaneId);
		}

		// disable remaining clip planes
		for ( ; i < 6/*GL_MAX_CLIP_PLANES*/; ++i)
		{
			glDisable(static_cast<GLenum>(GL_CLIP_PLANE0 + i));
		}

		// restore matrices
		glPopMatrix();
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::setScissorTest(bool enabled, size_t left, 
		size_t top, size_t right, size_t bottom)
	{
		// If request texture flipping, use "upper-left", otherwise use "lower-left"
		bool flipping = mActiveRenderTarget->requiresTextureFlipping();
		//  GL measures from the bottom, not the top
		size_t targetHeight = mActiveRenderTarget->getHeight();
		// Calculate the "lower-left" corner of the viewport
		GLsizei w, h, x, y;

		if (enabled)
		{
			glEnable(GL_SCISSOR_TEST);
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
			glDisable(GL_SCISSOR_TEST);
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
	//---------------------------------------------------------------------
	void GLRenderSystem::clearFrameBuffer(unsigned int buffers, 
		const ColourValue& colour, Real depth, unsigned short stencil)
	{

		bool colourMask = !mColourWrite[0] || !mColourWrite[1] 
		|| !mColourWrite[2] || !mColourWrite[3]; 

		GLbitfield flags = 0;
		if (buffers & FBT_COLOUR)
		{
			flags |= GL_COLOR_BUFFER_BIT;
			// Enable buffer for writing if it isn't
			if (colourMask)
			{
				glColorMask(true, true, true, true);
			}
			glClearColor(colour.r, colour.g, colour.b, colour.a);
		}
		if (buffers & FBT_DEPTH)
		{
			flags |= GL_DEPTH_BUFFER_BIT;
			// Enable buffer for writing if it isn't
			if (!mDepthWrite)
			{
				glDepthMask( GL_TRUE );
			}
			glClearDepth(depth);
		}
		if (buffers & FBT_STENCIL)
		{
			flags |= GL_STENCIL_BUFFER_BIT;
			// Enable buffer for writing if it isn't
			glStencilMask(0xFFFFFFFF);

			glClearStencil(stencil);
		}

		// Should be enable scissor test due the clear region is
		// relied on scissor box bounds.
		GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
		if (!scissorTestEnabled)
		{
			glEnable(GL_SCISSOR_TEST);
		}

		// Sets the scissor box as same as viewport
		GLint viewport[4], scissor[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetIntegerv(GL_SCISSOR_BOX, scissor);
		bool scissorBoxDifference =
			viewport[0] != scissor[0] || viewport[1] != scissor[1] ||
			viewport[2] != scissor[2] || viewport[3] != scissor[3];
		if (scissorBoxDifference)
		{
			glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		}

		// Clear buffers
		glClear(flags);

		// Restore scissor box
		if (scissorBoxDifference)
		{
			glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
		}
		// Restore scissor test
		if (!scissorTestEnabled)
		{
			glDisable(GL_SCISSOR_TEST);
		}

		// Reset buffer write state
		if (!mDepthWrite && (buffers & FBT_DEPTH))
		{
			glDepthMask( GL_FALSE );
		}
		if (colourMask && (buffers & FBT_COLOUR))
		{
			glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
		}
		if (buffers & FBT_STENCIL)
		{
			glStencilMask(mStencilMask);
		}

	}
	// ------------------------------------------------------------------
	void GLRenderSystem::_makeProjectionMatrix(Real left, Real right, 
		Real bottom, Real top, Real nearPlane, Real farPlane, Matrix4& dest, 
		bool forGpuProgram)
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
	//---------------------------------------------------------------------
	HardwareOcclusionQuery* GLRenderSystem::createHardwareOcclusionQuery(void)
	{
		GLHardwareOcclusionQuery* ret = new GLHardwareOcclusionQuery(); 
		mHwOcclusionQueries.push_back(ret);
		return ret;
	}
	//---------------------------------------------------------------------
	Real GLRenderSystem::getHorizontalTexelOffset(void)
	{
		// No offset in GL
		return 0.0f;
	}
	//---------------------------------------------------------------------
	Real GLRenderSystem::getVerticalTexelOffset(void)
	{
		// No offset in GL
		return 0.0f;
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
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
	//---------------------------------------------------------------------
	void GLRenderSystem::_oneTimeContextInitialization()
	{
		if (GLEW_VERSION_1_2)
		{
		// Set nicer lighting model -- d3d9 has this by default
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);        
		}
		if (GLEW_VERSION_1_4)
		{
		glEnable(GL_COLOR_SUM);
		glDisable(GL_DITHER);
		}

		// Check for FSAA
		// Enable the extension if it was enabled by the GLSupport
		if (mGLSupport->checkExtension("GL_ARB_multisample"))
		{
			int fsaa_active = false;
			glGetIntegerv(GL_SAMPLE_BUFFERS_ARB,(GLint*)&fsaa_active);
			if(fsaa_active)
			{
				glEnable(GL_MULTISAMPLE_ARB);
				LogManager::getSingleton().logMessage("Using FSAA from GL_ARB_multisample extension.");
			}            
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

		// It's ready to switching
		if (mCurrentContext)
			mCurrentContext->endCurrent();
		mCurrentContext = context;
		mCurrentContext->setCurrent();

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
		glDepthMask(mDepthWrite);
		glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
		glStencilMask(mStencilMask);

	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_setRenderTarget(RenderTarget *target)
	{
		// Unbind frame buffer object
		if(mActiveRenderTarget)
			mRTTManager->unbind(mActiveRenderTarget);

		mActiveRenderTarget = target;

		// Switch context if different from current one
		GLContext *newContext = 0;
		target->getCustomAttribute("GLCONTEXT", &newContext);
		if(newContext && mCurrentContext != newContext) 
		{
			_switchContext(newContext);
		}

		// Bind frame buffer object
		mRTTManager->bind(target);

		if (GLEW_VERSION_1_2)
		{
		// Enable / disable sRGB states
		if (target->isHardwareGammaEnabled())
		{
			glEnable(GL_FRAMEBUFFER_SRGB_EXT);
			
			// Note: could test GL_FRAMEBUFFER_SRGB_CAPABLE_EXT here before
			// enabling, but GL spec says incapable surfaces ignore the setting
			// anyway. We test the capability to enable isHardwareGammaEnabled.
		}
		else
		{
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);
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
			}
		}
	}
	//---------------------------------------------------------------------
	Real GLRenderSystem::getMinimumDepthInputValue(void)
	{
		// Range [-1.0f, 1.0f]
		return -1.0f;
	}
	//---------------------------------------------------------------------
	Real GLRenderSystem::getMaximumDepthInputValue(void)
	{
		// Range [-1.0f, 1.0f]
		return 1.0f;
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::registerThread()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex)
		// This is only valid once we've created the main context
		if (!mMainContext)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Cannot register a background thread before the main context "
				"has been created.", 
				"GLRenderSystem::registerThread");
		}

		// Create a new context for this thread. Cloning from the main context
		// will ensure that resources are shared with the main context
		// We want a separate context so that we can safely create GL
		// objects in parallel with the main thread
		GLContext* newContext = mMainContext->clone();
		mBackgroundContextList.push_back(newContext);

		// Bind this new context to this thread. 
		newContext->setCurrent();

		_oneTimeContextInitialization();
		newContext->setInitialized();


	}
	//---------------------------------------------------------------------
	void GLRenderSystem::unregisterThread()
	{
		// nothing to do here?
		// Don't need to worry about active context, just make sure we delete
		// on shutdown.

	}
	//---------------------------------------------------------------------
	void GLRenderSystem::preExtraThreadsStarted()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex)
		// free context, we'll need this to share lists
		mCurrentContext->endCurrent();
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::postExtraThreadsStarted()
	{
		OGRE_LOCK_MUTEX(mThreadInitMutex)
		// reacquire context
		mCurrentContext->setCurrent();
	}
	//---------------------------------------------------------------------
	bool GLRenderSystem::activateGLTextureUnit(size_t unit)
	{
		if (mActiveTextureUnit != unit)
		{
			if (GLEW_VERSION_1_2 && unit < getCapabilities()->getNumTextureUnits())
			{
				glActiveTextureARB(GL_TEXTURE0 + unit);
				mActiveTextureUnit = unit;
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

	//---------------------------------------------------------------------
	unsigned int GLRenderSystem::getDisplayMonitorCount() const
	{
		return mGLSupport->getDisplayMonitorCount();
	}


}
