/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/

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

    // Sample each of them checking whether the pixel under test is shadowed or not
    float lightMask = (lightSpacePos.z < tex2D_inBranch( shadowMapUnit, lightSpacePos.xy).r);

    // Hack to prevent these getting optimized out, thereby preventing OGRE errors
    lightMask += 0.001 * (0.001*shadowmapSize + inverseShadowmapSize);

    // Get the average
    return lightMask;
}
