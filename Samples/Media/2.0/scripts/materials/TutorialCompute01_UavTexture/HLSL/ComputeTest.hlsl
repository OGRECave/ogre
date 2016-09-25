
RWTexture2D<float4> testTexture			: register(u0);

[numthreads(@value( threads_per_group_x ), @value( threads_per_group_y ), @value( threads_per_group_z ))]
void main
(
    uint3 gl_LocalInvocationID : SV_GroupThreadID,
    uint3 gl_GlobalInvocationID : SV_DispatchThreadId
)
{
    testTexture[gl_GlobalInvocationID.xy].xyzw = float4( float2(gl_LocalInvocationID.xy) / 16.0f, 0.0f, 1.0f );
}
