@node GLSL
@subsection OpenGL GLSL
OpenGL GLSL has a similar language syntax to HLSL but is tied to the OpenGL API. The are a few benefits over Cg in that it only requires the OpenGL render system plugin, not any additional plugins. Declaring a OpenGL GLSL program is similar to Cg but simpler. Here's an example:
@example
vertex_program myGLSLVertexProgram glsl
{
    source myGLSLVertexProgram.txt
}
@end example
In GLSL, no entry point needs to be defined since it is always 'main()' and there is no target definition since GLSL source is compiled into native GPU code and not intermediate assembly. @*@*

GLSL supports the use of modular shaders.  This means you can write GLSL external functions that can be used in multiple shaders.

@example
vertex_program myExternalGLSLFunction1 glsl
{
    source myExternalGLSLfunction1.txt
}

vertex_program myExternalGLSLFunction2 glsl
{
    source myExternalGLSLfunction2.txt
}

vertex_program myGLSLVertexProgram1 glsl
{
    source myGLSLfunction.txt
    attach myExternalGLSLFunction1 myExternalGLSLFunction2
}

vertex_program myGLSLVertexProgram2 glsl
{
    source myGLSLfunction.txt
    attach myExternalGLSLFunction1
}
@end example

External GLSL functions are attached to the program that needs them by using 'attach' and including the names of all external programs required on the same line separated by spaces.  This can be done for both vertex and fragment programs.

@subheading GLSL Texture Samplers
To pass texture unit index values from the material script to texture samplers in glsl use 'int' type named parameters.  See the example below:@*

excerpt from GLSL example.frag source:
@example
varying vec2 UV;
uniform sampler2D diffuseMap;

void main(void)
{
	gl_FragColor = texture2D(diffuseMap, UV);
}
@end example

In material script:
@example
fragment_program myFragmentShader glsl
{
  source example.frag
}

material exampleGLSLTexturing
{
  technique
  {
    pass
    {
      fragment_program_ref myFragmentShader
      { 
        param_named diffuseMap int 0
      }

      texture_unit 
      {
        texture myTexture.jpg 2d
      }
    }
  }
}
@end example

An index value of 0 refers to the first texture unit in the pass, an index value of 1 refers to the second unit in the pass and so on.@*@*

@subheading Matrix parameters

Here are some examples of passing matrices to GLSL mat2, mat3, mat4 uniforms:

@example
material exampleGLSLmatrixUniforms
{
  technique matrix_passing
  {
    pass examples
    {
      vertex_program_ref myVertexShader
      { 
        // mat4 uniform
        param_named OcclusionMatrix matrix4x4 1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 0 
        // or
        param_named ViewMatrix float16 0 1 0 0  0 0 1 0  0 0 0 1  0 0 0 0 
        
        // mat3
        param_named TextRotMatrix float9 1 0 0  0 1 0  0 0 1  
      }
      
      fragment_program_ref myFragmentShader
      { 
        // mat2 uniform
        param_named skewMatrix float4 0.5 0 -0.5 1.0
      }

    }
  }
}
@end example

@subheading Accessing OpenGL states in GLSL
GLSL can access most of the GL states directly so you do not need to pass these states through @ref{param_named_auto} in the material script.  This includes lights, material state, and all the matrices used in the openGL state i.e. model view matrix, worldview projection matrix etc. @*@*

@subheading Binding vertex attributes
GLSL natively supports automatic binding of the most common incoming per-vertex attributes (e.g. gl_Vertex, gl_Normal, gl_MultiTexCoord0 etc). However, there are some which are not automatically bound, which must be declared in the shader using the 'attribute <type> <name>' syntax, and the vertex data bound to it by Ogre. @*@*

In addition to the built in attributes described in section 7.3 of the GLSL manual, Ogre supports a number of automatically bound custom vertex attributes. There are some drivers that do not behave correctly when mixing built-in vertex attributes like gl_Normal and custom vertex attributes, so for maximum compatibility you may well wish to use all custom attributes in shaders where you need at least one (e.g. for skeletal animation). 
@table @asis
@item vertex
Binds VES_POSITION, declare as 'attribute vec4 vertex;'.
@item normal
Binds VES_NORMAL, declare as 'attribute vec3 normal;'.
@item colour
Binds VES_DIFFUSE, declare as 'attribute vec4 colour;'.
@item secondary_colour
Binds VES_SPECULAR, declare as 'attribute vec4 secondary_colour;'.
@item uv0 - uv7
Binds VES_TEXTURE_COORDINATES, declare as 'attribute vec4 uv0;'. Note that uv6 and uv7 share attributes with tangent and binormal respectively so cannot both be present.
@item tangent
Binds VES_TANGENT, declare as 'attribute vec3 tangent;'.
@item binormal
Binds VES_BINORMAL, declare as 'attribute vec3 binormal;'.
@item blendIndices
Binds VES_BLEND_INDICES, declare as 'attribute vec4 blendIndices;'.
@item blendWeights
Binds VES_BLEND_WEIGHTS, declare as 'attribute vec4 blendWeights;'.

@subheading Preprocessor definitions
GLSL supports using preprocessor definitions in your code - some are defined by the implementation, but you can also define your own, say in order to use the same source code for a few different variants of the same technique. In order to use this feature, include preprocessor conditions in your GLSL code, of the kind #ifdef SYMBOL, #if SYMBOL==2 etc. Then in your program definition, use the 'preprocessor_defines' option, following it with a string if definitions.  Definitions are separated by ';' or ',' and may optionally have a '=' operator within them to specify a definition value. Those without an '=' will implicitly have a definition of 1. For example:@*@*
@example
// in your GLSL

#ifdef CLEVERTECHNIQUE
	// some clever stuff here
#else
	// normal technique
#endif

#if NUM_THINGS==2
	// Some specific code
#else
	// something else
#endif

// in  your program definition
preprocessor_defines CLEVERTECHNIQUE,NUMTHINGS=2
@end example
This way you can use the same source code but still include small variations, each one defined as a different Ogre program name but based on the same source code.

@subheading GLSL Geometry shader specification
GLSL allows the same shader to run on different types of geometry primitives. In order to properly link the shaders together, you have to specify which primitives it will receive as input, which primitives it will emit and how many vertices a single run of the shader can generate. The GLSL geometry_program definition requires three additional parameters :
@table @asis
@item input_operation_type
The operation type of the geometry that the shader will receive. Can be 'point_list', 'line_list', 'line_strip', 'triangle_list', 'triangle_strip' or 'triangle_fan'.
@item output_operation_type
The operation type of the geometry that the shader will emit. Can be 'point_list', 'line_strip' or 'triangle_strip'.
@item max_output_vertices
The maximum number of vertices that the shader can emit. There is an upper limit for this value, it is exposed in the render system capabilities.

For example:
@example
geometry_program Ogre/GPTest/Swizzle_GP_GLSL glsl
{
	source SwizzleGP.glsl
	input_operation_type triangle_list
	output_operation_type line_strip
	max_output_vertices 6
}
@end example