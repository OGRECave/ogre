# GPU Program Scripts {#High-level-Programs}

@anchor Declaring-Vertex_002fGeometry_002fFragment-Programs

@tableofcontents

In order [to use a vertex, geometry or fragment program in your materials](@ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass), you first have to define them. A single program definition can be used by any number of materials, the only prerequisite is that a program must be defined before being referenced in the pass section of a material.

The definition of a program can either be embedded in the .material script itself (in which case it must precede any references to it in the script), or if you wish to use the same program across multiple .material files, you can define it in an external .program script. You define the program in exactly the same way whether you use a .program script or a .material script, the only difference is that all .program scripts are guaranteed to have been parsed before **all** .material scripts, so you can guarantee that your program has been defined before any .material script that might use it. Just like .material scripts, .program scripts will be read from any location which is on your resource path, and you can define many programs in a single script.

Vertex, geometry and fragment programs can be low-level (i.e. assembler code written to the specification of a given low level syntax such as vs\_1\_1 or arbfp1) or high-level such as DirectX HLSL and OpenGL GLSL. High level languages give you a number of advantages, such as being able to write more intuitive code, and possibly being able to target multiple architectures in a single program (for example, the same Cg program might be able to be used in both D3D and GL, whilst the equivalent low-level programs would require separate techniques, each targeting a different API). High-level programs also allow you to use named parameters instead of simply indexed ones, although parameters are not defined here, they are used in the Pass.

Here is an example of a definition of a low-level vertex program:

```cpp
vertex_program myVertexProgram spirv
{
    source myVertexProgram.spv
}
```

As you can see, that’s very simple, and defining a fragment or geometry program is exactly the same, just with @c vertex_program replaced with @c fragment_program or @c geometry_program, respectively. Likewise, for tessellation and compute programs, use @c tessellation_hull_program, @c tessellation_domain_program, and @c compute_program.

You give the program a name in the header, followed by the word @c spirv to indicate the syntax being used. Inside the braces, you specify where the source is going to come from (and this is loaded from any of the resource locations as with other media). The syntax specification is necessary for the engine to know what syntax the program is in before reading it. During the compilation of the material, it is important to quickly skip programs that use unsupported syntax to avoid loading the program first.

# Default Program Parameters {#Default-Program-Parameters}

While defining a vertex, geometry or fragment program, you can also specify the default parameters to be used for materials which use it, unless they specifically override them. You do this by including a nested ’default\_params’ section, like so:

@snippet Samples/Media/materials/scripts/Examples-Advanced.material celshading_vp

The syntax of the parameter definition is exactly the same as when you define parameters when using programs, See @ref Program-Parameter-Specification. Defining default parameters allows you to avoid rebinding common parameters repeatedly (clearly in the above example, all but ’shininess’ are unlikely to change between uses of the program) which makes your material declarations shorter.

# High Level Programs

