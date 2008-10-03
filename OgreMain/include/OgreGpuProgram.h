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
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __GpuProgram_H_
#define __GpuProgram_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreResource.h"
#include "OgreSharedPtr.h"
#include "OgreIteratorWrappers.h"
#include "OgreSerializer.h"
#include "OgreRenderOperation.h"

namespace Ogre {

	/** Enumerates the types of programs which can run on the GPU. */
	enum GpuProgramType
	{
		GPT_VERTEX_PROGRAM,
		GPT_FRAGMENT_PROGRAM,
		GPT_GEOMETRY_PROGRAM
	};

	/** Enumeration of the types of constant we may encounter in programs. 
	@note Low-level programs, by definition, will always use either
	float4 or int4 constant types since that is the fundamental underlying
	type in assembler.
	*/
	enum GpuConstantType
	{
		GCT_FLOAT1 = 1,
		GCT_FLOAT2 = 2,
		GCT_FLOAT3 = 3,
		GCT_FLOAT4 = 4,
		GCT_SAMPLER1D = 5,
		GCT_SAMPLER2D = 6,
		GCT_SAMPLER3D = 7,
		GCT_SAMPLERCUBE = 8,
		GCT_SAMPLER1DSHADOW = 9,
		GCT_SAMPLER2DSHADOW = 10,
		GCT_MATRIX_2X2 = 11,
		GCT_MATRIX_2X3 = 12,
		GCT_MATRIX_2X4 = 13,
		GCT_MATRIX_3X2 = 14,
		GCT_MATRIX_3X3 = 15,
		GCT_MATRIX_3X4 = 16,
		GCT_MATRIX_4X2 = 17,
		GCT_MATRIX_4X3 = 18,
		GCT_MATRIX_4X4 = 19,
		GCT_INT1 = 20,
		GCT_INT2 = 21,
		GCT_INT3 = 22,
		GCT_INT4 = 23,
		GCT_UNKNOWN = 99
	};

	/** Information about predefined program constants. 
	@note Only available for high-level programs but is referenced generically
		by GpuProgramParameters.
	*/
	struct _OgreExport GpuConstantDefinition
	{
		/// Data type
		GpuConstantType constType;
		/// Physical start index in buffer (either float or int buffer)
		size_t physicalIndex;
		/// Logical index - used to communicate this constant to the rendersystem
		size_t logicalIndex;
		/** Number of raw buffer slots per element 
		(some programs pack each array element to float4, some do not) */
		size_t elementSize;
		/// Length of array
		size_t arraySize;

		bool isFloat() const
		{
			switch(constType)
			{
			case GCT_INT1:
			case GCT_INT2:
			case GCT_INT3:
			case GCT_INT4:
			case GCT_SAMPLER1D:
			case GCT_SAMPLER2D:
			case GCT_SAMPLER3D:
			case GCT_SAMPLERCUBE:
			case GCT_SAMPLER1DSHADOW:
			case GCT_SAMPLER2DSHADOW:
				return false;
			default:
				return true;
			};

		}

		bool isSampler() const
		{
			switch(constType)
			{
			case GCT_SAMPLER1D:
			case GCT_SAMPLER2D:
			case GCT_SAMPLER3D:
			case GCT_SAMPLERCUBE:
			case GCT_SAMPLER1DSHADOW:
			case GCT_SAMPLER2DSHADOW:
				return true;
			default:
				return false;
			};

		}

		GpuConstantDefinition()
			: constType(GCT_UNKNOWN)
			, physicalIndex((std::numeric_limits<size_t>::max)())
			, elementSize(0)
			, arraySize(1) {}
	};
	typedef std::map<String, GpuConstantDefinition> GpuConstantDefinitionMap;
	typedef ConstMapIterator<GpuConstantDefinitionMap> GpuConstantDefinitionIterator;

	/// Struct collecting together the information for named constants.
	struct _OgreExport GpuNamedConstants
	{
		/// Total size of the float buffer required
		size_t floatBufferSize;
		/// Total size of the int buffer required
		size_t intBufferSize;
		/// Map of parameter names to GpuConstantDefinition
		GpuConstantDefinitionMap map;

		/** Generate additional constant entries for arrays based on a base definition.
		@remarks
			Array uniforms will be added just with their base name with no array
			suffix. This method will add named entries for array suffixes too
			so individual array entries can be addressed. Note that we only 
			individually index array elements if the array size is up to 16
			entries in size. Anything larger than that only gets a [0] entry
			as well as the main entry, to save cluttering up the name map. After
			all, you can address the larger arrays in a bulk fashion much more
			easily anyway. 
		*/
		void generateConstantDefinitionArrayEntries(const String& paramName, 
			const GpuConstantDefinition& baseDef);

        /// Indicates whether all array entries will be generated and added to the definitions map
        static bool getGenerateAllConstantDefinitionArrayEntries();

        /** Sets whether all array entries will be generated and added to the definitions map.
		@remarks
			Usually, array entries can only be individually indexed if they're up to 16 entries long,
			to save memory - arrays larger than that can be set but only via the bulk setting
			methods. This option allows you to choose to individually index every array entry. 
		*/
        static void setGenerateAllConstantDefinitionArrayEntries(bool generateAll);

		/** Saves constant definitions to a file, compatible with GpuProgram::setManualNamedConstantsFile. 
		@see GpuProgram::setManualNamedConstantsFile
		*/
		void save(const String& filename) const;
		/** Loads constant definitions from a stream, compatible with GpuProgram::setManualNamedConstantsFile. 
		@see GpuProgram::setManualNamedConstantsFile
		*/
		void load(DataStreamPtr& stream);

    protected:
        /** Indicates whether all array entries will be generated and added to the definitions map
        @remarks
            Normally, the number of array entries added to the definitions map is capped at 16
            to save memory. Setting this value to <code>true</code> allows all of the entries
            to be generated and added to the map.
        */
        static bool msGenerateAllConstantDefinitionArrayEntries;
	};

	/// Simple class for loading / saving GpuNamedConstants
	class _OgreExport GpuNamedConstantsSerializer : public Serializer
	{
	public:
		GpuNamedConstantsSerializer();
		virtual ~GpuNamedConstantsSerializer();
		void exportNamedConstants(const GpuNamedConstants* pConsts, const String& filename,
			Endian endianMode = ENDIAN_NATIVE);
		void importNamedConstants(DataStreamPtr& stream, GpuNamedConstants* pDest);
	};

	/** Structure recording the use of a physical buffer by a logical parameter
		index. Only used for low-level programs.
	*/
	struct _OgreExport GpuLogicalIndexUse
	{
		/// Physical buffer index
		size_t physicalIndex;
		/// Current physical size allocation
		size_t currentSize;

		GpuLogicalIndexUse(size_t bufIdx, size_t curSz) 
			: physicalIndex(bufIdx), currentSize(curSz) {}
	};
	typedef std::map<size_t, GpuLogicalIndexUse> GpuLogicalIndexUseMap;
	/// Container struct to allow params to safely & update shared list of logical buffer assignments
	struct _OgreExport GpuLogicalBufferStruct
	{
		OGRE_MUTEX(mutex)
		/// Map from logical index to physical buffer location
		GpuLogicalIndexUseMap map;
		/// Shortcut to know the buffer size needs
		size_t bufferSize;
		GpuLogicalBufferStruct() : bufferSize(0) {}
	};

