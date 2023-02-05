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
#include "OgreStableHeaders.h"
#include "OgreGpuProgramParams.h"
#include "OgreGpuProgramManager.h"
#include "OgreDualQuaternion.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    GpuProgramParameters::AutoConstantDefinition GpuProgramParameters::AutoConstantDictionary[] = {
        AutoConstantDefinition(ACT_WORLD_MATRIX,                  "world_matrix",                16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLD_MATRIX,          "inverse_world_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLD_MATRIX,             "transpose_world_matrix",            16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, "inverse_transpose_world_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_BONE_MATRIX_ARRAY_3x4,        "bone_matrix_array_3x4",      12, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_MATRIX_ARRAY,            "bone_matrix_array",          16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_DUALQUATERNION_ARRAY_2x4, "bone_dualquaternion_array_2x4",      8, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_SCALE_SHEAR_MATRIX_ARRAY_3x4, "bone_scale_shear_matrix_array_3x4", 12, ET_REAL, ACDT_NONE),
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
        AutoConstantDefinition(ACT_NORMAL_MATRIX,                  "normal_matrix",     9, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_WORLDVIEWPROJ_MATRIX,          "worldviewproj_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLDVIEWPROJ_MATRIX,       "inverse_worldviewproj_matrix",      16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX,     "transpose_worldviewproj_matrix",    16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX, "inverse_transpose_worldviewproj_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_RENDER_TARGET_FLIPPING,          "render_target_flipping",         1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_VERTEX_WINDING,          "vertex_winding",         1, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_FOG_COLOUR,                    "fog_colour",                   4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_FOG_PARAMS,                    "fog_params",                   4, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_SURFACE_AMBIENT_COLOUR,          "surface_ambient_colour",           4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_DIFFUSE_COLOUR,          "surface_diffuse_colour",           4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_SPECULAR_COLOUR,         "surface_specular_colour",          4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_EMISSIVE_COLOUR,         "surface_emissive_colour",          4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_SHININESS,               "surface_shininess",                1, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SURFACE_ALPHA_REJECTION_VALUE,   "surface_alpha_rejection_value",    1, ET_REAL, ACDT_NONE),

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
        AutoConstantDefinition(ACT_LIGHT_POWER_SCALE,                     "light_power",  1, ET_REAL, ACDT_INT),
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
        AutoConstantDefinition(ACT_LIGHT_POWER_SCALE_ARRAY,               "light_power_array",  1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SPOTLIGHT_PARAMS_ARRAY,              "spotlight_params_array",             4, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_DERIVED_AMBIENT_LIGHT_COLOUR,    "derived_ambient_light_colour",     4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_DERIVED_SCENE_COLOUR,            "derived_scene_colour",             4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_DIFFUSE_COLOUR,    "derived_light_diffuse_colour",     4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_SPECULAR_COLOUR,   "derived_light_specular_colour",    4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY,  "derived_light_diffuse_colour_array",   4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY, "derived_light_specular_colour_array",  4, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_LIGHT_NUMBER,                                          "light_number",  1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_CASTS_SHADOWS,                           "light_casts_shadows",  1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LIGHT_CASTS_SHADOWS_ARRAY,     "light_casts_shadows_array",  1, ET_REAL, ACDT_INT),

        AutoConstantDefinition(ACT_SHADOW_EXTRUSION_DISTANCE,     "shadow_extrusion_distance",    1, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_CAMERA_POSITION,               "camera_position",              3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_CAMERA_POSITION_OBJECT_SPACE,  "camera_position_object_space", 3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_CAMERA_RELATIVE_POSITION,      "camera_relative_position",     3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TEXTURE_VIEWPROJ_MATRIX,       "texture_viewproj_matrix",     16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY, "texture_viewproj_matrix_array", 16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_TEXTURE_WORLDVIEWPROJ_MATRIX,  "texture_worldviewproj_matrix",16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY, "texture_worldviewproj_matrix_array",16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SPOTLIGHT_VIEWPROJ_MATRIX,       "spotlight_viewproj_matrix",     16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY, "spotlight_viewproj_matrix_array", 16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX,  "spotlight_worldviewproj_matrix",16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY,  "spotlight_worldviewproj_matrix_array",16, ET_REAL, ACDT_INT),
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
        AutoConstantDefinition(ACT_TEXEL_OFFSETS,               "texel_offsets",                                  4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SCENE_DEPTH_RANGE,           "scene_depth_range",                      4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_SHADOW_SCENE_DEPTH_RANGE,    "shadow_scene_depth_range",               4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY,    "shadow_scene_depth_range_array",           4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_SHADOW_COLOUR,                               "shadow_colour",                                  4, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TEXTURE_SIZE,                "texture_size",                   4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_INVERSE_TEXTURE_SIZE,        "inverse_texture_size",           4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_PACKED_TEXTURE_SIZE,         "packed_texture_size",            4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_TEXTURE_MATRIX,  "texture_matrix", 16, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_LOD_CAMERA_POSITION,               "lod_camera_position",              3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_LOD_CAMERA_POSITION_OBJECT_SPACE,  "lod_camera_position_object_space", 3, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_LIGHT_CUSTOM,        "light_custom", 4, ET_REAL, ACDT_INT),
        AutoConstantDefinition(ACT_POINT_PARAMS,                    "point_params",                   4, ET_REAL, ACDT_NONE),

        // NOTE: new auto constants must be added before this line, as the following are merely aliases
        // to allow legacy world_ names in scripts
        AutoConstantDefinition(ACT_BONE_MATRIX_ARRAY_3x4,        "world_matrix_array_3x4",      12, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_MATRIX_ARRAY,            "world_matrix_array",          16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_DUALQUATERNION_ARRAY_2x4, "world_dualquaternion_array_2x4",      8, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_BONE_SCALE_SHEAR_MATRIX_ARRAY_3x4, "world_scale_shear_matrix_array_3x4", 12, ET_REAL, ACDT_NONE),
    };

    GpuNamedConstants::GpuNamedConstants() : bufferSize(0), registerCount(0) {}
    GpuNamedConstants::~GpuNamedConstants() {}

    GpuLogicalBufferStruct::GpuLogicalBufferStruct() : bufferSize(0) {}
    GpuLogicalBufferStruct::~GpuLogicalBufferStruct() {}

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
    //-----------------------------------------------------------------------------
    size_t GpuNamedConstants::calculateSize(void) const
    {
        size_t memSize = sizeof(*this);
        // Tally up constant defs
        memSize += sizeof(GpuConstantDefinition) * map.size();

        return memSize;
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
        DataStreamPtr stream = _openFileStream(filename, std::ios::binary | std::ios::out);
        exportNamedConstants(pConsts, stream, endianMode);

        stream->close();
    }
    //---------------------------------------------------------------------
    void GpuNamedConstantsSerializer::exportNamedConstants(
        const GpuNamedConstants* pConsts, DataStreamPtr stream, Endian endianMode)
    {
        // Decide on endian mode
        determineEndianness(endianMode);

        mStream =stream;
        if (!stream->isWriteable())
        {
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE,
                        "Unable to write to stream " + stream->getName(),
                        "GpuNamedConstantsSerializer::exportNamedConstants");
        }

        writeFileHeader();

        writeInts(((const uint32*)&pConsts->bufferSize), 1);
        writeInts(((const uint32*)&pConsts->bufferSize), 1);
        writeInts(((const uint32*)&pConsts->bufferSize), 1);

        // simple export of all the named constants, no chunks
        // name, physical index
        for (const auto& i : pConsts->map)
        {
            const String& name = i.first;
            const GpuConstantDefinition& def = i.second;

            writeString(name);
            writeInts(((const uint32*)&def.physicalIndex), 1);
            writeInts(((const uint32*)&def.logicalIndex), 1);
            uint32 constType = static_cast<uint32>(def.constType);
            writeInts(&constType, 1);
            writeInts(((const uint32*)&def.elementSize), 1);
            writeInts(((const uint32*)&def.arraySize), 1);
        }

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

        readInts(stream, ((uint32*)&pDest->bufferSize), 1);
        readInts(stream, ((uint32*)&pDest->bufferSize), 1);
        readInts(stream, ((uint32*)&pDest->bufferSize), 1);

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
    //      GpuSharedParameters Methods
    //-----------------------------------------------------------------------------
    GpuSharedParameters::GpuSharedParameters(const String& name)
        :mName(name)
        , mVersion(0), mOffset(0), mDirty(false)
    {

    }
    //-----------------------------------------------------------------------------
    size_t GpuSharedParameters::calculateSize(void) const
    {
        size_t memSize = sizeof(*this);

        memSize += mConstants.size();
        memSize += mName.size() * sizeof(char);

        return memSize;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::addConstantDefinition(const String& name, GpuConstantType constType, uint32 arraySize)
    {
        if (mNamedConstants.map.find(name) != mNamedConstants.map.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Constant entry with name '" + name + "' already exists. ",
                        "GpuSharedParameters::addConstantDefinition");
        }
        GpuConstantDefinition def;
        def.arraySize = arraySize;
        def.constType = constType;

		// here, we do not consider padding, but rather alignment
        def.elementSize = GpuConstantDefinition::getElementSize(constType, false);

		// we try to adhere to GLSL std140 packing rules
		// handle alignment requirements
        size_t align_size = std::min<size_t>(def.elementSize == 3 ? 4 : def.elementSize, 4); // vec3 is 16 byte aligned, which is max
        align_size *= 4; // bytes

        size_t nextAlignedOffset = ((mOffset + align_size - 1) / align_size) * align_size;

        // abuse logical index to store offset
        if (mOffset + align_size > nextAlignedOffset)
        {
            def.logicalIndex = nextAlignedOffset;
        }
        else
        {
            def.logicalIndex = mOffset;
        }

        mOffset = def.logicalIndex + (def.constType == GCT_MATRIX_4X3 ? 16 : def.elementSize) * 4; // mat4x3 have a size of 64 bytes

        def.variability = (uint16)GPV_GLOBAL;

        if (def.isFloat())
        {
            def.physicalIndex = mConstants.size();
            mConstants.resize(mConstants.size() + def.arraySize * def.elementSize*4);
        }
        else if (def.isDouble())
        {
            def.physicalIndex = mConstants.size();
            mConstants.resize(mConstants.size() + def.arraySize * def.elementSize*8);
        }
        else if (def.isInt() || def.isUnsignedInt() || def.isBool())
        {
            def.physicalIndex = mConstants.size();
            mConstants.resize(mConstants.size() + def.arraySize * def.elementSize*4);
        }
        else 
        {
            //FIXME Is this the right exception type?
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Constant entry with name '" + name + "' is not a known type.",
                        "GpuSharedParameters::addConstantDefinition");
        }

        mNamedConstants.map[name] = def;

        ++mVersion;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::removeConstantDefinition(const String& name)
    {
        GpuConstantDefinitionMap::iterator i = mNamedConstants.map.find(name);
        if (i != mNamedConstants.map.end())
        {
            GpuConstantDefinition& def = i->second;
            size_t numElems = def.elementSize * def.arraySize;

            for (auto& j : mNamedConstants.map)
            {
                GpuConstantDefinition& otherDef = j.second;
                // comes after in the buffer
                if (otherDef.physicalIndex > def.physicalIndex)
                {
                    // adjust index
                    otherDef.logicalIndex -= numElems;
                    otherDef.physicalIndex -= numElems;
                }
            }

            // remove and reduce buffer
            if (def.isFloat() || def.isInt() || def.isUnsignedInt() || def.isBool())
            {
                mNamedConstants.bufferSize -= numElems;

                ConstantList::iterator beg = mConstants.begin();
                std::advance(beg, def.physicalIndex);
                ConstantList::iterator en = beg;
                std::advance(en, numElems*4);
                mConstants.erase(beg, en);
            }
            else {
                //TODO exception handling
            }

            ++mVersion;
        }

    }

    void GpuSharedParameters::_upload() const
    {
        OgreAssert(mHardwareBuffer, "not backed by a HardwareBuffer");

        if (!mDirty)
            return;

        mHardwareBuffer->writeData(0, mConstants.size(), mConstants.data());
    }
    void GpuSharedParameters::download()
    {
        OgreAssert(mHardwareBuffer, "not backed by a HardwareBuffer");
        mHardwareBuffer->readData(0, mConstants.size(), mConstants.data());
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::removeAllConstantDefinitions()
    {
		mOffset = 0;
        mNamedConstants.map.clear();
        mNamedConstants.bufferSize = 0;
        mConstants.clear();
    }
    //---------------------------------------------------------------------
    GpuConstantDefinitionIterator GpuSharedParameters::getConstantDefinitionIterator(void) const
    {
        return GpuConstantDefinitionIterator(mNamedConstants.map.begin(), mNamedConstants.map.end());
    }
    //---------------------------------------------------------------------
    const GpuConstantDefinition& GpuSharedParameters::getConstantDefinition(const String& name) const
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i == mNamedConstants.map.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Constant entry with name '" + name + "' does not exist. ",
                        "GpuSharedParameters::getConstantDefinition");
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    const GpuNamedConstants& GpuSharedParameters::getConstantDefinitions() const
    {
        return mNamedConstants;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Matrix4& m)
    {
        _setNamedConstant(name, m[0], 16);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Matrix4* m, uint32 numEntries)
    {
        _setNamedConstant(name, m[0][0], 16 * numEntries);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const ColourValue& colour)
    {
        _setNamedConstant(name, colour.ptr(), 4);
    }
    //---------------------------------------------------------------------
    template <typename T> void GpuSharedParameters::_setNamedConstant(const String& name, const T* val, uint32 count)
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i == mNamedConstants.map.end())
            return; // ignore

        const GpuConstantDefinition& def = i->second;
        memcpy(&mConstants[def.physicalIndex], val, sizeof(float) * std::min(count, def.elementSize * def.arraySize));
        _markDirty();
    }
    void GpuSharedParameters::setNamedConstant(const String& name, const float* val, uint32 count)
    {
        _setNamedConstant(name, val, count);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const double* val, uint32 count)
    {
        _setNamedConstant(name, val, count);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const int* val, uint32 count)
    {
        _setNamedConstant(name, val, count);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const uint* val, uint32 count)
    {
        _setNamedConstant(name, val, count);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::_markClean()
    {
        mDirty = false;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::_markDirty()
    {
        mDirty = true;
    }
    

    //-----------------------------------------------------------------------------
    //      GpuSharedParametersUsage Methods
    //-----------------------------------------------------------------------------
    GpuSharedParametersUsage::GpuSharedParametersUsage(GpuSharedParametersPtr sharedParams,
                                                       GpuProgramParameters* params)
        : mSharedParams(sharedParams)
        , mParams(params)
    {
        initCopyData();
    }
    //---------------------------------------------------------------------
    void GpuSharedParametersUsage::initCopyData()
    {

        mCopyDataList.clear();

        const GpuConstantDefinitionMap& sharedmap = mSharedParams->getConstantDefinitions().map;
        for (const auto& i : sharedmap)
        {
            const String& pName = i.first;
            const GpuConstantDefinition& shareddef = i.second;

            const GpuConstantDefinition* instdef = mParams->_findNamedConstantDefinition(pName, false);
            if (instdef)
            {
                // Check that the definitions are the same
                if (instdef->constType == shareddef.constType &&
                    instdef->arraySize <= shareddef.arraySize)
                {
                    CopyDataEntry e;
                    e.srcDefinition = &shareddef;
                    e.dstDefinition = instdef;
                    mCopyDataList.push_back(e);
                }
                else
                    LogManager::getSingleton().logWarning("cannot copy shared parameter '" + pName +
                                                          "' - type or variability mismatch");
            }
        }

        mCopyDataVersion = mSharedParams->getVersion();
    }
    //---------------------------------------------------------------------
    void GpuSharedParametersUsage::_copySharedParamsToTargetParams() const
    {
        // check copy data version
        if (mCopyDataVersion != mSharedParams->getVersion())
            const_cast<GpuSharedParametersUsage*>(this)->initCopyData();

        // force const call to get*Pointer
        const GpuSharedParameters* sharedParams = mSharedParams.get();

        for (const CopyDataEntry& e : mCopyDataList)
        {
            if (e.dstDefinition->isFloat())
            {
                const float* pSrc = sharedParams->getFloatPointer(e.srcDefinition->physicalIndex);
                float* pDst = mParams->getFloatPointer(e.dstDefinition->physicalIndex);

                // Deal with matrix transposition here!!!
                // transposition is specific to the dest param set, shared params don't do it
                if (mParams->getTransposeMatrices() && (e.dstDefinition->constType == GCT_MATRIX_4X4))
                {
                    // for each matrix that needs to be transposed and copied,
                    for (size_t iMat = 0; iMat < e.dstDefinition->arraySize; ++iMat)
                    {
                        for (int row = 0; row < 4; ++row)
                            for (int col = 0; col < 4; ++col)
                                pDst[row * 4 + col] = pSrc[col * 4 + row];
                        pSrc += 16;
                        pDst += 16;
                    }
                }
                else
                {
                    if (e.dstDefinition->elementSize == e.srcDefinition->elementSize)
                    {
                        // simple copy
                        memcpy(pDst, pSrc, sizeof(float) * e.dstDefinition->elementSize * e.dstDefinition->arraySize);
                    }
                    else
                    {
                        // target params may be padded to 4 elements, shared params are packed
                        assert(e.dstDefinition->elementSize % 4 == 0);
                        size_t iterations = e.dstDefinition->elementSize / 4
                            * e.dstDefinition->arraySize;
                        assert(iterations > 0);
                        size_t valsPerIteration = e.srcDefinition->elementSize;
                        for (size_t l = 0; l < iterations; ++l)
                        {
                            memcpy(pDst, pSrc, sizeof(float) * valsPerIteration);
                            pSrc += valsPerIteration;
                            pDst += 4;
                        }
                    }
                }
            }
            else if (e.dstDefinition->isDouble())
            {
                const double* pSrc = sharedParams->getDoublePointer(e.srcDefinition->physicalIndex);
                double* pDst = mParams->getDoublePointer(e.dstDefinition->physicalIndex);

                // Deal with matrix transposition here!!!
                // transposition is specific to the dest param set, shared params don't do it
                if (mParams->getTransposeMatrices() && (e.dstDefinition->constType == GCT_MATRIX_DOUBLE_4X4))
                {
                    // for each matrix that needs to be transposed and copied,
                    for (size_t iMat = 0; iMat < e.dstDefinition->arraySize; ++iMat)
                    {
                        for (int row = 0; row < 4; ++row)
                            for (int col = 0; col < 4; ++col)
                                pDst[row * 4 + col] = pSrc[col * 4 + row];
                        pSrc += 16;
                        pDst += 16;
                    }
                }
                else
                {
                    if (e.dstDefinition->elementSize == e.srcDefinition->elementSize)
                    {
                        // simple copy
                        memcpy(pDst, pSrc, sizeof(double) * e.dstDefinition->elementSize * e.dstDefinition->arraySize);
                    }
                    else
                    {
                        // target params may be padded to 4 elements, shared params are packed
                        assert(e.dstDefinition->elementSize % 4 == 0);
                        size_t iterations = e.dstDefinition->elementSize / 4
                            * e.dstDefinition->arraySize;
                        assert(iterations > 0);
                        size_t valsPerIteration = e.srcDefinition->elementSize;
                        for (size_t l = 0; l < iterations; ++l)
                        {
                            memcpy(pDst, pSrc, sizeof(double) * valsPerIteration);
                            pSrc += valsPerIteration;
                            pDst += 4;
                        }
                    }
                }
            }
            else if (e.dstDefinition->isInt())
            {
                const int* pSrc = sharedParams->getIntPointer(e.srcDefinition->physicalIndex);
                int* pDst = mParams->getIntPointer(e.dstDefinition->physicalIndex);

                if (e.dstDefinition->elementSize == e.srcDefinition->elementSize)
                {
                    // simple copy
                    memcpy(pDst, pSrc, sizeof(int) * e.dstDefinition->elementSize * e.dstDefinition->arraySize);
                }
                else
                {
                    // target params may be padded to 4 elements, shared params are packed
                    assert(e.dstDefinition->elementSize % 4 == 0);
                    size_t iterations = (e.dstDefinition->elementSize / 4)
                        * e.dstDefinition->arraySize;
                    assert(iterations > 0);
                    size_t valsPerIteration = e.srcDefinition->elementSize;
                    for (size_t l = 0; l < iterations; ++l)
                    {
                        memcpy(pDst, pSrc, sizeof(int) * valsPerIteration);
                        pSrc += valsPerIteration;
                        pDst += 4;
                    }
                }
            }
            else if (e.dstDefinition->isUnsignedInt() || e.dstDefinition->isBool()) 
            {
                const uint* pSrc = sharedParams->getUnsignedIntPointer(e.srcDefinition->physicalIndex);
                uint* pDst = mParams->getUnsignedIntPointer(e.dstDefinition->physicalIndex);

                if (e.dstDefinition->elementSize == e.srcDefinition->elementSize)
                {
                    // simple copy
                    memcpy(pDst, pSrc, sizeof(uint) * e.dstDefinition->elementSize * e.dstDefinition->arraySize);
                }
                else
                {
                    // target params may be padded to 4 elements, shared params are packed
                    assert(e.dstDefinition->elementSize % 4 == 0);
                    size_t iterations = (e.dstDefinition->elementSize / 4)
                        * e.dstDefinition->arraySize;
                    assert(iterations > 0);
                    size_t valsPerIteration = e.srcDefinition->elementSize;
                    for (size_t l = 0; l < iterations; ++l)
                    {
                        memcpy(pDst, pSrc, sizeof(uint) * valsPerIteration);
                        pSrc += valsPerIteration;
                        pDst += 4;
                    }
                }
            }
            else {
                //TODO add error
            }
        }
    }



    //-----------------------------------------------------------------------------
    //      GpuProgramParameters Methods
    //-----------------------------------------------------------------------------
    GpuProgramParameters::GpuProgramParameters() :
        mCombinedVariability(GPV_GLOBAL)
        , mTransposeMatrices(false)
        , mIgnoreMissingParams(false)
        , mActivePassIterationIndex(std::numeric_limits<size_t>::max())
    {
    }
    GpuProgramParameters::~GpuProgramParameters() {}

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
        mConstants = oth.mConstants;
        mRegisters = oth.mRegisters;
        mAutoConstants = oth.mAutoConstants;
        mLogicalToPhysical = oth.mLogicalToPhysical;
        mNamedConstants = oth.mNamedConstants;
        copySharedParamSetUsage(oth.mSharedParamSets);

        mCombinedVariability = oth.mCombinedVariability;
        mTransposeMatrices = oth.mTransposeMatrices;
        mIgnoreMissingParams  = oth.mIgnoreMissingParams;
        mActivePassIterationIndex = oth.mActivePassIterationIndex;

        return *this;
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::copySharedParamSetUsage(const GpuSharedParamUsageList& srcList)
    {
        mSharedParamSets.clear();
        for (const auto& i : srcList)
        {
            mSharedParamSets.push_back(GpuSharedParametersUsage(i.getSharedParams(), this));
        }

    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::calculateSize(void) const
    {
        size_t memSize = sizeof(*this);

        memSize += mConstants.size();
        memSize += mRegisters.size()*4;

        for (const auto& ac : mAutoConstants)
        {
            memSize += sizeof(ac);
        }

        return memSize;
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::_setNamedConstants(
        const GpuNamedConstantsPtr& namedConstants)
    {
        mNamedConstants = namedConstants;

        // Determine any extension to local buffers

        // Size and reset buffer (fill with zero to make comparison later ok)
        if (namedConstants->bufferSize*4 > mConstants.size())
        {
            mConstants.insert(mConstants.end(), namedConstants->bufferSize * 4 - mConstants.size(), 0);
        }

        if(namedConstants->registerCount > mRegisters.size())
            mRegisters.insert(mRegisters.end(), namedConstants->registerCount - mRegisters.size(), 0);
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::_setLogicalIndexes(const GpuLogicalBufferStructPtr& indexMap)
    {
        mLogicalToPhysical = indexMap;

        // resize the internal buffers
        // Note that these will only contain something after the first parameter
        // set has set some parameters

        // Size and reset buffer (fill with zero to make comparison later ok)
        if (indexMap && indexMap->bufferSize*4 > mConstants.size())
        {
            mConstants.insert(mConstants.end(), indexMap->bufferSize * 4 - mConstants.size(), 0);
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
    void GpuProgramParameters::setConstant(size_t index, const Vector2& vec)
    {
        setConstant(index, Vector4(vec.x, vec.y, 1.0f, 1.0f));
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
        assert(mLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getConstantPhysicalIndex(index, rawCount, GPV_GLOBAL, BCT_FLOAT);

        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const double *val, size_t count)
    {
        // Raw buffer size is 4x count
        size_t rawCount = count * 4;
        // get physical index
        assert(mLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getConstantPhysicalIndex(index, rawCount, GPV_GLOBAL, BCT_DOUBLE);
        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const int *val, size_t count)
    {
        // Raw buffer size is 4x count
        size_t rawCount = count * 4;
        // get physical index
        assert(mLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getConstantPhysicalIndex(index, rawCount, GPV_GLOBAL, BCT_INT);
        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const uint *val, size_t count)
    {
        // Raw buffer size is 4x count
        size_t rawCount = count * 4;
        // get physical index
        assert(mLogicalToPhysical && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getConstantPhysicalIndex(index, rawCount, GPV_GLOBAL, BCT_INT);
        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Matrix4& m, size_t elementCount)
    {

        // remember, raw content access uses raw float count rather than float4
        if (mTransposeMatrices)
        {
            Matrix4 t = m.transpose();
            _writeRawConstants(physicalIndex, t[0], elementCount > 16 ? 16 : elementCount);
        }
        else
        {
            _writeRawConstants(physicalIndex, m[0], elementCount > 16 ? 16 : elementCount);
        }

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Matrix3& m, size_t elementCount)
    {

        // remember, raw content access uses raw float count rather than float4
        if (mTransposeMatrices)
        {
            Matrix3 t = m.transpose();
            _writeRawConstants(physicalIndex, t[0], elementCount > 9 ? 9 : elementCount);
        }
        else
        {
            _writeRawConstants(physicalIndex, m[0], elementCount > 9 ? 9 : elementCount);
        }

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const TransformBaseReal* pMatrix, size_t numEntries)
    {
        // remember, raw content access uses raw float count rather than float4
        if (mTransposeMatrices)
        {
            for (size_t i = 0; i < numEntries; ++i)
            {
                Matrix4 t = pMatrix[i].transpose();
                _writeRawConstants(physicalIndex, t[0], 16);
                physicalIndex += 16*sizeof(Real);
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
        assert(physicalIndex + sizeof(float) * count <= mConstants.size());
        for (size_t i = 0; i < count; ++i)
        {
            float tmp = val[i];
            memcpy(&mConstants[physicalIndex + i * sizeof(float)], &tmp, sizeof(float));
        }
    }
    void GpuProgramParameters::_writeRegisters(size_t index, const int* val, size_t count)
    {
        assert(index + count <= mRegisters.size());
        memcpy(&mRegisters[index], val, sizeof(int) * count);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_readRawConstants(size_t physicalIndex, size_t count, float* dest)
    {
        assert(physicalIndex + count <= mConstants.size());
        memcpy(dest, &mConstants[physicalIndex], sizeof(float) * count);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_readRawConstants(size_t physicalIndex, size_t count, int* dest)
    {
        assert(physicalIndex + count <= mConstants.size());
        memcpy(dest, &mConstants[physicalIndex], sizeof(int) * count);
    }
    //---------------------------------------------------------------------
    uint16 GpuProgramParameters::deriveVariability(GpuProgramParameters::AutoConstantType act)
    {
        switch(act)
        {
        case ACT_VIEW_MATRIX:
        case ACT_INVERSE_VIEW_MATRIX:
        case ACT_TRANSPOSE_VIEW_MATRIX:
        case ACT_INVERSE_TRANSPOSE_VIEW_MATRIX:
        case ACT_PROJECTION_MATRIX:
        case ACT_INVERSE_PROJECTION_MATRIX:
        case ACT_TRANSPOSE_PROJECTION_MATRIX:
        case ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX:
        case ACT_VIEWPROJ_MATRIX:
        case ACT_INVERSE_VIEWPROJ_MATRIX:
        case ACT_TRANSPOSE_VIEWPROJ_MATRIX:
        case ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX:
        case ACT_RENDER_TARGET_FLIPPING:
        case ACT_VERTEX_WINDING:
        case ACT_AMBIENT_LIGHT_COLOUR:
        case ACT_DERIVED_AMBIENT_LIGHT_COLOUR:
        case ACT_DERIVED_SCENE_COLOUR:
        case ACT_FOG_COLOUR:
        case ACT_FOG_PARAMS:
        case ACT_SURFACE_AMBIENT_COLOUR:
        case ACT_SURFACE_DIFFUSE_COLOUR:
        case ACT_SURFACE_SPECULAR_COLOUR:
        case ACT_SURFACE_EMISSIVE_COLOUR:
        case ACT_SURFACE_SHININESS:
        case ACT_SURFACE_ALPHA_REJECTION_VALUE:
        case ACT_CAMERA_POSITION:
        case ACT_CAMERA_RELATIVE_POSITION:
        case ACT_TIME:
        case ACT_TIME_0_X:
        case ACT_COSTIME_0_X:
        case ACT_SINTIME_0_X:
        case ACT_TANTIME_0_X:
        case ACT_TIME_0_X_PACKED:
        case ACT_TIME_0_1:
        case ACT_COSTIME_0_1:
        case ACT_SINTIME_0_1:
        case ACT_TANTIME_0_1:
        case ACT_TIME_0_1_PACKED:
        case ACT_TIME_0_2PI:
        case ACT_COSTIME_0_2PI:
        case ACT_SINTIME_0_2PI:
        case ACT_TANTIME_0_2PI:
        case ACT_TIME_0_2PI_PACKED:
        case ACT_FRAME_TIME:
        case ACT_FPS:
        case ACT_VIEWPORT_WIDTH:
        case ACT_VIEWPORT_HEIGHT:
        case ACT_INVERSE_VIEWPORT_WIDTH:
        case ACT_INVERSE_VIEWPORT_HEIGHT:
        case ACT_VIEWPORT_SIZE:
        case ACT_TEXEL_OFFSETS:
        case ACT_TEXTURE_SIZE:
        case ACT_INVERSE_TEXTURE_SIZE:
        case ACT_PACKED_TEXTURE_SIZE:
        case ACT_SCENE_DEPTH_RANGE:
        case ACT_VIEW_DIRECTION:
        case ACT_VIEW_SIDE_VECTOR:
        case ACT_VIEW_UP_VECTOR:
        case ACT_FOV:
        case ACT_NEAR_CLIP_DISTANCE:
        case ACT_FAR_CLIP_DISTANCE:
        case ACT_PASS_NUMBER:
        case ACT_TEXTURE_MATRIX:
        case ACT_LOD_CAMERA_POSITION:

            return (uint16)GPV_GLOBAL;

        case ACT_WORLD_MATRIX:
        case ACT_INVERSE_WORLD_MATRIX:
        case ACT_TRANSPOSE_WORLD_MATRIX:
        case ACT_INVERSE_TRANSPOSE_WORLD_MATRIX:
        case ACT_WORLD_MATRIX_ARRAY_3x4:
        case ACT_WORLD_MATRIX_ARRAY:
        case ACT_WORLD_DUALQUATERNION_ARRAY_2x4:
        case ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4:
        case ACT_WORLDVIEW_MATRIX:
        case ACT_INVERSE_WORLDVIEW_MATRIX:
        case ACT_TRANSPOSE_WORLDVIEW_MATRIX:
        case ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX:
        case ACT_NORMAL_MATRIX:
        case ACT_WORLDVIEWPROJ_MATRIX:
        case ACT_INVERSE_WORLDVIEWPROJ_MATRIX:
        case ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
        case ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
        case ACT_CAMERA_POSITION_OBJECT_SPACE:
        case ACT_LOD_CAMERA_POSITION_OBJECT_SPACE:
        case ACT_CUSTOM:
        case ACT_ANIMATION_PARAMETRIC:

            return (uint16)GPV_PER_OBJECT;

        case ACT_LIGHT_POSITION_OBJECT_SPACE:
        case ACT_LIGHT_DIRECTION_OBJECT_SPACE:
        case ACT_LIGHT_DISTANCE_OBJECT_SPACE:
        case ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY:
        case ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY:
        case ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY:
        case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX:
        case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY:
        case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX:
        case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY:
        case ACT_SHADOW_EXTRUSION_DISTANCE:

            // These depend on BOTH lights and objects
            return ((uint16)GPV_PER_OBJECT) | ((uint16)GPV_LIGHTS);

        case ACT_LIGHT_COUNT:
        case ACT_LIGHT_DIFFUSE_COLOUR:
        case ACT_LIGHT_SPECULAR_COLOUR:
        case ACT_LIGHT_POSITION:
        case ACT_LIGHT_DIRECTION:
        case ACT_LIGHT_POSITION_VIEW_SPACE:
        case ACT_LIGHT_DIRECTION_VIEW_SPACE:
        case ACT_SHADOW_SCENE_DEPTH_RANGE:
        case ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY:
        case ACT_SHADOW_COLOUR:
        case ACT_LIGHT_POWER_SCALE:
        case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED:
        case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED:
        case ACT_LIGHT_NUMBER:
        case ACT_LIGHT_CASTS_SHADOWS:
        case ACT_LIGHT_CASTS_SHADOWS_ARRAY:
        case ACT_LIGHT_ATTENUATION:
        case ACT_SPOTLIGHT_PARAMS:
        case ACT_LIGHT_DIFFUSE_COLOUR_ARRAY:
        case ACT_LIGHT_SPECULAR_COLOUR_ARRAY:
        case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY:
        case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY:
        case ACT_LIGHT_POSITION_ARRAY:
        case ACT_LIGHT_DIRECTION_ARRAY:
        case ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY:
        case ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY:
        case ACT_LIGHT_POWER_SCALE_ARRAY:
        case ACT_LIGHT_ATTENUATION_ARRAY:
        case ACT_SPOTLIGHT_PARAMS_ARRAY:
        case ACT_TEXTURE_VIEWPROJ_MATRIX:
        case ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY:
        case ACT_SPOTLIGHT_VIEWPROJ_MATRIX:
        case ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY:
        case ACT_LIGHT_CUSTOM:

            return (uint16)GPV_LIGHTS;

        case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR:
        case ACT_DERIVED_LIGHT_SPECULAR_COLOUR:
        case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY:
        case ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY:

            return ((uint16)GPV_GLOBAL | (uint16)GPV_LIGHTS);

        case ACT_PASS_ITERATION_NUMBER:

            return (uint16)GPV_PASS_ITERATION_NUMBER;

        default:
            return (uint16)GPV_GLOBAL;
        };

    }
    //---------------------------------------------------------------------
    GpuLogicalIndexUse* GpuProgramParameters::getConstantLogicalIndexUse(
        size_t logicalIndex, size_t requestedSize, uint16 variability, BaseConstantType type)
    {
        OgreAssert(type != BCT_SAMPLER, "");
        if (!mLogicalToPhysical)
            return NULL;

        GpuLogicalIndexUse* indexUse = 0;
        OGRE_LOCK_MUTEX(mLogicalToPhysical->mutex);

        auto logi = mLogicalToPhysical->map.find(logicalIndex);
        if (logi == mLogicalToPhysical->map.end())
        {
            if (requestedSize)
            {
                size_t physicalIndex = mConstants.size();

                // Expand at buffer end
                mConstants.insert(mConstants.end(), requestedSize*4, 0);

                // Record extended size for future GPU params re-using this information
                mLogicalToPhysical->bufferSize = mConstants.size()/4;

                // low-level programs will not know about mapping ahead of time, so
                // populate it. Other params objects will be able to just use this
                // accepted mapping since the constant structure will be the same

                // Set up a mapping for all items in the count
                size_t currPhys = physicalIndex;
                size_t count = requestedSize / 4;
                GpuLogicalIndexUseMap::iterator insertedIterator;

                for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
                {
                    GpuLogicalIndexUseMap::iterator it =
                            mLogicalToPhysical->map.emplace(
                                logicalIndex + logicalNum,
                                GpuLogicalIndexUse(currPhys, requestedSize, variability, type)).first;
                    currPhys += 4;

                    if (logicalNum == 0)
                        insertedIterator = it;
                }

                indexUse = &(insertedIterator->second);
            }
            else
            {
                // no match & ignore
                return 0;
            }

        }
        else
        {
            size_t physicalIndex = logi->second.physicalIndex;
            indexUse = &(logi->second);
            // check size
            if (logi->second.currentSize < requestedSize)
            {
                // init buffer entry wasn't big enough; could be a mistake on the part
                // of the original use, or perhaps a variable length we can't predict
                // until first actual runtime use e.g. world matrix array
                size_t insertCount = requestedSize - logi->second.currentSize;
                auto insertPos = mConstants.begin();
                std::advance(insertPos, physicalIndex);
                mConstants.insert(insertPos, insertCount*4, 0);

                // shift all physical positions after this one
                for (auto& p : mLogicalToPhysical->map)
                {
                    if (p.second.physicalIndex > physicalIndex)
                        p.second.physicalIndex += insertCount*4;
                }
                mLogicalToPhysical->bufferSize += insertCount;
                for (auto& ac : mAutoConstants)
                {
                    auto def = getAutoConstantDefinition(ac.paramType);
                    if (ac.physicalIndex > physicalIndex && def)
                    {
                        ac.physicalIndex += insertCount*4;
                    }
                }
                if (mNamedConstants)
                {
                    for (auto& p : mNamedConstants->map)
                    {
                        if (p.second.physicalIndex > physicalIndex)
                            p.second.physicalIndex += insertCount * 4;
                    }
                    mNamedConstants->bufferSize += insertCount;
                }

                logi->second.currentSize += insertCount;
            }
        }

        if (indexUse && requestedSize)
            indexUse->variability = variability;

        return indexUse;
    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::_getConstantPhysicalIndex(
        size_t logicalIndex, size_t requestedSize, uint16 variability, BaseConstantType type)
    {
        GpuLogicalIndexUse* indexUse = getConstantLogicalIndexUse(logicalIndex, requestedSize, variability, type);
        return indexUse ? indexUse->physicalIndex : 0;
    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::getLogicalIndexForPhysicalIndex(size_t physicalIndex)
    {
        // perhaps build a reverse map of this sometime (shared in GpuProgram)
        for (const auto& p : mLogicalToPhysical->map)
        {
            if (p.second.physicalIndex == physicalIndex)
                return p.first;
        }
        return std::numeric_limits<size_t>::max();
    }
    //-----------------------------------------------------------------------------
    GpuConstantDefinitionIterator GpuProgramParameters::getConstantDefinitionIterator(void) const
    {
        if (!mNamedConstants)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Named constants have not been initialised, perhaps a compile error");

        return GpuConstantDefinitionIterator(mNamedConstants->map.begin(),
                                             mNamedConstants->map.end());

    }
    //-----------------------------------------------------------------------------
    const GpuNamedConstants& GpuProgramParameters::getConstantDefinitions() const
    {
        if (!mNamedConstants)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Named constants have not been initialised, perhaps a compile error");

        return *mNamedConstants;
    }
    //-----------------------------------------------------------------------------
    const GpuConstantDefinition& GpuProgramParameters::getConstantDefinition(const String& name) const
    {
        if (!mNamedConstants)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Named constants have not been initialised, perhaps a compile error",
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
                            "Named constants have not been initialised, perhaps a compile error");
            return 0;
        }

        // strip array extension
        size_t arrStart = name.back() == ']' ? name.find('[') : String::npos;
        auto i = mNamedConstants->map.find(arrStart == String::npos ? name : name.substr(0, arrStart));
        if (i == mNamedConstants->map.end() || (i->second.arraySize == 1 && arrStart != String::npos))
        {
            if (throwExceptionIfNotFound)
			{
				String knownNames;
#if OGRE_DEBUG_MODE
				// make it easy to catch typo and/or unused shader parameter elimination made by some drivers
				knownNames = "Known names are: ";
				for (i = mNamedConstants->map.begin(); i != mNamedConstants->map.end(); ++i)
					knownNames.append(i->first).append(" ");
#endif
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
				"Parameter called " + name + " does not exist. " + knownNames,
                            "GpuProgramParameters::_findNamedConstantDefinition");
			}
            return 0;
        }
        else
        {
            return &(i->second);
        }
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstant(size_t index, AutoConstantType acType, uint32 extraInfo)
    {
        // Get auto constant definition for sizing
        const AutoConstantDefinition* autoDef = getAutoConstantDefinition(acType);

        if(!autoDef)
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No constant definition found for type " +
                        StringConverter::toString(acType),
                        "GpuProgramParameters::setAutoConstant");

        // round up to nearest multiple of 4
        size_t sz = autoDef->elementCount;
        if (sz % 4 > 0)
        {
            sz += 4 - (sz % 4);
        }

        GpuLogicalIndexUse* indexUse = getConstantLogicalIndexUse(index, sz, deriveVariability(acType), BCT_FLOAT);

        if(indexUse)
            _setRawAutoConstant(indexUse->physicalIndex, acType, extraInfo, indexUse->variability, sz);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_setRawAutoConstant(size_t physicalIndex,
                                                   AutoConstantType acType, uint32 extraInfo, uint16 variability, uint8 elementSize)
    {
        // update existing index if it exists
        bool found = false;
        for (auto& ac : mAutoConstants)
        {
            if (ac.physicalIndex == physicalIndex)
            {
                ac.paramType = acType;
                ac.data = extraInfo;
                ac.elementCount = elementSize;
                ac.variability = variability;
                found = true;
                break;
            }
        }
        if (!found)
            mAutoConstants.push_back(AutoConstantEntry(acType, physicalIndex, extraInfo, variability, elementSize));

        mCombinedVariability |= variability;


    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_setRawAutoConstantReal(size_t physicalIndex,
                                                       AutoConstantType acType, float rData, uint16 variability, uint8 elementSize)
    {
        // update existing index if it exists
        bool found = false;
        for (auto& ac : mAutoConstants)
        {
            if (ac.physicalIndex == physicalIndex)
            {
                ac.paramType = acType;
                ac.fData = rData;
                ac.elementCount = elementSize;
                ac.variability = variability;
                found = true;
                break;
            }
        }
        if (!found)
            mAutoConstants.push_back(AutoConstantEntry(acType, physicalIndex, rData, variability, elementSize));

        mCombinedVariability |= variability;
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::clearAutoConstant(size_t index)
    {
        GpuLogicalIndexUse* indexUse = getConstantLogicalIndexUse(index, 0, GPV_GLOBAL, BCT_FLOAT);

        if (indexUse)
        {
            // do not clear variability, indexUse is shared across all instances

            size_t physicalIndex = indexUse->physicalIndex;
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
            // do not clear variability, def is shared across all instances

            // Autos are always floating point
            if (def->isFloat()) {
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
        mCombinedVariability = GPV_GLOBAL;
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstantReal(size_t index, AutoConstantType acType, float rData)
    {
        // Get auto constant definition for sizing
        const AutoConstantDefinition* autoDef = getAutoConstantDefinition(acType);

        if(!autoDef)
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No constant definition found for type " +
                        StringConverter::toString(acType),
                        "GpuProgramParameters::setAutoConstantReal");

        // round up to nearest multiple of 4
        size_t sz = autoDef->elementCount;
        if (sz % 4 > 0)
        {
            sz += 4 - (sz % 4);
        }

        GpuLogicalIndexUse* indexUse = getConstantLogicalIndexUse(index, sz, deriveVariability(acType), BCT_FLOAT);

        _setRawAutoConstantReal(indexUse->physicalIndex, acType, rData, indexUse->variability, sz);
    }
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_updateAutoParams(const AutoParamDataSource* source, uint16 mask)
    {
        // abort early if no autos
        if (!hasAutoConstants()) return;
        // abort early if variability doesn't match any param
        if (!(mask & mCombinedVariability))
            return;

        size_t index;
        size_t numMatrices;
        const Affine3* pMatrix;
        size_t m;
        Vector3 vec3;
        Vector4 vec4;
        Matrix3 m3;
        Matrix4 scaleM;
        DualQuaternion dQuat;

        mActivePassIterationIndex = std::numeric_limits<size_t>::max();

        // Autoconstant index is not a physical index
        for (const auto& ac : mAutoConstants)
        {
            // Only update needed slots
            if (ac.variability & mask)
            {

                switch(ac.paramType)
                {
                case ACT_VIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getViewMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_VIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseViewMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_VIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeViewMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_VIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeViewMatrix(),ac.elementCount);
                    break;

                case ACT_PROJECTION_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getProjectionMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_PROJECTION_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseProjectionMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_PROJECTION_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeProjectionMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeProjectionMatrix(),ac.elementCount);
                    break;

                case ACT_VIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getViewProjectionMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_RENDER_TARGET_FLIPPING:
                    _writeRawConstant(ac.physicalIndex, source->getCurrentRenderTarget()->requiresTextureFlipping() ? -1.f : +1.f);
                    break;
                case ACT_VERTEX_WINDING:
                    {
                        RenderSystem* rsys = Root::getSingleton().getRenderSystem();
                        _writeRawConstant(ac.physicalIndex, rsys->getInvertVertexWinding() ? -1.f : +1.f);
                    }
                    break;

                    // NB ambient light still here because it's not related to a specific light
                case ACT_AMBIENT_LIGHT_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getAmbientLightColour(),
                                      ac.elementCount);
                    break;
                case ACT_DERIVED_AMBIENT_LIGHT_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getDerivedAmbientLightColour(),
                                      ac.elementCount);
                    break;
                case ACT_DERIVED_SCENE_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getDerivedSceneColour(),
                                      ac.elementCount);
                    break;

                case ACT_FOG_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getFogColour(), ac.elementCount);
                    break;
                case ACT_FOG_PARAMS:
                    _writeRawConstant(ac.physicalIndex, source->getFogParams(), ac.elementCount);
                    break;
                case ACT_POINT_PARAMS:
                    _writeRawConstant(ac.physicalIndex, source->getPointParams(), ac.elementCount);
                    break;
                case ACT_SURFACE_AMBIENT_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceAmbientColour(),
                                      ac.elementCount);
                    break;
                case ACT_SURFACE_DIFFUSE_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceDiffuseColour(),
                                      ac.elementCount);
                    break;
                case ACT_SURFACE_SPECULAR_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceSpecularColour(),
                                      ac.elementCount);
                    break;
                case ACT_SURFACE_EMISSIVE_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceEmissiveColour(),
                                      ac.elementCount);
                    break;
                case ACT_SURFACE_SHININESS:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceShininess());
                    break;
                case ACT_SURFACE_ALPHA_REJECTION_VALUE:
                    _writeRawConstant(ac.physicalIndex, source->getSurfaceAlphaRejectionValue());
                    break;

                case ACT_CAMERA_POSITION:
                    _writeRawConstant(ac.physicalIndex, source->getCameraPosition(), ac.elementCount);
                    break;
                case ACT_CAMERA_RELATIVE_POSITION:
                    _writeRawConstant (ac.physicalIndex, source->getCameraRelativePosition(), ac.elementCount);
                    break;
                case ACT_TIME:
                    _writeRawConstant(ac.physicalIndex, source->getTime() * ac.fData);
                    break;
                case ACT_TIME_0_X:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_X(ac.fData));
                    break;
                case ACT_COSTIME_0_X:
                    _writeRawConstant(ac.physicalIndex, source->getCosTime_0_X(ac.fData));
                    break;
                case ACT_SINTIME_0_X:
                    _writeRawConstant(ac.physicalIndex, source->getSinTime_0_X(ac.fData));
                    break;
                case ACT_TANTIME_0_X:
                    _writeRawConstant(ac.physicalIndex, source->getTanTime_0_X(ac.fData));
                    break;
                case ACT_TIME_0_X_PACKED:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_X_packed(ac.fData), ac.elementCount);
                    break;
                case ACT_TIME_0_1:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_1(ac.fData));
                    break;
                case ACT_COSTIME_0_1:
                    _writeRawConstant(ac.physicalIndex, source->getCosTime_0_1(ac.fData));
                    break;
                case ACT_SINTIME_0_1:
                    _writeRawConstant(ac.physicalIndex, source->getSinTime_0_1(ac.fData));
                    break;
                case ACT_TANTIME_0_1:
                    _writeRawConstant(ac.physicalIndex, source->getTanTime_0_1(ac.fData));
                    break;
                case ACT_TIME_0_1_PACKED:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_1_packed(ac.fData), ac.elementCount);
                    break;
                case ACT_TIME_0_2PI:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_2Pi(ac.fData));
                    break;
                case ACT_COSTIME_0_2PI:
                    _writeRawConstant(ac.physicalIndex, source->getCosTime_0_2Pi(ac.fData));
                    break;
                case ACT_SINTIME_0_2PI:
                    _writeRawConstant(ac.physicalIndex, source->getSinTime_0_2Pi(ac.fData));
                    break;
                case ACT_TANTIME_0_2PI:
                    _writeRawConstant(ac.physicalIndex, source->getTanTime_0_2Pi(ac.fData));
                    break;
                case ACT_TIME_0_2PI_PACKED:
                    _writeRawConstant(ac.physicalIndex, source->getTime_0_2Pi_packed(ac.fData), ac.elementCount);
                    break;
                case ACT_FRAME_TIME:
                    _writeRawConstant(ac.physicalIndex, source->getFrameTime() * ac.fData);
                    break;
                case ACT_FPS:
                    _writeRawConstant(ac.physicalIndex, source->getFPS());
                    break;
                case ACT_VIEWPORT_WIDTH:
                    _writeRawConstant(ac.physicalIndex, source->getViewportWidth());
                    break;
                case ACT_VIEWPORT_HEIGHT:
                    _writeRawConstant(ac.physicalIndex, source->getViewportHeight());
                    break;
                case ACT_INVERSE_VIEWPORT_WIDTH:
                    _writeRawConstant(ac.physicalIndex, source->getInverseViewportWidth());
                    break;
                case ACT_INVERSE_VIEWPORT_HEIGHT:
                    _writeRawConstant(ac.physicalIndex, source->getInverseViewportHeight());
                    break;
                case ACT_VIEWPORT_SIZE:
                    _writeRawConstant(ac.physicalIndex, Vector4f(
                        source->getViewportWidth(),
                        source->getViewportHeight(),
                        source->getInverseViewportWidth(),
                        source->getInverseViewportHeight()), ac.elementCount);
                    break;
                case ACT_TEXEL_OFFSETS:
                    {
                        RenderSystem* rsys = Root::getSingleton().getRenderSystem();
                        _writeRawConstant(ac.physicalIndex, Vector4f(
                            rsys->getHorizontalTexelOffset(),
                            rsys->getVerticalTexelOffset(),
                            rsys->getHorizontalTexelOffset() * source->getInverseViewportWidth(),
                            rsys->getVerticalTexelOffset() * source->getInverseViewportHeight()),
                                          ac.elementCount);
                    }
                    break;
                case ACT_TEXTURE_SIZE:
                    _writeRawConstant(ac.physicalIndex, source->getTextureSize(ac.data), ac.elementCount);
                    break;
                case ACT_INVERSE_TEXTURE_SIZE:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTextureSize(ac.data), ac.elementCount);
                    break;
                case ACT_PACKED_TEXTURE_SIZE:
                    _writeRawConstant(ac.physicalIndex, source->getPackedTextureSize(ac.data), ac.elementCount);
                    break;
                case ACT_SCENE_DEPTH_RANGE:
                    _writeRawConstant(ac.physicalIndex, source->getSceneDepthRange(), ac.elementCount);
                    break;
                case ACT_VIEW_DIRECTION:
                    _writeRawConstant(ac.physicalIndex, source->getViewDirection());
                    break;
                case ACT_VIEW_SIDE_VECTOR:
                    _writeRawConstant(ac.physicalIndex, source->getViewSideVector());
                    break;
                case ACT_VIEW_UP_VECTOR:
                    _writeRawConstant(ac.physicalIndex, source->getViewUpVector());
                    break;
                case ACT_FOV:
                    _writeRawConstant(ac.physicalIndex, source->getFOV());
                    break;
                case ACT_NEAR_CLIP_DISTANCE:
                    _writeRawConstant(ac.physicalIndex, source->getNearClipDistance());
                    break;
                case ACT_FAR_CLIP_DISTANCE:
                    _writeRawConstant(ac.physicalIndex, source->getFarClipDistance());
                    break;
                case ACT_PASS_NUMBER:
                    _writeRawConstant(ac.physicalIndex, (float)source->getPassNumber());
                    break;
                case ACT_PASS_ITERATION_NUMBER:
                    // this is actually just an initial set-up, it's bound separately, so still global
                    _writeRawConstant(ac.physicalIndex, 0.0f);
                    mActivePassIterationIndex = ac.physicalIndex;
                    break;
                case ACT_TEXTURE_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTextureTransformMatrix(ac.data),ac.elementCount);
                    break;
                case ACT_LOD_CAMERA_POSITION:
                    _writeRawConstant(ac.physicalIndex, source->getLodCameraPosition(), ac.elementCount);
                    break;

                case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX:
                    // can also be updated in lights
                    _writeRawConstant(ac.physicalIndex, source->getTextureWorldViewProjMatrix(ac.data),ac.elementCount);
                    break;
                case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Matrix4),
                                          source->getTextureWorldViewProjMatrix(l),ac.elementCount);
                    }
                    break;
                case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getSpotlightWorldViewProjMatrix(ac.data),ac.elementCount);
                    break;
                case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Matrix4), source->getSpotlightWorldViewProjMatrix(l), ac.elementCount);
                    break;
                case ACT_LIGHT_POSITION_OBJECT_SPACE:
                    _writeRawConstant(ac.physicalIndex,
                                      source->getInverseWorldMatrix() *
                                          source->getLightAs4DVector(ac.data),
                                      ac.elementCount);
                    break;
                case ACT_LIGHT_DIRECTION_OBJECT_SPACE:
                    // We need the inverse of the inverse transpose
                    m3 = source->getTransposeWorldMatrix().linear();
                    vec3 = m3 * source->getLightDirection(ac.data);
                    vec3.normalise();
                    // Set as 4D vector for compatibility
                    _writeRawConstant(ac.physicalIndex, Vector4f(vec3.x, vec3.y, vec3.z, 0.0f), ac.elementCount);
                    break;
                case ACT_LIGHT_DISTANCE_OBJECT_SPACE:
                    vec3 = source->getInverseWorldMatrix() * source->getLightPosition(ac.data);
                    _writeRawConstant(ac.physicalIndex, vec3.length());
                    break;
                case ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          source->getInverseWorldMatrix() *
                                              source->getLightAs4DVector(l),
                                          ac.elementCount);
                    break;

                case ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY:
                    // We need the inverse of the inverse transpose
                    m3 = source->getTransposeWorldMatrix().linear();
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        vec3 = m3 * source->getLightDirection(l);
                        vec3.normalise();
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          Vector4f(vec3.x, vec3.y, vec3.z, 0.0f), ac.elementCount);
                    }
                    break;

                case ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        vec3 = source->getInverseWorldMatrix() * source->getLightPosition(l);
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Real), vec3.length());
                    }
                    break;

                case ACT_WORLD_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getWorldMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_WORLD_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseWorldMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_WORLD_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeWorldMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_WORLD_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeWorldMatrix(),ac.elementCount);
                    break;

                case ACT_BONE_MATRIX_ARRAY_3x4:
                    // Loop over matrices
                    pMatrix = source->getBoneMatrixArray();
                    numMatrices = source->getBoneMatrixCount();
                    index = ac.physicalIndex;
                    for (m = 0; m < numMatrices; ++m)
                    {
                        _writeRawConstants(index, (*pMatrix)[0], 12);
                        index += 12*sizeof(Real);
                        ++pMatrix;
                    }
                    break;
                case ACT_BONE_MATRIX_ARRAY:
                    _writeRawConstant(ac.physicalIndex, source->getBoneMatrixArray(),
                                      source->getBoneMatrixCount());
                    break;
                case ACT_BONE_DUALQUATERNION_ARRAY_2x4:
                    // Loop over matrices
                    pMatrix = source->getBoneMatrixArray();
                    numMatrices = source->getBoneMatrixCount();
                    index = ac.physicalIndex;
                    for (m = 0; m < numMatrices; ++m)
                    {
                        dQuat.fromTransformationMatrix(*pMatrix);
                        _writeRawConstants(index, dQuat.ptr(), 8);
                        index += sizeof(DualQuaternion);
                        ++pMatrix;
                    }
                    break;
                case ACT_BONE_SCALE_SHEAR_MATRIX_ARRAY_3x4:
                    // Loop over matrices
                    pMatrix = source->getBoneMatrixArray();
                    numMatrices = source->getBoneMatrixCount();
                    index = ac.physicalIndex;

                    scaleM = Matrix4::IDENTITY;

                    for (m = 0; m < numMatrices; ++m)
                    {
                        //Based on Matrix4::decompostion, but we don't need the rotation or position components
                        //but do need the scaling and shearing. Shearing isn't available from Matrix4::decomposition
                        m3 = pMatrix->linear();

                        Matrix3 matQ;
                        Vector3 scale;

                        //vecU is the scaling component with vecU[0] = u01, vecU[1] = u02, vecU[2] = u12
                        //vecU[0] is shearing (x,y), vecU[1] is shearing (x,z), and vecU[2] is shearing (y,z)
                        //The first component represents the coordinate that is being sheared,
                        //while the second component represents the coordinate which performs the shearing.
                        Vector3 vecU;
                        m3.QDUDecomposition( matQ, scale, vecU );

                        scaleM[0][0] = scale.x;
                        scaleM[1][1] = scale.y;
                        scaleM[2][2] = scale.z;

                        scaleM[0][1] = vecU[0];
                        scaleM[0][2] = vecU[1];
                        scaleM[1][2] = vecU[2];

                        _writeRawConstants(index, scaleM[0], 12);
                        index += 12*sizeof(Real);
                        ++pMatrix;
                    }
                    break;
                case ACT_WORLDVIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getWorldViewMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseWorldViewMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeWorldViewMatrix(),ac.elementCount);
                    break;
                case ACT_NORMAL_MATRIX:
                    if(ac.elementCount == 9) // check if shader supports packed data
                    {
                        _writeRawConstant(ac.physicalIndex, source->getInverseTransposeWorldViewMatrix().linear(),ac.elementCount);
                        break;
                    }
                    OGRE_FALLTHROUGH; // fallthrough to padded 4x4 matrix
                case ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeWorldViewMatrix(),ac.elementCount);
                    break;

                case ACT_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getWorldViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseWorldViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getTransposeWorldViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getInverseTransposeWorldViewProjMatrix(),ac.elementCount);
                    break;
                case ACT_CAMERA_POSITION_OBJECT_SPACE:
                    _writeRawConstant(ac.physicalIndex, source->getCameraPositionObjectSpace(), ac.elementCount);
                    break;
                case ACT_LOD_CAMERA_POSITION_OBJECT_SPACE:
                    _writeRawConstant(ac.physicalIndex, source->getLodCameraPositionObjectSpace(), ac.elementCount);
                    break;

                case ACT_CUSTOM:
                case ACT_ANIMATION_PARAMETRIC:
                    source->getCurrentRenderable()->_updateCustomGpuParameter(ac, this);
                    break;
                case ACT_LIGHT_CUSTOM:
                    source->updateLightCustomGpuParameter(ac, this);
                    break;
                case ACT_LIGHT_COUNT:
                    _writeRawConstant(ac.physicalIndex, source->getLightCount());
                    break;
                case ACT_LIGHT_DIFFUSE_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getLightDiffuseColour(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_SPECULAR_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getLightSpecularColour(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_POSITION:
                    // Get as 4D vector, works for directional lights too
                    // Use element count in case uniform slot is smaller
                    _writeRawConstant(ac.physicalIndex,
                                      source->getLightAs4DVector(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_DIRECTION:
                    vec3 = source->getLightDirection(ac.data);
                    // Set as 4D vector for compatibility
                    // Use element count in case uniform slot is smaller
                    _writeRawConstant(ac.physicalIndex, Vector4f(vec3.x, vec3.y, vec3.z, 1.0f), ac.elementCount);
                    break;
                case ACT_LIGHT_POSITION_VIEW_SPACE:
                    _writeRawConstant(ac.physicalIndex,
                                      source->getViewMatrix() * source->getLightAs4DVector(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_DIRECTION_VIEW_SPACE:
                    m3 = source->getInverseTransposeViewMatrix().linear();
                    // inverse transpose in case of scaling
                    vec3 = m3 * source->getLightDirection(ac.data);
                    vec3.normalise();
                    // Set as 4D vector for compatibility
                    _writeRawConstant(ac.physicalIndex, Vector4f(vec3.x, vec3.y, vec3.z, 0.0f),ac.elementCount);
                    break;
                case ACT_SHADOW_EXTRUSION_DISTANCE:
                    // extrusion is in object-space, so we have to rescale by the inverse
                    // of the world scaling to deal with scaled objects
                    m3 = source->getWorldMatrix().linear();
                    _writeRawConstant(ac.physicalIndex, source->getShadowExtrusionDistance() /
                                      Math::Sqrt(std::max(std::max(m3.GetColumn(0).squaredLength(), m3.GetColumn(1).squaredLength()), m3.GetColumn(2).squaredLength())));
                    break;
                case ACT_SHADOW_SCENE_DEPTH_RANGE:
                    _writeRawConstant(ac.physicalIndex, source->getShadowSceneDepthRange(ac.data));
                    break;
                case ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*ac.elementCount, source->getShadowSceneDepthRange(l), ac.elementCount);
                    break;
                case ACT_SHADOW_COLOUR:
                    _writeRawConstant(ac.physicalIndex, source->getShadowColour(), ac.elementCount);
                    break;
                case ACT_LIGHT_POWER_SCALE:
                    _writeRawConstant(ac.physicalIndex, source->getLightPowerScale(ac.data));
                    break;
                case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED:
                    _writeRawConstant(ac.physicalIndex, source->getLightDiffuseColourWithPower(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED:
                    _writeRawConstant(ac.physicalIndex, source->getLightSpecularColourWithPower(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_NUMBER:
                    _writeRawConstant(ac.physicalIndex, source->getLightNumber(ac.data));
                    break;
                case ACT_LIGHT_CASTS_SHADOWS:
                    _writeRawConstant(ac.physicalIndex, source->getLightCastsShadows(ac.data));
                    break;
                case ACT_LIGHT_CASTS_SHADOWS_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(float), source->getLightCastsShadows(l));
                    break;
                case ACT_LIGHT_ATTENUATION:
                    _writeRawConstant(ac.physicalIndex, source->getLightAttenuation(ac.data), ac.elementCount);
                    break;
                case ACT_SPOTLIGHT_PARAMS:
                    _writeRawConstant(ac.physicalIndex, source->getSpotlightParams(ac.data), ac.elementCount);
                    break;
                case ACT_LIGHT_DIFFUSE_COLOUR_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightDiffuseColour(l), ac.elementCount);
                    break;

                case ACT_LIGHT_SPECULAR_COLOUR_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightSpecularColour(l), ac.elementCount);
                    break;
                case ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightDiffuseColourWithPower(l), ac.elementCount);
                    break;

                case ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightSpecularColourWithPower(l), ac.elementCount);
                    break;

                case ACT_LIGHT_POSITION_ARRAY:
                    // Get as 4D vector, works for directional lights too
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          source->getLightAs4DVector(l), ac.elementCount);
                    break;

                case ACT_LIGHT_DIRECTION_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        vec3 = source->getLightDirection(l);
                        // Set as 4D vector for compatibility
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          Vector4f(vec3.x, vec3.y, vec3.z, 0.0f), ac.elementCount);
                    }
                    break;

                case ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          source->getViewMatrix() *
                                              source->getLightAs4DVector(l),
                                          ac.elementCount);
                    break;

                case ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY:
                    m3 = source->getInverseTransposeViewMatrix().linear();
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        vec3 = m3 * source->getLightDirection(l);
                        vec3.normalise();
                        // Set as 4D vector for compatibility
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          Vector4f(vec3.x, vec3.y, vec3.z, 0.0f), ac.elementCount);
                    }
                    break;

                case ACT_LIGHT_POWER_SCALE_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Real),
                                          source->getLightPowerScale(l));
                    break;

                case ACT_LIGHT_ATTENUATION_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f),
                                          source->getLightAttenuation(l), ac.elementCount);
                    }
                    break;
                case ACT_SPOTLIGHT_PARAMS_ARRAY:
                    for (size_t l = 0 ; l < ac.data; ++l)
                    {
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Vector4f), source->getSpotlightParams(l),
                                          ac.elementCount);
                    }
                    break;
                case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR:
                    _writeRawConstant(ac.physicalIndex,
                                      source->getLightDiffuseColourWithPower(ac.data) * source->getSurfaceDiffuseColour(),
                                      ac.elementCount);
                    break;
                case ACT_DERIVED_LIGHT_SPECULAR_COLOUR:
                    _writeRawConstant(ac.physicalIndex,
                                      source->getLightSpecularColourWithPower(ac.data) * source->getSurfaceSpecularColour(),
                                      ac.elementCount);
                    break;
                case ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightDiffuseColourWithPower(l) * source->getSurfaceDiffuseColour(),
                                          ac.elementCount);
                    }
                    break;
                case ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        _writeRawConstant(ac.physicalIndex + l*sizeof(ColourValue),
                                          source->getLightSpecularColourWithPower(l) * source->getSurfaceSpecularColour(),
                                          ac.elementCount);
                    }
                    break;
                case ACT_TEXTURE_VIEWPROJ_MATRIX:
                    // can also be updated in lights
                    _writeRawConstant(ac.physicalIndex, source->getTextureViewProjMatrix(ac.data),ac.elementCount);
                    break;
                case ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Matrix4),
                                          source->getTextureViewProjMatrix(l),ac.elementCount);
                    }
                    break;
                case ACT_SPOTLIGHT_VIEWPROJ_MATRIX:
                    _writeRawConstant(ac.physicalIndex, source->getSpotlightViewProjMatrix(ac.data),ac.elementCount);
                    break;
                case ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < ac.data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(ac.physicalIndex + l*sizeof(Matrix4),
                                          source->getSpotlightViewProjMatrix(l),ac.elementCount);
                    }
                    break;

                default:
                    break;
                };
            }
        }

    }
    //---------------------------------------------------------------------------
    static size_t withArrayOffset(const GpuConstantDefinition* def, const String& name)
    {
        uint32 offset = 0;
        if(name.back() == ']')
        {
            size_t start = name.find('[');
            offset = StringConverter::parseInt(name.substr(start + 1, name.size() - start - 2));
            offset = std::min(offset, def->arraySize - 1);
            size_t type_sz = def->isDouble() ? 8 : ( def->isSampler() ? 1 : 4);
            offset *= type_sz;
        }

        return def->physicalIndex + offset * def->elementSize;
    }

    void GpuProgramParameters::setNamedConstant(const String& name, Real val)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), val);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, int val)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def = _findNamedConstantDefinition(name, !mIgnoreMissingParams);

        if(!def)
            return;

        if(def->isSampler())
            _writeRegisters(withArrayOffset(def, name), &val, 1);
        else
            _writeRawConstant(withArrayOffset(def, name), val);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, uint val)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), val);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector4& vec)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), vec, def->elementSize);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector3& vec)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), vec);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector2& vec)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), vec);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4& m)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), m, def->elementSize);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4* m,
                                                size_t numEntries)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), m, numEntries);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const ColourValue& colour)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(withArrayOffset(def, name), colour, def->elementSize);
    }
    //---------------------------------------------------------------------------
    template <typename T> void GpuProgramParameters::_setNamedConstant(const String& name, const T* val, size_t count)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def = _findNamedConstantDefinition(name, !mIgnoreMissingParams);

        if (!def)
            return;

        if (count > def->arraySize * def->elementSize)
        {
            // The shader compiler may trim the array if the trailing elements
            // are unused or their usage can be optimized away. Therefore,
            // a count exceeding the array size not not an error.
            count = def->arraySize * def->elementSize;
        }

        _writeRawConstants(withArrayOffset(def, name), val, count);
    }
    void GpuProgramParameters::setNamedConstant(const String& name, const float* val, size_t count, size_t multiple)
    {
        _setNamedConstant(name, val, count * multiple);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const double* val, size_t count, size_t multiple)
    {
        _setNamedConstant(name, val, count * multiple);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name,
                                                const int *val, size_t count, size_t multiple)
    {
        size_t rawCount = count * multiple;
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def = _findNamedConstantDefinition(name, !mIgnoreMissingParams);

        if(!def)
            return;

        if (rawCount > def->arraySize * def->elementSize)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        StringUtil::format("Too many values for parameter %s: %zu > %d", name.c_str(), count,
                                           def->arraySize * def->elementSize));

        if (def->isSampler())
            _writeRegisters(withArrayOffset(def, name), val, rawCount);
        else
            _writeRawConstants(withArrayOffset(def, name), val, rawCount);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const uint* val, size_t count, size_t multiple)
    {
        _setNamedConstant(name, val, count * multiple);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstant(const String& name,
                                                    AutoConstantType acType, uint32 extraInfo)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
        {
            def->variability = deriveVariability(acType);
            // make sure we also set variability on the logical index map
            getConstantLogicalIndexUse(def->logicalIndex, def->elementSize * def->arraySize, def->variability, BCT_FLOAT);
            _setRawAutoConstant(withArrayOffset(def, name), acType, extraInfo, def->variability, def->elementSize);
        }

    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstantReal(const String& name,
                                                        AutoConstantType acType, Real rData)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
        {
            def->variability = deriveVariability(acType);
            // make sure we also set variability on the logical index map
            getConstantLogicalIndexUse(def->logicalIndex, def->elementSize * def->arraySize, def->variability, BCT_FLOAT);
            _setRawAutoConstantReal(withArrayOffset(def, name), acType, rData, def->variability, def->elementSize);
        }
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
        if (!mLogicalToPhysical)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::findFloatAutoConstantEntry");

        return _findRawAutoConstantEntryFloat(
            _getConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL, BCT_FLOAT));

    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::findAutoConstantEntry(const String& paramName) const
    {
        if (!mNamedConstants)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This params object is not based on a program with named parameters.",
                        "GpuProgramParameters::findAutoConstantEntry");

        const GpuConstantDefinition& def = getConstantDefinition(paramName);
        if(def.isSampler())
            return NULL;
        return _findRawAutoConstantEntryFloat(def.physicalIndex);
    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::_findRawAutoConstantEntryFloat(size_t physicalIndex) const
    {
        for(const auto& ac : mAutoConstants)
        {
            // should check that auto is float and not int so that physicalIndex
            // doesn't have any ambiguity
            // However, all autos are float I think so no need
            if (ac.physicalIndex == physicalIndex)
                return &ac;
        }

        return 0;

    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::copyConstantsFrom(const GpuProgramParameters& source)
    {
        // Pull buffers & auto constant list over directly
        mConstants = source.getConstantList();
        mRegisters = source.mRegisters;
        mAutoConstants = source.getAutoConstantList();
        mCombinedVariability = source.mCombinedVariability;
        copySharedParamSetUsage(source.mSharedParamSets);
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::copyMatchingNamedConstantsFrom(const GpuProgramParameters& source)
    {
        if (mNamedConstants && source.mNamedConstants)
        {
            std::map<size_t, String> srcToDestNamedMap;
            for (auto& m : source.mNamedConstants->map)
            {
                const String& paramName = m.first;
                const GpuConstantDefinition& olddef = m.second;
                const GpuConstantDefinition* newdef = _findNamedConstantDefinition(paramName, false);
                if (newdef)
                {
                    // Copy data across, based on smallest common definition size
                    size_t srcsz = olddef.elementSize * olddef.arraySize;
                    size_t destsz = newdef->elementSize * newdef->arraySize;
                    size_t sz = std::min(srcsz, destsz);
                    if (newdef->isFloat() || newdef->isInt() || newdef->isUnsignedInt() || newdef->isBool())
                    {
                        OgreAssertDbg(mConstants.size() >= (newdef->physicalIndex + sz * 4), "Invalid physical index");
                        memcpy(getFloatPointer(newdef->physicalIndex),
                               source.getFloatPointer(olddef.physicalIndex),
                               sz * 4);
                    }
                    else if (newdef->isDouble())
                    {

                        memcpy(getDoublePointer(newdef->physicalIndex),
                               source.getDoublePointer(olddef.physicalIndex),
                               sz * sizeof(double));
                    }
                    else if (newdef->isSampler())
                    {
                        *getRegPointer(newdef->physicalIndex) = *source.getRegPointer(olddef.physicalIndex);
                    }
                    else
                    {
                        //TODO exception handling
                    }
                    // we'll use this map to resolve autos later
                    // ignore the [0] aliases
                    if (!StringUtil::endsWith(paramName, "[0]") && source.findAutoConstantEntry(paramName))
                        srcToDestNamedMap[olddef.physicalIndex] = paramName;
                }
            }

            for (const auto& autoEntry : source.mAutoConstants)
            {
                // find dest physical index
                std::map<size_t, String>::iterator mi = srcToDestNamedMap.find(autoEntry.physicalIndex);
                if (mi != srcToDestNamedMap.end())
                {
                    if (getAutoConstantDefinition(autoEntry.paramType)->dataType == ACDT_REAL)
                    {
                        setNamedAutoConstantReal(mi->second, autoEntry.paramType, autoEntry.fData);
                    }
                    else
                    {
                        setNamedAutoConstant(mi->second, autoEntry.paramType, autoEntry.data);
                    }
                }

            }

            // Copy shared param sets
            for (const auto& usage : source.mSharedParamSets)
            {
                if (!isUsingSharedParameters(usage.getName()))
                {
                    addSharedParameters(usage.getSharedParams());
                }
            }
        }
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
            *getFloatPointer(mActivePassIterationIndex) += 1;
        }
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::addSharedParameters(GpuSharedParametersPtr sharedParams)
    {
        if (!isUsingSharedParameters(sharedParams->getName()))
        {
            mSharedParamSets.push_back(GpuSharedParametersUsage(sharedParams, this));
        }
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::addSharedParameters(const String& sharedParamsName)
    {
        addSharedParameters(GpuProgramManager::getSingleton().getSharedParameters(sharedParamsName));
    }
    //---------------------------------------------------------------------
    bool GpuProgramParameters::isUsingSharedParameters(const String& sharedParamsName) const
    {
        for (const auto& mSharedParamSet : mSharedParamSets)
        {
            if (mSharedParamSet.getName() == sharedParamsName)
                return true;
        }
        return false;
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::removeSharedParameters(const String& sharedParamsName)
    {
        for (GpuSharedParamUsageList::iterator i = mSharedParamSets.begin();
             i != mSharedParamSets.end(); ++i)
        {
            if (i->getName() == sharedParamsName)
            {
                mSharedParamSets.erase(i);
                break;
            }
        }
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::removeAllSharedParameters()
    {
        mSharedParamSets.clear();
    }
    //---------------------------------------------------------------------
    const GpuProgramParameters::GpuSharedParamUsageList&
    GpuProgramParameters::getSharedParameters() const
    {
        return mSharedParamSets;
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::_copySharedParams()
    {
        for (auto& usage : mSharedParamSets)
        {
            usage._copySharedParamsToTargetParams();
        }
    }

    void GpuProgramParameters::_updateSharedParams()
    {
        for (auto& usage : mSharedParamSets)
        {
            const GpuSharedParametersPtr& sharedParams = usage.getSharedParams();
            if(sharedParams->_getHardwareBuffer())
            {
                sharedParams->_upload();
                sharedParams->_markClean();
                continue;
            }

            usage._copySharedParamsToTargetParams();
        }
    }
}
