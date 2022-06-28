To use these shaders, enable the GLSLang Plugin and reference them in `BlackAndWhite.material` and `StdQuad_vp.program` by e.g. modifying the GLSL declaration as

```
fragment_program Ogre/Compositor/B&W_GLSL_FP glslang
{
    source GrayScale.frag
}

...

vertex_program Ogre/Compositor/StdQuad_Tex2a_GLSL_vp glslang
{
    source StdQuad_Tex2a_vp.vert
    default_params
    {
		    param_indexed_auto 0 worldviewproj_matrix
    }
}
```

For SPIRV support, the `"Separate Shader Objects"` option (GL3+) must be enabled. This also allows mixing GLSL and SPIRV shaders in one pass.

To manually compile these shaders install `glslangValidator` and run `compile.sh <shader>`.