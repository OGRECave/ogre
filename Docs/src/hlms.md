#  HLMS: High Level Material System {#hlms}

This component allows you to manage shader variations of a specific shader template.
It is a different take to the Uber shader management, but instead of using plain
@c \#ifdefs it uses a custom, more powerful preprocessor language.

Additionally it allows you to define a set of abstract properties that are then used to
configure the shader generation.

Currently there is only the Physically Based Shading (PBS) material implementation based on the HLMS
that does not read the classical Materials and therefore does not respect 
the settings for fog, diffuse_color etc.

@attention This documentation was originally written for %Ogre 2.1, so not all details apply to the actual HLMS backport.

@tableofcontents

#  The three components {#components}

![](hlms_components.svg)

1.  Scripts. To set the material properties (i.e. type of Hlms to use:
    PBS, Toon shading, GUI; what textures, diffuse colour,
    roughness, etc). **You currently have to do this from C++.** Everybody
    will be using this part.

2.  Shader template. The Hlms takes a couple hand-written glsl/hlsl
    files as template and then adapts it to fit the needs on the
    fly (i.e. if the mesh doesn’t contain skeleton, the bit of code
    pertaining to skeletal animation is stripped from the
    vertex shader). The Hlms provides a simple preprocessor to deal with
    this entirely within from the template, but you’re not forced to
    use it. Here’s a simple example of the preprocessor. I won’t be
    explaining the main keywords today. Advanced users will probably
    want to modify these files (or write some of their own) to fit their
    custom needs.

3.  C++ classes implementation. The C++ takes care of picking the shader
    templates and manipulating them before compiling; and most
    importantly it feeds the shaders with uniform/constans data and sets
    the textures that are being in use. It is extremely flexible,
    powerful, efficient and scalable, but it’s harder to use than good
    ol’ Materials because those used to be data-driven: there are no
    AutoParamsSource here. Want the view matrix? You better grab it from
    the camera when the scene pass is about to start, and then pass it
    yourself to the shader. This is very powerful, because in D3D11/GL3+
    you can just set the uniform buffer with the view matrix just once
    for the entire frame, and thus have multiple uniforms buffers sorted
    by update frequency. Very advanced user will be using messing with
    this part.

@note Material scripts in Ogre 1.x do not yet support 
the HLMS - you must use the C++ API. e.g. Ogre::PbsMaterial.

Based on your skillset and needs, you can pick up to which parts you
want to mess with. Most users will just use the scripts to define
materials, advanced users will change the template, and very advanced
users who need something entirely different will change all three.

For example the PBS material has its own C++ implementation and its own set of shader templates.
The Toon Shading has its own C++ implementation and set of shaders.

It is theoretically possible to implement both Toon & PBS in the same
C++ module, but that would be crazy, hard to maintain and not very
modular.

#  Material parameters are stored in “Blocks” {#data}

You could be thinking the reason I came up with these two is to fit with
D3D11′s grand scheme of things while being compatible with OpenGL. But
that’s a half truth and an awesome side effect. I’ve been developing the
Hlms using OpenGL this whole time.

An OpenGL fan will tell you that grouping these together in single call
like D3D11 did barely reduce API overhead in practice (as long as you
keep sorting by state), and they’re right about that.

However, there are big advantages for using blocks:

1.  Many materials in practice share the same Macro- &
    Blendblock parameters. In an age where we want many 3D primitives
    with the same shader but slightly different parameters like texture,
    colour, or roughness (which equals, a different material) having
    these settings repeated per material wastes a lot of memory space…
    and a lot of bandwidth (and wastes cache space). Ogre 2.0 is
    bandwidth bound, so having all materials share the same pointer to
    the same Macroblock can potentially save a lot of bandwidth, and be
    friendlier to the cache at the same time.This stays true whether we
    use D3D11, D3D12, OpenGL, GL ES 2, or Mantle.