    /** Collects together the program parameters used for a GpuProgram.
    @remarks
        Gpu program state includes constant parameters used by the program, and
        bindings to render system state which is propagated into the constants 
        by the engine automatically if requested.
    @par
        GpuProgramParameters objects should be created through the GpuProgram and
        may be shared between multiple Pass instances. For this reason they
        are managed using a shared pointer, which will ensure they are automatically
        deleted when no Pass is using them anymore. 
	@par
		High-level programs use named parameters (uniforms), low-level programs 
		use indexed constants. This class supports both, but you can tell whether 
		named constants are supported by calling hasNamedParameters(). There are
		references in the documentation below to 'logical' and 'physical' indexes;
		logical indexes are the indexes used by low-level programs and represent 
		indexes into an array of float4's, some of which may be settable, some of
		which may be predefined constants in the program. We only store those
		constants which have actually been set, therefore our buffer could have 
		gaps if we used the logical indexes in our own buffers. So instead we map
		these logical indexes to physical indexes in our buffer. When using 
		high-level programs, logical indexes don't necessarily exist, although they
		might if the high-level program has a direct, exposed mapping from parameter
		names to logical indexes. In addition, high-level languages may or may not pack
		arrays of elements that are smaller than float4 (e.g. float2/vec2) contiguously.
		This kind of information is held in the ConstantDefinition structure which 
		is only populated for high-level programs. You don't have to worry about
		any of this unless you intend to read parameters back from this structure
		rather than just setting them.
    */
	class _OgreExport GpuProgramParameters : public GpuParamsAlloc
    {
    public:
        /** Defines the types of automatically updated values that may be bound to GpuProgram
        parameters, or used to modify parameters on a per-object basis.
        */
        enum AutoConstantType
        {
            /// The current world matrix
            ACT_WORLD_MATRIX,
            /// The current world matrix, inverted
            ACT_INVERSE_WORLD_MATRIX,
 			/** Provides transpose of world matrix.
 			Equivalent to RenderMonkey's "WorldTranspose".
 			*/
 			ACT_TRANSPOSE_WORLD_MATRIX,
            /// The current world matrix, inverted & transposed
            ACT_INVERSE_TRANSPOSE_WORLD_MATRIX,


            /// The current array of world matrices, as a 3x4 matrix, used for blending
            ACT_WORLD_MATRIX_ARRAY_3x4,
            /// The current array of world matrices, used for blending
            ACT_WORLD_MATRIX_ARRAY,

            /// The current view matrix
            ACT_VIEW_MATRIX,
			/// The current view matrix, inverted
			ACT_INVERSE_VIEW_MATRIX,
			/** Provides transpose of view matrix.
			Equivalent to RenderMonkey's "ViewTranspose".
			*/
			ACT_TRANSPOSE_VIEW_MATRIX,
			/** Provides inverse transpose of view matrix.
			Equivalent to RenderMonkey's "ViewInverseTranspose".
			*/
			ACT_INVERSE_TRANSPOSE_VIEW_MATRIX,


            /// The current projection matrix
            ACT_PROJECTION_MATRIX,
			/** Provides inverse of projection matrix.
			Equivalent to RenderMonkey's "ProjectionInverse".
			*/
			ACT_INVERSE_PROJECTION_MATRIX,
			/** Provides transpose of projection matrix.
			Equivalent to RenderMonkey's "ProjectionTranspose".
			*/
			ACT_TRANSPOSE_PROJECTION_MATRIX,
			/** Provides inverse transpose of projection matrix.
			Equivalent to RenderMonkey's "ProjectionInverseTranspose".
			*/
			ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX,


            /// The current view & projection matrices concatenated
            ACT_VIEWPROJ_MATRIX,
			/** Provides inverse of concatenated view and projection matrices.
			Equivalent to RenderMonkey's "ViewProjectionInverse".
			*/
			ACT_INVERSE_VIEWPROJ_MATRIX,
			/** Provides transpose of concatenated view and projection matrices.
			Equivalent to RenderMonkey's "ViewProjectionTranspose".
			*/
			ACT_TRANSPOSE_VIEWPROJ_MATRIX,
			/** Provides inverse transpose of concatenated view and projection matrices.
			Equivalent to RenderMonkey's "ViewProjectionInverseTranspose".
			*/
			ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX,


            /// The current world & view matrices concatenated
            ACT_WORLDVIEW_MATRIX,
            /// The current world & view matrices concatenated, then inverted
            ACT_INVERSE_WORLDVIEW_MATRIX,
 			/** Provides transpose of concatenated world and view matrices.
 				Equivalent to RenderMonkey's "WorldViewTranspose".
 			*/
 			ACT_TRANSPOSE_WORLDVIEW_MATRIX,
            /// The current world & view matrices concatenated, then inverted & transposed
            ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX,
			/// view matrices.


            /// The current world, view & projection matrices concatenated
            ACT_WORLDVIEWPROJ_MATRIX,
			/** Provides inverse of concatenated world, view and projection matrices.
			Equivalent to RenderMonkey's "WorldViewProjectionInverse".
			*/
			ACT_INVERSE_WORLDVIEWPROJ_MATRIX,
			/** Provides transpose of concatenated world, view and projection matrices.
			Equivalent to RenderMonkey's "WorldViewProjectionTranspose".
			*/
			ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX,
			/** Provides inverse transpose of concatenated world, view and projection
			matrices. Equivalent to RenderMonkey's "WorldViewProjectionInverseTranspose".
 			*/
			ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX,


            /// render target related values
            /** -1 if requires texture flipping, +1 otherwise. It's useful when you bypassed
            projection matrix transform, still able use this value to adjust transformed y position.
            */
            ACT_RENDER_TARGET_FLIPPING,


            /// Fog colour
            ACT_FOG_COLOUR,
            /// Fog params: density, linear start, linear end, 1/(end-start)
            ACT_FOG_PARAMS,


            /// Surface ambient colour, as set in Pass::setAmbient
            ACT_SURFACE_AMBIENT_COLOUR,
            /// Surface diffuse colour, as set in Pass::setDiffuse
            ACT_SURFACE_DIFFUSE_COLOUR,
            /// Surface specular colour, as set in Pass::setSpecular
            ACT_SURFACE_SPECULAR_COLOUR,
            /// Surface emissive colour, as set in Pass::setSelfIllumination
            ACT_SURFACE_EMISSIVE_COLOUR,
            /// Surface shininess, as set in Pass::setShininess
            ACT_SURFACE_SHININESS,


			/// The number of active light sources (better than gl_MaxLights)
			ACT_LIGHT_COUNT,


			/// The ambient light colour set in the scene
			ACT_AMBIENT_LIGHT_COLOUR, 

            /// Light diffuse colour (index determined by setAutoConstant call)
            ACT_LIGHT_DIFFUSE_COLOUR,
            /// Light specular colour (index determined by setAutoConstant call)
            ACT_LIGHT_SPECULAR_COLOUR,
            /// Light attenuation parameters, Vector4(range, constant, linear, quadric)
            ACT_LIGHT_ATTENUATION,
			/** Spotlight parameters, Vector4(innerFactor, outerFactor, falloff, isSpot)
				innerFactor and outerFactor are cos(angle/2)
				The isSpot parameter is 0.0f for non-spotlights, 1.0f for spotlights.
				Also for non-spotlights the inner and outer factors are 1 and nearly 1 respectively
			*/ 
			ACT_SPOTLIGHT_PARAMS,
            /// A light position in world space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION,
            /// A light position in object space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_OBJECT_SPACE,
			/// A light position in view space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_VIEW_SPACE,
            /// A light direction in world space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION,
            /// A light direction in object space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION_OBJECT_SPACE,
			/// A light direction in view space (index determined by setAutoConstant call)
			ACT_LIGHT_DIRECTION_VIEW_SPACE,
			/** The distance of the light from the center of the object
				a useful approximation as an alternative to per-vertex distance
				calculations.
			*/
			ACT_LIGHT_DISTANCE_OBJECT_SPACE,
			/** Light power level, a single scalar as set in Light::setPowerScale  (index determined by setAutoConstant call) */
			ACT_LIGHT_POWER_SCALE,
			/// Light diffuse colour pre-scaled by Light::setPowerScale (index determined by setAutoConstant call)
			ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED,
			/// Light specular colour pre-scaled by Light::setPowerScale (index determined by setAutoConstant call)
			ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED,
			/// Array of light diffuse colours (count set by extra param)
			ACT_LIGHT_DIFFUSE_COLOUR_ARRAY,
			/// Array of light specular colours (count set by extra param)
			ACT_LIGHT_SPECULAR_COLOUR_ARRAY,
			/// Array of light diffuse colours scaled by light power (count set by extra param)
			ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY,
			/// Array of light specular colours scaled by light power (count set by extra param)
			ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY,
			/// Array of light attenuation parameters, Vector4(range, constant, linear, quadric) (count set by extra param)
			ACT_LIGHT_ATTENUATION_ARRAY,
			/// Array of light positions in world space (count set by extra param)
			ACT_LIGHT_POSITION_ARRAY,
			/// Array of light positions in object space (count set by extra param)
			ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY,
			/// Array of light positions in view space (count set by extra param)
			ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY,
			/// Array of light directions in world space (count set by extra param)
			ACT_LIGHT_DIRECTION_ARRAY,
			/// Array of light directions in object space (count set by extra param)
			ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY,
			/// Array of light directions in view space (count set by extra param)
			ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY,
			/** Array of distances of the lights from the center of the object
			a useful approximation as an alternative to per-vertex distance
			calculations. (count set by extra param)
			*/
			ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY,
			/** Array of light power levels, a single scalar as set in Light::setPowerScale 
			 (count set by extra param)
			*/
			ACT_LIGHT_POWER_SCALE_ARRAY,
			/** Spotlight parameters array of Vector4(innerFactor, outerFactor, falloff, isSpot)
			innerFactor and outerFactor are cos(angle/2)
			The isSpot parameter is 0.0f for non-spotlights, 1.0f for spotlights.
			Also for non-spotlights the inner and outer factors are 1 and nearly 1 respectively.
			(count set by extra param)
			*/ 
			ACT_SPOTLIGHT_PARAMS_ARRAY,

