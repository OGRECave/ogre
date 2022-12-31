// This file is part of the OGRE project.
// code adapted from Google Filament
// SPDX-License-Identifier: Apache-2.0

vec3 specularDFG(const vec3 dfg, vec3 f0) {
    return mix(dfg.xxx, dfg.yyy, f0);
}

vec3 decodeDataForIBL(const vec4 data) {
    return pow(data.rgb, vec3_splat(2.2)); // gamma to linear
}

vec3 Irradiance_RoughnessOne(samplerCube light_iblSpecular, const vec3 n, float iblRoughnessOneLevel) {
    // note: lod used is always integer, hopefully the hardware skips tri-linear filtering
    return decodeDataForIBL(textureCubeLod(light_iblSpecular, n, iblRoughnessOneLevel));
}

vec3 PrefilteredDFG_LUT(sampler2D light_iblDFG, float lod, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    return texture2DLod(light_iblDFG, vec2(NoV, lod), 0.0).rgb;
}

float perceptualRoughnessToLod(float iblRoughnessOneLevel, float perceptualRoughness) {
    // The mapping below is a quadratic fit for log2(perceptualRoughness)+iblRoughnessOneLevel when
    // iblRoughnessOneLevel is 4. We found empirically that this mapping works very well for
    // a 256 cubemap with 5 levels used. But also scales well for other iblRoughnessOneLevel values.
    return iblRoughnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
}

vec3 prefilteredRadiance(samplerCube light_iblSpecular, const vec3 r, float perceptualRoughness, float iblRoughnessOneLevel) {
    float lod = perceptualRoughnessToLod(iblRoughnessOneLevel, perceptualRoughness);
    return decodeDataForIBL(textureCubeLod(light_iblSpecular, r, lod));
}

vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

void evaluateIBL(in vec3 baseColor,
                 in vec2 mrParam,
                 in vec3 vNormal,
                 in vec3 viewPos,
                 in mat4 invViewMat,
                 in sampler2D dfgTex,
                 in samplerCube iblEnvTex,
                 in float iblRoughnessOneLevel,
                 in float iblLuminance,
                 inout vec3 color)
{
    // gamma to linear
    baseColor = pow(baseColor, vec3_splat(2.2));

    float perceptualRoughness = mrParam.x;
    float metallic = saturate(mrParam.y);

    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    float roughness = perceptualRoughnessToRoughness(perceptualRoughness);

    vec3 f0 = computeF0(baseColor, metallic, 0.04);
    vec3 diffuseColor = computeDiffuseColor(baseColor, metallic);

    vec3 shading_normal = normalize(vNormal);
    vec3 shading_view = -normalize(viewPos);
    float shading_NoV = clampNoV(abs(dot(shading_normal, shading_view)));

    // the above is currently duplicated with CookTorrance

    vec3 shading_reflected = reflect(-shading_view, shading_normal);

    // Pre-filtered DFG term used for image-based lighting
    vec3 dfg = PrefilteredDFG_LUT(dfgTex, perceptualRoughness, shading_NoV);

    vec3 E = specularDFG(dfg, f0);
    vec3 r = getSpecularDominantDirection(shading_normal, shading_reflected, roughness);

    // OGRE specific: convert r and n back to world space for texture sampling
    r = normalize(mul(invViewMat, vec4(r, 0.0)).xyz);
    r.z *= -1.0;
    shading_normal = normalize(mul(invViewMat, vec4(shading_normal, 0.0)).xyz);

    // specular layer
    vec3 Fr = E * prefilteredRadiance(iblEnvTex, r, perceptualRoughness, iblRoughnessOneLevel);

    vec3 diffuseIrradiance = Irradiance_RoughnessOne(iblEnvTex, shading_normal, iblRoughnessOneLevel);
    vec3 Fd = diffuseColor * diffuseIrradiance * (1.0 - E);

    Fr *= iblLuminance;
    Fd *= iblLuminance;

    // Combine all terms
    // Note: iblLuminance is already premultiplied by the exposure

    color = pow(color, vec3_splat(2.2)); // gamma to linear

    color += Fr + Fd;

    // linear to gamma
    color = pow(color, vec3_splat(1.0/2.2));
    //color = saturate(color);
}