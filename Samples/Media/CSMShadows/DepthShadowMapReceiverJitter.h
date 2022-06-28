/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/

float2 offsetSample(float2 uv, float2 offset, float invMapSize)
{
    return float2(uv.xy + offset * invMapSize);
}

float2 offsetRotateSample(float2 uv, float2 offset, float2x2 rotMat, float invMapSize)
{
    offset = mul(rotMat, offset);
    return float2(uv.xy + offset * invMapSize);
}


half getShadowFactor
(
    uniform sampler2D shadowMapUnit,
    float4 lightSpacePos,
    uniform float shadowmapSize,
    uniform float inverseShadowmapSize,
    uniform float fixedDepthBias,
    uniform float gradientScaleBias,
    half shadowLightDotLN
)
{
    // point on shadowmap
    float depthAdjust = fixedDepthBias + (1.0f - abs(shadowLightDotLN)) * gradientScaleBias;
    lightSpacePos.z -= depthAdjust; // lightSpacePos.z contains lightspace position of current object

    const float radius = 3.0f;
    const float4 offsets[4] = {	float4(-0.3464376f, -0.7120676f,  0.4692491f, -0.7939163f) * radius,
                                float4(0.1561844f, -0.04416115f,  -0.9133415f, -0.3314315f) * radius,
                                float4(-0.4210564f, 0.4838058f, 0.5794852f, 0.474482f) * radius,
                                float4(0.7723071f, -0.2627881f, 0.07587272f, 0.926478f) * radius};



    float2 rotations = tex2D_inBranch( jitterMapUnit, lightSpacePos.xy * shadowmapSize) * 2 - 1;
    float2x2 rotMat = float2x2(rotations.x, -rotations.y, rotations.y, rotations.x);

    half4 shadows;
    shadows.r = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[0].xy, rotMat, inverseShadowmapSize));
    shadows.g = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[0].zw, rotMat, inverseShadowmapSize));
    shadows.b = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[1].xy, rotMat, inverseShadowmapSize));
    shadows.a = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[1].zw, rotMat, inverseShadowmapSize));

    half4 inLight = (lightSpacePos.zzzz < shadows);
    half shadow = dot( inLight, half4(0.125, 0.125, 0.125, 0.125) );

    shadows.r = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[2].xy, rotMat, inverseShadowmapSize));
    shadows.g = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[2].zw, rotMat, inverseShadowmapSize));
    shadows.b = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[3].xy, rotMat, inverseShadowmapSize));
    shadows.a = tex2D_inBranch( shadowMapUnit, offsetRotateSample( lightSpacePos.xy, offsets[3].zw, rotMat, inverseShadowmapSize));

    inLight = (lightSpacePos.zzzz < shadows);
    shadow += dot( inLight, half4(0.125, 0.125, 0.125, 0.125) );
    //shadow = 0.001 * shadow + rotations.x;
    return shadow;
}
