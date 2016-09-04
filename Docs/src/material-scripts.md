# Material Scripts {#Material-Scripts}

Material scripts offer you the ability to define complex materials in a script which can be reused easily. Whilst you could set up all materials for a scene in code using the methods of the Material and TextureLayer classes, in practice it’s a bit unwieldy. Instead you can store material definitions in text files which can then be loaded whenever required.

@tableofcontents

# Loading scripts {#Loading-scripts}

Material scripts are loaded when resource groups are initialised: OGRE looks in all resource locations associated with the group (see Root::addResourceLocation) for files with the ’.material’ extension and parses them. If you want to parse files manually, use MaterialSerializer::parseScript.

It’s important to realise that materials are not loaded completely by this parsing process: only the definition is loaded, no textures or other resources are loaded. This is because it is common to have a large library of materials, but only use a relatively small subset of them in any one scene. To load every material completely in every script would therefore cause unnecessary memory overhead. You can access a ’deferred load’ Material in the normal way (MaterialManager::getSingleton().getByName()), but you must call the ’load’ method before trying to use it. Ogre does this for you when using the normal material assignment methods of entities etc.

Another important factor is that material names must be unique throughout ALL scripts loaded by the system, since materials are always identified by name.

# Format {#Format}

Several materials may be defined in a single script. The script format is pseudo-C++, with sections delimited by curly braces (’{’, ’}’), and comments indicated by starting a line with ’//’ (note, no nested form comments allowed). The general format is shown below in the example below (note that to start with, we only consider fixed-function materials which don’t use vertex, geometry or fragment programs, these are covered later):

```cpp
// This is a comment
material walls/funkywall1
{
    // first, preferred technique
    technique
    {
        // first pass
        pass
        {
            ambient 0.5 0.5 0.5
            diffuse 1.0 1.0 1.0
            
            // Texture unit 0
            texture_unit 
            {
                texture wibbly.jpg
                scroll_anim 0.1 0.0
                wave_xform scale sine 0.0 0.7 0.0 1.0
            }
            // Texture unit 1 (this is a multitexture pass)
            texture_unit
            {
                texture wobbly.png
                rotate_anim 0.25
                colour_op add
            }
        }
    }

    // Second technique, can be used as a fallback or LOD level
    technique
    {
        // .. and so on
    }   
}
```

