//
// This fragment shader defines a reference implementation for Physically Based Shading of
// a microfacet surface material defined by a glTF model.
//
// References:
// [1] Real Shading in Unreal Engine 4
//     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// [2] Physically Based Shading at Disney
//     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// [3] README.md - Environment Maps
//     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
// [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
//     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf

const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    #ifdef MANUAL_SRGB
    #ifdef SRGB_FAST_APPROXIMATION
    vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
    #else //SRGB_FAST_APPROXIMATION
    vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
    vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
    #endif //SRGB_FAST_APPROXIMATION
    return vec4(linOut,srgbIn.w);;
    #else //MANUAL_SRGB
    return srgbIn;
    #endif //MANUAL_SRGB
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(vec3 diffuseColor)
{
    return diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(vec3 reflectance0, vec3 reflectance90, float VdotH)
{
    return reflectance0 + (reflectance90 - reflectance0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float visibilityOcclusion(float alphaRoughness, float NdotL, float NdotV)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    return 0.5 / (GGXV + GGXL);
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(float alphaRoughness, float NdotH)
{
    float roughnessSq = alphaRoughness * alphaRoughness;
    float f = (NdotH * roughnessSq - NdotH) * NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

vec3 getPointShade(vec3 diffuseColor, float alphaRoughness, vec3 specularEnvironmentR0, vec3 specularEnvironmentR90, float NdotL, float NdotV, float NdotH, float VdotH)
{
    // If one of the dot products is larger than zero, no division by zero can happen. Avoids black borders.
    if (NdotL > 0.0 || NdotV > 0.0)
    {
        // Calculate the shading terms for the microfacet specular shading model
        vec3 F = specularReflection(specularEnvironmentR0, specularEnvironmentR90, VdotH);
        float Vis = visibilityOcclusion(alphaRoughness, NdotL, NdotV);
        float D = microfacetDistribution(alphaRoughness, NdotH);

        // Calculation of analytical lighting contribution
        vec3 diffuseContrib = (1.0 - F) * diffuse(diffuseColor);
        vec3 specContrib = F * Vis * D;

        // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
        return NdotL * (diffuseContrib + specContrib);
    }

    return vec3(0.0, 0.0, 0.0);
}

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
vec3 getIBLContribution(sampler2D brdfLUT, samplerCube diffuseEnv, samplerCube specularEnv, 
                        vec3 diffuseColor, vec3 specularColor, float perceptualRoughness, float NdotV, vec3 n, vec3 reflection)
{
    float mipCount = 9.0; // resolution of 512x512
    float lod = (perceptualRoughness * mipCount);
    // retrieve a scale and bias to F0. See [1], Figure 3
    vec3 brdf = SRGBtoLINEAR(texture2D(brdfLUT, vec2(NdotV, 1.0 - perceptualRoughness))).rgb;
    vec3 diffuseLight = SRGBtoLINEAR(textureCube(diffuseEnv, n)).rgb;

#ifdef USE_TEX_LOD
    vec3 specularLight = SRGBtoLINEAR(textureCubeLod(specularEnv, reflection, lod)).rgb;
#else
    vec3 specularLight = SRGBtoLINEAR(textureCube(specularEnv, reflection)).rgb;
#endif

    vec3 diffuse = diffuseLight * diffuseColor;
    vec3 specular = specularLight * (specularColor * brdf.x + brdf.y);

    return diffuse + specular;
}

void getMetallicRoughSpecularParams(vec3 baseColor, float metallic, 
    inout float perceptualRoughness, out float alphaRoughness, out vec3 specularColor, out vec3 specularEnvironmentR90)
{
    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);
    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness [2].
    alphaRoughness = perceptualRoughness * perceptualRoughness;

    vec3 f0 = vec3(0.04);
    specularColor = mix(f0, baseColor.rgb, metallic);

    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));
}