#version 420
//#extension GL_ARB_compute_shader : enable

uniform float number;
//uniform image2D destTex;

// struct particle{
//     vec4 currentPos;
//     vec4 oldPos;
// };

// layout(std430, binding=0) buffer particles{
//     struct particle p[];
// };

layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
    // uint gid = gl_GlobalInvocationID.x;
    // p[gid].currentPos.x += 100;
    
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    
    float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);\
    float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;\

    // imageStore(destTex, storePos, 
    //            number * vec4(1.0 - globalCoef * localCoef, 0.0, 0.0, 0.0)));
}
