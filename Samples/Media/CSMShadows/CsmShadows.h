/*  Copyright 2010-2012 Matthew Paul Reid
    This file is subject to the terms and conditions defined in
    file 'License.txt', which is part of this source code package.
*/

uniform float4 fixedDepthBias;
uniform float4 gradientScaleBias;
uniform float4 shadowMapSize;



#define BLENDING // enables blending between cascades

#ifdef DEBUG_CSM
    static half3 cascadeColor = float3(1,1,1);
#endif

half getCsmShadowFactor
(
    uniform sampler2D shadowTexture0,
    uniform sampler2D shadowTexture1,
    uniform sampler2D shadowTexture2,
    uniform sampler2D shadowTexture3,
    float4 lightSpacePos0,
    float4 lightSpacePos1,
    float4 lightSpacePos2,
    float4 lightSpacePos3,
    half shadowLightDotLN
)
{
    half factor = 1;

    if (lightSpacePos0.x > 0.01 && lightSpacePos0.y > 0.01 && lightSpacePos0.x < 0.99 && lightSpacePos0.y < 0.99)
    {
        factor = getShadowFactor(shadowTexture0, lightSpacePos0, shadowMapSize.x, shadowMapSize.z,
                                         fixedDepthBias.x, gradientScaleBias.x, shadowLightDotLN);

    #ifdef DEBUG_CSM
        cascadeColor = float3(1,0,0);
    #endif

    #ifdef BLENDING
        half blend = getShadowFactor(shadowTexture1, lightSpacePos1, shadowMapSize.x, shadowMapSize.z,
                                         fixedDepthBias.y, gradientScaleBias.y, shadowLightDotLN);

        half weight = saturate((max( abs(lightSpacePos0.x-0.5), abs(lightSpacePos0.y-0.5)) -0.375) * 8);
        factor = lerp (factor, blend, weight);

        #ifdef DEBUG_CSM
            cascadeColor = lerp(cascadeColor, float3(0,1,0), weight);
        #endif

    #endif

    }
    else if (lightSpacePos1.x > 0.01 && lightSpacePos1.y > 0.01 && lightSpacePos1.x < 0.99 && lightSpacePos1.y < 0.99)
    {
        factor = getShadowFactor(shadowTexture1, lightSpacePos1, shadowMapSize.x, shadowMapSize.z,
                                            fixedDepthBias.y, gradientScaleBias.y, shadowLightDotLN);

    #ifdef DEBUG_CSM
        cascadeColor = float3(0,1,0);
    #endif

    #ifdef BLENDING
        half blend = getShadowFactor(shadowTexture2, lightSpacePos2, shadowMapSize.x, shadowMapSize.z,
                                                fixedDepthBias.z, gradientScaleBias.z, shadowLightDotLN);
        half weight = saturate((max( abs(lightSpacePos1.x-0.5), abs(lightSpacePos1.y-0.5) ) -0.4375) * 16);
        factor = lerp (factor, blend, weight);

        #ifdef DEBUG_CSM
            cascadeColor = lerp(cascadeColor, float3(0,0,1), weight);
        #endif

    #endif
    }
    else if (lightSpacePos2.x > 0.01 && lightSpacePos2.y > 0.01 && lightSpacePos2.x < 0.99 && lightSpacePos2.y < 0.99)
    {
        factor = getShadowFactor(shadowTexture2, lightSpacePos2, shadowMapSize.x, shadowMapSize.z,
                                            fixedDepthBias.z, gradientScaleBias.z, shadowLightDotLN);

    #ifdef DEBUG_CSM
        cascadeColor = float3(0,0,1);
    #endif

    #ifdef BLENDING
        half blend = getShadowFactor(shadowTexture3, lightSpacePos3, shadowMapSize.x, shadowMapSize.z,
                                                    fixedDepthBias.w, gradientScaleBias.w, shadowLightDotLN);

        half weight = saturate((max( abs(lightSpacePos2.x-0.5), abs(lightSpacePos2.y-0.5)) -0.375) * 8);
        factor = lerp (factor, blend, weight);

        #ifdef DEBUG_CSM
            cascadeColor = lerp(cascadeColor, float3(1,1,0), weight);
        #endif
    #endif
    }
    else
    {
        factor = getShadowFactor(shadowTexture3, lightSpacePos3, shadowMapSize.x, shadowMapSize.z,
                                            fixedDepthBias.w, gradientScaleBias.w, shadowLightDotLN);

        // Fade out to edges
        half weight = saturate((max( abs(lightSpacePos3.x-0.5), abs(lightSpacePos3.y-0.5)) -0.375) * 8);
        factor = lerp (factor, 1, weight);

        #ifdef DEBUG_CSM
            cascadeColor = float3(1,1,weight);
        #endif
    }

    return factor;
}
