#define FROXEL_GRID_VEC4_COUNT 1024   // 4096 froxels / 4 per uvec4
#define FROXEL_RECORD_VEC4_COUNT 1024  // 16384 light indices / 16 per uvec4
#if 0
layout(std140) uniform OgreFroxels {
    uvec4 froxelGrid[FROXEL_GRID_VEC4_COUNT];  // (offset << 8) | count
    uvec4 froxelRecords[FROXEL_RECORD_VEC4_COUNT];
};
#endif

void debugFroxel(inout vec3 color, int index)
{
    const vec3 debugColors[17] = vec3[](
        vec3(0.0,     0.0,     0.0),         // black
        vec3(0.0,     0.0,     0.1647),      // darkest blue
        vec3(0.0,     0.0,     0.3647),      // darker blue
        vec3(0.0,     0.0,     0.6647),      // dark blue
        vec3(0.0,     0.0,     0.9647),      // blue
        vec3(0.0,     0.9255,  0.9255),      // cyan
        vec3(0.0,     0.5647,  0.0),         // dark green
        vec3(0.0,     0.7843,  0.0),         // green
        vec3(1.0,     1.0,     0.0),         // yellow
        vec3(0.90588, 0.75294, 0.0),         // yellow-orange
        vec3(1.0,     0.5647,  0.0),         // orange
        vec3(1.0,     0.0,     0.0),         // bright red
        vec3(0.8392,  0.0,     0.0),         // red
        vec3(1.0,     0.0,     1.0),         // magenta
        vec3(0.6,     0.3333,  0.7882),      // purple
        vec3(1.0,     1.0,     1.0),         // white
        vec3(1.0,     1.0,     1.0)          // white
    );
    color = mix(color, debugColors[clamp(index, 0, 16)], 0.8);
}

#define DEBUG_FROXELS 1

// fragCoord = gl_FragCoord.xyz: .xy in pixels, .z = screen depth
// froxel_params = (countX, countY, countZ, tileSizePx)
// froxel_z_params = (scaleZ, biasZ, linZ, sliceCount)
uint getFroxelIndex(in vec3 fragCoord, in vec4 froxel_params, in vec4 froxel_z_params
#if DEBUG_FROXELS
    , inout vec4 color
#endif
) {
    uint x = min(uint(fragCoord.x / froxel_params.w), uint(froxel_params.x) - 1u);
    uint y = min(uint(fragCoord.y / froxel_params.w), uint(froxel_params.y) - 1u);

#if DEBUG_FROXELS == 1
    uint tile = y * uint(froxel_params.x) + x;
    int  idx  = int(tile % 16);

    debugFroxel(color.rgb, idx);
#endif

    float depth = fragCoord.z;
#if OGRE_REVERSED_Z
    depth = 1.0 - depth;   // reversed-z is exactly 1 - standard depth for perspective
#endif

    // screen depth -> reciprocal normalized view-Z (linear in depth)
    float recipViewZ = froxel_z_params.x * depth + froxel_z_params.y;
    recipViewZ       = max(recipViewZ, 1e-6);                 // guard: far plane -> log2(0)
    float sliceCount = froxel_z_params.w;
    float sliceZ     = log2(recipViewZ) * froxel_z_params.z;  // linearizerNeg is negative
    uint  z = uint(clamp(sliceZ + sliceCount, 0.0, sliceCount - 1.0));

#if DEBUG_FROXELS == 2
    debugFroxel(color.rgb, int(z));
#endif

    return (z * uint(froxel_params.y) + y) * uint(froxel_params.x) + x;
}

uint getLightIndex(uint record, in uvec4 froxelRecords[FROXEL_RECORD_VEC4_COUNT]) {
    uint word = froxelRecords[record >> 4][(record >> 2) & 3u]; // which uint
    return (word >> ((record & 3u) * 8u)) & 0xFFu;              // which byte
}

#if !DEBUG_FROXELS
void shadeClustered(vec3 fragCoord, in uvec4 froxelGrid[FROXEL_GRID_VEC4_COUNT], in uvec4 froxelRecords[FROXEL_RECORD_VEC4_COUNT], in vec4 froxel_params, in vec4 froxel_z_params) {
    uint f      = getFroxelIndex(fragCoord, froxel_params, froxel_z_params);
    uint grid  = froxelGrid[f >> 2][f & 3u];
    uint offset = grid >> 8;
    uint count  = grid & 0xFFu;

    for (uint i = 0u; i < count; ++i) {
        uint lightIndex = getLightIndex(offset + i, froxelRecords);
    }
}
#endif