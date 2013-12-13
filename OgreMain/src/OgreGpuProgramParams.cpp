/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

  Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreDualQuaternion.h"
#include "OgreAutoParamDataSource.h"
#include "OgreLight.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"


namespace Ogre
{

    //---------------------------------------------------------------------
    GpuProgramParameters::AutoConstantDefinition GpuProgramParameters::AutoConstantDictionary[] = {
        AutoConstantDefinition(ACT_WORLD_MATRIX,                  "world_matrix",                16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_WORLD_MATRIX,          "inverse_world_matrix",        16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_TRANSPOSE_WORLD_MATRIX,             "transpose_world_matrix",            16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_INVERSE_TRANSPOSE_WORLD_MATRIX, "inverse_transpose_world_matrix", 16, ET_REAL, ACDT_NONE),

        AutoConstantDefinition(ACT_WORLD_MATRIX_ARRAY_3x4,        "world_matrix_array_3x4",      12, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_WORLD_MATRIX_ARRAY,            "world_matrix_array",          16, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_WORLD_DUALQUATERNION_ARRAY_2x4, "world_dualquaternion_array_2x4",      8, ET_REAL, ACDT_NONE),
        AutoConstantDefinition(ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4, "world_scale_shear_matrix_array_3x4", 9, ET_REAL, ACDT_NONE),
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
        AutoConstantDefinition(ACT_LIGHT_CUSTOM,        "light_custom", 4, ET_REAL, ACDT_INT)
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
        size_t memSize = 0;

        // Buffer size refs
        memSize += 3 * sizeof(size_t);

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
        std::fstream *f = OGRE_NEW_T(std::fstream, MEMCATEGORY_GENERAL)();
        f->open(filename.c_str(), std::ios::binary | std::ios::out);
        DataStreamPtr stream(OGRE_NEW FileStreamDataStream(f));

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
                        "GpuNamedConstantsSerializer::exportSkeleton");
        }

        writeFileHeader();

        writeInts(((const uint32*)&pConsts->floatBufferSize), 1);
        writeInts(((const uint32*)&pConsts->doubleBufferSize), 1);
        writeInts(((const uint32*)&pConsts->intBufferSize), 1);
        writeInts(((const uint32*)&pConsts->uintBufferSize), 1);
        // writeInts(((const uint32*)&pConsts->boolBufferSize), 1);

        // simple export of all the named constants, no chunks
        // name, physical index
        for (GpuConstantDefinitionMap::const_iterator i = pConsts->map.begin();
             i != pConsts->map.end(); ++i)
        {
            const String& name = i->first;
            const GpuConstantDefinition& def = i->second;

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

        readInts(stream, ((uint32*)&pDest->floatBufferSize), 1);
        readInts(stream, ((uint32*)&pDest->doubleBufferSize), 1);
        readInts(stream, ((uint32*)&pDest->intBufferSize), 1);
        readInts(stream, ((uint32*)&pDest->uintBufferSize), 1);
        // readInts(stream, ((uint32*)&pDest->boolBufferSize), 1);

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
        , mFrameLastUpdated(Root::getSingleton().getNextFrameNumber())
        , mVersion(0), mDirty(false)
    {

    }
    //---------------------------------------------------------------------
    GpuSharedParameters::~GpuSharedParameters()
    {

    }
    //-----------------------------------------------------------------------------
    size_t GpuSharedParameters::calculateSize(void) const
    {
        size_t memSize = 0;

        memSize += sizeof(float) * mFloatConstants.size();
        memSize += sizeof(double) * mDoubleConstants.size();
        memSize += sizeof(int) * mIntConstants.size(); 
        memSize += sizeof(uint) * mUnsignedIntConstants.size(); 
        // memSize += sizeof(bool) * mBoolConstants.size(); 
        memSize += mName.size() * sizeof(char);
        memSize += sizeof(Any);
        memSize += sizeof(size_t);
        memSize += sizeof(unsigned long);

        return memSize;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::addConstantDefinition(const String& name, GpuConstantType constType, size_t arraySize)
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
        // for compatibility we do not pad values to multiples of 4
        // when it comes to arrays, user is responsible for creating matching defs
        def.elementSize = GpuConstantDefinition::getElementSize(constType, false);

        // not used
        def.logicalIndex = 0;
        def.variability = (uint16)GPV_GLOBAL;

        if (def.isFloat())
        {
            def.physicalIndex = mFloatConstants.size();
            mFloatConstants.resize(mFloatConstants.size() + def.arraySize * def.elementSize);
        }
        else if (def.isDouble())
        {
            def.physicalIndex = mDoubleConstants.size();
            mDoubleConstants.resize(mDoubleConstants.size() + def.arraySize * def.elementSize);
        }
        else if (def.isInt() || def.isSampler() || def.isSubroutine())
        {
            def.physicalIndex = mIntConstants.size();
            mIntConstants.resize(mIntConstants.size() + def.arraySize * def.elementSize);
        }
        else if (def.isUnsignedInt() || def.isBool())
        {
            def.physicalIndex = mUnsignedIntConstants.size();
            mUnsignedIntConstants.resize(mUnsignedIntConstants.size() + def.arraySize * def.elementSize);
        }
        // else if (def.isBool())
        // {
        //     def.physicalIndex = mBoolConstants.size();
        //     mBoolConstants.resize(mBoolConstants.size() + def.arraySize * def.elementSize);
        // }
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
            bool isFloat = def.isFloat(); //TODO does a double check belong here too?
            size_t numElems = def.elementSize * def.arraySize;

            for (GpuConstantDefinitionMap::iterator j = mNamedConstants.map.begin();
                 j != mNamedConstants.map.end(); ++j)
            {
                GpuConstantDefinition& otherDef = j->second;
                bool otherIsFloat = otherDef.isFloat();

                // same type, and comes after in the buffer
                if ( (isFloat == otherIsFloat) &&
                     otherDef.physicalIndex > def.physicalIndex)
                {
                    // adjust index
                    otherDef.physicalIndex -= numElems;
                }
            }

            // remove floats and reduce buffer
            if (isFloat)
            {
                mNamedConstants.floatBufferSize -= numElems;

                FloatConstantList::iterator beg = mFloatConstants.begin();
                std::advance(beg, def.physicalIndex);
                FloatConstantList::iterator en = beg;
                std::advance(en, numElems);
                mFloatConstants.erase(beg, en);
            }
            else if (def.isInt() || def.isSampler() || def.isSubroutine())
            {
                mNamedConstants.intBufferSize -= numElems;

                IntConstantList::iterator beg = mIntConstants.begin();
                std::advance(beg, def.physicalIndex);
                IntConstantList::iterator en = beg;
                std::advance(en, numElems);
                mIntConstants.erase(beg, en);
            }
            else if (def.isUnsignedInt() || def.isBool()) {
                mNamedConstants.uintBufferSize -= numElems;

                UnsignedIntConstantList::iterator beg = mUnsignedIntConstants.begin();
                std::advance(beg, def.physicalIndex);
                UnsignedIntConstantList::iterator en = beg;
                std::advance(en, numElems);
                mUnsignedIntConstants.erase(beg, en);
            }
            else {
                //TODO exception handling
            }
            // else { // bool
            //     mNamedConstants.boolBufferSize -= numElems;

            //     BoolConstantList::iterator beg = mBoolConstants.begin();
            //     std::advance(beg, def.physicalIndex);
            //     BoolConstantList::iterator en = beg;
            //     std::advance(en, numElems);
            //     mBoolConstants.erase(beg, en);
            // }

            ++mVersion;
        }

    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::removeAllConstantDefinitions()
    {
        mNamedConstants.map.clear();
        mNamedConstants.floatBufferSize = 0;
        mNamedConstants.doubleBufferSize = 0;
        mNamedConstants.intBufferSize = 0;
        mNamedConstants.uintBufferSize = 0;
        // mNamedConstants.boolBufferSize = 0;
        mFloatConstants.clear();
        mDoubleConstants.clear();
        mIntConstants.clear();
        mUnsignedIntConstants.clear();
        // mBoolConstants.clear();
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
    void GpuSharedParameters::setNamedConstant(const String& name, Real val)
    {
        setNamedConstant(name, &val, 1);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, int val)
    {
        setNamedConstant(name, &val, 1);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, uint val)
    {
        setNamedConstant(name, &val, 1);
    }
    //---------------------------------------------------------------------
    // void GpuSharedParameters::setNamedConstant(const String& name, bool val)
    // {
    //     setNamedConstant(name, &val, 1);
    // }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Vector4& vec)
    {
        setNamedConstant(name, vec.ptr(), 4);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Vector3& vec)
    {
        setNamedConstant(name, vec.ptr(), 3);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Vector2& vec)
    {
        setNamedConstant(name, vec.ptr(), 2);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Matrix4& m)
    {
        setNamedConstant(name, m[0], 16);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const Matrix4* m, size_t numEntries)
    {
        setNamedConstant(name, m[0][0], 16 * numEntries);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const ColourValue& colour)
    {
        setNamedConstant(name, colour.ptr(), 4);
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const float *val, size_t count)
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i != mNamedConstants.map.end())
        {
            const GpuConstantDefinition& def = i->second;
            memcpy(&mFloatConstants[def.physicalIndex], val,
                   sizeof(float) * std::min(count, def.elementSize * def.arraySize));
        }

        _markDirty();
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const double *val, size_t count)
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i != mNamedConstants.map.end())
        {
            const GpuConstantDefinition& def = i->second;

            count = std::min(count, def.elementSize * def.arraySize);
            const double* src = val;
            double* dst = &mDoubleConstants[def.physicalIndex];
            for (size_t v = 0; v < count; ++v)
            {
                *dst++ = static_cast<double>(*src++);
            }
        }

        _markDirty();
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const int *val, size_t count)
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i != mNamedConstants.map.end())
        {
            const GpuConstantDefinition& def = i->second;
            memcpy(&mIntConstants[def.physicalIndex], val,
                   sizeof(int) * std::min(count, def.elementSize * def.arraySize));
        }

        _markDirty();
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::setNamedConstant(const String& name, const uint *val, size_t count)
    {
        GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
        if (i != mNamedConstants.map.end())
        {
            const GpuConstantDefinition& def = i->second;
            memcpy(&mUnsignedIntConstants[def.physicalIndex], val,
                   sizeof(uint) * std::min(count, def.elementSize * def.arraySize));
        }

        _markDirty();
    }
    //---------------------------------------------------------------------
    // void GpuSharedParameters::setNamedConstant(const String& name, const bool *val, size_t count)
    // {
    //     GpuConstantDefinitionMap::const_iterator i = mNamedConstants.map.find(name);
    //     if (i != mNamedConstants.map.end())
    //     {
    //         const GpuConstantDefinition& def = i->second;
    //         memcpy(&mBoolConstants[def.physicalIndex], val,
    //                sizeof(bool) * std::min(count, def.elementSize * def.arraySize));
    //     }

    //     _markDirty();
    // }
    //---------------------------------------------------------------------
    void GpuSharedParameters::_markClean()
    {
        mDirty = false;
    }
    //---------------------------------------------------------------------
    void GpuSharedParameters::_markDirty()
    {
        mFrameLastUpdated = Root::getSingleton().getNextFrameNumber();
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
        for (GpuConstantDefinitionMap::const_iterator i = sharedmap.begin(); i != sharedmap.end(); ++i)
        {
            const String& pName = i->first;
            const GpuConstantDefinition& shareddef = i->second;

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
            }

        }

        mCopyDataVersion = mSharedParams->getVersion();
    }
    //---------------------------------------------------------------------
    void GpuSharedParametersUsage::_copySharedParamsToTargetParams()
    {
        // check copy data version
        if (mCopyDataVersion != mSharedParams->getVersion())
            initCopyData();

        for (CopyDataList::iterator i = mCopyDataList.begin(); i != mCopyDataList.end(); ++i)
        {
            CopyDataEntry& e = *i;

            if (e.dstDefinition->isFloat())
            {
                const float* pSrc = mSharedParams->getFloatPointer(e.srcDefinition->physicalIndex);
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
                const double* pSrc = mSharedParams->getDoublePointer(e.srcDefinition->physicalIndex);
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
            else if (e.dstDefinition->isInt() || 
                     e.dstDefinition->isSampler() ||
                     e.dstDefinition->isSubroutine())
            {
                const int* pSrc = mSharedParams->getIntPointer(e.srcDefinition->physicalIndex);
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
                const uint* pSrc = mSharedParams->getUnsignedIntPointer(e.srcDefinition->physicalIndex);
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
            // else if (e.dstDefinition->isBool()) 
            // {
            //     const bool* pSrc = mSharedParams->getBoolPointer(e.srcDefinition->physicalIndex);
            //     bool* pDst = mParams->getBoolPointer(e.dstDefinition->physicalIndex);

            //     if (e.dstDefinition->elementSize == e.srcDefinition->elementSize)
            //     {
            //         // simple copy
            //         memcpy(pDst, pSrc, sizeof(bool) * e.dstDefinition->elementSize * e.dstDefinition->arraySize);
            //     }
            //     else
            //     {
            //         // target params may be padded to 4 elements, shared params are packed
            //         assert(e.dstDefinition->elementSize % 4 == 0);
            //         size_t iterations = (e.dstDefinition->elementSize / 4)
            //             * e.dstDefinition->arraySize;
            //         assert(iterations > 0);
            //         size_t valsPerIteration = e.srcDefinition->elementSize;
            //         for (size_t l = 0; l < iterations; ++l)
            //         {
            //             memcpy(pDst, pSrc, sizeof(bool) * valsPerIteration);
            //             pSrc += valsPerIteration;
            //             pDst += 4;
            //         }
            //     }
            // }
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
        mDoubleConstants = oth.mDoubleConstants;
        mIntConstants  = oth.mIntConstants;
        mUnsignedIntConstants  = oth.mUnsignedIntConstants;
        // mBoolConstants  = oth.mBoolConstants;
        mAutoConstants = oth.mAutoConstants;
        mFloatLogicalToPhysical = oth.mFloatLogicalToPhysical;
        mDoubleLogicalToPhysical = oth.mDoubleLogicalToPhysical;
        mIntLogicalToPhysical = oth.mIntLogicalToPhysical;
        mUnsignedIntLogicalToPhysical = oth.mUnsignedIntLogicalToPhysical;
        mBoolLogicalToPhysical = oth.mBoolLogicalToPhysical;
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
        for (GpuSharedParamUsageList::const_iterator i = srcList.begin(); i != srcList.end(); ++i)
        {
            mSharedParamSets.push_back(GpuSharedParametersUsage(i->getSharedParams(), this));
        }

    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::calculateSize(void) const
    {
        size_t memSize = 0;

        memSize += sizeof(float) * mFloatConstants.size();
        memSize += sizeof(double) * mDoubleConstants.size();
        memSize += sizeof(int) * mIntConstants.size();
        memSize += sizeof(uint) * mUnsignedIntConstants.size();
        // memSize += sizeof(bool) * mBoolConstants.size();
        memSize += sizeof(Any);
        memSize += sizeof(size_t);
        memSize += sizeof(bool) * 2;
        memSize += sizeof(uint16);

        for (AutoConstantList::const_iterator i = mAutoConstants.begin();
             i != mAutoConstants.end(); ++i)
        {
            memSize += sizeof((*i));
        }

        if(!mFloatLogicalToPhysical.isNull())
            memSize += mFloatLogicalToPhysical->bufferSize;
        if(!mDoubleLogicalToPhysical.isNull())
            memSize += mDoubleLogicalToPhysical->bufferSize;
        if(!mIntLogicalToPhysical.isNull())
            memSize += mIntLogicalToPhysical->bufferSize;
        if(!mUnsignedIntLogicalToPhysical.isNull())
            memSize += mUnsignedIntLogicalToPhysical->bufferSize;
        if(!mBoolLogicalToPhysical.isNull())
            memSize += mBoolLogicalToPhysical->bufferSize;

        return memSize;
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::_setNamedConstants(
        const GpuNamedConstantsPtr& namedConstants)
    {
        mNamedConstants = namedConstants;

        // Determine any extension to local buffers

        // Size and reset buffer (fill with zero to make comparison later ok)
        if (namedConstants->floatBufferSize > mFloatConstants.size())
        {
            mFloatConstants.insert(mFloatConstants.end(),
                                   namedConstants->floatBufferSize - mFloatConstants.size(), 0.0f);
        }
        if (namedConstants->doubleBufferSize > mDoubleConstants.size())
        {
            mDoubleConstants.insert(mDoubleConstants.end(),
                                   namedConstants->doubleBufferSize - mDoubleConstants.size(), 0.0f);
        }
        if (namedConstants->intBufferSize > mIntConstants.size())
        {
            mIntConstants.insert(mIntConstants.end(),
                                 namedConstants->intBufferSize - mIntConstants.size(), 0);
        }
        if (namedConstants->uintBufferSize > mUnsignedIntConstants.size())
        {
            mUnsignedIntConstants.insert(mUnsignedIntConstants.end(),
                                 namedConstants->uintBufferSize - mUnsignedIntConstants.size(), 0);
        }
        // if (namedConstants->boolBufferSize > mBoolConstants.size())
        // {
        //     mBoolConstants.insert(mBoolConstants.end(),
        //                          namedConstants->boolBufferSize - mBoolConstants.size(), false);
        // }
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::_setLogicalIndexes(
        const GpuLogicalBufferStructPtr& floatIndexMap,
        const GpuLogicalBufferStructPtr& doubleIndexMap,
        const GpuLogicalBufferStructPtr& intIndexMap,
        const GpuLogicalBufferStructPtr& uintIndexMap,
        const GpuLogicalBufferStructPtr& boolIndexMap
    )
    {
        mFloatLogicalToPhysical = floatIndexMap;
        mDoubleLogicalToPhysical = doubleIndexMap;
        mIntLogicalToPhysical = intIndexMap;
        mUnsignedIntLogicalToPhysical = uintIndexMap;
        mBoolLogicalToPhysical = boolIndexMap;

        // resize the internal buffers
        // Note that these will only contain something after the first parameter
        // set has set some parameters

        // Size and reset buffer (fill with zero to make comparison later ok)
        if (!floatIndexMap.isNull() && floatIndexMap->bufferSize > mFloatConstants.size())
        {
            mFloatConstants.insert(mFloatConstants.end(),
                                   floatIndexMap->bufferSize - mFloatConstants.size(), 0.0f);
        }
        if (!doubleIndexMap.isNull() && doubleIndexMap->bufferSize > mDoubleConstants.size())
        {
            mDoubleConstants.insert(mDoubleConstants.end(),
                                    doubleIndexMap->bufferSize - mDoubleConstants.size(), 0.0f);
        }
        if (!intIndexMap.isNull() &&  intIndexMap->bufferSize > mIntConstants.size())
        {
            mIntConstants.insert(mIntConstants.end(),
                                 intIndexMap->bufferSize - mIntConstants.size(), 0);
        }
        if (!uintIndexMap.isNull() &&  uintIndexMap->bufferSize > mUnsignedIntConstants.size())
        {
            mUnsignedIntConstants.insert(mUnsignedIntConstants.end(),
                                 uintIndexMap->bufferSize - mUnsignedIntConstants.size(), 0);
        }
        // if (!boolIndexMap.isNull() &&  boolIndexMap->bufferSize > mBoolConstants.size())
        // {
        //     mBoolConstants.insert(mBoolConstants.end(),
        //                          boolIndexMap->bufferSize - mBoolConstants.size(), 0);
        // }

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
        assert(!mFloatLogicalToPhysical.isNull() && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getFloatConstantPhysicalIndex(index, rawCount, GPV_GLOBAL);

        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const double *val, size_t count)
    {
        // Raw buffer size is 4x count
        size_t rawCount = count * 4;
        // get physical index
        assert(!mFloatLogicalToPhysical.isNull() && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getFloatConstantPhysicalIndex(index, rawCount, GPV_GLOBAL);
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
        assert(!mIntLogicalToPhysical.isNull() && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getIntConstantPhysicalIndex(index, rawCount, GPV_GLOBAL);
        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const uint *val, size_t count)
    {
        // Raw buffer size is 4x count
        size_t rawCount = count * 4;
        // get physical index
        assert(!mUnsignedIntLogicalToPhysical.isNull() && "GpuProgram hasn't set up the logical -> physical map!");

        size_t physicalIndex = _getUnsignedIntConstantPhysicalIndex(index, rawCount, GPV_GLOBAL);
        // Copy
        _writeRawConstants(physicalIndex, val, rawCount);
    }
    //-----------------------------------------------------------------------------
    // void GpuProgramParameters::setConstant(size_t index, const bool *val, size_t count)
    // {
    //     // Raw buffer size is 4x count
    //     size_t rawCount = count * 4;
    //     // get physical index
    //     assert(!mBoolLogicalToPhysical.isNull() && "GpuProgram hasn't set up the logical -> physical map!");

    //     size_t physicalIndex = _getBoolConstantPhysicalIndex(index, rawCount, GPV_GLOBAL);
    //     // Copy
    //     _writeRawConstants(physicalIndex, val, rawCount);
    // }
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
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, Real val, size_t count)
    {
        _writeRawConstants(physicalIndex, &val, count);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, int val)
    {
        _writeRawConstants(physicalIndex, &val, 1);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, uint val)
    {
        _writeRawConstants(physicalIndex, &val, 1);
    }
    //-----------------------------------------------------------------------------
    // void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, bool val)
    // {
    //     _writeRawConstants(physicalIndex, &val, 1);
    // }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Vector3& vec)
    {
        _writeRawConstants(physicalIndex, vec.ptr(), 3);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_writeRawConstant(size_t physicalIndex, const Vector2& vec)
    {
        _writeRawConstants(physicalIndex, vec.ptr(), 2);
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
    void GpuProgramParameters::_writeRawConstants(size_t physicalIndex, const uint* val, size_t count)
    {
        assert(physicalIndex + count <= mUnsignedIntConstants.size());
        memcpy(&mUnsignedIntConstants[physicalIndex], val, sizeof(uint) * count);
    }
    //-----------------------------------------------------------------------------
    // void GpuProgramParameters::_writeRawConstants(size_t physicalIndex, const bool* val, size_t count)
    // {
    //     assert(physicalIndex + count <= mBoolConstants.size());
    //     memcpy(&mBoolConstants[physicalIndex], val, sizeof(bool) * count);
    // }
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
    GpuLogicalIndexUse* GpuProgramParameters::_getFloatConstantLogicalIndexUse(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        if (mFloatLogicalToPhysical.isNull())
            return 0;

        GpuLogicalIndexUse* indexUse = 0;
        OGRE_LOCK_MUTEX(mFloatLogicalToPhysical->mutex);

        GpuLogicalIndexUseMap::iterator logi = mFloatLogicalToPhysical->map.find(logicalIndex);
        if (logi == mFloatLogicalToPhysical->map.end())
        {
            if (requestedSize)
            {
                size_t physicalIndex = mFloatConstants.size();

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
                GpuLogicalIndexUseMap::iterator insertedIterator;

                for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
                {
                    GpuLogicalIndexUseMap::iterator it =
                        mFloatLogicalToPhysical->map.insert(
                            GpuLogicalIndexUseMap::value_type(
                                logicalIndex + logicalNum,
                                GpuLogicalIndexUse(currPhys, requestedSize, variability))).first;
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
                mFloatLogicalToPhysical->bufferSize += insertCount;
                for (AutoConstantList::iterator i = mAutoConstants.begin();
                     i != mAutoConstants.end(); ++i)
                {
                    const GpuProgramParameters::AutoConstantDefinition* def = getAutoConstantDefinition(i->paramType);
                    if (i->physicalIndex > physicalIndex &&
                        def && def->elementType == ET_REAL)
                    {
                        i->physicalIndex += insertCount;
                    }
                }
                if (!mNamedConstants.isNull())
                {
                    for (GpuConstantDefinitionMap::iterator i = mNamedConstants->map.begin();
                         i != mNamedConstants->map.end(); ++i)
                    {
                        if (i->second.isFloat() && i->second.physicalIndex > physicalIndex)
                            i->second.physicalIndex += insertCount;
                    }
                    mNamedConstants->floatBufferSize += insertCount;
                }

                logi->second.currentSize += insertCount;
            }
        }

        if (indexUse)
            indexUse->variability = variability;

        return indexUse;

    }
    //---------------------------------------------------------------------
    GpuLogicalIndexUse* GpuProgramParameters::_getDoubleConstantLogicalIndexUse(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        if (mDoubleLogicalToPhysical.isNull())
            return 0;

        GpuLogicalIndexUse* indexUse = 0;
        OGRE_LOCK_MUTEX(mDoubleLogicalToPhysical->mutex);

        GpuLogicalIndexUseMap::iterator logi = mDoubleLogicalToPhysical->map.find(logicalIndex);
        if (logi == mDoubleLogicalToPhysical->map.end())
        {
            if (requestedSize)
            {
                size_t physicalIndex = mDoubleConstants.size();

                // Expand at buffer end
                mDoubleConstants.insert(mDoubleConstants.end(), requestedSize, 0.0f);

                // Record extended size for future GPU params re-using this information
                mDoubleLogicalToPhysical->bufferSize = mDoubleConstants.size();

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
                        mDoubleLogicalToPhysical->map.insert(
                            GpuLogicalIndexUseMap::value_type(
                                logicalIndex + logicalNum,
                                GpuLogicalIndexUse(currPhys, requestedSize, variability))).first;
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
                DoubleConstantList::iterator insertPos = mDoubleConstants.begin();
                std::advance(insertPos, physicalIndex);
                mDoubleConstants.insert(insertPos, insertCount, 0.0f);
                // shift all physical positions after this one
                for (GpuLogicalIndexUseMap::iterator i = mDoubleLogicalToPhysical->map.begin();
                     i != mDoubleLogicalToPhysical->map.end(); ++i)
                {
                    if (i->second.physicalIndex > physicalIndex)
                        i->second.physicalIndex += insertCount;
                }
                mDoubleLogicalToPhysical->bufferSize += insertCount;
                for (AutoConstantList::iterator i = mAutoConstants.begin();
                     i != mAutoConstants.end(); ++i)
                {
                    const GpuProgramParameters::AutoConstantDefinition* def = getAutoConstantDefinition(i->paramType);
                    if (i->physicalIndex > physicalIndex &&
                        def && def->elementType == ET_REAL)
                    {
                        i->physicalIndex += insertCount;
                    }
                }
                if (!mNamedConstants.isNull())
                {
                    for (GpuConstantDefinitionMap::iterator i = mNamedConstants->map.begin();
                         i != mNamedConstants->map.end(); ++i)
                    {
                        if (i->second.isDouble() && i->second.physicalIndex > physicalIndex)
                            i->second.physicalIndex += insertCount;
                    }
                    mNamedConstants->doubleBufferSize += insertCount;
                }

                logi->second.currentSize += insertCount;
            }
        }

        if (indexUse)
            indexUse->variability = variability;

        return indexUse;

    }
    //---------------------------------------------------------------------()
    GpuLogicalIndexUse* GpuProgramParameters::_getIntConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        if (mIntLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::_getIntConstantPhysicalIndex");

        GpuLogicalIndexUse* indexUse = 0;
        OGRE_LOCK_MUTEX(mIntLogicalToPhysical->mutex);

        GpuLogicalIndexUseMap::iterator logi = mIntLogicalToPhysical->map.find(logicalIndex);
        if (logi == mIntLogicalToPhysical->map.end())
        {
            if (requestedSize)
            {
                size_t physicalIndex = mIntConstants.size();

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
                GpuLogicalIndexUseMap::iterator insertedIterator;
                for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
                {
                    GpuLogicalIndexUseMap::iterator it =
                        mIntLogicalToPhysical->map.insert(
                            GpuLogicalIndexUseMap::value_type(
                                logicalIndex + logicalNum,
                                GpuLogicalIndexUse(currPhys, requestedSize, variability))).first;
                    if (logicalNum == 0)
                        insertedIterator = it;
                    currPhys += 4;
                }
                indexUse = &(insertedIterator->second);

            }
            else
            {
                // no match
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
                mIntLogicalToPhysical->bufferSize += insertCount;
                for (AutoConstantList::iterator i = mAutoConstants.begin();
                     i != mAutoConstants.end(); ++i)
                {
                    const GpuProgramParameters::AutoConstantDefinition* def = getAutoConstantDefinition(i->paramType);
                    if (i->physicalIndex > physicalIndex &&
                        def && def->elementType == ET_INT)
                    {
                        i->physicalIndex += insertCount;
                    }
                }
                if (!mNamedConstants.isNull())
                {
                    for (GpuConstantDefinitionMap::iterator i = mNamedConstants->map.begin();
                         i != mNamedConstants->map.end(); ++i)
                    {
                        if (!i->second.isFloat() && i->second.physicalIndex > physicalIndex)
                            i->second.physicalIndex += insertCount;
                    }
                    mNamedConstants->intBufferSize += insertCount;
                }

                logi->second.currentSize += insertCount;
            }
        }

        if (indexUse)
            indexUse->variability = variability;

        return indexUse;

    }
    //---------------------------------------------------------------------()
    //TODO double check that this is implemented correctly
    GpuLogicalIndexUse* GpuProgramParameters::_getUnsignedIntConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        if (mUnsignedIntLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::_getUnsignedIntConstantPhysicalIndex");

        GpuLogicalIndexUse* indexUse = 0;
        OGRE_LOCK_MUTEX(mUnsignedIntLogicalToPhysical->mutex);

        GpuLogicalIndexUseMap::iterator logi = mUnsignedIntLogicalToPhysical->map.find(logicalIndex);
        if (logi == mUnsignedIntLogicalToPhysical->map.end())
        {
            if (requestedSize)
            {
                size_t physicalIndex = mUnsignedIntConstants.size();

                // Expand at buffer end
                mUnsignedIntConstants.insert(mUnsignedIntConstants.end(), requestedSize, 0);

                // Record extended size for future GPU params re-using this information
                mUnsignedIntLogicalToPhysical->bufferSize = mUnsignedIntConstants.size();

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
                        mUnsignedIntLogicalToPhysical->map.insert(
                            GpuLogicalIndexUseMap::value_type(
                                logicalIndex + logicalNum,
                                GpuLogicalIndexUse(currPhys, requestedSize, variability))).first;
                    if (logicalNum == 0)
                        insertedIterator = it;
                    currPhys += 4;
                }
                indexUse = &(insertedIterator->second);

            }
            else
            {
                // no match
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
                UnsignedIntConstantList::iterator insertPos = mUnsignedIntConstants.begin();
                std::advance(insertPos, physicalIndex);
                mUnsignedIntConstants.insert(insertPos, insertCount, 0);
                // shift all physical positions after this one
                for (GpuLogicalIndexUseMap::iterator i = mUnsignedIntLogicalToPhysical->map.begin();
                     i != mUnsignedIntLogicalToPhysical->map.end(); ++i)
                {
                    if (i->second.physicalIndex > physicalIndex)
                        i->second.physicalIndex += insertCount;
                }
                mUnsignedIntLogicalToPhysical->bufferSize += insertCount;
                // for (AutoConstantList::iterator i = mAutoConstants.begin();
                //      i != mAutoConstants.end(); ++i)
                // {
                //     const GpuProgramParameters::AutoConstantDefinition* def = getAutoConstantDefinition(i->paramType);
                //     if (i->physicalIndex > physicalIndex &&
                //         def && def->elementType == ET_UINT)
                //     {
                //         i->physicalIndex += insertCount;
                //     }
                // }
                if (!mNamedConstants.isNull())
                {
                    for (GpuConstantDefinitionMap::iterator i = mNamedConstants->map.begin();
                         i != mNamedConstants->map.end(); ++i)
                    {
                        if (!i->second.isFloat() && i->second.physicalIndex > physicalIndex)
                            i->second.physicalIndex += insertCount;
                    }
                    mNamedConstants->uintBufferSize += insertCount;
                }

                logi->second.currentSize += insertCount;
            }
        }

        if (indexUse)
            indexUse->variability = variability;

        return indexUse;

    }
    //---------------------------------------------------------------------()
    // //TODO double check that this is implemented correctly
    // GpuLogicalIndexUse* GpuProgramParameters::_getBoolConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability)
    // {
    //     if (mBoolLogicalToPhysical.isNull())
    //         OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
    //                     "This is not a low-level parameter parameter object",
    //                     "GpuProgramParameters::_getBoolConstantPhysicalIndex");

    //     GpuLogicalIndexUse* indexUse = 0;
    //     OGRE_LOCK_MUTEX(mBoolLogicalToPhysical->mutex);

    //     GpuLogicalIndexUseMap::iterator logi = mBoolLogicalToPhysical->map.find(logicalIndex);
    //     if (logi == mBoolLogicalToPhysical->map.end())
    //     {
    //         if (requestedSize)
    //         {
    //             size_t physicalIndex = mBoolConstants.size();

    //             // Expand at buffer end
    //             mBoolConstants.insert(mBoolConstants.end(), requestedSize, false);

    //             // Record extended size for future GPU params re-using this information
    //             mBoolLogicalToPhysical->bufferSize = mBoolConstants.size();

    //             // low-level programs will not know about mapping ahead of time, so
    //             // populate it. Other params objects will be able to just use this
    //             // accepted mapping since the constant structure will be the same

    //             // Set up a mapping for all items in the count
    //             size_t currPhys = physicalIndex;
    //             size_t count = requestedSize / 4;
    //             GpuLogicalIndexUseMap::iterator insertedIterator;
    //             for (size_t logicalNum = 0; logicalNum < count; ++logicalNum)
    //             {
    //                 GpuLogicalIndexUseMap::iterator it =
    //                     mBoolLogicalToPhysical->map.insert(
    //                         GpuLogicalIndexUseMap::value_type(
    //                             logicalIndex + logicalNum,
    //                             GpuLogicalIndexUse(currPhys, requestedSize, variability))).first;
    //                 if (logicalNum == 0)
    //                     insertedIterator = it;
    //                 currPhys += 4;
    //             }
    //             indexUse = &(insertedIterator->second);

    //         }
    //         else
    //         {
    //             // no match
    //             return 0;
    //         }

    //     }
    //     else
    //     {
    //         size_t physicalIndex = logi->second.physicalIndex;
    //         indexUse = &(logi->second);

    //         // check size
    //         if (logi->second.currentSize < requestedSize)
    //         {
    //             // init buffer entry wasn't big enough; could be a mistake on the part
    //             // of the original use, or perhaps a variable length we can't predict
    //             // until first actual runtime use e.g. world matrix array
    //             size_t insertCount = requestedSize - logi->second.currentSize;
    //             BoolConstantList::iterator insertPos = mBoolConstants.begin();
    //             std::advance(insertPos, physicalIndex);
    //             mBoolConstants.insert(insertPos, insertCount, 0);
    //             // shift all physical positions after this one
    //             for (GpuLogicalIndexUseMap::iterator i = mBoolLogicalToPhysical->map.begin();
    //                  i != mBoolLogicalToPhysical->map.end(); ++i)
    //             {
    //                 if (i->second.physicalIndex > physicalIndex)
    //                     i->second.physicalIndex += insertCount;
    //             }
    //             mBoolLogicalToPhysical->bufferSize += insertCount;
    //             // for (AutoConstantList::iterator i = mAutoConstants.begin();
    //             //      i != mAutoConstants.end(); ++i)
    //             // {
    //             //     const GpuProgramParameters::AutoConstantDefinition* def = getAutoConstantDefinition(i->paramType);
    //             //     if (i->physicalIndex > physicalIndex &&
    //             //         def && def->elementType == ET_BOOL)
    //             //     {
    //             //         i->physicalIndex += insertCount;
    //             //     }
    //             // }
    //             if (!mNamedConstants.isNull())
    //             {
    //                 for (GpuConstantDefinitionMap::iterator i = mNamedConstants->map.begin();
    //                      i != mNamedConstants->map.end(); ++i)
    //                 {
    //                     if (!i->second.isFloat() && i->second.physicalIndex > physicalIndex)
    //                         i->second.physicalIndex += insertCount;
    //                 }
    //                 mNamedConstants->boolBufferSize += insertCount;
    //             }

    //             logi->second.currentSize += insertCount;
    //         }
    //     }

    //     if (indexUse)
    //         indexUse->variability = variability;

    //     return indexUse;

    // }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::_getFloatConstantPhysicalIndex(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(logicalIndex, requestedSize, variability);
        return indexUse ? indexUse->physicalIndex : 0;
    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::_getDoubleConstantPhysicalIndex(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        GpuLogicalIndexUse* indexUse = _getDoubleConstantLogicalIndexUse(logicalIndex, requestedSize, variability);
        return indexUse ? indexUse->physicalIndex : 0;
    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::_getIntConstantPhysicalIndex(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        GpuLogicalIndexUse* indexUse = _getIntConstantLogicalIndexUse(logicalIndex, requestedSize, variability);
        return indexUse ? indexUse->physicalIndex : 0;
    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::_getUnsignedIntConstantPhysicalIndex(
        size_t logicalIndex, size_t requestedSize, uint16 variability)
    {
        GpuLogicalIndexUse* indexUse = _getUnsignedIntConstantLogicalIndexUse(logicalIndex, requestedSize, variability);
        return indexUse ? indexUse->physicalIndex : 0;
    }
    //-----------------------------------------------------------------------------
    // size_t GpuProgramParameters::_getBoolConstantPhysicalIndex(
    //     size_t logicalIndex, size_t requestedSize, uint16 variability)
    // {
    //     GpuLogicalIndexUse* indexUse = _getBoolConstantLogicalIndexUse(logicalIndex, requestedSize, variability);
    //     return indexUse ? indexUse->physicalIndex : 0;
    // }
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
    size_t GpuProgramParameters::getDoubleLogicalIndexForPhysicalIndex(size_t physicalIndex)
    {
        // perhaps build a reverse map of this sometime (shared in GpuProgram)
        for (GpuLogicalIndexUseMap::iterator i = mDoubleLogicalToPhysical->map.begin();
             i != mDoubleLogicalToPhysical->map.end(); ++i)
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
    size_t GpuProgramParameters::getUnsignedIntLogicalIndexForPhysicalIndex(size_t physicalIndex)
    {
        // perhaps build a reverse map of this sometime (shared in GpuProgram)
        for (GpuLogicalIndexUseMap::iterator i = mUnsignedIntLogicalToPhysical->map.begin();
             i != mUnsignedIntLogicalToPhysical->map.end(); ++i)
        {
            if (i->second.physicalIndex == physicalIndex)
                return i->first;
        }
        return std::numeric_limits<size_t>::max();

    }
    //-----------------------------------------------------------------------------
    size_t GpuProgramParameters::getBoolLogicalIndexForPhysicalIndex(size_t physicalIndex)
    {
        // perhaps build a reverse map of this sometime (shared in GpuProgram)
        for (GpuLogicalIndexUseMap::iterator i = mBoolLogicalToPhysical->map.begin();
             i != mBoolLogicalToPhysical->map.end(); ++i)
        {
            if (i->second.physicalIndex == physicalIndex)
                return i->first;
        }
        return std::numeric_limits<size_t>::max();

    }
    //-----------------------------------------------------------------------------
    GpuConstantDefinitionIterator GpuProgramParameters::getConstantDefinitionIterator(void) const
    {
        if (mNamedConstants.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This params object is not based on a program with named parameters.",
                        "GpuProgramParameters::getConstantDefinitionIterator");

        return GpuConstantDefinitionIterator(mNamedConstants->map.begin(),
                                             mNamedConstants->map.end());

    }
    //-----------------------------------------------------------------------------
    const GpuNamedConstants& GpuProgramParameters::getConstantDefinitions() const
    {
        if (mNamedConstants.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This params object is not based on a program with named parameters.",
                        "GpuProgramParameters::getConstantDefinitionIterator");

        return *mNamedConstants;
    }
    //-----------------------------------------------------------------------------
    const GpuConstantDefinition& GpuProgramParameters::getConstantDefinition(const String& name) const
    {
        if (mNamedConstants.isNull())
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
        if (mNamedConstants.isNull())
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
                            "Parameter called " + name + " does not exist.",
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

        GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(index, sz, deriveVariability(acType));

        if(indexUse)
            _setRawAutoConstant(indexUse->physicalIndex, acType, extraInfo, indexUse->variability, sz);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_setRawAutoConstant(size_t physicalIndex,
                                                   AutoConstantType acType, size_t extraInfo, uint16 variability, size_t elementSize)
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
                i->variability = variability;
                found = true;
                break;
            }
        }
        if (!found)
            mAutoConstants.push_back(AutoConstantEntry(acType, physicalIndex, extraInfo, variability, elementSize));

        mCombinedVariability |= variability;


    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstant(size_t index, AutoConstantType acType, uint16 extraInfo1, uint16 extraInfo2)
    {
        size_t extraInfo = (size_t)extraInfo1 | ((size_t)extraInfo2) << 16;

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

        GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(index, sz, deriveVariability(acType));

        _setRawAutoConstant(indexUse->physicalIndex, acType, extraInfo, indexUse->variability, sz);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::_setRawAutoConstantReal(size_t physicalIndex,
                                                       AutoConstantType acType, Real rData, uint16 variability, size_t elementSize)
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
                i->variability = variability;
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
        GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(index, 0, GPV_GLOBAL);

        if (indexUse)
        {
            indexUse->variability = GPV_GLOBAL;
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
            def->variability = GPV_GLOBAL;

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
    GpuProgramParameters::AutoConstantIterator GpuProgramParameters::getAutoConstantIterator(void) const
    {
        return AutoConstantIterator(mAutoConstants.begin(), mAutoConstants.end());
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstantReal(size_t index, AutoConstantType acType, Real rData)
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

        GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(index, sz, deriveVariability(acType));

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
        const Matrix4* pMatrix;
        size_t m;
        Vector3 vec3;
        Vector4 vec4;
        Matrix3 m3;
        Matrix4 scaleM;
        DualQuaternion dQuat;

        mActivePassIterationIndex = std::numeric_limits<size_t>::max();

        // Autoconstant index is not a physical index
        for (AutoConstantList::const_iterator i = mAutoConstants.begin(); i != mAutoConstants.end(); ++i)
        {
            // Only update needed slots
            if (i->variability & mask)
            {

                switch(i->paramType)
                {
                case ACT_VIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getViewMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_VIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseViewMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_VIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeViewMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_VIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeViewMatrix(),i->elementCount);
                    break;

                case ACT_PROJECTION_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getProjectionMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_PROJECTION_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseProjectionMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_PROJECTION_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeProjectionMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeProjectionMatrix(),i->elementCount);
                    break;

                case ACT_VIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getViewProjectionMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseViewProjMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeViewProjMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeViewProjMatrix(),i->elementCount);
                    break;
                case ACT_RENDER_TARGET_FLIPPING:
                    _writeRawConstant(i->physicalIndex, source->getCurrentRenderTarget()->requiresTextureFlipping() ? -1.f : +1.f);
                    break;
                case ACT_VERTEX_WINDING:
                    {
                        RenderSystem* rsys = Root::getSingleton().getRenderSystem();
                        _writeRawConstant(i->physicalIndex, rsys->getInvertVertexWinding() ? -1.f : +1.f);
                    }
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
                case ACT_SURFACE_ALPHA_REJECTION_VALUE:
                    _writeRawConstant(i->physicalIndex, source->getSurfaceAlphaRejectionValue());
                    break;

                case ACT_CAMERA_POSITION:
                    _writeRawConstant(i->physicalIndex, source->getCameraPosition(), i->elementCount);
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
                    // this is actually just an initial set-up, it's bound separately, so still global
                    _writeRawConstant(i->physicalIndex, 0.0f);
                    mActivePassIterationIndex = i->physicalIndex;
                    break;
                case ACT_TEXTURE_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTextureTransformMatrix(i->data),i->elementCount);
                    break;
                case ACT_LOD_CAMERA_POSITION:
                    _writeRawConstant(i->physicalIndex, source->getLodCameraPosition(), i->elementCount);
                    break;

                case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX:
                    // can also be updated in lights
                    _writeRawConstant(i->physicalIndex, source->getTextureWorldViewProjMatrix(i->data),i->elementCount);
                    break;
                case ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(i->physicalIndex + l*i->elementCount,
                                          source->getTextureWorldViewProjMatrix(l),i->elementCount);
                    }
                    break;
                case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getSpotlightWorldViewProjMatrix(i->data),i->elementCount);
                    break;
                case ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                        _writeRawConstant(i->physicalIndex + l*i->elementCount, source->getSpotlightWorldViewProjMatrix(i->data), i->elementCount);
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
                case ACT_LIGHT_DISTANCE_OBJECT_SPACE:
                    vec3 = source->getInverseWorldMatrix().transformAffine(source->getLightPosition(i->data));
                    _writeRawConstant(i->physicalIndex, vec3.length());
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

                case ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                    {
                        vec3 = source->getInverseWorldMatrix().transformAffine(source->getLightPosition(l));
                        _writeRawConstant(i->physicalIndex + l*i->elementCount, vec3.length());
                    }
                    break;

                case ACT_WORLD_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getWorldMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_WORLD_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseWorldMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_WORLD_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeWorldMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_WORLD_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldMatrix(),i->elementCount);
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
                case ACT_WORLD_DUALQUATERNION_ARRAY_2x4:
                    // Loop over matrices
                    pMatrix = source->getWorldMatrixArray();
                    numMatrices = source->getWorldMatrixCount();
                    index = i->physicalIndex;
                    for (m = 0; m < numMatrices; ++m)
                    {
                        dQuat.fromTransformationMatrix(*pMatrix);
                        _writeRawConstants(index, dQuat.ptr(), 8);
                        index += 8;
                        ++pMatrix;
                    }
                    break;
                case ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4:
                    // Loop over matrices
                    pMatrix = source->getWorldMatrixArray();
                    numMatrices = source->getWorldMatrixCount();
                    index = i->physicalIndex;

                    scaleM = Matrix4::IDENTITY;

                    for (m = 0; m < numMatrices; ++m)
                    {
                        //Based on Matrix4::decompostion, but we don't need the rotation or position components
                        //but do need the scaling and shearing. Shearing isn't available from Matrix4::decomposition
                        assert((*pMatrix).isAffine());

                        (*pMatrix).extract3x3Matrix(m3);

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
                        index += 12;
                        ++pMatrix;
                    }
                    break;
                case ACT_WORLDVIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getWorldViewMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseWorldViewMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeWorldViewMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldViewMatrix(),i->elementCount);
                    break;

                case ACT_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getWorldViewProjMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseWorldViewProjMatrix(),i->elementCount);
                    break;
                case ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getTransposeWorldViewProjMatrix(),i->elementCount);
                    break;
                case ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getInverseTransposeWorldViewProjMatrix(),i->elementCount);
                    break;
                case ACT_CAMERA_POSITION_OBJECT_SPACE:
                    _writeRawConstant(i->physicalIndex, source->getCameraPositionObjectSpace(), i->elementCount);
                    break;
                case ACT_LOD_CAMERA_POSITION_OBJECT_SPACE:
                    _writeRawConstant(i->physicalIndex, source->getLodCameraPositionObjectSpace(), i->elementCount);
                    break;

                case ACT_CUSTOM:
                case ACT_ANIMATION_PARAMETRIC:
                    source->getCurrentRenderable()->_updateCustomGpuParameter(*i, this);
                    break;
                case ACT_LIGHT_CUSTOM:
                    source->updateLightCustomGpuParameter(*i, this);
                    break;
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
                case ACT_SHADOW_EXTRUSION_DISTANCE:
                    // extrusion is in object-space, so we have to rescale by the inverse
                    // of the world scaling to deal with scaled objects
                    source->getWorldMatrix().extract3x3Matrix(m3);
                    _writeRawConstant(i->physicalIndex, source->getShadowExtrusionDistance() /
                                      Math::Sqrt(std::max(std::max(m3.GetColumn(0).squaredLength(), m3.GetColumn(1).squaredLength()), m3.GetColumn(2).squaredLength())));
                    break;
                case ACT_SHADOW_SCENE_DEPTH_RANGE:
                    _writeRawConstant(i->physicalIndex, source->getShadowSceneDepthRange(i->data));
                    break;
                case ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                        _writeRawConstant(i->physicalIndex + l*i->elementCount, source->getShadowSceneDepthRange(i->data), i->elementCount);
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
                case ACT_LIGHT_CASTS_SHADOWS_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                        _writeRawConstant(i->physicalIndex + l*i->elementCount, source->getLightCastsShadows(i->data), i->elementCount);
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
                    _writeRawConstant(i->physicalIndex, source->getTextureViewProjMatrix(i->data),i->elementCount);
                    break;
                case ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(i->physicalIndex + l*i->elementCount,
                                          source->getTextureViewProjMatrix(l),i->elementCount);
                    }
                    break;
                case ACT_SPOTLIGHT_VIEWPROJ_MATRIX:
                    _writeRawConstant(i->physicalIndex, source->getSpotlightViewProjMatrix(i->data),i->elementCount);
                    break;
                case ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY:
                    for (size_t l = 0; l < i->data; ++l)
                    {
                        // can also be updated in lights
                        _writeRawConstant(i->physicalIndex + l*i->elementCount,
                                          source->getSpotlightViewProjMatrix(l),i->elementCount);
                    }
                    break;

                default:
                    break;
                };
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
    void GpuProgramParameters::setNamedConstant(const String& name, uint val)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstant(def->physicalIndex, val);
    }
    //---------------------------------------------------------------------------
    // void GpuProgramParameters::setNamedConstant(const String& name, bool val)
    // {
    //     // look up, and throw an exception if we're not ignoring missing
    //     const GpuConstantDefinition* def =
    //         _findNamedConstantDefinition(name, !mIgnoreMissingParams);
    //     if (def)
    //         _writeRawConstant(def->physicalIndex, val);
    // }
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
    void GpuProgramParameters::setNamedConstant(const String& name, const Vector2& vec)
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
            _writeRawConstant(def->physicalIndex, m, def->elementSize);
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
    void GpuProgramParameters::setNamedConstant(const String& name,
                                                const uint *val, size_t count, size_t multiple)
    {
        size_t rawCount = count * multiple;
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
            _writeRawConstants(def->physicalIndex, val, rawCount);
    }
    //---------------------------------------------------------------------------
    // void GpuProgramParameters::setNamedConstant(const String& name,
    //                                             const bool *val, size_t count, size_t multiple)
    // {
    //     size_t rawCount = count * multiple;
    //     // look up, and throw an exception if we're not ignoring missing
    //     const GpuConstantDefinition* def =
    //         _findNamedConstantDefinition(name, !mIgnoreMissingParams);
    //     if (def)
    //         _writeRawConstants(def->physicalIndex, val, rawCount);
    // }
    //---------------------------------------------------------------------
    void GpuProgramParameters::setNamedSubroutine(const String& subroutineSlot, const String& subroutine)
    {
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(subroutineSlot, !mIgnoreMissingParams);
        if (def)
        {
            setSubroutine(def->logicalIndex, subroutine);
        }
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::setSubroutine(size_t index, const String& subroutine)
    {
        mSubroutineMap.insert(std::make_pair(index, subroutine));
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstant(const String& name,
                                                    AutoConstantType acType, size_t extraInfo)
    {
        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
        {
            def->variability = deriveVariability(acType);
            // make sure we also set variability on the logical index map
            GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(def->logicalIndex, def->elementSize * def->arraySize, def->variability);
            if (indexUse)
                indexUse->variability = def->variability;

            _setRawAutoConstant(def->physicalIndex, acType, extraInfo, def->variability, def->elementSize);
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
            GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(def->logicalIndex, def->elementSize * def->arraySize, def->variability);
            if (indexUse)
                indexUse->variability = def->variability;
            _setRawAutoConstantReal(def->physicalIndex, acType, rData, def->variability, def->elementSize);
        }
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstant(const String& name,
                                                    AutoConstantType acType, uint16 extraInfo1, uint16 extraInfo2)
    {
        size_t extraInfo = (size_t)extraInfo1 | ((size_t)extraInfo2) << 16;

        // look up, and throw an exception if we're not ignoring missing
        const GpuConstantDefinition* def =
            _findNamedConstantDefinition(name, !mIgnoreMissingParams);
        if (def)
        {
            def->variability = deriveVariability(acType);
            // make sure we also set variability on the logical index map
            GpuLogicalIndexUse* indexUse = _getFloatConstantLogicalIndexUse(def->logicalIndex, def->elementSize * def->arraySize, def->variability);
            if (indexUse)
                indexUse->variability = def->variability;

            _setRawAutoConstant(def->physicalIndex, acType, extraInfo, def->variability, def->elementSize);
        }
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
        if (mFloatLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::findFloatAutoConstantEntry");

        return _findRawAutoConstantEntryFloat(
            _getFloatConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL));

    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::findDoubleAutoConstantEntry(size_t logicalIndex)
    {
        if (mDoubleLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::findDoubleAutoConstantEntry");

        return _findRawAutoConstantEntryDouble(
            _getDoubleConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL));
    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::findIntAutoConstantEntry(size_t logicalIndex)
    {
        if (mIntLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::findIntAutoConstantEntry");

        return _findRawAutoConstantEntryInt(
            _getIntConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL));
    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::findUnsignedIntAutoConstantEntry(size_t logicalIndex)
    {
        if (mUnsignedIntLogicalToPhysical.isNull())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "This is not a low-level parameter parameter object",
                        "GpuProgramParameters::findUnsignedIntAutoConstantEntry");

        return _findRawAutoConstantEntryUnsignedInt(
            _getUnsignedIntConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL));
    }
    //---------------------------------------------------------------------------
    // const GpuProgramParameters::AutoConstantEntry*
    // GpuProgramParameters::findBoolAutoConstantEntry(size_t logicalIndex)
    // {
    //     if (mBoolLogicalToPhysical.isNull())
    //         OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
    //                     "This is not a low-level parameter parameter object",
    //                     "GpuProgramParameters::findBoolAutoConstantEntry");

    //     return _findRawAutoConstantEntryBool(
    //         _getBoolConstantPhysicalIndex(logicalIndex, 0, GPV_GLOBAL));
    // }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::findAutoConstantEntry(const String& paramName)
    {
        if (mNamedConstants.isNull())
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
    GpuProgramParameters::_findRawAutoConstantEntryDouble(size_t physicalIndex)
    {
        for(AutoConstantList::iterator i = mAutoConstants.begin();
            i != mAutoConstants.end(); ++i)
        {
            AutoConstantEntry& ac = *i;
            // should check that auto is double and not int or float so that physicalIndex
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
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::_findRawAutoConstantEntryUnsignedInt(size_t physicalIndex)
    {
        //TODO  No autos are float?
        return 0;
    }
    //---------------------------------------------------------------------------
    const GpuProgramParameters::AutoConstantEntry*
    GpuProgramParameters::_findRawAutoConstantEntryBool(size_t physicalIndex)
    {
        //TODO No autos are float?
        return 0;
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::copyConstantsFrom(const GpuProgramParameters& source)
    {
        // Pull buffers & auto constant list over directly
        mFloatConstants = source.getFloatConstantList();
        mDoubleConstants = source.getDoubleConstantList();
        mIntConstants = source.getIntConstantList();
        mUnsignedIntConstants = source.getUnsignedIntConstantList();
        // mBoolConstants = source.getBoolConstantList();
        mAutoConstants = source.getAutoConstantList();
        mCombinedVariability = source.mCombinedVariability;
        copySharedParamSetUsage(source.mSharedParamSets);
    }
    //---------------------------------------------------------------------
    void GpuProgramParameters::copyMatchingNamedConstantsFrom(const GpuProgramParameters& source)
    {
        if (!mNamedConstants.isNull() && !source.mNamedConstants.isNull())
        {
            std::map<size_t, String> srcToDestNamedMap;
            for (GpuConstantDefinitionMap::const_iterator i = source.mNamedConstants->map.begin();
                 i != source.mNamedConstants->map.end(); ++i)
            {
                const String& paramName = i->first;
                const GpuConstantDefinition& olddef = i->second;
                const GpuConstantDefinition* newdef = _findNamedConstantDefinition(paramName, false);
                if (newdef)
                {
                    // Copy data across, based on smallest common definition size
                    size_t srcsz = olddef.elementSize * olddef.arraySize;
                    size_t destsz = newdef->elementSize * newdef->arraySize;
                    size_t sz = std::min(srcsz, destsz);
                    if (newdef->isFloat())
                    {

                        memcpy(getFloatPointer(newdef->physicalIndex),
                               source.getFloatPointer(olddef.physicalIndex),
                               sz * sizeof(float));
                    }
                    else if (newdef->isDouble())
                    {

                        memcpy(getDoublePointer(newdef->physicalIndex),
                               source.getDoublePointer(olddef.physicalIndex),
                               sz * sizeof(double));
                    }
                    else if (newdef->isInt() || 
                             newdef->isSampler() || 
                             newdef->isSubroutine())
                    {
                        memcpy(getIntPointer(newdef->physicalIndex),
                               source.getIntPointer(olddef.physicalIndex),
                               sz * sizeof(int));
                    }
                    else if (newdef->isUnsignedInt() || newdef->isBool())
                    {
                        memcpy(getUnsignedIntPointer(newdef->physicalIndex),
                               source.getUnsignedIntPointer(olddef.physicalIndex),
                               sz * sizeof(uint));
                    }
                    // else // bool
                    // {
                    //     memcpy(getBoolPointer(newdef->physicalIndex),
                    //            source.getBoolPointer(olddef.physicalIndex),
                    //            sz * sizeof(bool));
                    // }
                    else
                    {
                        //TODO exception handling
                    }
                    // we'll use this map to resolve autos later
                    // ignore the [0] aliases
                    if (!StringUtil::endsWith(paramName, "[0]"))
                        srcToDestNamedMap[olddef.physicalIndex] = paramName;
                }
            }

            for (AutoConstantList::const_iterator i = source.mAutoConstants.begin();
                 i != source.mAutoConstants.end(); ++i)
            {
                const GpuProgramParameters::AutoConstantEntry& autoEntry = *i;
                // find dest physical index
                std::map<size_t, String>::iterator mi = srcToDestNamedMap.find(autoEntry.physicalIndex);
                if (mi != srcToDestNamedMap.end())
                {
                    if (autoEntry.fData)
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
            for (GpuSharedParamUsageList::const_iterator i = source.mSharedParamSets.begin();
                 i != source.mSharedParamSets.end(); ++i)
            {
                const GpuSharedParametersUsage& usage = *i;
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
            ++mFloatConstants[mActivePassIterationIndex];
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
        for (GpuSharedParamUsageList::const_iterator i = mSharedParamSets.begin();
             i != mSharedParamSets.end(); ++i)
        {
            if (i->getName() == sharedParamsName)
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
        for (GpuSharedParamUsageList::iterator i = mSharedParamSets.begin();
             i != mSharedParamSets.end(); ++i )
        {
            i->_copySharedParamsToTargetParams();
        }

    }




}
