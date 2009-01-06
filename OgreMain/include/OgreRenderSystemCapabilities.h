/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __RenderSystemCapabilities__
#define __RenderSystemCapabilities__ 1

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreResource.h"
#include "OgreLogManager.h"

// Because there are more than 32 possible Capabilities, more than 1 int is needed to store them all.
// In fact, an array of integers is used to store capabilities. However all the capabilities are defined in the single
// enum. The only way to know which capabilities should be stored where in the array is to use some of the 32 bits
// to record the category of the capability.  These top few bits are used as an index into mCapabilities array
// The lower bits are used to identify each capability individually by setting 1 bit for each

// Identifies how many bits are reserved for categories
// NOTE: Although 4 bits (currently) are enough
#define CAPS_CATEGORY_SIZE 4
#define OGRE_CAPS_BITSHIFT (32 - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK (((1 << CAPS_CATEGORY_SIZE) - 1) << OGRE_CAPS_BITSHIFT)
#define OGRE_CAPS_VALUE(cat, val) ((cat << OGRE_CAPS_BITSHIFT) | (1 << val))

namespace Ogre 
{
	/// Enumerates the categories of capabilities
	enum CapabilitiesCategory
	{
		CAPS_CATEGORY_COMMON = 0,
		CAPS_CATEGORY_COMMON_2 = 1,
		CAPS_CATEGORY_D3D9 = 2,
		CAPS_CATEGORY_GL = 3,
		/// Placeholder for max value
		CAPS_CATEGORY_COUNT = 4
	};

