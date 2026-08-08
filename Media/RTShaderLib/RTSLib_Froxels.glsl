// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#define USE_FROXELS

#define FROXEL_GRID_VEC4_COUNT 1024   // 4096 froxels / 4 per uvec4
#define FROXEL_RECORD_VEC4_COUNT 1024  // 16384 light indices / 16 per uvec4

// macros for lighting SubRenderStates to get the current froxel lights and light index
#define CURRENT_LIGHT_COUNT lights.count
#define GET_LIGHT_INDEX(n) int(getLightIndex(lights, n))

struct FroxelLights
{
    int offset;
    int count;
    int dirLights;
};

// fragCoord = gl_FragCoord.xyz: .xy in pixels, .z = screen depth
// froxel_params = (countX, countY, fixY, tileSizePx)
// froxel_z_params = (scaleZ, biasZ, linZ, sliceCount)
uvec3 getFroxelCoord(in vec3 fragCoord, in vec4 froxel_params, in vec4 froxel_z_params)
{
#ifdef OGRE_GLSL
    if(froxel_params.z > 0.0) // needs flipping
        fragCoord.y = froxel_params.z - fragCoord.y;
#endif

    uint x = min(uint(fragCoord.x / froxel_params.w), uint(froxel_params.x) - 1u);
    uint y = min(uint(fragCoord.y / froxel_params.w), uint(froxel_params.y) - 1u);

    // recipViewZ = zLightFar / viewZ, coefficients fitted from the real projection matrix
    float recipViewZ = froxel_z_params.x * fragCoord.z + froxel_z_params.y;
    float sliceZ     = log2(max(recipViewZ, 1e-8)) * froxel_z_params.z;
    uint  z          = uint(clamp(sliceZ + froxel_z_params.w, 0.0, froxel_z_params.w - 1.0));

    return uvec3(x, y, z);
}

uint getLightIndex(uint record)
{
    uint word = froxelRecords[record >> 4][(record >> 2) & 3u]; // which uint
    return (word >> ((record & 3u) * 8u)) & 0xFFu;              // which byte
}

// wrapper considering dir lights
int getLightIndex(in FroxelLights lights, int i)
{
    return i < lights.dirLights ? i : int(getLightIndex(uint(lights.offset + i)));
}

// jet-like ramp (blue -> cyan -> green -> yellow -> red), t in [0, 1]
vec3 debugRamp(float t)
{
    t = saturate(t);
    return saturate(vec3(1.5 - abs(4.0 * t - 3.0),
                         1.5 - abs(4.0 * t - 2.0),
                         1.5 - abs(4.0 * t - 1.0)));
}
void debugFroxelOccupancy(in FroxelLights lights, inout vec3 color)
{
#ifdef LIGHT_COUNT
    float count = float(lights.count - lights.dirLights);
    if (count < 1.0)
        return;

    float t = saturate(count / float(LIGHT_COUNT - lights.dirLights));
    color = mix(color, debugRamp(t), 0.8);
#endif
}

void getFroxelLights(in vec3 fragCoord, in vec4 froxel_params, in vec4 froxel_z_params,
                     in vec4 light0Pos,
                     out FroxelLights lights
#ifdef DEBUG_FROXELS
                     , inout vec3 color
#endif
                    )
{
    lights.dirLights = int(light0Pos.w == 0.0);

    uvec3 f   = getFroxelCoord(fragCoord, froxel_params, froxel_z_params);
    uint fidx     = (f.z * uint(froxel_params.y) + f.y) * uint(froxel_params.x) + f.x;
    uint grid  = froxelGrid[fidx >> 2][fidx & 3u];
    lights.offset = int(grid >> 8) - lights.dirLights;
    lights.count  = int(grid & 0xFFu) + lights.dirLights;

#ifdef DEBUG_FROXELS
    // depth
    //color = mix(color, debugRamp((float(f.z) + 0.5f) / max(froxel_z_params.w - 1.0, 1.0)), 0.8);
    //color = mix(color, debugRamp(fract(float(f.x + f.y) / 8.0)), 0.8);
    debugFroxelOccupancy(lights, color);
#endif
}