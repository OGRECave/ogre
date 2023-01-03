#include <OgreUnifiedShader.h>

RWTexture2D<float4> tex;

uniform float roll;

[numthreads(16, 16, 1)]
void main(
    uint3 gl_WorkGroupID : SV_GroupID,
    uint3 gl_LocalInvocationID : SV_GroupThreadID,
    uint3 gl_GlobalInvocationID : SV_DispatchThreadID,
    uint gl_LocalInvocationIndex : SV_GroupIndex)
{
    float localCoef = length((float2(gl_LocalInvocationID.xy)-8)/8.0);
    float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;
    tex[gl_GlobalInvocationID.xy] = float4(float2(1.0-globalCoef*localCoef, 1.0-globalCoef*localCoef), 1, 1);
}
