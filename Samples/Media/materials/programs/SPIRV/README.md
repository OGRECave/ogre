To compile these shaders install `glslangValidator` and run `compile.sh <shader>`.

Then reference them in `BlackAndWhite.material` and `StdQuad_vp.program` by e.g. modifying the GLSL declaration as

```
fragment_program Ogre/Compositor/B&W_GLSL_FP spirv
{
    source GrayScale.frag.spv
}

...

vertex_program Ogre/Compositor/StdQuad_Tex2a_GLSL_vp spirv
{
    source StdQuad_Tex2a_vp.vert.spv
    default_params
    {
		param_indexed_auto 0 worldviewproj_matrix
    }
}
```

For SPIRV support, the `"Separate Shader Objects"` option (GL3+) must be enabled. This also allows mixing GLSL and SPIRV shaders in one pass.