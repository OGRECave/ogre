struct ObjectUniform
{
    mat4 worldFromModelMatrix;  // World matrix
    mat3 worldFromModelNormalMatrix; // This would be inverse transpose world matrix with only upper left 3x3 but Ogre doesn't provide one so we use the one above.
#if defined(HAS_SKINNING_OR_MORPHING)
    vec4 morphWeights;          // Parametric animation value
    int morphingEnabled;
    int skinningEnabled;
#endif
};

uniform ObjectUniform objectUniforms;