            /** The derived ambient light colour, with 'r', 'g', 'b' components filled with
                product of surface ambient colour and ambient light colour, respectively,
                and 'a' component filled with surface ambient alpha component.
            */
            ACT_DERIVED_AMBIENT_LIGHT_COLOUR,
            /** The derived scene colour, with 'r', 'g' and 'b' components filled with sum
                of derived ambient light colour and surface emissive colour, respectively,
                and 'a' component filled with surface diffuse alpha component.
            */
            ACT_DERIVED_SCENE_COLOUR,

            /** The derived light diffuse colour (index determined by setAutoConstant call),
                with 'r', 'g' and 'b' components filled with product of surface diffuse colour,
				light power scale and light diffuse colour, respectively, and 'a' component filled with surface
                diffuse alpha component.
            */
            ACT_DERIVED_LIGHT_DIFFUSE_COLOUR,
            /** The derived light specular colour (index determined by setAutoConstant call),
                with 'r', 'g' and 'b' components filled with product of surface specular colour
                and light specular colour, respectively, and 'a' component filled with surface
                specular alpha component.
            */
            ACT_DERIVED_LIGHT_SPECULAR_COLOUR,

			/// Array of derived light diffuse colours (count set by extra param)
            ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY,
			/// Array of derived light specular colours (count set by extra param)
            ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY,
			/** The absolute light number of a local light index. Each pass may have
				a number of lights passed to it, and each of these lights will have
				an index in the overall light list, which will differ from the local
				light index due to factors like setStartLight and setIteratePerLight.
				This binding provides the global light index for a local index.
			*/
			ACT_LIGHT_NUMBER,
			/// Returns (int) 1 if the  given light casts shadows, 0 otherwise (index set in extra param)
			ACT_LIGHT_CASTS_SHADOWS,


			/** The distance a shadow volume should be extruded when using
			    finite extrusion programs.
			*/
			ACT_SHADOW_EXTRUSION_DISTANCE,
            /// The current camera's position in world space
            ACT_CAMERA_POSITION,
            /// The current camera's position in object space 
            ACT_CAMERA_POSITION_OBJECT_SPACE,
            /// The view/projection matrix of the assigned texture projection frustum
            ACT_TEXTURE_VIEWPROJ_MATRIX,
			/// Array of view/projection matrices of the first n texture projection frustums
			ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY,
			/** The view/projection matrix of the assigned texture projection frustum, 
				combined with the current world matrix
			*/
			ACT_TEXTURE_WORLDVIEWPROJ_MATRIX,
			/// Array of world/view/projection matrices of the first n texture projection frustums
			ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY,
			/// The view/projection matrix of a given spotlight
			ACT_SPOTLIGHT_VIEWPROJ_MATRIX,
			/** The view/projection matrix of a given spotlight projection frustum, 
			combined with the current world matrix
			*/
			ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX,
            /// A custom parameter which will come from the renderable, using 'data' as the identifier
            ACT_CUSTOM,
            /** provides current elapsed time
            */
            ACT_TIME,
			/** Single float value, which repeats itself based on given as
			parameter "cycle time". Equivalent to RenderMonkey's "Time0_X".
			*/
			ACT_TIME_0_X,
			/// Cosine of "Time0_X". Equivalent to RenderMonkey's "CosTime0_X".
			ACT_COSTIME_0_X,
			/// Sine of "Time0_X". Equivalent to RenderMonkey's "SinTime0_X".
			ACT_SINTIME_0_X,
			/// Tangent of "Time0_X". Equivalent to RenderMonkey's "TanTime0_X".
			ACT_TANTIME_0_X,
			/** Vector of "Time0_X", "SinTime0_X", "CosTime0_X", 
			"TanTime0_X". Equivalent to RenderMonkey's "Time0_X_Packed".
			*/
			ACT_TIME_0_X_PACKED,
			/** Single float value, which represents scaled time value [0..1],
			which repeats itself based on given as parameter "cycle time".
			Equivalent to RenderMonkey's "Time0_1".
			*/
			ACT_TIME_0_1,
			/// Cosine of "Time0_1". Equivalent to RenderMonkey's "CosTime0_1".
			ACT_COSTIME_0_1,
			/// Sine of "Time0_1". Equivalent to RenderMonkey's "SinTime0_1".
			ACT_SINTIME_0_1,
			/// Tangent of "Time0_1". Equivalent to RenderMonkey's "TanTime0_1".
			ACT_TANTIME_0_1,
			/** Vector of "Time0_1", "SinTime0_1", "CosTime0_1",
			"TanTime0_1". Equivalent to RenderMonkey's "Time0_1_Packed".
			*/
			ACT_TIME_0_1_PACKED,
			/**	Single float value, which represents scaled time value [0..2*Pi],
			which repeats itself based on given as parameter "cycle time".
			Equivalent to RenderMonkey's "Time0_2PI".
			*/
			ACT_TIME_0_2PI,
			/// Cosine of "Time0_2PI". Equivalent to RenderMonkey's "CosTime0_2PI".
			ACT_COSTIME_0_2PI,
			/// Sine of "Time0_2PI". Equivalent to RenderMonkey's "SinTime0_2PI".
			ACT_SINTIME_0_2PI,
			/// Tangent of "Time0_2PI". Equivalent to RenderMonkey's "TanTime0_2PI".
			ACT_TANTIME_0_2PI,
			/** Vector of "Time0_2PI", "SinTime0_2PI", "CosTime0_2PI",
			"TanTime0_2PI". Equivalent to RenderMonkey's "Time0_2PI_Packed".
			*/
			ACT_TIME_0_2PI_PACKED,
			/// provides the scaled frame time, returned as a floating point value.
            ACT_FRAME_TIME,
			/// provides the calculated frames per second, returned as a floating point value.
			ACT_FPS,
			/// viewport-related values
			/** Current viewport width (in pixels) as floating point value.
			Equivalent to RenderMonkey's "ViewportWidth".
			*/
			ACT_VIEWPORT_WIDTH,
			/** Current viewport height (in pixels) as floating point value.
			Equivalent to RenderMonkey's "ViewportHeight".
			*/
			ACT_VIEWPORT_HEIGHT,
			/** This variable represents 1.0/ViewportWidth. 
			Equivalent to RenderMonkey's "ViewportWidthInverse".
			*/
			ACT_INVERSE_VIEWPORT_WIDTH,
			/** This variable represents 1.0/ViewportHeight.
			Equivalent to RenderMonkey's "ViewportHeightInverse".
			*/
			ACT_INVERSE_VIEWPORT_HEIGHT,
            /** Packed of "ViewportWidth", "ViewportHeight", "ViewportWidthInverse",
            "ViewportHeightInverse".
            */
            ACT_VIEWPORT_SIZE,