2.  Sorting by Macroblock is a lot easier (and faster) than sorting by
    its individual parameters: when preparing the hash used for sorting,
    it’s much easier to just do (every frame, per object) `hash
    |= (macroblock->getId() << bits) & mask` than to do: `hash =| m->depth_check | m->depthWrite << 1 | m->depthBias << 2 | m->depth_slope_bias << 3 | m->cullMode << 18 | ... ;` We also need a lot more bits we can’t afford. Ogre
    2.0 imposes a limit on the amount of live Macroblocks you can have
    at the same time; as we run out of hashing space (by the way, D3D11
    has its own limit). It operates around the idea that most setting
    combinations won’t be used in practice.

Of course it’s not perfect, it can’t fit every use case. We inherit the
same problems D3D11 has. If a particular rendering technique relies on
regularly changing a property that lives in a Macroblock (i.e. like
alternating depth comparison function between less & greater with every
draw call, or gradually incrementing the depth bias on each draw call);
you’ll end up redundantly changing a lot of other states (culling mode,
polygon mode, depth check & write flags, depth bias) alongside it. This
is rare. We’re aiming the general use case.

These problems make me wonder if D3D11 made the right choice of using
blocks from an API perspective, since I’m not used to driver
development. However from an engine perspective, blocks make sense.

## Datablocks {#toc52}

We’re introducing the concept of Datablocks.
A Datablock is a “material” from the user’s perspective.
It holds data (i.e. material properties) that will be
passed directly to the shaders.

![](hlms_blocks.svg)

The diagram shows a typical layout of a datablock.
Samplerblocks do not live inside base Ogre::HlmsDatablock, but rather in its
derived implementation. This is because some implementations may not
need textures at all, and the number of samplerblocks is unknown. Some
implementations may want one samplerblock per texture, whereas others
may just need one.

@note Macroblocks and Blendblocks are not available in 1.x - use Ogre::Pass::setDepthCheckEnabled etc. as usual, to change the respective properties

# Hlms templates {#toc69}

The Hlms will parse the template files from the template folder
according to the following rules:

1.  The files with the names "VertexShader_vs", "PixelShader_ps",
    "GeometryShader_gs", "HullShader_hs", "DomainShader_ds" will be
    fully parsed and compiled into the shader. If an implementation only
    provides "VertexShader_vs.glslt", "PixelShader_ps.glslt"; only the
    vertex and pixel shaders for OpenGL will be created. There will be
    no geometry or tesellation shaders.

2.  The files that contain the string "_piece_vs" in their filenames
    will be parsed only for collecting pieces (more on pieces later).
    Likewise, the words "_piece_ps", "_piece_gs", "_piece_hs",
    "_piece_ds” correspond to the pieces for their respective
    shader stages. Note that you can concatenate, thus
    "MyUtilities_piece_vs_piece_ps.glslt” will be collected both in
    the vertex and pixel shader stages.

The Hlms takes a template file (i.e. a file written in GLSL or HLSL) and
spits out valid shader code. Templates can take advantage of the Hlms'
preprocessor, which is a simple yet powerful macro-like preprocessor
that helps writing the required code.

##  The Hlms preprocessor {#preproc}

The preprocessor was written with speed and simplicity in mind. It does
not implement an AST or anything fancy. This is very important to
account while writing templates because there will be cases when using
the preprocessor may feel counter-intuitive or frustrating.

For example
```cpp
  \@property( IncludeLighting )

  /* code here */

  @end
```

is analogous to
```cpp
  #if IncludeLighting != 0

  /* code here */

  #endif
```

However you can't evaluate IncludeLighting to anything other than zero
and non-zero, i.e. you can't check whether IncludeLighting == 2 with the
Hlms preprocessor. A simple workaround is to define, from C++, the
variable “IncludeLightingEquals2” and check whether it's non-zero.
Another solution is to use the GLSL/HLSL preprocessor itself instead of
Hlms'. However, the advantage of Hlms is that you can see its generated
output in a file for inspection, whereas you can't see the GLSL/HLSL
after the macro preprocessor without vendor-specific tools. Plus, in the
case of GLSL, you'll depend on the driver implementation having a good
macro preprocessor.

