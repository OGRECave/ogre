
struct FrameUniform
{
    mat4 viewFromWorldMatrix;   // View matrix
    mat4 clipFromViewMatrix;    // Projection matrix
    mat4 worldFromViewMatrix;   // Inverse view matrix
    mat4 viewFromClipMatrix;    // Inverse projection matrix
    mat4 clipFromWorldMatrix;   // View and projection matrix concatenated
    mat4 worldFromClipMatrix;   // Inverse of concatenated view and projection matrices
    mat4 lightFromWorldMatrix;
    vec2 iblMaxMipLevel; // x = max mip, y = image resolution
    // when reading from the cubemap, we are not pre-exposed so we apply iblLuminance
    // which is not the case when we'll read from the screen-space buffer
    float iblLuminance;
    // light_punctual :: getPointLight/getSpotLight :: computePreExposedIntensity
    // shading_lit :: addEmissive
    float exposure;
    // shading_parameters :: computeShadingParams
    vec3 cameraPosition;
    // Computes the camera's EV100 from exposure settings
    // aperture in f-stops
    // shutterSpeed in seconds
    // sensitivity in ISO
    // log2((aperture * aperture) / shutterSpeed * 100.0 / sensitivity);
    // Only used when MATERIAL_HAS_EMISSIVE defined.
    float ev100;
    // Never actually used but defined in common_getters.fs
    vec3 worldOffset;
    // Used by dithering.fs (so not in use atm)
    float time;
    // Never actually used but defined in common_getters.fs
    vec4 userTime;
    // Used by SSAO (zw might be as view bounds) and dithering.fs
    // post_process.vs uses the .xy components and light_punctual.fs uses .y when using froxels.
    // So we never actually use this.
    vec4 resolution;
    float viewportWidth;
    float viewportHeight;
    // froxel specific values that we don't use
    vec2 origin;
    float oneOverFroxelDimension;
    float oneOverFroxelDimensionY;
    vec4 zParams;
    // getFroxelIndex
    uint fParamsX;
    uvec2 fParams;

    vec3 iblSH[9]; // Spherical harmonics
};

uniform FrameUniform frameUniforms;
