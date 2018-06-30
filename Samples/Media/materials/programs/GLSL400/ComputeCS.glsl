#version 430

layout(binding = 0, rgba8) coherent uniform image2D image_data;

uniform float roll;

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// source/ details: http://wili.cc/blog/opengl-cs.html
void main()
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

    float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);
    float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;
    imageStore(image_data, storePos, vec4(vec2(1.0-globalCoef*localCoef),1.0,1.0));
}
