# High-level Programs {#High-level-Programs}

Support for high level vertex and fragment programs is provided through plugins; this is to make sure that an application using OGRE can use as little or as much of the high-level program functionality as they like. OGRE currently supports 3 high-level program types, Cg ([Cg](#Cg)) (an API- and card-independent, high-level language which lets you write programs for both OpenGL and DirectX for lots of cards), DirectX 9 High-Level Shader Language ([HLSL](#HLSL)), and OpenGL Shader Language ([GLSL](#GLSL)). HLSL can only be used with the DirectX rendersystem, and GLSL can only be used with the GL rendersystem. Cg can be used with both, although experience has shown that more advanced programs, particularly fragment programs which perform a lot of texture fetches, can produce better code in the rendersystem-specific shader language. 

One way to support both HLSL and GLSL is to include separate techniques in the material script, each one referencing separate programs. However, if the programs are basically the same, with the same parameters, and the techniques are complex this can bloat your material scripts with duplication fairly quickly. Instead, if the only difference is the language of the vertex & fragment program you can use OGRE’s [Unified High-level Programs](#Unified-High_002dlevel-Programs) to automatically pick a program suitable for your rendersystem whilst using a single technique.

@tableofcontents

# Skeletal Animation in Vertex Programs {#Skeletal-Animation-in-Vertex-Programs}

You can implement skeletal animation in hardware by writing a vertex program which uses the per-vertex blending indices and blending weights, together with an array of world matrices (which will be provided for you by Ogre if you bind the automatic parameter ’world\_matrix\_array\_3x4’). However, you need to communicate this support to Ogre so it does not perform skeletal animation in software for you. You do this by adding the following attribute to your vertex\_program definition:

```cpp
   includes_skeletal_animation true
```

When you do this, any skeletally animated entity which uses this material will forgo the usual animation blend and will expect the vertex program to do it, for both vertex positions and normals. Note that ALL submeshes must be assigned a material which implements this, and that if you combine skeletal animation with vertex animation (See [Animation](#Animation)) then all techniques must be hardware accelerated for any to be.


# Morph Animation in Vertex Programs {#Morph-Animation-in-Vertex-Programs}

You can implement morph animation in hardware by writing a vertex program which linearly blends between the first and second position keyframes passed as positions and the first free texture coordinate set, and by binding the animation\_parametric value to a parameter (which tells you how far to interpolate between the two). However, you need to communicate this support to Ogre so it does not perform morph animation in software for you. You do this by adding the following attribute to your vertex\_program definition:

```cpp
   includes_morph_animation true
```

When you do this, any skeletally animated entity which uses this material will forgo the usual software morph and will expect the vertex program to do it. Note that if your model includes both skeletal animation and morph animation, they must both be implemented in the vertex program if either is to be hardware acceleration. Note that ALL submeshes must be assigned a material which implements this, and that if you combine skeletal animation with vertex animation (See [Animation](#Animation)) then all techniques must be hardware accelerated for any to be.

# Pose Animation in Vertex Programs {#Pose-Animation-in-Vertex-Programs}

You can implement pose animation (blending between multiple poses based on weight) in a vertex program by pulling in the original vertex data (bound to position), and as many pose offset buffers as you’ve defined in your ’includes\_pose\_animation’ declaration, which will be in the first free texture unit upwards. You must also use the animation\_parametric parameter to define the starting point of the constants which will contain the pose weights; they will start at the parameter you define and fill ’n’ constants, where ’n’ is the max number of poses this shader can blend, i.e. the parameter to includes\_pose\_animation.

```cpp
   includes_pose_animation 4
```

Note that ALL submeshes must be assigned a material which implements this, and that if you combine skeletal animation with vertex animation (See [Animation](#Animation)) then all techniques must be hardware accelerated for any to be.

# Vertex texture fetching in vertex programs {#Vertex-texture-fetching-in-vertex-programs}

If your vertex program makes use of [Vertex Texture Fetch](#Vertex-Texture-Fetch), you should declare that with the ’uses\_vertex\_texture\_fetch’ directive. This is enough to tell Ogre that your program uses this feature and that hardware support for it should be checked.

```cpp
   uses_vertex_texture_fetch true
```

# Adjacency information in Geometry Programs {#Adjacency-information-in-Geometry-Programs}

Some geometry programs require adjacency information from the geometry. It means that a geometry shader doesn’t only get the information of the primitive it operates on, it also has access to its neighbours (in the case of lines or triangles). This directive will tell Ogre to send the information to the geometry shader.

```cpp
   uses_adjacency_information true
```

# Vertex Programs With Shadows {#Vertex-Programs-With-Shadows}

When using shadows (See [Shadows](@ref Shadows)), the use of vertex programs can add some additional complexities, because Ogre can only automatically deal with everything when using the fixed-function pipeline. If you use vertex programs, and you are also using shadows, you may need to make some adjustments. 

If you use **stencil shadows**, then any vertex programs which do vertex deformation can be a problem, because stencil shadows are calculated on the CPU, which does not have access to the modified vertices. If the vertex program is doing standard skeletal animation, this is ok (see section above) because Ogre knows how to replicate the effect in software, but any other vertex deformation cannot be replicated, and you will either have to accept that the shadow will not reflect this deformation, or you should turn off shadows for that object. 

If you use **texture shadows**, then vertex deformation is acceptable; however, when rendering the object into a shadow texture (the shadow caster pass), the shadow has to be rendered in a solid colour (linked to the ambient colour for modulative shadows, black for additive shadows). You must therefore provide an alternative vertex program, so Ogre provides you with a way of specifying one to use when rendering the caster, See [Shadows and Vertex Programs](#Shadows-and-Vertex-Programs).

# Cg programs {#Cg}

In order to define Cg programs, you have to have to load Plugin\_CgProgramManager.so/.dll at startup, either through plugins.cfg or through your own plugin loading code. They are very easy to define:

```cpp
fragment_program myCgFragmentProgram cg
{
    source myCgFragmentProgram.cg
    entry_point main
    profiles ps_2_0 arbfp1
}
```

There are a few differences between this and the assembler program - to begin with, we declare that the fragment program is of type ’cg’ rather than ’asm’, which indicates that it’s a high-level program using Cg. The ’source’ parameter is the same, except this time it’s referencing a Cg source file instead of a file of assembler.  Here is where things start to change. Firstly, we need to define an ’entry\_point’, which is the name of a function in the Cg program which will be the first one called as part of the fragment program. Unlike assembler programs, which just run top-to-bottom, Cg programs can include multiple functions and as such you must specify the one which start the ball rolling. Next, instead of a fixed ’syntax’ parameter, you specify one or more ’profiles’; profiles are how Cg compiles a program down to the low-level assembler. The profiles have the same names as the assembler syntax codes mentioned above; the main difference is that you can list more than one, thus allowing the program to be compiled down to more low-level syntaxes so you can write a single high-level program which runs on both D3D and GL. You are advised to just enter the simplest profiles under which your programs can be compiled in order to give it the maximum compatibility. The ordering also matters; if a card supports more than one syntax then the one listed first will be used.

Lastly, there is a final option called ’compile\_arguments’, where you can specify arguments exactly as you would to the cgc command-line compiler, should you wish to.

# DirectX9 HLSL {#HLSL}

DirectX9 HLSL has a very similar language syntax to Cg but is tied to the DirectX API. The only benefit over Cg is that it only requires the DirectX 9 render system plugin, not any additional plugins. Declaring a DirectX9 HLSL program is very similar to Cg. Here’s an example:

```cpp
vertex_program myHLSLVertexProgram hlsl
{
    source myHLSLVertexProgram.txt
    entry_point main
    target vs_2_0
}
```

As you can see, the main syntax is almost identical, except that instead of ’profiles’ with a list of assembler formats, you have a ’target’ parameter which allows a single assembler target to be specified - obviously this has to be a DirectX assembler format syntax code.

**Important Matrix Ordering Note:** One thing to bear in mind is that HLSL allows you to use 2 different ways to multiply a vector by a matrix - mul(v,m) or mul(m,v). The only difference between them is that the matrix is effectively transposed. You should use mul(m,v) with the matrices passed in from Ogre - this agrees with the shaders produced from tools like RenderMonkey, and is consistent with Cg too, but disagrees with the Dx9 SDK and FX Composer which use mul(v,m) - you will have to switch the parameters to mul() in those shaders.

Note that if you use the float3x4 / matrix3x4 type in your shader, bound to an OGRE auto-definition (such as bone matrices) you should use the column\_major\_matrices = false option (discussed below) in your program definition. This is because OGRE passes float3x4 as row-major to save constant space (3 float4’s rather than 4 float4’s with only the top 3 values used) and this tells OGRE to pass all matrices like this, so that you can use mul(m,v) consistently for all calculations. OGRE will also to tell the shader to compile in row-major form (you don’t have to set the /Zpr compile option or \#pragma pack(row-major) option, OGRE does this for you). Note that passing bones in float4x3 form is not supported by OGRE, but you don’t need it given the above.

**Advanced options**<br>

<dl compact="compact">
<dt>preprocessor\_defines &lt;defines&gt;</dt> <dd>

This allows you to define symbols which can be used inside the HLSL shader code to alter the behaviour (through \#ifdef or \#if clauses). Definitions are separated by ’;’ or ’,’ and may optionally have a ’=’ operator within them to specify a definition value. Those without an ’=’ will implicitly have a definition of 1.

</dd> <dt>column\_major\_matrices &lt;true|false&gt;</dt> <dd>

The default for this option is ’true’ so that OGRE passes matrices auto-bound matrices in a form where mul(m,v) works. Setting this option to false does 2 things - it transpose auto-bound 4x4 matrices and also sets the /Zpr (row-major) option on the shader compilation. This means you can still use mul(m,v), but the matrix layout is row-major instead. This is only useful if you need to use bone matrices (float3x4) in a shader since it saves a float4 constant for every bone involved.

</dd> <dt>optimisation\_level &lt;opt&gt;</dt> <dd>

Set the optimisation level, which can be one of ’default’, ’none’, ’0’, ’1’, ’2’, or ’3’. This corresponds to the /O parameter of fxc.exe, except that in ’default’ mode, optimisation is disabled in debug mode and set to 1 in release mode (fxc.exe uses 1 all the time). Unsurprisingly the default value is ’default’. You may want to change this if you want to tweak the optimisation, for example if your shader gets so complex that it will not longer compile without some minimum level of optimisation.

</dd> </dl>


# OpenGL GLSL {#GLSL}

OpenGL GLSL has a similar language syntax to HLSL but is tied to the OpenGL API. The are a few benefits over Cg in that it only requires the OpenGL render system plugin, not any additional plugins. Declaring a OpenGL GLSL program is similar to Cg but simpler. Here’s an example:

```cpp
vertex_program myGLSLVertexProgram glsl
{
    source myGLSLVertexProgram.txt
}
```

In GLSL, no entry point needs to be defined since it is always ’main()’ and there is no target definition since GLSL source is compiled into native GPU code and not intermediate assembly. 

GLSL supports the use of modular shaders. This means you can write GLSL external functions that can be used in multiple shaders.

```cpp
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
```

External GLSL functions are attached to the program that needs them by using ’attach’ and including the names of all external programs required on the same line separated by spaces. This can be done for both vertex and fragment programs.

## GLSL Texture Samplers {#GLSL-Texture-Samplers}

To pass texture unit index values from the material script to texture samplers in glsl use ’int’ type named parameters. See the example below:<br>

excerpt from GLSL example.frag source:

```cpp
varying vec2 UV;
uniform sampler2D diffuseMap;

void main(void)
{
    gl_FragColor = texture2D(diffuseMap, UV);
}
```

In material script:

```cpp
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
```

An index value of 0 refers to the first texture unit in the pass, an index value of 1 refers to the second unit in the pass and so on.

## Matrix parameters {#Matrix-parameters}

Here are some examples of passing matrices to GLSL mat2, mat3, mat4 uniforms:

```cpp
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
```

## Accessing OpenGL states in GLSL {#Accessing-OpenGL-states-in-GLSL}

GLSL can access most of the GL states directly so you do not need to pass these states through [param\_named\_auto](#param_005fnamed_005fauto) in the material script. This includes lights, material state, and all the matrices used in the openGL state i.e. model view matrix, worldview projection matrix etc. 

## Binding vertex attributes {#Binding-vertex-attributes}

GLSL natively supports automatic binding of the most common incoming per-vertex attributes (e.g. gl\_Vertex, gl\_Normal, gl\_MultiTexCoord0 etc). However, there are some which are not automatically bound, which must be declared in the shader using the ’attribute &lt;type&gt; &lt;name&gt;’ syntax, and the vertex data bound to it by Ogre. 

In addition to the built in attributes described in section 7.3 of the GLSL manual, Ogre supports a number of automatically bound custom vertex attributes. There are some drivers that do not behave correctly when mixing built-in vertex attributes like gl\_Normal and custom vertex attributes, so for maximum compatibility you may well wish to use all custom attributes in shaders where you need at least one (e.g. for skeletal animation).

<dl compact="compact">
<dt>vertex</dt> <dd>

Binds VES\_POSITION, declare as ’attribute vec4 vertex;’.

</dd> <dt>normal</dt> <dd>

Binds VES\_NORMAL, declare as ’attribute vec3 normal;’.

</dd> <dt>colour</dt> <dd>

Binds VES\_DIFFUSE, declare as ’attribute vec4 colour;’.

</dd> <dt>secondary\_colour</dt> <dd>

Binds VES\_SPECULAR, declare as ’attribute vec4 secondary\_colour;’.

</dd> <dt>uv0 - uv7</dt> <dd>

Binds VES\_TEXTURE\_COORDINATES, declare as ’attribute vec4 uv0;’. Note that uv6 and uv7 share attributes with tangent and binormal respectively so cannot both be present.

</dd> <dt>tangent</dt> <dd>

Binds VES\_TANGENT, declare as ’attribute vec3 tangent;’.

</dd> <dt>binormal</dt> <dd>

Binds VES\_BINORMAL, declare as ’attribute vec3 binormal;’.

</dd> <dt>blendIndices</dt> <dd>

Binds VES\_BLEND\_INDICES, declare as ’attribute vec4 blendIndices;’.

</dd> <dt>blendWeights</dt> <dd>

Binds VES\_BLEND\_WEIGHTS, declare as ’attribute vec4 blendWeights;’.

</dd> </dl>

## Preprocessor definitions {#Preprocessor-definitions}

GLSL supports using preprocessor definitions in your code - some are defined by the implementation, but you can also define your own, say in order to use the same source code for a few different variants of the same technique. In order to use this feature, include preprocessor conditions in your GLSL code, of the kind \#ifdef SYMBOL, \#if SYMBOL==2 etc. Then in your program definition, use the ’preprocessor\_defines’ option, following it with a string if definitions. Definitions are separated by ’;’ or ’,’ and may optionally have a ’=’ operator within them to specify a definition value. Those without an ’=’ will implicitly have a definition of 1. For example:

```cpp
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
```

This way you can use the same source code but still include small variations, each one defined as a different Ogre program name but based on the same source code.

## GLSL Geometry shader specification {#GLSL-Geometry-shader-specification}

GLSL allows the same shader to run on different types of geometry primitives. In order to properly link the shaders together, you have to specify which primitives it will receive as input, which primitives it will emit and how many vertices a single run of the shader can generate. The GLSL geometry\_program definition requires three additional parameters :

<dl compact="compact">
<dt>input\_operation\_type</dt> <dd>

The operation type of the geometry that the shader will receive. Can be ’point\_list’, ’line\_list’, ’line\_strip’, ’triangle\_list’, ’triangle\_strip’ or ’triangle\_fan’.

</dd> <dt>output\_operation\_type</dt> <dd>

The operation type of the geometry that the shader will emit. Can be ’point\_list’, ’line\_strip’ or ’triangle\_strip’.

</dd> <dt>max\_output\_vertices</dt> <dd>

The maximum number of vertices that the shader can emit. There is an upper limit for this value, it is exposed in the render system capabilities.

</dd> </dl>

For example:

```cpp
geometry_program Ogre/GPTest/Swizzle_GP_GLSL glsl
{
    source SwizzleGP.glsl
    input_operation_type triangle_list
    output_operation_type line_strip
    max_output_vertices 6
}
```

# Unified High-level Programs {#Unified-High_002dlevel-Programs}

As mentioned above, it can often be useful to write both HLSL and GLSL programs to specifically target each platform, but if you do this via multiple material techniques this can cause a bloated material definition when the only difference is the program language. Well, there is another option. You can ’wrap’ multiple programs in a ’unified’ program definition, which will automatically choose one of a series of ’delegate’ programs depending on the rendersystem and hardware support.

```cpp
vertex_program myVertexProgram unified
{
    delegate realProgram1
    delegate realProgram2
    ... etc
}
```

This works for both vertex and fragment programs, and you can list as many delegates as you like - the first one to be supported by the current rendersystem & hardware will be used as the real program. This is almost like a mini-technique system, but for a single program and with a much tighter purpose. You can only use this where the programs take all the same inputs, particularly textures and other pass / sampler state. Where the only difference between the programs is the language (or possibly the target in HLSL - you can include multiple HLSL programs with different targets in a single unified program too if you want, or indeed any number of other high-level programs), this can become a very powerful feature. For example, without this feature here’s how you’d have to define a programmable material which supported HLSL and GLSL:

```cpp
vertex_program myVertexProgramHLSL hlsl
{
    source prog.hlsl
    entry_point main_vp
    target vs_2_0
}
fragment_program myFragmentProgramHLSL hlsl
{
    source prog.hlsl
    entry_point main_fp
    target ps_2_0
}
vertex_program myVertexProgramGLSL glsl
{
    source prog.vert
}
fragment_program myFragmentProgramGLSL glsl
{
    source prog.frag
    default_params
    {
        param_named tex int 0
    }
}
material SupportHLSLandGLSLwithoutUnified
{
    // HLSL technique
    technique
    {
        pass
        {
            vertex_program_ref myVertexProgramHLSL
            {
                param_named_auto worldViewProj world_view_proj_matrix
                param_named_auto lightColour light_diffuse_colour 0
                param_named_auto lightSpecular light_specular_colour 0
                param_named_auto lightAtten light_attenuation 0
            }
            fragment_program_ref myFragmentProgramHLSL
            {
            }
        }
    }
    // GLSL technique
    technique
    {
        pass
        {
            vertex_program_ref myVertexProgramHLSL
            {
                param_named_auto worldViewProj world_view_proj_matrix
                param_named_auto lightColour light_diffuse_colour 0
                param_named_auto lightSpecular light_specular_colour 0
                param_named_auto lightAtten light_attenuation 0
            }
            fragment_program_ref myFragmentProgramHLSL
            {
            }
        }
    }
}
```

And that’s a really small example. Everything you added to the HLSL technique, you’d have to duplicate in the GLSL technique too. So instead, here’s how you’d do it with unified program definitions:

```cpp
vertex_program myVertexProgramHLSL hlsl
{
    source prog.hlsl
    entry_point main_vp
    target vs_2_0
}
fragment_program myFragmentProgramHLSL hlsl
{
    source prog.hlsl
    entry_point main_fp
    target ps_2_0
}
vertex_program myVertexProgramGLSL glsl
{
    source prog.vert
}
fragment_program myFragmentProgramGLSL glsl
{
    source prog.frag
    default_params
    {
        param_named tex int 0
    }
}
// Unified definition
vertex_program myVertexProgram unified
{
    delegate myVertexProgramGLSL
    delegate myVertexProgramHLSL
}
fragment_program myFragmentProgram unified
{
    delegate myFragmentProgramGLSL
    delegate myFragmentProgramHLSL
}
material SupportHLSLandGLSLwithUnified
{
    // HLSL technique
    technique
    {
        pass
        {
            vertex_program_ref myVertexProgram
            {
                param_named_auto worldViewProj world_view_proj_matrix
                param_named_auto lightColour light_diffuse_colour 0
                param_named_auto lightSpecular light_specular_colour 0
                param_named_auto lightAtten light_attenuation 0
            }
            fragment_program_ref myFragmentProgram
            {
            }
        }
    }
}
```

At runtime, when myVertexProgram or myFragmentProgram are used, OGRE automatically picks a real program to delegate to based on what’s supported on the current hardware / rendersystem. If none of the delegates are supported, the entire technique referencing the unified program is marked as unsupported and the next technique in the material is checked fro fallback, just like normal. As your materials get larger, and you find you need to support HLSL and GLSL specifically (or need to write multiple interface-compatible versions of a program for whatever other reason), unified programs can really help reduce duplication.

# Using Vertex/Geometry/Fragment Programs in a Pass {#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass}

Within a pass section of a material script, you can reference a vertex, geometry and / or a fragment program which is been defined in a .program script (See [Declaring Vertex/Geometry/Fragment Programs](@ref Declaring-Vertex_002fGeometry_002fFragment-Programs)). The programs are defined separately from the usage of them in the pass, since the programs are very likely to be reused between many separate materials, probably across many different .material scripts, so this approach lets you define the program only once and use it many times.

As well as naming the program in question, you can also provide parameters to it. Here’s a simple example:

```cpp
vertex_program_ref myVertexProgram
{
    param_indexed_auto 0 worldviewproj_matrix
    param_indexed      4 float4  10.0 0 0 0
}
```

In this example, we bind a vertex program called ’myVertexProgram’ (which will be defined elsewhere) to the pass, and give it 2 parameters, one is an ’auto’ parameter, meaning we do not have to supply a value as such, just a recognised code (in this case it’s the world/view/projection matrix which is kept up to date automatically by Ogre). The second parameter is a manually specified parameter, a 4-element float. The indexes are described later.

The syntax of the link to a vertex program and a fragment or geometry program are identical, the only difference is that ’fragment\_program\_ref’ and ’geometry\_program\_ref’ are used respectively instead of ’vertex\_program\_ref’.

For many situations vertex, geometry and fragment programs are associated with each other in a pass but this is not cast in stone. You could have a vertex program that can be used by several different fragment programs. Another situation that arises is that you can mix fixed pipeline and programmable pipeline (shaders) together. You could use the non-programmable vertex fixed function pipeline and then provide a fragment\_program\_ref in a pass i.e. there would be no vertex\_program\_ref section in the pass. The fragment program referenced in the pass must meet the requirements as defined in the related API in order to read from the outputs of the vertex fixed pipeline. You could also just have a vertex program that outputs to the fragment fixed function pipeline.

The requirements to read from or write to the fixed function pipeline are similar between rendering API’s (DirectX and OpenGL) but how its actually done in each type of shader (vertex, geometry or fragment) depends on the shader language. For HLSL (DirectX API) and associated asm consult MSDN at <http://msdn.microsoft.com/library/>. For GLSL (OpenGL), consult section 7.6 of the GLSL spec 1.1 available at <http://www.opengl.org/registry/>. The built in varying variables provided in GLSL allow your program to read/write to the fixed function pipeline varyings. For Cg consult the Language Profiles section in CgUsersManual.pdf that comes with the Cg Toolkit available at <https://developer.nvidia.com/cg-toolkit>. For HLSL and Cg its the varying bindings that allow your shader programs to read/write to the fixed function pipeline varyings.

## Parameter specification {#Program-Parameter-Specification}

Parameters can be specified using one of 4 commands as shown below. The same syntax is used whether you are defining a parameter just for this particular use of the program, or when specifying the [Default Program Parameters](#Default-Program-Parameters). Parameters set in the specific use of the program override the defaults.

-   [param\_indexed](#param_005findexed)
-   [param\_indexed\_auto](#param_005findexed_005fauto)
-   [param\_named](#param_005fnamed)
-   [param\_named\_auto](#param_005fnamed_005fauto)
-   [shared\_params\_ref](#shared_005fparams_005fref)

<a name="param_005findexed"></a><a name="param_005findexed-1"></a>

## param\_indexed

This command sets the value of an indexed parameter. 

format: param\_indexed &lt;index&gt; &lt;type&gt; &lt;value&gt; example: param\_indexed 0 float4 10.0 0 0 0

The ’index’ is simply a number representing the position in the parameter list which the value should be written, and you should derive this from your program definition. The index is relative to the way constants are stored on the card, which is in 4-element blocks. For example if you defined a float4 parameter at index 0, the next index would be 1. If you defined a matrix4x4 at index 0, the next usable index would be 4, since a 4x4 matrix takes up 4 indexes.

The value of ’type’ can be float4, matrix4x4, float&lt;n&gt;, int4, int&lt;n&gt;. Note that ’int’ parameters are only available on some more advanced program syntaxes, check the D3D or GL vertex / fragment program documentation for full details. Typically the most useful ones will be float4 and matrix4x4. Note that if you use a type which is not a multiple of 4, then the remaining values up to the multiple of 4 will be filled with zeroes for you (since GPUs always use banks of 4 floats per constant even if only one is used).

’value’ is simply a space or tab-delimited list of values which can be converted into the type you have specified.

<a name="param_005findexed_005fauto"></a><a name="param_005findexed_005fauto-1"></a>

## param\_indexed\_auto

This command tells Ogre to automatically update a given parameter with a derived value. This frees you from writing code to update program parameters every frame when they are always changing.

format: param\_indexed\_auto &lt;index&gt; &lt;value\_code&gt; &lt;extra\_params&gt; example: param\_indexed\_auto 0 worldviewproj\_matrix

’index’ has the same meaning as [param\_indexed](#param_005findexed); note this time you do not have to specify the size of the parameter because the engine knows this already. In the example, the world/view/projection matrix is being used so this is implicitly a matrix4x4.

’value\_code’ is one of a list of recognised values:<br>

<dl compact="compact">
<dt>world\_matrix</dt> <dd>

The current world matrix.

</dd> <dt>inverse\_world\_matrix</dt> <dd>

The inverse of the current world matrix.

</dd> <dt>transpose\_world\_matrix</dt> <dd>

The transpose of the world matrix

</dd> <dt>inverse\_transpose\_world\_matrix</dt> <dd>

The inverse transpose of the world matrix

</dd> <dt>world\_matrix\_array\_3x4</dt> <dd>

An array of world matrices, each represented as only a 3x4 matrix (3 rows of 4columns) usually for doing hardware skinning. You should make enough entries available in your vertex program for the number of bones in use, i.e. an array of numBones\*3 float4’s.

</dd> <dt>view\_matrix</dt> <dd>

The current view matrix.

</dd> <dt>inverse\_view\_matrix</dt> <dd>

The inverse of the current view matrix.

</dd> <dt>transpose\_view\_matrix</dt> <dd>

The transpose of the view matrix

</dd> <dt>inverse\_transpose\_view\_matrix</dt> <dd>

The inverse transpose of the view matrix

</dd> <dt>projection\_matrix</dt> <dd>

The current projection matrix.

</dd> <dt>inverse\_projection\_matrix</dt> <dd>

The inverse of the projection matrix

</dd> <dt>transpose\_projection\_matrix</dt> <dd>

The transpose of the projection matrix

</dd> <dt>inverse\_transpose\_projection\_matrix</dt> <dd>

The inverse transpose of the projection matrix

</dd> <dt>worldview\_matrix</dt> <dd>

The current world and view matrices concatenated.

</dd> <dt>inverse\_worldview\_matrix</dt> <dd>

The inverse of the current concatenated world and view matrices.

</dd> <dt>transpose\_worldview\_matrix</dt> <dd>

The transpose of the world and view matrices

</dd> <dt>inverse\_transpose\_worldview\_matrix</dt> <dd>

The inverse transpose of the current concatenated world and view matrices.

</dd> <dt>viewproj\_matrix</dt> <dd>

The current view and projection matrices concatenated.

</dd> <dt>inverse\_viewproj\_matrix</dt> <dd>

The inverse of the view & projection matrices

</dd> <dt>transpose\_viewproj\_matrix</dt> <dd>

The transpose of the view & projection matrices

</dd> <dt>inverse\_transpose\_viewproj\_matrix</dt> <dd>

The inverse transpose of the view & projection matrices

</dd> <dt>worldviewproj\_matrix</dt> <dd>

The current world, view and projection matrices concatenated.

</dd> <dt>inverse\_worldviewproj\_matrix</dt> <dd>

The inverse of the world, view and projection matrices

</dd> <dt>transpose\_worldviewproj\_matrix</dt> <dd>

The transpose of the world, view and projection matrices

</dd> <dt>inverse\_transpose\_worldviewproj\_matrix</dt> <dd>

The inverse transpose of the world, view and projection matrices

</dd> <dt>texture\_matrix</dt> <dd>

The transform matrix of a given texture unit, as it would usually be seen in the fixed-function pipeline. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ texture unit of the pass in question. NB if the given index exceeds the number of texture units available for this pass, then the parameter will be set to Matrix4::IDENTITY.

</dd> <dt>render\_target\_flipping</dt> <dd>

The value use to adjust transformed y position if bypassed projection matrix transform. It’s -1 if the render target requires texture flipping, +1 otherwise.

</dd> <dt>vertex\_winding</dt> <dd>

Indicates what vertex winding mode the render state is in at this point; +1 for standard, -1 for inverted (e.g. when processing reflections).

</dd> <dt>light\_diffuse\_colour</dt> <dd>

The diffuse colour of a given light; this requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light - note that directional lights are always first in the list and always present). NB if there are no lights this close, then the parameter will be set to black.

</dd> <dt>light\_specular\_colour</dt> <dd>

The specular colour of a given light; this requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to black.

</dd> <dt>light\_attenuation</dt> <dd>

A float4 containing the 4 light attenuation variables for a given light. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. The order of the parameters is range, constant attenuation, linear attenuation, quadric attenuation.

</dd> <dt>spotlight\_params</dt> <dd>

A float4 containing the 3 spotlight parameters and a control value. The order of the parameters is cos(inner angle /2 ), cos(outer angle / 2), falloff, and the final w value is 1.0f. For non-spotlights the value is float4(1,0,0,0). This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). If there are less lights than this, the details are like a non-spotlight.

</dd> <dt>light\_position</dt> <dd>

The position of a given light in world space. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. Note that this property will work with all kinds of lights, even directional lights, since the parameter is set as a 4D vector. Point lights will be (pos.x, pos.y, pos.z, 1.0f) whilst directional lights will be (-dir.x, -dir.y, -dir.z, 0.0f). Operations like dot products will work consistently on both.

</dd> <dt>light\_direction</dt> <dd>

The direction of a given light in world space. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. DEPRECATED - this property only works on directional lights, and we recommend that you use light\_position instead since that returns a generic 4D vector.

</dd> <dt>light\_position\_object\_space</dt> <dd>

The position of a given light in object space (i.e. when the object is at (0,0,0)). This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. Note that this property will work with all kinds of lights, even directional lights, since the parameter is set as a 4D vector. Point lights will be (pos.x, pos.y, pos.z, 1.0f) whilst directional lights will be (-dir.x, -dir.y, -dir.z, 0.0f). Operations like dot products will work consistently on both.

</dd> <dt>light\_direction\_object\_space</dt> <dd>

The direction of a given light in object space (i.e. when the object is at (0,0,0)). This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. DEPRECATED, except for spotlights - for directional lights we recommend that you use light\_position\_object\_space instead since that returns a generic 4D vector.

</dd> <dt>light\_distance\_object\_space</dt> <dd>

The distance of a given light from the center of the object - this is a useful approximation to per-vertex distance calculations for relatively small objects. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes.

</dd> <dt>light\_position\_view\_space</dt> <dd>

The position of a given light in view space (i.e. when the camera is at (0,0,0)). This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. Note that this property will work with all kinds of lights, even directional lights, since the parameter is set as a 4D vector. Point lights will be (pos.x, pos.y, pos.z, 1.0f) whilst directional lights will be (-dir.x, -dir.y, -dir.z, 0.0f). Operations like dot products will work consistently on both.

</dd> <dt>light\_direction\_view\_space</dt> <dd>

The direction of a given light in view space (i.e. when the camera is at (0,0,0)). This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light). NB if there are no lights this close, then the parameter will be set to all zeroes. DEPRECATED, except for spotlights - for directional lights we recommend that you use light\_position\_view\_space instead since that returns a generic 4D vector.

</dd> <dt>light\_power</dt> <dd>

The ’power’ scaling for a given light, useful in HDR rendering. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light).

</dd> <dt>light\_diffuse\_colour\_power\_scaled</dt> <dd>

As light\_diffuse\_colour, except the RGB channels of the passed colour have been pre-scaled by the light’s power scaling as given by light\_power.

</dd> <dt>light\_specular\_colour\_power\_scaled</dt> <dd>

As light\_specular\_colour, except the RGB channels of the passed colour have been pre-scaled by the light’s power scaling as given by light\_power.

</dd> <dt>light\_number</dt> <dd>

When rendering, there is generally a list of lights available for use by all of the passes for a given object, and those lights may or may not be referenced in one or more passes. Sometimes it can be useful to know where in that overall list a given light light (as seen from a pass) is. For example if you use iterate once\_per\_light, the pass always sees the light as index 0, but in each iteration the actual light referenced is different. This binding lets you pass through the actual index of the light in that overall list. You just need to give it a parameter of the pass-relative light number and it will map it to the overall list index.

</dd> <dt>light\_diffuse\_colour\_array</dt> <dd>

As light\_diffuse\_colour, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_specular\_colour\_array</dt> <dd>

As light\_specular\_colour, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_diffuse\_colour\_power\_scaled\_array</dt> <dd>

As light\_diffuse\_colour\_power\_scaled, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_specular\_colour\_power\_scaled\_array</dt> <dd>

As light\_specular\_colour\_power\_scaled, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_attenuation\_array</dt> <dd>

As light\_attenuation, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>spotlight\_params\_array</dt> <dd>

As spotlight\_params, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_position\_array</dt> <dd>

As light\_position, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_direction\_array</dt> <dd>

As light\_direction, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_position\_object\_space\_array</dt> <dd>

As light\_position\_object\_space, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_direction\_object\_space\_array</dt> <dd>

As light\_direction\_object\_space, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_distance\_object\_space\_array</dt> <dd>

As light\_distance\_object\_space, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_position\_view\_space\_array</dt> <dd>

As light\_position\_view\_space, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_direction\_view\_space\_array</dt> <dd>

As light\_direction\_view\_space, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_power\_array</dt> <dd>

As light\_power, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>light\_count</dt> <dd>

The total number of lights active in this pass.

</dd> <dt>light\_casts\_shadows</dt> <dd>

Sets an integer parameter to 1 if the given light casts shadows, 0 otherwise, Requires a light index parameter.

</dd> <dt>ambient\_light\_colour</dt> <dd>

The colour of the ambient light currently set in the scene.

</dd> <dt>surface\_ambient\_colour</dt> <dd>

The ambient colour reflectance properties of the pass (See [ambient](#ambient)). This allows you access to fixed-function pipeline property handily.

</dd> <dt>surface\_diffuse\_colour</dt> <dd>

The diffuse colour reflectance properties of the pass (See [diffuse](#diffuse)). This allows you access to fixed-function pipeline property handily.

</dd> <dt>surface\_specular\_colour</dt> <dd>

The specular colour reflectance properties of the pass (See [specular](#specular)). This allows you access to fixed-function pipeline property handily.

</dd> <dt>surface\_emissive\_colour</dt> <dd>

The amount of self-illumination of the pass (See [emissive](#emissive)). This allows you access to fixed-function pipeline property handily.

</dd> <dt>surface\_shininess</dt> <dd>

The shininess of the pass, affecting the size of specular highlights (See [specular](#specular)). This allows you bind to fixed-function pipeline property handily.

</dd> <dt>derived\_ambient\_light\_colour</dt> <dd>

The derived ambient light colour, with ’r’, ’g’, ’b’ components filled with product of surface\_ambient\_colour and ambient\_light\_colour, respectively, and ’a’ component filled with surface ambient alpha component.

</dd> <dt>derived\_scene\_colour</dt> <dd>

The derived scene colour, with ’r’, ’g’ and ’b’ components filled with sum of derived\_ambient\_light\_colour and surface\_emissive\_colour, respectively, and ’a’ component filled with surface diffuse alpha component.

</dd> <dt>derived\_light\_diffuse\_colour</dt> <dd>

The derived light diffuse colour, with ’r’, ’g’ and ’b’ components filled with product of surface\_diffuse\_colour, light\_diffuse\_colour and light\_power, respectively, and ’a’ component filled with surface diffuse alpha component. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light).

</dd> <dt>derived\_light\_specular\_colour</dt> <dd>

The derived light specular colour, with ’r’, ’g’ and ’b’ components filled with product of surface\_specular\_colour and light\_specular\_colour, respectively, and ’a’ component filled with surface specular alpha component. This requires an index in the ’extra\_params’ field, and relates to the ’nth’ closest light which could affect this object (i.e. 0 refers to the closest light).

</dd> <dt>derived\_light\_diffuse\_colour\_array</dt> <dd>

As derived\_light\_diffuse\_colour, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>derived\_light\_specular\_colour\_array</dt> <dd>

As derived\_light\_specular\_colour, except that this populates an array of parameters with a number of lights, and the ’extra\_params’ field refers to the number of ’nth closest’ lights to be processed. This parameter is not compatible with light-based pass\_iteration options but can be used for single-pass lighting.

</dd> <dt>fog\_colour</dt> <dd>

The colour of the fog currently set in the scene.

</dd> <dt>fog\_params</dt> <dd>

The parameters of the fog currently set in the scene. Packed as (exp\_density, linear\_start, linear\_end, 1.0 / (linear\_end - linear\_start)).

</dd> <dt>camera\_position</dt> <dd>

The current cameras position in world space.

</dd> <dt>camera\_position\_object\_space</dt> <dd>

The current cameras position in object space (i.e. when the object is at (0,0,0)).

</dd> <dt>lod\_camera\_position</dt> <dd>

The current LOD camera position in world space. A LOD camera is a separate camera associated with the rendering camera which allows LOD calculations to be calculated separately. The classic example is basing the LOD of the shadow texture render on the position of the main camera, not the shadow camera.

</dd> <dt>lod\_camera\_position\_object\_space</dt> <dd>

The current LOD camera position in object space (i.e. when the object is at (0,0,0)).

</dd> <dt>time</dt> <dd>

The current time, factored by the optional parameter (or 1.0f if not supplied).

</dd> <dt>time\_0\_x</dt> <dd>

Single float time value, which repeats itself based on "cycle time" given as an ’extra\_params’ field

</dd> <dt>costime\_0\_x</dt> <dd>

Cosine of time\_0\_x

</dd> <dt>sintime\_0\_x</dt> <dd>

Sine of time\_0\_x

</dd> <dt>tantime\_0\_x</dt> <dd>

Tangent of time\_0\_x

</dd> <dt>time\_0\_x\_packed</dt> <dd>

4-element vector of time0\_x, sintime0\_x, costime0\_x, tantime0\_x

</dd> <dt>time\_0\_1</dt> <dd>

As time0\_x but scaled to \[0..1\]

</dd> <dt>costime\_0\_1</dt> <dd>

As costime0\_x but scaled to \[0..1\]

</dd> <dt>sintime\_0\_1</dt> <dd>

As sintime0\_x but scaled to \[0..1\]

</dd> <dt>tantime\_0\_1</dt> <dd>

As tantime0\_x but scaled to \[0..1\]

</dd> <dt>time\_0\_1\_packed</dt> <dd>

As time0\_x\_packed but all values scaled to \[0..1\]

</dd> <dt>time\_0\_2pi</dt> <dd>

As time0\_x but scaled to \[0..2\*Pi\]

</dd> <dt>costime\_0\_2pi</dt> <dd>

As costime0\_x but scaled to \[0..2\*Pi\]

</dd> <dt>sintime\_0\_2pi</dt> <dd>

As sintime0\_x but scaled to \[0..2\*Pi\]

</dd> <dt>tantime\_0\_2pi</dt> <dd>

As tantime0\_x but scaled to \[0..2\*Pi\]

</dd> <dt>time\_0\_2pi\_packed</dt> <dd>

As time0\_x\_packed but scaled to \[0..2\*Pi\]

</dd> <dt>frame\_time</dt> <dd>

The current frame time, factored by the optional parameter (or 1.0f if not supplied).

</dd> <dt>fps</dt> <dd>

The current frames per second

</dd> <dt>viewport\_width</dt> <dd>

The current viewport width in pixels

</dd> <dt>viewport\_height</dt> <dd>

The current viewport height in pixels

</dd> <dt>inverse\_viewport\_width</dt> <dd>

1.0/the current viewport width in pixels

</dd> <dt>inverse\_viewport\_height</dt> <dd>

1.0/the current viewport height in pixels

</dd> <dt>viewport\_size</dt> <dd>

4-element vector of viewport\_width, viewport\_height, inverse\_viewport\_width, inverse\_viewport\_height

</dd> <dt>texel\_offsets</dt> <dd>

Provides details of the rendersystem-specific texture coordinate offsets required to map texels onto pixels. float4(horizontalOffset, verticalOffset, horizontalOffset / viewport\_width, verticalOffset / viewport\_height).

</dd> <dt>view\_direction</dt> <dd>

View direction vector in object space

</dd> <dt>view\_side\_vector</dt> <dd>

View local X axis

</dd> <dt>view\_up\_vector</dt> <dd>

View local Y axis

</dd> <dt>fov</dt> <dd>

Vertical field of view, in radians

</dd> <dt>near\_clip\_distance</dt> <dd>

Near clip distance, in world units

</dd> <dt>far\_clip\_distance</dt> <dd>

Far clip distance, in world units (may be 0 for infinite view projection)

</dd> <dt>texture\_viewproj\_matrix</dt> <dd>

Applicable to vertex programs which have been specified as the ’shadow receiver’ vertex program alternative, or where a texture unit is marked as content\_type shadow; this provides details of the view/projection matrix for the current shadow projector. The optional ’extra\_params’ entry specifies which light the projector refers to (for the case of content\_type shadow where more than one shadow texture may be present in a single pass), where 0 is the default and refers to the first light referenced in this pass.

</dd> <dt>texture\_viewproj\_matrix\_array</dt> <dd>

As texture\_viewproj\_matrix, except an array of matrices is passed, up to the number that you specify as the ’extra\_params’ value.

</dd> <dt>texture\_worldviewproj\_matrix</dt> <dd>

As texture\_viewproj\_matrix except it also includes the world matrix.

</dd> <dt>texture\_worldviewproj\_matrix\_array</dt> <dd>

As texture\_worldviewproj\_matrix, except an array of matrices is passed, up to the number that you specify as the ’extra\_params’ value.

</dd> <dt>spotlight\_viewproj\_matrix</dt> <dd>

Provides a view / projection matrix which matches the set up of a given spotlight (requires an ’extra\_params’ entry to indicate the light index, which must be a spotlight). Can be used to project a texture from a given spotlight.

</dd> <dt>spotlight\_viewproj\_matrix\_array</dt> <dd>

As spotlight\_viewproj\_matrix, except an array of matrices is passed, up to the number that you specify as the ’extra\_params’ value.

</dd> <dt>spotlight\_worldviewproj\_matrix</dt> <dd>

As spotlight\_viewproj\_matrix except it also includes the world matrix.

</dd> <dt>scene\_depth\_range</dt> <dd>

Provides information about the depth range as viewed from the current camera being used to render. Provided as float4(minDepth, maxDepth, depthRange, 1 / depthRange).

</dd> <dt>shadow\_scene\_depth\_range</dt> <dd>

Provides information about the depth range as viewed from the shadow camera relating to a selected light. Requires a light index parameter. Provided as float4(minDepth, maxDepth, depthRange, 1 / depthRange).

</dd> <dt>shadow\_colour</dt> <dd>

The shadow colour (for modulative shadows) as set via SceneManager::setShadowColour.

</dd> <dt>shadow\_extrusion\_distance</dt> <dd>

The shadow extrusion distance as determined by the range of a non-directional light or set via SceneManager::setShadowDirectionalLightExtrusionDistance for directional lights.

</dd> <dt>texture\_size</dt> <dd>

Provides texture size of the selected texture unit. Requires a texture unit index parameter. Provided as float4(width, height, depth, 1). For 2D-texture, depth sets to 1, for 1D-texture, height and depth sets to 1.

</dd> <dt>inverse\_texture\_size</dt> <dd>

Provides inverse texture size of the selected texture unit. Requires a texture unit index parameter. Provided as float4(1 / width, 1 / height, 1 / depth, 1). For 2D-texture, depth sets to 1, for 1D-texture, height and depth sets to 1.

</dd> <dt>packed\_texture\_size</dt> <dd>

Provides packed texture size of the selected texture unit. Requires a texture unit index parameter. Provided as float4(width, height, 1 / width, 1 / height). For 3D-texture, depth is ignored, for 1D-texture, height sets to 1. <a name="pass_005fnumber"></a>

</dd> <dt>pass\_number</dt> <dd>

Sets the active pass index number in a gpu parameter. The first pass in a technique has an index of 0, the second an index of 1 and so on. This is useful for multipass shaders (i.e. fur or blur shader) that need to know what pass it is. By setting up the auto parameter in a [Default Program Parameters](#Default-Program-Parameters) list in a program definition, there is no requirement to set the pass number parameter in each pass and lose track. (See [fur\_example](#fur_005fexample)) <a name="pass_005fiteration_005fnumber"></a>

</dd> <dt>pass\_iteration\_number</dt> <dd>

Useful for GPU programs that need to know what the current pass iteration number is. The first iteration of a pass is numbered 0. The last iteration number is one less than what is set for the pass iteration number. If a pass has its iteration attribute set to 5 then the last iteration number (5th execution of the pass) is 4.(See [iteration](#iteration)) <a name="animation_005fparametric"></a>

</dd> <dt>animation\_parametric</dt> <dd>

Useful for hardware vertex animation. For morph animation, sets the parametric value (0..1) representing the distance between the first position keyframe (bound to positions) and the second position keyframe (bound to the first free texture coordinate) so that the vertex program can interpolate between them. For pose animation, indicates a group of up to 4 parametric weight values applying to a sequence of up to 4 poses (each one bound to x, y, z and w of the constant), one for each pose. The original positions are held in the usual position buffer, and the offsets to take those positions to the pose where weight == 1.0 are in the first ’n’ free texture coordinates; ’n’ being determined by the value passed to includes\_pose\_animation. If more than 4 simultaneous poses are required, then you’ll need more than 1 shader constant to hold the parametric values, in which case you should use this binding more than once, referencing a different constant entry; the second one will contain the parametrics for poses 5-8, the third for poses 9-12, and so on.

</dd> <dt>custom</dt> <dd>

This allows you to map a custom parameter on an individual Renderable (see Renderable::setCustomParameter) to a parameter on a GPU program. It requires that you complete the ’extra\_params’ field with the index that was used in the Renderable::setCustomParameter call, and this will ensure that whenever this Renderable is used, it will have it’s custom parameter mapped in. It’s very important that this parameter has been defined on all Renderables that are assigned the material that contains this automatic mapping, otherwise the process will fail.

</dd> </dl> <a name="param_005fnamed"></a><a name="param_005fnamed-1"></a>

## param\_named

This is the same as param\_indexed, but uses a named parameter instead of an index. This can only be used with high-level programs which include parameter names; if you’re using an assembler program then you have no choice but to use indexes. Note that you can use indexed parameters for high-level programs too, but it is less portable since if you reorder your parameters in the high-level program the indexes will change. format: param\_named &lt;name&gt; &lt;type&gt; &lt;value&gt; example: param\_named shininess float4 10.0 0 0 0 The type is required because the program is not compiled and loaded when the material script is parsed, so at this stage we have no idea what types the parameters are. Programs are only loaded and compiled when they are used, to save memory.

<a name="param_005fnamed_005fauto"></a><a name="param_005fnamed_005fauto-1"></a>

## param\_named\_auto

This is the named equivalent of param\_indexed\_auto, for use with high-level programs. Format: param\_named\_auto &lt;name&gt; &lt;value\_code&gt; &lt;extra\_params&gt; Example: param\_named\_auto worldViewProj WORLDVIEWPROJ\_MATRIX

The allowed value codes and the meaning of extra\_params are detailed in [param\_indexed\_auto](#param_005findexed_005fauto).

<a name="shared_005fparams_005fref"></a><a name="shared_005fparams_005fref-1"></a>

## shared\_params\_ref

This option allows you to reference shared parameter sets as defined in [Declaring Shared Parameters](#Declaring-Shared-Parameters).  Format: shared\_params\_ref &lt;shared\_set\_name&gt; Example: shared\_params\_ref mySharedParams

The only required parameter is a name, which must be the name of an already defined shared parameter set. All named parameters which are present in the program that are also present in the shared parameter set will be linked, and the shared parameters used as if you had defined them locally. This is dependent on the definitions (type and array size) matching between the shared set and the program.

# Shadows and Vertex Programs {#Shadows-and-Vertex-Programs}

When using shadows (See [Shadows](@ref Shadows)), the use of vertex programs can add some additional complexities, because Ogre can only automatically deal with everything when using the fixed-function pipeline. If you use vertex programs, and you are also using shadows, you may need to make some adjustments. 

If you use **stencil shadows**, then any vertex programs which do vertex deformation can be a problem, because stencil shadows are calculated on the CPU, which does not have access to the modified vertices. If the vertex program is doing standard skeletal animation, this is ok (see section above) because Ogre knows how to replicate the effect in software, but any other vertex deformation cannot be replicated, and you will either have to accept that the shadow will not reflect this deformation, or you should turn off shadows for that object. 

If you use **texture shadows**, then vertex deformation is acceptable; however, when rendering the object into the shadow texture (the shadow caster pass), the shadow has to be rendered in a solid colour (linked to the ambient colour). You must therefore provide an alternative vertex program, so Ogre provides you with a way of specifying one to use when rendering the caster. Basically you link an alternative vertex program, using exactly the same syntax as the original vertex program link:

```cpp
shadow_caster_vertex_program_ref myShadowCasterVertexProgram
{
    param_indexed_auto 0 worldviewproj_matrix
    param_indexed_auto 4 ambient_light_colour
}
```

When rendering a shadow caster, Ogre will automatically use the alternate program. You can bind the same or different parameters to the program - the most important thing is that you bind **ambient\_light\_colour**, since this determines the colour of the shadow in modulative texture shadows. If you don’t supply an alternate program, Ogre will fall back on a fixed-function material which will not reflect any vertex deformation you do in your vertex program. 

In addition, when rendering the shadow receivers with shadow textures, Ogre needs to project the shadow texture. It does this automatically in fixed function mode, but if the receivers use vertex programs, they need to have a shadow receiver program which does the usual vertex deformation, but also generates projective texture coordinates. The additional program linked into the pass like this:

```cpp
shadow_receiver_vertex_program_ref myShadowReceiverVertexProgram
{
    param_indexed_auto 0 worldviewproj_matrix
    param_indexed_auto 4 texture_viewproj_matrix
}
```

For the purposes of writing this alternate program, there is an automatic parameter binding of ’texture\_viewproj\_matrix’ which provides the program with texture projection parameters. The vertex program should do it’s normal vertex processing, and generate texture coordinates using this matrix and place them in texture coord sets 0 and 1, since some shadow techniques use 2 texture units. The colour of the vertices output by this vertex program must always be white, so as not to affect the final colour of the rendered shadow. 

When using additive texture shadows, the shadow pass render is actually the lighting render, so if you perform any fragment program lighting you also need to pull in a custom fragment program. You use the shadow\_receiver\_fragment\_program\_ref for this:

```cpp
shadow_receiver_fragment_program_ref myShadowReceiverFragmentProgram
{
    param_named_auto lightDiffuse light_diffuse_colour 0 
}
```

You should pass the projected shadow coordinates from the custom vertex program. As for textures, texture unit 0 will always be the shadow texture. Any other textures which you bind in your pass will be carried across too, but will be moved up by 1 unit to make room for the shadow texture. Therefore your shadow receiver fragment program is likely to be the same as the bare lighting pass of your normal material, except that you insert an extra texture sampler at index 0, which you will use to adjust the result by (modulating diffuse and specular components).

# Vertex Texture Fetch {#Vertex-Texture-Fetch}

More recent generations of video card allow you to perform a read from a texture in the vertex program rather than just the fragment program, as is traditional. This allows you to, for example, read the contents of a texture and displace vertices based on the intensity of the colour contained within.

<a name="Declaring-the-use-of-vertex-texture-fetching"></a>

## Declaring the use of vertex texture fetching

Since hardware support for vertex texture fetching is not ubiquitous, you should use the uses\_vertex\_texture\_fetch (See [Vertex texture fetching in vertex programs](#Vertex-texture-fetching-in-vertex-programs)) directive when declaring your vertex programs which use vertex textures, so that if it is not supported, technique fallback can be enabled. This is not strictly necessary for DirectX-targeted shaders, since vertex texture fetching is only supported in vs\_3\_0, which can be stated as a required syntax in your shader definition, but for OpenGL (GLSL), there are cards which support GLSL but not vertex textures, so you should be explicit about your need for them.

<a name="Render-system-texture-binding-differences"></a>

## Render system texture binding differences

Unfortunately the method for binding textures so that they are available to a vertex program is not well standardised. As at the time of writing, Shader Model 3.0 (SM3.0) hardware under DirectX9 include 4 separate sampler bindings for the purposes of vertex textures. OpenGL, on the other hand, is able to access vertex textures in GLSL (and in assembler through NV\_vertex\_program\_3, although this is less popular), but the textures are shared with the fragment pipeline. I expect DirectX to move to the GL model with the advent of DirectX10, since a unified shader architecture implies sharing of texture resources between the two stages. As it is right now though, we’re stuck with an inconsistent situation.

To reflect this, you should use the [binding\_type](#binding_005ftype) attribute in a texture unit to indicate which unit you are targeting with your texture - ’fragment’ (the default) or ’vertex’. For render systems that don’t have separate bindings, this actually does nothing. But for those that do, it will ensure your texture gets bound to the right processing unit.

Note that whilst DirectX9 has separate bindings for the vertex and fragment pipelines, binding a texture to the vertex processing unit still uses up a ’slot’ which is then not available for use in the fragment pipeline. I didn’t manage to find this documented anywhere, but the nVidia samples certainly avoid binding a texture to the same index on both vertex and fragment units, and when I tried to do it, the texture did not appear correctly in the fragment unit, whilst it did as soon as I moved it into the next unit.

<a name="Texture-format-limitations"></a>

## Texture format limitations

Again as at the time of writing, the types of texture you can use in a vertex program are limited to 1- or 4-component, full precision floating point formats. In code that equates to PF\_FLOAT32\_R or PF\_FLOAT32\_RGBA. No other formats are supported. In addition, the textures must be regular 2D textures (no cube or volume maps) and mipmapping and filtering is not supported, although you can perform filtering in your vertex program if you wish by sampling multiple times.

<a name="Hardware-limitations"></a>

## Hardware limitations

As at the time of writing (early Q3 2006), ATI do not support texture fetch in their current crop of cards (Radeon X1n00). nVidia do support it in both their 6n00 and 7n00 range. ATI support an alternative called ’Render to Vertex Buffer’, but this is not standardised at this time and is very much different in its implementation, so cannot be considered to be a drop-in replacement. This is the case even though the Radeon X1n00 cards claim to support vs\_3\_0 (which requires vertex texture fetch).
