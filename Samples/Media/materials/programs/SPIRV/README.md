To compile these shaders install `glslangValidator` and run `compile.sh <shader>`.

Then reference them in `BlackAndWhite.material` and `StdQuad_vp.program` by e.g. modifying the GLSL declaration as

```
fragment_program Ogre/Compositor/B&W_GLSL_FP spirv
{
	source GrayScale.frag.spv
}
```

to mix SPIRV and GLSL shaders, `RSC_GLSL_SSO_REDECLARE` must be enabled.