##  Preprocessor syntax {#syntax}

The preprocessor always starts with \@ followed by the command, and often
with arguments inside parenthesis. Note that the preprocessor is always
case-sensitive. The following keywords are recognized:

-   \@property

-   \@foreach

-   \@counter

-   \@value

-   \@set add sub mul div mod min max

-   \@piece

-   \@insertpiece

-   \@pset padd psub pmul pdiv pmod pmin pmax

###  \@property( expression )

Checks whether the variables in the expression are true, if so, the text
inside the block is printed. Must be finazlied with \@end. The expression
is case-sensitive. When the variable hasn't been declared, it evaluates
to false.

The logical operands && || ! are valid.

Examples:
```cpp
  \@property( hlms_skeleton )

  //Skeleton animation code here

  @end

  \@property( hlms_skeleton && !hlms_normal )

  //Print this code if it has skeleton animation but no normals

  @end

  \@property( hlms_normal || hlms_tangent )

  //Print this code if it has normals or tangents

  @end

  \@property( hlms_normal && (!hlms_skeleton || hlms_tangent) )

  //Print this code if it has normals and either no skeleton or tangents

  @end
```

It is very similar to \#if hlms_skeleton != 0 \#endif; however there is
no equivalent \#else or \#elif syntax. As a simple workaround you can
do:
```cpp
  \@property( hlms_skeleton )

  //Skeleton animation code here

  @end \@property( !hlms_skeleton )

  //Non-Skeleton code here

  @end
```

Newlines are not necessary. The following is perfectly valid:
```
  diffuse = surfaceDiffuse \@property( hasLights )* lightDiffuse@end ;
```

Which will print:
```
  hasLights != 0                              hasLights == 0
  diffuse = surfaceDiffuse * lightDiffuse;   diffuse = surfaceDiffuse ;
```

###  \@foreach( scopedVar, count, \[start\] )

Loop that prints the text inside the block, The text is repeated count -
start times. Must be finalized with \@end.

-   scopedVar is a variable that can be used to print the current
    iteration of the loop while inside the block. i.e. “\@scopedVar” will
    be converted into a number in the range \[start; count)

-   count The number of times to repeat the loop (if start = 0). Count
    can read variables.

-   start Optional. Allows to start from a value different than 0. Start
    can read variables.

Newlines are very important, as they will be printed with the loop.

Examples:
|  Expression    |         Output |
|----------------|----------------|
|  \@foreach( 4, n ) <br>&emsp; \@n\@end  | <br>0<br>1<br>2<br>3|
|  \@foreach( 4, n ) \@n\@end             |   0 1 2 3 |
|  \@foreach( 4, n )<br>&emsp;\@n<br>\@end        |  <br>0<br><br>1<br><br>2<br><br>3<br> |
|  \@foreach( 4, n, 2 ) \@n\@end  |            2 3 |
| \@pset( myStartVar, 1 )<br>\@pset( myCountVar, 3 )<br>\@foreach( myStartVar, n, myCountVar )<br>&emsp;\@n\@end         |          1<br>2 |
|  \@foreach( 2, n )<br>&emsp;\@insertpiece( pieceName\@n )\@end | \@insertpiece( pieceName0 )<br>        \@insertpiece( pieceName1 ) |

> **Attention \#1!**
>
>  Don't use the common letter i for the loop counter. It will conflict with other keywords.
>
>  i.e. “\@foreach( 1, i )\@insertpiece( pieceName )\@end” will print “0nsertpiece( pieceName )” which is probably not what you intended.
>
>  **Attention \#2!**
>
>  foreach is parsed after property math (pset, padd, etc). That means that driving each iteration through a combination of properties and padd functions will not work as you would expect.
>
>  i.e. The following code will not work:
>  
> ```cpp
>    @pset( myVar, 1 )
>
>    @foreach( 2, n )
>
>    //Code
>
>    @psub( myVar, 1 ) //Decrement myVar on each loop
>
>    \@property( myVar )
>
>    //Code that shouldn't be printed in the last iteration
>
>    @end
>
>    @end
>```
>
> Because psub will be evaluated before expanding the foreach.