			/// view parameters
			/** This variable provides the view direction vector (world space).
			Equivalent to RenderMonkey's "ViewDirection".
			*/
			ACT_VIEW_DIRECTION,
			/** This variable provides the view side vector (world space).
			Equivalent to RenderMonkey's "ViewSideVector".
			*/
			ACT_VIEW_SIDE_VECTOR,
			/** This variable provides the view up vector (world space).
			Equivalent to RenderMonkey's "ViewUpVector".
			*/
			ACT_VIEW_UP_VECTOR,
			/** This variable provides the field of view as a floating point value.
			Equivalent to RenderMonkey's "FOV".
			*/
			ACT_FOV,
			/**	This variable provides the near clip distance as a floating point value.
			Equivalent to RenderMonkey's "NearClipPlane".
			*/
			ACT_NEAR_CLIP_DISTANCE,
			/**	This variable provides the far clip distance as a floating point value.
			Equivalent to RenderMonkey's "FarClipPlane".
			*/
			ACT_FAR_CLIP_DISTANCE,

            /** provides the pass index number within the technique
                of the active materil.
            */
            ACT_PASS_NUMBER,

            /** provides the current iteration number of the pass. The iteration
                number is the number of times the current render operation has
                been drawn for the active pass.
            */
            ACT_PASS_ITERATION_NUMBER,


			/** Provides a parametric animation value [0..1], only available
				where the renderable specifically implements it.
			*/
			ACT_ANIMATION_PARAMETRIC,

			/** Provides the texel offsets required by this rendersystem to map
				texels to pixels. Packed as 
				float4(absoluteHorizontalOffset, absoluteVerticalOffset, 
					horizontalOffset / viewportWidth, verticalOffset / viewportHeight)
			*/
			ACT_TEXEL_OFFSETS,

			/** Provides information about the depth range of the scene as viewed
				from the current camera. 
				Passed as float4(minDepth, maxDepth, depthRange, 1 / depthRange)
			*/
			ACT_SCENE_DEPTH_RANGE,

			/** Provides information about the depth range of the scene as viewed
			from a given shadow camera. Requires an index parameter which maps
			to a light index relative to the current light list.
			Passed as float4(minDepth, maxDepth, depthRange, 1 / depthRange)
			*/
			ACT_SHADOW_SCENE_DEPTH_RANGE,

			/** Provides the fixed shadow colour as configured via SceneManager::setShadowColour;
				useful for integrated modulative shadows.
			*/
			ACT_SHADOW_COLOUR,
            /** Provides texture size of the texture unit (index determined by setAutoConstant
                call). Packed as float4(width, height, depth, 1)
            */
            ACT_TEXTURE_SIZE,
            /** Provides inverse texture size of the texture unit (index determined by setAutoConstant
                call). Packed as float4(1 / width, 1 / height, 1 / depth, 1)
            */
            ACT_INVERSE_TEXTURE_SIZE,
            /** Provides packed texture size of the texture unit (index determined by setAutoConstant
                call). Packed as float4(width, height, 1 / width, 1 / height)
            */
            ACT_PACKED_TEXTURE_SIZE,

			/** Provides the current transform matrix of the texture unit (index determined by setAutoConstant
				call), as seen by the fixed-function pipeline. 
			*/
			ACT_TEXTURE_MATRIX, 

			/** Provides the position of the LOD camera in world space, allowing you 
				to perform separate LOD calculations in shaders independent of the rendering
				camera. If there is no separate LOD camera then this is the real camera
				position. See Camera::setLodCamera.
			*/
			ACT_LOD_CAMERA_POSITION, 
			/** Provides the position of the LOD camera in object space, allowing you 
			to perform separate LOD calculations in shaders independent of the rendering
			camera. If there is no separate LOD camera then this is the real camera
			position. See Camera::setLodCamera.
			*/
			ACT_LOD_CAMERA_POSITION_OBJECT_SPACE, 
        };

        /** Defines the type of the extra data item used by the auto constant.

        */
        enum ACDataType {
            /// no data is required
            ACDT_NONE,
            /// the auto constant requires data of type int
            ACDT_INT,
            /// the auto constant requires data of type real
            ACDT_REAL
        };

        /** Defines the base element type of the auto constant
        */
        enum ElementType {
            ET_INT,
            ET_REAL
        };

        /** Structure defining an auto constant that's available for use in 
			a parameters object.
		*/
		struct AutoConstantDefinition
        {
            AutoConstantType acType;
            String name;
            size_t elementCount;
			/// The type of the constant in the program
            ElementType elementType;
			/// The type of any extra data
            ACDataType dataType;

			AutoConstantDefinition(AutoConstantType _acType, const String& _name, 
				size_t _elementCount, ElementType _elementType, 
				ACDataType _dataType)
				:acType(_acType), name(_name), elementCount(_elementCount), 
				elementType(_elementType), dataType(_dataType)
			{
				
			}
        };

        /** Structure recording the use of an automatic parameter. */
        class AutoConstantEntry
        {
        public:
            /// The type of parameter
            AutoConstantType paramType;
			/// The target (physical) constant index
            size_t physicalIndex;
			/** The number of elements per individual entry in this constant
				Used in case people used packed elements smaller than 4 (e.g. GLSL)
				and bind an auto which is 4-element packed to it */
			size_t elementCount;
            /// Additional information to go with the parameter
			union{
				size_t data;
				Real fData;
			};

            AutoConstantEntry(AutoConstantType theType, size_t theIndex, size_t theData, 
				size_t theElemCount = 4)
                : paramType(theType), physicalIndex(theIndex), elementCount(theElemCount), data(theData) {}

			AutoConstantEntry(AutoConstantType theType, size_t theIndex, Real theData, 
				size_t theElemCount = 4)
				: paramType(theType), physicalIndex(theIndex), elementCount(theElemCount), fData(theData) {}

        };
		// Auto parameter storage
		typedef std::vector<AutoConstantEntry> AutoConstantList;

		/** Definition of container that holds the current float constants.
		@note Not necessarily in direct index order to constant indexes, logical
			to physical index map is derived from GpuProgram
		*/
		typedef std::vector<float> FloatConstantList;
		/** Definition of container that holds the current float constants.
		@note Not necessarily in direct index order to constant indexes, logical
			to physical index map is derived from GpuProgram
		*/
		typedef std::vector<int> IntConstantList;

	protected:
        static AutoConstantDefinition AutoConstantDictionary[];
		/// Packed list of floating-point constants (physical indexing)
        FloatConstantList mFloatConstants;
        /// Packed list of integer constants (physical indexing)
        IntConstantList mIntConstants;
		/** Logical index to physical index map - for low-level programs
			or high-level programs which pass params this way. */
		GpuLogicalBufferStruct* mFloatLogicalToPhysical;
		/** Logical index to physical index map - for low-level programs
		or high-level programs which pass params this way. */
		GpuLogicalBufferStruct* mIntLogicalToPhysical;
		/// Mapping from parameter names to def - high-level programs are expected to populate this
		const GpuNamedConstants* mNamedConstants;
        /// List of automatically updated parameters
        AutoConstantList mAutoConstants;
        /// Do we need to transpose matrices?
        bool mTransposeMatrices;
		/// flag to indicate if names not found will be ignored
		bool mIgnoreMissingParams;
        /// physical index for active pass iteration parameter real constant entry;
        size_t mActivePassIterationIndex;

    public:
		GpuProgramParameters();
		~GpuProgramParameters() {}

        /// Copy constructor
        GpuProgramParameters(const GpuProgramParameters& oth);
        /// Operator = overload
        GpuProgramParameters& operator=(const GpuProgramParameters& oth);

		/** Internal method for providing a link to a name->definition map for parameters. */
		void _setNamedConstants(const GpuNamedConstants* constantmap);

		/** Internal method for providing a link to a logical index->physical index map for parameters. */
		void _setLogicalIndexes(GpuLogicalBufferStruct* floatIndexMap, 
			GpuLogicalBufferStruct* intIndexMap);


		/// Does this parameter set include named parameters?
		bool hasNamedParameters() const { return mNamedConstants != 0;}
		/** Does this parameter set include logically indexed parameters?
		@note Not mutually exclusive with hasNamedParameters since some high-level
			programs still use logical indexes to set the parameters on the 
			rendersystem.
		*/
		bool hasLogicalIndexedParameters() const { return mFloatLogicalToPhysical != 0;}