Support for high level vertex and fragment programs is provided through plugins; this is to make sure that an application using OGRE can use as little or as much of the high-level program functionality as they like. OGRE supports multiple high-level program types. Notably DirectX [HLSL](#HLSL), and OpenGL [GLSL](#GLSL). HLSL can only be used with the DirectX rendersystem, and GLSL can only be used with the GL and Vulkan rendersystems.

One way to support both HLSL and GLSL is to include separate techniques in the material script, each one referencing separate programs. However, if the programs are basically the same, with the same parameters, and the techniques are complex this can bloat your material scripts with duplication fairly quickly. Instead, if the only difference is the language of the vertex & fragment program you can use OGRE’s [Unified High-level Programs](#Unified-High_002dlevel-Programs) to automatically pick a program suitable for your rendersystem whilst using a single technique.

There is also [Cg](#Cg) which is deprecated, but allows to use the same Shader code across different APIs - although experience has shown that more advanced programs, particularly fragment programs which perform a lot of texture fetches, can produce better code in the rendersystem-specific shader language.
The better alternative to Cg is to use @ref multi-language-programs with `OgreUnifiedShader.h`. This achieves the same goal as Cg, but uses only a few straightforward preprocessor macros.

## Preprocessor definitions {#Preprocessor-definitions}

Both GLSL and HLSL support using preprocessor definitions in your code - some are defined by the implementation, but you can also define your own, say in order to use the same source code for a few different variants of the same technique. In order to use this feature, include preprocessor conditions in your code, of the kind <tt>\#ifdef SYMBOL</tt>, <tt>\#if SYMBOL==2</tt> etc. Then in your program definition, use the `preprocessor_defines` option, following it with a string of definitions. Definitions are separated by `;` or `,` and may optionally have a `=` operator within them to specify a definition value. Those without an `=` will implicitly have a definition of 1.

@par
Format: preprocessor_defines &lt;defines&gt;
@par
Example: preprocessor_defines CLEVERTECHNIQUE,NUMTHINGS=2

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
```

This way you can use the same source code but still include small variations, each one defined as a different %Ogre program name but based on the same source code.

@note on GLSL %Ogre pre-processes the source itself instead on relying on the driver implementation which is often buggy. This relaxes using @c \#ifdef directives compared to the standard - e.g. you can <tt>\#ifdef \#version</tt>. However this means that defines specified in GLSL extensions are not present.

## Entry Point

The parameter @c entry_point, specifies the name of a function which will be the first one called as part of the program. Unlike assembler programs, which just run top-to-bottom, high-level programs can include multiple functions and as such you must specify the one which start the ball rolling.
If you omit this line, %Ogre will default to looking for a function called `main`.

@par
Format: entry_point &lt;name&gt;
@par
Example: entry_point main_vp

@note on GLSL the @c entry_point is required to be `main`

# GLSL programs {#GLSL}

GLSL is the native shading language of the OpenGL API and requires no plugins there. Additionally, you can use it with Vulkan by loading the @c Plugin_GLSLangProgramManager.
Declaring a OpenGL GLSL program is similar to Cg but simpler. Here’s an example:

```cpp
vertex_program myGLSLVertexProgram glsl
{
    source myGLSLVertexProgram.vert
}
```

The type `glsl` works with the GL and GL3+ RenderSystems, while with the GLES2 RenderSystem you must specify `glsles` instead.
For Vulkan, you must specify `glslang` so the GLSLang Plugin is used. This also works with GL3+, in case you prefer not to use the GLSL compiler of your driver.
If your shader is designed for this, you can also specify all of those at once. See @ref multi-language-programs.

In GLSL, no entry point needs to be defined since it is always `main()` and there is no target definition since GLSL source is compiled into native GPU code and not intermediate assembly. 

For modularity %Ogre supports the non-standard <tt>\#include <something.glsl></tt> directive in GLSL. It also works with OpenGL ES and resembles what is available with HLSL and Cg.

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
| Ogre::VES_COLOUR | colour | 3 | gl_Color |
| Ogre::VES_COLOUR2 | secondary_colour | 4 | gl_SecondaryColor |
| Ogre::VES_BLEND_INDICES | blendIndices | 7 | n/a |
| Ogre::VES_TEXTURE_COORDINATES | uv0 - uv7 | 8-15 | gl_MultiTexCoord0 - gl_MultiTexCoord7 |
| Ogre::VES_TANGENT | tangent | 14 | n/a |
| Ogre::VES_BINORMAL | binormal | 15 | n/a |

@note uv6 and uv7 share attributes with tangent and binormal respectively so cannot both be present.

## Binding Texture Samplers {#GLSL-Texture-Samplers}

To bind samplers to texture unit indices from the material scripts, you can either use the explicit binding with GL4.2+ or
set the sampler via a `int` type named parameter.

```cpp
// modern (GL4.2+) syntax with explicit binding
layout(binding = 0) uniform sampler2D diffuseMap;

// legacy syntax
uniform sampler2D diffuseMap;
```

Binding the sampler in material script:

```cpp
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
// mat4 uniform
param_named OcclusionMatrix matrix4x4 1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 0
// or
param_named ViewMatrix float16 0 1 0 0  0 0 1 0  0 0 0 1  0 0 0 0

// mat3
param_named TextRotMatrix float9 1 0 0  0 1 0  0 0 1

// mat2 uniform
param_named skewMatrix float4 0.5 0 -0.5 1.0
```

@note GLSL uses column-major storage by default, while %Ogre is using row-major storage. Furthermore, GLSL is using column-major addressing, while %Ogre and HLSL use row-major addressing.
This means that `mat[0]` is the first column in GLSL, but the first row in HLSL and %Ogre. %Ogre takes care of transposing square matrices before uploading them with GLSL, so matrix-vector multiplication `M*v` just works and `mat[0]` will return the same data.
However, with non-square matrices transposing would change their GLSL type from e.g. `mat2x4` (two columns, four rows) to `mat4x2` (two rows, four columns) and consequently what `mat[0]` would return. Therefore %Ogre just passes such matrices unchanged and you have to handle this case (notably in skinning) yourself by either transposing the matrix in the shader or column-wise access.

## Transform Feedback Varyings {#Transform-Feedback-Varyings}

Similarly to vertex attributes, the transform feedback varyings are bound by name.
Only interleaved output to a single buffer is supported. The offsets are given by the Ogre::VertexDeclaration.
The available varyings are:

| Semantic | Varying name |
|----------|--------------|
| Ogre::VES_POSITION | xfb_position |
| Ogre::VES_NORMAL | xfb_normal |
| Ogre::VES_COLOUR | xfb_colour |
| Ogre::VES_COLOUR2 | xfb_colour2 |
| Ogre::VES_TEXTURE_COORDINATES | xfb_uv0 - xfb_uv7 |

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

# Cg programs {#Cg}

In order to define Cg programs, you have to have to load @c Plugin_CgProgramManager at startup, either through plugins.cfg or through your own plugin loading code. They are very easy to define:

```cpp
fragment_program myCgFragmentProgram cg
{
    source myCgFragmentProgram.cg
    entry_point main_fp
    profiles ps_2_0 arbfp1
}
```

There are a few differences between this and the assembler program - to begin with, we declare that the fragment program is of type `cg` rather than `spirv`, which indicates that it’s a high-level program using Cg. The `source` parameter is the same, except this time it’s referencing a Cg source file instead of a file of assembler.

Here is where things start to change. Instead of a fixed `syntax` parameter, you specify one or more `profiles`; profiles are how Cg compiles a program down to the low-level assembler. The profiles have the same names as the assembler syntax codes mentioned above; the main difference is that you can list more than one, thus allowing the program to be compiled down to more low-level syntaxes so you can write a single high-level program which runs on both D3D and GL. You are advised to just enter the simplest profiles under which your programs can be compiled in order to give it the maximum compatibility. The ordering also matters; if a card supports more than one syntax then the one listed first will be used.

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

### Multi module shaders

The `attach` keyword allows creating GLSL shaders from multiple shader modules of the same type. The referencing shader has to forward-declare the functions it intends to use

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

# Assembler Shaders

The current supported syntaxes are:

<dl compact="compact">
<dt>spirv</dt> <dd>

The assembly language used by Vulkan

</dd>
<dt>gl_spirv</dt> <dd>

SPIRV variant exposed by ARB_gl_spirv

</dd>
<dt>vs_*</dt> <dd>

These is are the DirectX vertex shader assembler syntaxes.

</dd> <dt>arbvp1</dt> <dd>

This is the OpenGL standard assembler format for vertex programs. It’s roughly equivalent to DirectX vs\_1\_1.

</dd> <dt>vp*</dt> <dd>

These are nVidia-specific OpenGL vertex shader syntax which is a superset of vs_1_1_, that have otherwise no equivalent in OpenGL.

</dd>
<dt>ps_*</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax.
@note for ATI 8500, 9000, 9100, 9200 hardware, these profiles can also be used in OpenGL. The ATI 8500 to 9200 do not support arbfp1 but do support atifs extension in OpenGL which is very similar in function to ps\_1\_4 in DirectX. Ogre has a built in ps\_1\_x to atifs compiler that is automatically invoked when ps\_1\_x is used in OpenGL on ATI hardware.

</dd>
<dt>arbfp1</dt> <dd>

This is the OpenGL standard assembler format for fragment programs. It’s roughly equivalent to ps\_2\_0, which means that not all cards that support basic pixel shaders under DirectX support arbfp1 (for example neither the GeForce3 or GeForce4 support arbfp1, but they do support ps\_1\_1).

</dd> <dt>fp20</dt> <dd>

This is an nVidia-specific OpenGL fragment syntax which is a superset of ps 1.3. It allows you to use the [nvparse format](https://www.nvidia.com/attach/6400) for basic fragment programs. It actually uses NV\_texture\_shader and NV\_register\_combiners to provide functionality equivalent to DirectX’s ps\_1\_1 under GL, but only for nVidia cards. However, since ATI cards adopted arbfp1 a little earlier than nVidia, it is mainly nVidia cards like the GeForce3 and GeForce4 that this will be useful for.

</dd> <dt>fp*</dt> <dd>

Another nVidia-specific OpenGL fragment shader syntax.

</dd>
<dt>gpu\_gp, gp4\_gp</dt> <dd>

An nVidia-specific OpenGL geometry shader syntax. <br> Supported cards: nVidia GeForce FX8 series<br>

</dd> </dl>

You can get a definitive list of the syntaxes supported by the current card by calling `Ogre::GpuProgramManager::getSupportedSyntax()`.

## Specifying Named Constants {#Specifying-Named-Constants-for-Assembler-Shaders}

Assembler shaders don’t have named constants (also called uniform parameters) because the language does not support them - however if you for example decided to precompile your shaders from a high-level language down to assembler for performance or obscurity, you might still want to use the named parameters. Well, you actually can - GpuNamedConstants which contains the named parameter mappings has a ’save’ method which you can use to write this data to disk, where you can reference it later using the manual\_named\_constants directive inside your assembler program declaration, e.g.

```cpp
vertex_program myVertexProgram spirv
{
    source myVertexProgram.spv
    manual_named_constants myVertexProgram.constants
}
```

In this case myVertexProgram.constants has been created by calling `Ogre::GpuNamedConstants::save("myVertexProgram.constants");` sometime earlier as preparation, from the original high-level program. Once you’ve used this directive, you can use named parameters here even though the assembler program itself has no knowledge of them.

# Multi-language Programs {#multi-language-programs}

Basic programs, like the `example.frag` stated above, are compatible with GLSL and GLSLES. To avoid duplicating the whole
program declaration, you can simply specify all the language types the program is compatible with as:

```cpp
fragment_program myFragmentShader glsl glsles
{
    source example.frag
}
```

If you use the built-in defines like @c OGRE_HLSL, you can even write programs compatible with both HLSL and GLSL. In fact, you can use `#include <OgreUnifiedShader.h>` in the shader which provides cross-language macros to help with this.

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
            vertex_program_ref myVertexProgramGLSL
            {
                param_named_auto worldViewProj worldviewproj_matrix
                param_named_auto lightColour light_diffuse_colour 0
                param_named_auto lightSpecular light_specular_colour 0
                param_named_auto lightAtten light_attenuation 0
            }
            fragment_program_ref myFragmentProgramGLSL
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

At runtime, when myVertexProgram or myFragmentProgram are used, OGRE automatically picks a real program to delegate to based on what’s supported on the current hardware / rendersystem. If none of the delegates are supported, the entire technique referencing the unified program is marked as unsupported and the next technique in the material is checked for fallback, just like normal. As your materials get larger, and you find you need to support HLSL and GLSL specifically (or need to write multiple interface-compatible versions of a program for whatever other reason), unified programs can really help reduce duplication.

# Parameter specification {#Program-Parameter-Specification}

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
Format: param\_indexed\_auto &lt;index&gt; &lt;autoConstType&gt; &lt;extraInfo&gt;
@par
Example: param\_indexed\_auto 0 worldviewproj\_matrix

@param index
has the same meaning as [param\_indexed](#param_005findexed); note this time you do not have to specify the size of the parameter because the engine knows this already. In the example, the world/view/projection matrix is being used so this is implicitly a matrix4x4.

@param autoConstType, extraInfo
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
Format: param\_named\_auto &lt;name&gt; &lt;autoConstType&gt; &lt;extraInfo&gt;
@par
Example: param\_named\_auto worldViewProj worldviewproj\_matrix

The allowed @c autoConstType and the meaning of @c extraInfo are detailed in [param\_indexed\_auto](#param_005findexed_005fauto).

<a name="shared_005fparams_005fref"></a><a name="shared_005fparams_005fref-1"></a>

## shared\_params\_ref

This option allows you to reference shared parameter sets as defined in @ref Declaring-Shared-Parameters.
@par
Format: shared\_params\_ref &lt;shared\_set\_name&gt; 
@par
Example: shared\_params\_ref mySharedParams

The only required parameter is a name, which must be the name of an already defined shared parameter set. All named parameters which are present in both the program and the shared parameter set will be linked, and the shared parameters values will be used.

# Declaring Shared Parameters {#Declaring-Shared-Parameters}

Often, not every parameter you want to pass to a shader is unique to that program, and perhaps you want to give the same value to a number of different programs, and a number of different materials using that program. Shared parameter sets allow you to define a ’holding area’ for shared parameters that can then be referenced when you need them in particular shaders, while keeping the definition of that value in one place. To define a set of shared parameters, you do this:

```cpp
shared_params YourSharedParamsName
{
    shared_param_named mySharedParam1 float4 0.1 0.2 0.3 0.4
    ...
}
```

As you can see, you need to use the keyword ’shared\_params’ and follow it with the name that you will use to identify these shared parameters. Inside the curly braces, you can define one parameter per line, in a way which is very similar to the [param\_named](#param_005fnamed) syntax. The definition of these lines is:
@par
Format: shared\_param\_named &lt;param\_name&gt; &lt;param\_type&gt; \[&lt;\[array\_size\]&gt;\] \[&lt;initial\_values&gt;\]

@param param_name must be unique within the set
@param param_type can be any one of float, float2, float3, float4, int, int2, int3, int4, matrix2x2, matrix2x3, matrix2x4, matrix3x2, matrix3x3, matrix3x4, matrix4x2, matrix4x3 and matrix4x4.
@param array_size allows you to define arrays of param\_type should you wish, and if present must be a number enclosed in square brackets (and note, must be separated from the param\_type with whitespace).
@param initial_values If you wish, you can also initialise the parameters by providing a list of values.

Once you have defined the shared parameters, you can reference them inside default\_params and params blocks using [shared\_params\_ref](#shared_005fparams_005fref). You can also obtain a reference to them in your code via Ogre::GpuProgramManager::getSharedParameters, and update the values for all instances using them.

## Hardware Support

Furthermore, shared_params can be mapped to hardware buffers, if supported by the RenderSystem. To enable this, you have to provide a matching declaration in your shader.

For HLSL, that would be a constant buffer defined as
```cpp
cbuffer YourSharedParamsName
{
    float4 mySharedParam1;
}
```

and for GLSL a uniform block defined as
```cpp
layout(std140, row_major) uniform YourSharedParamsName
{
    vec4 mySharedParam1;
};
```

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

When rendering a shadow caster, Ogre will automatically use the alternate material. You can bind the same or different parameters to the program - the most important thing is that you bind @c *ambient_light_colour*, since this determines the colour of the shadow in modulative texture shadows. If you don’t supply an alternate material, Ogre will fall back on a fixed-function material which will not reflect any vertex deformation you do in your vertex or geometry programs.

In addition, when rendering the shadow receivers with shadow textures, Ogre needs to project the shadow texture. It does this automatically in fixed function mode, but if the receivers use vertex programs, they need to have a shadow receiver material which does the usual vertex deformation, but also generates projective texture coordinates.

@note At this point you can as well just extend your original program for @ref Integrated-Texture-Shadows. The only advantage of the technique below is that you can use the same material with and without shadow mapping.

The alternative material is linked into the technique - similarly to the caster material - like:

```cpp
technique myShaderBasedTechnique
{
    shadow_receiver_material myShadowReceiverMaterial
    ...
}

material myShadowReceiverMaterial
{
    ...
    vertex_program_ref myShadowReceiverVertexProgram
    {
        param_indexed_auto 0 worldviewproj_matrix
        param_indexed_auto 4 texture_worldviewproj_matrix
    }
    ...
}
```

For the purposes of writing the alternate program, there is an automatic parameter binding of @c texture_worldviewproj_matrix which provides the program with texture projection parameters. The vertex program should do it’s normal vertex processing, and generate texture coordinates using this matrix and place them in texture coord sets 0 and 1, since some shadow techniques use 2 texture units. The colour of the vertices output by this vertex program must always be white, so as not to affect the final colour of the rendered shadow.

When using additive texture shadows, the @c shadow_receiver_material replaces the lighting render, so if you perform any fragment program lighting you also need to pull in a custom fragment program:

```cpp
pass
{
    fragment_program_ref myShadowReceiverFragmentProgram
    {
        param_named_auto lightDiffuse light_diffuse_colour 0
    }
    texture_unit
    {
        content_type shadow
    }
}
```

You should pass the projected shadow coordinates from the custom vertex program. As for textures, define a @c texture_unit with @c content_type @c shadow to pull the shadow texture. Your shadow receiver fragment program is likely to be the same as the bare lighting pass of your normal material, except that you insert an extra texture sampler for the shadow texture, which you will use to adjust the result by (modulating diffuse and specular components).

# Instancing in Vertex Programs {#Instancing-in-Vertex-Programs}

You can implement hardware instancing by writing a vertex program which reads the world matrix of each instance from a `float3x4` vertex attribute.
However, you need to communicate this support to %Ogre so it batches the instances instead of rendering them individually.
You do this by adding the following attribute to your `vertex_program` definition:

```cpp
   includes_instancing true
```

When you do this, all SubEntities with the same material will be batched together. %Ogre will create and populate an instance buffer with the world matrices of the instances. This buffer is provided in the `TEXCOORD1` attribute (also consuming `TEXCOORD2` and `TEXCOORD3`) to the vertex shader.

When batching, all instances are rendered in a single draw-call. All per-renderable operations are only performed with the first SubMesh of the batch.

Therefore the following features are not supported:
- `start_light` and `iteration` (all instances share the same lights)
- flip culling on negative scale (all instances use the same face order)
- custom renderable parameters (not implemented)
- light scissoring & clipping (not implemented)
- manualLightList (not implemented)

@note Instancing cannot be used together with skeletal animation (neither Hardware nor Software).

# Skeletal Animation in Vertex Programs {#Skeletal-Animation-in-Vertex-Programs}

You can implement skeletal animation in hardware by writing a vertex program which uses the per-vertex blending indices and blending weights, together with an array of world matrices (which will be provided for you by Ogre if you bind the automatic parameter ’world\_matrix\_array\_3x4’). However, you need to communicate this support to Ogre so it does not perform skeletal animation in software for you. You do this by adding the following attribute to your `vertex_program` definition:

```cpp
   includes_skeletal_animation true
```

When you do this, any skeletally animated entity which uses this material will forgo the usual animation blend and will expect the vertex program to do it, for both vertex positions and normals. Note that ALL submeshes must be assigned a material which implements this, and that if you combine skeletal animation with vertex animation (See [Animation](@ref Animation)) then all techniques must be hardware accelerated for any to be.


# Morph Animation in Vertex Programs {#Morph-Animation-in-Vertex-Programs}

You can implement morph animation in hardware by writing a vertex program which linearly blends between the first and second position keyframes passed as positions and the first free texture coordinate set, and by binding the animation\_parametric value to a parameter (which tells you how far to interpolate between the two). However, you need to communicate this support to Ogre so it does not perform morph animation in software for you. You do this by adding the following attribute to your `vertex_program` definition:

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

Shader Model 3.0 (SM3.0) hardware under DirectX9 includes 4 sampler bindings for the purposes of vertex textures. Ogre assigns the first 4 texture units to these bindings - therefore you should put fragment-only textures last.

<a name="Texture-format-limitations"></a>

## Texture format limitations

Again as at the time of writing, the types of texture you can use in a vertex program are limited to 1- or 4-component, full precision floating point formats. In code that equates to PF\_FLOAT32\_R or PF\_FLOAT32\_RGBA. No other formats are supported. In addition, the textures must be regular 2D textures (no cube or volume maps) and mipmapping and filtering is not supported, although you can perform filtering in your vertex program if you wish by sampling multiple times.

<a name="Hardware-limitations"></a>

## Hardware limitations

As at the time of writing (early Q3 2006), ATI do not support texture fetch in their current crop of cards (Radeon X1n00). nVidia do support it in both their 6n00 and 7n00 range. ATI support an alternative called ’Render to Vertex Buffer’, but this is not standardised at this time and is very much different in its implementation, so cannot be considered to be a drop-in replacement. This is the case even though the Radeon X1n00 cards claim to support vs\_3\_0 (which requires vertex texture fetch).

# Programmatic creation {#GpuProgram-API}

In case you need to create GPU Programs programmatically, see the following example for how the script is mapped to the API.

```cpp
vertex_program glTF2/PBR_vs glsl
{
	source pbr-vert.glsl
    preprocessor_defines HAS_NORMALS,HAS_TANGENTS
    default_params
    {
        param_named_auto u_MVPMatrix worldviewproj_matrix
    }
}
```
becomes
```cpp
using namespace Ogre;
GpuProgramManager& mgr = GpuProgramManager::getSingleton();

GpuProgramPtr vertex_program = mgr.createProgram("glTF2/PBR_vs", RGN_DEFAULT, "glsl", GPT_VERTEX_PROGRAM);
vertex_program->setSource("pbr-vert.glsl");
vertex_program->setParameter("preprocessor_defines", "HAS_NORMALS,HAS_TANGENTS");

GpuProgramParametersPtr params = vertex_program->getDefaultParameters();
params->setNamedAutoConstant("u_MVPMatrix", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
```

@page Cross-platform-Shaders Cross-platform Shaders

When targeting multiple graphics APIs, one typically needs to provide separate shaders for each API. This results in lots of duplicated code gets out of hand quickly.

@tableofcontents

To support using the same shader code for multiple APIs, %Ogre provides the following mechanisms.
- @c \#include directives are universally supported - even with GLSL, so you can rely on them to organised your shaders.
- There are several built-in defines that can be used to conditionally compile different versions of the same shader.

# Built-in defines
The following defines are available:
- The current shading language and native version: e.g. @c OGRE_GLSL=120, @c OGRE_HLSL=3
- The current shader type: e.g. @c OGRE_VERTEX_SHADER, @c OGRE_FRAGMENT_SHADER
- Whether @ref reversed-depth is enabled: @c OGRE_REVERSED_Z

# Cross-platform macros {#OgreUnifiedShader}

Additionally, the `OgreUnifiedShader.h` provides macros to map GLSL to HLSL and (to some extent) Metal.

As everything is handled by standard macros, the conversion can be performed by simply running the standard c preprocessor (`cpp`) on them - even without running %Ogre.

In general, you have to do the following changes compared to regular GLSL:
- Add the `#include <OgreUnifiedShader.h>` directive at the top of the file
- Use the `MAIN_PARAMETERS` and `MAIN_DECLARATION` directives instead of `void main()`
- Use the `IN`/ `OUT` macros to specify non-uniform parameters that are passed to the main function.
- Wrap the uniform paramters in the `OGRE_UNIFORMS` macro
- Declare Samplers with `SAMPLER2D/3D/CUBE/..` macros instead of `sampler2D/3D/Cube/..`
- Use `mtxFromRows` / `mtxFromCols` to construct matrices from vectors
- Use the HLSL style `mul` instead of `*` to multiply matrices
- Use `vec2_splat(1.0)` instead of the `vec2(1.0)` single component constructor.

Lets take a look on how to use the `OgreUnifiedShader.h` macros by starting with a simple GLSL shader:

```cpp
uniform mat4 worldMatrix;

attribute vec4 vertex;
void main()
{
    gl_Position = worldMatrix * vertex;
}
```

to make it cross-platform, we need to modify it as:

```cpp
#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
uniform mat4 worldMatrix;
)

MAIN_PARAMETERS
IN(vec4 vertex, POSITION)
MAIN_DECLARATION
{
    gl_Position = mul(worldMatrix, vertex);
}
```

@note If you only target different versions of the same API, there are reduced headers:
- `HLSL_SM4Support.hlsl` providing the `SAMPLER*` macros for mapping HLSL9/ Cg and HLSL SM4 (D3D11)
- `GLSL_GL3Support.glsl` providing the `IN`/ `OUT` macros and `texture` aliases for mapping GLSL <= 120 and GLSL >= 130

# Uber shader tips

To toggle features on and off use a shader skeleton like this:

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

then in the material file, you can instantiate it as:

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

Incidentally, this is very similar to what the [RTSS](@ref rtss) is doing internally. Except, you do not need the @c preprocessor_defines part, as it can derive automatically from the material what needs to be done.