###  \@counter( variable )

Prints the current value of variable and increments it by 1. If the
variable hasn't been declared yet, it is initialized to 0.

Examples:
```
  Expression          Output

  @counter( myVar )   0
                      
  @counter( myVar )   1
                      
  @counter( myVar )   2
```

### \@value( variable )

Prints the current value of variable without incrementing it. If the
variable hasn't been declared, prints 0.
```cpp
  Expression          Output

  @value( myVar )     0
                      
  @value( myVar )     0
                      
  @counter( myVar )   0
                      
  @value( myVar )     1
                      
  @value( myVar )     1
```

### \@set add sub mul div mod min max

Sets a variable to a given value, adds, subtracts, multiplies, divides,
calculates modulus, or the minimum/maximum of a variable and a constant,
or two variables. This family of functions get evaluated after
foreach(s) have been expanded and pieces have been inserted. Doesn't
print its value.

Arguments can be in the form \@add(a, b) meaning a += b; or in the form
\@add( a, b, c ) meaning a = b + c

Useful in combination with \@counter and \@value

|  Expression     |        Output |  Math |
|-----------------|---------------|-------|
|  \@set( myVar, 1 ) <br> \@value( myVar ) |       1     |   myVar = 1 |                              
|  \@add( myVar, 5 )<br> \@value( myVar )   |    6  |      myVar = 1 + 5|
|  \@div( myVar, 2 ) <br> \@value( myVar ) |     3   |     myVar = 6 / 2|
|  \@mul( myVar, myVar )<br> \@value( myVar ) |  9   |     myVar = 3 * 3|
|  \@mod( myVar, 5 ) <br> \@value( myVar )    |  4   |     myVar = 9 % 5|
|  \@add( myVar, 1, 1 ) <br> \@value( myVar ) |  2  |       myVar = 1 + 1|

###  \@piece( nameOfPiece )