		/** Sets a 4-element floating-point parameter to the program.
		@param index The logical constant index at which to place the parameter 
			(each constant is a 4D float)
		@param vec The value to set
		*/
		void setConstant(size_t index, const Vector4& vec);
		/** Sets a single floating-point parameter to the program.
		@note This is actually equivalent to calling 
		setConstant(index Vector4(val, 0, 0, 0)) since all constants are 4D.
		@param index The logical constant index at which to place the parameter (each constant is
		a 4D float)
		@param val The value to set
		*/
		void setConstant(size_t index, Real val);
		/** Sets a 4-element floating-point parameter to the program via Vector3.
		@param index The logical constant index at which to place the parameter (each constant is
            a 4D float).
            Note that since you're passing a Vector3, the last element of the 4-element
            value will be set to 1 (a homogeneous vector)
		@param vec The value to set
		*/
		void setConstant(size_t index, const Vector3& vec);
		/** Sets a Matrix4 parameter to the program.
		@param index The logical constant index at which to place the parameter (each constant is
            a 4D float).
            NB since a Matrix4 is 16 floats long, this parameter will take up 4 indexes.
		@param m The value to set
		*/
		void setConstant(size_t index, const Matrix4& m);
        /** Sets a list of Matrix4 parameters to the program.
        @param index The logical constant index at which to start placing the parameter (each constant is
        a 4D float).
        NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
        @param m Pointer to an array of matrices to set
        @param numEntries Number of Matrix4 entries
        */
        void setConstant(size_t index, const Matrix4* m, size_t numEntries);
		/** Sets a multiple value constant floating-point parameter to the program.
		@param index The logical constant index at which to start placing parameters (each constant is
            a 4D float)
		@param val Pointer to the values to write, must contain 4*count floats
		@param count The number of groups of 4 floats to write
		*/
		void setConstant(size_t index, const float *val, size_t count);
		/** Sets a multiple value constant floating-point parameter to the program.
		@param index The logical constant index at which to start placing parameters (each constant is
            a 4D float)
		@param val Pointer to the values to write, must contain 4*count floats
		@param count The number of groups of 4 floats to write
		*/
		void setConstant(size_t index, const double *val, size_t count);
		/** Sets a ColourValue parameter to the program.
		@param index The logical constant index at which to place the parameter (each constant is
            a 4D float)
		@param colour The value to set
		*/
        void setConstant(size_t index, const ColourValue& colour);
		
		/** Sets a multiple value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
		@param index The logical constant index at which to place the parameter (each constant is
            a 4D integer)
		@param val Pointer to the values to write, must contain 4*count ints
		@param count The number of groups of 4 ints to write
		*/
		void setConstant(size_t index, const int *val, size_t count);

		/** Write a series of floating point values into the underlying float 
			constant buffer at the given physical index.
		@param physicalIndex The buffer position to start writing
		@param val Pointer to a list of values to write
		@param count The number of floats to write
		*/
		void _writeRawConstants(size_t physicalIndex, const float* val, size_t count);
		/** Write a series of floating point values into the underlying float 
		constant buffer at the given physical index.
		@param physicalIndex The buffer position to start writing
		@param val Pointer to a list of values to write
		@param count The number of floats to write
		*/
		void _writeRawConstants(size_t physicalIndex, const double* val, size_t count);
		/** Write a series of integer values into the underlying integer
			constant buffer at the given physical index.
		@param physicalIndex The buffer position to start writing
		@param val Pointer to a list of values to write
		@param count The number of ints to write
		*/
		void _writeRawConstants(size_t physicalIndex, const int* val, size_t count);
		/** Read a series of floating point values from the underlying float 
			constant buffer at the given physical index.
		@param physicalIndex The buffer position to start reading
		@param count The number of floats to read
		@param dest Pointer to a buffer to receive the values
		*/
		void _readRawConstants(size_t physicalIndex, size_t count, float* dest);
		/** Read a series of integer values from the underlying integer 
		constant buffer at the given physical index.
		@param physicalIndex The buffer position to start reading
		@param count The number of ints to read
		@param dest Pointer to a buffer to receive the values
		*/
		void _readRawConstants(size_t physicalIndex, size_t count, int* dest);

		/** Write a 4-element floating-point parameter to the program directly to 
			the underlying constants buffer.
		@note You can use these methods if you have already derived the physical
			constant buffer location, for a slight speed improvement over using
			the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param vec The value to set
		@param count The number of floats to write; if for example
			the uniform constant 'slot' is smaller than a Vector4
		*/
		void _writeRawConstant(size_t physicalIndex, const Vector4& vec, 
			size_t count = 4);
		/** Write a single floating-point parameter to the program.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param val The value to set
		*/
		void _writeRawConstant(size_t physicalIndex, Real val);
		/** Write a single integer parameter to the program.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param val The value to set
		*/
		void _writeRawConstant(size_t physicalIndex, int val);
		/** Write a 3-element floating-point parameter to the program via Vector3.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param vec The value to set
		*/
		void _writeRawConstant(size_t physicalIndex, const Vector3& vec);
		/** Write a Matrix4 parameter to the program.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param m The value to set
		*/
		void _writeRawConstant(size_t physicalIndex, const Matrix4& m);
        /** Write a list of Matrix4 parameters to the program.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
        @param numEntries Number of Matrix4 entries
        */
        void _writeRawConstant(size_t physicalIndex, const Matrix4* m, size_t numEntries);
		/** Write a ColourValue parameter to the program.
		@note You can use these methods if you have already derived the physical
		constant buffer location, for a slight speed improvement over using
		the named / logical index versions.
		@param physicalIndex The physical buffer index at which to place the parameter 
		@param colour The value to set
		@param count The number of floats to write; if for example
			the uniform constant 'slot' is smaller than a Vector4
		*/
        void _writeRawConstant(size_t physicalIndex, const ColourValue& colour, 
			size_t count = 4);
		

        /** Gets an iterator over the named GpuConstantDefinition instances as defined
			by the program for which these parameters exist.
		@note
			Only available if this parameters object has named parameters.
		*/
        GpuConstantDefinitionIterator getConstantDefinitionIterator(void) const;

		/** Get a specific GpuConstantDefinition for a named parameter.
		@note
			Only available if this parameters object has named parameters.
		*/
		const GpuConstantDefinition& getConstantDefinition(const String& name) const;

		/** Get the full list of GpuConstantDefinition instances.
		@note
		Only available if this parameters object has named parameters.
		*/
		const GpuNamedConstants& getConstantDefinitions() const;

		/** Get the current list of mappings from low-level logical param indexes
			to physical buffer locations in the float buffer.
		@note
			Only applicable to low-level programs.
		*/
		const GpuLogicalBufferStruct* getFloatLogicalBufferStruct() const { return mFloatLogicalToPhysical; }

		/** Retrieves the logical index relating to a physical index in the float
			buffer, for programs which support that (low-level programs and 
			high-level programs which use logical parameter indexes).
			@returns std::numeric_limits<size_t>::max() if not found
		*/
		size_t getFloatLogicalIndexForPhysicalIndex(size_t physicalIndex);
		/** Retrieves the logical index relating to a physical index in the int
		buffer, for programs which support that (low-level programs and 
		high-level programs which use logical parameter indexes).
		@returns std::numeric_limits<size_t>::max() if not found
		*/
		size_t getIntLogicalIndexForPhysicalIndex(size_t physicalIndex);

		/** Get the current list of mappings from low-level logical param indexes
			to physical buffer locations in the integer buffer.
		@note
			Only applicable to low-level programs.
		*/
		const GpuLogicalBufferStruct* getIntLogicalBufferStruct() const { return mIntLogicalToPhysical; }
		/// Get a reference to the list of float constants
		const FloatConstantList& getFloatConstantList() const { return mFloatConstants; }
		/// Get a pointer to the 'nth' item in the float buffer
		float* getFloatPointer(size_t pos) { return &mFloatConstants[pos]; }
		/// Get a pointer to the 'nth' item in the float buffer
		const float* getFloatPointer(size_t pos) const { return &mFloatConstants[pos]; }
		/// Get a reference to the list of int constants
		const IntConstantList& getIntConstantList() const { return mIntConstants; }
		/// Get a pointer to the 'nth' item in the int buffer
		int* getIntPointer(size_t pos) { return &mIntConstants[pos]; }
		/// Get a pointer to the 'nth' item in the int buffer
		const int* getIntPointer(size_t pos) const { return &mIntConstants[pos]; }
		/// Get a reference to the list of auto constant bindings
		const AutoConstantList& getAutoConstantList() const { return mAutoConstants; }

