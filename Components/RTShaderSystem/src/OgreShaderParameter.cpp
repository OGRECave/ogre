/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#include "OgreShaderParameter.h"
#include "OgreStringConverter.h"

namespace Ogre {
namespace RTShader {

	struct AutoShaderParameter
	{
		GpuProgramParameters::AutoConstantType	autoType;
		String									name;
		GpuConstantType							type;


		AutoShaderParameter(GpuProgramParameters::AutoConstantType _autoType, 
							const String& _name, 
							GpuConstantType _type)
		{
			autoType = _autoType;
			name	 = _name;
			type	 = _type;
		}
	};

	AutoShaderParameter g_AutoParameters[] = {
		AutoShaderParameter(GpuProgramParameters::ACT_WORLD_MATRIX,                   		"world_matrix",							GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX,           		"inverse_world_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_WORLD_MATRIX,         		"transpose_world_matrix",           	GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, 		"inverse_transpose_world_matrix",		GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4,					"world_matrix_array_3x4",				GCT_MATRIX_3X4),
		AutoShaderParameter(GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY,						"world_matrix_array",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEW_MATRIX,							"view_matrix",							GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX,					"inverse_view_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_VIEW_MATRIX,          		"transpose_view_matrix",				GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEW_MATRIX,  		"inverse_transpose_view_matrix",		GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_PROJECTION_MATRIX,              		"projection_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX,      		"inverse_projection_matrix",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_PROJECTION_MATRIX,    		"transpose_projection_matrix",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX,	"inverse_transpose_projection_matrix",	GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEWPROJ_MATRIX,						"viewproj_matrix",						GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_VIEWPROJ_MATRIX,				"inverse_viewproj_matrix",				GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_VIEWPROJ_MATRIX,				"transpose_viewproj_matrix",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX,		"inverse_transpose_viewproj_matrix",	GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX,              			"worldview_matrix",						GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_WORLDVIEW_MATRIX,      			"inverse_worldview_matrix",				GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEW_MATRIX,				"transpose_worldview_matrix",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX,		"inverse_transpose_worldview_matrix",	GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX,					"worldviewproj_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_WORLDVIEWPROJ_MATRIX,			"inverse_worldviewproj_matrix",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX,			"transpose_worldviewproj_matrix",		GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX, "inverse_transpose_worldviewproj_matrix", GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING,					"render_target_flipping",				GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_FOG_COLOUR,								"fog_colour",							GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_FOG_PARAMS,								"fog_params",							GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR,         		"surface_ambient_colour",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR,         		"surface_diffuse_colour",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR,        		"surface_specular_colour",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SURFACE_EMISSIVE_COLOUR,        		"surface_emissive_colour",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SURFACE_SHININESS,              		"surface_shininess",					GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_COUNT,                   			"light_count",                  		GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR,          			"ambient_light_colour",         		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR,          			"light_diffuse_colour",         		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR,         			"light_specular_colour",        		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION,             			"light_attenuation",            		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS,              			"spotlight_params",             		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION,                			"light_position",               		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE,   			"light_position_object_space",  		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE,				"light_position_view_space",    		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION,						"light_direction",              		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE,			"light_direction_object_space", 		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE,				"light_direction_view_space",   		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE,			"light_distance_object_space",  		GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POWER_SCALE,   					"light_power",							GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED,		"light_diffuse_colour_power_scaled",         GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED,		"light_specular_colour_power_scaled",        GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_ARRAY,				"light_diffuse_colour_array",			GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_ARRAY,			"light_specular_colour_array",			GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY, "light_diffuse_colour_power_scaled_array",         GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY, "light_specular_colour_power_scaled_array",        GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_ATTENUATION_ARRAY,				"light_attenuation_array",            		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION_ARRAY,					"light_position_array",               		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY,		"light_position_object_space_array",  		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY,        "light_position_view_space_array",    		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_ARRAY,					"light_direction_array",              		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY,		"light_direction_object_space_array", 		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY,       "light_direction_view_space_array",   		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY,		"light_distance_object_space_array",		GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_POWER_SCALE_ARRAY,   				"light_power_array",						GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_SPOTLIGHT_PARAMS_ARRAY,					"spotlight_params_array",					GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_AMBIENT_LIGHT_COLOUR,    		"derived_ambient_light_colour",     		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR,            		"derived_scene_colour",             		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR,    		"derived_light_diffuse_colour",     		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR,   		"derived_light_specular_colour",    		GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY,		"derived_light_diffuse_colour_array",   	GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY,	"derived_light_specular_colour_array",  	GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_NUMBER,   						"light_number",								GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_LIGHT_CASTS_SHADOWS, 					"light_casts_shadows",						GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_SHADOW_EXTRUSION_DISTANCE,     			"shadow_extrusion_distance",				GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_CAMERA_POSITION,               			"camera_position",							GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE,  			"camera_position_object_space",				GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX,       			"texture_viewproj_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY, 			"texture_viewproj_matrix_array",			GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX,  			"texture_worldviewproj_matrix",				GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY,		"texture_worldviewproj_matrix_array",		GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX,				"spotlight_viewproj_matrix",				GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX,			"spotlight_worldviewproj_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_CUSTOM,									"custom",											GCT_FLOAT4),  // *** needs to be tested
		AutoShaderParameter(GpuProgramParameters::ACT_TIME,										"time",											GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_X,                      			"time_0_x",                     GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_COSTIME_0_X,                   			"costime_0_x",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_SINTIME_0_X,                   			"sintime_0_x",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TANTIME_0_X,                   			"tantime_0_x",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_X_PACKED,               			"time_0_x_packed",              GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_1,                      			"time_0_1",                     GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_COSTIME_0_1,                   			"costime_0_1",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_SINTIME_0_1,                   			"sintime_0_1",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TANTIME_0_1,                   			"tantime_0_1",                  GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_1_PACKED,               			"time_0_1_packed",              GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_2PI,                    			"time_0_2pi",                   GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_COSTIME_0_2PI,                 			"costime_0_2pi",                GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_SINTIME_0_2PI,                 			"sintime_0_2pi",                GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TANTIME_0_2PI,                 			"tantime_0_2pi",                GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_TIME_0_2PI_PACKED,             			"time_0_2pi_packed",            GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_FRAME_TIME,                    			"frame_time",                   GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_FPS,                           			"fps",                          GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEWPORT_WIDTH,                			"viewport_width",               GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEWPORT_HEIGHT,               			"viewport_height",              GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_VIEWPORT_WIDTH,        			"inverse_viewport_width",       GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_VIEWPORT_HEIGHT,       			"inverse_viewport_height",      GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEWPORT_SIZE,                 			"viewport_size",                GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEW_DIRECTION,                			"view_direction",               GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEW_SIDE_VECTOR,              			"view_side_vector",             GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_VIEW_UP_VECTOR,                			"view_up_vector",               GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_FOV,                           			"fov",                          GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_NEAR_CLIP_DISTANCE,						"near_clip_distance",           GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_FAR_CLIP_DISTANCE,						"far_clip_distance",            GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_PASS_NUMBER,							"pass_number",                        GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_PASS_ITERATION_NUMBER,					"pass_iteration_number",              GCT_FLOAT1),
		AutoShaderParameter(GpuProgramParameters::ACT_ANIMATION_PARAMETRIC,					"animation_parametric",               GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXEL_OFFSETS,               			"texel_offsets",					GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SCENE_DEPTH_RANGE,           			"scene_depth_range",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SHADOW_SCENE_DEPTH_RANGE,    			"shadow_scene_depth_range",			GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_SHADOW_COLOUR,							"shadow_colour",					GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_SIZE,               			 "texture_size",					GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE,       			 "inverse_texture_size",			GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_PACKED_TEXTURE_SIZE,        			 "packed_texture_size",				GCT_FLOAT4),
		AutoShaderParameter(GpuProgramParameters::ACT_TEXTURE_MATRIX,							 "texture_matrix",					GCT_MATRIX_4X4),
		AutoShaderParameter(GpuProgramParameters::ACT_LOD_CAMERA_POSITION,					"lod_camera_position",              GCT_FLOAT3),
		AutoShaderParameter(GpuProgramParameters::ACT_LOD_CAMERA_POSITION_OBJECT_SPACE,		"lod_camera_position_object_space", GCT_FLOAT3),
	};

//-----------------------------------------------------------------------
Parameter::Parameter(GpuConstantType type, const String& name, 
			const Semantic& semantic, int index, 
			const Content& content,
			uint16 variability)
{
	mName					= name;
	mType					= type;
	mSemantic				= semantic;
	mIndex					= index;
	mContent				= content;
	mIsAutoConstantReal		= false;	
	mIsAutoConstantInt		= false;	
	mAutoConstantRealData	= 0.0f;
	mVariability			= variability;
}

//-----------------------------------------------------------------------
Parameter::Parameter(GpuProgramParameters::AutoConstantType autoType, Real fAutoConstantData)
{
	AutoShaderParameter* parameterDef = &g_AutoParameters[autoType];

	mName				= parameterDef->name;
	if (fAutoConstantData != 0.0)
		mName += StringConverter::toString(fAutoConstantData);
	mType				= parameterDef->type;
	mSemantic			= SPS_UNKNOWN;
	mIndex				= -1;
	mContent			= SPC_UNKNOWN;
	mIsAutoConstantReal	= true;	
	mIsAutoConstantInt	= false;
	mAutoConstantType	= autoType;
	mAutoConstantRealData = fAutoConstantData;
	mVariability		= (uint16)GPV_GLOBAL;
}

//-----------------------------------------------------------------------
Parameter::Parameter(GpuProgramParameters::AutoConstantType autoType, size_t nAutoConstantData)
{
	AutoShaderParameter* parameterDef = &g_AutoParameters[autoType];

	mName				= parameterDef->name;
	if (nAutoConstantData != 0)
		mName += StringConverter::toString(nAutoConstantData);
	mType				= parameterDef->type;
	mSemantic			= SPS_UNKNOWN;
	mIndex				= -1;
	mContent			= SPC_UNKNOWN;
	mIsAutoConstantReal	= false;	
	mIsAutoConstantInt	= true;
	mAutoConstantType	= autoType;
	mAutoConstantIntData = nAutoConstantData;
	mVariability		= (uint16)GPV_GLOBAL;
}

//-----------------------------------------------------------------------
bool Parameter::isFloat() const
{
	switch(getType())
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

//-----------------------------------------------------------------------
bool Parameter::isSampler() const
{
	switch(getType())
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



//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInPosition(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "iPos_" + StringConverter::toString(index), 
		Parameter::SPS_POSITION, index, 
		Parameter::SPC_POSITION_OBJECT_SPACE,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutPosition(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "oPos_" + StringConverter::toString(index), 
		Parameter::SPS_POSITION, index, 
		Parameter::SPC_POSITION_PROJECTIVE_SPACE,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInNormal(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "iNormal_" + StringConverter::toString(index), 
		Parameter::SPS_NORMAL, index, 
		Parameter::SPC_NORMAL_OBJECT_SPACE,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInBiNormal(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "iBiNormal_" + StringConverter::toString(index), 
		Parameter::SPS_BINORMAL, index, 
		Parameter::SPC_BINORMAL,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTangent(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "iTangent_" + StringConverter::toString(index), 
		Parameter::SPS_TANGENT, index, 
		Parameter::SPC_TANGENT,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutNormal(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "oNormal_" + StringConverter::toString(index), 
		Parameter::SPS_NORMAL, index, 
		Parameter::SPC_NORMAL_OBJECT_SPACE,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutBiNormal(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "oBiNormal_" + StringConverter::toString(index), 
		Parameter::SPS_BINORMAL, index, 
		Parameter::SPC_BINORMAL,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTangent(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "oTangent_" + StringConverter::toString(index), 
		Parameter::SPS_TANGENT, index, 
		Parameter::SPC_TANGENT,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInColor(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "iColor_" + StringConverter::toString(index), 
		Parameter::SPS_COLOR, index, 
		index == 0 ? Parameter::SPC_COLOR_DIFFUSE : Parameter::SPC_COLOR_SPECULAR,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutColor(int index)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "oColor_" + StringConverter::toString(index), 
		Parameter::SPS_COLOR, index, 
		index == 0 ? Parameter::SPC_COLOR_DIFFUSE : Parameter::SPC_COLOR_SPECULAR,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord(GpuConstantType type, int index, Parameter::Content content)
{
	switch (type)
	{
	case GCT_FLOAT1:
		return createInTexcoord1(index, content);
		
	case GCT_FLOAT2:
		return createInTexcoord2(index, content);
		
	case GCT_FLOAT3:
		return createInTexcoord3(index, content);
		
	case GCT_FLOAT4:
		return createInTexcoord4(index, content);		
	}

	return ParameterPtr();
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord(GpuConstantType type, int index, Parameter::Content content)
{
	switch (type)
	{
	case GCT_FLOAT1:
		return createOutTexcoord1(index, content);

	case GCT_FLOAT2:
		return createOutTexcoord2(index, content);

	case GCT_FLOAT3:
		return createOutTexcoord3(index, content);

	case GCT_FLOAT4:
		return createOutTexcoord4(index, content);		
	}

	return ParameterPtr();
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord1(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT1, "iTexcoord1_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord1(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT1, "oTexcoord1_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord2(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT2, "iTexcoord2_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord2(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT2, "oTexcoord2_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord3(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "iTexcoord3_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord3(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT3, "oTexcoord3_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createInTexcoord4(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "iTexcoord4_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createOutTexcoord4(int index, Parameter::Content content)
{
	return ParameterPtr(new Parameter(GCT_FLOAT4, "oTexcoord4_" + StringConverter::toString(index), 
		Parameter::SPS_TEXTURE_COORDINATES, index, 
		content,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createSampler(GpuConstantType type, int index)
{
	switch (type)
	{
	case GCT_SAMPLER1D:
		return createSampler1D(index);

	case GCT_SAMPLER2D:
		return createSampler2D(index);

	case GCT_SAMPLER3D:
		return createSampler3D(index);

	case GCT_SAMPLERCUBE:
		return createSamplerCUBE(index);
	}

	return ParameterPtr();
	
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createSampler1D(int index)
{
	return ParameterPtr(new Parameter(GCT_SAMPLER1D, "gSampler1D_" + StringConverter::toString(index), 
		Parameter::SPS_UNKNOWN, index, 
		Parameter::SPC_UNKNOWN,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createSampler2D(int index)
{
	return ParameterPtr(new Parameter(GCT_SAMPLER2D, "gSampler2D_" + StringConverter::toString(index), 
		Parameter::SPS_UNKNOWN, index, 
		Parameter::SPC_UNKNOWN,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createSampler3D(int index)
{
	return ParameterPtr(new Parameter(GCT_SAMPLER3D, "gSampler3D_" + StringConverter::toString(index), 
		Parameter::SPS_UNKNOWN, index, 
		Parameter::SPC_UNKNOWN,
		(uint16)GPV_GLOBAL));
}

//-----------------------------------------------------------------------
ParameterPtr ParameterFactory::createSamplerCUBE(int index)
{
	return ParameterPtr(new Parameter(GCT_SAMPLERCUBE, "gSamplerCUBE_" + StringConverter::toString(index), 
		Parameter::SPS_UNKNOWN, index, 
		Parameter::SPC_UNKNOWN,
		(uint16)GPV_GLOBAL));
}

}
}