Every material in the script must be given a name, which is the line ’material &lt;blah&gt;’ before the first opening ’{’. This name must be globally unique. It can include path characters (as in the example) to logically divide up your materials, and also to avoid duplicate names, but the engine does not treat the name as hierarchical, just as a string. If you include spaces in the name, it must be enclosed in double quotes.

**NOTE: ’:’ is the delimiter for specifying material copy in the script so it can’t be used as part of the material name.** 

A material can inherit from a previously defined material by using a *colon* **:** after the material name followed by the name of the reference material to inherit from. You can in fact even inherit just *parts* of a material from others; all this is covered in See [Script Inheritance](#Script-Inheritance)). You can also use variables in your script which can be replaced in inheriting versions, see See [Script Variables](#Script-Variables).

A material can be made up of many techniques (See [Techniques](@ref Techniques))- a technique is one way of achieving the effect you are looking for. You can supply more than one technique in order to provide fallback approaches where a card does not have the ability to render the preferred technique, or where you wish to define lower level of detail versions of the material in order to conserve rendering power when objects are more distant. 

Each technique can be made up of many passes (See [Passes](#Passes)), that is a complete render of the object can be performed multiple times with different settings in order to produce composite effects. Ogre may also split the passes you have defined into many passes at runtime, if you define a pass which uses too many texture units for the card you are currently running on (note that it can only do this if you are not using a fragment program). Each pass has a number of top-level attributes such as ’ambient’ to set the amount & colour of the ambient light reflected by the material. Some of these options do not apply if you are using vertex programs, See [Passes](#Passes) for more details. 

Within each pass, there can be zero or many texture units in use (See [Texture Units](#Texture-Units)). These define the texture to be used, and optionally some blending operations (which use multitexturing) and texture effects.

You can also reference vertex and fragment programs (or vertex and pixel shaders, if you want to use that terminology) in a pass with a given set of parameters. Programs themselves are declared in separate .program scripts (See [Declaring Vertex/Geometry/Fragment Programs](#Declaring-Vertex_002fGeometry_002fFragment-Programs)) and are used as described in [Using Vertex/Geometry/Fragment Programs in a Pass](#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass).

<a name="Top_002dlevel-material-attributes"></a>

## Top-level material attributes

The outermost section of a material definition does not have a lot of attributes of its own (most of the configurable parameters are within the child sections. However, it does have some, and here they are: <a name="lod_005fdistances"></a>

## lod\_distances (deprecated)

This option is deprecated in favour of [lod\_values](#lod_005fvalues) now.  <a name="lod_005fstrategy"></a>

<a name="lod_005fstrategy-1"></a>

## lod\_strategy

Sets the name of the LOD strategy to use. Defaults to ’Distance’ which means LOD changes based on distance from the camera. Also supported is ’PixelCount’ which changes LOD based on an estimate of the screen-space pixels affected.  Format: lod\_strategy &lt;name&gt;<br> Default: lod\_strategy Distance



<a name="lod_005fvalues"></a>

## lod\_values

<a name="lod_005fvalues-1"></a>

## lod\_values

This attribute defines the values used to control the LOD transition for this material. By setting this attribute, you indicate that you want this material to alter the Technique that it uses based on some metric, such as the distance from the camera, or the approximate screen space coverage. The exact meaning of these values is determined by the option you select for [lod\_strategy](#lod_005fstrategy) - it is a list of distances for the ’Distance’ strategy, and a list of pixel counts for the ’PixelCount’ strategy, for example. You must give it a list of values, in order from highest LOD value to lowest LOD value, each one indicating the point at which the material will switch to the next LOD. Implicitly, all materials activate LOD index 0 for values less than the first entry, so you do not have to specify ’0’ at the start of the list. You must ensure that there is at least one Technique with a [lod\_index](#lod_005findex) value for each value in the list (so if you specify 3 values, you must have techniques for LOD indexes 0, 1, 2 and 3). Note you must always have at least one Technique at lod\_index 0.  Format: lod\_values &lt;value0&gt; &lt;value1&gt; &lt;value2&gt; ...<br> Default: none

Example: <br> lod\_strategy Distance lod\_values 300.0 600.5 1200

The above example would cause the material to use the best Technique at lod\_index 0 up to a distance of 300 world units, the best from lod\_index 1 from 300 up to 600, lod\_index 2 from 600 to 1200, and lod\_index 3 from 1200 upwards.

<a name="receive_005fshadows"></a><a name="receive_005fshadows-1"></a>

## receive\_shadows

This attribute controls whether objects using this material can have shadows cast upon them.

Format: receive\_shadows &lt;on|off&gt;<br> Default: on

Whether or not an object receives a shadow is the combination of a number of factors, See [Shadows](#Shadows) for full details; however this allows you to make a material opt-out of receiving shadows if required. Note that transparent materials never receive shadows so this option only has an effect on solid materials.

<a name="transparency_005fcasts_005fshadows"></a><a name="transparency_005fcasts_005fshadows-1"></a>

## transparency\_casts\_shadows

This attribute controls whether transparent materials can cast certain kinds of shadow.

Format: transparency\_casts\_shadows &lt;on|off&gt;<br> Default: off Whether or not an object casts a shadow is the combination of a number of factors, See [Shadows](#Shadows) for full details; however this allows you to make a transparent material cast shadows, when it would otherwise not. For example, when using texture shadows, transparent materials are normally not rendered into the shadow texture because they should not block light. This flag overrides that.

<a name="set_005ftexture_005falias"></a><a name="set_005ftexture_005falias-1"></a>

## set\_texture\_alias

This attribute associates a texture alias with a texture name.

Format: set\_texture\_alias &lt;alias name&gt; &lt;texture name&gt;

This attribute can be used to set the textures used in texture unit states that were inherited from another material.(See [Texture Aliases](#Texture-Aliases))



## Techniques {#Techniques}

A "technique" section in your material script encapsulates a single method of rendering an object. The simplest of material definitions only contains a single technique, however since PC hardware varies quite greatly in it’s capabilities, you can only do this if you are sure that every card for which you intend to target your application will support the capabilities which your technique requires. In addition, it can be useful to define simpler ways to render a material if you wish to use material LOD, such that more distant objects use a simpler, less performance-hungry technique.

When a material is used for the first time, it is ’compiled’. That involves scanning the techniques which have been defined, and marking which of them are supportable using the current rendering API and graphics card. If no techniques are supportable, your material will render as blank white. The compilation examines a number of things, such as:

-   The number of texture\_unit entries in each pass<br> Note that if the number of texture\_unit entries exceeds the number of texture units in the current graphics card, the technique may still be supportable so long as a fragment program is not being used. In this case, Ogre will split the pass which has too many entries into multiple passes for the less capable card, and the multitexture blend will be turned into a multipass blend (See [colour\_op\_multipass\_fallback](#colour_005fop_005fmultipass_005ffallback)).
-   Whether vertex, geometry or fragment programs are used, and if so which syntax they use (e.g. vs\_1\_1, ps\_2\_x, arbfp1 etc.)
-   Other effects like cube mapping and dot3 blending
-   Whether the vendor or device name of the current graphics card matches some user-specified rules

<br>

In a material script, techniques must be listed in order of preference, i.e. the earlier techniques are preferred over the later techniques. This normally means you will list your most advanced, most demanding techniques first in the script, and list fallbacks afterwards.

To help clearly identify what each technique is used for, the technique can be named but its optional. Techniques not named within the script will take on a name that is the technique index number. For example: the first technique in a material is index 0, its name would be "0" if it was not given a name in the script. The technique name must be unique within the material or else the final technique is the resulting merge of all techniques with the same name in the material. A warning message is posted in the Ogre.log if this occurs. Named techniques can help when inheriting a material and modifying an existing technique: (See [Script Inheritance](#Script-Inheritance))

Format: technique name

Techniques have only a small number of attributes of their own:

-   [scheme](#scheme)
-   [lod\_index](#lod_005findex) (and also see [lod\_distances](#lod_005fdistances) in the parent material)
-   [shadow\_caster\_material](#shadow_005fcaster_005fmaterial)
-   [shadow\_receiver\_material](#shadow_005freceiver_005fmaterial)
-   [gpu\_vendor\_rule](#gpu_005fvendor_005frule)
-   [gpu\_device\_rule](#gpu_005fdevice_005frule)

<a name="scheme"></a><a name="scheme-1"></a>

## scheme

Sets the ’scheme’ this Technique belongs to. Material schemes are used to control top-level switching from one set of techniques to another. For example, you might use this to define ’high’, ’medium’ and ’low’ complexity levels on materials to allow a user to pick a performance / quality ratio. Another possibility is that you have a fully HDR-enabled pipeline for top machines, rendering all objects using unclamped shaders, and a simpler pipeline for others; this can be implemented using schemes. The active scheme is typically controlled at a viewport level, and the active one defaults to ’Default’.

Format: scheme &lt;name&gt;<br> Example: scheme hdr<br> Default: scheme Default

<a name="lod_005findex"></a><a name="lod_005findex-1"></a>

## lod\_index

Sets the level-of-detail (LOD) index this Technique belongs to. 

Format: lod\_index &lt;number&gt;<br> NB Valid values are 0 (highest level of detail) to 65535, although this is unlikely. You should not leave gaps in the LOD indexes between Techniques.

Example: lod\_index 1

All techniques must belong to a LOD index, by default they all belong to index 0, i.e. the highest LOD. Increasing indexes denote lower levels of detail. You can (and often will) assign more than one technique to the same LOD index, what this means is that OGRE will pick the best technique of the ones listed at the same LOD index. For readability, it is advised that you list your techniques in order of LOD, then in order of preference, although the latter is the only prerequisite (OGRE determines which one is ’best’ by which one is listed first). You must always have at least one Technique at lod\_index 0. The distance at which a LOD level is applied is determined by the lod\_distances attribute of the containing material, See [lod\_distances](#lod_005fdistances) for details.

Default: lod\_index 0

Techniques also contain one or more passes (and there must be at least one), See [Passes](#Passes).

<a name="shadow_005fcaster_005fmaterial"></a><a name="shadow_005fcaster_005fmaterial-1"></a>

## shadow\_caster\_material

When using See [Texture-based Shadows](#Texture_002dbased-Shadows) you can specify an alternate material to use when rendering the object using this material into the shadow texture. This is like a more advanced version of using shadow\_caster\_vertex\_program, however note that for the moment you are expected to render the shadow in one pass, i.e. only the first pass is respected.

<a name="shadow_005freceiver_005fmaterial"></a><a name="shadow_005freceiver_005fmaterial-1"></a>

## shadow\_receiver\_material

When using See [Texture-based Shadows](#Texture_002dbased-Shadows) you can specify an alternate material to use when performing the receiver shadow pass. Note that this explicit ’receiver’ pass is only done when you’re **not** using [Integrated Texture Shadows](#Integrated-Texture-Shadows) - i.e. the shadow rendering is done separately (either as a modulative pass, or a masked light pass). This is like a more advanced version of using shadow\_receiver\_vertex\_program and shadow\_receiver\_fragment\_program, however note that for the moment you are expected to render the shadow in one pass, i.e. only the first pass is respected.

<a name="gpu_005fvendor_005frule"></a><a name="gpu_005fdevice_005frule"></a><a name="gpu_005fvendor_005frule-and-gpu_005fdevice_005frule"></a>

## gpu\_vendor\_rule and gpu\_device\_rule

Although Ogre does a good job of detecting the capabilities of graphics cards and setting the supportability of techniques from that, occasionally card-specific behaviour exists which is not necessarily detectable and you may want to ensure that your materials go down a particular path to either use or avoid that behaviour. This is what these rules are for - you can specify matching rules so that a technique will be considered supportable only on cards from a particular vendor, or which match a device name pattern, or will be considered supported only if they **don’t** fulfil such matches. The format of the rules are as follows:

gpu\_vendor\_rule &lt;include|exclude&gt; &lt;vendor\_name&gt;<br> gpu\_device\_rule &lt;include|exclude&gt; &lt;device\_pattern&gt; \[case\_sensitive\]<br> <br> An ’include’ rule means that the technique will only be supported if one of the include rules is matched (if no include rules are provided, anything will pass). An ’exclude’ rules means that the technique is considered unsupported if any of the exclude rules are matched. You can provide as many rules as you like, although &lt;vendor\_name&gt; and &lt;device\_pattern&gt; must obviously be unique. The valid list of &lt;vendor\_name&gt; values is currently ’nvidia’, ’ati’, ’intel’, ’s3’, ’matrox’ and ’3dlabs’. &lt;device\_pattern&gt; can be any string, and you can use wildcards (’\*’) if you need to match variants. Here’s an example:

gpu\_vendor\_rule include nvidia<br> gpu\_vendor\_rule include intel<br> gpu\_device\_rule exclude \*950\*<br> <br> These rules, if all included in one technique, will mean that the technique will only be considered supported on graphics cards made by NVIDIA and Intel, and so long as the device name doesn’t have ’950’ in it.

Note that these rules can only mark a technique ’unsupported’ when it would otherwise be considered ’supported’ judging by the hardware capabilities. Even if a technique passes these rules, it is still subject to the usual hardware support tests.

## Passes {#Passes}

A pass is a single render of the geometry in question; a single call to the rendering API with a certain set of rendering properties. A technique can have between one and 16 passes, although clearly the more passes you use, the more expensive the technique will be to render.

To help clearly identify what each pass is used for, the pass can be named but its optional. Passes not named within the script will take on a name that is the pass index number. For example: the first pass in a technique is index 0 so its name would be "0" if it was not given a name in the script. The pass name must be unique within the technique or else the final pass is the resulting merge of all passes with the same name in the technique. A warning message is posted in the Ogre.log if this occurs. Named passes can help when inheriting a material and modifying an existing pass: (See [Script Inheritance](#Script-Inheritance))

Passes have a set of global attributes (described below), zero or more nested texture\_unit entries (See [Texture Units](#Texture-Units)), and optionally a reference to a vertex and / or a fragment program (See [Using Vertex/Geometry/Fragment Programs in a Pass](#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass)).



Here are the attributes you can use in a ’pass’ section of a .material script:

-   [ambient](#ambient)
-   [diffuse](#diffuse)
-   [specular](#specular)
-   [emissive](#emissive)
-   [scene\_blend](#scene_005fblend)
-   [separate\_scene\_blend](#separate_005fscene_005fblend)
-   [scene\_blend\_op](#scene_005fblend_005fop)
-   [separate\_scene\_blend\_op](#separate_005fscene_005fblend_005fop)
-   [depth\_check](#depth_005fcheck)
-   [depth\_write](#depth_005fwrite)
-   [depth\_func](#depth_005ffunc)
-   [depth\_bias](#depth_005fbias)
-   [iteration\_depth\_bias](#iteration_005fdepth_005fbias)
-   [alpha\_rejection](#alpha_005frejection)
-   [alpha\_to\_coverage](#alpha_005fto_005fcoverage)
-   [light\_scissor](#light_005fscissor)
-   [light\_clip\_planes](#light_005fclip_005fplanes)
-   [illumination\_stage](#illumination_005fstage)
-   [transparent\_sorting](#transparent_005fsorting)
-   [normalise\_normals](#normalise_005fnormals)
-   [cull\_hardware](#cull_005fhardware)
-   [cull\_software](#cull_005fsoftware)
-   [lighting](#lighting)
-   [shading](#shading)
-   [polygon\_mode](#polygon_005fmode)
-   [polygon\_mode\_overrideable](#polygon_005fmode_005foverrideable)
-   [fog\_override](#fog_005foverride)
-   [colour\_write](#colour_005fwrite)
-   [max\_lights](#max_005flights)
-   [start\_light](#start_005flight)
-   [iteration](#iteration)
-   [point\_size](#point_005fsize)
-   [point\_sprites](#point_005fsprites)
-   [point\_size\_attenuation](#point_005fsize_005fattenuation)
-   [point\_size\_min](#point_005fsize_005fmin)
-   [point\_size\_max](#point_005fsize_005fmax)

<a name="Attribute-Descriptions"></a>

# Attribute Descriptions

<a name="ambient"></a><a name="ambient-1"></a>

## ambient

Sets the ambient colour reflectance properties of this pass. **This attribute has no effect if an asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.** 

Format: ambient (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

Example: ambient 0.0 0.8 0.0

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much ambient light (directionless global light) is reflected. It is also possible to make the ambient reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is full white, meaning objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light effects, or change the blend of colours to make the object have a base colour other than white. This setting has no effect if dynamic lighting is disabled using the ’lighting off’ attribute, or if any texture layer has a ’colour\_op replace’ attribute.

Default: ambient 1.0 1.0 1.0 1.0

<a name="diffuse"></a><a name="diffuse-1"></a>

## diffuse

Sets the diffuse colour reflectance properties of this pass. **This attribute has no effect if an asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.**

Format: diffuse (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

Example: diffuse 1.0 0.5 0.5

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much diffuse light (light from instances of the Light class in the scene) is reflected. It is also possible to make the diffuse reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is full white, meaning objects reflect the maximum white light they can from Light objects. This setting has no effect if dynamic lighting is disabled using the ’lighting off’ attribute, or if any texture layer has a ’colour\_op replace’ attribute.

Default: diffuse 1.0 1.0 1.0 1.0

<a name="specular"></a><a name="specular-1"></a>

## specular

Sets the specular colour reflectance properties of this pass. **This attribute has no effect if an asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.**

Format: specular (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour) &lt;shininess&gt;<br> NB valid colour values are between 0.0 and 1.0. Shininess can be any value greater than 0.

Example: specular 1.0 1.0 1.0 12.5

The base colour of a pass is determined by how much red, green and blue light is reflects at each vertex. This property determines how much specular light (highlights from instances of the Light class in the scene) is reflected. It is also possible to make the diffuse reflectance track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. The default is to reflect no specular light. The colour of the specular highlights is determined by the colour parameters, and the size of the highlights by the separate shininess parameter.. The higher the value of the shininess parameter, the sharper the highlight i.e. the radius is smaller. Beware of using shininess values in the range of 0 to 1 since this causes the the specular colour to be applied to the whole surface that has the material applied to it. When the viewing angle to the surface changes, ugly flickering will also occur when shininess is in the range of 0 to 1. Shininess values between 1 and 128 work best in both DirectX and OpenGL renderers. This setting has no effect if dynamic lighting is disabled using the ’lighting off’ attribute, or if any texture layer has a ’colour\_op replace’ attribute.

Default: specular 0.0 0.0 0.0 0.0 0.0

<a name="emissive"></a><a name="emissive-1"></a>

## emissive

Sets the amount of self-illumination an object has. **This attribute has no effect if an asm, CG, or HLSL shader program is used. With GLSL, the shader can read the OpenGL material state.**

Format: emissive (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

Example: emissive 1.0 0.0 0.0

If an object is self-illuminating, it does not need external sources to light it, ambient or otherwise. It’s like the object has it’s own personal ambient light. Unlike the name suggests, this object doesn’t act as a light source for other objects in the scene (if you want it to, you have to create a light which is centered on the object). It is also possible to make the emissive colour track the vertex colour as defined in the mesh by using the keyword vertexcolour instead of the colour values. This setting has no effect if dynamic lighting is disabled using the ’lighting off’ attribute, or if any texture layer has a ’colour\_op replace’ attribute.

Default: emissive 0.0 0.0 0.0 0.0

<a name="scene_005fblend"></a><a name="scene_005fblend-1"></a>

## scene\_blend

Sets the kind of blending this pass has with the existing contents of the scene. Whereas the texture blending operations seen in the texture\_unit entries are concerned with blending between texture layers, this blending is about combining the output of this pass as a whole with the existing contents of the rendering target. This blending therefore allows object transparency and other special effects. There are 2 formats, one using predefined blend types, the other allowing a roll-your-own approach using source and destination factors.

Format1: scene\_blend &lt;add|modulate|alpha\_blend|colour\_blend&gt;

Example: scene\_blend add

This is the simpler form, where the most commonly used blending modes are enumerated using a single parameter. Valid &lt;blend\_type&gt; parameters are:

<dl compact="compact">
<dt>add</dt> <dd>

The colour of the rendering output is added to the scene. Good for explosions, flares, lights, ghosts etc. Equivalent to ’scene\_blend one one’.

</dd> <dt>modulate</dt> <dd>

The colour of the rendering output is multiplied with the scene contents. Generally colours and darkens the scene, good for smoked glass, semi-transparent objects etc. Equivalent to ’scene\_blend dest\_colour zero’.

</dd> <dt>colour\_blend</dt> <dd>

Colour the scene based on the brightness of the input colours, but don’t darken. Equivalent to ’scene\_blend src\_colour one\_minus\_src\_colour’

</dd> <dt>alpha\_blend</dt> <dd>

The alpha value of the rendering output is used as a mask. Equivalent to ’scene\_blend src\_alpha one\_minus\_src\_alpha’

</dd> </dl> <br>

Format2: scene\_blend &lt;src\_factor&gt; &lt;dest\_factor&gt;

Example: scene\_blend one one\_minus\_dest\_alpha

This version of the method allows complete control over the blending operation, by specifying the source and destination blending factors. The resulting colour which is written to the rendering target is (texture \* sourceFactor) + (scene\_pixel \* destFactor). Valid values for both parameters are:

<dl compact="compact">
<dt>one</dt> <dd>

Constant value of 1.0

</dd> <dt>zero</dt> <dd>

Constant value of 0.0

</dd> <dt>dest\_colour</dt> <dd>

The existing pixel colour

</dd> <dt>src\_colour</dt> <dd>

The texture pixel (texel) colour

</dd> <dt>one\_minus\_dest\_colour</dt> <dd>

1 - (dest\_colour)

</dd> <dt>one\_minus\_src\_colour</dt> <dd>

1 - (src\_colour)

</dd> <dt>dest\_alpha</dt> <dd>

The existing pixel alpha value

</dd> <dt>src\_alpha</dt> <dd>

The texel alpha value

</dd> <dt>one\_minus\_dest\_alpha</dt> <dd>

1 - (dest\_alpha)

</dd> <dt>one\_minus\_src\_alpha</dt> <dd>

1 - (src\_alpha)

</dd> </dl> <br>

Default: scene\_blend one zero (opaque)  Also see [separate\_scene\_blend](#separate_005fscene_005fblend).

<a name="separate_005fscene_005fblend"></a><a name="separate_005fscene_005fblend-1"></a>

## separate\_scene\_blend

This option operates in exactly the same way as [scene\_blend](#scene_005fblend), except that it allows you to specify the operations to perform between the rendered pixel and the frame buffer separately for colour and alpha components. By nature this option is only useful when rendering to targets which have an alpha channel which you’ll use for later processing, such as a render texture.

Format1: separate\_scene\_blend &lt;simple\_colour\_blend&gt; &lt;simple\_alpha\_blend&gt;

Example: separate\_scene\_blend add modulate

This example would add colour components but multiply alpha components. The blend modes available are as in [scene\_blend](#scene_005fblend). The more advanced form is also available:

Format2: separate\_scene\_blend &lt;colour\_src\_factor&gt; &lt;colour\_dest\_factor&gt; &lt;alpha\_src\_factor&gt; &lt;alpha\_dest\_factor&gt;

Example: separate\_scene\_blend one one\_minus\_dest\_alpha one one 

Again the options available in the second format are the same as those in the second format of [scene\_blend](#scene_005fblend).

<a name="scene_005fblend_005fop"></a><a name="scene_005fblend_005fop-1"></a>

## scene\_blend\_op

This directive changes the operation which is applied between the two components of the scene blending equation, which by default is ’add’ (sourceFactor \* source + destFactor \* dest). You may change this to ’add’, ’subtract’, ’reverse\_subtract’, ’min’ or ’max’.

Format: scene\_blend\_op &lt;add|subtract|reverse\_subtract|min|max&gt; Default: scene\_blend\_op add

<a name="separate_005fscene_005fblend_005fop"></a><a name="separate_005fscene_005fblend_005fop-1"></a>

## separate\_scene\_blend\_op

This directive is as scene\_blend\_op, except that you can set the operation for colour and alpha separately.

Format: separate\_scene\_blend\_op &lt;colourOp&gt; &lt;alphaOp&gt; Default: separate\_scene\_blend\_op add add

<a name="depth_005fcheck"></a><a name="depth_005fcheck-1"></a>

## depth\_check

Sets whether or not this pass renders with depth-buffer checking on or not.

Format: depth\_check &lt;on|off&gt;

If depth-buffer checking is on, whenever a pixel is about to be written to the frame buffer the depth buffer is checked to see if the pixel is in front of all other pixels written at that point. If not, the pixel is not written. If depth checking is off, pixels are written no matter what has been rendered before. Also see depth\_func for more advanced depth check configuration.

Default: depth\_check on

<a name="depth_005fwrite"></a><a name="depth_005fwrite-1"></a>

## depth\_write

Sets whether or not this pass renders with depth-buffer writing on or not.<br>

Format: depth\_write &lt;on|off&gt;

If depth-buffer writing is on, whenever a pixel is written to the frame buffer the depth buffer is updated with the depth value of that new pixel, thus affecting future rendering operations if future pixels are behind this one. If depth writing is off, pixels are written without updating the depth buffer. Depth writing should normally be on but can be turned off when rendering static backgrounds or when rendering a collection of transparent objects at the end of a scene so that they overlap each other correctly.

Default: depth\_write on<br>

<a name="depth_005ffunc"></a><a name="depth_005ffunc-1"></a>

## depth\_func

Sets the function used to compare depth values when depth checking is on.

Format: depth\_func &lt;func&gt;

If depth checking is enabled (see depth\_check) a comparison occurs between the depth value of the pixel to be written and the current contents of the buffer. This comparison is normally less\_equal, i.e. the pixel is written if it is closer (or at the same distance) than the current contents. The possible functions are:

<dl compact="compact">
<dt>always\_fail</dt> <dd>

Never writes a pixel to the render target

</dd> <dt>always\_pass</dt> <dd>

Always writes a pixel to the render target

</dd> <dt>less</dt> <dd>

Write if (new\_Z &lt; existing\_Z)

</dd> <dt>less\_equal</dt> <dd>

Write if (new\_Z &lt;= existing\_Z)

</dd> <dt>equal</dt> <dd>

Write if (new\_Z == existing\_Z)

</dd> <dt>not\_equal</dt> <dd>

Write if (new\_Z != existing\_Z)

</dd> <dt>greater\_equal</dt> <dd>

Write if (new\_Z &gt;= existing\_Z)

</dd> <dt>greater</dt> <dd>

Write if (new\_Z &gt;existing\_Z)

</dd> </dl> <br>

Default: depth\_func less\_equal

<a name="depth_005fbias"></a><a name="depth_005fbias-1"></a>

## depth\_bias

Sets the bias applied to the depth value of this pass. Can be used to make coplanar polygons appear on top of others e.g. for decals. 

Format: depth\_bias &lt;constant\_bias&gt; \[&lt;slopescale\_bias&gt;\]

The final depth bias value is constant\_bias \* minObservableDepth + maxSlope \* slopescale\_bias. Slope scale biasing is relative to the angle of the polygon to the camera, which makes for a more appropriate bias value, but this is ignored on some older hardware. Constant biasing is expressed as a factor of the minimum depth value, so a value of 1 will nudge the depth by one ’notch’ if you will. Also see [iteration\_depth\_bias](#iteration_005fdepth_005fbias)

<a name="iteration_005fdepth_005fbias"></a><a name="iteration_005fdepth_005fbias-1"></a>

## iteration\_depth\_bias

Sets an additional bias derived from the number of times a given pass has been iterated. Operates just like [depth\_bias](#depth_005fbias) except that it applies an additional bias factor to the base depth\_bias value, multiplying the provided value by the number of times this pass has been iterated before, through one of the [iteration](#iteration) variants. So the first time the pass will get the depth\_bias value, the second time it will get depth\_bias + iteration\_depth\_bias, the third time it will get depth\_bias + iteration\_depth\_bias \* 2, and so on. The default is zero. 

Format: iteration\_depth\_bias &lt;bias\_per\_iteration&gt;

 <a name="alpha_005frejection"></a><a name="alpha_005frejection-1"></a>

## alpha\_rejection

Sets the way the pass will have use alpha to totally reject pixels from the pipeline.

Format: alpha\_rejection &lt;function&gt; &lt;value&gt;

Example: alpha\_rejection greater\_equal 128

The function parameter can be any of the options listed in the material depth\_function attribute. The value parameter can theoretically be any value between 0 and 255, but is best limited to 0 or 128 for hardware compatibility.

Default: alpha\_rejection always\_pass

<a name="alpha_005fto_005fcoverage"></a><a name="alpha_005fto_005fcoverage-1"></a>

## alpha\_to\_coverage

Sets whether this pass will use ’alpha to coverage’, a way to multisample alpha texture edges so they blend more seamlessly with the background. This facility is typically only available on cards from around 2006 onwards, but it is safe to enable it anyway - Ogre will just ignore it if the hardware does not support it. The common use for alpha to coverage is foliage rendering and chain-link fence style textures. 

Format: alpha\_to\_coverage &lt;on|off&gt;

Default: alpha\_to\_coverage off <a name="light_005fscissor"></a>

<a name="light_005fscissor-1"></a>

## light\_scissor

Sets whether when rendering this pass, rendering will be limited to a screen-space scissor rectangle representing the coverage of the light(s) being used in this pass, derived from their attenuation ranges.

Format: light\_scissor &lt;on|off&gt; Default: light\_scissor off

This option is usually only useful if this pass is an additive lighting pass, and is at least the second one in the technique. Ie areas which are not affected by the current light(s) will never need to be rendered. If there is more than one light being passed to the pass, then the scissor is defined to be the rectangle which covers all lights in screen-space. Directional lights are ignored since they are infinite.

This option does not need to be specified if you are using a standard additive shadow mode, i.e. SHADOWTYPE\_STENCIL\_ADDITIVE or SHADOWTYPE\_TEXTURE\_ADDITIVE, since it is the default behaviour to use a scissor for each additive shadow pass. However, if you’re not using shadows, or you’re using [Integrated Texture Shadows](#Integrated-Texture-Shadows) where passes are specified in a custom manner, then this could be of use to you.

<a name="light_005fclip_005fplanes"></a><a name="light_005fclip_005fplanes-1"></a>

## light\_clip\_planes

Sets whether when rendering this pass, triangle setup will be limited to clipping volume covered by the light. Directional lights are ignored, point lights clip to a cube the size of the attenuation range or the light, and spotlights clip to a pyramid bounding the spotlight angle and attenuation range.

Format: light\_clip\_planes &lt;on|off&gt; Default: light\_clip\_planes off

This option will only function if there is a single non-directional light being used in this pass. If there is more than one light, or only directional lights, then no clipping will occur. If there are no lights at all then the objects won’t be rendered at all.

When using a standard additive shadow mode, i.e. SHADOWTYPE\_STENCIL\_ADDITIVE or SHADOWTYPE\_TEXTURE\_ADDITIVE, you have the option of enabling clipping for all light passes by calling SceneManager::setShadowUseLightClipPlanes regardless of this pass setting, since rendering is done lightwise anyway. This is off by default since using clip planes is not always faster - it depends on how much of the scene the light volumes cover. Generally the smaller your lights are the more chance you’ll see a benefit rather than a penalty from clipping. If you’re not using shadows, or you’re using [Integrated Texture Shadows](#Integrated-Texture-Shadows) where passes are specified in a custom manner, then specify the option per-pass using this attribute. A specific note about OpenGL: user clip planes are completely ignored when you use an ARB vertex program. This means light clip planes won’t help much if you use ARB vertex programs on GL, although OGRE will perform some optimisation of its own, in that if it sees that the clip volume is completely off-screen, it won’t perform a render at all. When using GLSL, user clipping can be used but you have to use gl\_ClipVertex in your shader, see the GLSL documentation for more information. In Direct3D user clip planes are always respected.

<a name="illumination_005fstage"></a><a name="illumination_005fstage-1"></a>

## illumination\_stage

When using an additive lighting mode (SHADOWTYPE\_STENCIL\_ADDITIVE or SHADOWTYPE\_TEXTURE\_ADDITIVE), the scene is rendered in 3 discrete stages, ambient (or pre-lighting), per-light (once per light, with shadowing) and decal (or post-lighting). Usually OGRE figures out how to categorise your passes automatically, but there are some effects you cannot achieve without manually controlling the illumination. For example specular effects are muted by the typical sequence because all textures are saved until the ’decal’ stage which mutes the specular effect. Instead, you could do texturing within the per-light stage if it’s possible for your material and thus add the specular on after the decal texturing, and have no post-light rendering. 

If you assign an illumination stage to a pass you have to assign it to all passes in the technique otherwise it will be ignored. Also note that whilst you can have more than one pass in each group, they cannot alternate, i.e. all ambient passes will be before all per-light passes, which will also be before all decal passes. Within their categories the passes will retain their ordering though.

Format: illumination\_stage &lt;ambient|per\_light|decal&gt; Default: none (autodetect)

<a name="normalise_005fnormals"></a><a name="normalise_005fnormals-1"></a>

## normalise\_normals

Sets whether or not this pass renders with all vertex normals being automatically re-normalised.<br>

Format: normalise\_normals &lt;on|off&gt;

Scaling objects causes normals to also change magnitude, which can throw off your lighting calculations. By default, the SceneManager detects this and will automatically re-normalise normals for any scaled object, but this has a cost. If you’d prefer to control this manually, call SceneManager::setNormaliseNormalsOnScale(false) and then use this option on materials which are sensitive to normals being resized. 

Default: normalise\_normals off<br>

<a name="transparent_005fsorting"></a><a name="transparent_005fsorting-1"></a>

## transparent\_sorting

Sets if transparent textures should be sorted by depth or not.

Format: transparent\_sorting &lt;on|off|force&gt;

By default all transparent materials are sorted such that renderables furthest away from the camera are rendered first. This is usually the desired behaviour but in certain cases this depth sorting may be unnecessary and undesirable. If for example it is necessary to ensure the rendering order does not change from one frame to the next. In this case you could set the value to ’off’ to prevent sorting.

You can also use the keyword ’force’ to force transparent sorting on, regardless of other circumstances. Usually sorting is only used when the pass is also transparent, and has a depth write or read which indicates it cannot reliably render without sorting. By using ’force’, you tell OGRE to sort this pass no matter what other circumstances are present.

Default: transparent\_sorting on

<a name="cull_005fhardware"></a><a name="cull_005fhardware-1"></a>

## cull\_hardware

Sets the hardware culling mode for this pass.

Format: cull\_hardware &lt;clockwise|anticlockwise|none&gt;

A typical way for the hardware rendering engine to cull triangles is based on the ’vertex winding’ of triangles. Vertex winding refers to the direction in which the vertices are passed or indexed to in the rendering operation as viewed from the camera, and will wither be clockwise or anticlockwise (that’s ’counterclockwise’ for you Americans out there ;). If the option ’cull\_hardware clockwise’ is set, all triangles whose vertices are viewed in clockwise order from the camera will be culled by the hardware. ’anticlockwise’ is the reverse (obviously), and ’none’ turns off hardware culling so all triangles are rendered (useful for creating 2-sided passes).

Default: cull\_hardware clockwise<br> NB this is the same as OpenGL’s default but the opposite of Direct3D’s default (because Ogre uses a right-handed coordinate system like OpenGL).

<a name="cull_005fsoftware"></a><a name="cull_005fsoftware-1"></a>

## cull\_software

Sets the software culling mode for this pass.

Format: cull\_software &lt;back|front|none&gt;

In some situations the engine will also cull geometry in software before sending it to the hardware renderer. This setting only takes effect on SceneManager’s that use it (since it is best used on large groups of planar world geometry rather than on movable geometry since this would be expensive), but if used can cull geometry before it is sent to the hardware. In this case the culling is based on whether the ’back’ or ’front’ of the triangle is facing the camera - this definition is based on the face normal (a vector which sticks out of the front side of the polygon perpendicular to the face). Since Ogre expects face normals to be on anticlockwise side of the face, ’cull\_software back’ is the software equivalent of ’cull\_hardware clockwise’ setting, which is why they are both the default. The naming is different to reflect the way the culling is done though, since most of the time face normals are pre-calculated and they don’t have to be the way Ogre expects - you could set ’cull\_hardware none’ and completely cull in software based on your own face normals, if you have the right SceneManager which uses them.

Default: cull\_software back

<a name="lighting"></a><a name="lighting-1"></a>

## lighting

Sets whether or not dynamic lighting is turned on for this pass or not. If lighting is turned off, all objects rendered using the pass will be fully lit. **This attribute has no effect if a vertex program is used.**

Format: lighting &lt;on|off&gt;

Turning dynamic lighting off makes any ambient, diffuse, specular, emissive and shading properties for this pass redundant. When lighting is turned on, objects are lit according to their vertex normals for diffuse and specular light, and globally for ambient and emissive.

Default: lighting on

<a name="shading"></a><a name="shading-1"></a>

## shading

Sets the kind of shading which should be used for representing dynamic lighting for this pass.

Format: shading &lt;flat|gouraud|phong&gt;

When dynamic lighting is turned on, the effect is to generate colour values at each vertex. Whether these values are interpolated across the face (and how) depends on this setting.

<dl compact="compact">
<dt>flat</dt> <dd>

No interpolation takes place. Each face is shaded with a single colour determined from the first vertex in the face.

</dd> <dt>gouraud</dt> <dd>

Colour at each vertex is linearly interpolated across the face.

</dd> <dt>phong</dt> <dd>

Vertex normals are interpolated across the face, and these are used to determine colour at each pixel. Gives a more natural lighting effect but is more expensive and works better at high levels of tessellation. Not supported on all hardware.

</dd> </dl>

Default: shading gouraud

<a name="polygon_005fmode"></a><a name="polygon_005fmode-1"></a>

## polygon\_mode

Sets how polygons should be rasterised, i.e. whether they should be filled in, or just drawn as lines or points.

Format: polygon\_mode &lt;solid|wireframe|points&gt;

<dl compact="compact">
<dt>solid</dt> <dd>

The normal situation - polygons are filled in.

</dd> <dt>wireframe</dt> <dd>

Polygons are drawn in outline only.

</dd> <dt>points</dt> <dd>

Only the points of each polygon are rendered.

</dd> </dl>

Default: polygon\_mode solid

<a name="polygon_005fmode_005foverrideable"></a><a name="polygon_005fmode_005foverrideable-1"></a>

## polygon\_mode\_overrideable

Sets whether or not the [polygon\_mode](#polygon_005fmode) set on this pass can be downgraded by the camera, if the camera itself is set to a lower polygon mode. If set to false, this pass will always be rendered at its own chosen polygon mode no matter what the camera says. The default is true.

Format: polygon\_mode\_overrideable &lt;true|false&gt;

<a name="fog_005foverride"></a><a name="fog_005foverride-1"></a>

## fog\_override

Tells the pass whether it should override the scene fog settings, and enforce it’s own. Very useful for things that you don’t want to be affected by fog when the rest of the scene is fogged, or vice versa. Note that this only affects fixed-function fog - the original scene fog parameters are still sent to shaders which use the fog\_params parameter binding (this allows you to turn off fixed function fog and calculate it in the shader instead; if you want to disable shader fog you can do that through shader parameters anyway). 

Format: fog\_override &lt;override?&gt; \[&lt;type&gt; &lt;colour&gt; &lt;density&gt; &lt;start&gt; &lt;end&gt;\]

Default: fog\_override false

If you specify ’true’ for the first parameter and you supply the rest of the parameters, you are telling the pass to use these fog settings in preference to the scene settings, whatever they might be. If you specify ’true’ but provide no further parameters, you are telling this pass to never use fogging no matter what the scene says. Here is an explanation of the parameters:<br>

<dl compact="compact">
<dt>type</dt> <dd>

**none** = No fog, equivalent of just using ’fog\_override true’<br> **linear** = Linear fog from the &lt;start&gt; and &lt;end&gt; distances<br> **exp** = Fog increases exponentially from the camera (fog = 1/e^(distance \* density)), use &lt;density&gt; param to control it<br> **exp2** = Fog increases at the square of FOG\_EXP, i.e. even quicker (fog = 1/e^(distance \* density)^2), use &lt;density&gt; param to control it

</dd> <dt>colour</dt> <dd>

Sequence of 3 floating point values from 0 to 1 indicating the red, green and blue intensities

</dd> <dt>density</dt> <dd>

The density parameter used in the ’exp’ or ’exp2’ fog types. Not used in linear mode but param must still be there as a placeholder

</dd> <dt>start</dt> <dd>

The start distance from the camera of linear fog. Must still be present in other modes, even though it is not used.

</dd> <dt>end</dt> <dd>

The end distance from the camera of linear fog. Must still be present in other modes, even though it is not used.

</dd> </dl> <br>

Example: fog\_override true exp 1 1 1 0.002 100 10000

<a name="colour_005fwrite"></a><a name="colour_005fwrite-1"></a>

## colour\_write

Sets whether or not this pass renders with colour writing on or not.<br>

Format: colour\_write &lt;on|off&gt;

If colour writing is off no visible pixels are written to the screen during this pass. You might think this is useless, but if you render with colour writing off, and with very minimal other settings, you can use this pass to initialise the depth buffer before subsequently rendering other passes which fill in the colour data. This can give you significant performance boosts on some newer cards, especially when using complex fragment programs, because if the depth check fails then the fragment program is never run. 

Default: colour\_write on<br>

<a name="start_005flight"></a><a name="start_005flight-1"></a>

## start\_light

Sets the first light which will be considered for use with this pass. Format: start\_light &lt;number&gt;

You can use this attribute to offset the starting point of the lights for this pass. In other words, if you set start\_light to 2 then the first light to be processed in that pass will be the third actual light in the applicable list. You could use this option to use different passes to process the first couple of lights versus the second couple of lights for example, or use it in conjunction with the [iteration](#iteration) option to start the iteration from a given point in the list (e.g. doing the first 2 lights in the first pass, and then iterating every 2 lights from then on perhaps). 

Default: start\_light 0<br>

<a name="max_005flights"></a><a name="max_005flights-1"></a>

## max\_lights

Sets the maximum number of lights which will be considered for use with this pass. Format: max\_lights &lt;number&gt;

The maximum number of lights which can be used when rendering fixed-function materials is set by the rendering system, and is typically set at 8. When you are using the programmable pipeline (See [Using Vertex/Geometry/Fragment Programs in a Pass](@ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass)) this limit is dependent on the program you are running, or, if you use ’iteration once\_per\_light’ or a variant (See [iteration](#iteration)), it effectively only bounded by the number of passes you are willing to use. If you are not using pass iteration, the light limit applies once for this pass. If you are using pass iteration, the light limit applies across all iterations of this pass - for example if you have 12 lights in range with an ’iteration once\_per\_light’ setup but your max\_lights is set to 4 for that pass, the pass will only iterate 4 times. 

Default: max\_lights 8<br>

<a name="iteration"></a><a name="iteration-1"></a>

## iteration

Sets whether or not this pass is iterated, i.e. issued more than once.

Format 1: iteration &lt;once | once\_per\_light&gt; \[lightType\] Format 2: iteration &lt;number&gt; \[&lt;per\_light&gt; \[lightType\]\] Format 3: iteration &lt;number&gt; \[&lt;per\_n\_lights&gt; &lt;num\_lights&gt; \[lightType\]\] Examples:

<dl compact="compact">
<dt>iteration once</dt> <dd>

The pass is only executed once which is the default behaviour.

</dd> <dt>iteration once\_per\_light point</dt> <dd>

The pass is executed once for each point light.

</dd> <dt>iteration 5</dt> <dd>

The render state for the pass will be setup and then the draw call will execute 5 times.

</dd> <dt>iteration 5 per\_light point</dt> <dd>

The render state for the pass will be setup and then the draw call will execute 5 times. This will be done for each point light.

</dd> <dt>iteration 1 per\_n\_lights 2 point</dt> <dd>

The render state for the pass will be setup and the draw call executed once for every 2 lights.

</dd> </dl> <br>

By default, passes are only issued once. However, if you use the programmable pipeline, or you wish to exceed the normal limits on the number of lights which are supported, you might want to use the once\_per\_light option. In this case, only light index 0 is ever used, and the pass is issued multiple times, each time with a different light in light index 0. Clearly this will make the pass more expensive, but it may be the only way to achieve certain effects such as per-pixel lighting effects which take into account 1..n lights.

Using a number instead of "once" instructs the pass to iterate more than once after the render state is setup. The render state is not changed after the initial setup so repeated draw calls are very fast and ideal for passes using programmable shaders that must iterate more than once with the same render state i.e. shaders that do fur, motion blur, special filtering.

If you use once\_per\_light, you should also add an ambient pass to the technique before this pass, otherwise when no lights are in range of this object it will not get rendered at all; this is important even when you have no ambient light in the scene, because you would still want the objects silhouette to appear.

The lightType parameter to the attribute only applies if you use once\_per\_light, per\_light, or per\_n\_lights and restricts the pass to being run for lights of a single type (either ’point’, ’directional’ or ’spot’). In the example, the pass will be run once per point light. This can be useful because when you’re writing a vertex / fragment program it is a lot easier if you can assume the kind of lights you’ll be dealing with. However at least point and directional lights can be dealt with in one way.  Default: iteration once

<a name="fur_005fexample"></a>

Example: Simple Fur shader material script that uses a second pass with 10 iterations to grow the fur:

```cpp
// GLSL simple Fur
vertex_program GLSLDemo/FurVS glsl 
{
  source fur.vert 
  default_params
  {
    param_named_auto lightPosition light_position_object_space 0
    param_named_auto eyePosition camera_position_object_space
    param_named_auto passNumber pass_number
    param_named_auto multiPassNumber pass_iteration_number
    param_named furLength float 0.15
  }
}

fragment_program GLSLDemo/FurFS glsl 
{
  source fur.frag 
  default_params
  {
    param_named Ka float 0.2
    param_named Kd float 0.5
    param_named Ks float 0.0
    param_named furTU int 0
  }
}

material Fur
{
  technique GLSL
  {
    pass base_coat
    {
      ambient 0.7 0.7 0.7
      diffuse 0.5 0.8 0.5
      specular 1.0 1.0 1.0 1.5

      vertex_program_ref GLSLDemo/FurVS
      {
      }

      fragment_program_ref GLSLDemo/FurFS 
      {
      }

      texture_unit
      {
        texture Fur.tga
        tex_coord_set 0
        filtering trilinear
      }
    }

    pass grow_fur
    {
      ambient 0.7 0.7 0.7
      diffuse 0.8 1.0 0.8
      specular 1.0 1.0 1.0 64
      depth_write off

      scene_blend src_alpha one
      iteration 10
      
      vertex_program_ref GLSLDemo/FurVS
      {
      }

      fragment_program_ref GLSLDemo/FurFS
      {
      }

      texture_unit
      {
        texture Fur.tga
        tex_coord_set 0
        filtering trilinear
      }
    }
  }
}
```

Note: use gpu program auto parameters [pass\_number](#pass_005fnumber) and [pass\_iteration\_number](#pass_005fiteration_005fnumber) to tell the vertex, geometry or fragment program the pass number and iteration number.

<a name="point_005fsize"></a><a name="point_005fsize-1"></a>

## point\_size

This setting allows you to change the size of points when rendering a point list, or a list of point sprites. The interpretation of this command depends on the [point\_size\_attenuation](#point_005fsize_005fattenuation) option - if it is off (the default), the point size is in screen pixels, if it is on, it expressed as normalised screen coordinates (1.0 is the height of the screen) when the point is at the origin. 

NOTE: Some drivers have an upper limit on the size of points they support - this can even vary between APIs on the same card! Don’t rely on point sizes that cause the points to get very large on screen, since they may get clamped on some cards. Upper sizes can range from 64 to 256 pixels.

Format: point\_size &lt;size&gt; Default: point\_size 1.0

<a name="point_005fsprites"></a><a name="point_005fsprites-1"></a>

## point\_sprites

This setting specifies whether or not hardware point sprite rendering is enabled for this pass. Enabling it means that a point list is rendered as a list of quads rather than a list of dots. It is very useful to use this option if you’re using a BillboardSet and only need to use point oriented billboards which are all of the same size. You can also use it for any other point list render. 

Format: point\_sprites &lt;on|off&gt; Default: point\_sprites off

<a name="point_005fsize_005fattenuation"></a><a name="point_005fsize_005fattenuation-1"></a>

## point\_size\_attenuation

Defines whether point size is attenuated with view space distance, and in what fashion. This option is especially useful when you’re using point sprites (See [point\_sprites](#point_005fsprites)) since it defines how they reduce in size as they get further away from the camera. You can also disable this option to make point sprites a constant screen size (like points), or enable it for points so they change size with distance.

You only have to provide the final 3 parameters if you turn attenuation on. The formula for attenuation is that the size of the point is multiplied by 1 / (constant + linear \* dist + quadratic \* d^2); therefore turning it off is equivalent to (constant = 1, linear = 0, quadratic = 0) and standard perspective attenuation is (constant = 0, linear = 1, quadratic = 0). The latter is assumed if you leave out the final 3 parameters when you specify ’on’.

Note that the resulting attenuated size is clamped to the minimum and maximum point size, see the next section.

Format: point\_size\_attenuation &lt;on|off&gt; \[constant linear quadratic\] Default: point\_size\_attenuation off

<a name="point_005fsize_005fmin"></a><a name="point_005fsize_005fmin-1"></a>

## point\_size\_min

Sets the minimum point size after attenuation ([point\_size\_attenuation](#point_005fsize_005fattenuation)). For details on the size metrics, See [point\_size](#point_005fsize).

Format: point\_size\_min &lt;size&gt; Default: point\_size\_min 0

<a name="point_005fsize_005fmax"></a><a name="point_005fsize_005fmax-1"></a>

## point\_size\_max

Sets the maximum point size after attenuation ([point\_size\_attenuation](#point_005fsize_005fattenuation)). For details on the size metrics, See [point\_size](#point_005fsize). A value of 0 means the maximum is set to the same as the max size reported by the current card. 

Format: point\_size\_max &lt;size&gt; Default: point\_size\_max 0


## Texture Units {#Texture-Units}

Here are the attributes you can use in a ’texture\_unit’ section of a .material script:

<a name="Available-Texture-Layer-Attributes"></a>

# Available Texture Layer Attributes

-   [texture\_alias](#texture_005falias)
-   [texture](#texture)
-   [anim\_texture](#anim_005ftexture)
-   [cubic\_texture](#cubic_005ftexture)
-   [tex\_coord\_set](#tex_005fcoord_005fset)
-   [tex\_address\_mode](#tex_005faddress_005fmode)
-   [tex\_border\_colour](#tex_005fborder_005fcolour)
-   [filtering](#filtering)
-   [max\_anisotropy](#max_005fanisotropy)
-   [mipmap\_bias](#mipmap_005fbias)
-   [colour\_op](#colour_005fop)
-   [colour\_op\_ex](#colour_005fop_005fex)
-   [colour\_op\_multipass\_fallback](#colour_005fop_005fmultipass_005ffallback)
-   [alpha\_op\_ex](#alpha_005fop_005fex)
-   [env\_map](#env_005fmap)
-   [scroll](#scroll)
-   [scroll\_anim](#scroll_005fanim)
-   [rotate](#rotate)
-   [rotate\_anim](#rotate_005fanim)
-   [scale](#scale)
-   [wave\_xform](#wave_005fxform)
-   [transform](#transform)
-   [binding\_type](#binding_005ftype)
-   [content\_type](#content_005ftype)

You can also use a nested ’texture\_source’ section in order to use a special add-in as a source of texture data, See [External Texture Sources](#External-Texture-Sources) for details.

<a name="Attribute-Descriptions-1"></a>

# Attribute Descriptions

<a name="texture_005falias"></a><a name="texture_005falias-1"></a>

## texture\_alias

Sets the alias name for this texture unit.

Format: texture\_alias &lt;name&gt;

Example: texture\_alias NormalMap

Setting the texture alias name is useful if this material is to be inherited by other other materials and only the textures will be changed in the new material.(See [Texture Aliases](#Texture-Aliases)) Default: If a texture\_unit has a name then the texture\_alias defaults to the texture\_unit name.

<a name="texture"></a><a name="texture-1"></a>

## texture

Sets the name of the static texture image this layer will use.

Format: texture &lt;texturename&gt; \[&lt;type&gt;\] \[unlimited | numMipMaps\] \[alpha\] \[&lt;PixelFormat&gt;\] \[gamma\]

Example: texture funkywall.jpg

This setting is mutually exclusive with the anim\_texture attribute. Note that the texture file cannot include spaces. Those of you Windows users who like spaces in filenames, please get over it and use underscores instead. The ’type’ parameter allows you to specify a the type of texture to create - the default is ’2d’, but you can override this; here’s the full list:

<dl compact="compact">
<dt>1d</dt> <dd>

A 1-dimensional texture; that is, a texture which is only 1 pixel high. These kinds of textures can be useful when you need to encode a function in a texture and use it as a simple lookup, perhaps in a fragment program. It is important that you use this setting when you use a fragment program which uses 1-dimensional texture coordinates, since GL requires you to use a texture type that matches (D3D will let you get away with it, but you ought to plan for cross-compatibility). Your texture widths should still be a power of 2 for best compatibility and performance.

</dd> <dt>2d</dt> <dd>

The default type which is assumed if you omit it, your texture has a width and a height, both of which should preferably be powers of 2, and if you can, make them square because this will look best on the most hardware. These can be addressed with 2D texture coordinates.

</dd> <dt>3d</dt> <dd>

A 3 dimensional texture i.e. volume texture. Your texture has a width, a height, both of which should be powers of 2, and has depth. These can be addressed with 3d texture coordinates i.e. through a pixel shader.

</dd> <dt>cubic</dt> <dd>

This texture is made up of 6 2D textures which are pasted around the inside of a cube. Alternatively 1 cube texture can be used if supported by the texture format(DDS for example) and rendersystem. Can be addressed with 3D texture coordinates and are useful for cubic reflection maps and normal maps.

</dd> </dl>

The ’numMipMaps’ option allows you to specify the number of mipmaps to generate for this texture. The default is ’unlimited’ which means mips down to 1x1 size are generated. You can specify a fixed number (even 0) if you like instead. Note that if you use the same texture in many material scripts, the number of mipmaps generated will conform to the number specified in the first texture\_unit used to load the texture - so be consistent with your usage.

The ’alpha’ option allows you to specify that a single channel (luminance) texture should be loaded as alpha, rather than the default which is to load it into the red channel. This can be helpful if you want to use alpha-only textures in the fixed function pipeline.

Default: none

The &lt;PixelFormat&gt; option allows you to specify the desired pixel format of the texture to create, which may be different to the pixel format of the texture file being loaded. Bear in mind that the final pixel format will be constrained by hardware capabilities so you may not get exactly what you ask for. The available options are:

<dl compact="compact">
<dt>PF\_L8</dt> <dd>

8-bit pixel format, all bits luminance.

</dd> <dt>PF\_L16</dt> <dd>

16-bit pixel format, all bits luminance.

</dd> <dt>PF\_A8</dt> <dd>

8-bit pixel format, all bits alpha.

</dd> <dt>PF\_A4L4</dt> <dd>

8-bit pixel format, 4 bits alpha, 4 bits luminance.

</dd> <dt>PF\_BYTE\_LA</dt> <dd>

2 byte pixel format, 1 byte luminance, 1 byte alpha

</dd> <dt>PF\_R5G6B5</dt> <dd>

16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.

</dd> <dt>PF\_B5G6R5</dt> <dd>

16-bit pixel format, 5 bits blue, 6 bits green, 5 bits red.

</dd> <dt>PF\_R3G3B2</dt> <dd>

8-bit pixel format, 3 bits red, 3 bits green, 2 bits blue.

</dd> <dt>PF\_A4R4G4B4</dt> <dd>

16-bit pixel format, 4 bits for alpha, red, green and blue.

</dd> <dt>PF\_A1R5G5B5</dt> <dd>

16-bit pixel format, 1 bit for alpha, 5 bits for red, green and blue.

</dd> <dt>PF\_R8G8B8</dt> <dd>

24-bit pixel format, 8 bits for red, green and blue.

</dd> <dt>PF\_B8G8R8</dt> <dd>

24-bit pixel format, 8 bits for blue, green and red.

</dd> <dt>PF\_A8R8G8B8</dt> <dd>

32-bit pixel format, 8 bits for alpha, red, green and blue.

</dd> <dt>PF\_A8B8G8R8</dt> <dd>

32-bit pixel format, 8 bits for alpha, blue, green and red.

</dd> <dt>PF\_B8G8R8A8</dt> <dd>

32-bit pixel format, 8 bits for blue, green, red and alpha.

</dd> <dt>PF\_R8G8B8A8</dt> <dd>

32-bit pixel format, 8 bits for red, green, blue and alpha.

</dd> <dt>PF\_X8R8G8B8</dt> <dd>

32-bit pixel format, 8 bits for red, 8 bits for green, 8 bits for blue like PF\_A8R8G8B8, but alpha will get discarded

</dd> <dt>PF\_X8B8G8R8</dt> <dd>

32-bit pixel format, 8 bits for blue, 8 bits for green, 8 bits for red like PF\_A8B8G8R8, but alpha will get discarded

</dd> <dt>PF\_A2R10G10B10</dt> <dd>

32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.

</dd> <dt>PF\_A2B10G10R10</dt> <dd>

32-bit pixel format, 2 bits for alpha, 10 bits for blue, green and red.

</dd> <dt>PF\_DXT1</dt> <dd>

DDS (DirectDraw Surface) DXT1 format

</dd> <dt>PF\_DXT2</dt> <dd>

DDS (DirectDraw Surface) DXT2 format

</dd> <dt>PF\_DXT3</dt> <dd>

DDS (DirectDraw Surface) DXT3 format

</dd> <dt>PF\_DXT4</dt> <dd>

DDS (DirectDraw Surface) DXT4 format

</dd> <dt>PF\_DXT5</dt> <dd>

DDS (DirectDraw Surface) DXT5 format

</dd> <dt>PF\_FLOAT16\_R</dt> <dd>

16-bit pixel format, 16 bits (float) for red

</dd> <dt>PF\_FLOAT16\_RGB</dt> <dd>

48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue

</dd> <dt>PF\_FLOAT16\_RGBA</dt> <dd>

64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha

</dd> <dt>PF\_FLOAT32\_R</dt> <dd>

16-bit pixel format, 16 bits (float) for red

</dd> <dt>PF\_FLOAT32\_RGB</dt> <dd>

96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue

</dd> <dt>PF\_FLOAT32\_RGBA</dt> <dd>

128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha

</dd> <dt>PF\_SHORT\_RGBA</dt> <dd>

64-bit pixel format, 16 bits for red, green, blue and alpha

</dd> <dt>PF\_FLOAT16\_GR</dt> <dd>

32-bit, 2-channel s10e5 floating point pixel format, 16-bit green, 16-bit red

</dd> <dt>PF\_FLOAT32\_GR</dt> <dd>

64-bit, 2-channel floating point pixel format, 32-bit green, 32-bit red

</dd> <dt>PF\_DEPTH</dt> <dd>

Depth texture format

</dd> <dt>PF\_SHORT\_GR</dt> <dd>

32-bit pixel format, 16-bit green, 16-bit red

</dd> <dt>PF\_SHORT\_RGB</dt> <dd>

48-bit pixel format, 16 bits for red, green and blue

</dd> <dt>PF\_PVRTC\_RGB2</dt> <dd>

PVRTC (PowerVR) RGB 2 bpp

</dd> <dt>PF\_PVRTC\_RGBA2</dt> <dd>

PVRTC (PowerVR) RGBA 2 bpp

</dd> <dt>PF\_PVRTC\_RGB4</dt> <dd>

PVRTC (PowerVR) RGB 4 bpp

</dd> <dt>PF\_PVRTC\_RGBA4</dt> <dd>

PVRTC (PowerVR) RGBA 4 bpp

</dd> <dt>PF\_R8</dt> <dd>

8-bit pixel format, all bits red.

</dd> <dt>PF\_RG8</dt> <dd>

16-bit pixel format, 8 bits red, 8 bits green.

</dd> </dl>

The ’gamma’ option informs the renderer that you want the graphics hardware to perform gamma correction on the texture values as they are sampled for rendering. This is only applicable for textures which have 8-bit colour channels (e.g.PF\_R8G8B8). Often, 8-bit per channel textures will be stored in gamma space in order to increase the precision of the darker colours (<http://en.wikipedia.org/wiki/Gamma_correction>) but this can throw out blending and filtering calculations since they assume linear space colour values. For the best quality shading, you may want to enable gamma correction so that the hardware converts the texture values to linear space for you automatically when sampling the texture, then the calculations in the pipeline can be done in a reliable linear colour space. When rendering to a final 8-bit per channel display, you’ll also want to convert back to gamma space which can be done in your shader (by raising to the power 1/2.2) or you can enable gamma correction on the texture being rendered to or the render window. Note that the ’gamma’ option on textures is applied on loading the texture so must be specified consistently if you use this texture in multiple places.

<a name="anim_005ftexture"></a><a name="anim_005ftexture-1"></a>

## anim\_texture

Sets the images to be used in an animated texture layer. In this case an animated texture layer means one which has multiple frames, each of which is a separate image file. There are 2 formats, one for implicitly determined image names, one for explicitly named images.

Format1 (short): anim\_texture &lt;base\_name&gt; &lt;num\_frames&gt; &lt;duration&gt;

Example: anim\_texture flame.jpg 5 2.5

This sets up an animated texture layer made up of 5 frames named flame\_0.jpg, flame\_1.jpg, flame\_2.jpg etc, with an animation length of 2.5 seconds (2fps). If duration is set to 0, then no automatic transition takes place and frames must be changed manually in code.

Format2 (long): anim\_texture &lt;frame1&gt; &lt;frame2&gt; ... &lt;duration&gt;

Example: anim\_texture flamestart.jpg flamemore.png flameagain.jpg moreflame.jpg lastflame.tga 2.5

This sets up the same duration animation but from 5 separately named image files. The first format is more concise, but the second is provided if you cannot make your images conform to the naming standard required for it. 

Default: none

<a name="cubic_005ftexture"></a><a name="cubic_005ftexture-1"></a>

## cubic\_texture

Sets the images used in a cubic texture, i.e. one made up of 6 individual images making up the faces of a cube or 1 cube texture if supported by the texture format(DDS for example) and rendersystem.. These kinds of textures are used for reflection maps (if hardware supports cubic reflection maps) or skyboxes. There are 2 formats, a brief format expecting image names of a particular format and a more flexible but longer format for arbitrarily named textures.

Format1 (short): cubic\_texture &lt;base\_name&gt; &lt;combinedUVW|separateUV&gt;

The base\_name in this format is something like ’skybox.jpg’, and the system will expect you to provide skybox\_fr.jpg, skybox\_bk.jpg, skybox\_up.jpg, skybox\_dn.jpg, skybox\_lf.jpg, and skybox\_rt.jpg for the individual faces.

Format2 (long): cubic\_texture &lt;front&gt; &lt;back&gt; &lt;left&gt; &lt;right&gt; &lt;up&gt; &lt;down&gt; separateUV

In this case each face is specified explicitly, incase you don’t want to conform to the image naming standards above. You can only use this for the separateUV version since the combinedUVW version requires a single texture name to be assigned to the combined 3D texture (see below).

In both cases the final parameter means the following:

<dl compact="compact">
<dt>combinedUVW</dt> <dd>

The 6 textures are combined into a single ’cubic’ texture map which is then addressed using 3D texture coordinates with U, V and W components. Necessary for reflection maps since you never know which face of the box you are going to need. Note that not all cards support cubic environment mapping.

</dd> <dt>separateUV</dt> <dd>

The 6 textures are kept separate but are all referenced by this single texture layer. One texture at a time is active (they are actually stored as 6 frames), and they are addressed using standard 2D UV coordinates. This type is good for skyboxes since only one face is rendered at one time and this has more guaranteed hardware support on older cards.

</dd> </dl> <br>

Default: none

<a name="binding_005ftype"></a><a name="binding_005ftype-1"></a>

## binding\_type

Tells this texture unit to bind to either the fragment processing unit or the vertex processing unit (for [Vertex Texture Fetch](#Vertex-Texture-Fetch)). 

Format: binding\_type &lt;vertex|fragment&gt; Default: binding\_type fragment

<a name="content_005ftype"></a><a name="content_005ftype-1"></a>

## content\_type

Tells this texture unit where it should get its content from. The default is to get texture content from a named texture, as defined with the [texture](#texture), [cubic\_texture](#cubic_005ftexture), [anim\_texture](#anim_005ftexture) attributes. However you can also pull texture information from other automated sources. The options are:

<dl compact="compact">
<dt>named</dt> <dd>

The default option, this derives texture content from a texture name, loaded by ordinary means from a file or having been manually created with a given name.

</dd> <dt>shadow</dt> <dd>

This option allows you to pull in a shadow texture, and is only valid when you use texture shadows and one of the ’custom sequence’ shadowing types (See [Shadows](#Shadows)). The shadow texture in question will be from the ’n’th closest light that casts shadows, unless you use light-based pass iteration or the light\_start option which may start the light index higher. When you use this option in multiple texture units within the same pass, each one references the next shadow texture. The shadow texture index is reset in the next pass, in case you want to take into account the same shadow textures again in another pass (e.g. a separate specular / gloss pass). By using this option, the correct light frustum projection is set up for you for use in fixed-function, if you use shaders just reference the texture\_viewproj\_matrix auto parameter in your shader.

</dd> <dt>compositor</dt> <dd>

This option allows you to reference a texture from a compositor, and is only valid when the pass is rendered within a compositor sequence. This can be either in a render\_scene directive inside a compositor script, or in a general pass in a viewport that has a compositor attached. Note that this is a reference only, meaning that it does not change the render order. You must make sure that the order is reasonable for what you are trying to achieve (for example, texture pooling might cause the referenced texture to be overwritten by something else by the time it is referenced).  The extra parameters for the content\_type are only required for this type:  The first is the name of the compositor being referenced. (Required)  The second is the name of the texture to reference in the compositor. (Required)  The third is the index of the texture to take, in case of an MRT. (Optional)

</dd> </dl>

Format: content\_type &lt;named|shadow|compositor&gt; \[&lt;Referenced Compositor Name&gt;\] \[&lt;Referenced Texture Name&gt;\] \[&lt;Referenced MRT Index&gt;\]  Default: content\_type named  Example: content\_type compositor DepthCompositor OutputTexture 

<a name="tex_005fcoord_005fset"></a><a name="tex_005fcoord_005fset-1"></a>

## tex\_coord\_set

Sets which texture coordinate set is to be used for this texture layer. A mesh can define multiple sets of texture coordinates, this sets which one this material uses.

**Note:** Only applies to the fixed-function pipeline, if you’re using a fragment program this will have no effect.

Format: tex\_coord\_set &lt;set\_num&gt;

Example: tex\_coord\_set 2

Default: tex\_coord\_set 0

<a name="tex_005faddress_005fmode"></a><a name="tex_005faddress_005fmode-1"></a>

## tex\_address\_mode

Defines what happens when texture coordinates exceed 1.0 for this texture layer.You can use the simple format to specify the addressing mode for all 3 potential texture coordinates at once, or you can use the 2/3 parameter extended format to specify a different mode per texture coordinate. 

Simple Format: tex\_address\_mode &lt;uvw\_mode&gt; <br> Extended Format: tex\_address\_mode &lt;u\_mode&gt; &lt;v\_mode&gt; \[&lt;w\_mode&gt;\]

<dl compact="compact">
<dt>wrap</dt> <dd>

Any value beyond 1.0 wraps back to 0.0. Texture is repeated.

</dd> <dt>clamp</dt> <dd>

Values beyond 1.0 are clamped to 1.0. Texture ’streaks’ beyond 1.0 since last line of pixels is used across the rest of the address space. Useful for textures which need exact coverage from 0.0 to 1.0 without the ’fuzzy edge’ wrap gives when combined with filtering.

</dd> <dt>mirror</dt> <dd>

Texture flips every boundary, meaning texture is mirrored every 1.0 u or v

</dd> <dt>border</dt> <dd>

Values outside the range \[0.0, 1.0\] are set to the border colour, you might also set the [tex\_border\_colour](#tex_005fborder_005fcolour) attribute too.

</dd> </dl> <br>

Default: tex\_address\_mode wrap

<a name="tex_005fborder_005fcolour"></a><a name="tex_005fborder_005fcolour-1"></a>

## tex\_border\_colour

Sets the border colour of border texture address mode (see [tex\_address\_mode](#tex_005faddress_005fmode)). 

Format: tex\_border\_colour &lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]<br> NB valid colour values are between 0.0 and 1.0.

Example: tex\_border\_colour 0.0 1.0 0.3

Default: tex\_border\_colour 0.0 0.0 0.0 1.0

<a name="filtering"></a><a name="filtering-1"></a>

## filtering

Sets the type of texture filtering used when magnifying or minifying a texture. There are 2 formats to this attribute, the simple format where you simply specify the name of a predefined set of filtering options, and the complex format, where you individually set the minification, magnification, and mip filters yourself. **Simple Format**<br> Format: filtering &lt;none|bilinear|trilinear|anisotropic&gt;<br> Default: filtering bilinear With this format, you only need to provide a single parameter which is one of the following:

<dl compact="compact">
<dt>none</dt> <dd>

No filtering or mipmapping is used. This is equivalent to the complex format ’filtering point point none’.

</dd> <dt>bilinear</dt> <dd>

2x2 box filtering is performed when magnifying or reducing a texture, and a mipmap is picked from the list but no filtering is done between the levels of the mipmaps. This is equivalent to the complex format ’filtering linear linear point’.

</dd> <dt>trilinear</dt> <dd>

2x2 box filtering is performed when magnifying and reducing a texture, and the closest 2 mipmaps are filtered together. This is equivalent to the complex format ’filtering linear linear linear’.

</dd> <dt>anisotropic</dt> <dd>

This is the same as ’trilinear’, except the filtering algorithm takes account of the slope of the triangle in relation to the camera rather than simply doing a 2x2 pixel filter in all cases. This makes triangles at acute angles look less fuzzy. Equivalent to the complex format ’filtering anisotropic anisotropic linear’. Note that in order for this to make any difference, you must also set the [max\_anisotropy](#max_005fanisotropy) attribute too.

</dd> </dl> 

**Complex Format**<br> Format: filtering &lt;minification&gt; &lt;magnification&gt; &lt;mip&gt;<br> Default: filtering linear linear point This format gives you complete control over the minification, magnification, and mip filters. Each parameter can be one of the following:

<dl compact="compact">
<dt>none</dt> <dd>

Nothing - only a valid option for the ’mip’ filter , since this turns mipmapping off completely. The lowest setting for min and mag is ’point’.

</dd> <dt>point</dt> <dd>

Pick the closet pixel in min or mag modes. In mip mode, this picks the closet matching mipmap.

</dd> <dt>linear</dt> <dd>

Filter a 2x2 box of pixels around the closest one. In the ’mip’ filter this enables filtering between mipmap levels.

</dd> <dt>anisotropic</dt> <dd>

Only valid for min and mag modes, makes the filter compensate for camera-space slope of the triangles. Note that in order for this to make any difference, you must also set the [max\_anisotropy](#max_005fanisotropy) attribute too.

</dd> </dl> <a name="max_005fanisotropy"></a><a name="max_005fanisotropy-1"></a>

## max\_anisotropy

Sets the maximum degree of anisotropy that the renderer will try to compensate for when filtering textures. The degree of anisotropy is the ratio between the height of the texture segment visible in a screen space region versus the width - so for example a floor plane, which stretches on into the distance and thus the vertical texture coordinates change much faster than the horizontal ones, has a higher anisotropy than a wall which is facing you head on (which has an anisotropy of 1 if your line of sight is perfectly perpendicular to it). You should set the max\_anisotropy value to something greater than 1 to begin compensating; higher values can compensate for more acute angles. The maximum value is determined by the hardware, but it is usually 8 or 16.  In order for this to be used, you have to set the minification and/or the magnification [filtering](#filtering) option on this texture to anisotropic.

Format: max\_anisotropy &lt;value&gt;<br> Default: max\_anisotropy 1

<a name="mipmap_005fbias"></a><a name="mipmap_005fbias-1"></a>

## mipmap\_bias

Sets the bias value applied to the mipmapping calculation, thus allowing you to alter the decision of which level of detail of the texture to use at any distance. The bias value is applied after the regular distance calculation, and adjusts the mipmap level by 1 level for each unit of bias. Negative bias values force larger mip levels to be used, positive bias values force smaller mip levels to be used. The bias is a floating point value so you can use values in between whole numbers for fine tuning. In order for this option to be used, your hardware has to support mipmap biasing (exposed through the render system capabilities), and your minification [filtering](#filtering) has to be set to point or linear.

Format: mipmap\_bias &lt;value&gt;<br> Default: mipmap\_bias 0

<a name="colour_005fop"></a><a name="colour_005fop-1"></a>

## colour\_op

Determines how the colour of this texture layer is combined with the one below it (or the lighting effect on the geometry if this is the first layer). **Note:** Only applies to the fixed-function pipeline, if you’re using a fragment program this will have no effect.

Format: colour\_op &lt;replace|add|modulate|alpha\_blend&gt;

This method is the simplest way to blend texture layers, because it requires only one parameter, gives you the most common blending types, and automatically sets up 2 blending methods: one for if single-pass multitexturing hardware is available, and another for if it is not and the blending must be achieved through multiple rendering passes. It is, however, quite limited and does not expose the more flexible multitexturing operations, simply because these can’t be automatically supported in multipass fallback mode. If want to use the fancier options, use [colour\_op\_ex](#colour_005fop_005fex), but you’ll either have to be sure that enough multitexturing units will be available, or you should explicitly set a fallback using [colour\_op\_multipass\_fallback](#colour_005fop_005fmultipass_005ffallback).<br>

<dl compact="compact">
<dt>replace</dt> <dd>

Replace all colour with texture with no adjustment.

</dd> <dt>add</dt> <dd>

Add colour components together.

</dd> <dt>modulate</dt> <dd>

Multiply colour components together.

</dd> <dt>alpha\_blend</dt> <dd>

Blend based on texture alpha.

</dd> </dl> <br>

Default: colour\_op modulate

<a name="colour_005fop_005fex"></a><a name="colour_005fop_005fex-1"></a>

## colour\_op\_ex

This is an extended version of the [colour\_op](#colour_005fop) attribute which allows extremely detailed control over the blending applied between this and earlier layers. Multitexturing hardware can apply more complex blending operations that multipass blending, but you are limited to the number of texture units which are available in hardware. **Note:** Only applies to the fixed-function pipeline, if you’re using a fragment program this will have no effect.

Format: colour\_op\_ex &lt;operation&gt; &lt;source1&gt; &lt;source2&gt; \[&lt;manual\_factor&gt;\] \[&lt;manual\_colour1&gt;\] \[&lt;manual\_colour2&gt;\]

Example colour\_op\_ex add\_signed src\_manual src\_current 0.5

See the IMPORTANT note below about the issues between multipass and multitexturing that using this method can create. Texture colour operations determine how the final colour of the surface appears when rendered. Texture units are used to combine colour values from various sources (e.g. the diffuse colour of the surface from lighting calculations, combined with the colour of the texture). This method allows you to specify the ’operation’ to be used, i.e. the calculation such as adds or multiplies, and which values to use as arguments, such as a fixed value or a value from a previous calculation.

<dl compact="compact">
<dt>Operation options</dt> <dd>

<dl compact="compact">
<dt>source1</dt> <dd>

Use source1 without modification

</dd> <dt>source2</dt> <dd>

Use source2 without modification

</dd> <dt>modulate</dt> <dd>

Multiply source1 and source2 together.

</dd> <dt>modulate\_x2</dt> <dd>

Multiply source1 and source2 together, then by 2 (brightening).

</dd> <dt>modulate\_x4</dt> <dd>

Multiply source1 and source2 together, then by 4 (brightening).

</dd> <dt>add</dt> <dd>

Add source1 and source2 together.

</dd> <dt>add\_signed</dt> <dd>

Add source1 and source2 then subtract 0.5.

</dd> <dt>add\_smooth</dt> <dd>

Add source1 and source2, subtract the product

</dd> <dt>subtract</dt> <dd>

Subtract source2 from source1

</dd> <dt>blend\_diffuse\_alpha</dt> <dd>

Use interpolated alpha value from vertices to scale source1, then add source2 scaled by (1-alpha).

</dd> <dt>blend\_texture\_alpha</dt> <dd>

As blend\_diffuse\_alpha but use alpha from texture

</dd> <dt>blend\_current\_alpha</dt> <dd>

As blend\_diffuse\_alpha but use current alpha from previous stages (same as blend\_diffuse\_alpha for first layer)

</dd> <dt>blend\_manual</dt> <dd>

As blend\_diffuse\_alpha but use a constant manual alpha value specified in &lt;manual&gt;

</dd> <dt>dotproduct</dt> <dd>

The dot product of source1 and source2

</dd> <dt>blend\_diffuse\_colour</dt> <dd>

Use interpolated colour value from vertices to scale source1, then add source2 scaled by (1-colour).

</dd> </dl> </dd> <dt>Source1 and source2 options</dt> <dd>

<dl compact="compact">
<dt>src\_current</dt> <dd>

The colour as built up from previous stages.

</dd> <dt>src\_texture</dt> <dd>

The colour derived from the texture assigned to this layer.

</dd> <dt>src\_diffuse</dt> <dd>

The interpolated diffuse colour from the vertices (same as ’src\_current’ for first layer).

</dd> <dt>src\_specular</dt> <dd>

The interpolated specular colour from the vertices.

</dd> <dt>src\_manual</dt> <dd>

The manual colour specified at the end of the command.

</dd> </dl> </dd> </dl> <br>

For example ’modulate’ takes the colour results of the previous layer, and multiplies them with the new texture being applied. Bear in mind that colours are RGB values from 0.0-1.0 so multiplying them together will result in values in the same range, ’tinted’ by the multiply. Note however that a straight multiply normally has the effect of darkening the textures - for this reason there are brightening operations like modulate\_x2. Note that because of the limitations on some underlying APIs (Direct3D included) the ’texture’ argument can only be used as the first argument, not the second. 

Note that the last parameter is only required if you decide to pass a value manually into the operation. Hence you only need to fill these in if you use the ’blend\_manual’ operation.

IMPORTANT: Ogre tries to use multitexturing hardware to blend texture layers together. However, if it runs out of texturing units (e.g. 2 of a GeForce2, 4 on a GeForce3) it has to fall back on multipass rendering, i.e. rendering the same object multiple times with different textures. This is both less efficient and there is a smaller range of blending operations which can be performed. For this reason, if you use this method you really should set the colour\_op\_multipass\_fallback attribute to specify which effect you want to fall back on if sufficient hardware is not available (the default is just ’modulate’ which is unlikely to be what you want if you’re doing swanky blending here). If you wish to avoid having to do this, use the simpler colour\_op attribute which allows less flexible blending options but sets up the multipass fallback automatically, since it only allows operations which have direct multipass equivalents.

Default: none (colour\_op modulate)<br>

<a name="colour_005fop_005fmultipass_005ffallback"></a><a name="colour_005fop_005fmultipass_005ffallback-1"></a>

## colour\_op\_multipass\_fallback

Sets the multipass fallback operation for this layer, if you used colour\_op\_ex and not enough multitexturing hardware is available.

Format: colour\_op\_multipass\_fallback &lt;src\_factor&gt; &lt;dest\_factor&gt;

Example: colour\_op\_multipass\_fallback one one\_minus\_dest\_alpha

Because some of the effects you can create using colour\_op\_ex are only supported under multitexturing hardware, if the hardware is lacking the system must fallback on multipass rendering, which unfortunately doesn’t support as many effects. This attribute is for you to specify the fallback operation which most suits you.

The parameters are the same as in the scene\_blend attribute; this is because multipass rendering IS effectively scene blending, since each layer is rendered on top of the last using the same mechanism as making an object transparent, it’s just being rendered in the same place repeatedly to get the multitexture effect. If you use the simpler (and less flexible) colour\_op attribute you don’t need to call this as the system sets up the fallback for you.

<a name="alpha_005fop_005fex"></a><a name="alpha_005fop_005fex-1"></a>

## alpha\_op\_ex

Behaves in exactly the same away as [colour\_op\_ex](#colour_005fop_005fex) except that it determines how alpha values are combined between texture layers rather than colour values.The only difference is that the 2 manual colours at the end of colour\_op\_ex are just single floating-point values in alpha\_op\_ex. **Note:** Only applies to the fixed-function pipeline, if you’re using a fragment program this will have no effect.

<a name="env_005fmap"></a><a name="env_005fmap-1"></a>

## env\_map

Turns on/off texture coordinate effect that makes this layer an environment map. **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect.

Format: env\_map &lt;off|spherical|planar|cubic\_reflection|cubic\_normal&gt;

Environment maps make an object look reflective by using automatic texture coordinate generation depending on the relationship between the objects vertices or normals and the eye.

<dl compact="compact">
<dt>spherical</dt> <dd>

A spherical environment map. Requires a single texture which is either a fish-eye lens view of the reflected scene, or some other texture which looks good as a spherical map (a texture of glossy highlights is popular especially in car sims). This effect is based on the relationship between the eye direction and the vertex normals of the object, so works best when there are a lot of gradually changing normals, i.e. curved objects.

</dd> <dt>planar</dt> <dd>

Similar to the spherical environment map, but the effect is based on the position of the vertices in the viewport rather than vertex normals. This effect is therefore useful for planar geometry (where a spherical env\_map would not look good because the normals are all the same) or objects without normals.

</dd> <dt>cubic\_reflection</dt> <dd>

A more advanced form of reflection mapping which uses a group of 6 textures making up the inside of a cube, each of which is a view if the scene down each axis. Works extremely well in all cases but has a higher technical requirement from the card than spherical mapping. Requires that you bind a [cubic\_texture](#cubic_005ftexture) to this texture unit and use the ’combinedUVW’ option.

</dd> <dt>cubic\_normal</dt> <dd>

Generates 3D texture coordinates containing the camera space normal vector from the normal information held in the vertex data. Again, full use of this feature requires a [cubic\_texture](#cubic_005ftexture) with the ’combinedUVW’ option.

</dd> </dl> <br>

Default: env\_map off<br>

<a name="scroll"></a><a name="scroll-1"></a>

## scroll

Sets a fixed scroll offset for the texture. **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: scroll &lt;x&gt; &lt;y&gt;

This method offsets the texture in this layer by a fixed amount. Useful for small adjustments without altering texture coordinates in models. However if you wish to have an animated scroll effect, see the [scroll\_anim](#scroll_005fanim) attribute.

<a name="scroll_005fanim"></a><a name="scroll_005fanim-1"></a>

## scroll\_anim

Sets up an animated scroll for the texture layer. Useful for creating fixed-speed scrolling effects on a texture layer (for varying scroll speeds, see [wave\_xform](#wave_005fxform)). **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: scroll\_anim &lt;xspeed&gt; &lt;yspeed&gt;<br>

<a name="rotate"></a><a name="rotate-1"></a>

## rotate

Rotates a texture to a fixed angle. This attribute changes the rotational orientation of a texture to a fixed angle, useful for fixed adjustments. If you wish to animate the rotation, see [rotate\_anim](#rotate_005fanim). **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: rotate &lt;angle&gt;

The parameter is a anti-clockwise angle in degrees.

**Note:** Only applies to the fixed-function pipeline, if you’re using a vertex shader this will have no effect unless you use the texture\_matrix auto-param.

<a name="rotate_005fanim"></a><a name="rotate_005fanim-1"></a>

## rotate\_anim

Sets up an animated rotation effect of this layer. Useful for creating fixed-speed rotation animations (for varying speeds, see [wave\_xform](#wave_005fxform)). **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: rotate\_anim &lt;revs\_per\_second&gt;

The parameter is a number of anti-clockwise revolutions per second.

<a name="scale"></a><a name="scale-1"></a>

## scale

Adjusts the scaling factor applied to this texture layer. Useful for adjusting the size of textures without making changes to geometry. This is a fixed scaling factor, if you wish to animate this see [wave\_xform](#wave_005fxform). **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: scale &lt;x\_scale&gt; &lt;y\_scale&gt;

Valid scale values are greater than 0, with a scale factor of 2 making the texture twice as big in that dimension etc.

<a name="wave_005fxform"></a><a name="wave_005fxform-1"></a>

## wave\_xform

Sets up a transformation animation based on a wave function. Useful for more advanced texture layer transform effects. You can add multiple instances of this attribute to a single texture layer if you wish. **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: wave\_xform &lt;xform\_type&gt; &lt;wave\_type&gt; &lt;base&gt; &lt;frequency&gt; &lt;phase&gt; &lt;amplitude&gt;

Example: wave\_xform scale\_x sine 1.0 0.2 0.0 5.0

<dl compact="compact">
<dt>xform\_type</dt> <dd>

<dl compact="compact">
<dt>scroll\_x</dt> <dd>

Animate the x scroll value

</dd> <dt>scroll\_y</dt> <dd>

Animate the y scroll value

</dd> <dt>rotate</dt> <dd>

Animate the rotate value

</dd> <dt>scale\_x</dt> <dd>

Animate the x scale value

</dd> <dt>scale\_y</dt> <dd>

Animate the y scale value

</dd> </dl> </dd> <dt>wave\_type</dt> <dd>

<dl compact="compact">
<dt>sine</dt> <dd>

A typical sine wave which smoothly loops between min and max values

</dd> <dt>triangle</dt> <dd>

An angled wave which increases & decreases at constant speed, changing instantly at the extremes

</dd> <dt>square</dt> <dd>

Max for half the wavelength, min for the rest with instant transition between

</dd> <dt>sawtooth</dt> <dd>

Gradual steady increase from min to max over the period with an instant return to min at the end.

</dd> <dt>inverse\_sawtooth</dt> <dd>

Gradual steady decrease from max to min over the period, with an instant return to max at the end.

</dd> </dl> </dd> <dt>base</dt> <dd>

The base value, the minimum if amplitude &gt; 0, the maximum if amplitude &lt; 0

</dd> <dt>frequency</dt> <dd>

The number of wave iterations per second, i.e. speed

</dd> <dt>phase</dt> <dd>

Offset of the wave start

</dd> <dt>amplitude</dt> <dd>

The size of the wave

</dd> </dl> <br>

The range of the output of the wave will be {base, base+amplitude}. So the example above scales the texture in the x direction between 1 (normal size) and 5 along a sine wave at one cycle every 5 second (0.2 waves per second).

<a name="transform"></a><a name="transform-1"></a>

## transform

This attribute allows you to specify a static 4x4 transformation matrix for the texture unit, thus replacing the individual scroll, rotate and scale attributes mentioned above.  **Note:** Only applies to the fixed-function pipeline, if you’re using a vertex program this will have no effect unless you use the texture\_matrix auto-param.

Format: transform m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33

The indexes of the 4x4 matrix value above are expressed as m&lt;row&gt;&lt;col&gt;.

## Declaring Vertex/Geometry/Fragment Programs {#Declaring-Vertex_002fGeometry_002fFragment-Programs}

In order to use a vertex, geometry or fragment program in your materials (See [Using Vertex/Geometry/Fragment Programs in a Pass](@ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass)), you first have to define them. A single program definition can be used by any number of materials, the only prerequisite is that a program must be defined before being referenced in the pass section of a material.

The definition of a program can either be embedded in the .material script itself (in which case it must precede any references to it in the script), or if you wish to use the same program across multiple .material files, you can define it in an external .program script. You define the program in exactly the same way whether you use a .program script or a .material script, the only difference is that all .program scripts are guaranteed to have been parsed before **all** .material scripts, so you can guarantee that your program has been defined before any .material script that might use it. Just like .material scripts, .program scripts will be read from any location which is on your resource path, and you can define many programs in a single script.

Vertex, geometry and fragment programs can be low-level (i.e. assembler code written to the specification of a given low level syntax such as vs\_1\_1 or arbfp1) or high-level such as DirectX9 HLSL, Open GL Shader Language, or nVidia’s Cg language (See @subpage High-level-Programs). High level languages give you a number of advantages, such as being able to write more intuitive code, and possibly being able to target multiple architectures in a single program (for example, the same Cg program might be able to be used in both D3D and GL, whilst the equivalent low-level programs would require separate techniques, each targeting a different API). High-level programs also allow you to use named parameters instead of simply indexed ones, although parameters are not defined here, they are used in the Pass.

Here is an example of a definition of a low-level vertex program:

```cpp
vertex_program myVertexProgram asm
{
    source myVertexProgram.asm 
    syntax vs_1_1
}
```

As you can see, that’s very simple, and defining a fragment or geometry program is exactly the same, just with vertex\_program replaced with fragment\_program or geometry\_program, respectively. You give the program a name in the header, followed by the word ’asm’ to indicate that this is a low-level program. Inside the braces, you specify where the source is going to come from (and this is loaded from any of the resource locations as with other media), and also indicate the syntax being used. You might wonder why the syntax specification is required when many of the assembler syntaxes have a header identifying them anyway - well the reason is that the engine needs to know what syntax the program is in before reading it, because during compilation of the material, we want to skip programs which use an unsupportable syntax quickly, without loading the program first.

The current supported syntaxes are:

<dl compact="compact">
<dt>vs\_1\_1</dt> <dd>

This is one of the DirectX vertex shader assembler syntaxes. <br> Supported on cards from: ATI Radeon 8500, nVidia GeForce 3 <br>

</dd> <dt>vs\_2\_0</dt> <dd>

Another one of the DirectX vertex shader assembler syntaxes. <br> Supported on cards from: ATI Radeon 9600, nVidia GeForce FX 5 series <br>

</dd> <dt>vs\_2\_x</dt> <dd>

Another one of the DirectX vertex shader assembler syntaxes. <br> Supported on cards from: ATI Radeon X series, nVidia GeForce FX 6 series <br>

</dd> <dt>vs\_3\_0</dt> <dd>

Another one of the DirectX vertex shader assembler syntaxes. <br> Supported on cards from: ATI Radeon HD 2000+, nVidia GeForce FX 6 series

</dd> <dt>arbvp1</dt> <dd>

This is the OpenGL standard assembler format for vertex programs. It’s roughly equivalent to DirectX vs\_1\_1.

</dd> <dt>vp20</dt> <dd>

This is an nVidia-specific OpenGL vertex shader syntax which is a superset of vs 1.1. ATI Radeon HD 2000+ also supports it.

</dd> <dt>vp30</dt> <dd>

Another nVidia-specific OpenGL vertex shader syntax. It is a superset of vs 2.0, which is supported on nVidia GeForce FX 5 series and higher. ATI Radeon HD 2000+ also supports it.

</dd> <dt>vp40</dt> <dd>

Another nVidia-specific OpenGL vertex shader syntax. It is a superset of vs 3.0, which is supported on nVidia GeForce FX 6 series and higher.

</dd> <dt>ps\_1\_1, ps\_1\_2, ps\_1\_3</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. <br> Supported on cards from: ATI Radeon 8500, nVidia GeForce 3 <br> NOTE: for ATI 8500, 9000, 9100, 9200 hardware, this profile can also be used in OpenGL. The ATI 8500 to 9200 do not support arbfp1 but do support atifs extension in OpenGL which is very similar in function to ps\_1\_4 in DirectX. Ogre has a built in ps\_1\_x to atifs compiler that is automatically invoked when ps\_1\_x is used in OpenGL on ATI hardware.

</dd> <dt>ps\_1\_4</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. <br> Supported on cards from: ATI Radeon 8500, nVidia GeForce FX 5 series <br> NOTE: for ATI 8500, 9000, 9100, 9200 hardware, this profile can also be used in OpenGL. The ATI 8500 to 9200 do not support arbfp1 but do support atifs extension in OpenGL which is very similar in function to ps\_1\_4 in DirectX. Ogre has a built in ps\_1\_x to atifs compiler that is automatically invoked when ps\_1\_x is used in OpenGL on ATI hardware.

</dd> <dt>ps\_2\_0</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. <br> Supported cards: ATI Radeon 9600, nVidia GeForce FX 5 series<br>

</dd> <dt>ps\_2\_x</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. This is basically ps\_2\_0 with a higher number of instructions. <br> Supported cards: ATI Radeon X series, nVidia GeForce FX 6 series<br>

</dd> <dt>ps\_3\_0</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. <br> Supported cards: ATI Radeon HD 2000+, nVidia GeForce FX6 series<br>

</dd> <dt>ps\_3\_x</dt> <dd>

DirectX pixel shader (i.e. fragment program) assembler syntax. <br> Supported cards: nVidia GeForce FX7 series<br>

</dd> <dt>arbfp1</dt> <dd>

This is the OpenGL standard assembler format for fragment programs. It’s roughly equivalent to ps\_2\_0, which means that not all cards that support basic pixel shaders under DirectX support arbfp1 (for example neither the GeForce3 or GeForce4 support arbfp1, but they do support ps\_1\_1).

</dd> <dt>fp20</dt> <dd>

This is an nVidia-specific OpenGL fragment syntax which is a superset of ps 1.3. It allows you to use the ’nvparse’ format for basic fragment programs. It actually uses NV\_texture\_shader and NV\_register\_combiners to provide functionality equivalent to DirectX’s ps\_1\_1 under GL, but only for nVidia cards. However, since ATI cards adopted arbfp1 a little earlier than nVidia, it is mainly nVidia cards like the GeForce3 and GeForce4 that this will be useful for. You can find more information about nvparse at http://developer.nvidia.com/object/nvparse.html.

</dd> <dt>fp30</dt> <dd>

Another nVidia-specific OpenGL fragment shader syntax. It is a superset of ps 2.0, which is supported on nVidia GeForce FX 5 series and higher. ATI Radeon HD 2000+ also supports it.

</dd> <dt>fp40</dt> <dd>

Another nVidia-specific OpenGL fragment shader syntax. It is a superset of ps 3.0, which is supported on nVidia GeForce FX 6 series and higher.

</dd> <dt>gpu\_gp, gp4\_gp</dt> <dd>

An nVidia-specific OpenGL geometry shader syntax. <br> Supported cards: nVidia GeForce FX8 series<br>

</dd> <dt>glsles</dt> <dd>

OpenGL Shading Language for Embedded Systems. It is a variant of GLSL, streamlined for low power devices. Supported cards: PowerVR SGX series

</dd> </dl>

You can get a definitive list of the syntaxes supported by the current card by calling GpuProgramManager::getSingleton().getSupportedSyntax().

## Specifying Named Constants for Assembler Shaders {#Specifying-Named-Constants-for-Assembler-Shaders}

Assembler shaders don’t have named constants (also called uniform parameters) because the language does not support them - however if you for example decided to precompile your shaders from a high-level language down to assembler for performance or obscurity, you might still want to use the named parameters. Well, you actually can - GpuNamedConstants which contains the named parameter mappings has a ’save’ method which you can use to write this data to disk, where you can reference it later using the manual\_named\_constants directive inside your assembler program declaration, e.g.

```cpp
vertex_program myVertexProgram asm
{
    source myVertexProgram.asm 
    syntax vs_1_1
    manual_named_constants myVertexProgram.constants
}
```

In this case myVertexProgram.constants has been created by calling highLevelGpuProgram-&gt;getNamedConstants().save("myVertexProgram.constants"); sometime earlier as preparation, from the original high-level program. Once you’ve used this directive, you can use named parameters here even though the assembler program itself has no knowledge of them.

## Default Program Parameters {#Default-Program-Parameters}

While defining a vertex, geometry or fragment program, you can also specify the default parameters to be used for materials which use it, unless they specifically override them. You do this by including a nested ’default\_params’ section, like so:

```cpp
vertex_program Ogre/CelShadingVP cg
{
    source Example_CelShading.cg
    entry_point main_vp
    profiles vs_1_1 arbvp1

    default_params
    {
        param_named_auto lightPosition light_position_object_space 0
        param_named_auto eyePosition camera_position_object_space
        param_named_auto worldViewProj worldviewproj_matrix
        param_named shininess float 10 
    }
}
```

The syntax of the parameter definition is exactly the same as when you define parameters when using programs, See [Program Parameter Specification](#Program-Parameter-Specification). Defining default parameters allows you to avoid rebinding common parameters repeatedly (clearly in the above example, all but ’shininess’ are unlikely to change between uses of the program) which makes your material declarations shorter.

## Declaring Shared Parameters {#Declaring-Shared-Parameters}

Often, not every parameter you want to pass to a shader is unique to that program, and perhaps you want to give the same value to a number of different programs, and a number of different materials using that program. Shared parameter sets allow you to define a ’holding area’ for shared parameters that can then be referenced when you need them in particular shaders, while keeping the definition of that value in one place. To define a set of shared parameters, you do this:

```cpp
shared_params YourSharedParamsName
{
    shared_param_named mySharedParam1 float4 0.1 0.2 0.3 0.4
    ...
}
```

As you can see, you need to use the keyword ’shared\_params’ and follow it with the name that you will use to identify these shared parameters. Inside the curly braces, you can define one parameter per line, in a way which is very similar to the [param\_named](#param_005fnamed) syntax. The definition of these lines is: Format: shared\_param\_name &lt;param\_name&gt; &lt;param\_type&gt; \[&lt;\[array\_size\]&gt;\] \[&lt;initial\_values&gt;\]

The param\_name must be unique within the set, and the param\_type can be any one of float, float2, float3, float4, int, int2, int3, int4, matrix2x2, matrix2x3, matrix2x4, matrix3x2, matrix3x3, matrix3x4, matrix4x2, matrix4x3 and matrix4x4. The array\_size option allows you to define arrays of param\_type should you wish, and if present must be a number enclosed in square brackets (and note, must be separated from the param\_type with whitespace). If you wish, you can also initialise the parameters by providing a list of values.

Once you have defined the shared parameters, you can reference them inside default\_params and params blocks using [shared\_params\_ref](#shared_005fparams_005fref). You can also obtain a reference to them in your code via GpuProgramManager::getSharedParameters, and update the values for all instances using them.

## Script Inheritance {#Script-Inheritance}

When creating new script objects that are only slight variations of another object, it’s good to avoid copying and pasting between scripts. Script inheritance lets you do this; in this section we’ll use material scripts as an example, but this applies to all scripts parsed with the script compilers in Ogre 1.6 onwards.

For example, to make a new material that is based on one previously defined, add a *colon* **:** after the new material name followed by the name of the material that is to be copied.

Format: material &lt;NewUniqueChildName&gt; : &lt;ReferenceParentMaterial&gt;

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed. The easiest way to achieve this is to either place parents at the beginning of the material script file, or to use the ’import’ directive (See [Script Import Directive](#Script-Import-Directive)). Note that inheritance is actually a copy - after scripts are loaded into Ogre, objects no longer maintain their copy inheritance structure. If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier. Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them, Techniques and passes can take a name and texture units can be numbered within the material script. You can also use variables, See [Script Variables](#Script-Variables).

Names become very useful in materials that copy from other materials. In order to override values they must be in the correct technique, pass, texture unit etc. The script could be lain out using the sequence of techniques, passes, texture units in the child material but if only one parameter needs to change in say the 5th pass then the first four passes prior to the fifth would have to be placed in the script:

Here is an example:

```cpp
material test2 : test1
{
  technique
  {
    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
      ambient 0.5 0.7 0.3 1.0
    }
  }
}
```

This method is tedious for materials that only have slight variations to their parent. An easier way is to name the pass directly without listing the previous passes:<br>

```cpp
material test2 : test1
{
  technique 0
  {
    pass 4
    {
      ambient 0.5 0.7 0.3 1.0
    }
  }
}
```

The parent pass name must be known and the pass must be in the correct technique in order for this to work correctly. Specifying the technique name and the pass name is the best method. If the parent technique/pass are not named then use their index values for their name as done in the example.

## Adding new Techniques, Passes, to copied materials {#Adding-new-Techniques_002c-Passes_002c-to-copied-materials_003a}

If a new technique or pass needs to be added to a copied material then use a unique name for the technique or pass that does not exist in the parent material. Using an index for the name that is one greater than the last index in the parent will do the same thing. The new technique/pass will be added to the end of the techniques/passes copied from the parent material.

Note: if passes or techniques aren’t given a name, they will take on a default name based on their index. For example the first pass has index 0 so its name will be 0.

## Identifying Texture Units to override values {#Identifying-Texture-Units-to-override-values}

A specific texture unit state (TUS) can be given a unique name within a pass of a material so that it can be identified later in cloned materials that need to override specified texture unit states in the pass without declaring previous texture units. Using a unique name for a Texture unit in a pass of a cloned material adds a new texture unit at the end of the texture unit list for the pass.

```cpp
material BumpMap2 : BumpMap1
{
  technique ati8500
  {
    pass 0
    {
      texture_unit NormalMap
      {
        texture BumpyMetalNM.png
      }
    }
  }
}
```

## Advanced Script Inheritance {#Advanced-Script-Inheritance}

Starting with Ogre 1.6, script objects can now inherit from each other more generally. The previous concept of inheritance, material copying, was restricted only to the top-level material objects. Now, any level of object can take advantage of inheritance (for instance, techniques, passes, and compositor targets).

```cpp
material Test
{
    technique
    {
        pass : ParentPass
        {
        }
    }
}
```

Notice that the pass inherits from ParentPass. This allows for the creation of more fine-grained inheritance hierarchies.

Along with the more generalized inheritance system comes an important new keyword: "abstract." This keyword is used at a top-level object declaration (not inside any other object) to denote that it is not something that the compiler should actually attempt to compile, but rather that it is only for the purpose of inheritance. For example, a material declared with the abstract keyword will never be turned into an actual usable material in the material framework. Objects which cannot be at a top-level in the document (like a pass) but that you would like to declare as such for inheriting purpose must be declared with the abstract keyword.

```cpp
abstract pass ParentPass
{
    diffuse 1 0 0 1
}
```

That declares the ParentPass object which was inherited from in the above example. Notice the abstract keyword which informs the compiler that it should not attempt to actually turn this object into any sort of Ogre resource. If it did attempt to do so, then it would obviously fail, since a pass all on its own like that is not valid.

The final matching option is based on wildcards. Using the ’\*’ character, you can make a powerful matching scheme and override multiple objects at once, even if you don’t know exact names or positions of those objects in the inherited object.

```cpp
abstract technique Overrider
{
   pass *colour*
   {
      diffuse 0 0 0 0
   }
}
```

This technique, when included in a material, will override all passes matching the wildcard "\*color\*" (color has to appear in the name somewhere) and turn their diffuse properties black. It does not matter their position or exact name in the inherited technique, this will match them. 

## Texture Aliases {#Texture-Aliases}

Texture aliases are useful for when only the textures used in texture units need to be specified for a cloned material. In the source material i.e. the original material to be cloned, each texture unit can be given a texture alias name. The cloned material in the script can then specify what textures should be used for each texture alias. Note that texture aliases are a more specific version of [Script Variables](#Script-Variables) which can be used to easily set other values.

Using texture aliases within texture units:<br> Format:<br> texture\_alias &lt;name&gt;

Default: &lt;name&gt; will default to texture\_unit &lt;name&gt; if set

```cpp
texture_unit DiffuseTex
{
  texture diffuse.jpg
}
```

texture\_alias defaults to DiffuseTex.<br>

Example: The base material to be cloned:<br>

```cpp
material TSNormalSpecMapping
{
  technique GLSL
  {
    pass 
    {
      ambient 0.1 0.1 0.1
      diffuse 0.7 0.7 0.7
      specular 0.7 0.7 0.7 128
        
      vertex_program_ref GLSLDemo/OffsetMappingVS
      {
        param_named_auto lightPosition light_position_object_space 0
        param_named_auto eyePosition camera_position_object_space
        param_named textureScale float 1.0
      }

      fragment_program_ref GLSLDemo/TSNormalSpecMappingFS 
      {
        param_named normalMap int 0
        param_named diffuseMap int 1
        param_named fxMap int 2
      }

      // Normal map
      texture_unit NormalMap
      {
        texture defaultNM.png
        tex_coord_set 0
        filtering trilinear
      }

      // Base diffuse texture map
      texture_unit DiffuseMap
      {
        texture defaultDiff.png
        filtering trilinear
        tex_coord_set 1
      }

      // spec map for shininess
      texture_unit SpecMap
      {
        texture defaultSpec.png
        filtering trilinear
        tex_coord_set 2
      }

    }
  }

  technique HLSL_DX9
  {
    pass 
    {
            
      vertex_program_ref FxMap_HLSL_VS
      {
        param_named_auto worldViewProj_matrix worldviewproj_matrix 
        param_named_auto lightPosition light_position_object_space 0
        param_named_auto eyePosition camera_position_object_space
      }

      fragment_program_ref FxMap_HLSL_PS 
      {
        param_named ambientColor float4 0.2 0.2 0.2 0.2
      }

      // Normal map
      texture_unit 
      {
        texture_alias NormalMap
        texture defaultNM.png
        tex_coord_set 0
        filtering trilinear
      }

      // Base diffuse texture map
      texture_unit 
      {
        texture_alias DiffuseMap
        texture defaultDiff.png
        filtering trilinear
        tex_coord_set 1
      }

      // spec map for shininess
      texture_unit
      {
        texture_alias SpecMap
        texture defaultSpec.png
        filtering trilinear
        tex_coord_set 2
      }

    }
  }
}
```

Note that the GLSL and HLSL techniques use the same textures. For each texture usage type a texture alias is given that describes what the texture is used for. So the first texture unit in the GLSL technique has the same alias as the TUS in the HLSL technique since its the same texture used. Same goes for the second and third texture units.<br> For demonstration purposes, the GLSL technique makes use of texture\_unit naming and therefore the texture\_alias name does not have to be set since it defaults to the texture unit name. So why not use the default all the time since its less typing? For most situations you can. Its when you clone a material that and then want to change the alias that you must use the texture\_alias command in the script. You cannot change the name of a texture\_unit in a cloned material so texture\_alias provides a facility to assign an alias name.

Now we want to clone the material but only want to change the textures used. We could copy and paste the whole material but if we decide to change the base material later then we also have to update the copied material in the script. With set\_texture\_alias, copying a material is very easy now. set\_texture\_alias is specified at the top of the material definition. All techniques using the specified texture alias will be effected by set\_texture\_alias.

Format:<br> set\_texture\_alias &lt;alias name&gt; &lt;texture name&gt;<br>

```cpp
material fxTest : TSNormalSpecMapping
{
  set_texture_alias NormalMap fxTestNMap.png
  set_texture_alias DiffuseMap fxTestDiff.png
  set_texture_alias SpecMap fxTestMap.png
}
```

The textures in both techniques in the child material will automatically get replaced with the new ones we want to use.

The same process can be done in code as long you set up the texture alias names so then there is no need to traverse technique/pass/TUS to change a texture. You just call myMaterialPtr-&gt;applyTextureAliases(myAliasTextureNameList) which will update all textures in all texture units that match the alias names in the map container reference you passed as a parameter.

You don’t have to supply all the textures in the copied material.<br>

```cpp
material fxTest2 : fxTest
{
  set_texture_alias DiffuseMap fxTest2Diff.png
  set_texture_alias SpecMap fxTest2Map.png
}
```

Material fxTest2 only changes the diffuse and spec maps of material fxTest and uses the same normal map.

Another example:

```cpp
material fxTest3 : TSNormalSpecMapping
{
  set_texture_alias DiffuseMap fxTest2Diff.png
}
```

fxTest3 will end up with the default textures for the normal map and spec map setup in TSNormalSpecMapping material but will have a different diffuse map. So your base material can define the default textures to use and then the child materials can override specific textures.

## Script Variables {#Script-Variables}

A very powerful new feature in Ogre 1.6 is variables. Variables allow you to parameterize data in materials so that they can become more generalized. This enables greater reuse of scripts by targeting specific customization points. Using variables along with inheritance allows for huge amounts of overrides and easy object reuse.

```cpp
abstract pass ParentPass
{
   diffuse $diffuse_colour
}

material Test
{
   technique
   {
       pass : ParentPass
       {
           set $diffuse_colour "1 0 0 1"
       }
   }
}
```

The ParentPass object declares a variable called "diffuse\_colour" which is then overridden in the Test material’s pass. The "set" keyword is used to set the value of that variable. The variable assignment follows lexical scoping rules, which means that the value of "1 0 0 1" is only valid inside that pass definition. Variable assignment in outer scopes carry over into inner scopes.

```cpp
material Test
{
    set $diffuse_colour "1 0 0 1"
    technique
    {
        pass : ParentPass
        {
        }
    }
}
```

The $diffuse\_colour assignment carries down through the technique and into the pass. 

## Script Import Directive {#Script-Import-Directive}

Imports are a feature introduced to remove ambiguity from script dependencies. When using scripts that inherit from each other but which are defined in separate files sometimes errors occur because the scripts are loaded in incorrect order. Using imports removes this issue. The script which is inheriting another can explicitly import its parent’s definition which will ensure that no errors occur because the parent’s definition was not found.

```cpp
import * from "parent.material"
material Child : Parent
{
}
```

The material "Parent" is defined in parent.material and the import ensures that those definitions are found properly. You can also import specific targets from within a file.

```cpp
import Parent from "parent.material"
```

If there were other definitions in the parent.material file, they would not be imported.

Note, however that importing does not actually cause objects in the imported script to be fully parsed & created, it just makes the definitions available for inheritance. This has a specific ramification for vertex / fragment program definitions, which must be loaded before any parameters can be specified. You should continue to put common program definitions in .program files to ensure they are fully parsed before being referenced in multiple .material files. The ’import’ command just makes sure you can resolve dependencies between equivalent script definitions (e.g. material to material).