        /** Sets up a constant which will automatically be updated by the system.
        @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you 
            to set up a certain number of predefined parameter mappings that are kept up to 
            date for you.
        @param index The location in the constant list to place this updated constant every time
            it is changed. Note that because of the nature of the types, we know how big the 
            parameter details will be so you don't need to set that like you do for manual constants.
        @param acType The type of automatic constant to set
        @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        void setAutoConstant(size_t index, AutoConstantType acType, size_t extraInfo = 0);
		void setAutoConstantReal(size_t index, AutoConstantType acType, Real rData);

		/** As setAutoConstant, but sets up the auto constant directly against a
			physical buffer index.
		*/
		void _setRawAutoConstant(size_t physicalIndex, AutoConstantType acType, size_t extraInfo, 
			size_t elementSize = 4);
		/** As setAutoConstantReal, but sets up the auto constant directly against a
		physical buffer index.
		*/
		void _setRawAutoConstantReal(size_t physicalIndex, AutoConstantType acType, Real rData, 
			size_t elementSize = 4);


		/** Unbind an auto constant so that the constant is manually controlled again. */
		void clearAutoConstant(size_t index);

        /** Sets a named parameter up to track a derivation of the current time.
        @param index The index of the parameter
        @param factor The amount by which to scale the time value
        */  
        void setConstantFromTime(size_t index, Real factor);

        /** Clears all the existing automatic constants. */
        void clearAutoConstants(void);
        typedef ConstVectorIterator<AutoConstantList> AutoConstantIterator;
        /** Gets an iterator over the automatic constant bindings currently in place. */
        AutoConstantIterator getAutoConstantIterator(void) const;
        /// Gets the number of int constants that have been set
        size_t getAutoConstantCount(void) const { return mAutoConstants.size(); }
		/** Gets a specific Auto Constant entry if index is in valid range
			otherwise returns a NULL
		@param index which entry is to be retrieved
		*/
		AutoConstantEntry* getAutoConstantEntry(const size_t index);
        /** Returns true if this instance has any automatic constants. */
        bool hasAutoConstants(void) const { return !(mAutoConstants.empty()); }
		/** Finds an auto constant that's affecting a given logical parameter 
			index for floating-point values.
		@note Only applicable for low-level programs.
		*/
		const AutoConstantEntry* findFloatAutoConstantEntry(size_t logicalIndex);
		/** Finds an auto constant that's affecting a given logical parameter 
		index for integer values.
		@note Only applicable for low-level programs.
		*/
		const AutoConstantEntry* findIntAutoConstantEntry(size_t logicalIndex);
		/** Finds an auto constant that's affecting a given named parameter index.
		@note Only applicable to high-level programs.
		*/
		const AutoConstantEntry* findAutoConstantEntry(const String& paramName);
		/** Finds an auto constant that's affecting a given physical position in 
			the floating-point buffer
		*/
		const AutoConstantEntry* _findRawAutoConstantEntryFloat(size_t physicalIndex);
		/** Finds an auto constant that's affecting a given physical position in 
		the integer buffer
		*/
		const AutoConstantEntry* _findRawAutoConstantEntryInt(size_t physicalIndex);

        /** Updates the automatic parameters (except lights) based on the details provided. */
        void _updateAutoParamsNoLights(const AutoParamDataSource* source);
        /** Updates the automatic parameters for lights based on the details provided. */
        void _updateAutoParamsLightsOnly(const AutoParamDataSource* source);

		/** Tells the program whether to ignore missing parameters or not.
		*/
		void setIgnoreMissingParams(bool state) { mIgnoreMissingParams = state; }

		/** Sets a single value constant floating-point parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
		@param name The name of the parameter
		@param val The value to set
		*/
		void setNamedConstant(const String& name, Real val);
		/** Sets a single value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val The value to set
		*/
		void setNamedConstant(const String& name, int val);
		/** Sets a Vector4 parameter to the program.
        @param name The name of the parameter
		@param vec The value to set
		*/
		void setNamedConstant(const String& name, const Vector4& vec);
		/** Sets a Vector3 parameter to the program.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
		@param index The index at which to place the parameter
			NB this index refers to the number of floats, so a Vector3 is 3. Note that many 
            rendersystems & programs assume that every floating point parameter is passed in
            as a vector of 4 items, so you are strongly advised to check with 
            RenderSystemCapabilities before using this version - if in doubt use Vector4
            or ColourValue instead (both are 4D).
		@param vec The value to set
		*/
		void setNamedConstant(const String& name, const Vector3& vec);
		/** Sets a Matrix4 parameter to the program.
        @param name The name of the parameter
		@param m The value to set
		*/
		void setNamedConstant(const String& name, const Matrix4& m);
        /** Sets a list of Matrix4 parameters to the program.
        @param name The name of the parameter; this must be the first index of an array,
            for examples 'matrices[0]'
        NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
        @param m Pointer to an array of matrices to set
        @param numEntries Number of Matrix4 entries
        */
        void setNamedConstant(const String& name, const Matrix4* m, size_t numEntries);
		/** Sets a multiple value constant floating-point parameter to the program.
        @par
            Some systems only allow constants to be set on certain boundaries, 
			e.g. in sets of 4 values for example. The 'multiple' parameter allows
			you to control that although you should only change it if you know
			your chosen language supports that (at the time of writing, only
			GLSL allows constants which are not a multiple of 4).
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val Pointer to the values to write
		@param count The number of 'multiples' of floats to write
		@param multiple The number of raw entries in each element to write, 
			the default is 4 so count = 1 would write 4 floats.
		*/
		void setNamedConstant(const String& name, const float *val, size_t count, 
			size_t multiple = 4);
		/** Sets a multiple value constant floating-point parameter to the program.
        @par
            Some systems only allow constants to be set on certain boundaries, 
			e.g. in sets of 4 values for example. The 'multiple' parameter allows
			you to control that although you should only change it if you know
			your chosen language supports that (at the time of writing, only
			GLSL allows constants which are not a multiple of 4).
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val Pointer to the values to write
		@param count The number of 'multiples' of floats to write
		@param multiple The number of raw entries in each element to write, 
			the default is 4 so count = 1 would write 4 floats.
		*/
		void setNamedConstant(const String& name, const double *val, size_t count, 
			size_t multiple = 4);
		/** Sets a ColourValue parameter to the program.
        @param name The name of the parameter
		@param colour The value to set
		*/
        void setNamedConstant(const String& name, const ColourValue& colour);
		
		/** Sets a multiple value constant floating-point parameter to the program.
        @par
            Some systems only allow constants to be set on certain boundaries, 
			e.g. in sets of 4 values for example. The 'multiple' parameter allows
			you to control that although you should only change it if you know
			your chosen language supports that (at the time of writing, only
			GLSL allows constants which are not a multiple of 4).
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val Pointer to the values to write
		@param count The number of 'multiples' of floats to write
		@param multiple The number of raw entries in each element to write, 
			the default is 4 so count = 1 would write 4 floats.
		*/
		void setNamedConstant(const String& name, const int *val, size_t count, 
			size_t multiple = 4);

        /** Sets up a constant which will automatically be updated by the system.
        @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you 
            to set up a certain number of predefined parameter mappings that are kept up to 
            date for you.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
        @param acType The type of automatic constant to set
        @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        void setNamedAutoConstant(const String& name, AutoConstantType acType, size_t extraInfo = 0);
		void setNamedAutoConstantReal(const String& name, AutoConstantType acType, Real rData);

        /** Sets a named parameter up to track a derivation of the current time.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
        @param factor The amount by which to scale the time value
        */  
        void setNamedConstantFromTime(const String& name, Real factor);

		/** Unbind an auto constant so that the constant is manually controlled again. */
		void clearNamedAutoConstant(const String& name);

