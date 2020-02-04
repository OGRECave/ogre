# High-level Programs {#High-level-Programs}

Support for high level vertex and fragment programs is provided through plugins; this is to make sure that an application using OGRE can use as little or as much of the high-level program functionality as they like. OGRE currently supports 3 high-level program types, Cg ([Cg](#Cg)) (an API- and card-independent, high-level language which lets you write programs for both OpenGL and DirectX for lots of cards), DirectX 9 High-Level Shader Language ([HLSL](#HLSL)), and OpenGL Shader Language ([GLSL](#GLSL)). HLSL can only be used with the DirectX rendersystem, and GLSL can only be used with the GL rendersystem. Cg can be used with both, although experience has shown that more advanced programs, particularly fragment programs which perform a lot of texture fetches, can produce better code in the rendersystem-specific shader language. 

One way to support both HLSL and GLSL is to include separate techniques in the material script, each one referencing separate programs. However, if the programs are basically the same, with the same parameters, and the techniques are complex this can bloat your material scripts with duplication fairly quickly. Instead, if the only difference is the language of the vertex & fragment program you can use OGRE’s [Unified High-level Programs](#Unified-High_002dlevel-Programs) to automatically pick a program suitable for your rendersystem whilst using a single technique.

@tableofcontents


# Preprocessor definitions {#Preprocessor-definitions}

Both GLSL and HLSL support using preprocessor definitions in your code - some are defined by the implementation, but you can also define your own, say in order to use the same source code for a few different variants of the same technique. In order to use this feature, include preprocessor conditions in your code, of the kind <tt>\#ifdef SYMBOL</tt>, <tt>\#if SYMBOL==2</tt> etc. Then in your program definition, use the `preprocessor_defines` option, following it with a string of definitions. Definitions are separated by `;` or `,` and may optionally have a `=` operator within them to specify a definition value. Those without an `=` will implicitly have a definition of 1. For example:

```cpp
// in your shader

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

@note on GLSL %Ogre pre-processes the source itself instead on relying on the driver implementation which is often buggy. This relaxes using @c \#ifdef directives compared to the standard - e.g. you can <tt>\#ifdef \#version</tt>. However this means that defines specified in GLSL extensions are not present.

# Cg programs {#Cg}

In order to define Cg programs, you have to have to load Plugin\_CgProgramManager.so/.dll at startup, either through plugins.cfg or through your own plugin loading code. They are very easy to define:

```cpp
fragment_program myCgFragmentProgram cg
{
    source myCgFragmentProgram.cg
    entry_point main_fp
    profiles ps_2_0 arbfp1
}
```

There are a few differences between this and the assembler program - to begin with, we declare that the fragment program is of type `cg` rather than `asm`, which indicates that it’s a high-level program using Cg. The `source` parameter is the same, except this time it’s referencing a Cg source file instead of a file of assembler.  Here is where things start to change. Firstly, we need to define an `entry_point`, which is the name of a function in the Cg program which will be the first one called as part of the fragment program. Unlike assembler programs, which just run top-to-bottom, Cg programs can include multiple functions and as such you must specify the one which start the ball rolling.
If you omit this line, %Ogre will default to looking for a function called `main`.

Next, instead of a fixed `syntax` parameter, you specify one or more `profiles`; profiles are how Cg compiles a program down to the low-level assembler. The profiles have the same names as the assembler syntax codes mentioned above; the main difference is that you can list more than one, thus allowing the program to be compiled down to more low-level syntaxes so you can write a single high-level program which runs on both D3D and GL. You are advised to just enter the simplest profiles under which your programs can be compiled in order to give it the maximum compatibility. The ordering also matters; if a card supports more than one syntax then the one listed first will be used.

@note Instead of `preprocessor_defines` Cg uses the `compile_arguments` option where you can specify arguments exactly as you would to the [cgc command-line compiler](http://developer.download.nvidia.com/cg/cgc.html). While this gives you more flexibility, it means that you must specify the defines as `-DSYMBOL` separated by spaces. Keep this in mind when copying program definitions across the supported languages.

# DirectX HLSL {#HLSL}

DirectX HLSL has an almost identical language syntax to Cg but is tied to the DirectX API. The benefit over Cg is that it only requires the DirectX render system plugin, not any additional plugins. Declaring a DirectX HLSL program is very similar to Cg. Here’s an example:

```cpp
vertex_program myHLSLVertexProgram hlsl
{
    source myHLSLVertexProgram.hlsl
    entry_point main_vp
    target vs_2_0
}
```

As you can see, the main syntax is almost identical, except that instead of `profiles` with a list of assembler formats, you have a `target` parameter which allows a single assembler target to be specified - obviously this has to be a DirectX assembler format syntax code.

@note One thing to bear in mind is that HLSL allows you to use 2 different ways to multiply a vector by a matrix - mul(v,m) or mul(m,v). The only difference between them is that the matrix is effectively transposed. You should use mul(m,v) with the matrices passed in from Ogre - this agrees with the shaders produced from tools like RenderMonkey, and is consistent with Cg too, but disagrees with the Dx9 SDK and FX Composer which use mul(v,m) - you will have to switch the parameters to mul() in those shaders.
@note
If you use the @c float3x4 / @c matrix3x4 type in your shader, bound to an OGRE auto-definition (such as bone matrices) you should use the `column_major_matrices = false` option (discussed below) in your program definition. This is because OGRE passes @c float3x4 as row-major to save constant space (3 float4’s rather than 4 float4’s with only the top 3 values used) and this tells OGRE to pass all matrices like this, so that you can use mul(m,v) consistently for all calculations. OGRE will also to tell the shader to compile in row-major form (you don’t have to set the `/Zpr` compile option or \#pragma pack(row-major) option, OGRE does this for you). Note that passing bones in float4x3 form is not supported by OGRE, but you don’t need it given the above.

**Advanced options**<br>

<dl compact="compact">
<dt>column\_major\_matrices &lt;true|false&gt;</dt> <dd>

The default for this option is ’true’ so that OGRE passes matrices auto-bound matrices in a form where mul(m,v) works. Setting this option to false does 2 things - it transpose auto-bound 4x4 matrices and also sets the /Zpr (row-major) option on the shader compilation. This means you can still use mul(m,v), but the matrix layout is row-major instead. This is only useful if you need to use bone matrices (float3x4) in a shader since it saves a float4 constant for every bone involved.

</dd> <dt>optimisation\_level &lt;opt&gt;</dt> <dd>

Set the optimisation level, which can be one of ’default’, ’none’, ’0’, ’1’, ’2’, or ’3’. This corresponds to the /O parameter of fxc.exe, except that in ’default’ mode, optimisation is disabled in debug mode and set to 1 in release mode (fxc.exe uses 1 all the time). Unsurprisingly the default value is ’default’. You may want to change this if you want to tweak the optimisation, for example if your shader gets so complex that it will not longer compile without some minimum level of optimisation.

</dd> </dl>


# OpenGL GLSL {#GLSL}

OpenGL GLSL has a similar language syntax to HLSL but is tied to the OpenGL API. The are a few benefits over Cg in that it only requires the OpenGL render system plugin, not any additional plugins. Declaring a OpenGL GLSL program is similar to Cg but simpler. Here’s an example:

```cpp
vertex_program myGLSLVertexProgram glsl
{
    source myGLSLVertexProgram.vert
}
```

In GLSL, no entry point needs to be defined since it is always `main()` and there is no target definition since GLSL source is compiled into native GPU code and not intermediate assembly. 

For modularity %Ogre supports the non-standard <tt>\#include <something.glsl></tt> directive in GLSL. It also works with OpenGL ES and resembles what is available with HLSL and Cg.

@deprecated The @c attach keyword for multi-module shaders is not supported on OpenGL ES and therefore deprecated in favor of the @c \#include directive

```cpp
vertex_program myExternalGLSLFunction glsl
{
    source myExternalGLSLfunction.vert
}

vertex_program myGLSLVertexProgram glsl
{
    source myGLSLfunction.vert
    attach myExternalGLSLFunction
}
```

The `attach` keyword allows creating GLSL shaders from multiple shader modules of the same type. The referencing shader has to forward-declare the functions it intends to use

## GLSL Texture Samplers {#GLSL-Texture-Samplers}

To pass texture unit index values from the material script to texture samplers in glsl use `int` type named parameters. See the example below:<br>

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

@note GLSL uses column-major storage by default, while %Ogre is using row-major storage. Furthermore, GLSL is using column-major addressing, while %Ogre and HLSL use row-major addressing.
This means that `mat[0]` is the first column in GLSL, but the first row in HLSL and %Ogre. %Ogre takes care of transposing square matrices before uploading them with GLSL, so matrix-vector multiplication `M*v` just works and `mat[0]` will return the same data.
However, with non-square matrices transposing would change their GLSL type from e.g. `mat2x4` (two columns, four rows) to `mat4x2` (two rows, four columns) and consequently what `mat[0]` would return. Therefore %Ogre just passes such matrices unchanged and you have to handle this case (notably in skinning) yourself by either transposing the matrix in the shader or column-wise access.

## Binding vertex attributes {#Binding-vertex-attributes}

Vertex attributes must be declared in the shader, for the vertex data bound to it by Ogre.

```cpp
// legacy GLSL syntax
attribute vec4 vertex;

// modern GLSL syntax with explicit layout qualifier
layout(location = 0) in vec4 vertex;
```

refer to the following table for the location indices and names to use:

| Semantic | Custom name | Binding location | Legacy OpenGL built-in |
|----------|------|------------------|-----------------------|
| Ogre::VES_POSITION | vertex | 0 | gl_Vertex |
| Ogre::VES_BLEND_WEIGHTS | blendWeights | 1 | n/a |
| Ogre::VES_NORMAL | normal | 2 | gl_Normal |
| Ogre::VES_DIFFUSE | colour | 3 | gl_Color |
| Ogre::VES_SPECULAR | secondary_colour | 4 | gl_SecondaryColor |
| Ogre::VES_BLEND_INDICES | blendIndices | 7 | n/a |
| Ogre::VES_TEXTURE_COORDINATES | uv0 - uv7 | 8-15 | gl_MultiTexCoord0 - gl_MultiTexCoord7 |
| Ogre::VES_TANGENT | tangent | 14 | n/a |
| Ogre::VES_BINORMAL | binormal | 15 | n/a |

@note uv6 and uv7 share attributes with tangent and binormal respectively so cannot both be present.

## Compatibility profile GLSL features {#Legacy-GLSL-features}

The following features are only available when using the legacy OpenGL profile. Notably they are not available with GL3+ or GLES2.

### Accessing OpenGL state
GLSL can access most of the GL states directly so you do not need to pass these states through [param\_named\_auto](#param_005fnamed_005fauto) in the material script. This includes lights, material state, and all the matrices used in the openGL state i.e. model view matrix, worldview projection matrix etc.

### Access to built-in attributes
GLSL natively supports automatic binding of the most common incoming per-vertex attributes (e.g. `gl_Vertex`, `gl_Normal`, `gl_MultiTexCoord0` etc)
as described in section 7.3 of the GLSL manual.
There are some drivers that do not behave correctly when mixing built-in vertex attributes like `gl_Normal` and custom vertex attributes, so for maximum compatibility you should use all custom attributes

### Geometry shader specification
GLSL allows the same shader to run on different types of geometry primitives. In order to properly link the shaders together, you have to specify which primitives it will receive as input, which primitives it will emit and how many vertices a single run of the shader can generate. The GLSL geometry\_program definition requires three additional parameters

@param input\_operation\_type
The operation type of the geometry that the shader will receive. Can be ’point\_list’, ’line\_list’, ’line\_strip’, ’triangle\_list’, ’triangle\_strip’ or ’triangle\_fan’.

@param output\_operation\_type
The operation type of the geometry that the shader will emit. Can be ’point\_list’, ’line\_strip’ or ’triangle\_strip’.

@param max\_output\_vertices
The maximum number of vertices that the shader can emit. There is an upper limit for this value, it is exposed in the render system capabilities.

@par Example
```cpp
geometry_program Ogre/GPTest/Swizzle_GP_GLSL glsl
{
    source SwizzleGP.glsl
    input_operation_type triangle_list
    output_operation_type line_strip
    max_output_vertices 6
}
```

With GL3+ these values are specified using the `layout` modifier.

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
                param_named_auto worldViewProj worldviewproj_matrix
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
                param_named_auto worldViewProj worldviewproj_matrix
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
                param_named_auto worldViewProj worldviewproj_matrix
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

# Using GPU Programs in a Pass {#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass}

Within a pass section of a material script, you can reference a vertex, geometry and / or a fragment program which is been defined in a .program script (See @ref Declaring-Vertex_002fGeometry_002fFragment-Programs). The programs are defined separately from the usage of them in the pass, since the programs are very likely to be reused between many separate materials, probably across many different .material scripts, so this approach lets you define the program only once and use it many times.

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

Parameters can be specified using one of 4 commands as shown below. The same syntax is used whether you are defining a parameter just for this particular use of the program, or when specifying the @ref Default-Program-Parameters. Parameters set in the specific use of the program override the defaults.

-   [param\_indexed](#param_005findexed)
-   [param\_indexed\_auto](#param_005findexed_005fauto)
-   [param\_named](#param_005fnamed)
-   [param\_named\_auto](#param_005fnamed_005fauto)
-   [shared\_params\_ref](#shared_005fparams_005fref)

<a name="param_005findexed"></a><a name="param_005findexed-1"></a>

## param\_indexed

This command sets the value of an indexed parameter. 

@par
Format: param\_indexed &lt;index&gt; &lt;type&gt; &lt;value&gt;
@par
Example: param\_indexed 0 float4 10.0 0 0 0

@param index
simply a number representing the position in the parameter list which the value should be written, and you should derive this from your program definition. The index is relative to the way constants are stored on the card, which is in 4-element blocks. For example if you defined a float4 parameter at index 0, the next index would be 1. If you defined a matrix4x4 at index 0, the next usable index would be 4, since a 4x4 matrix takes up 4 indexes.

@param type
can be float4, matrix4x4, float&lt;n&gt;, int4, int&lt;n&gt;. Note that ’int’ parameters are only available on some more advanced program syntaxes, check the D3D or GL vertex / fragment program documentation for full details. Typically the most useful ones will be float4 and matrix4x4. Note that if you use a type which is not a multiple of 4, then the remaining values up to the multiple of 4 will be filled with zeroes for you (since GPUs always use banks of 4 floats per constant even if only one is used).

@param value
a space or tab-delimited list of values which can be converted into the type you have specified.

<a name="param_005findexed_005fauto"></a><a name="param_005findexed_005fauto-1"></a>

## param\_indexed\_auto

This command tells Ogre to automatically update a given parameter with a derived value. This frees you from writing code to update program parameters every frame when they are always changing.

@par
Format: param\_indexed\_auto &lt;index&gt; &lt;value\_code&gt; &lt;extra\_params&gt;
@par
Example: param\_indexed\_auto 0 worldviewproj\_matrix

@param index
has the same meaning as [param\_indexed](#param_005findexed); note this time you do not have to specify the size of the parameter because the engine knows this already. In the example, the world/view/projection matrix is being used so this is implicitly a matrix4x4.

@param value_code
is one of Ogre::GpuProgramParameters::AutoConstantType without the `ACT_` prefix. E.g. `ACT_WORLD_MATRIX` becomes `world_matrix`.

 <a name="param_005fnamed"></a><a name="param_005fnamed-1"></a>

## param\_named

This is the same as param\_indexed, but uses a named parameter instead of an index. This can only be used with high-level programs which include parameter names; if you’re using an assembler program then you have no choice but to use indexes. Note that you can use indexed parameters for high-level programs too, but it is less portable since if you reorder your parameters in the high-level program the indexes will change.
@par
Format: param\_named &lt;name&gt; &lt;type&gt; &lt;value&gt;
@par
Example: param\_named shininess float4 10.0 0 0 0

The type is required because the program is not compiled and loaded when the material script is parsed, so at this stage we have no idea what types the parameters are. Programs are only loaded and compiled when they are used, to save memory.

<a name="param_005fnamed_005fauto"></a><a name="param_005fnamed_005fauto-1"></a>

## param\_named\_auto

This is the named equivalent of param\_indexed\_auto, for use with high-level programs.

@par
Format: param\_named\_auto &lt;name&gt; &lt;value\_code&gt; &lt;extra\_params&gt;
@par
Example: param\_named\_auto worldViewProj worldviewproj\_matrix

The allowed value codes and the meaning of extra\_params are detailed in [param\_indexed\_auto](#param_005findexed_005fauto).

<a name="shared_005fparams_005fref"></a><a name="shared_005fparams_005fref-1"></a>

## shared\_params\_ref

This option allows you to reference shared parameter sets as defined in @ref Declaring-Shared-Parameters.
@par
Format: shared\_params\_ref &lt;shared\_set\_name&gt; 
@par
Example: shared\_params\_ref mySharedParams

The only required parameter is a name, which must be the name of an already defined shared parameter set. All named parameters which are present in the program that are also present in the shared parameter set will be linked, and the shared parameters used as if you had defined them locally. This is dependent on the definitions (type and array size) matching between the shared set and the program.

# Shadows and Vertex Programs {#Shadows-and-Vertex-Programs}

When using @ref Shadows, the use of vertex programs can add some additional complexities, because %Ogre can only automatically deal with everything when using the fixed-function pipeline. If you use vertex programs, and you are also using shadows, you may need to make some adjustments.

- If you use **stencil shadows**, then any vertex programs which do vertex deformation can be a problem, because stencil shadows are calculated on the CPU, which does not have access to the modified vertices. If the vertex program is doing standard skeletal animation, this is ok (see section above) because %Ogre knows how to replicate the effect in software, but any other vertex deformation cannot be replicated, and you will either have to accept that the shadow will not reflect this deformation, or you should turn off shadows for that object.
- If you use **texture shadows**, then vertex deformation is acceptable; however, when rendering the object into the shadow texture (the shadow caster pass), the shadow has to be rendered in a solid colour (linked to the ambient colour). You must therefore provide an alternative vertex program, so %Ogre provides you with a way of specifying one to use when rendering the caster.

Basically you specify an alternate material to use when rendering the object into the shadow texture:

```cpp
technique myShaderBasedTechnique
{
    shadow_caster_material myShadowCasterMaterial
    pass
    {
        ...
    }
}
```

When rendering a shadow caster, Ogre will automatically use the alternate material. You can bind the same or different parameters to the program - the most important thing is that you bind **ambient\_light\_colour**, since this determines the colour of the shadow in modulative texture shadows. If you don’t supply an alternate material, Ogre will fall back on a fixed-function material which will not reflect any vertex deformation you do in your vertex or geometry programs.

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

# Skeletal Animation in Vertex Programs {#Skeletal-Animation-in-Vertex-Programs}

You can implement skeletal animation in hardware by writing a vertex program which uses the per-vertex blending indices and blending weights, together with an array of world matrices (which will be provided for you by Ogre if you bind the automatic parameter ’world\_matrix\_array\_3x4’). However, you need to communicate this support to Ogre so it does not perform skeletal animation in software for you. You do this by adding the following attribute to your vertex\_program definition:

```cpp
   includes_skeletal_animation true
```

When you do this, any skeletally animated entity which uses this material will forgo the usual animation blend and will expect the vertex program to do it, for both vertex positions and normals. Note that ALL submeshes must be assigned a material which implements this, and that if you combine skeletal animation with vertex animation (See [Animation](@ref Animation)) then all techniques must be hardware accelerated for any to be.


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

# Vertex Texture Fetch {#Vertex-Texture-Fetch}

More recent generations of video card allow you to perform a read from a texture in the vertex program rather than just the fragment program, as is traditional. This allows you to, for example, read the contents of a texture and displace vertices based on the intensity of the colour contained within.

<a name="Declaring-the-use-of-vertex-texture-fetching"></a>

## Declaring the use of vertex texture fetching

If your vertex program makes use of Vertex Texture Fetch, you should declare that as

```cpp
   uses_vertex_texture_fetch true
```

Since hardware support for vertex texture fetching is not ubiquitous, you should use the directive when declaring your vertex programs which use vertex textures, so that if it is not supported, technique fallback can be enabled. This is not strictly necessary for DirectX-targeted shaders, since vertex texture fetching is only supported in vs\_3\_0, which can be stated as a required syntax in your shader definition, but for OpenGL (GLSL), there are cards which support GLSL but not vertex textures, so you should be explicit about your need for them.

<a name="Render-system-texture-binding-differences"></a>

## DirectX9 binding limitations

Unfortunately the method for binding textures so that they are available to a vertex program is not well standardised. As at the time of writing, Shader Model 3.0 (SM3.0) hardware under DirectX9 include 4 separate sampler bindings for the purposes of vertex textures. OpenGL, on the other hand, is able to access vertex textures in GLSL (and in assembler through NV\_vertex\_program\_3, although this is less popular), but the textures are shared with the fragment pipeline. I expect DirectX to move to the GL model with the advent of DirectX10, since a unified shader architecture implies sharing of texture resources between the two stages. As it is right now though, we’re stuck with an inconsistent situation.

To reflect this, you should use the `binding_type` attribute in a texture unit to indicate which unit you are targeting with your texture - ’fragment’ (the default) or ’vertex’. For render systems that don’t have separate bindings, this actually does nothing. But for those that do, it will ensure your texture gets bound to the right processing unit.

Note that whilst DirectX9 has separate bindings for the vertex and fragment pipelines, binding a texture to the vertex processing unit still uses up a ’slot’ which is then not available for use in the fragment pipeline. I didn’t manage to find this documented anywhere, but the nVidia samples certainly avoid binding a texture to the same index on both vertex and fragment units, and when I tried to do it, the texture did not appear correctly in the fragment unit, whilst it did as soon as I moved it into the next unit.

<a name="Texture-format-limitations"></a>

## Texture format limitations

Again as at the time of writing, the types of texture you can use in a vertex program are limited to 1- or 4-component, full precision floating point formats. In code that equates to PF\_FLOAT32\_R or PF\_FLOAT32\_RGBA. No other formats are supported. In addition, the textures must be regular 2D textures (no cube or volume maps) and mipmapping and filtering is not supported, although you can perform filtering in your vertex program if you wish by sampling multiple times.

<a name="Hardware-limitations"></a>

## Hardware limitations

As at the time of writing (early Q3 2006), ATI do not support texture fetch in their current crop of cards (Radeon X1n00). nVidia do support it in both their 6n00 and 7n00 range. ATI support an alternative called ’Render to Vertex Buffer’, but this is not standardised at this time and is very much different in its implementation, so cannot be considered to be a drop-in replacement. This is the case even though the Radeon X1n00 cards claim to support vs\_3\_0 (which requires vertex texture fetch).

@page Runtime-Shader-Generation Runtime Shader Generation 

Writing shading programs is a common task when developing 3D based application. Most of the visual effects used by 3D based applications involve shader programs.
Additionally with D3D11/ GL3, support for fixed pipeline functionality was removed. Meaning you can only render objects using shaders.

While @ref High-level-Programs offer you maximal control and flexibility over how your objects are rendered, writing and maintaining them is also a very time consuming task.

Instead %Ogre can also automatically generate shaders on the fly, based on object material properties, scene setup and other user definitions. While the resulting shaders are less optimized, they offer the following advantages:

* Save development time e.g. when your target scene has dynamic lights and the number changes, fog changes and the number of material attributes increases the total count of needed shaders dramatically. It can easily cross 100 and it becomes a time consuming development task.
* Reusable code - once you've written the shader extension you can use it anywhere due to its independent nature.
* Custom shaders extension library - enjoy the shared library of effects created by the community. Unlike hand written shader code, which may require many adjustments to be plugged into your own shader code, using the extensions library requires minimum changes.

The system is implemented as a component, so you can enable/ disable it at compile time.

* @subpage rtss <br />
The RTSS is not another Uber shader with an exploding amount of @c \#ifdefs that make it increasingly difficult to add new functionality. 
Instead, it manages a set of opaque isolated components (SubRenderStates) where each implements a specific effect.
These "effects" notable include full Fixed Function emulation. At the core these components are plain shader files providing a set of functions. The shaders are based on properties defined in @ref Material-Scripts.

# Uber shader tips

In case, you are not conviced and want to go with your hand-rolled uber shader, here are some tips:

1. %Ogre supports @c \#include directives universally - even with GLSL, so use them to split up your shader.
2. There is the `HLSL_SM4Support.hlsl` helper which abstracts the differences between HLSL9/ Cg and HLSL SM4.

Then you can have a shader skeleton like this:

```cpp
#ifdef USE_UV
#include "parameters_uv.glsl"
#endif
#ifdef USE_SKINNING
#include <parameters_skinning.glsl>
#endif
...

void main()
{
#ifdef USE_UV
    #include <transform_uv.glsl>
#endif
#ifdef USE_SKINNING
    #include <transform_skinning.glsl>
#endif
#ifdef USE_TANGENT
    #include <construct_tbn.glsl>
#endif
    ...
    gl_Position = ...;
}
```

then in the material file, you can instanciate it as:

```cpp
vertex_program TextureAndSkinning glsl
{
    source UberShader_vp.glsl
    preprocessor_defines USE_UV,USE_SKINNING
    default_params
    {
        ...
    }
}
```
and reference it with your materials.

Incidentally, this is very similar to what the RTSS is doing internally. Except, you do not need the @c preprocessor_defines part, as it can derive automatically from the material what needs to be done.

# Historical background

When the early graphic cards came into the market they contained a fixed but large set of functions with which you could influence how 3D object were rendered. These included influencing object positions using matrices, calculating the effect of textures on a pixel, calculating the effect of lights on vertices and so on. These set of functions and their implementation in hardware became later known as the graphic card fixed pipeline (or Fixed Function Pipeline).

As graphic cards became more powerful and graphic application became more complex, a need for new ways to manipulate the rendering of 3D models became apparent. This need saw the introduction of shaders. 

Shaders are small custom made programs that run directly on the graphics card. Using these programs, one could replace the calculations that were made by the fixed pipeline and add new functionality. However there was a catch: If shaders are used on an object, the object can no longer use any of the functionality of the fixed pipeline. Any calculation that was used in the fixed pipeline needed to be recreated in the shaders. With early graphics applications this was not problematic. Shaders were simple and their numbers were kept low. However as applications grew in complexity this meant that the need for shaders grew as well. As a programmer you were left with 2 choices, both bad. Either create an exuberant amount of small shaders that soon became too many to effectively maintain. Or create an uber shader, a huge complex shader, that soon became too complex to effectively maintain as well.

The RTSS seeks to fix those problems by automatically generating shaders based on the operations previously required from the fixed pipeline and new capabilities required by the user.

With the introduction of the version 11 of Direct3D, a new reason for having an RTSS like system became apparent. With D3D11 support for fixed pipeline functionality was removed. Meaning, you can only render objects using shaders. The RTSS is an excellent tool for this purpose.