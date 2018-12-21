#ifndef GL_ES
#version 120
#ifdef USE_TEX_LOD
#extension GL_ARB_shader_texture_lod : require  
#endif
#else
#version 100
#extension GL_OES_standard_derivatives : enable
#extension GL_EXT_shader_texture_lod: enable
#define textureCubeLod textureLodEXT
precision highp float;
#endif

#include "SGXLib_CookTorrance.glsl"

uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;

#ifdef USE_IBL
uniform samplerCube u_DiffuseEnvSampler;
uniform samplerCube u_SpecularEnvSampler;
uniform sampler2D u_brdfLUT;
#endif

#ifdef HAS_BASECOLORMAP
uniform sampler2D u_BaseColorSampler;
#endif
#ifdef HAS_NORMALMAP
uniform sampler2D u_NormalSampler;
uniform float u_NormalScale;
#endif
#ifdef HAS_EMISSIVEMAP
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

uniform vec3 u_Camera;

// debugging flags used for shader output of intermediate PBR variables
uniform vec4 u_ScaleDiffBaseMR;
uniform vec4 u_ScaleFGDSpec;
uniform vec4 u_ScaleIBLAmbient;

varying vec3 v_Position;

varying vec2 v_UV;

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
varying mat3 v_TBN;
#else
varying vec3 v_Normal;
#endif
#endif

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
    // Retrieve the tangent space matrix
#ifndef HAS_TANGENTS
    vec3 pos_dx = dFdx(v_Position);
    vec3 pos_dy = dFdy(v_Position);
    vec3 tex_dx = dFdx(vec3(v_UV, 0.0));
    vec3 tex_dy = dFdy(vec3(v_UV, 0.0));
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);

#ifdef HAS_NORMALS
    vec3 ng = normalize(v_Normal);
#else
    vec3 ng = cross(pos_dx, pos_dy);
#endif

    t = normalize(t - ng * dot(ng, t));
    vec3 b = normalize(cross(ng, t));
    mat3 tbn = mat3(t, b, ng);
#else // HAS_TANGENTS
    mat3 tbn = v_TBN;
#endif

#ifdef HAS_NORMALMAP
    vec3 n = texture2D(u_NormalSampler, v_UV).rgb;
    n = normalize(tbn * ((2.0 * n - 1.0) * vec3(u_NormalScale, u_NormalScale, 1.0)));
#else
    vec3 n = tbn[2].xyz;
#endif

    return n;
}

void main()
{
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
    float perceptualRoughness = u_MetallicRoughnessValues.y;
    float metallic = u_MetallicRoughnessValues.x;
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture2D(u_MetallicRoughnessSampler, v_UV);
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
#endif

    // The albedo may be defined from a base texture or a flat color
#ifdef HAS_BASECOLORMAP
    vec4 baseColor = SRGBtoLINEAR(texture2D(u_BaseColorSampler, v_UV)) * u_BaseColorFactor;
#else
    vec4 baseColor = u_BaseColorFactor;
#endif

    float alphaRoughness;
    vec3 specularColor;
    vec3 specularEnvironmentR90;
    getMetallicRoughSpecularParams(baseColor.rgb, metallic, perceptualRoughness, alphaRoughness, specularColor, specularEnvironmentR90);
    vec3 diffuseColor = baseColor.rgb - specularColor;

    vec3 n = getNormal();                             // normal at surface point
    vec3 v = normalize(u_Camera - v_Position);        // Vector from surface point to camera
    vec3 l = normalize(u_LightDirection);             // Vector from surface point to light
    vec3 h = normalize(l+v);                          // Half vector between both l and v
    vec3 reflection = -normalize(reflect(v, n));

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = abs(dot(n, v)) + 0.001;
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    vec3 color = u_LightColor * getPointShade(diffuseColor, alphaRoughness, specularColor, specularEnvironmentR90, NdotL, NdotV, NdotH, VdotH);

    // Calculate lighting contribution from image based lighting source (IBL)
#ifdef USE_IBL
    color += getIBLContribution(u_brdfLUT, u_DiffuseEnvSampler, u_SpecularEnvSampler, diffuseColor, specularColor, perceptualRoughness, NdotV, n, reflection);
#endif

    // Apply optional PBR terms for additional (optional) shading
#ifdef HAS_OCCLUSIONMAP
    float ao = texture2D(u_OcclusionSampler, v_UV).r;
    color = mix(color, color * ao, u_OcclusionStrength);
#endif

#ifdef HAS_EMISSIVEMAP
    vec3 emissive = SRGBtoLINEAR(texture2D(u_EmissiveSampler, v_UV)).rgb * u_EmissiveFactor;
    color += emissive;
#endif

    // This section uses mix to override final color for reference app visualization
    // of various parameters in the lighting equation.
    /*color = mix(color, F, u_ScaleFGDSpec.x);
    color = mix(color, vec3(G), u_ScaleFGDSpec.y);
    color = mix(color, vec3(D), u_ScaleFGDSpec.z);
    color = mix(color, specContrib, u_ScaleFGDSpec.w);

    color = mix(color, diffuseContrib, u_ScaleDiffBaseMR.x);*/
    color = mix(color, baseColor.rgb, u_ScaleDiffBaseMR.y);
    color = mix(color, vec3(metallic), u_ScaleDiffBaseMR.z);
    color = mix(color, vec3(perceptualRoughness), u_ScaleDiffBaseMR.w);

    gl_FragColor = vec4(pow(color,vec3(1.0/2.2)), baseColor.a);
}