Saves all the text inside the blocks and saves it as a named piece. If a
piece with the given name already exists, a compiler error will be
thrown. The text that was inside the block won't be printed. Useful when
in combination with \@insertpiece. Pieces can also be defined from C++ or
[*collected*](#toc69) from piece template files.

Example:
```cpp
  Expression                        Output

  @piece( VertexTransform )         
                                    
  outPos = worldViewProj * inPos   
                                    
  @end
```

###  \@insertpiece( nameOfPiece )

Prints a block of text that was previously saved with piece (or from
C++). If no piece with such name exists, prints nothing.

Example:
```
  Expression                                                     Output

  @piece( VertexTransform )outPos = worldViewProj * inPos@end   void main()
                                                                 
  void main()                                                    {
                                                                 
  {                                                              outPos = worldViewProj * inPos
                                                                 
  @insertpiece( VertexTransform )                                }
                                                                 
  @insertpiece( InexistentPiece )                                
                                                                 
  }
```

###  \@pset padd psub pmul pdiv pmod pmin pmax

Analogous to [*the family of math functions without the 'p'
prefix*](#toc304). The difference is that the math is evaluated before
anything else. There is no much use to these functions, probably except
for quickly testing whether a given flag/variable is being properly set
from C++ without having to recompile.

i.e. If you suspect hlms_normal is never being set, try \@pset(
hlms_normal, 1 )

One important use worth mentioning, is that variables retain their
values across shader stages. First the vertex shader template is parsed,
then the pixel shader one. If 'myVal' is 0 and the vertex shader
contains \@counter( myVal ); when the pixel shader is parsed \@value(
myVal ) will return 1, not 0.

If you need to reset these variables across shader stages, you can use
pset( myVal, 0 ); which is guaranteed to reset your variable to 0 before
anything else happens; even if the pset is stored in a piece file.

#  Creation of shaders {#shaders}

There are two components that needs to be evaluated that may affect the
shader itself and would need to be recompiled:

1.  The Datablock/Material. Does it have Normal maps? Then include code
    to sample the normal map and affect the lighting calculations. Does
    it have a diffuse map? If not, avoid sampling the diffuse map and
    multiplying it against the diffuse colour, etc.

2.  The Mesh. Is it skeletally animated? Then include skeletal
    animation code. How many blend weights? Modify the skeletal
    animation code appropiately. It doesn't have tangents? Then skip the
    normal map defined in the material. And so on.

When calling Ogre::SceneManager::_renderScene, what happens is that
Ogre::ShaderManager::getGpuProgram will get called and this function evaluates both
the mesh and datablock compatibility.

If they're compatible, all the variables (aka properties) and pieces are
generated and cached in a structure (mShaderCache) with a hash key
to this cache entry. If a different pair of datablock-mesh ends up
having the same properties and pieces, they will get the same hash (and
share the same shader).

The following graph summarizes the process:

![](hlms_hash.svg)

Later on during rendering, at the start each render pass, a similar
process is done, which ends up generating a “[*pass hash*](#toc567)”
instead of a renderable hash. Pass data stores settings like number of
shadow casting lights, number of lights per type (directional, point,
spot).

While iterating each renderable for render, the hash key is read from
the Renderable and merged with the pass' hash. With the merged hash, the
shader is retrieved from a cache. If it's not in the cache, the shader
will be generated and compiled by merging the cached data (pieces and
variables) from the Renderable and the Pass. The following graph
illustrates the process:

![](hlms_caching.svg)

#  C++ interaction with shader templates {#cpp}

Note: This section is relevant to those seeking to write their own Hlms
implementation.

C++ can use Ogre::HlmsMaterialBase::getPropertyMap().setProperty( "key", value ) to set “key” to the given
value. This value can be read by \@property, \@foreach,
\@add/sub/mul/div/mod, \@counter, \@value and \@padd/psub/pmul/pdiv/pmod

To create pieces (or read them) you need to pass your custom
Hlms::PiecesMap to Hlms::addRenderableCache.

The recommended place to do this is in Hlms::calculateHashForPreCreate
and Hlms::calculateHashForPreCaster. Both are virtual. The former gets
called right before adding the set of properties, pieces and hash to the
cache, while the latter happens right before adding the similar set for
the shadow caster pass.

In those two functions you get the chance to call setProperty to set
your own variables and add your own pieces.

Another option is to overload Hlms::calculateHashFor which gives you
more control but you'll have to do some of the work the base class does.

For some particularly complex features, the Hlms preprocessor may not be
enough, too difficult, or just impossible to implement, and thus you can
generate the string from C++ and send it as a piece. The template shader
can insert it using \@insertpiece.

The function Hlms::createShaderCacheEntry is the main responsible for
generating the shaders and parsing the template through the Hlms
preprocessor. If you overload it, you can ignore pieces, properties;
basically override the entire Hlms system and provide the source for the
shaders yourself.

##  Common conventions

Properties starting with 'hlms_' prefix are common to all or most Hlms
implementations. i.e. 'hlms_skeleton' is set to 1 when a skeleton is
present and hardware skinning should be performed.

Save properties' IdStrings (hashed strings) into constant as performance
optimizations. Ideally the compiler should detect the constant
propagation and this shouldn't be needed, but this often isn't the case.

For mobile, avoid mat4 and do the math yourself. As for 4x3 matrices
(i.e. skinning), perform the math manually as many GLES2 drivers have
issues compiling valid glsl code.

Properties in underscore\_case are set from C++; propierties in
camelCase are set from the template.

Propierties and pieces starting with 'custom\_' are for user
customizations of the template

TBD

##  Disabling a stage

By default if a template isn't present, the shader stage won't be
created. e.g. if there is no GeometryShader\_gs.glsl file, no geometry
shader will be created. However there are times where you want to use a
template but only use this stage in particular scenarios (e.g. toggled
by a material parameter, disable it for shadow mapping, etc.). In this
case, set the property hlms_disable\_stage to non-zero from within the
template (i.e. using \@set) . The value of this property is reset to 0
for every stage.

Note that even when disabled, the Hlms template will be fully parsed and
dumped to disk; and any modification you perform to the Hlms properties
will be carried over to the next stages. Setting hlms_disable\_stage is
not an early out or an abort.

#  Customization {#customization}

In many cases, users may want to slightly customize the shaders to
achieve a particular look, implement a specific feature, or solve a
unique problem; without having to rewrite the whole implementation.

Maximum flexibility can be get by directly modifying the original source
code. However this isn't modular, making it difficult to merge when the
original source code has changed. Most of of the customizations don't
require such intrusive approach.

Note: For performance reasons, the listener interface does not allow you
to add customizations that work per Renderable, as that loop is
performance sensitive. The only listener callback that works inside
Hlms::fillBuffersFor is hlmsTypeChanged which only gets evaluated when
the previous Renderable used a different Hlms implementation; which is
rare, and since we sort the RenderQueue, it often branch predicts well.

There are different levels in which an Hlms implementation can be
customized:

1.  Using a library, see [*Hlms Initialization*](#toc574). pass a set
    of piece files in a folder by pushing the folder to ArchiveVec. The
    files in that folder will be parsed first, in order (archiveVec\[0\]
    then archiveVec\[1\], … archiveVec\[N-1\]); which will let you
    define your own pieces to insert code into the default template (see
    the the table at the end). You can also do clever tricky things to
    avoid dealing with C++ code at all even if there are no 'custom\_'
    pieces for it. For example, you can write the following code to
    override the BRDF declarations and provide a custom BRDF:
```cpp
  //Disable all known BRDFs that the implementation may enable

  @pset( BRDF_CookTorrance, 0 )

  @pset( BRDF_Default, 0 )

  @piece( DeclareBRDF )

  // Your BRDF code declaration here

  @end
```

1.  Via listener, through HlmsListener. This allows you to have access
    to the buffer pass to fill extra information; or bind extra buffers
    to the shader.

2.  Overload HlmsPbs. Useful for overriding only specific parts, or
    adding new functionality that requires storing extra information in
    a datablock (e.g. overload HlmsPbsDatablock to add more variables,
    and then overload HlmsPbs::createDatablockImpl to create these
    custom datablocks)

3.  Directly modify HlmsPbs, HlmsPbsDatablock and the template.
| Variable | Description |
|----------|-------------|
| custom_passBuffer            |   Piece where users can add extra information for the pass buffer (only useful if the user is using HlmsListener or overloaded HlmsPbs. |
| custom_VStoPS                |   Piece where users can add more interpolants for passing data from the vertex to the pixel shader.|
| custom_vs_attributes         |  Custom vertex shader attributes in the Vertex Shader (i.e. a special texcoord, etc).|
| custom_vs_uniformDeclaration |  Data declaration (textures, texture buffers, uniform buffers) in the Vertex Shader.|
| custom_vs_preExecution       |  Executed before Ogre's code from the Vertex Shader.|
| custom_vs_posExecution       |  Executed after all code from the Vertex Shader has been performed.                     |
| custom_ps_uniformDeclaration |  Same as custom_vs_uniformDeclaration, but for the Pixel Shader|
| custom_ps_preExecution       |  Executed before Ogre's code from the Pixel Shader.|
| custom_ps_posMaterialLoad    |  Executed right after loading material data; and before anything else. May not get executed if there is no relevant material data (i.e. doesn't have normals or QTangents for lighting calculation)|
| custom_ps_preLights          |  Executed right before any light (i.e. to perform your own ambient / global illumination pass). All relevant texture data should be loaded by now.|
| custom_ps_posExecution       |  Executed after all code from the Pixel Shader has been performed.|
