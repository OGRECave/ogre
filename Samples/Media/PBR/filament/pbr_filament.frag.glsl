#version 330

// Apparently needed by layout(location = ) syntax
#extension GL_ARB_separate_shader_objects : enable

// #include "common_types.fs"

uniform vec4 u_lightPositionFalloff[LIGHT_COUNT]; // Light position world space
uniform vec4 u_lightColorIntensity[LIGHT_COUNT]; // Light color, .w = attenuation multiplier
uniform vec4 u_lightDirection[LIGHT_COUNT]; // Light direction
uniform vec4 u_lightSpotlightParams[LIGHT_COUNT]; // Spotlight parameters, Vector4(innerFactor, outerFactor, falloff, isSpot) innerFactor and outerFactor are cos(angle/2) The isSpot parameter is 0.0f for non-spotlights, 1.0f for spotlights.

#ifdef HAS_BASECOLORMAP
uniform sampler2D u_BaseColorSampler;
#endif
#ifdef MATERIAL_HAS_NORMAL
uniform sampler2D u_NormalSampler;
uniform float u_NormalScale;
#endif
#ifdef MATERIAL_HAS_EMISSIVE
uniform sampler2D u_EmissiveSampler;
uniform vec3 u_EmissiveFactor;
#endif
#ifdef HAS_METALROUGHNESSMAP
uniform sampler2D u_MetallicRoughnessSampler;
#endif
#ifdef HAS_OCCLUSIONMAP
uniform sampler2D u_OcclusionSampler;
uniform float u_OcclusionStrength;
#endif

uniform vec2 u_MetallicRoughnessValues;
uniform vec4 u_BaseColorFactor;
#ifdef MATERIAL_HAS_CLEAR_COAT
uniform vec2 u_clearCoatRoughnessValues;
#endif
#ifdef MATERIAL_HAS_ANISOTROPY
uniform float u_anisotropy;
#endif

uniform sampler2D light_iblDFG;
uniform samplerCube light_iblSpecular;
#ifdef HAS_SSAO
uniform sampler2D light_ssao;
#endif

#include "pbr_filament_frameuniforms.glsl"
#include "pbr_filament_lightuniforms.glsl"

in highp vec3 vertex_worldPosition;
#if defined(HAS_ATTRIBUTE_TANGENTS)
in mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
in mediump vec3 vertex_worldTangent;
in mediump vec3 vertex_worldBitangent;
#endif
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
in mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
in highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
in highp vec4 vertex_uv01;
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
in highp vec4 vertex_lightSpacePosition;
#endif

out vec4 fragColor;

// This exposes "global" variables, some of which will be
// initialized by the prepareMaterial()
#include "common_shading.fs"
#include "common_math.fs"
#include "common_material.fs"
#include "common_graphics.fs"
#include "common_lighting.fs"

#include "tone_mapping.fs"
#include "conversion_functions.fs"

#include "material_inputs.fs"

#include "getters.fs"

#include "shading_parameters.fs"

#include "ambient_occlusion.fs"

#include "brdf.fs"

#ifdef SHADING_MODEL_CLOTH
#include "shading_model_cloth.fs"
#else
#include "shading_model_standard.fs"
#endif

#include "light_indirect.fs"
#include "light_punctual.fs"

// Contains the root evaluateMaterial() function that calls everything else.
#include "shading_lit.fs"

float sRGB_OECF(const float sRGB) {
    // IEC 61966-2-1:1999
    float linearLow = sRGB / 12.92;
    float linearHigh = pow((sRGB + 0.055) / 1.055, 2.4);
    return sRGB <= 0.04045 ? linearLow : linearHigh;
}

/**
 * Reverse opto-electronic conversion function to the one that filament
 * provides. Filament version has LDR RGB linear color -> LDR RGB non-linear
 * color in sRGB space. This function will thus provide LDR RGB non-linear
 * color in sRGB space -> LDR RGB linear color conversion.
 */