	/// Enum describing the different hardware capabilities we want to check for
	/// OGRE_CAPS_VALUE(a, b) defines each capability
	// a is the category (which can be from 0 to 15)
	// b is the value (from 0 to 27)
	enum Capabilities
	{
		/// Supports generating mipmaps in hardware
		RSC_AUTOMIPMAP              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 0),
		RSC_BLENDING                = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 1),
		/// Supports anisotropic texture filtering
		RSC_ANISOTROPY              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 2),
		/// Supports fixed-function DOT3 texture blend
		RSC_DOT3                    = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 3),
		/// Supports cube mapping
		RSC_CUBEMAPPING             = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 4),
		/// Supports hardware stencil buffer
		RSC_HWSTENCIL               = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 5),
		/// Supports hardware vertex and index buffers
		RSC_VBO                     = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 7),
		/// Supports vertex programs (vertex shaders
		RSC_VERTEX_PROGRAM          = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 9),
		/// Supports fragment programs (pixel shaders)
		RSC_FRAGMENT_PROGRAM        = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 10),
		/// Supports performing a scissor test to exclude areas of the screen
		RSC_SCISSOR_TEST            = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 11),
		/// Supports separate stencil updates for both front and back faces
		RSC_TWO_SIDED_STENCIL       = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 12),
		/// Supports wrapping the stencil value at the range extremeties
		RSC_STENCIL_WRAP            = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 13),
		/// Supports hardware occlusion queries
		RSC_HWOCCLUSION             = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 14),
		/// Supports user clipping planes
		RSC_USER_CLIP_PLANES        = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 15),
		/// Supports the VET_UBYTE4 vertex element type
		RSC_VERTEX_FORMAT_UBYTE4    = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 16),
		/// Supports infinite far plane projection
		RSC_INFINITE_FAR_PLANE      = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 17),
		/// Supports hardware render-to-texture (bigger than framebuffer)
		RSC_HWRENDER_TO_TEXTURE     = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 18),
		/// Supports float textures and render targets
		RSC_TEXTURE_FLOAT           = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 19),
		/// Supports non-power of two textures
		RSC_NON_POWER_OF_2_TEXTURES = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 20),
		/// Supports 3d (volume) textures
		RSC_TEXTURE_3D              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 21),
		/// Supports basic point sprite rendering
		RSC_POINT_SPRITES           = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 22),
		/// Supports extra point parameters (minsize, maxsize, attenuation)
		RSC_POINT_EXTENDED_PARAMETERS = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 23),
		/// Supports vertex texture fetch
		RSC_VERTEX_TEXTURE_FETCH = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 24),
		/// Supports mipmap LOD biasing
		RSC_MIPMAP_LOD_BIAS = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 25),
		/// Supports hardware geometry programs
		RSC_GEOMETRY_PROGRAM = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 26),
		/// Supports rendering to vertex buffers
		RSC_HWRENDER_TO_VERTEX_BUFFER = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 27),

		/// Supports compressed textures
		RSC_TEXTURE_COMPRESSION = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 0),
		/// Supports compressed textures in the DXT/ST3C formats
		RSC_TEXTURE_COMPRESSION_DXT = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 1),
		/// Supports compressed textures in the VTC format
		RSC_TEXTURE_COMPRESSION_VTC = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 2),
		/// Supports fixed-function pipeline
		RSC_FIXED_FUNCTION = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 3),
		/// Supports MRTs with different bit depths
		RSC_MRT_DIFFERENT_BIT_DEPTHS = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 4),
		/// Supports Alpha to Coverage (A2C)
		RSC_ALPHA_TO_COVERAGE = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 5),
		/// Supports Blending operations other than +
		RSC_ADVANCED_BLEND_OPERATIONS = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 6),

		// ***** DirectX specific caps *****
		/// Is DirectX feature "per stage constants" supported
		RSC_PERSTAGECONSTANT = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 0),

		// ***** GL Specific Caps *****
		/// Supports openGL GLEW version 1.5
		RSC_GL1_5_NOVBO    = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 1),
		/// Support for Frame Buffer Objects (FBOs)
		RSC_FBO              = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 2),
		/// Support for Frame Buffer Objects ARB implementation (regular FBO is higher precedence)
		RSC_FBO_ARB          = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 3),
		/// Support for Frame Buffer Objects ATI implementation (ARB FBO is higher precedence)
		RSC_FBO_ATI          = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 4),
		/// Support for PBuffer
		RSC_PBUFFER          = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 5),
		/// Support for GL 1.5 but without HW occlusion workaround
		RSC_GL1_5_NOHWOCCLUSION = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 6),
		/// Support for point parameters ARB implementation
		RSC_POINT_EXTENDED_PARAMETERS_ARB = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 7),
		/// Support for point parameters EXT implementation
		RSC_POINT_EXTENDED_PARAMETERS_EXT = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 8),


	};

	/// DriverVersion is used by RenderSystemCapabilities and both GL and D3D9
	/// to store the version of the current GPU driver
	struct _OgreExport DriverVersion 
	{
		int major;
		int minor;
		int release;
		int build;

		DriverVersion() 
		{
			major = minor = release = build = 0;
		}

		String toString() const 
		{
			StringUtil::StrStreamType str;
			str << major << "." << minor << "." << release << "." << build;
			return str.str();
		}

		void fromString(const String& versionString)
		{
			StringVector tokens = StringUtil::split(versionString, ".");
			if(!tokens.empty())
			{
				major = StringConverter::parseInt(tokens[0]);
				if (tokens.size() > 1)
					minor = StringConverter::parseInt(tokens[1]);
				if (tokens.size() > 2)
					release = StringConverter::parseInt(tokens[2]);
				if (tokens.size() > 3)
					build = StringConverter::parseInt(tokens[3]);
			}

		}
	};

	/** Enumeration of GPU vendors. */
	enum GPUVendor
	{
		GPU_UNKNOWN = 0,
		GPU_NVIDIA = 1,
		GPU_ATI = 2, 
		GPU_INTEL = 3,
		GPU_S3 = 4,
		GPU_MATROX = 5,
		GPU_3DLABS = 6,
		/// placeholder
		GPU_VENDOR_COUNT = 7
	};

	/** singleton class for storing the capabilities of the graphics card. 
	@remarks
	This class stores the capabilities of the graphics card.  This
	information is set by the individual render systems.
	*/
	class _OgreExport RenderSystemCapabilities : public RenderSysAlloc
	{

	public:

		typedef set<String>::type ShaderProfiles;
	private:
		/// This is used to build a database of RSC's
		/// if a RSC with same name, but newer version is introduced, the older one 
		/// will be removed
		DriverVersion mDriverVersion;
		/// GPU Vendor
		GPUVendor mVendor;

		static StringVector msGPUVendorStrings;
		static void initVendorStrings();

		/// The number of world matrices available
		ushort mNumWorldMatrices;
		/// The number of texture units available
		ushort mNumTextureUnits;
		/// The stencil buffer bit depth
		ushort mStencilBufferBitDepth;
		/// The number of matrices available for hardware blending
		ushort mNumVertexBlendMatrices;
		/// Stores the capabilities flags.
		int mCapabilities[CAPS_CATEGORY_COUNT];
		/// Which categories are relevant
		bool mCategoryRelevant[CAPS_CATEGORY_COUNT];
		/// The name of the device as reported by the render system
		String mDeviceName;
		/// The identifier associated with the render system for which these capabilities are valid
		String mRenderSystemName;

		/// The number of floating-point constants vertex programs support
		ushort mVertexProgramConstantFloatCount;           
		/// The number of integer constants vertex programs support
		ushort mVertexProgramConstantIntCount;           
		/// The number of boolean constants vertex programs support
		ushort mVertexProgramConstantBoolCount;           
		/// The number of floating-point constats geometry programs support
		ushort mGeometryProgramConstantFloatCount;           
		/// The number of integer constants vertex geometry support
		ushort mGeometryProgramConstantIntCount;           
		/// The number of boolean constants vertex geometry support
		ushort mGeometryProgramConstantBoolCount;           
		/// The number of floating-point constants fragment programs support
		ushort mFragmentProgramConstantFloatCount;           
		/// The number of integer constants fragment programs support
		ushort mFragmentProgramConstantIntCount;           
		/// The number of boolean constants fragment programs support
		ushort mFragmentProgramConstantBoolCount;
		/// The number of simultaneous render targets supported
		ushort mNumMultiRenderTargets;
		/// The maximum point size
		Real mMaxPointSize;
		/// Are non-POW2 textures feature-limited?
		bool mNonPOW2TexturesLimited;
		/// The number of vertex texture units supported
		ushort mNumVertexTextureUnits;
		/// Are vertex texture units shared with fragment processor?
		bool mVertexTextureUnitsShared;
		/// The number of vertices a geometry program can emit in a single run
		int mGeometryProgramNumOutputVertices;


		/// The list of supported shader profiles
		ShaderProfiles mSupportedShaderProfiles;

	public:	
		RenderSystemCapabilities ();
		virtual ~RenderSystemCapabilities ();

		virtual size_t calculateSize() const {return 0;}

		/** Set the driver version. */
		void setDriverVersion(const DriverVersion& version)
		{
			mDriverVersion = version;
		}

		void parseDriverVersionFromString(const String& versionString)
		{
			DriverVersion version;
			version.fromString(versionString);
			setDriverVersion(version);
		}


		DriverVersion getDriverVersion() const
		{
			return mDriverVersion;
		}

		GPUVendor getVendor() const
		{
			return mVendor;
		}

		void setVendor(GPUVendor v)
		{
			mVendor = v;
		}

		/// Parse and set vendor
		void parseVendorFromString(const String& vendorString)
		{
			setVendor(vendorFromString(vendorString));
		}

		/// Convert a vendor string to an enum
		static GPUVendor vendorFromString(const String& vendorString);
		/// Convert a vendor enum to a string
		static String vendorToString(GPUVendor v);

		bool isDriverOlderThanVersion(DriverVersion v) const
		{
			if (mDriverVersion.major < v.major)
				return true;
			else if (mDriverVersion.major == v.major && 
				mDriverVersion.minor < v.minor)
				return true;
			else if (mDriverVersion.major == v.major && 
				mDriverVersion.minor == v.minor && 
				mDriverVersion.release < v.release)
				return true;
			else if (mDriverVersion.major == v.major && 
				mDriverVersion.minor == v.minor && 
				mDriverVersion.release == v.release &&
				mDriverVersion.build < v.build)
				return true;
			return false;
		}

		void setNumWorldMatrices(ushort num)
		{
			mNumWorldMatrices = num;
		}

		void setNumTextureUnits(ushort num)
		{
			mNumTextureUnits = num;
		}

		void setStencilBufferBitDepth(ushort num)
		{
			mStencilBufferBitDepth = num;
		}

		void setNumVertexBlendMatrices(ushort num)
		{
			mNumVertexBlendMatrices = num;
		}

		/// The number of simultaneous render targets supported
		void setNumMultiRenderTargets(ushort num)
		{
			mNumMultiRenderTargets = num;
		}

		ushort getNumWorldMatrices(void) const
		{ 
			return mNumWorldMatrices;
		}

		/** Returns the number of texture units the current output hardware
		supports.

		For use in rendering, this determines how many texture units the
		are available for multitexturing (i.e. rendering multiple 
		textures in a single pass). Where a Material has multiple 
		texture layers, it will try to use multitexturing where 
		available, and where it is not available, will perform multipass
		rendering to achieve the same effect. This property only applies
		to the fixed-function pipeline, the number available to the 
		programmable pipeline depends on the shader model in use.
		*/
		ushort getNumTextureUnits(void) const
		{
			return mNumTextureUnits;
		}

		/** Determines the bit depth of the hardware accelerated stencil 
		buffer, if supported.
		@remarks
		If hardware stencilling is not supported, the software will
		provide an 8-bit software stencil.
		*/
		ushort getStencilBufferBitDepth(void) const
		{
			return mStencilBufferBitDepth;
		}

		/** Returns the number of matrices available to hardware vertex 
		blending for this rendering system. */
		ushort getNumVertexBlendMatrices(void) const
		{
			return mNumVertexBlendMatrices;
		}

		/// The number of simultaneous render targets supported
		ushort getNumMultiRenderTargets(void) const
		{
			return mNumMultiRenderTargets;
		}

		/** Returns true if capability is render system specific
		*/
		bool isCapabilityRenderSystemSpecific(const Capabilities c)
		{
			int cat = c >> OGRE_CAPS_BITSHIFT;
			if(cat == CAPS_CATEGORY_GL || cat == CAPS_CATEGORY_D3D9)
				return true;
			return false;
		}

		/** Adds a capability flag
		*/
		void setCapability(const Capabilities c) 
		{ 
			int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
			// zero out the index from the stored capability
			mCapabilities[index] |= (c & ~CAPS_CATEGORY_MASK);
		}

		/** Remove a capability flag
		*/
		void unsetCapability(const Capabilities c) 
		{ 
			int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
			// zero out the index from the stored capability
			mCapabilities[index] &= (~c | CAPS_CATEGORY_MASK);
		}

		/** Checks for a capability
		*/
		bool hasCapability(const Capabilities c) const
		{
			int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
			// test against
			if(mCapabilities[index] & (c & ~CAPS_CATEGORY_MASK))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		/** Adds the profile to the list of supported profiles
		*/
		void addShaderProfile(const String& profile)
		{
			mSupportedShaderProfiles.insert(profile);

		}

		/** Remove a given shader profile, if present.
		*/
		void removeShaderProfile(const String& profile)
		{
			mSupportedShaderProfiles.erase(profile);
		}

		/** Returns true if profile is in the list of supported profiles
		*/
		bool isShaderProfileSupported(const String& profile) const
		{
			return (mSupportedShaderProfiles.end() != mSupportedShaderProfiles.find(profile));
		}


		/** Returns a set of all supported shader profiles
		* */
		const ShaderProfiles& getSupportedShaderProfiles() const
		{
			return mSupportedShaderProfiles;
		}


		/// The number of floating-point constants vertex programs support
		ushort getVertexProgramConstantFloatCount(void) const
		{
			return mVertexProgramConstantFloatCount;           
		}
		/// The number of integer constants vertex programs support
		ushort getVertexProgramConstantIntCount(void) const
		{
			return mVertexProgramConstantIntCount;           
		}
		/// The number of boolean constants vertex programs support
		ushort getVertexProgramConstantBoolCount(void) const
		{
			return mVertexProgramConstantBoolCount;           
		}
		/// The number of floating-point constants geometry programs support
		ushort getGeometryProgramConstantFloatCount(void) const
		{
			return mGeometryProgramConstantFloatCount;           
		}
		/// The number of integer constants geometry programs support
		ushort getGeometryProgramConstantIntCount(void) const
		{
			return mGeometryProgramConstantIntCount;           
		}
		/// The number of boolean constants geometry programs support
		ushort getGeometryProgramConstantBoolCount(void) const
		{
			return mGeometryProgramConstantBoolCount;           
		}
		/// The number of floating-point constants fragment programs support
		ushort getFragmentProgramConstantFloatCount(void) const
		{
			return mFragmentProgramConstantFloatCount;           
		}
		/// The number of integer constants fragment programs support
		ushort getFragmentProgramConstantIntCount(void) const
		{
			return mFragmentProgramConstantIntCount;           
		}
		/// The number of boolean constants fragment programs support
		ushort getFragmentProgramConstantBoolCount(void) const
		{
			return mFragmentProgramConstantBoolCount;           
		}

		/// sets the device name for Render system
		void setDeviceName(const String& name)
		{
			mDeviceName = name;
		}

		/// gets the device name for render system
		String getDeviceName() const
		{
			return mDeviceName;
		}

		/// The number of floating-point constants vertex programs support
		void setVertexProgramConstantFloatCount(ushort c)
		{
			mVertexProgramConstantFloatCount = c;           
		}
		/// The number of integer constants vertex programs support
		void setVertexProgramConstantIntCount(ushort c)
		{
			mVertexProgramConstantIntCount = c;           
		}
		/// The number of boolean constants vertex programs support
		void setVertexProgramConstantBoolCount(ushort c)
		{
			mVertexProgramConstantBoolCount = c;           
		}
		/// The number of floating-point constants geometry programs support
		void setGeometryProgramConstantFloatCount(ushort c)
		{
			mGeometryProgramConstantFloatCount = c;           
		}
		/// The number of integer constants geometry programs support
		void setGeometryProgramConstantIntCount(ushort c)
		{
			mGeometryProgramConstantIntCount = c;           
		}
		/// The number of boolean constants geometry programs support
		void setGeometryProgramConstantBoolCount(ushort c)
		{
			mGeometryProgramConstantBoolCount = c;           
		}
		/// The number of floating-point constants fragment programs support
		void setFragmentProgramConstantFloatCount(ushort c)
		{
			mFragmentProgramConstantFloatCount = c;           
		}
		/// The number of integer constants fragment programs support
		void setFragmentProgramConstantIntCount(ushort c)
		{
			mFragmentProgramConstantIntCount = c;           
		}
		/// The number of boolean constants fragment programs support
		void setFragmentProgramConstantBoolCount(ushort c)
		{
			mFragmentProgramConstantBoolCount = c;           
		}
		/// Maximum point screen size in pixels
		void setMaxPointSize(Real s)
		{
			mMaxPointSize = s;
		}
		/// Maximum point screen size in pixels
		Real getMaxPointSize(void) const
		{
			return mMaxPointSize;
		}
		/// Non-POW2 textures limited
		void setNonPOW2TexturesLimited(bool l)
		{
			mNonPOW2TexturesLimited = l;
		}
		/** Are non-power of two textures limited in features?
		@remarks
		If the RSC_NON_POWER_OF_2_TEXTURES capability is set, but this
		method returns true, you can use non power of 2 textures only if:
		<ul><li>You load them explicitly with no mip maps</li>
		<li>You don't use DXT texture compression</li>
		<li>You use clamp texture addressing</li></ul>
		*/
		bool getNonPOW2TexturesLimited(void) const
		{
			return mNonPOW2TexturesLimited;
		}

		/// Set the number of vertex texture units supported
		void setNumVertexTextureUnits(ushort n)
		{
			mNumVertexTextureUnits = n;
		}
		/// Get the number of vertex texture units supported
		ushort getNumVertexTextureUnits(void) const
		{
			return mNumVertexTextureUnits;
		}
		/// Set whether the vertex texture units are shared with the fragment processor
		void setVertexTextureUnitsShared(bool shared)
		{
			mVertexTextureUnitsShared = shared;
		}
		/// Get whether the vertex texture units are shared with the fragment processor
		bool getVertexTextureUnitsShared(void) const
		{
			return mVertexTextureUnitsShared;
		}

		/// Set the number of vertices a single geometry program run can emit
		void setGeometryProgramNumOutputVertices(int numOutputVertices)
		{
			mGeometryProgramNumOutputVertices = numOutputVertices;
		}
		/// Get the number of vertices a single geometry program run can emit
		int getGeometryProgramNumOutputVertices(void) const
		{
			return mGeometryProgramNumOutputVertices;
		}

		/// Get the identifier of the rendersystem from which these capabilities were generated
		String getRenderSystemName(void) const
		{
			return mRenderSystemName;
		}
		///  Set the identifier of the rendersystem from which these capabilities were generated
		void setRenderSystemName(const String& rs)
		{
			mRenderSystemName = rs;
		}

		/// Mark a category as 'relevant' or not, ie will it be reported
		void setCategoryRelevant(CapabilitiesCategory cat, bool relevant)
		{
			mCategoryRelevant[cat] = relevant;
		}

		/// Return whether a category is 'relevant' or not, ie will it be reported
		bool isCategoryRelevant(CapabilitiesCategory cat)
		{
			return mCategoryRelevant[cat];
		}



		/** Write the capabilities to the pass in Log */
		void log(Log* pLog);

	};

} // namespace

#endif // __RenderSystemCapabilities__