        /** Find a constant definition for a named parameter.
		@remarks
			This method returns null if the named parameter did not exist, unlike
			getConstantDefinition which is more strict; unless you set the 
			last parameter to true.
		@param name The name to look up
		@param throwExceptionIfMissing If set to true, failure to find an entry
			will throw an exception.
		*/
        const GpuConstantDefinition* _findNamedConstantDefinition(
			const String& name, bool throwExceptionIfMissing = false) const;
		/** Gets the physical buffer index associated with a logical float constant index. 
		@note Only applicable to low-level programs.
		@param logicalIndex The logical parameter index
		@param requestedSize The requested size - pass 0 to ignore missing entries
			and return std::numeric_limits<size_t>::max() 
		*/
		size_t _getFloatConstantPhysicalIndex(size_t logicalIndex, size_t requestedSize);
		/** Gets the physical buffer index associated with a logical int constant index. 
		@note Only applicable to low-level programs.
		@param logicalIndex The logical parameter index
		@param requestedSize The requested size - pass 0 to ignore missing entries
			and return std::numeric_limits<size_t>::max() 
		*/
		size_t _getIntConstantPhysicalIndex(size_t logicalIndex, size_t requestedSize);


        /** Sets whether or not we need to transpose the matrices passed in from the rest of OGRE.
        @remarks
            D3D uses transposed matrices compared to GL and OGRE; this is not important when you
            use programs which are written to process row-major matrices, such as those generated
            by Cg, but if you use a program written to D3D's matrix layout you will need to enable
            this flag.
        */
        void setTransposeMatrices(bool val) { mTransposeMatrices = val; } 
        /// Gets whether or not matrices are to be transposed when set
        bool getTransposeMatrices(void) const { return mTransposeMatrices; } 

		/** Copies the values of all constants (including auto constants) from another
			GpuProgramParameters object.
		*/
		void copyConstantsFrom(const GpuProgramParameters& source);

        /** gets the auto constant definition associated with name if found else returns NULL
        @param name The name of the auto constant
        */
        static const AutoConstantDefinition* getAutoConstantDefinition(const String& name);
        /** gets the auto constant definition using an index into the auto constant definition array.
            If the index is out of bounds then NULL is returned;
        @param idx The auto constant index
        */
        static const AutoConstantDefinition* getAutoConstantDefinition(const size_t idx);
        /** Returns the number of auto constant definitions
        */
        static size_t getNumAutoConstantDefinitions(void);


        /** increments the multipass number entry by 1 if it exists
        */
        void incPassIterationNumber(void);
		/** Does this parameters object have a pass iteration number constant? */
		bool hasPassIterationNumber() const 
		{ return mActivePassIterationIndex != (std::numeric_limits<size_t>::max)(); }
		/** Get the physical buffer index of the pass iteration number constant */
		size_t getPassIterationNumberIndex() const 
		{ return mActivePassIterationIndex; }


    };

    /// Shared pointer used to hold references to GpuProgramParameters instances
    typedef SharedPtr<GpuProgramParameters> GpuProgramParametersSharedPtr;

    // Forward declaration 
    class GpuProgramPtr;

	/** Defines a program which runs on the GPU such as a vertex or fragment program. 
	@remarks
		This class defines the low-level program in assembler code, the sort used to
		directly assemble into machine instructions for the GPU to execute. By nature,
		this means that the assembler source is rendersystem specific, which is why this
		is an abstract class - real instances are created through the RenderSystem. 
		If you wish to use higher level shading languages like HLSL and Cg, you need to 
		use the HighLevelGpuProgram class instead.
	*/
	class _OgreExport GpuProgram : public Resource
	{
	protected:
		/// Command object - see ParamCommand 
		class _OgreExport CmdType : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdSyntax : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdSkeletal : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdMorph : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdPose : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdVTF : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdManualNamedConstsFile : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		class _OgreExport CmdAdjacency : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};
		// Command object for setting / getting parameters
		static CmdType msTypeCmd;
		static CmdSyntax msSyntaxCmd;
		static CmdSkeletal msSkeletalCmd;
		static CmdMorph msMorphCmd;
		static CmdPose msPoseCmd;
		static CmdVTF msVTFCmd;
		static CmdManualNamedConstsFile msManNamedConstsFileCmd;
		static CmdAdjacency msAdjacencyCmd;
		/// The type of the program
		GpuProgramType mType;
		/// The name of the file to load source from (may be blank)
		String mFilename;
        /// The assembler source of the program (may be blank until file loaded)
        String mSource;
        /// Whether we need to load source from file or not
        bool mLoadFromFile;
        /// Syntax code e.g. arbvp1, vs_2_0 etc
        String mSyntaxCode;
        /// Does this (vertex) program include skeletal animation?
        bool mSkeletalAnimation;
		/// Does this (vertex) program include morph animation?
		bool mMorphAnimation;
		/// Does this (vertex) program include pose animation (count of number of poses supported)
		ushort mPoseAnimation;
		/// Does this (vertex) program require support for vertex texture fetch?
		bool mVertexTextureFetch;
		/// Does this (geometry) program require adjacency information?
		bool mNeedsAdjacencyInfo;
		/// The default parameters for use with this object
		GpuProgramParametersSharedPtr mDefaultParams;
		/// Does this program want light states passed through fixed pipeline
		bool mPassSurfaceAndLightStates;
		/// Did we encounter a compilation error?
		bool mCompileError;
		/** Record of logical to physical buffer maps. Mandatory for low-level
			programs or high-level programs which set their params the same way. */
		mutable GpuLogicalBufferStruct mFloatLogicalToPhysical;
		/** Record of logical to physical buffer maps. Mandatory for low-level
			programs or high-level programs which set their params the same way. */
		mutable GpuLogicalBufferStruct mIntLogicalToPhysical;
		/// Parameter name -> ConstantDefinition map, shared instance used by all parameter objects
		mutable GpuNamedConstants mConstantDefs;
		/// File from which to load named constants manually
		String mManualNamedConstantsFile;
		bool mLoadedManualNamedConstants;


		/** Internal method for setting up the basic parameter definitions for a subclass. 
		@remarks
		Because StringInterface holds a dictionary of parameters per class, subclasses need to
		call this to ask the base class to add it's parameters to their dictionary as well.
		Can't do this in the constructor because that runs in a non-virtual context.
		@par
		The subclass must have called it's own createParamDictionary before calling this method.
		*/
		void setupBaseParamDictionary(void);

        /** Internal method returns whether required capabilities for this program is supported.
        */
        bool isRequiredCapabilitiesSupported(void) const;

		/// @copydoc Resource::calculateSize
		size_t calculateSize(void) const { return 0; } // TODO 

		/// @copydoc Resource::loadImpl
		void loadImpl(void);
	public:

		GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual = false, ManualResourceLoader* loader = 0);

		virtual ~GpuProgram() {}

        /** Sets the filename of the source assembly for this program.
        @remarks
            Setting this will have no effect until you (re)load the program.
        */
        virtual void setSourceFile(const String& filename);

		/** Sets the source assembly for this program from an in-memory string.
        @remarks
            Setting this will have no effect until you (re)load the program.
        */
        virtual void setSource(const String& source);

        /** Gets the syntax code for this program e.g. arbvp1, fp20, vs_1_1 etc */
        virtual const String& getSyntaxCode(void) const { return mSyntaxCode; }

		/** Sets the syntax code for this program e.g. arbvp1, fp20, vs_1_1 etc */
		virtual void setSyntaxCode(const String& syntax);

		/** Gets the name of the file used as source for this program. */
		virtual const String& getSourceFile(void) const { return mFilename; }
        /** Gets the assembler source for this program. */
        virtual const String& getSource(void) const { return mSource; }
		/// Set the program type (only valid before load)
		virtual void setType(GpuProgramType t);
        /// Get the program type
        virtual GpuProgramType getType(void) const { return mType; }

        /** Returns the GpuProgram which should be bound to the pipeline.
        @remarks
            This method is simply to allow some subclasses of GpuProgram to delegate
            the program which is bound to the pipeline to a delegate, if required. */
        virtual GpuProgram* _getBindingDelegate(void) { return this; }

        /** Returns whether this program can be supported on the current renderer and hardware. */
        virtual bool isSupported(void) const;

        /** Creates a new parameters object compatible with this program definition. 
        @remarks
            It is recommended that you use this method of creating parameters objects
            rather than going direct to GpuProgramManager, because this method will
            populate any implementation-specific extras (like named parameters) where
            they are appropriate.
        */
        virtual GpuProgramParametersSharedPtr createParameters(void);

        /** Sets whether a vertex program includes the required instructions
        to perform skeletal animation. 
        @remarks
        If this is set to true, OGRE will not blend the geometry according to 
        skeletal animation, it will expect the vertex program to do it.
        */
        virtual void setSkeletalAnimationIncluded(bool included) 
        { mSkeletalAnimation = included; }

        /** Returns whether a vertex program includes the required instructions
            to perform skeletal animation. 
        @remarks
            If this returns true, OGRE will not blend the geometry according to 
            skeletal animation, it will expect the vertex program to do it.
        */
        virtual bool isSkeletalAnimationIncluded(void) const { return mSkeletalAnimation; }

        /** Sets whether a vertex program includes the required instructions
        to perform morph animation. 
        @remarks
        If this is set to true, OGRE will not blend the geometry according to 
        morph animation, it will expect the vertex program to do it.
        */
        virtual void setMorphAnimationIncluded(bool included) 
		{ mMorphAnimation = included; }

        /** Sets whether a vertex program includes the required instructions
        to perform pose animation. 
        @remarks
        If this is set to true, OGRE will not blend the geometry according to 
        pose animation, it will expect the vertex program to do it.
		@param poseCount The number of simultaneous poses the program can blend
        */
        virtual void setPoseAnimationIncluded(ushort poseCount) 
		{ mPoseAnimation = poseCount; }

		/** Returns whether a vertex program includes the required instructions
            to perform morph animation. 
        @remarks
            If this returns true, OGRE will not blend the geometry according to 
            morph animation, it will expect the vertex program to do it.
        */
        virtual bool isMorphAnimationIncluded(void) const { return mMorphAnimation; }

		/** Returns whether a vertex program includes the required instructions
            to perform pose animation. 
        @remarks
            If this returns true, OGRE will not blend the geometry according to 
            pose animation, it will expect the vertex program to do it.
        */
        virtual bool isPoseAnimationIncluded(void) const { return mPoseAnimation > 0; }
		/** Returns the number of simultaneous poses the vertex program can 
			blend, for use in pose animation.
        */
        virtual ushort getNumberOfPosesIncluded(void) const { return mPoseAnimation; }
		/** Sets whether this vertex program requires support for vertex 
			texture fetch from the hardware.
		*/
		virtual void setVertexTextureFetchRequired(bool r) { mVertexTextureFetch = r; }
		/** Returns whether this vertex program requires support for vertex 
			texture fetch from the hardware.
		*/
		virtual bool isVertexTextureFetchRequired(void) const { return mVertexTextureFetch; }

		/** Sets whether this geometry program requires adjacency information
			from the input primitives.
		*/
		virtual void setAdjacencyInfoRequired(bool r) { mNeedsAdjacencyInfo = r; }
		/** Returns whether this geometry program requires adjacency information 
			from the input primitives.
		*/
		virtual bool isAdjacencyInfoRequired(void) const { return mNeedsAdjacencyInfo; }
		
		/** Get a reference to the default parameters which are to be used for all
			uses of this program.
		@remarks
			A program can be set up with a list of default parameters, which can save time when 
			using a program many times in a material with roughly the same settings. By 
			retrieving the default parameters and populating it with the most used options, 
			any new parameter objects created from this program afterwards will automatically include
			the default parameters; thus users of the program need only change the parameters
			which are unique to their own usage of the program.
		*/
		virtual GpuProgramParametersSharedPtr getDefaultParameters(void);

        /** Returns true if default parameters have been set up.  
        */
        virtual bool hasDefaultParameters(void) const { return !mDefaultParams.isNull(); }

		/** Sets whether a vertex program requires light and material states to be passed
		to through fixed pipeline low level API rendering calls.
		@remarks
		If this is set to true, OGRE will pass all active light states to the fixed function
		pipeline.  This is useful for high level shaders like GLSL that can read the OpenGL
		light and material states.  This way the user does not have to use autoparameters to 
		pass light position, color etc.
		*/
		virtual void setSurfaceAndPassLightStates(bool state)
			{ mPassSurfaceAndLightStates = state; }

		/** Returns whether a vertex program wants light and material states to be passed
		through fixed pipeline low level API rendering calls
		*/
		virtual bool getPassSurfaceAndLightStates(void) const { return mPassSurfaceAndLightStates; }

        /** Returns a string that specifies the language of the gpu programs as specified
        in a material script. ie: asm, cg, hlsl, glsl
        */
        virtual const String& getLanguage(void) const;

		/** Did this program encounter a compile error when loading?
		*/
		virtual bool hasCompileError(void) const { return mCompileError; }

		/** Reset a compile error if it occurred, allowing the load to be retried
		*/
		virtual void resetCompileError(void) { mCompileError = false; }

		/** Allows you to manually provide a set of named parameter mappings
			to a program which would not be able to derive named parameters itself.
		@remarks
			You may wish to use this if you have assembler programs that were compiled
			from a high-level source, and want the convenience of still being able
			to use the named parameters from the original high-level source.
		@see setManualNamedConstantsFile
		*/
		virtual void setManualNamedConstants(const GpuNamedConstants& namedConstants);

		/// Get a read-only reference to the named constants registered for this program (manually or automatically)
		virtual const GpuNamedConstants& getNamedConstants() const { return mConstantDefs; }

		/** Specifies the name of a file from which to load named parameters mapping
			for a program which would not be able to derive named parameters itself.
		@remarks
			You may wish to use this if you have assembler programs that were compiled
			from a high-level source, and want the convenience of still being able
			to use the named parameters from the original high-level source. This
			method will make a low-level program search in the resource group of the
			program for the named file from which to load parameter names from. 
			The file must be in the format produced by GpuNamedConstants::save.
		*/
		virtual void setManualNamedConstantsFile(const String& paramDefFile);

		/** Gets the name of a file from which to load named parameters mapping
			for a program which would not be able to derive named parameters itself.
		*/
		virtual const String& getManualNamedConstantsFile() const { return mManualNamedConstantsFile; }
		/** Get the full list of named constants.
		@note
		Only available if this parameters object has named parameters, which means either
		a high-level program which loads them, or a low-level program which has them
		specified manually.
		*/
		virtual const GpuNamedConstants& getConstantDefinitions() const { return mConstantDefs; }


    protected:
        /// Virtual method which must be implemented by subclasses, load from mSource
        virtual void loadFromSource(void) = 0;

	};


	/** Specialisation of SharedPtr to allow SharedPtr to be assigned to GpuProgramPtr 
	@note Has to be a subclass since we need operator=.
	We could templatise this instead of repeating per Resource subclass, 
	except to do so requires a form VC6 does not support i.e.
	ResourceSubclassPtr<T> : public SharedPtr<T>
	*/
	class _OgreExport GpuProgramPtr : public SharedPtr<GpuProgram> 
	{
	public:
		GpuProgramPtr() : SharedPtr<GpuProgram>() {}
		explicit GpuProgramPtr(GpuProgram* rep) : SharedPtr<GpuProgram>(rep) {}
		GpuProgramPtr(const GpuProgramPtr& r) : SharedPtr<GpuProgram>(r) {} 
		GpuProgramPtr(const ResourcePtr& r) : SharedPtr<GpuProgram>()
		{
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
			    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
			    pRep = static_cast<GpuProgram*>(r.getPointer());
			    pUseCount = r.useCountPointer();
			    if (pUseCount)
			    {
				    ++(*pUseCount);
			    }
            }
		}

		/// Operator used to convert a ResourcePtr to a GpuProgramPtr
		GpuProgramPtr& operator=(const ResourcePtr& r)
		{
			if (pRep == static_cast<GpuProgram*>(r.getPointer()))
				return *this;
			release();
			// lock & copy other mutex pointer
            OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
            {
                OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
			    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
			    pRep = static_cast<GpuProgram*>(r.getPointer());
			    pUseCount = r.useCountPointer();
			    if (pUseCount)
			    {
				    ++(*pUseCount);
			    }
            }
			else
			{
				// RHS must be a null pointer
				assert(r.isNull() && "RHS must be null if it has no mutex!");
				setNull();
			}
			return *this;
		}
        /// Operator used to convert a HighLevelGpuProgramPtr to a GpuProgramPtr
        GpuProgramPtr& operator=(const HighLevelGpuProgramPtr& r);
	};
}

#endif