vec3 sRGB_OECF(const vec3 sRGB)
{
    return vec3(sRGB_OECF(sRGB.r), sRGB_OECF(sRGB.g), sRGB_OECF(sRGB.b));
}

void material(inout MaterialInputs material)
{
#ifdef HAS_BASECOLORMAP
    vec4 baseColor = texture(u_BaseColorSampler, vertex_uv01.xy);
    baseColor *= u_BaseColorFactor;
    baseColor.rgb = sRGB_OECF(baseColor.rgb);
#else
    vec4 baseColor = u_BaseColorFactor;
#endif
    material.baseColor = baseColor;

    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
    float perceptualRoughness = u_MetallicRoughnessValues.y;
    float metallic = u_MetallicRoughnessValues.x;
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(u_MetallicRoughnessSampler, vertex_uv01.xy);
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
#endif
#ifndef SHADING_MODEL_CLOTH
    material.roughness = perceptualRoughness;
    material.metallic = metallic;
#endif

    // TODO Add reflectance.
    // material.reflectance
#ifdef HAS_OCCLUSIONMAP
    material.ambientOcclusion = texture(u_OcclusionSampler, vertex_uv01.zw).r * u_OcclusionStrength;
#endif
#ifdef MATERIAL_HAS_EMISSIVE
    material.emissive = texture(u_EmissiveSampler, vertex_uv01.xy);
    material.emissive.rgb *= u_EmissiveFactor;
    material.emissive.rgb = sRGB_OECF(material.emissive.rgb);
#endif
#ifdef MATERIAL_HAS_NORMAL
    material.normal = texture2D(u_NormalSampler, vertex_uv01.xy).xyz;
    // Z is assumed to be always positive from range 0 to 1 like defaulted by the NVIDIA Normal Map Filter.
    material.normal.xy = (2.0 * material.normal.xy - 1.0) * vec2(u_NormalScale);
#endif
#ifdef MATERIAL_HAS_CLEAR_COAT
    float clearCoat = u_clearCoatRoughnessValues.x;
    float clearCoatRoughness = u_clearCoatRoughnessValues.y;
#ifndef HAS_METALROUGHNESSMAP
    vec4 mrSample = texture(u_MetallicRoughnessSampler, vertex_uv01.xy);
#endif
    clearCoat = mrSample.a * clearCoat;
    clearCoatRoughness = mrSample.r * clearCoatRoughness;
    material.clearCoat = clearCoat;
    material.clearCoatRoughness = clearCoatRoughness;
#endif
#ifdef MATERIAL_HAS_ANISOTROPY
    material.anisotropy = u_anisotropy;
    material.anisotropyDirection = normalize(vertex_worldNormal);
#endif

    // Must be invoked after setting material.normal.
    prepareMaterial(material);
}

// Mainly from main.fs

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
void blendPostLightingColor(const MaterialInputs material, inout vec4 color) {
#if defined(POST_LIGHTING_BLEND_MODE_OPAQUE)
    color = material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_TRANSPARENT)
    color = material.postLightingColor + color * (1.0 - material.postLightingColor.a);
#elif defined(POST_LIGHTING_BLEND_MODE_ADD)
    color += material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_MULTIPLY)
    color *= material.postLightingColor;
#elif defined(POST_LIGHTING_BLEND_MODE_SCREEN)
    color += material.postLightingColor * (1.0 - color);
#endif
}
#endif

void main()
{
    // See shading_parameters.fs
    // Computes global variables we need to evaluate material and lighting
    computeShadingParams();

    // Initialize the inputs to sensible default values, see material_inputs.fs
    MaterialInputs inputs;
    initMaterial(inputs);

    // Invoke user code
    material(inputs);

    fragColor = evaluateMaterial(inputs);

#if defined(MATERIAL_HAS_POST_LIGHTING_COLOR)
    blendPostLightingColor(inputs, fragColor);
#endif

    // Convert from linear HDR pre-exposed to linear LDR.
    fragColor.rgb = OECF(tonemap(fragColor.rgb));
}
