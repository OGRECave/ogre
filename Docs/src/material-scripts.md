# Material Scripts {#Material-Scripts}

Material scripts offer you the ability to define complex materials in a script which can be reused easily. Whilst you could set up all materials for a scene in code using the methods of the Material and TextureLayer classes, in practice it's a bit unwieldy. Instead you can store material definitions in text files which can then be loaded whenever required.

@tableofcontents

It’s important to realise that materials are not loaded completely by the parsing process: only the definition is loaded, no textures or other resources are loaded. This is because it is common to have a large library of materials, but only use a relatively small subset of them in any one scene. To load every material completely in every script would therefore cause unnecessary memory overhead. You can access a ’deferred load’ Material in the normal way (Ogre::MaterialManager::getSingleton().getByName()), but you must call the ’load’ method before trying to use it. Ogre does this for you when using the normal material assignment methods of entities etc.

To start with, we only consider fixed-function materials which don’t use vertex, geometry or fragment programs, these are covered later:

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

            // Additional RT Shader system options
            rtshader_system
            {
                // Do lighting calculations per-pixel 
                lighting_stage per_pixel
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

A material can be made up of many @ref Techniques - a technique is one way of achieving the effect you are looking for. You can supply more than one technique in order to provide fallback approaches where a card does not have the ability to render the preferred technique, or where you wish to define lower level of detail versions of the material in order to conserve rendering power when objects are more distant. 

Each technique can be made up of many @ref Passes, that is a complete render of the object can be performed multiple times with different settings in order to produce composite effects. Ogre may also split the passes you have defined into many passes at runtime, if you define a pass which uses too many texture units for the card you are currently running on (note that it can only do this if you are not using a fragment program). Each pass has a number of top-level attributes such as ’ambient’ to set the amount & colour of the ambient light reflected by the material. Some of these options do not apply if you are using vertex programs, See @ref Passes for more details. 

Within each pass, there can be zero or many @ref Texture-Units in use. These define the texture to be used, and optionally some blending operations (which use multitexturing) and texture effects.

You can also reference vertex and fragment programs (or vertex and pixel shaders, if you want to use that terminology) in a pass with a given set of parameters. Programs themselves are declared in separate .program scripts (See [GPU Program Scripts](@ref Declaring-Vertex_002fGeometry_002fFragment-Programs)) and are used as described in @ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass.

<a name="Top_002dlevel-material-attributes"></a>

# Material {#Material}

The outermost section of a material definition does not have a lot of attributes of its own (most of the configurable parameters are within the child sections. However, it does have some, and here they are: 

<a name="lod_005fstrategy"></a>
<a name="lod_005fstrategy-1"></a>

## lod\_strategy

Sets the name of the LOD strategy to be used.

@par
Format: lod\_strategy &lt;name&gt;
@par
Default: lod\_strategy distance_sphere

@par Valid values are:
- @c distance_sphere which means LOD changes based on distance from the camera (calculated via the bounding sphere radius).
- @c distance_box behaves the same as 'distance_sphere' except that it uses the object’s bounding box to approximate the distance.
- @c pixel_count changes LOD levels based on an absolute estimate of the screen-space pixels occupied (internally approximated via the bounding radius).
- @c screen_ratio_pixel_count sets that absolute screen space value in relation to the screen size (1.0 = object covering complete screen, 0.5 = half screen covered by object, etc.).

<a name="lod_005fvalues"></a>
<a name="lod_005fvalues-1"></a>

## lod\_values

This attribute defines the values used to control the LOD transition for this material. By setting this attribute, you indicate that you want this material to alter the Technique that it uses based on some metric, such as the distance from the camera, or the approximate screen space coverage. The exact meaning of these values is determined by the option you select for [lod\_strategy](#lod_005fstrategy) - it is a list of distances for the @c distance_sphere strategy, and a list of pixel counts for the @c pixel_count strategy, for example. You must give it a list of values, in order from highest LOD value to lowest LOD value, each one indicating the point at which the material will switch to the next LOD. All materials automatically activate LOD index 0 for values less than the first entry, so you don't have to explicitly specify this. Additionally, if there is no technique that matches the active LOD index, a technique with a lower LOD index will be used instead. Therefore, it is important to always have at least one technique with LOD index 0.

@par
Format: lod\_values &lt;value0&gt; &lt;value1&gt; &lt;value2&gt; ...
@par
Default: none
@par Example:
lod\_strategy distance_sphere
@par
lod\_values 300.0 600.5 1200

The above example would cause the material to use the best Technique at lod\_index 0 up to a distance of 300 world units, the best from lod\_index 1 from 300 up to 600, lod\_index 2 from 600 to 1200, and lod\_index 3 from 1200 upwards.

<a name="lod_005fdistances"></a>
## lod\_distances

@deprecated This option is deprecated in favour of [lod\_values](#lod_005fvalues) now.  

<a name="receive_005fshadows"></a><a name="receive_005fshadows-1"></a>

## receive\_shadows

This attribute controls whether objects using this material can have shadows cast upon them.

@par
Format: receive\_shadows &lt;on|off&gt;<br> Default: on

Whether or not an object receives a shadow is the combination of a number of factors, See @ref Shadows for full details; however this allows you to make a material opt-out of receiving shadows if required. Note that transparent materials never receive shadows so this option only has an effect on solid materials.

<a name="transparency_005fcasts_005fshadows"></a><a name="transparency_005fcasts_005fshadows-1"></a>

## transparency\_casts\_shadows

This attribute controls whether transparent materials can cast certain kinds of shadow.

@par
Format: transparency\_casts\_shadows &lt;on|off&gt;<br> 
Default: off 

Whether or not an object casts a shadow is the combination of a number of factors, See @ref Shadows for full details; however this allows you to make a transparent material cast shadows, when it would otherwise not. For example, when using texture shadows, transparent materials are normally not rendered into the shadow texture because they should not block light. This flag overrides that.

<a name="set_005ftexture_005falias"></a><a name="set_005ftexture_005falias-1"></a>

## set\_texture\_alias

This attribute associates a texture alias with a texture name.

@par
Format: set\_texture\_alias &lt;alias name&gt; &lt;texture name&gt;

@deprecated texture aliases are a restricted version of @ref Script-Variables, which you should instead.

# Techniques {#Techniques}

A "technique" section in your material script encapsulates a single method of rendering an object. The simplest of material definitions only contains a single technique, however since PC hardware varies quite greatly in it’s capabilities, you can only do this if you are sure that every card for which you intend to target your application will support the capabilities which your technique requires. In addition, it can be useful to define simpler ways to render a material if you wish to use material LOD, such that more distant objects use a simpler, less performance-hungry technique.

When a material is used for the first time, it is ’compiled’. That involves scanning the techniques which have been defined, and marking which of them are supportable using the current rendering API and graphics card. If no techniques are supportable, your material will render as blank white. The compilation examines a number of things, such as:

-   The number of texture\_unit entries in each pass<br> Note that if the number of texture\_unit entries exceeds the number of texture units in the current graphics card, the technique may still be supportable so long as a fragment program is not being used. In this case, Ogre will split the pass which has too many entries into multiple passes for the less capable card, and the multitexture blend will be turned into a multipass blend (See [colour\_op\_multipass\_fallback](#colour_005fop_005fmultipass_005ffallback)).
-   Whether vertex, geometry or fragment programs are used, and if so which syntax they use (e.g. vs\_1\_1, ps\_2\_x, arbfp1 etc.)
-   Other effects like cube mapping and dot3 blending
-   Whether the vendor or device name of the current graphics card matches some user-specified rules

<br>

In a material script, techniques must be listed in order of preference, i.e. the earlier techniques are preferred over the later techniques. This normally means you will list your most advanced, most demanding techniques first in the script, and list fallbacks afterwards.

To help clearly identify what each technique is used for, the technique can be named but its optional. Techniques not named within the script will take on a name that is the technique index number. For example: the first technique in a material is index 0, its name would be "0" if it was not given a name in the script. The technique name must be unique within the material or else the final technique is the resulting merge of all techniques with the same name in the material. A warning message is posted in the Ogre.log if this occurs. Named techniques can help when inheriting a material and modifying an existing technique: (See @ref Script-Inheritance)

@par
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

@par
Format: scheme &lt;name&gt;<br> Example: scheme hdr<br> Default: scheme Default

<a name="lod_005findex"></a><a name="lod_005findex-1"></a>

## lod\_index

Sets the level-of-detail (LOD) index this Technique belongs to. 

@par
Format: lod\_index &lt;number&gt;<br> NB Valid values are 0 (highest level of detail) to 65535, although this is unlikely. You should not leave gaps in the LOD indexes between Techniques.

@par
Example: lod\_index 1

All techniques are automatically assigned to a LOD index, with the default being index 0, which corresponds to the highest LOD. Increasing indexes denote lower levels of detail. You can (and often will) assign more than one technique to the same LOD index, what this means is that OGRE will pick the best technique of the ones listed at the same LOD index. For readability, it is advised that you list your techniques in order of LOD, then in order of preference, although the latter is the only prerequisite (OGRE determines which one is ’best’ by which one is listed first). You must always have at least one Technique at lod\_index 0. The distance at which a LOD level is applied is determined by the [lod_values](#lod_005fvalues) attribute of the containing material.

@par
Default: lod\_index 0

Techniques also contain one or more @ref Passes (and there must be at least one).

<a name="shadow_005fcaster_005fmaterial"></a><a name="shadow_005fcaster_005fmaterial-1"></a>

## shadow\_caster\_material

When using @ref Texture_002dbased-Shadows you can specify an alternate material to use when rendering the object using this material into the shadow texture. This is like a more advanced version of using @c shadow_caster_vertex_program, however note that for the moment you are expected to render the shadow in one pass, i.e. only the first pass is respected.

<a name="shadow_005freceiver_005fmaterial"></a><a name="shadow_005freceiver_005fmaterial-1"></a>

## shadow\_receiver\_material

When using @ref Texture_002dbased-Shadows you can specify an alternate material to use when performing the receiver shadow pass. This is like a more advanced version of using @c shadow_receiver_vertex_program and @c shadow_receiver_fragment_program, however note that for the moment you are expected to render the shadow in one pass, i.e. only the first pass is respected.

@note This explicit ’receiver’ pass is only done when you’re **not** using @ref Integrated-Texture-Shadows - i.e. the shadow rendering is done separately (either as a modulative pass, or a masked light pass).

<a name="gpu_005fvendor_005frule"></a><a name="gpu_005fdevice_005frule"></a><a name="gpu_005fvendor_005frule-and-gpu_005fdevice_005frule"></a>

## gpu\_vendor\_rule and gpu\_device\_rule

Although Ogre does a good job of detecting the capabilities of graphics cards and setting the supportability of techniques from that, occasionally card-specific behaviour exists which is not necessarily detectable and you may want to ensure that your materials go down a particular path to either use or avoid that behaviour. This is what these rules are for - you can specify matching rules so that a technique will be considered supportable only on cards from a particular vendor, or which match a device name pattern, or will be considered supported only if they **don’t** fulfil such matches. The format of the rules are as follows:

@par
gpu\_vendor\_rule &lt;include|exclude&gt; &lt;vendor\_name&gt;<br> gpu\_device\_rule &lt;include|exclude&gt; &lt;device\_pattern&gt; \[case\_sensitive\]

An ’include’ rule means that the technique will only be supported if one of the include rules is matched (if no include rules are provided, anything will pass). An ’exclude’ rules means that the technique is considered unsupported if any of the exclude rules are matched. You can provide as many rules as you like, although &lt;vendor\_name&gt; and &lt;device\_pattern&gt; must obviously be unique. The valid list of &lt;vendor\_name&gt; values is currently ’nvidia’, ’ati’, ’intel’, ’s3’, ’matrox’ and ’3dlabs’. &lt;device\_pattern&gt; can be any string, and you can use wildcards (’\*’) if you need to match variants. Here’s an example:

@par
gpu\_vendor\_rule include nvidia<br> gpu\_vendor\_rule include intel<br> gpu\_device\_rule exclude \*950\*

These rules, if all included in one technique, will mean that the technique will only be considered supported on graphics cards made by NVIDIA and Intel, and so long as the device name doesn’t have ’950’ in it.

Note that these rules can only mark a technique ’unsupported’ when it would otherwise be considered ’supported’ judging by the hardware capabilities. Even if a technique passes these rules, it is still subject to the usual hardware support tests.

# Passes {#Passes}

A pass is a single render of the geometry in question; a single call to the rendering API with a certain set of rendering properties. A technique can have between one and 16 passes, although clearly the more passes you use, the more expensive the technique will be to render.

To help clearly identify what each pass is used for, the pass can be named but its optional. Passes not named within the script will take on a name that is the pass index number. For example: the first pass in a technique is index 0 so its name would be "0" if it was not given a name in the script. The pass name must be unique within the technique or else the final pass is the resulting merge of all passes with the same name in the technique. A warning message is posted in the Ogre.log if this occurs. Named passes can help when inheriting a material and modifying an existing pass: (See @ref Script-Inheritance)

Passes have a set of global attributes (described below) and optionally
- zero or more nested texture\_unit entries (See @ref Texture-Units)
- references to shader programs (See @ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass)
- additional instructions for the RTSS (See @ref rtss_custom_mat)



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
-   [line_width](#line_width)

<a name="Attribute-Descriptions"></a>

# Attribute Descriptions

<a name="ambient"></a><a name="ambient-1"></a>

## ambient

Sets the ambient colour reflectance properties of this pass.

@par
Format: ambient (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

@copydetails Ogre::Pass::setAmbient
@shaderparam

@par
Example: ambient 0.0 0.8 0.0
@par
Default: ambient 1.0 1.0 1.0 1.0

<a name="diffuse"></a><a name="diffuse-1"></a>

## diffuse

Sets the diffuse colour reflectance properties of this pass.
@par
Format: diffuse (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

@copydetails Ogre::Pass::setDiffuse
@shaderparam

@par
Example: diffuse 1.0 0.5 0.5
@par
Default: diffuse 1.0 1.0 1.0 1.0

<a name="specular"></a><a name="specular-1"></a>

## specular

Sets the specular colour reflectance properties of this pass.
@par
Format: specular (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour) &lt;shininess&gt;<br> NB valid colour values are between 0.0 and 1.0. Shininess can be any value greater than 0.


This property determines how much specular light (highlights from instances of the Light class in the scene) is reflected. The default is to reflect no specular light. The colour of the specular highlights is determined by the colour parameters, and the size of the highlights by the separate shininess parameter.
It is also possible to make the specular reflectance track the vertex colour as defined in
the mesh instead of the colour values.

@copydetails Ogre::Pass::setShininess
@shaderparam

@par
Example: specular 1.0 1.0 1.0 12.5

@par
Default: specular 0.0 0.0 0.0 0.0 0.0

<a name="emissive"></a><a name="emissive-1"></a>

## emissive

Sets the amount of self-illumination an object has.

@par
Format: emissive (&lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]| vertexcolour)<br> NB valid colour values are between 0.0 and 1.0.

Unlike the name suggests, this object doesn’t act as a light source for other objects in the scene (if you want it to, you have to create a light which is centered on the object).
@copydetails Ogre::Pass::setSelfIllumination
@shaderparam

@par
Example: emissive 1.0 0.0 0.0
@par
Default: emissive 0.0 0.0 0.0 0.0

<a name="scene_005fblend"></a><a name="scene_005fblend-1"></a>

## scene\_blend

Sets the kind of blending this pass has with the existing contents of the scene. 

Whereas the texture blending operations seen in the texture\_unit entries are concerned with blending between texture layers, this blending is about combining the output of this pass as a whole with the existing contents of the rendering target. This blending therefore allows object transparency and other special effects. 

There are 2 formats, one using predefined blend types, the other allowing a roll-your-own approach using source and destination factors.
@par
Format1: scene\_blend &lt;blend\_type&gt; 
@par
Example: scene\_blend add

This is the simpler form, where the most commonly used blending modes are enumerated using a single parameter.

@param blend_type
<dl compact="compact">
<dt>add</dt>
<dd>@copybrief Ogre::SBT_ADD 

Equivalent to ’scene_blend one one’.</dd>
<dt>modulate</dt> 
<dd>@copybrief Ogre::SBT_MODULATE 

Equivalent to ’scene_blend dest_colour zero’.</dd>
<dt>colour_blend</dt>
<dd>@copybrief Ogre::SBT_TRANSPARENT_COLOUR 

Equivalent to ’scene_blend src_colour one_minus_src_colour’</dd>
<dt>alpha_blend</dt>
<dd>@copybrief Ogre::SBT_TRANSPARENT_ALPHA 

Equivalent to ’scene_blend src_alpha one_minus_src_alpha’</dd>
</dl>

@par
Format2: scene\_blend &lt;sourceFactor&gt; &lt;destFactor&gt;

@copydetails Ogre::Pass::setSceneBlending(const SceneBlendFactor, const SceneBlendFactor)

Valid values for both parameters are one of Ogre::SceneBlendFactor without the `SBF_` prefix. E.g. `SBF_DEST_COLOUR` becomes `dest_colour`.

@par
Example: scene\_blend one one\_minus\_dest\_alpha

@par
Default: scene\_blend one zero (opaque)  

Also see [separate\_scene\_blend](#separate_005fscene_005fblend).

<a name="separate_005fscene_005fblend"></a><a name="separate_005fscene_005fblend-1"></a>

## separate\_scene\_blend

This option operates in exactly the same way as [scene\_blend](#scene_005fblend), except that it allows you to specify the operations to perform between the rendered pixel and the frame buffer separately for colour and alpha components. By nature this option is only useful when rendering to targets which have an alpha channel which you’ll use for later processing, such as a render texture.
@par
Format1: separate\_scene\_blend &lt;simple\_colour\_blend&gt; &lt;simple\_alpha\_blend&gt;
@par
Example: separate\_scene\_blend add modulate

This example would add colour components but multiply alpha components. The blend modes available are as in [scene\_blend](#scene_005fblend). The more advanced form is also available:
@par
Format2: separate\_scene\_blend &lt;colour\_src\_factor&gt; &lt;colour\_dest\_factor&gt; &lt;alpha\_src\_factor&gt; &lt;alpha\_dest\_factor&gt;
@par
Example: separate\_scene\_blend one one\_minus\_dest\_alpha one one 

Again the options available in the second format are the same as those in the second format of [scene\_blend](#scene_005fblend).

<a name="scene_005fblend_005fop"></a><a name="scene_005fblend_005fop-1"></a>

## scene\_blend\_op

This directive changes the operation which is applied between the two components of the scene blending equation

@par
Format: scene\_blend\_op &lt;op&gt; 

@copydetails Ogre::Pass::setSceneBlendingOperation
You may change this to ’add’, ’subtract’, ’reverse_subtract’, ’min’ or ’max’.

<a name="separate_005fscene_005fblend_005fop"></a><a name="separate_005fscene_005fblend_005fop-1"></a>

## separate\_scene\_blend\_op

This directive is as scene\_blend\_op, except that you can set the operation for colour and alpha separately.
@par
Format: separate\_scene\_blend\_op &lt;colourOp&gt; &lt;alphaOp&gt; Default: separate\_scene\_blend\_op add add

<a name="depth_005fcheck"></a><a name="depth_005fcheck-1"></a>

## depth\_check

Sets whether or not this pass renders with depth-buffer checking on or not.
@par
Format: depth\_check &lt;on|off&gt;

@copydetails Ogre::Pass::setDepthCheckEnabled

Default: depth\_check on

<a name="depth_005fwrite"></a><a name="depth_005fwrite-1"></a>

## depth\_write

Sets whether or not this pass renders with depth-buffer writing on or not.

@par
Format: depth\_write &lt;on|off&gt;

@copydetails Ogre::Pass::setDepthWriteEnabled

@par
Default: depth\_write on<br>

<a name="depth_005ffunc"></a><a name="depth_005ffunc-1"></a>

## depth\_func

Sets the function used to compare depth values when depth checking is on.
@par
Format: depth\_func &lt;func&gt;

@copydetails Ogre::Pass::setDepthFunction

@param func one of Ogre::CompareFunction without the `CMPF_` prefix. E.g. `CMPF_LESS_EQUAL` becomes `less_equal`.

@par
Default: depth\_func less\_equal

<a name="depth_005fbias"></a><a name="depth_005fbias-1"></a>

## depth\_bias

Sets the bias applied to the depth value of this pass.
@par
Format: depth\_bias &lt;constantBias&gt; \[&lt;slopeScaleBias&gt;\]

@copydetails Ogre::Pass::setDepthBias

Also see [iteration\_depth\_bias](#iteration_005fdepth_005fbias)

<a name="iteration_005fdepth_005fbias"></a><a name="iteration_005fdepth_005fbias-1"></a>

## iteration\_depth\_bias

Sets an additional bias derived from the number of times a given pass has been iterated. Operates just like [depth\_bias](#depth_005fbias) except that it applies an additional bias factor to the base depth\_bias value, multiplying the provided value by the number of times this pass has been iterated before, through one of the [iteration](#iteration) variants. So the first time the pass will get the depth\_bias value, the second time it will get depth\_bias + iteration\_depth\_bias, the third time it will get depth\_bias + iteration\_depth\_bias \* 2, and so on. The default is zero. 
@par
Format: iteration\_depth\_bias &lt;bias\_per\_iteration&gt;

 <a name="alpha_005frejection"></a><a name="alpha_005frejection-1"></a>

## alpha\_rejection

Sets the way the pass will have use alpha to totally reject pixels from the pipeline.
@par
Format: alpha\_rejection &lt;function&gt; &lt;value&gt;
@par
Example: alpha\_rejection greater\_equal 128

The function parameter can be any of the options listed in the material depth\_function attribute. The value parameter can theoretically be any value between 0 and 255, but is best limited to 0 or 128 for hardware compatibility.

@ffp_rtss_only

@par
Default: alpha\_rejection always\_pass

<a name="alpha_005fto_005fcoverage"></a><a name="alpha_005fto_005fcoverage-1"></a>

## alpha\_to\_coverage

Sets whether this pass will use ’alpha to coverage’,

@par
Format: alpha\_to\_coverage &lt;on|off&gt;

@copydetails Ogre::Pass::setAlphaToCoverageEnabled

@par
Default: alpha\_to\_coverage off <a name="light_005fscissor"></a>

<a name="light_005fscissor-1"></a>

## light_scissor

Sets whether when rendering this pass, rendering will be limited to a screen-space scissor rectangle representing the coverage of the light(s) being used in this pass.
@par
Format: light\_scissor &lt;on|off&gt; 
@par
Default: light\_scissor off

@copydetails Ogre::Pass::setLightScissoringEnabled

<a name="light_005fclip_005fplanes"></a><a name="light_005fclip_005fplanes-1"></a>

## light\_clip\_planes

Sets whether when rendering this pass, triangle setup will be limited to clipping volume covered by the light.
@par
Format: light\_clip\_planes &lt;on|off&gt; 
@par
Default: light\_clip\_planes off

@copydetails Ogre::Pass::setLightClipPlanesEnabled

@see @ref Integrated-Texture-Shadows

## illumination_stage {#illumination_005fstage}

@copydetails Ogre::Pass::setIlluminationStage

@par
Format: illumination\_stage &lt;ambient|per\_light|decal&gt; Default: none (autodetect)

<a name="transparent_005fsorting"></a><a name="transparent_005fsorting-1"></a>

## transparent\_sorting

Sets if transparent textures should be sorted by depth or not.
@par
Format: transparent\_sorting &lt;on|off|force&gt;

By default all transparent materials are sorted such that renderables furthest away from the camera are rendered first. This is usually the desired behaviour but in certain cases this depth sorting may be unnecessary and undesirable. If for example it is necessary to ensure the rendering order does not change from one frame to the next. In this case you could set the value to ’off’ to prevent sorting.

You can also use the keyword ’force’ to force transparent sorting on, regardless of other circumstances. Usually sorting is only used when the pass is also transparent, and has a depth write or read which indicates it cannot reliably render without sorting. By using ’force’, you tell OGRE to sort this pass no matter what other circumstances are present.
@par
Default: transparent\_sorting on

<a name="cull_005fhardware"></a><a name="cull_005fhardware-1"></a>

## cull\_hardware

Sets the hardware culling mode for this pass.
@par
Format: cull\_hardware &lt;clockwise|anticlockwise|none&gt;

@copydetails Ogre::Pass::setCullingMode

@par
Default: cull\_hardware clockwise<br> NB this is the same as OpenGL’s default but the opposite of Direct3D’s default (because Ogre uses a right-handed coordinate system like OpenGL).

<a name="cull_005fsoftware"></a><a name="cull_005fsoftware-1"></a>

## cull\_software

Sets the software culling mode for this pass.
@par
Format: cull\_software &lt;back|front|none&gt;

@copydetails Ogre::Pass::setManualCullingMode

@par
Default: cull\_software back

<a name="lighting"></a><a name="lighting-1"></a>

## lighting

Sets whether or not dynamic lighting is turned on for this pass or not.

@par
Format: lighting &lt;on|off&gt;

@copydetails Ogre::Pass::setLightingEnabled
@par
Default: lighting on

<a name="shading"></a><a name="shading-1"></a>

## shading

Sets the kind of shading which should be used for representing dynamic lighting for this pass.
@par
Format: shading &lt;mode&gt;

@copydetails Ogre::Pass::setShadingMode

@param mode one of Ogre::ShadeOptions without the `SO_` prefix. E.g. `SO_FLAT` becomes `flat`.

@par
Default: shading gouraud

<a name="polygon_005fmode"></a><a name="polygon_005fmode-1"></a>

## polygon\_mode

@copydetails Ogre::Pass::setPolygonMode

@par
Format: polygon_mode &lt;solid|wireframe|points&gt;

@param mode one of Ogre::PolygonMode without the `PM_` prefix. E.g. `PM_SOLID` becomes `solid`.

@par
Default: polygon\_mode solid

<a name="polygon_005fmode_005foverrideable"></a><a name="polygon_005fmode_005foverrideable-1"></a>

## polygon\_mode\_overrideable

Sets whether or not the [polygon\_mode](#polygon_005fmode) set on this pass can be downgraded by the camera

@par
Format: polygon\_mode\_overrideable &lt;override&gt;

@copydetails Ogre::Pass::setPolygonModeOverrideable

<a name="fog_005foverride"></a><a name="fog_005foverride-1"></a>

## fog\_override

Tells the pass whether it should override the scene fog settings, and enforce it’s own. Very useful for things that you don’t want to be affected by fog when the rest of the scene is fogged, or vice versa.
@par
Format: fog\_override &lt;override?&gt; \[&lt;type&gt; &lt;colour&gt; &lt;density&gt; &lt;start&gt; &lt;end&gt;\]
@par
Default: fog\_override false

If you specify ’true’ for the first parameter and you supply the rest of the parameters, you are telling the pass to use these fog settings in preference to the scene settings, whatever they might be. If you specify ’true’ but provide no further parameters, you are telling this pass to never use fogging no matter what the scene says.

@param type **none** = No fog, equivalent of just using ’fog\_override true’<br> **linear** = Linear fog from the &lt;start&gt; and &lt;end&gt; distances<br> **exp** = Fog increases exponentially from the camera (fog = 1/e^(distance \* density)), use &lt;density&gt; param to control it<br> **exp2** = Fog increases at the square of FOG\_EXP, i.e. even quicker (fog = 1/e^(distance \* density)^2), use &lt;density&gt; param to control it

@param colour Sequence of 3 floating point values from 0 to 1 indicating the red, green and blue intensities

@param density The density parameter used in the ’exp’ or ’exp2’ fog types. Not used in linear mode but param must still be there as a placeholder

@param start The start distance from the camera of linear fog. Must still be present in other modes, even though it is not used.

@param end The end distance from the camera of linear fog. Must still be present in other modes, even though it is not used.

@par
Example: fog\_override true exp 1 1 1 0.002 100 10000

<a name="colour_005fwrite"></a><a name="colour_005fwrite-1"></a>

## colour\_write

Sets whether this pass renders with colour writing on or not. Alternatively, it can also be used to enable/disable colour writing specific channels.
In the second format, the parameters are in the red, green, blue, alpha order.

@par
Format 1: colour\_write &lt;on|off&gt;
@par
Format 2: colour\_write &lt;on|off&gt; &lt;on|off&gt; &lt;on|off&gt; &lt;on|off&gt;

@copydetails Ogre::Pass::setColourWriteEnabled

@par
Default: colour\_write on<br>

<a name="colour_005fmask"></a><a name="colour_005fmask-1"></a>

## start\_light

Sets the first light which will be considered for use with this pass.
@par
Format: start\_light &lt;number&gt;

@copydetails Ogre::Pass::setStartLight

@par
Default: start\_light 0<br>

<a name="max_005flights"></a><a name="max_005flights-1"></a>

## max\_lights

Sets the maximum number of lights which will be considered for use with this pass.
@par
Format: max\_lights &lt;number&gt;

The maximum number of lights which can be used when rendering fixed-function materials is set by the rendering system, and is typically set at 8. When you are using the programmable pipeline (See [Using Vertex/Geometry/Fragment Programs in a Pass](@ref Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass)) this limit is dependent on the program you are running, or, if you use ’iteration once\_per\_light’ or a variant (See @ref iteration), it effectively only bounded by the number of passes you are willing to use. If you are not using pass iteration, the light limit applies once for this pass. If you are using pass iteration, the light limit applies across all iterations of this pass - for example if you have 12 lights in range with an ’iteration once\_per\_light’ setup but your max\_lights is set to 4 for that pass, the pass will only iterate 4 times. 
@par
Default: max\_lights 8<br>

## iteration {#iteration}

Sets whether or not this pass is iterated, i.e. issued more than once.
@par
Format 1: iteration &lt;once | once\_per\_light&gt; \[lightType\]

@par
Format 2: iteration &lt;number&gt; \[&lt;per\_light&gt; \[lightType\]

@par
Format 3: iteration &lt;number&gt; \[&lt;per\_n\_lights&gt; &lt;num\_lights&gt; \[lightType\]

@par
Examples:

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

@note use gpu program auto parameters [pass\_number](#pass_005fnumber) and [pass\_iteration\_number](#pass_005fiteration_005fnumber) to tell the vertex, geometry or fragment program the pass number and iteration number.

<a name="point_005fsize"></a><a name="point_005fsize-1"></a>

## point\_size

@copydetails Ogre::Pass::setPointSize

@par
Format: point\_size &lt;size&gt; Default: point\_size 1.0

@ffp_rtss_only

<a name="point_005fsprites"></a><a name="point_005fsprites-1"></a>

## point\_sprites

@copydetails Ogre::Pass::setPointSpritesEnabled

@par
Format: point\_sprites &lt;on|off&gt; Default: point\_sprites off

<a name="point_005fsize_005fattenuation"></a><a name="point_005fsize_005fattenuation-1"></a>

## point\_size\_attenuation

Defines whether point size is attenuated with view space distance, and in what fashion.

@par
Format: point\_size\_attenuation &lt;enabled&gt; \[constant linear quadratic\] Default: point\_size\_attenuation off

@copydetails Ogre::Pass::setPointAttenuation

@ffp_rtss_only

<a name="point_005fsize_005fmin"></a><a name="point_005fsize_005fmin-1"></a>

## point\_size\_min

Sets the minimum point size after attenuation ([point\_size\_attenuation](#point_005fsize_005fattenuation)). For details on the size metrics, See [point\_size](#point_005fsize).
@par
Format: point\_size\_min &lt;size&gt; Default: point\_size\_min 0

<a name="point_005fsize_005fmax"></a><a name="point_005fsize_005fmax-1"></a>

## point\_size\_max

Sets the maximum point size after attenuation ([point\_size\_attenuation](#point_005fsize_005fattenuation)). For details on the size metrics, See [point\_size](#point_005fsize). A value of 0 means the maximum is set to the same as the max size reported by the current card. 
@par
Format: point\_size\_max &lt;size&gt; Default: point\_size\_max 0

<a name="line_width"></a>
## line_width
@copydetails Ogre::Pass::setLineWidth

@par
Format: line_width &lt;width&gt; 
@par
Default: line_width 1

# Texture Units {#Texture-Units}

Here are the attributes you can use in a @c texture_unit section of a .material script:

<a name="Available-Texture-Layer-Attributes"></a>

## Available Texture Unit Attributes

-   [texture\_alias](#texture_005falias)
-   [texture](#texture)
-   [anim\_texture](#anim_005ftexture)
-   [cubic\_texture](#cubic_005ftexture)
-   [tex\_coord\_set](#tex_005fcoord_005fset)
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
-   [content\_type](#content_005ftype)
-   [sampler_ref](#sampler_ref)
-   [unordered_access_mip](#unordered_access_mip)

@note Furthermore all attributes of @ref Samplers are available. Using any of them will create a new Ogre::Sampler local to the texture unit.
This means that any changes you made to the Default Sampler e.g. via Ogre::MaterialManager::setDefaultTextureFiltering have no effect anymore.
If several texture units share the same Sampler settings, you are encouraged to reference the same Sampler via [sampler_ref](#sampler_ref) for improved performance.

You can also use nested section in order to use a special add-ins
- @c texture_source as a source of texture data, see @ref External-Texture-Sources for details
- @c rtshader_system for additional layer blending  options, see @ref rtss for details.

<a name="Attribute-Descriptions-1"></a>

## Attribute Descriptions

<a name="texture_005falias"></a><a name="texture_005falias-1"></a>

## texture\_alias

Sets the alias name for this texture unit.
@par
Format: texture\_alias &lt;name&gt;
@par
Example: texture\_alias NormalMap

@deprecated texture aliases are a restricted version of @ref Script-Variables, which you should instead.

<a name="texture"></a><a name="texture-1"></a>

## texture

Sets the name of the static texture image this layer will use.
@par
Format: texture &lt;texturename&gt; \[&lt;type&gt;\] \[unlimited | numMipMaps\] \[&lt;PixelFormat&gt;\] \[gamma\]
@par
Example: texture funkywall.jpg

This setting is mutually exclusive with the anim\_texture attribute. Note that the texture file cannot include spaces. Those of you Windows users who like spaces in filenames, please get over it and use underscores instead. 

@param type
specify a the type of texture to create - the default is ’2d’, but you can override this; here’s the full list:
<dl compact="compact">
<dt>1d</dt> <dd>

A 1-dimensional texture; that is, a texture which is only 1 pixel high. These kinds of textures can be useful when you need to encode a function in a texture and use it as a simple lookup, perhaps in a fragment program. It is important that you use this setting when you use a fragment program which uses 1-dimensional texture coordinates, since GL requires you to use a texture type that matches (D3D will let you get away with it, but you ought to plan for cross-compatibility). Your texture widths should still be a power of 2 for best compatibility and performance.

</dd> <dt>2d</dt> <dd>

The default type which is assumed if you omit it, your texture has a width and a height, both of which should preferably be powers of 2, and if you can, make them square because this will look best on the most hardware. These can be addressed with 2D texture coordinates.

</dd> <dt>3d</dt> <dd>

A 3 dimensional texture i.e. volume texture. Your texture has a width, a height, both of which should be powers of 2, and has depth. These can be addressed with 3d texture coordinates i.e. through a pixel shader.

</dd> <dt>cubic</dt> <dd>

This texture is made up of 6 2D textures which are pasted around the inside of a cube.
Can be addressed with 3D texture coordinates and are useful for cubic reflection maps.
If the @c texturename in this format is something like @c skybox.jpg, the system will expect `skybox_px.jpg, skybox_nx.jpg, skybox_py.jpg, skybox_ny.jpg, skybox_pz.jpg, skybox_nz.jpg` for the individual faces. For compatibility, the suffixes `_lf, _rt, _up, _dn, _fr, _bk`  are also supported.
Alternatively a single file with all faces can be used, if supported by the texture format (e.g. DDS).
</dd> </dl>

@param numMipMaps
specify the number of mipmaps to generate for this texture. The default is ’unlimited’ which means mips down to 1x1 size are generated. You can specify a fixed number (even 0) if you like instead. Note that if you use the same texture in many material scripts, the number of mipmaps generated will conform to the number specified in the first texture\_unit used to load the texture - so be consistent with your usage.

@param PixelFormat
specify the desired pixel format of the texture to create, which may be different to the pixel format of the texture file being loaded. Bear in mind that the final pixel format will be constrained by hardware capabilities so you may not get exactly what you ask for. 
Names defined in Ogre::PixelFormat are valid values.

@param gamma
informs the renderer that you want the graphics hardware to perform gamma correction on the texture values as they are sampled for rendering. This is only applicable for textures which have 8-bit colour channels (e.g.PF\_R8G8B8). Often, 8-bit per channel textures will be stored in [gamma space](http://en.wikipedia.org/wiki/Gamma_correction) in order to increase the precision of the darker colours but this can throw out blending and filtering calculations since they assume linear space colour values. For the best quality shading, you may want to enable gamma correction so that the hardware converts the texture values to linear space for you automatically when sampling the texture, then the calculations in the pipeline can be done in a reliable linear colour space. When rendering to a final 8-bit per channel display, you’ll also want to convert back to gamma space which can be done in your shader (by raising to the power 1/2.2) or you can enable gamma correction on the texture being rendered to or the render window. Note that the ’gamma’ option on textures is applied on loading the texture so must be specified consistently if you use this texture in multiple places.

<a name="anim_005ftexture"></a><a name="anim_005ftexture-1"></a>

## anim\_texture

Sets the images to be used in an animated texture layer. There are 2 formats, one for implicitly determined image names, one for explicitly named images.
@par
Format1 (short): anim\_texture &lt;name&gt; &lt;numFrames&gt; &lt;duration&gt;

@copydetails Ogre::TextureUnitState::setAnimatedTextureName

@par
Example: anim\_texture flame.jpg 5 2.5

@par
Format2 (long): anim\_texture &lt;frame1&gt; &lt;frame2&gt; ... &lt;duration&gt;
@par
Example: anim\_texture flamestart.jpg flamemore.png flameagain.jpg moreflame.jpg lastflame.tga 2.5

This sets up the same duration animation but from 5 separately named image files. The first format is more concise, but the second is provided if you cannot make your images conform to the naming standard required for it. 
@par
Default: none

<a name="cubic_005ftexture"></a><a name="cubic_005ftexture-1"></a>

## cubic\_texture

Sets the images used in a cubic texture, i.e. one made up of 6 individual images making up the faces of a cube or 1 cube texture if supported by the texture format(DDS for example) and rendersystem.. These kinds of textures are used for reflection maps (if hardware supports cubic reflection maps) or skyboxes. There are 2 formats, a brief format expecting image names of a particular format and a more flexible but longer format for arbitrarily named textures.
@par
Format1 (short): cubic\_texture &lt;base\_name&gt; &lt;combinedUVW|separateUV&gt;

@deprecated use the format '`texture <basename> cubic`' instead

@par
Format2 (long): cubic\_texture &lt;pz&gt; &lt;nz&gt; &lt;nx&gt; &lt;px&gt; &lt;py&gt; &lt;ny&gt; &lt;combinedUVW|separateUV&gt;

In this case each face is specified explicitly, in case you don’t want to conform to the image naming standards above.

In both cases the final parameter means the following:

<dl compact="compact">
<dt>combinedUVW</dt> <dd>
The 6 textures are combined into a single ’cubic’ texture map which is then addressed using 3D texture coordinates.
</dd>
<dt>separateUV</dt> <dd>
This is no longer supported and behaves like combinedUVW.
</dd>
</dl> <br>

<a name="content_005ftype"></a>

## content_type

Tells this texture unit where it should get its content from. The default is to get texture content from a named texture, as defined with the [texture](#texture), [cubic\_texture](#cubic_005ftexture), [anim\_texture](#anim_005ftexture) attributes. However you can also pull texture information from other automated sources.

@par
Format: content\_type &lt;type&gt; \[&lt;compositorName&gt;\] \[&lt;textureName&gt;\] \[&lt;mrtIndex&gt;\]

@param type
<dl compact="compact">
<dt>named</dt> <dd>

@copybrief Ogre::TextureUnitState::CONTENT_NAMED

</dd> <dt>shadow</dt> <dd>

This option allows you to pull in a shadow texture, and is only valid when you use texture shadows and one of the ’custom sequence’ shadowing types (See @ref Shadows). The shadow texture in question will be from the ’n’th closest light that casts shadows, unless you use light-based pass iteration or the light\_start option which may start the light index higher. When you use this option in multiple texture units within the same pass, each one references the next shadow texture. The shadow texture index is reset in the next pass, in case you want to take into account the same shadow textures again in another pass (e.g. a separate specular / gloss pass). By using this option, the correct light frustum projection is set up for you for use in fixed-function, if you use shaders just reference the texture\_viewproj\_matrix auto parameter in your shader.

</dd> <dt>compositor</dt> <dd>

@copybrief Ogre::TextureUnitState::CONTENT_COMPOSITOR This can be either in a render\_scene directive inside a compositor script, or in a general pass in a viewport that has a compositor attached. Note that this is a reference only, meaning that it does not change the render order. You must make sure that the order is reasonable for what you are trying to achieve (for example, texture pooling might cause the referenced texture to be overwritten by something else by the time it is referenced).

</dd> </dl>

@copydetails Ogre::TextureUnitState::setCompositorReference

@par
Example: content\_type compositor DepthCompositor OutputTexture 

@par
Default: content\_type named


<a name="tex_005fcoord_005fset"></a><a name="tex_005fcoord_005fset-1"></a>

## tex\_coord\_set

@copybrief Ogre::TextureUnitState::setTextureCoordSet
@copydetails Ogre::TextureUnitState::setTextureCoordSet

@par
Format: tex\_coord\_set &lt;set\_num&gt;

@ffp_rtss_only

@par
Example: tex\_coord\_set 2
@par
Default: tex\_coord\_set 0

<a name="colour_005fop"></a><a name="colour_005fop-1"></a>

## colour\_op

@ffp_rtss_only

Determines how the colour of this texture layer is combined with the one below it (or the lighting effect on the geometry if this is the first layer).
@par
Format: colour\_op &lt;op&gt;

@copydetails Ogre::TextureUnitState::setColourOperation Without the `LBO_` prefix. E.g. `LBO_REPLACE` becomes `replace`.

@par
Default: colour\_op modulate

<a name="colour_005fop_005fex"></a><a name="colour_005fop_005fex-1"></a>

## colour\_op\_ex

@ffp_rtss_only
@par
Format: colour\_op\_ex &lt;op&gt; &lt;source1&gt; &lt;source2&gt; \[&lt;manualBlend&gt;\] \[&lt;arg1&gt;\] \[&lt;arg2&gt;\]
@par
Example colour\_op\_ex add\_signed src\_manual src\_current 0.5

@copydetails Ogre::TextureUnitState::setColourOperationEx

Each parameter can be one of Ogre::LayerBlendOperationEx or Ogre::LayerBlendSource without the prefix. E.g. `LBX_MODULATE_X4` becomes `modulate_x4`.

@par
Default: none (colour\_op modulate)<br>

<a name="colour_005fop_005fmultipass_005ffallback"></a><a name="colour_005fop_005fmultipass_005ffallback-1"></a>

## colour\_op\_multipass\_fallback

Sets the multipass fallback operation for this layer, if you used colour\_op\_ex and not enough multitexturing hardware is available.
@par
Format: colour\_op\_multipass\_fallback &lt;src\_factor&gt; &lt;dest\_factor&gt;
@par
Example: colour\_op\_multipass\_fallback one one\_minus\_dest\_alpha

@copydetails Ogre::TextureUnitState::setColourOpMultipassFallback

<a name="alpha_005fop_005fex"></a><a name="alpha_005fop_005fex-1"></a>

## alpha\_op\_ex

@ffp_rtss_only

@par
Format: alpha\_op\_ex &lt;op&gt; &lt;source1&gt; &lt;source2&gt; \[&lt;manualBlend&gt;\] \[&lt;arg1&gt;\] \[&lt;arg2&gt;\]

@copydetails Ogre::TextureUnitState::setAlphaOperation

<a name="env_005fmap"></a><a name="env_005fmap-1"></a>

## env\_map

@ffp_rtss_only

@copybrief Ogre::TextureUnitState::setEnvironmentMap

@par
Format: env\_map &lt;off|spherical|planar|cubic\_reflection|cubic\_normal&gt;

Environment maps make an object look reflective by using automatic texture coordinate generation depending on the relationship between the objects vertices or normals and the eye.

<dl compact="compact">
<dt>spherical</dt> <dd>

@copybrief Ogre::TextureUnitState::ENV_CURVED
Requires a single texture which is either a fish-eye lens view of the reflected scene, or some other texture which looks good as a spherical map (a texture of glossy highlights is popular especially in car sims). This effect is based on the relationship between the eye direction and the vertex normals of the object, so works best when there are a lot of gradually changing normals, i.e. curved objects.

</dd> <dt>planar</dt> <dd>

The effect is based on the position of the vertices in the viewport rather than vertex normals. This is useful for planar geometry (where a spherical env\_map would not look good because the normals are all the same) or objects without normals.

@remarks This was never actually implemented. Same as @c spherical on all backends.

</dd> <dt>cubic\_reflection</dt> <dd>

@copybrief Ogre::TextureUnitState::ENV_REFLECTION
Uses a group of 6 textures making up the inside of a cube, each of which is a view if the scene down each axis. Works extremely well in all cases but has a higher technical requirement from the card than spherical mapping. Requires that you bind a [cubic texture](#texture) to this unit.

</dd> <dt>cubic\_normal</dt> <dd>
@copybrief Ogre::TextureUnitState::ENV_NORMAL
Generates 3D texture coordinates containing the camera space normal vector from the normal information held in the vertex data. Again, use of this feature requires a [cubic texture](#texture).

</dd> </dl> <br>
@par
Default: env\_map off<br>

<a name="scroll"></a><a name="scroll-1"></a>

## scroll

@copybrief Ogre::TextureUnitState::setTextureScroll 
@par
Format: scroll &lt;u&gt; &lt;v&gt;

@copydetails Ogre::TextureUnitState::setTextureScroll

@shaderparam

<a name="scroll_005fanim"></a><a name="scroll_005fanim-1"></a>

## scroll\_anim

@copybrief Ogre::TextureUnitState::setScrollAnimation 
@par
Format: scroll\_anim &lt;uSpeed&gt; &lt;vSpeed&gt;<br>

@copydetails Ogre::TextureUnitState::setScrollAnimation 

@shaderparam
<a name="rotate"></a><a name="rotate-1"></a>

## rotate

@copybrief Ogre::TextureUnitState::setTextureRotate

@par
Format: rotate &lt;angle_in_deg&gt;

@copydetails Ogre::TextureUnitState::setTextureRotate

@shaderparam

<a name="rotate_005fanim"></a><a name="rotate_005fanim-1"></a>

## rotate\_anim

@copybrief Ogre::TextureUnitState::setRotateAnimation 

@par
Format: rotate\_anim &lt;speed&gt;

@copydetails Ogre::TextureUnitState::setRotateAnimation 

@shaderparam

<a name="scale"></a><a name="scale-1"></a>

## scale

@copybrief Ogre::TextureUnitState::setTextureScale

@par
Format: scale &lt;uScale&gt; &lt;vScale&gt;

@copydetails Ogre::TextureUnitState::setTextureScale


@shaderparam

<a name="wave_005fxform"></a><a name="wave_005fxform-1"></a>

## wave\_xform

@copybrief Ogre::TextureUnitState::setTransformAnimation

@par
Format: wave\_xform &lt;ttype&gt; &lt;waveType&gt; &lt;base&gt; &lt;frequency&gt; &lt;phase&gt; &lt;amplitude&gt;
@par
Example: wave\_xform scale\_x sine 1.0 0.2 0.0 5.0

@copydetails Ogre::TextureUnitState::setTransformAnimation

ttype is one of
<dl compact="compact">
<dt>scroll\_x</dt> <dd>

Animate the u scroll value

</dd> <dt>scroll\_y</dt> <dd>

Animate the v scroll value

</dd> <dt>rotate</dt> <dd>

Animate the rotate value

</dd> <dt>scale\_x</dt> <dd>

Animate the u scale value

</dd> <dt>scale\_y</dt> <dd>

Animate the v scale value

</dd> </dl>

waveType is one of Ogre::WaveformType without the `WFT_` prefix. E.g. `WFT_SQUARE` becomes `square`.

@shaderparam

<a name="transform"></a><a name="transform-1"></a>

## transform

This attribute allows you to specify a static 4x4 transformation matrix for the texture unit, thus replacing the individual scroll, rotate and scale attributes mentioned above.
@par
Format: transform m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33

The indexes of the 4x4 matrix value above are expressed as m&lt;row&gt;&lt;col&gt;.

@shaderparam

<a name="sampler_ref"></a>
## sampler_ref

By default all texture units use a shared default Sampler object. This parameter allows you to explicitly set a different one.

@par
Format: sampler_ref &lt;name&gt;

@par
Example: sampler_ref mySampler

<a name="unordered_access_mip"></a>
## unordered_access_mip

@copybrief Ogre::TextureUnitState::setUnorderedAccessMipLevel

@par
Format: unordered_access_mip &lt;mipLevel&gt;

@par
Example: unordered_access_mip 0

# Samplers {#Samplers}

Samplers allow you to quickly change the settings for all associated Textures. Typically you have many Textures but only a few sampling states in your application.

```cpp
sampler mySampler
{
    filtering bilinear
    max_anisotropy 16
}

...
    texture_unit
    {
        texture myTexture.dds
        sampler_ref mySampler
    }
...
```

## Available parameters

-   [filtering](#filtering)
-   [max\_anisotropy](#max_005fanisotropy)
-   [tex\_address\_mode](#tex_005faddress_005fmode)
-   [tex\_border\_colour](#tex_005fborder_005fcolour)
-   [mipmap\_bias](#mipmap_005fbias)
-   [compare_test](#compare_test)
-   [comp_func](#comp_func)

<a name="tex_005faddress_005fmode"></a><a name="tex_005faddress_005fmode-1"></a>

## tex\_address\_mode

Defines what happens when texture coordinates exceed 1.0 for this texture layer.You can use the simple format to specify the addressing mode for all 3 potential texture coordinates at once, or you can use the 2/3 parameter extended format to specify a different mode per texture coordinate. 
@par
Simple Format: tex\_address\_mode &lt;uvw\_mode&gt; <br> Extended Format: tex\_address\_mode &lt;u\_mode&gt; &lt;v\_mode&gt; \[&lt;w\_mode&gt;\]

Valid values for both are one of Ogre::TextureAddressingMode without the `TAM_` prefix. E.g. `TAM_WRAP` becomes `wrap`.

@par
Default: tex\_address\_mode wrap

<a name="tex_005fborder_005fcolour"></a><a name="tex_005fborder_005fcolour-1"></a>

## tex\_border\_colour

Sets the border colour of border texture address mode (see [tex\_address\_mode](#tex_005faddress_005fmode)). 
@par
Format: tex\_border\_colour &lt;red&gt; &lt;green&gt; &lt;blue&gt; \[&lt;alpha&gt;\]<br> NB valid colour values are between 0.0 and 1.0.
@par
Example: tex\_border\_colour 0.0 1.0 0.3
@par
Default: tex\_border\_colour 0.0 0.0 0.0 1.0

<a name="filtering"></a><a name="filtering-1"></a>

## filtering

Sets the type of texture filtering used when magnifying or minifying a texture. There are 2 formats to this attribute, the simple format where you simply specify the name of a predefined set of filtering options, and the complex format, where you individually set the minification, magnification, and mip filters yourself.

### Simple Format
With this format, you only need to provide a single parameter

@par
Format: filtering &lt;none|bilinear|trilinear|anisotropic&gt;<br> Default: filtering bilinear 

<dl compact="compact">
<dt>none</dt> <dd>
@copybrief Ogre::TFO_NONE

@copydetails Ogre::TFO_NONE
</dd> 
<dt>bilinear</dt> <dd> 
@copybrief Ogre::TFO_BILINEAR

@copydetails Ogre::TFO_BILINEAR
</dd> 
<dt>trilinear</dt> <dd> 
@copybrief Ogre::TFO_TRILINEAR

@copydetails Ogre::TFO_TRILINEAR
</dd> 
<dt>anisotropic</dt> <dd> 
@copybrief Ogre::TFO_ANISOTROPIC

@copydetails Ogre::TFO_ANISOTROPIC
</dd> </dl> 

### Complex Format
This format gives you complete control over the minification, magnification, and mip filters. 

@par
Format: filtering &lt;minFilter&gt; &lt;magFilter&gt; &lt;mipFilter&gt;
@par
Default: filtering linear linear point 

Each parameter can be one of Ogre::FilterOptions without the `FO_` prefix. E.g. `FO_LINEAR` becomes `linear`.

@copydetails Ogre::Sampler::setFiltering(FilterOptions,FilterOptions,FilterOptions)

<a name="max_005fanisotropy"></a><a name="max_005fanisotropy-1"></a>

## max\_anisotropy

@copybrief Ogre::Sampler::setAnisotropy

@par
Format: max\_anisotropy &lt;maxAniso&gt;<br> Default: max\_anisotropy 1

@copydetails Ogre::Sampler::setAnisotropy

<a name="mipmap_005fbias"></a><a name="mipmap_005fbias-1"></a>

## mipmap\_bias

@copydetails Ogre::Sampler::setMipmapBias

@par
Format: mipmap\_bias &lt;value&gt;<br> Default: mipmap\_bias 0

<a name="compare_test"></a>
## compare_test

@copybrief Ogre::Sampler::setCompareEnabled

@copydetails Ogre::Sampler::setCompareEnabled

@par
Format: compare_test on

@par
Default: compare_test off

<a name="comp_func"></a>

## comp_func

The comparison func to use when @c compare_test is enabled
@par
Format: comp_func &lt;func&gt;

@param func one of Ogre::CompareFunction without the `CMPF_` prefix. E.g. `CMPF_LESS_EQUAL` becomes `less_equal`.

# Using GPU Programs in a Pass {#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass}

Within a pass section of a material script, you can reference a vertex, geometry, tessellation, compute, and / or a fragment program which has been defined in @ref High-level-Programs. The programs are defined separately from the usage of them in the pass, since the programs are very likely to be reused between many separate materials, probably across many different .material scripts, so this approach lets you define the program only once and use it many times.

As well as naming the program in question, you can also provide parameters to it. Here’s a simple example:

```cpp
vertex_program_ref myVertexProgram
{
    param_indexed_auto 0 worldviewproj_matrix
    param_indexed      4 float4  10.0 0 0 0
}
```

In this example, we bind a vertex program called ’myVertexProgram’ (which will be defined elsewhere) to the pass, and give it 2 parameters, one is an ’auto’ parameter, meaning we do not have to supply a value as such, just a recognised code (in this case it’s the world/view/projection matrix which is kept up to date automatically by Ogre). The second parameter is a manually specified parameter, a 4-element float. The indexes are described later.

The syntax of the link to a vertex program and a fragment or geometry program are identical, the only difference is that `fragment_program_ref` and `geometry_program_ref` are used respectively instead of `vertex_program_ref`. For tessellation shaders, use `tessellation_hull_program_ref` and `tessellation_domain_program_ref` to link to the hull tessellation program and the domain tessellation program respectively. Compute shader programs can be linked with `compute_program_ref`.

For many situations vertex, geometry and fragment programs are associated with each other in a pass but this is not cast in stone. You could have a vertex program that can be used by several different fragment programs.

Moreover, older APIs permit the use of both fixed pipeline and programmable pipeline (shaders) simultaneously. Specifically, the OpenGL compatibility profile and Direct3D SM2.x allow this.
You can utilize the vertex fixed function pipeline and just provide a @c fragment_program_ref in a pass, with no vertex program reference included. The fragment program must comply with the specified requirements of the related API in order to access the outputs of the vertex fixed pipeline. Alternatively, you can employ a vertex program that directly feeds into the fragment fixed function pipeline.
Most of Ogre's render systems do not support the Fixed Function pipeline. In that case, if you supply vertex shader, you will need to supply a fragment shader as well.

# Adding new Techniques, Passes, to copied materials {#Adding-new-Techniques_002c-Passes_002c-to-copied-materials_003a}

If a new technique or pass needs to be added to a copied material then use a unique name for the technique or pass that does not exist in the parent material. Using an index for the name that is one greater than the last index in the parent will do the same thing. The new technique/pass will be added to the end of the techniques/passes copied from the parent material.

@note if passes or techniques aren’t given a name, they will take on a default name based on their index. For example the first pass has index 0 so its name will be 0.

# Identifying Texture Units to override values {#Identifying-Texture-Units-to-override-values}

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
