/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreGpuProgram.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreAutoParamDataSource.h"
#include "OgreLight.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
    GpuProgram::CmdType GpuProgram::msTypeCmd;
    GpuProgram::CmdSyntax GpuProgram::msSyntaxCmd;
    GpuProgram::CmdSkeletal GpuProgram::msSkeletalCmd;
	GpuProgram::CmdMorph GpuProgram::msMorphCmd;
	GpuProgram::CmdPose GpuProgram::msPoseCmd;
	GpuProgram::CmdVTF GpuProgram::msVTFCmd;
	GpuProgram::CmdManualNamedConstsFile GpuProgram::msManNamedConstsFileCmd;
	GpuProgram::CmdAdjacency GpuProgram::msAdjacencyCmd;
	


    GpuProgramParameters::AutoConstantDefinition GpuProgramParameters::AutoConstantDictionary[] = {
        AutoConstantDefinition(ACT_WORLD_MATRIX,                  "world_matrix",                16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLD_MATRIX,          "inverse_world_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLD_MATRIX,             "transpose_world_matrix",            16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, "inverse_transpose_world_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_WORLD_MATRIX_ARRAY_3x4,        "world_matrix_array_3x4",      12, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_WORLD_MATRIX_ARRAY,            "world_matrix_array",          16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_VIEW_MATRIX,                   "view_matrix",                 16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_VIEW_MATRIX,           "inverse_view_matrix",         16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_VIEW_MATRIX,              "transpose_view_matrix",             16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_VIEW_MATRIX,       "inverse_transpose_view_matrix",     16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_PROJECTION_MATRIX,             "projection_matrix",           16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_PROJECTION_MATRIX,          "inverse_projection_matrix",         16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_PROJECTION_MATRIX,        "transpose_projection_matrix",       16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX, "inverse_transpose_projection_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_VIEWPROJ_MATRIX,               "viewproj_matrix",             16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_VIEWPROJ_MATRIX,       "inverse_viewproj_matrix",     16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_VIEWPROJ_MATRIX,          "transpose_viewproj_matrix",         16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX,   "inverse_transpose_viewproj_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_WORLDVIEW_MATRIX,              "worldview_matrix",            16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLDVIEW_MATRIX,      "inverse_worldview_matrix",    16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLDVIEW_MATRIX,         "transpose_worldview_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX, "inverse_transpose_worldview_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_WORLDVIEWPROJ_MATRIX,          "worldviewproj_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLDVIEWPROJ_MATRIX,       "inverse_worldviewproj_matrix",      16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX,     "transpose_worldviewproj_matrix",    16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX, "inverse_transpose_worldviewproj_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_RENDER_TARGET_FLIPPING,          "render_target_flipping",         1, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_FOG_COLOUR,                    "fog_colour",                   4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_FOG_PARAMS,                    "fog_params",                   4, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_SURFACE_AMBIENT_COLOUR,          "surface_ambient_colour",           4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_DIFFUSE_COLOUR,          "surface_diffuse_colour",           4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_SPECULAR_COLOUR,         "surface_specular_colour",          4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_EMISSIVE_COLOUR,         "surface_emissive_colour",          4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_SHININESS,               "surface_shininess",                1, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_LIGHT_COUNT,                   "light_count",                  1, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_AMBIENT_LIGHT_COLOUR,          "ambient_light_colour",         4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_LIGHT_DIFFUSE_COLOUR,          "light_diffuse_colour",         4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_SPECULAR_COLOUR,         "light_specular_colour",        4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_ATTENUATION,             "light_attenuation",            4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_SPOTLIGHT_PARAMS,              "spotlight_params",             4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_POSITION,                "light_position",               4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_POSITION_OBJECT_SPACE,   "light_position_object_space",  4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_POSITION_VIEW_SPACE,          "light_position_view_space",    4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_DIRECTION,               "light_direction",              4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_DIRECTION_OBJECT_SPACE,  "light_direction_object_space", 4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIRECTION_VIEW_SPACE,         "light_direction_view_space",   4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DISTANCE_OBJECT_SPACE,   "light_distance_object_space",  1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_POWER_SCALE,   		  "light_power",  1, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED, "light_diffuse_colour_power_scaled",         4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED, "light_specular_colour_power_scaled",        4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIFFUSE_COLOUR_ARRAY,          "light_diffuse_colour_array",         4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_SPECULAR_COLOUR_ARRAY,         "light_specular_colour_array",        4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY, "light_diffuse_colour_power_scaled_array",         4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY, "light_specular_colour_power_scaled_array",        4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_ATTENUATION_ARRAY,             "light_attenuation_array",            4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_POSITION_ARRAY,                "light_position_array",               4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY,   "light_position_object_space_array",  4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY,          "light_position_view_space_array",    4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIRECTION_ARRAY,               "light_direction_array",              4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY,  "light_direction_object_space_array", 4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY,         "light_direction_view_space_array",   4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY,   "light_distance_object_space_array",  1, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LIGHT_POWER_SCALE_ARRAY,   		  "light_power_array",  1, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_SPOTLIGHT_PARAMS_ARRAY,              "spotlight_params_array",             4, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_DERIVED_AMBIENT_LIGHT_COLOUR,    "derived_ambient_light_colour",     4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_DERIVED_SCENE_COLOUR,            "derived_scene_colour",             4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_DIFFUSE_COLOUR,    "derived_light_diffuse_colour",     4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_SPECULAR_COLOUR,   "derived_light_specular_colour",    4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY,  "derived_light_diffuse_colour_array",   4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY, "derived_light_specular_colour_array",  4, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_LIGHT_NUMBER,   					  "light_number",  1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_CASTS_SHADOWS, 			  "light_casts_shadows",  1, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_SHADOW_EXTRUSION_DISTANCE,     "shadow_extrusion_distance",    1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_CAMERA_POSITION,               "camera_position",              3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_CAMERA_POSITION_OBJECT_SPACE,  "camera_position_object_space", 3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TEXTURE_VIEWPROJ_MATRIX,       "texture_viewproj_matrix",     16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY, "texture_viewproj_matrix_array", 16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_TEXTURE_WORLDVIEWPROJ_MATRIX,  "texture_worldviewproj_matrix",16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY, "texture_worldviewproj_matrix_array",16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_SPOTLIGHT_VIEWPROJ_MATRIX,       "spotlight_viewproj_matrix",     16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX,  "spotlight_worldviewproj_matrix",16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_CUSTOM,                        "custom",                       4, ET_REAL, ACDT_INT),  // *** needs to be tested
        AutoConstantDefinition(ACT_TIME,                               "time",                               1, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_X,                      "time_0_x",                     4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_COSTIME_0_X,                   "costime_0_x",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_SINTIME_0_X,                   "sintime_0_x",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TANTIME_0_X,                   "tantime_0_x",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_X_PACKED,               "time_0_x_packed",              4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_1,                      "time_0_1",                     4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_COSTIME_0_1,                   "costime_0_1",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_SINTIME_0_1,                   "sintime_0_1",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TANTIME_0_1,                   "tantime_0_1",                  4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_1_PACKED,               "time_0_1_packed",              4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_2PI,                    "time_0_2pi",                   4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_COSTIME_0_2PI,                 "costime_0_2pi",                4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_SINTIME_0_2PI,                 "sintime_0_2pi",                4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TANTIME_0_2PI,                 "tantime_0_2pi",                4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_TIME_0_2PI_PACKED,             "time_0_2pi_packed",            4, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_FRAME_TIME,                    "frame_time",                   1, ET_REAL, ACDT_REAL),
        AutoConstantDefinition(ACT_FPS,                           "fps",                          1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEWPORT_WIDTH,                "viewport_width",               1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEWPORT_HEIGHT,               "viewport_height",              1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_VIEWPORT_WIDTH,        "inverse_viewport_width",       1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_VIEWPORT_HEIGHT,       "inverse_viewport_height",      1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEWPORT_SIZE,                 "viewport_size",                4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEW_DIRECTION,                "view_direction",               3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEW_SIDE_VECTOR,              "view_side_vector",             3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VIEW_UP_VECTOR,                "view_up_vector",               3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_FOV,                           "fov",                          1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_NEAR_CLIP_DISTANCE,            "near_clip_distance",           1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_FAR_CLIP_DISTANCE,             "far_clip_distance",            1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_PASS_NUMBER,                        "pass_number",                        1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_PASS_ITERATION_NUMBER,              "pass_iteration_number",              1, ET_REAL, ACDT_NONE),
		AutoConstantDefinition(ACT_ANIMATION_PARAMETRIC,               "animation_parametric",               4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_TEXEL_OFFSETS,               "texel_offsets",				  4, ET_REAL, ACDT_NONE),
		AutoConstantDefinition(ACT_SCENE_DEPTH_RANGE,           "scene_depth_range",			  4, ET_REAL, ACDT_NONE),
		AutoConstantDefinition(ACT_SHADOW_SCENE_DEPTH_RANGE,    "shadow_scene_depth_range",		  4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_SHADOW_COLOUR,				"shadow_colour",				  4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TEXTURE_SIZE,                "texture_size",                   4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_INVERSE_TEXTURE_SIZE,        "inverse_texture_size",           4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_PACKED_TEXTURE_SIZE,         "packed_texture_size",            4, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_TEXTURE_MATRIX,  "texture_matrix", 16, ET_REAL, ACDT_INT),
		AutoConstantDefinition(ACT_LOD_CAMERA_POSITION,               "lod_camera_position",              3, ET_REAL, ACDT_NONE),
		AutoConstantDefinition(ACT_LOD_CAMERA_POSITION_OBJECT_SPACE,  "lod_camera_position_object_space", 3, ET_REAL, ACDT_NONE),
    };

    bool GpuNamedConstants::msGenerateAllConstantDefinitionArrayEntries = false;

	//---------------------------------------------------------------------
	void GpuNamedConstants::generateConstantDefinitionArrayEntries(
		const String& paramName, const GpuConstantDefinition& baseDef)
	{
		// Copy definition for use with arrays
		GpuConstantDefinition arrayDef = baseDef;
		arrayDef.arraySize = 1;
		String arrayName;

		// Add parameters for array accessors
		// [0] will refer to the same location, [1+] will increment
		// only populate others individually up to 16 array slots so as not to get out of hand,
        // unless the system has been explicitly configured to allow all the parameters to be added

		// paramName[0] version will always exist 
		size_t maxArrayIndex = 1;
		if (baseDef.arraySize <= 16 || msGenerateAllConstantDefinitionArrayEntries)
			maxArrayIndex = baseDef.arraySize;

		for (size_t i = 0; i < maxArrayIndex; i++)
		{
			arrayName = paramName + "[" + StringConverter::toString(i) + "]";
			map.insert(GpuConstantDefinitionMap::value_type(arrayName, arrayDef));
			// increment location
			arrayDef.physicalIndex += arrayDef.elementSize;
		}
		// note no increment of buffer sizes since this is shared with main array def

	}

    //---------------------------------------------------------------------
    bool GpuNamedConstants::getGenerateAllConstantDefinitionArrayEntries()
    {
        return msGenerateAllConstantDefinitionArrayEntries;
    }

    //---------------------------------------------------------------------
    void GpuNamedConstants::setGenerateAllConstantDefinitionArrayEntries(bool generateAll)
    {
        msGenerateAllConstantDefinitionArrayEntries = generateAll;
    }    

    //-----------------------------------------------------------------------------
    GpuProgram::GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader) 
        :Resource(creator, name, handle, group, isManual, loader),
        mType(GPT_VERTEX_PROGRAM), mLoadFromFile(true), mSkeletalAnimation(false),
        mVertexTextureFetch(false), mPassSurfaceAndLightStates(false), mCompileError(false), 
		mLoadedManualNamedConstants(false), mNeedsAdjacencyInfo(false)
    {
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setType(GpuProgramType t)
    {
        mType = t;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSyntaxCode(const String& syntax)
    {
        mSyntaxCode = syntax;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSourceFile(const String& filename)
    {
        mFilename = filename;
        mSource.clear();
        mLoadFromFile = true;
		mCompileError = false;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setSource(const String& source)
    {
        mSource = source;
        mFilename.clear();
        mLoadFromFile = false;
		mCompileError = false;
    }

    //-----------------------------------------------------------------------------
    void GpuProgram::loadImpl(void)
    {
        if (mLoadFromFile)
        {
            // find & load source code
            DataStreamPtr stream = 
                ResourceGroupManager::getSingleton().openResource(
					mFilename, mGroup, true, this);
            mSource = stream->getAsString();
        }

        // Call polymorphic load
		try 
		{
			loadFromSource();
		}
		catch (const Exception&)
		{
			// will already have been logged
			LogManager::getSingleton().stream()
				<< "Gpu program " << mName << " encountered an error "
				<< "during loading and is thus not supported.";

			mCompileError = true;
		}

    }
    //-----------------------------------------------------------------------------
    bool GpuProgram::isRequiredCapabilitiesSupported(void) const
    {
		const RenderSystemCapabilities* caps = 
			Root::getSingleton().getRenderSystem()->getCapabilities();

        // If skeletal animation is being done, we need support for UBYTE4
        if (isSkeletalAnimationIncluded() && 
            !caps->hasCapability(RSC_VERTEX_FORMAT_UBYTE4))
        {
            return false;
        }

		// Vertex texture fetch required?
		if (isVertexTextureFetchRequired() && 
			!caps->hasCapability(RSC_VERTEX_TEXTURE_FETCH))
		{
			return false;
		}

        return true;
    }
    //-----------------------------------------------------------------------------
    bool GpuProgram::isSupported(void) const
    {
        if (mCompileError || !isRequiredCapabilitiesSupported())
            return false;

        return GpuProgramManager::getSingleton().isSyntaxSupported(mSyntaxCode);
    }
	//---------------------------------------------------------------------
	void GpuProgram::setManualNamedConstantsFile(const String& paramDefFile)
	{
		mManualNamedConstantsFile = paramDefFile;
		mLoadedManualNamedConstants = false;
	}
	//---------------------------------------------------------------------
	void GpuProgram::setManualNamedConstants(const GpuNamedConstants& namedConstants)
	{
		mConstantDefs = namedConstants;

		mFloatLogicalToPhysical.bufferSize = mConstantDefs.floatBufferSize;
		mIntLogicalToPhysical.bufferSize = mConstantDefs.intBufferSize;
		mFloatLogicalToPhysical.map.clear();
		mIntLogicalToPhysical.map.clear();
		// need to set up logical mappings too for some rendersystems
		for (GpuConstantDefinitionMap::const_iterator i = mConstantDefs.map.begin();
			i != mConstantDefs.map.end(); ++i)
		{
			const String& name = i->first;
			const GpuConstantDefinition& def = i->second;
			// only consider non-array entries
			if (name.find("[") == String::npos)
			{
				GpuLogicalIndexUseMap::value_type val(def.logicalIndex, 
					GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize));
				if (def.isFloat())
				{
					mFloatLogicalToPhysical.map.insert(val);
				}
				else
				{
					mIntLogicalToPhysical.map.insert(val);
				}
			}
		}


	}
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::createParameters(void)
    {
        // Default implementation simply returns standard parameters.
        GpuProgramParametersSharedPtr ret = 
            GpuProgramManager::getSingleton().createParameters();
		
		
		// optionally load manually supplied named constants
		if (!mManualNamedConstantsFile.empty() && !mLoadedManualNamedConstants)
		{
			try 
			{
				GpuNamedConstants namedConstants;
				DataStreamPtr stream = 
					ResourceGroupManager::getSingleton().openResource(
					mManualNamedConstantsFile, mGroup, true, this);
				namedConstants.load(stream);
				setManualNamedConstants(namedConstants);
			}
			catch(const Exception& e)
			{
				LogManager::getSingleton().stream() <<
					"Unable to load manual named constants for GpuProgram " << mName <<
					": " << e.getDescription();
			}
			mLoadedManualNamedConstants = true;
		}
		
		
		// set up named parameters, if any
		if (!mConstantDefs.map.empty())
		{
			ret->_setNamedConstants(&mConstantDefs);
		}
		// link shared logical / physical map for low-level use
		ret->_setLogicalIndexes(&mFloatLogicalToPhysical, &mIntLogicalToPhysical);

        // Copy in default parameters if present
        if (!mDefaultParams.isNull())
            ret->copyConstantsFrom(*(mDefaultParams.get()));
        
        return ret;
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::getDefaultParameters(void)
    {
        if (mDefaultParams.isNull())
        {
            mDefaultParams = createParameters();
        }
        return mDefaultParams;
    }
    //-----------------------------------------------------------------------------
    void GpuProgram::setupBaseParamDictionary(void)
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(
            ParameterDef("type", "'vertex_program', 'geometry_program' or 'fragment_program'",
                PT_STRING), &msTypeCmd);
        dict->addParameter(
            ParameterDef("syntax", "Syntax code, e.g. vs_1_1", PT_STRING), &msSyntaxCmd);
        dict->addParameter(
            ParameterDef("includes_skeletal_animation", 
            "Whether this vertex program includes skeletal animation", PT_BOOL), 
            &msSkeletalCmd);
		dict->addParameter(
			ParameterDef("includes_morph_animation", 
			"Whether this vertex program includes morph animation", PT_BOOL), 
			&msMorphCmd);
		dict->addParameter(
			ParameterDef("includes_pose_animation", 
			"The number of poses this vertex program supports for pose animation", PT_INT), 
			&msPoseCmd);
		dict->addParameter(
			ParameterDef("uses_vertex_texture_fetch", 
			"Whether this vertex program requires vertex texture fetch support.", PT_BOOL), 
			&msVTFCmd);
		dict->addParameter(
			ParameterDef("manual_named_constants", 
			"File containing named parameter mappings for low-level programs.", PT_BOOL), 
			&msManNamedConstsFileCmd);
		dict->addParameter(
			ParameterDef("uses_adjacency_information",
			"Whether this geometry program requires adjacency information from the input primitives.", PT_BOOL),
			&msAdjacencyCmd);
    }

    //-----------------------------------------------------------------------
    const String& GpuProgram::getLanguage(void) const
    {
        static const String language = "asm";

        return language;
    }
	//---------------------------------------------------------------------
	//  GpuNamedConstants methods
	//---------------------------------------------------------------------
	void GpuNamedConstants::save(const String& filename) const
	{
		GpuNamedConstantsSerializer ser;
		ser.exportNamedConstants(this, filename);
	}
	//---------------------------------------------------------------------
	void GpuNamedConstants::load(DataStreamPtr& stream)
	{
		GpuNamedConstantsSerializer ser;
		ser.importNamedConstants(stream, this);
	}
	//---------------------------------------------------------------------
	//  GpuNamedConstantsSerializer methods
	//---------------------------------------------------------------------
	GpuNamedConstantsSerializer::GpuNamedConstantsSerializer()
	{
		mVersion = "[v1.0]";
	}
	//---------------------------------------------------------------------
	GpuNamedConstantsSerializer::~GpuNamedConstantsSerializer()
	{

	}
	//---------------------------------------------------------------------
	void GpuNamedConstantsSerializer::exportNamedConstants(
		const GpuNamedConstants* pConsts, const String& filename, Endian endianMode)
	{
		// Decide on endian mode
		determineEndianness(endianMode);

		String msg;
		mpfFile = fopen(filename.c_str(), "wb");
		if (!mpfFile)
		{
			OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
				"Unable to open file " + filename + " for writing",
				"GpuNamedConstantsSerializer::exportSkeleton");
		}

		writeFileHeader();

		writeInts(((uint32*)&pConsts->floatBufferSize), 1);
		writeInts(((uint32*)&pConsts->intBufferSize), 1);

		// simple export of all the named constants, no chunks
		// name, physical index
		for (GpuConstantDefinitionMap::const_iterator i = pConsts->map.begin();
			i != pConsts->map.end(); ++i)
		{
			const String& name = i->first;
			const GpuConstantDefinition& def = i->second;

			writeString(name);
			writeInts(((uint32*)&def.physicalIndex), 1);
			writeInts(((uint32*)&def.logicalIndex), 1);
			uint32 constType = static_cast<uint32>(def.constType);
			writeInts(&constType, 1);
			writeInts(((uint32*)&def.elementSize), 1);
			writeInts(((uint32*)&def.arraySize), 1);		
		}

		fclose(mpfFile);

	}
	//---------------------------------------------------------------------
	void GpuNamedConstantsSerializer::importNamedConstants(
		DataStreamPtr& stream, GpuNamedConstants* pDest)
	{
		// Determine endianness (must be the first thing we do!)
		determineEndianness(stream);

		// Check header
		readFileHeader(stream);

		// simple file structure, no chunks
		pDest->map.clear();

		readInts(stream, ((uint32*)&pDest->floatBufferSize), 1);
		readInts(stream, ((uint32*)&pDest->intBufferSize), 1);

		while (!stream->eof())
		{
			GpuConstantDefinition def;
			String name = readString(stream);
			// Hmm, deal with trailing information
			if (name.empty())
				continue;
			readInts(stream, ((uint32*)&def.physicalIndex), 1);
			readInts(stream, ((uint32*)&def.logicalIndex), 1);
			uint constType;
			readInts(stream, &constType, 1);
			def.constType = static_cast<GpuConstantType>(constType);
			readInts(stream, ((uint32*)&def.elementSize), 1);
			readInts(stream, ((uint32*)&def.arraySize), 1);

			pDest->map[name] = def;

		}



	}

    //-----------------------------------------------------------------------------
    //      GpuProgramParameters Methods
    //-----------------------------------------------------------------------------
	GpuProgramParameters::GpuProgramParameters() :
		mFloatLogicalToPhysical(0)
		, mIntLogicalToPhysical(0)
		, mNamedConstants(0)
		, mTransposeMatrices(false)
		, mIgnoreMissingParams(false)
		, mActivePassIterationIndex(std::numeric_limits<size_t>::max())	
    {
    }
    //-----------------------------------------------------------------------------

    GpuProgramParameters::GpuProgramParameters(const GpuProgramParameters& oth)
    {
        *this = oth;
    }

    //-----------------------------------------------------------------------------
    GpuProgramParameters& GpuProgramParameters::operator=(const GpuProgramParameters& oth)
    {
        // let compiler perform shallow copies of structures 
        // AutoConstantEntry, RealConstantEntry, IntConstantEntry
        mFloatConstants = oth.mFloatConstants;
        mIntConstants  = oth.mIntConstants;
        mAutoConstants = oth.mAutoConstants;
		mFloatLogicalToPhysical = oth.mFloatLogicalToPhysical;
		mIntLogicalToPhysical = oth.mIntLogicalToPhysical;
		mNamedConstants = oth.mNamedConstants;

        mTransposeMatrices = oth.mTransposeMatrices;
        mIgnoreMissingParams  = oth.mIgnoreMissingParams;
		mActivePassIterationIndex = oth.mActivePassIterationIndex;

		return *this;
    }
	//---------------------------------------------------------------------
	void GpuProgramParameters::_setNamedConstants(
		const GpuNamedConstants* namedConstants)
	{
		mNamedConstants = namedConstants;

		// Determine any extension to local buffers

		// Size and reset buffer (fill with zero to make comparison later ok)
		if (namedConstants->floatBufferSize > mFloatConstants.size())
		{
			mFloatConstants.insert(mFloatConstants.end(), 
				namedConstants->floatBufferSize - mFloatConstants.size(), 0.0f);
		}
		if (namedConstants->intBufferSize > mIntConstants.size())
		{
			mIntConstants.insert(mIntConstants.end(), 
				namedConstants->intBufferSize - mIntConstants.size(), 0);
		}
	}
	//---------------------------------------------------------------------
	void GpuProgramParameters::_setLogicalIndexes(
		GpuLogicalBufferStruct* floatIndexMap, 
		GpuLogicalBufferStruct* intIndexMap)
	{
		mFloatLogicalToPhysical = floatIndexMap;
		mIntLogicalToPhysical = intIndexMap;

		// resize the internal buffers
		// Note that these will only contain something after the first parameter
		// set has set some parameters

		// Size and reset buffer (fill with zero to make comparison later ok)
		if (floatIndexMap->bufferSize > mFloatConstants.size())
		{
			mFloatConstants.insert(mFloatConstants.end(), 
				floatIndexMap->bufferSize - mFloatConstants.size(), 0.0f);
		}
		if (intIndexMap->bufferSize > mIntConstants.size())
		{
			mIntConstants.insert(mIntConstants.end(), 
				intIndexMap->bufferSize - mIntConstants.size(), 0);
		}

	}
	//---------------------------------------------------------------------()
    void GpuProgramParameters::setConstant(size_t index, const Vector4& vec)
    {
        setConstant(index, vec.ptr(), 1);
    }
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, Real val)
	{
		setConstant(index, Vector4(val, 0.0f, 0.0f, 0.0f));
	}
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Vector3& vec)
    {
        setConstant(index, Vector4(vec.x, vec.y, vec.z, 1.0f));
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Matrix4& m)
    {
        // set as 4x 4-element floats
        if (mTransposeMatrices)
        {
            Matrix4 t = m.transpose();
            GpuProgramParameters::setConstant(index, t[0], 4);
        }
        else
        {
            GpuProgramParameters::setConstant(index, m[0], 4);
        }

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Matrix4* pMatrix, 
        size_t numEntries)
    {
        if (mTransposeMatrices)
        {
            for (size_t i = 0; i < numEntries; ++i)
            {
                Matrix4 t = pMatrix[i].transpose();
                GpuProgramParameters::setConstant(index, t[0], 4);
                index += 4;
            }
        }
        else
        {
            GpuProgramParameters::setConstant(index, pMatrix[0][0], 4 * numEntries);
        }

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const ColourValue& colour)
    {
        setConstant(index, colour.ptr(), 1);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const float *val, size_t count)
    {
		// Raw buffer size is 4x count
		size_t rawCount = count * 4;
		// get physical index
		assert(mFloatLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

		size_t physicalIndex = _getFloatConstantPhysicalIndex(index, rawCount);

        // Copy 
		_writeRawConstants(physicalIndex, val, rawCount);

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const double *val, size_t count)
    {
		// Raw buffer size is 4x count
		size_t rawCount = count * 4;
		// get physical index
		assert(mFloatLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

		size_t physicalIndex = _getFloatConstantPhysicalIndex(index, rawCount);
		assert(physicalIndex + rawCount <= mFloatConstants.size());
		// Copy manually since cast required
		for (size_t i = 0; i < rawCount; ++i)
		{
			mFloatConstants[physicalIndex + i] = 
				static_cast<float>(val[i]);
		}

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const int *val, size_t count)
    {
		// Raw buffer size is 4x count
		size_t rawCount = count * 4;
		// get physical index
		assert(mIntLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

		size_t physicalIndex = _getIntConstantPhysicalIndex(index, rawCount);
		// Copy 
		_writeRawConstants(physicalIndex, val, rawCount);
    }
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Vector4& vec,
		size_t count)
	{
		// remember, raw content access uses raw float count rather than float4
		// write either the number requested (for packed types) or up to 4
		_writeRawConstants(physicalIndex, vec.ptr(), std::min(count, (size_t)4));
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, Real val)
	{
		_writeRawConstants(physicalIndex, &val, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, int val)
	{
		_writeRawConstants(physicalIndex, &val, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Vector3& vec)
	{
		_writeRawConstants(physicalIndex, vec.ptr(), 3);		
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Matrix4& m)
	{

		// remember, raw content access uses raw float count rather than float4
		if (mTransposeMatrices)
		{
			Matrix4 t = m.transpose();
			_writeRawConstants(physicalIndex, t[0], 16);
		}
		else
		{
			_writeRawConstants(physicalIndex, m[0], 16);
		}

	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Matrix4* pMatrix, size_t numEntries)
	{
		// remember, raw content access uses raw float count rather than float4
		if (mTransposeMatrices)
		{
			for (size_t i = 0; i < numEntries; ++i)
			{
				Matrix4 t = pMatrix[i].transpose();
				_writeRawConstants(physicalIndex, t[0], 16);
				physicalIndex += 16;
			}
		}
		else
		{
			_writeRawConstants(physicalIndex, pMatrix[0][0], 16 * numEntries);
		}


	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, 
		const ColourValue& colour, size_t count)
	{
		// write either the number requested (for packed types) or up to 4
		_writeRawConstants(physicalIndex, colour.ptr(), std::min(count, (size_t)4));
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstants(size_t physicalIndex, const double* val, size_t count)
	{
		assert(physicalIndex + count <= mFloatConstants.size());
		for (size_t i = 0; i < count; ++i)
		{
			mFloatConstants[physicalIndex+i] = static_cast<float>(val[i]);
		}
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstants(size_t physicalIndex, const float* val, size_t count)
	{
		assert(physicalIndex + count <= mFloatConstants.size());
		memcpy(&mFloatConstants[physicalIndex], val, sizeof(float) * count);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_writeRawConstants(size_t physicalIndex, const int* val, size_t count)
	{
		assert(physicalIndex + count <= mIntConstants.size());
		memcpy(&mIntConstants[physicalIndex], val, sizeof(int) * count);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_readRawConstants(size_t physicalIndex, size_t count, float* dest)
	{
		assert(physicalIndex + count <= mFloatConstants.size());
		memcpy(dest, &mFloatConstants[physicalIndex], sizeof(float) * count);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_readRawConstants(size_t physicalIndex, size_t count, int* dest)
	{
		assert(physicalIndex + count <= mIntConstants.size());
		memcpy(dest, &mIntConstants[physicalIndex], sizeof(int) * count);
	}
	//-----------------------------------------------------------------------------
	size_t GpuProgramParameters::_getFloatConstantPhysicalIndex(
		size_t logicalIndex, size_t requestedSize) 
	{
		if (!mFloatLogicalToPhysical)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"This is not a low-level parameter parameter object",
				"GpuProgramParameters::_getFloatConstantPhysicalIndex");

		size_t physicalIndex;
		OGRE_LOCK_MUTEX(mFloatLogicalToPhysical->mutex)

		GpuLogicalIndexUseMap::iterator logi = mFloatLogicalToPhysical->map.find(logicalIndex);
		if (logi == mFloatLogicalToPhysical->map.end())
		{
			if (requestedSize)
			{
				physicalIndex = mFloatConstants.size();

                // Expand at buffer end
                mFloatConstants.insert(mFloatConstants.end(), requestedSize, 0.0f);

				// Record extended size for future GPU params re-using this information
				mFloatLogicalToPhysical->bufferSize = mFloatConstants.size();

				// low-level programs will not know about mapping ahead of time, so 
				// populate it. Other params objects will be able to just use this
				// accepted mapping since the constant structure will be the same

				// Set up a mapping for all items in the count
				size_t currPhys = physicalIndex;
				size_t count = requestedSize / 4;
				for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
				{
					mFloatLogicalToPhysical->map.insert(
						GpuLogicalIndexUseMap::value_type(
							logicalIndex + logicalNum, 
							GpuLogicalIndexUse(currPhys, requestedSize)));
					currPhys += 4;
				}
			}
			else
			{
				// no match & ignore
				return std::numeric_limits<size_t>::max();
			}

		}
		else
		{
			physicalIndex = logi->second.physicalIndex;
			// check size
			if (logi->second.currentSize < requestedSize)
			{
				// init buffer entry wasn't big enough; could be a mistake on the part
				// of the original use, or perhaps a variable length we can't predict
				// until first actual runtime use e.g. world matrix array
				size_t insertCount = requestedSize - logi->second.currentSize;
				FloatConstantList::iterator insertPos = mFloatConstants.begin();
				std::advance(insertPos, physicalIndex);
				mFloatConstants.insert(insertPos, insertCount, 0.0f);
				// shift all physical positions after this one
				for (GpuLogicalIndexUseMap::iterator i = mFloatLogicalToPhysical->map.begin();
					i != mFloatLogicalToPhysical->map.end(); ++i)
				{
					if (i->second.physicalIndex > physicalIndex)
						i->second.physicalIndex += insertCount;
				}
				for (AutoConstantList::iterator i = mAutoConstants.begin();
					i != mAutoConstants.end(); ++i)
				{
					if (i->physicalIndex > physicalIndex)
						i->physicalIndex += insertCount;
				}
			}
		}

		return physicalIndex;
	}
	//-----------------------------------------------------------------------------
	size_t GpuProgramParameters::_getIntConstantPhysicalIndex(
		size_t logicalIndex, size_t requestedSize)
	{
		if (!mIntLogicalToPhysical)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This is not a low-level parameter parameter object",
			"GpuProgramParameters::_getIntConstantPhysicalIndex");

		size_t physicalIndex;
		OGRE_LOCK_MUTEX(mIntLogicalToPhysical->mutex)

			GpuLogicalIndexUseMap::iterator logi = mIntLogicalToPhysical->map.find(logicalIndex);
		if (logi == mIntLogicalToPhysical->map.end())
		{
			if (requestedSize)
			{
				physicalIndex = mIntConstants.size();

                // Expand at buffer end
                mIntConstants.insert(mIntConstants.end(), requestedSize, 0);

				// Record extended size for future GPU params re-using this information
				mIntLogicalToPhysical->bufferSize = mIntConstants.size();

				// low-level programs will not know about mapping ahead of time, so 
				// populate it. Other params objects will be able to just use this
				// accepted mapping since the constant structure will be the same

				// Set up a mapping for all items in the count
				size_t currPhys = physicalIndex;
				size_t count = requestedSize / 4;
				for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
				{
					mIntLogicalToPhysical->map.insert(
						GpuLogicalIndexUseMap::value_type(
						logicalIndex + logicalNum, 
						GpuLogicalIndexUse(currPhys, requestedSize)));
					currPhys += 4;
				}
			}
			else
			{
				// no match
				return std::numeric_limits<size_t>::max();
			}

		}
		else
		{
			physicalIndex = logi->second.physicalIndex;
			// check size
			if (logi->second.currentSize < requestedSize)
			{
				// init buffer entry wasn't big enough; could be a mistake on the part
				// of the original use, or perhaps a variable length we can't predict
				// until first actual runtime use e.g. world matrix array
				size_t insertCount = requestedSize - logi->second.currentSize;
				IntConstantList::iterator insertPos = mIntConstants.begin();
				std::advance(insertPos, physicalIndex);
				mIntConstants.insert(insertPos, insertCount, 0);
				// shift all physical positions after this one
				for (GpuLogicalIndexUseMap::iterator i = mIntLogicalToPhysical->map.begin();
					i != mIntLogicalToPhysical->map.end(); ++i)
				{
					if (i->second.physicalIndex > physicalIndex)
						i->second.physicalIndex += insertCount;
				}
				for (AutoConstantList::iterator i = mAutoConstants.begin();
					i != mAutoConstants.end(); ++i)
				{
					if (i->physicalIndex > physicalIndex)
						i->physicalIndex += insertCount;
				}
			}
		}

		return physicalIndex;
	}
	//-----------------------------------------------------------------------------
	size_t GpuProgramParameters::getFloatLogicalIndexForPhysicalIndex(size_t physicalIndex)
	{
		// perhaps build a reverse map of this sometime (shared in GpuProgram)
		for (GpuLogicalIndexUseMap::iterator i = mFloatLogicalToPhysical->map.begin();
			i != mFloatLogicalToPhysical->map.end(); ++i)
		{
			if (i->second.physicalIndex == physicalIndex)
				return i->first;
		}
		return std::numeric_limits<size_t>::max();

	}
	//-----------------------------------------------------------------------------
	size_t GpuProgramParameters::getIntLogicalIndexForPhysicalIndex(size_t physicalIndex)
	{
		// perhaps build a reverse map of this sometime (shared in GpuProgram)
		for (GpuLogicalIndexUseMap::iterator i = mIntLogicalToPhysical->map.begin();
			i != mIntLogicalToPhysical->map.end(); ++i)
		{
			if (i->second.physicalIndex == physicalIndex)
				return i->first;
		}
		return std::numeric_limits<size_t>::max();

	}
	//-----------------------------------------------------------------------------
	GpuConstantDefinitionIterator GpuProgramParameters::getConstantDefinitionIterator(void) const
	{
		if (!mNamedConstants)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"This params object is not based on a program with named parameters.",
				"GpuProgramParameters::getConstantDefinitionIterator");

		return GpuConstantDefinitionIterator(mNamedConstants->map.begin(), 
			mNamedConstants->map.end());

	}
	//-----------------------------------------------------------------------------
	const GpuNamedConstants& GpuProgramParameters::getConstantDefinitions() const
	{
		if (!mNamedConstants)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This params object is not based on a program with named parameters.",
			"GpuProgramParameters::getConstantDefinitionIterator");

		return *mNamedConstants;
	}
	//-----------------------------------------------------------------------------
	const GpuConstantDefinition& GpuProgramParameters::getConstantDefinition(const String& name) const
	{
		if (!mNamedConstants)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This params object is not based on a program with named parameters.",
			"GpuProgramParameters::getConstantDefinitionIterator");


		// locate, and throw exception if not found
		const GpuConstantDefinition* def = _findNamedConstantDefinition(name, true);

		return *def;

	}
	//-----------------------------------------------------------------------------
	const GpuConstantDefinition* 
	GpuProgramParameters::_findNamedConstantDefinition(const String& name, 
		bool throwExceptionIfNotFound) const
	{
		if (!mNamedConstants)
		{
			if (throwExceptionIfNotFound)
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Named constants have not been initialised, perhaps a compile error.",
				"GpuProgramParameters::_findNamedConstantDefinition");
			return 0;
		}

		GpuConstantDefinitionMap::const_iterator i = mNamedConstants->map.find(name);
		if (i == mNamedConstants->map.end())
		{
			if (throwExceptionIfNotFound)
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Parameter called " + name + " does not exist. ",
				"GpuProgramParameters::_findNamedConstantDefinition");
			return 0;
		}
		else
		{
			return &(i->second);
		}
	}
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstant(size_t index, AutoConstantType acType, size_t extraInfo)
    {
		// Get auto constant definition for sizing
		const AutoConstantDefinition* autoDef = getAutoConstantDefinition(acType);
		// round up to nearest multiple of 4
		size_t sz = autoDef->elementCount;
		if (sz % 4 > 0)
		{
			sz += 4 - (sz % 4);
		}

		size_t physicalIndex = _getFloatConstantPhysicalIndex(index, sz);

		_setRawAutoConstant(physicalIndex, acType, extraInfo);
    }
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_setRawAutoConstant(size_t physicalIndex, 
		AutoConstantType acType, size_t extraInfo, size_t elementSize)
	{
		// update existing index if it exists
		bool found = false;
		for (AutoConstantList::iterator i = mAutoConstants.begin(); 
			i != mAutoConstants.end(); ++i)
		{
			if (i->physicalIndex == physicalIndex)
			{
				i->paramType = acType;
				i->data = extraInfo;
				i->elementCount = elementSize;
				found = true;
				break;
			}
		}
		if (!found)
			mAutoConstants.push_back(AutoConstantEntry(acType, physicalIndex, extraInfo, elementSize));

	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_setRawAutoConstantReal(size_t physicalIndex, 
		AutoConstantType acType, Real rData, size_t elementSize)
	{
		// update existing index if it exists
		bool found = false;
		for (AutoConstantList::iterator i = mAutoConstants.begin(); 
			i != mAutoConstants.end(); ++i)
		{
			if (i->physicalIndex == physicalIndex)
			{
				i->paramType = acType;
				i->fData = rData;
				i->elementCount = elementSize;
				found = true;
				break;
			}
		}
		if (!found)
			mAutoConstants.push_back(AutoConstantEntry(acType, physicalIndex, rData, elementSize));

	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::clearAutoConstant(size_t index)
	{
		size_t physicalIndex = _getFloatConstantPhysicalIndex(index, 0);
		if (physicalIndex != std::numeric_limits<size_t>::max())
		{
			// update existing index if it exists
			for (AutoConstantList::iterator i = mAutoConstants.begin(); 
				i != mAutoConstants.end(); ++i)
			{
				if (i->physicalIndex == physicalIndex)
				{
					mAutoConstants.erase(i);
					break;
				}
			}
		}
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::clearNamedAutoConstant(const String& name)
	{
		const GpuConstantDefinition* def = _findNamedConstantDefinition(name);
		if (def)
		{
			// Autos are always floating point
			if (def->isFloat())
			{
				for (AutoConstantList::iterator i = mAutoConstants.begin(); 
					i != mAutoConstants.end(); ++i)
				{
					if (i->physicalIndex == def->physicalIndex)
					{
						mAutoConstants.erase(i);
						break;
					}
				}
			}

		}
	}
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::clearAutoConstants(void)
    {
        mAutoConstants.clear();
    }
    //-----------------------------------------------------------------------------
    GpuProgramParameters::AutoConstantIterator GpuProgramParameters::getAutoConstantIterator(void) const
    {
        return AutoConstantIterator(mAutoConstants.begin(), mAutoConstants.end());
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstantReal(size_t index, AutoConstantType acType, Real rData)
    {
		// Get auto constant definition for sizing
		const AutoConstantDefinition* autoDef = getAutoConstantDefinition(acType);
		// round up to nearest multiple of 4
		size_t sz = autoDef->elementCount;
		if (sz % 4 > 0)
		{
			sz += 4 - (sz % 4);
		}

		size_t physicalIndex = _getFloatConstantPhysicalIndex(index, sz);

		_setRawAutoConstantReal(physicalIndex, acType, rData);
    }
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_updateAutoParamsNoLights(const AutoParamDataSource* source)
    {
        if (!hasAutoConstants()) return; // abort early if no autos
        Vector3 vec3;
        Vector4 vec4;
        size_t index;
        size_t numMatrices;
        const Matrix4* pMatrix;
        size_t m;

		mActivePassIterationIndex = std::numeric_limits<size_t>::max();

		// Autoconstant index is not a physical index
        AutoConstantList::const_iterator i, iend;
        iend = mAutoConstants.end();
        for (i = mAutoConstants.begin(); i != iend; ++i)
        {
            switch(i->paramType)
            {
            case ACT_WORLD_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getWorldMatrix());
                break;
            case ACT_INVERSE_WORLD_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getInverseWorldMatrix());
                break;
            case ACT_TRANSPOSE_WORLD_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeWorldMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_WORLD_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldMatrix());
               break;

            case ACT_WORLD_MATRIX_ARRAY_3x4:
                // Loop over matrices
                pMatrix = source->getWorldMatrixArray();
                numMatrices = source->getWorldMatrixCount();
                index = i->physicalIndex;
                for (m = 0; m < numMatrices; ++m)
                {
                    _writeRawConstants(index, (*pMatrix)[0], 12);
                    index += 12;
                    ++pMatrix;
                }
                
                break;
            case ACT_WORLD_MATRIX_ARRAY:
                _writeRawConstant(i->physicalIndex, source->getWorldMatrixArray(), 
                    source->getWorldMatrixCount());
                break;
            case ACT_VIEW_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getViewMatrix());
                break;
            case ACT_INVERSE_VIEW_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseViewMatrix());
               break;
            case ACT_TRANSPOSE_VIEW_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeViewMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_VIEW_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeViewMatrix());
               break;

            case ACT_PROJECTION_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getProjectionMatrix());
                break;
            case ACT_INVERSE_PROJECTION_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseProjectionMatrix());
               break;
            case ACT_TRANSPOSE_PROJECTION_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeProjectionMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeProjectionMatrix());
               break;

            case ACT_VIEWPROJ_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getViewProjectionMatrix());
                break;
            case ACT_INVERSE_VIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseViewProjMatrix());
               break;
            case ACT_TRANSPOSE_VIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeViewProjMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeViewProjMatrix());
               break;

            case ACT_WORLDVIEW_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getWorldViewMatrix());
                break;
            case ACT_INVERSE_WORLDVIEW_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getInverseWorldViewMatrix());
                break;
            case ACT_TRANSPOSE_WORLDVIEW_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeWorldViewMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldViewMatrix());
               break;

            case ACT_WORLDVIEWPROJ_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getWorldViewProjMatrix());
                break;
            case ACT_INVERSE_WORLDVIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseWorldViewProjMatrix());
               break;
            case ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getTransposeWorldViewProjMatrix());
               break;
            case ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
               _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldViewProjMatrix());
               break;

            case ACT_RENDER_TARGET_FLIPPING:
               _writeRawConstant(i->physicalIndex, source->getCurrentRenderTarget()->requiresTextureFlipping() ? -1.f : +1.f);
               break;

            // NB ambient light still here because it's not related to a specific light
            case ACT_AMBIENT_LIGHT_COLOUR: 
                _writeRawConstant(i->physicalIndex, source->getAmbientLightColour(), 
					i->elementCount);
                break;
            case ACT_DERIVED_AMBIENT_LIGHT_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getDerivedAmbientLightColour(),
                    i->elementCount);
                break;
            case ACT_DERIVED_SCENE_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getDerivedSceneColour(),
                    i->elementCount);
                break;

            case ACT_FOG_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getFogColour());
                break;
            case ACT_FOG_PARAMS:
                _writeRawConstant(i->physicalIndex, source->getFogParams(), i->elementCount);
                break;

            case ACT_SURFACE_AMBIENT_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getSurfaceAmbientColour(),
                    i->elementCount);
                break;
            case ACT_SURFACE_DIFFUSE_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getSurfaceDiffuseColour(),
                    i->elementCount);
                break;
            case ACT_SURFACE_SPECULAR_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getSurfaceSpecularColour(),
                    i->elementCount);
                break;
            case ACT_SURFACE_EMISSIVE_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getSurfaceEmissiveColour(),
                    i->elementCount);
                break;
            case ACT_SURFACE_SHININESS:
                _writeRawConstant(i->physicalIndex, source->getSurfaceShininess());
                break;

            case ACT_CAMERA_POSITION:
                _writeRawConstant(i->physicalIndex, source->getCameraPosition(), i->elementCount);
                break;
            case ACT_CAMERA_POSITION_OBJECT_SPACE:
                _writeRawConstant(i->physicalIndex, source->getCameraPositionObjectSpace(), i->elementCount);
                break;

            case ACT_TIME:
               _writeRawConstant(i->physicalIndex, source->getTime() * i->fData);
               break;
           case ACT_TIME_0_X:
               _writeRawConstant(i->physicalIndex, source->getTime_0_X(i->fData));
               break;
            case ACT_COSTIME_0_X:
               _writeRawConstant(i->physicalIndex, source->getCosTime_0_X(i->fData));
               break;
            case ACT_SINTIME_0_X:
               _writeRawConstant(i->physicalIndex, source->getSinTime_0_X(i->fData));
               break;
            case ACT_TANTIME_0_X:
               _writeRawConstant(i->physicalIndex, source->getTanTime_0_X(i->fData));
               break;
            case ACT_TIME_0_X_PACKED:
               _writeRawConstant(i->physicalIndex, source->getTime_0_X_packed(i->fData), i->elementCount);
               break;
            case ACT_TIME_0_1:
               _writeRawConstant(i->physicalIndex, source->getTime_0_1(i->fData));
               break;
            case ACT_COSTIME_0_1:
               _writeRawConstant(i->physicalIndex, source->getCosTime_0_1(i->fData));
               break;
            case ACT_SINTIME_0_1:
               _writeRawConstant(i->physicalIndex, source->getSinTime_0_1(i->fData));
               break;
            case ACT_TANTIME_0_1:
               _writeRawConstant(i->physicalIndex, source->getTanTime_0_1(i->fData));
               break;
            case ACT_TIME_0_1_PACKED:
               _writeRawConstant(i->physicalIndex, source->getTime_0_1_packed(i->fData), i->elementCount);
               break;
            case ACT_TIME_0_2PI:
               _writeRawConstant(i->physicalIndex, source->getTime_0_2Pi(i->fData));
               break;
            case ACT_COSTIME_0_2PI:
               _writeRawConstant(i->physicalIndex, source->getCosTime_0_2Pi(i->fData));
               break;
            case ACT_SINTIME_0_2PI:
               _writeRawConstant(i->physicalIndex, source->getSinTime_0_2Pi(i->fData));
               break;
            case ACT_TANTIME_0_2PI:
               _writeRawConstant(i->physicalIndex, source->getTanTime_0_2Pi(i->fData));
               break;
            case ACT_TIME_0_2PI_PACKED:
               _writeRawConstant(i->physicalIndex, source->getTime_0_2Pi_packed(i->fData), i->elementCount);
               break;
            case ACT_FRAME_TIME:
               _writeRawConstant(i->physicalIndex, source->getFrameTime() * i->fData);
               break;
            case ACT_FPS:
               _writeRawConstant(i->physicalIndex, source->getFPS());
               break;
            case ACT_VIEWPORT_WIDTH:
               _writeRawConstant(i->physicalIndex, source->getViewportWidth());
               break;
            case ACT_VIEWPORT_HEIGHT:
               _writeRawConstant(i->physicalIndex, source->getViewportHeight());
               break;
            case ACT_INVERSE_VIEWPORT_WIDTH:
               _writeRawConstant(i->physicalIndex, source->getInverseViewportWidth());
               break;
            case ACT_INVERSE_VIEWPORT_HEIGHT:
               _writeRawConstant(i->physicalIndex, source->getInverseViewportHeight());
               break;
            case ACT_VIEWPORT_SIZE:
               _writeRawConstant(i->physicalIndex, Vector4(
                   source->getViewportWidth(),
                   source->getViewportHeight(),
                   source->getInverseViewportWidth(),
                   source->getInverseViewportHeight()), i->elementCount);
               break;
			case ACT_TEXEL_OFFSETS:
				{
					RenderSystem* rsys = Root::getSingleton().getRenderSystem();
					_writeRawConstant(i->physicalIndex, Vector4(
						rsys->getHorizontalTexelOffset(), 
						rsys->getVerticalTexelOffset(), 
						rsys->getHorizontalTexelOffset() * source->getInverseViewportWidth(),
						rsys->getVerticalTexelOffset() * source->getInverseViewportHeight()),
						i->elementCount);
				}
				break;
            case ACT_TEXTURE_SIZE:
                _writeRawConstant(i->physicalIndex, source->getTextureSize(i->data), i->elementCount);
                break;
            case ACT_INVERSE_TEXTURE_SIZE:
                _writeRawConstant(i->physicalIndex, source->getInverseTextureSize(i->data), i->elementCount);
                break;
            case ACT_PACKED_TEXTURE_SIZE:
                _writeRawConstant(i->physicalIndex, source->getPackedTextureSize(i->data), i->elementCount);
                break;
			case ACT_SCENE_DEPTH_RANGE:
				_writeRawConstant(i->physicalIndex, source->getSceneDepthRange(), i->elementCount);
				break;
            case ACT_VIEW_DIRECTION:
               _writeRawConstant(i->physicalIndex, source->getViewDirection());
               break;
            case ACT_VIEW_SIDE_VECTOR:
               _writeRawConstant(i->physicalIndex, source->getViewSideVector());
               break;
            case ACT_VIEW_UP_VECTOR:
               _writeRawConstant(i->physicalIndex, source->getViewUpVector());
               break;
            case ACT_FOV:
               _writeRawConstant(i->physicalIndex, source->getFOV());
               break;
            case ACT_NEAR_CLIP_DISTANCE:
               _writeRawConstant(i->physicalIndex, source->getNearClipDistance());
               break;
            case ACT_FAR_CLIP_DISTANCE:
               _writeRawConstant(i->physicalIndex, source->getFarClipDistance());
               break;
            case ACT_PASS_NUMBER:
                _writeRawConstant(i->physicalIndex, (float)source->getPassNumber());
                break;
            case ACT_PASS_ITERATION_NUMBER:
                _writeRawConstant(i->physicalIndex, 0.0f);
                mActivePassIterationIndex = i->physicalIndex;
                break;
            case ACT_CUSTOM:
			case ACT_ANIMATION_PARAMETRIC:
                source->getCurrentRenderable()->_updateCustomGpuParameter(*i, this);
                break;
            case ACT_TEXTURE_MATRIX:
                _writeRawConstant(i->physicalIndex, source->getTextureTransformMatrix(i->data));
                break;
			case ACT_LOD_CAMERA_POSITION:
				_writeRawConstant(i->physicalIndex, source->getLodCameraPosition(), i->elementCount);
				break;
			case ACT_LOD_CAMERA_POSITION_OBJECT_SPACE:
				_writeRawConstant(i->physicalIndex, source->getLodCameraPositionObjectSpace(), i->elementCount);
				break;
            default:
                break;
            }
        }
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_updateAutoParamsLightsOnly(const AutoParamDataSource* source)
    {
        if (!hasAutoConstants()) return; // abort early if no autos
        Vector3 vec3;
        Vector4 vec4;
		Matrix3 m3;

        AutoConstantList::const_iterator i, iend;
        iend = mAutoConstants.end();
        for (i = mAutoConstants.begin(); i != iend; ++i)
        {
            switch(i->paramType)
            {
            case ACT_LIGHT_COUNT:
                _writeRawConstant(i->physicalIndex, source->getLightCount());
                break;
            case ACT_LIGHT_DIFFUSE_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getLightDiffuseColour(i->data), i->elementCount);
                break;
            case ACT_LIGHT_SPECULAR_COLOUR:
                _writeRawConstant(i->physicalIndex, source->getLightSpecularColour(i->data), i->elementCount);
                break;
            case ACT_LIGHT_POSITION:
                // Get as 4D vector, works for directional lights too
				// Use element count in case uniform slot is smaller
                _writeRawConstant(i->physicalIndex, 
                    source->getLightAs4DVector(i->data), i->elementCount);
                break;
            case ACT_LIGHT_DIRECTION:
                vec3 = source->getLightDirection(i->data);
                // Set as 4D vector for compatibility
				// Use element count in case uniform slot is smaller
                _writeRawConstant(i->physicalIndex, Vector4(vec3.x, vec3.y, vec3.z, 1.0f), i->elementCount);
                break;
            case ACT_LIGHT_POSITION_OBJECT_SPACE:
                _writeRawConstant(i->physicalIndex, 
                    source->getInverseWorldMatrix().transformAffine(
						source->getLightAs4DVector(i->data)), 
					i->elementCount);
                break;
            case ACT_LIGHT_DIRECTION_OBJECT_SPACE:
				// We need the inverse of the inverse transpose 
				source->getInverseTransposeWorldMatrix().inverse().extract3x3Matrix(m3);
				vec3 = m3 * source->getLightDirection(i->data);
				vec3.normalise();
                // Set as 4D vector for compatibility
                _writeRawConstant(i->physicalIndex, Vector4(vec3.x, vec3.y, vec3.z, 0.0f), i->elementCount);
                break;
			case ACT_LIGHT_POSITION_VIEW_SPACE:
                _writeRawConstant(i->physicalIndex, 
                    source->getViewMatrix().transformAffine(source->getLightAs4DVector(i->data)), i->elementCount);
                break;
            case ACT_LIGHT_DIRECTION_VIEW_SPACE:
				source->getInverseTransposeViewMatrix().extract3x3Matrix(m3);
				// inverse transpose in case of scaling
				vec3 = m3 * source->getLightDirection(i->data);
                vec3.normalise();
                // Set as 4D vector for compatibility
                _writeRawConstant(i->physicalIndex, Vector4(vec3.x, vec3.y, vec3.z, 0.0f),i->elementCount);
                break;
            case ACT_LIGHT_DISTANCE_OBJECT_SPACE:
                vec3 = source->getInverseWorldMatrix().transformAffine(source->getLightPosition(i->data));
                _writeRawConstant(i->physicalIndex, vec3.length());
                break;
            case ACT_SHADOW_EXTRUSION_DISTANCE:
                _writeRawConstant(i->physicalIndex, source->getShadowExtrusionDistance());
                break;
			case ACT_SHADOW_SCENE_DEPTH_RANGE:
				_writeRawConstant(i->physicalIndex, source->getShadowSceneDepthRange(i->data));
				break;
			case ACT_SHADOW_COLOUR:
				_writeRawConstant(i->physicalIndex, source->getShadowColour(), i->elementCount);
				break;
            case ACT_LIGHT_POWER_SCALE:
				_writeRawConstant(i->physicalIndex, source->getLightPowerScale(i->data));
				break;
			case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED:
				_writeRawConstant(i->physicalIndex, source->getLightDiffuseColourWithPower(i->data), i->elementCount);
				break;
			case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED:
				_writeRawConstant(i->physicalIndex, source->getLightSpecularColourWithPower(i->data), i->elementCount);
				break;
			case ACT_LIGHT_NUMBER:
				_writeRawConstant(i->physicalIndex, source->getLightNumber(i->data));
				break;
			case ACT_LIGHT_CASTS_SHADOWS:
				_writeRawConstant(i->physicalIndex, source->getLightCastsShadows(i->data));
				break;
            case ACT_LIGHT_ATTENUATION:
				_writeRawConstant(i->physicalIndex, source->getLightAttenuation(i->data), i->elementCount);
                break;
			case ACT_SPOTLIGHT_PARAMS:
				_writeRawConstant(i->physicalIndex, source->getSpotlightParams(i->data), i->elementCount);
				break;
			case ACT_LIGHT_DIFFUSE_COLOUR_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getLightDiffuseColour(l), i->elementCount);
				break;

			case ACT_LIGHT_SPECULAR_COLOUR_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getLightSpecularColour(l), i->elementCount);
				break;
			case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
					source->getLightDiffuseColourWithPower(l), i->elementCount);
				break;

			case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
					source->getLightSpecularColourWithPower(l), i->elementCount);
				break;

			case ACT_LIGHT_POSITION_ARRAY:
				// Get as 4D vector, works for directional lights too
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getLightAs4DVector(l), i->elementCount);
				break;

			case ACT_LIGHT_DIRECTION_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
				{
					vec3 = source->getLightDirection(l);
					// Set as 4D vector for compatibility
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						Vector4(vec3.x, vec3.y, vec3.z, 0.0f), i->elementCount);
				}
				break;

			case ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getInverseWorldMatrix().transformAffine(
							source->getLightAs4DVector(l)), 
						i->elementCount);
				break;

			case ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY:
				// We need the inverse of the inverse transpose 
				source->getInverseTransposeWorldMatrix().inverse().extract3x3Matrix(m3);
				for (size_t l = 0; l < i->data; ++l)
				{
					vec3 = m3 * source->getLightDirection(l);
					vec3.normalise();
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						Vector4(vec3.x, vec3.y, vec3.z, 0.0f), i->elementCount); 
				}
				break;

			case ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getViewMatrix().transformAffine(
							source->getLightAs4DVector(l)),
						i->elementCount);
				break;

			case ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY:
				source->getInverseTransposeViewMatrix().extract3x3Matrix(m3);
				for (size_t l = 0; l < i->data; ++l)
				{
					vec3 = m3 * source->getLightDirection(l);
					vec3.normalise();
					// Set as 4D vector for compatibility
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						Vector4(vec3.x, vec3.y, vec3.z, 0.0f), i->elementCount);
				}
				break;

			case ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
				{
					vec3 = source->getInverseWorldMatrix().transformAffine(source->getLightPosition(l));
					_writeRawConstant(i->physicalIndex + l*i->elementCount, vec3.length());
				}
				break;

			case ACT_LIGHT_POWER_SCALE_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getLightPowerScale(l));
				break;

			case ACT_LIGHT_ATTENUATION_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
				{
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getLightAttenuation(l), i->elementCount);
				}
				break;
			case ACT_SPOTLIGHT_PARAMS_ARRAY:
				for (size_t l = 0 ; l < i->data; ++l)
				{
					_writeRawConstant(i->physicalIndex + l*i->elementCount, source->getSpotlightParams(l), 
						i->elementCount);
				}
				break;
            case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR:
                _writeRawConstant(i->physicalIndex,
                    source->getLightDiffuseColourWithPower(i->data) * source->getSurfaceDiffuseColour(),
                    i->elementCount);
                break;
            case ACT_DERIVED_LIGHT_SPECULAR_COLOUR:
                _writeRawConstant(i->physicalIndex,
                    source->getLightSpecularColourWithPower(i->data) * source->getSurfaceSpecularColour(),
                    i->elementCount);
                break;
            case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
                {
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
                        source->getLightDiffuseColourWithPower(l) * source->getSurfaceDiffuseColour(),
                        i->elementCount);
                }
                break;
            case ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
                {
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
                        source->getLightSpecularColourWithPower(l) * source->getSurfaceSpecularColour(),
                        i->elementCount);
                }
                break;
			case ACT_TEXTURE_VIEWPROJ_MATRIX:
				// can also be updated in lights
				_writeRawConstant(i->physicalIndex, source->getTextureViewProjMatrix(i->data));
				break;
			case ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
				{
					// can also be updated in lights
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getTextureViewProjMatrix(l));
				}
				break;
			case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX:
				// can also be updated in lights
				_writeRawConstant(i->physicalIndex, source->getTextureWorldViewProjMatrix(i->data));
				break;
			case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY:
				for (size_t l = 0; l < i->data; ++l)
				{
					// can also be updated in lights
					_writeRawConstant(i->physicalIndex + l*i->elementCount, 
						source->getTextureWorldViewProjMatrix(l));
				}
				break;
			case ACT_SPOTLIGHT_VIEWPROJ_MATRIX:
				_writeRawConstant(i->physicalIndex, source->getSpotlightViewProjMatrix(i->data));
				break;
			case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX:
				_writeRawConstant(i->physicalIndex, source->getSpotlightWorldViewProjMatrix(i->data));
				break;
            default:
                // do nothing
                break;
            }
        }
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, Real val)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, val);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, int val)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, val);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector4& vec)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, vec, def->elementSize);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector3& vec)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, vec);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4& m)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, m);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4* m, 
        size_t numEntries)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, m, numEntries);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, 
		const float *val, size_t count, size_t multiple)
    {
		size_t rawCount = count * multiple;
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstants(def->physicalIndex, val, rawCount);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, 
		const double *val, size_t count, size_t multiple)
    {
		size_t rawCount = count * multiple;
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstants(def->physicalIndex, val, rawCount);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const ColourValue& colour)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstant(def->physicalIndex, colour, def->elementSize);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, 
		const int *val, size_t count, size_t multiple)
    {
		size_t rawCount = count * multiple;
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_writeRawConstants(def->physicalIndex, val, rawCount);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstant(const String& name, 
		AutoConstantType acType, size_t extraInfo)
    {
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_setRawAutoConstant(def->physicalIndex, acType, extraInfo, def->elementSize);

    }
	//---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedAutoConstantReal(const String& name, 
		AutoConstantType acType, Real rData)
	{
		// look up, and throw an exception if we're not ignoring missing
		const GpuConstantDefinition* def = 
			_findNamedConstantDefinition(name, !mIgnoreMissingParams);
		if (def)
			_setRawAutoConstantReal(def->physicalIndex, acType, rData, def->elementSize);

	}
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setConstantFromTime(size_t index, Real factor)
    {
        setAutoConstantReal(index, ACT_TIME, factor);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstantFromTime(const String& name, Real factor)
    {
		setNamedAutoConstantReal(name, ACT_TIME, factor);
    }
    //---------------------------------------------------------------------------
    GpuProgramParameters::AutoConstantEntry* GpuProgramParameters::getAutoConstantEntry(const size_t index)
    {
        if (index < mAutoConstants.size())
        {
            return &(mAutoConstants[index]);
        }
        else
        {
            return NULL;
        }
    }
	//---------------------------------------------------------------------------
	const GpuProgramParameters::AutoConstantEntry* 
	GpuProgramParameters::findFloatAutoConstantEntry(size_t logicalIndex)
	{
		if (!mFloatLogicalToPhysical)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This is not a low-level parameter parameter object",
			"GpuProgramParameters::findFloatAutoConstantEntry");

		return _findRawAutoConstantEntryFloat(
			_getFloatConstantPhysicalIndex(logicalIndex, 0));

	}
	//---------------------------------------------------------------------------
	const GpuProgramParameters::AutoConstantEntry* 
		GpuProgramParameters::findIntAutoConstantEntry(size_t logicalIndex)
	{
		if (!mIntLogicalToPhysical)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This is not a low-level parameter parameter object",
			"GpuProgramParameters::findIntAutoConstantEntry");

		return _findRawAutoConstantEntryInt(
			_getIntConstantPhysicalIndex(logicalIndex, 0));


	}
	//---------------------------------------------------------------------------
	const GpuProgramParameters::AutoConstantEntry* 
	GpuProgramParameters::findAutoConstantEntry(const String& paramName)
	{
		if (!mNamedConstants)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"This params object is not based on a program with named parameters.",
			"GpuProgramParameters::findAutoConstantEntry");

		const GpuConstantDefinition& def = getConstantDefinition(paramName);
		if (def.isFloat())
		{
			return _findRawAutoConstantEntryFloat(def.physicalIndex);
		}
		else
		{
			return _findRawAutoConstantEntryInt(def.physicalIndex);
		}
	}
	//---------------------------------------------------------------------------
	const GpuProgramParameters::AutoConstantEntry* 
	GpuProgramParameters::_findRawAutoConstantEntryFloat(size_t physicalIndex)
	{
		for(AutoConstantList::iterator i = mAutoConstants.begin();
			i != mAutoConstants.end(); ++i)
		{
			AutoConstantEntry& ac = *i;
			// should check that auto is float and not int so that physicalIndex
			// doesn't have any ambiguity
			// However, all autos are float I think so no need
			if (ac.physicalIndex == physicalIndex)
				return &ac;
		}

		return 0;

	}
	//---------------------------------------------------------------------------
	const GpuProgramParameters::AutoConstantEntry* 
	GpuProgramParameters::_findRawAutoConstantEntryInt(size_t physicalIndex)
	{
		// No autos are float?
		return 0;
	}
    //---------------------------------------------------------------------------
    void GpuProgramParameters::copyConstantsFrom(const GpuProgramParameters& source)
    {
		// Pull buffers & auto constant list over directly
		mFloatConstants = source.getFloatConstantList();
		mIntConstants = source.getIntConstantList();
		mAutoConstants = source.getAutoConstantList();
    }
    //-----------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantDefinition* 
	GpuProgramParameters::getAutoConstantDefinition(const String& name)
    {
        // find a constant definition that matches name by iterating through the 
		// constant definition array
        bool nameFound = false;
        size_t i = 0;
        const size_t numDefs = getNumAutoConstantDefinitions();
        while (!nameFound && (i < numDefs))
        {
            if (name == AutoConstantDictionary[i].name) 
                nameFound = true;
            else
                ++i;
        }

        if (nameFound)
            return &AutoConstantDictionary[i];
        else
            return 0;
    }

    //-----------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantDefinition* 
	GpuProgramParameters::getAutoConstantDefinition(const size_t idx) 
    {

        if (idx < getNumAutoConstantDefinitions())
        {
            // verify index is equal to acType
            // if they are not equal then the dictionary was not setup properly
            assert(idx == static_cast<size_t>(AutoConstantDictionary[idx].acType));
            return &AutoConstantDictionary[idx];
        }
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    size_t GpuProgramParameters::getNumAutoConstantDefinitions(void)
    {
        return sizeof(AutoConstantDictionary)/sizeof(AutoConstantDefinition);
    }

    //-----------------------------------------------------------------------
    void GpuProgramParameters::incPassIterationNumber(void)
    {
		if (mActivePassIterationIndex != std::numeric_limits<size_t>::max())
        {
			// This is a physical index
			++mFloatConstants[mActivePassIterationIndex];
        }
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String GpuProgram::CmdType::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        if (t->getType() == GPT_VERTEX_PROGRAM)
        {
            return "vertex_program";
        }
		else if (t->getType() == GPT_GEOMETRY_PROGRAM)
		{
			return "geometry_program";
		}
		else
        {
            return "fragment_program";
        }
    }
    void GpuProgram::CmdType::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        if (val == "vertex_program")
        {
            t->setType(GPT_VERTEX_PROGRAM);
        }
        else if (val == "geometry_program")
		{
			t->setType(GPT_GEOMETRY_PROGRAM);
		}
		else
        {
            t->setType(GPT_FRAGMENT_PROGRAM);
        }
    }
    //-----------------------------------------------------------------------
    String GpuProgram::CmdSyntax::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        return t->getSyntaxCode();
    }
    void GpuProgram::CmdSyntax::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->setSyntaxCode(val);
    }
    //-----------------------------------------------------------------------
    String GpuProgram::CmdSkeletal::doGet(const void* target) const
    {
        const GpuProgram* t = static_cast<const GpuProgram*>(target);
        return StringConverter::toString(t->isSkeletalAnimationIncluded());
    }
    void GpuProgram::CmdSkeletal::doSet(void* target, const String& val)
    {
        GpuProgram* t = static_cast<GpuProgram*>(target);
        t->setSkeletalAnimationIncluded(StringConverter::parseBool(val));
    }
	//-----------------------------------------------------------------------
	String GpuProgram::CmdMorph::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isMorphAnimationIncluded());
	}
	void GpuProgram::CmdMorph::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setMorphAnimationIncluded(StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdPose::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->getNumberOfPosesIncluded());
	}
	void GpuProgram::CmdPose::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setPoseAnimationIncluded(StringConverter::parseUnsignedInt(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdVTF::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isVertexTextureFetchRequired());
	}
	void GpuProgram::CmdVTF::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setVertexTextureFetchRequired(StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdManualNamedConstsFile::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return t->getManualNamedConstantsFile();
	}
	void GpuProgram::CmdManualNamedConstsFile::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setManualNamedConstantsFile(val);
	}
    //-----------------------------------------------------------------------
	String GpuProgram::CmdAdjacency::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isAdjacencyInfoRequired());
	}
	void GpuProgram::CmdAdjacency::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setAdjacencyInfoRequired(StringConverter::parseBool(val));
	}
    //-----------------------------------------------------------------------
    GpuProgramPtr& GpuProgramPtr::operator=(const HighLevelGpuProgramPtr& r)
    {
        // Can assign direct
        if (pRep == r.getPointer())
            return *this;
        release();
		// lock & copy other mutex pointer
        OGRE_MUTEX_CONDITIONAL(r.OGRE_AUTO_MUTEX_NAME)
        {
		    OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		    OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
            pRep = r.getPointer();
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

}
