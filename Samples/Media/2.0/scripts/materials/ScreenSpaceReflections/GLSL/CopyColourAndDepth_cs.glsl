#version 430

uniform sampler2D srcRtt;
@property( !texture1_msaa )
	uniform sampler2D srcDepth;
@end @property( texture1_msaa )
	uniform sampler2DMS srcDepth;
@end

layout (@insertpiece(uav0_pf_type)) uniform restrict writeonly image2D dstRtt;
layout (@insertpiece(uav1_pf_type)) uniform restrict writeonly image2D dstDepth;

layout( local_size_x = @value( threads_per_group_x ),
		local_size_y = @value( threads_per_group_y ),
		local_size_z = @value( threads_per_group_z ) ) in;
/*layout (rgba8) uniform restrict writeonly image2D dstRtt;
layout (r32f) uniform restrict writeonly image2D dstDepth;

layout( local_size_x = 8,
		local_size_y = 8,
		local_size_z = 1 ) in;*/

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

void main()
{
	uvec2 xyPos0 = gl_GlobalInvocationID.xy << 1u;
	uvec2 xyPos1 = ivec2( xyPos0.xy + uvec2( 1, 0 ) );
	uvec2 xyPos2 = ivec2( xyPos0.xy + uvec2( 0, 1 ) );
	uvec2 xyPos3 = ivec2( xyPos0.xy + uvec2( 1, 1 ) );

	vec4 srcRttValue0 = texelFetch( srcRtt, ivec2( xyPos0.xy ), 0 );
	vec4 srcRttValue1 = texelFetch( srcRtt, ivec2( xyPos1.xy ), 0 );
	vec4 srcRttValue2 = texelFetch( srcRtt, ivec2( xyPos2.xy ), 0 );
	vec4 srcRttValue3 = texelFetch( srcRtt, ivec2( xyPos3.xy ), 0 );

	vec4 srcDepthValue0 = texelFetch( srcDepth, ivec2( xyPos0.xy ), 0 );
	vec4 srcDepthValue1 = texelFetch( srcDepth, ivec2( xyPos1.xy ), 0 );
	vec4 srcDepthValue2 = texelFetch( srcDepth, ivec2( xyPos2.xy ), 0 );
	vec4 srcDepthValue3 = texelFetch( srcDepth, ivec2( xyPos3.xy ), 0 );

	imageStore( dstRtt, ivec2( xyPos0.xy ), srcRttValue0 );
	imageStore( dstRtt, ivec2( xyPos1.xy ), srcRttValue1 );
	imageStore( dstRtt, ivec2( xyPos2.xy ), srcRttValue2 );
	imageStore( dstRtt, ivec2( xyPos3.xy ), srcRttValue3 );

	imageStore( dstDepth, ivec2( xyPos0.xy ), srcDepthValue0 );
	imageStore( dstDepth, ivec2( xyPos1.xy ), srcDepthValue1 );
	imageStore( dstDepth, ivec2( xyPos2.xy ), srcDepthValue2 );
	imageStore( dstDepth, ivec2( xyPos3.xy ), srcDepthValue3 );
}
