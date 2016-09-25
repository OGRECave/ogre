#version 430

layout (rgba8) uniform restrict writeonly image2D testTexture;

layout( local_size_x = @value( threads_per_group_x ),
        local_size_y = @value( threads_per_group_y ),
        local_size_z = @value( threads_per_group_z ) ) in;

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

//uniform vec3 cameraPos;

void main()
{
    imageStore( testTexture, ivec2(gl_GlobalInvocationID.xy),
                vec4( vec2(gl_LocalInvocationID.xy) / 16.0f, 0.0f, 1.0f ) );
//    imageStore( testTexture, ivec2(gl_GlobalInvocationID.xy),
//                vec4( cameraPos.xyz, 1.0f ) );
}
