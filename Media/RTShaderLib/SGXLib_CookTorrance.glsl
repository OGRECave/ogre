// This file is part of the OGRE project.
// code adapted from Google Filament
// SPDX-License-Identifier: Apache-2.0

#include "RTSLib_Lighting.glsl"

#ifdef HAVE_AREA_LIGHTS
#include "RTSLib_LTC.glsl"
#endif

#ifdef OGRE_GLSLES
    // min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
    #define MIN_PERCEPTUAL_ROUGHNESS 0.089
#else
    #define MIN_PERCEPTUAL_ROUGHNESS 0.045
#endif

#define MEDIUMP_FLT_MAX    65504.0
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)

#define MIN_N_DOT_V 1e-4

struct PixelParams
{
    vec3 baseColor;
    vec3 diffuseColor;
    float perceptualRoughness;
    float roughness;
    vec3  f0;
    vec3  dfg;
    vec3  energyCompensation;
};

float clampNoV(float NoV) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

// Computes x^5 using only multiply operations.
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/diffusebrdf
float Fd_Lambert() {
    return 1.0 / M_PI;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/fresnel(specularf)
vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

vec3 computeDiffuseColor(const vec3 baseColor, float metallic) {
    return baseColor.rgb * (1.0 - metallic);
}

vec3 computeF0(const vec3 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambdaV + lambdaL);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    // clamp to the maximum value representable in mediump
    return saturateMediump(v);
}

// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/normaldistributionfunction(speculard)
float D_GGX(float roughness, float NoH, const vec3 h, const vec3 n) {
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"

    // In mediump, there are two problems computing 1.0 - NoH^2
    // 1) 1.0 - NoH^2 suffers floating point cancellation when NoH^2 is close to 1 (highlights)
    // 2) NoH doesn't have enough precision around 1.0
    // Both problem can be fixed by computing 1-NoH^2 in highp and providing NoH in highp as well

    // However, we can do better using Lagrange's identity:
    //      ||a x b||^2 = ||a||^2 ||b||^2 - (a . b)^2
    // since N and H are unit vectors: ||N x H||^2 = 1.0 - NoH^2
    // This computes 1.0 - NoH^2 directly (which is close to zero in the highlights and has
    // enough precision).
    // Overall this yields better performance, keeping all computations in mediump
#ifdef OGRE_GLSLES
    vec3 NxH = cross(n, h);
    float oneMinusNoHSquared = dot(NxH, NxH);
#else
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
#endif

    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / M_PI);
    return saturateMediump(d);
}

vec3 evaluateLight(
                in vec3 vNormal,
                in vec3 viewPos,
                in vec4 lightPos,
                in vec3 lightColor,
                in vec4 pointParams,
                in vec4 vLightDirView,
                in vec4 spotParams,
                in PixelParams pixel)
{
    vec3 vLightView = lightPos.xyz;
    float fLightD = 0.0;

    if (lightPos.w != 0.0)
    {
        vLightView -= viewPos; // to light
        fLightD     = length(vLightView);

        if(fLightD > pointParams.x)
            return vec3_splat(0.0);
    }

	vLightView		   = normalize(vLightView);

	vec3 vNormalView = normalize(vNormal);
	float NoL		 = saturate(dot(vNormalView, vLightView));

    if(NoL <= 0.0)
        return vec3_splat(0.0); // not lit by this light

    // https://google.github.io/filament/Filament.md.html#toc5.6.2
    float f90 = saturate(dot(pixel.f0, vec3_splat(50.0 * 0.33)));

	vec3 vView       = -normalize(viewPos);

    // https://google.github.io/filament/Filament.md.html#materialsystem/standardmodelsummary
    vec3 h    = normalize(vView + vLightView);
    float NoH = saturate(dot(vNormalView, h));
    float NoV = clampNoV(abs(dot(vNormalView, vView)));

    float V = V_SmithGGXCorrelated(pixel.roughness, NoV, NoL);
    vec3 F  = F_Schlick(pixel.f0, f90, NoH);
    float D = D_GGX(pixel.roughness, NoH, h, vNormalView);

    vec3 Fr = (D * V) * F;
    vec3 Fd = pixel.diffuseColor * Fd_Lambert();

    // https://google.github.io/filament/Filament.md.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance
    vec3 color = NoL * lightColor * (Fr * pixel.energyCompensation + Fd);

    color *= getDistanceAttenuation(pointParams.yzw, fLightD);

    if(spotParams.w != 0.0)
    {
        color *= getAngleAttenuation(spotParams.xyz, vLightDirView.xyz, vLightView);
    }

    return color;
}

void PBR_MakeParams(in vec3 baseColor, in vec2 mrParam, inout PixelParams pixel)
{
    baseColor = pow(baseColor, vec3_splat(2.2));
    pixel.baseColor = baseColor;

    float perceptualRoughness = mrParam.x;
    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);

    float metallic = saturate(mrParam.y);
    pixel.f0 = computeF0(baseColor, metallic, 0.04);
    pixel.diffuseColor = computeDiffuseColor(baseColor, metallic);

    pixel.dfg = vec3_splat(0.5); // use full f0 for energy compensation
    pixel.energyCompensation = vec3_splat(0.0); // will be set later
}

#if LIGHT_COUNT > 0
void PBR_Lights(
#ifdef SHADOWLIGHT_COUNT
                in float shadowFactor[SHADOWLIGHT_COUNT],
#endif
#ifdef HAVE_AREA_LIGHTS
                in sampler2D ltcLUT1,
                in sampler2D ltcLUT2,
#endif
                in vec3 vNormal,
                in vec3 viewPos,
                in vec4 ambient,
                in vec4 lightPos[LIGHT_COUNT],
                in vec4 lightColor[LIGHT_COUNT],
                in vec4 pointParams[LIGHT_COUNT],
                in vec4 vLightDirView[LIGHT_COUNT],
                in vec4 spotParams[LIGHT_COUNT],
                in PixelParams pixel,
                inout vec3 vOutColour)
{
    vOutColour = pow(vOutColour, vec3_splat(2.2)); // gamma to linear

    // Energy compensation for multiple scattering in a microfacet model
    // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
    pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);

    for(int i = 0; i < LIGHT_COUNT; i++)
    {
#ifdef HAVE_AREA_LIGHTS
        if(spotParams[i].w == 2.0)
        {
            // rect area light
            vec3 dcol = pixel.diffuseColor;
            vec3 scol = pixel.f0;
            evaluateRectLight(ltcLUT1, ltcLUT2, pixel.roughness, normalize(vNormal), viewPos,
                              lightPos[i].xyz, spotParams[i].xyz, pointParams[i].xyz, scol, dcol);
            vOutColour += lightColor[i].xyz * (scol + dcol) * 4.0;
            continue;
        }
#endif
        vec3 lightVal = evaluateLight(vNormal, viewPos, lightPos[i], lightColor[i].xyz, pointParams[i], vLightDirView[i], spotParams[i],
                        pixel);

#ifdef SHADOWLIGHT_COUNT
        if(i < SHADOWLIGHT_COUNT)
            lightVal *= shadowFactor[i];
#endif
        vOutColour += lightVal;
    }

    vOutColour += pixel.baseColor * pow(ambient.rgb, vec3_splat(2.2));

    // linear to gamma
    vOutColour = pow(vOutColour, vec3_splat(1.0/2.2));

    vOutColour = saturate(vOutColour);
}
#endif