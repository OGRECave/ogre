#version 430

uniform float multiplier;

// layout(binding = 0, r32ui) coherent uniform uimage2D image_data;
layout(binding = 0, rgba8) coherent uniform image2D image_data;


// struct particle{
//     vec4 currentPos;
//     vec4 oldPos;
// };

// layout(std430, binding=0) buffer particles{
//     struct particle p[];
// };

layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;
void main() 
{
    // uint gid = gl_GlobalInvocationID.x;
    // p[gid].currentPos.x += 100;
    
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    
    float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);
    float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1)*0.5;

    // imageStore(image_data, storePos,
    //            multiplier * vec4(1.0 - globalCoef * localCoef, 0.0, 0.0, 1.0));
    imageStore(image_data, ivec2(0), vec4(0,1,0,1));
    // imageStore(image_data, ivec2(0), vec4(gl_GlobalInvocationID.x / 256, gl_GlobalInvocationID.y / 256, gl_GlobalInvocationID.z / 256, 1));
}
