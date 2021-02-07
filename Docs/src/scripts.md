# Scripts {#Scripts}

OGRE drives many of its features through scripts in order to make it easier to set up. The scripts are simply plain text files which can be edited in any standard text editor, and modifying them immediately takes effect on your OGRE-based applications, without any need to recompile. This makes prototyping a lot faster. Here are the items that OGRE lets you script:

- @subpage Material-Scripts
- @subpage High-level-Programs
- @subpage Compositor-Scripts
- @subpage Particle-Scripts
- @subpage Overlay-Scripts
- @subpage Font-Definition-Scripts

@tableofcontents

# Loading scripts

Scripts are loaded when resource groups are initialised: OGRE looks in all resource locations associated with the group (see Ogre::ResourceGroupManager::addResourceLocation) for files with the respective extension (e.g. ’.material’, ’.compositor’, ..) and parses them. If you want to parse files manually, use Ogre::ScriptCompilerManager::parseScript.

The file extension does not actually restrict the items that can be specified inside the file; e.g. %Ogre is perfectly fine with loading a particle-system from a ’.compositor’ file - but it will lead you straight to maintenance-hell if you do that.
The extensions, however, do specify the order in which the scripts are parsed, which is as follows:

1. "*.program"
2. "*.material"
3. "*.particle"
4. "*.compositor"
5. "*.os"

# Format {#Format}

Several script objects may be defined in a single file. The script format is pseudo-C++, with sections delimited by curly braces ({}), and comments indicated by starting a line with ’//’. The general format is shown below:

```cpp
// This is a comment
object_keyword Example/ObjectName
{
    attribute_name "some value"

    object_keyword2 "Nested Object"
    {
        other_attribute 1 2 3
        // and so on..
    }
}
```

Every script object must be given a name, which is the line ’object_keyword &lt;name&gt;’ before the first opening ’{’. This name must be globally unique. It can include path characters (as in the example) to logically divide up the objects, and also to avoid duplicate names, but the engine does not treat the name as hierarchical, just as a string. Names can include spaces but must be surrounded by double quotes i.e. `compositor "My Name"`.

@note ’:’ is the delimiter for specifying inheritance in the script so it can’t be used as part of the name.

An script object can inherit from a previously defined object by using a *colon* ’:’ after the name followed by the name of the reference object to inherit from. You can in fact even inherit just *parts* of a script object from others; all this is covered in @ref Script-Inheritance). You can also use variables in your script which can be replaced in inheriting versions, see @ref Script-Variables.

## Script Inheritance {#Script-Inheritance}

When creating new script objects that are only slight variations of another object, it’s good to avoid copying and pasting between scripts. Script inheritance lets you do this; in this section we’ll use material scripts as an example, but this applies to all scripts parsed with the script compilers in %Ogre 1.6 onwards.

For example, to make a new material that is based on one previously defined, add a *colon* ’:’ after the new material name followed by the name of the material that is to be copied.

Example
```cpp
material <NewUniqueChildName> : <ReferenceParentMaterial>
```

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed. The easiest way to achieve this is to either place parents at the beginning of the material script file, or to use the @ref Script-Import-Directive. Note that inheritance is actually a copy - after scripts are loaded into Ogre, objects no longer maintain their copy inheritance structure. If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier. Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them, Techniques and passes can take a name and texture units can be numbered within the material script. You can also use variables, See @ref Script-Variables.

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
  technique
  {
    pass "Fifth Pass"
    {
      ambient 0.5 0.7 0.3 1.0
    }
  }
}
```

The parent pass name must be known and the pass must be in the correct technique in order for this to work correctly. Specifying the technique name and the pass name is the best method.

### Advanced Script Inheritance {#Advanced-Script-Inheritance}

Script objects can also inherit from each other more generally. The previous concept of inheritance, material copying, was restricted only to the top-level material objects. Now, any level of object can take advantage of inheritance (for instance, techniques, passes, and compositor targets).

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

@note Importing does not actually cause objects in the imported script to be fully parsed & created, it just makes the definitions available for inheritance. This has a specific ramification for vertex / fragment program definitions, which must be loaded before any parameters can be specified. You should continue to put common program definitions in .program files to ensure they are fully parsed before being referenced in multiple .material files. The ’import’ command just makes sure you can resolve dependencies between equivalent script definitions (e.g. material to material).

# Custom Translators {#custom-translators}
Writing a custom translators allows you to extend Ogre's standard compilers with completely new functionality. The same scripting interfaces can be used to define application-specific functionality. Here's how you do it.

The first step is creating a custom translator class which extends Ogre::ScriptTranslator.

@snippet Components/Overlay/src/OgreOverlayTranslator.h font_translator

This class defines the important function to override: translate. This is called when the TestTranslator needs to process a sub-set of the parsed script. The definition of this function might look something like this:

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_translate

The translate function here expects all children to be atomic properties. Sub-objects can also be processed by checking if the child node type is Ogre::ANT_OBJECT.

From here you need to register the translator to be invoked when the proper object is found in the scripts. To do this we need to create a Ogre::ScriptTranslatorManager object to create your custom translator. The relevant parts look like this:

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_register

Note how we use Ogre::ScriptCompilerManager::registerCustomWordId to avoid string comparisons in our code.

@snippet Components/Overlay/src/OgreOverlayTranslator.cpp font_get_translator

No new translators are created here, just returned when requested. This is because our translator does not require separate instances to properly parse scripts, and so it is easier to only create one instance and reuse it. Often this strategy will work.

The order that custom translator managers are registered will make a difference. When the system is attempting to find translators to handle pieces of a script, it will query the translator managers one-by-one until it finds one that handles that script object. It is a first-come-first-served basis.

An important note is that this will recognize the above pattern no matter where in the script it is. That means that this may appear at the top-level of a script or inside several sub-objects. If this is not what you want then you can change the translator manager to do more advanced processing in the getTranslator function.

@page Compositor-Scripts Compositor Scripts

The compositor framework is a subsection of the OGRE API that allows you to easily define how to assemble the final image. A typical use-case are full screen post-processing effects - however Compositors are not limited to that. Compositor scripts offer you the ability to define rendering pipelines in a script which can be reused and modified easily, rather than having to use the API to define them. You still need to use code to instantiate a compositor against one of your visible viewports, but this is a much simpler process than actually defining the compositor itself.

@tableofcontents

# Compositor Fundamentals {#Compositor-Fundamentals}

Compositing generally involves rendering the scene to a texture, either in addition to or instead of the main window. Once the scene is in a texture, you can then pull the scene image into a fragment program and perform operations on it by rendering it through full screen quad. The target of this post processing render can be the main result (e.g. a window), or it can be another render texture so that you can perform multi-stage convolutions on the image. You can even ’ping-pong’ the render back and forth between a couple of render textures to perform convolutions which require many iterations, without using a separate texture for each stage. Eventually you’ll want to render the result to the final output, which you do with a full screen quad. This might replace the whole window (thus the main window doesn’t need to render the scene itself), or it might be a combinational effect. 

So that we can discuss how to implement these techniques efficiently, a number of definitions are required:

<dl compact="compact">
<dt>Compositor</dt> <dd>

Definition of a rendering pipeline that can be applied to a user viewport. This is what you’re defining when writing compositor scripts as detailed in this section.

</dd> <dt>Compositor Instance</dt> <dd>

An instance of a compositor as applied to a single viewport. You create these based on compositor definitions, See @ref Applying-a-Compositor.

</dd> <dt>Compositor Chain</dt> <dd>

It is possible to enable more than one compositor instance on a viewport at the same time, with one compositor taking the results of the previous one as input. This is known as a compositor chain. Every viewport which has at least one compositor attached to it has a compositor chain. See @ref Applying-a-Compositor

</dd> <dt>Target</dt> <dd>

This is a Ogre::RenderTarget, i.e. the place where the result of a series of render operations is sent. A target may be the final output (and this is implicit, you don’t have to declare it), or it may be an intermediate render texture, which you declare in your script with the [texture line](#compositor-texture). A target which is not the output target has a defined size and pixel format which you can control.

</dd> <dt>Output Target</dt> <dd>

As Target, but this is the single final result of all operations. The size and pixel format of this target cannot be controlled by the compositor since it is defined by the application using it, thus you don’t declare it in your script. However, you do declare a Target Section for it, see below.

</dd> <dt>Target Section</dt> <dd>

A Target may be rendered to many times in the course of a composition effect. In particular if you ’ping pong’ a convolution between a couple of textures, you will have more than one Target Sections per Target. Target Sections are declared in the script using a [target or target\_output line](#Compositor-Target-Passes), the latter being the final output of which there can be only one.
@note Internally this is referred to as Ogre::CompositionTargetPass

</dd> <dt>Pass</dt> <dd>

Within a Target Section, there are one or more individual @ref Compositor-Passes, which perform a very specific action, such as rendering the original scene (or pulling the result from the previous compositor in the chain), rendering a fullscreen quad, or clearing one or more buffers. Typically within a single target section you will use the either a `render_scene` pass or a `render_quad` pass, not both. Clear can be used with either type.

</dd> </dl>

@snippet Samples/Media/materials/scripts/Examples.compositor manual_sample

The major components of a compositor are the @ref Compositor-Techniques, the @ref Compositor-Target-Passes and the @ref Compositor-Passes, which are covered in detail in the following sections.

# Techniques {#Compositor-Techniques}

A compositor technique is much like a [material technique](@ref Techniques) in that it describes one approach to achieving the effect you’re looking for. A compositor definition can have more than one technique if you wish to provide some fallback should the hardware not support the technique you’d prefer to use. Techniques are evaluated for hardware support based on 2 things:

<dl compact="compact">
<dt>Material support</dt> <dd>

All @ref Compositor-Passes that render a fullscreen quad use a material; for the technique to be supported, all of the materials referenced must have at least one supported material technique. If they don’t, the compositor technique is marked as unsupported and won’t be used.

</dd> <dt>Texture format support</dt> <dd>

This one is slightly more complicated. When you request a @ref compositor-texture) in your technique, you request a pixel format. Not all formats are natively supported by hardware, especially the floating point formats. However, in this case the hardware will typically downgrade the texture format requested to one that the hardware does support - with compositor effects though, you might want to use a different approach if this is the case. So, when evaluating techniques, the compositor will first look for native support for the exact pixel format you’ve asked for, and will skip onto the next technique if it is not supported, thus allowing you to define other techniques with simpler pixel formats which use a different approach. If it doesn’t find any techniques which are natively supported, it tries again, this time allowing the hardware to downgrade the texture format and thus should find at least some support for what you’ve asked for.

</dd> </dl>

As with material techniques, compositor techniques are evaluated in the order you define them in the script, so techniques declared first are preferred over those declared later.

@par
Format: technique { }

Techniques can have the following nested elements:

-   [texture](#compositor-texture)
-   [texture\_ref](#compositor_005ftexture_005fref)
-   [scheme](#compositor_005fscheme)
-   [compositor\_logic](#compositor_005flogic)
-   [target](#Compositor-Target-Passes)
-   [target\_output](#Compositor-Target-Passes)

<a name="compositor_005ftexture"></a><a name="texture-2"></a>

## texture {#compositor-texture}

This declares a render texture for use in subsequent @ref Compositor-Target-Passes.
@par
Format: texture &lt;Name&gt; &lt;Width&gt; &lt;Height&gt; &lt;Pixel_Format&gt; \[&lt;MRT Pixel_Format2&gt;\] \[&lt;MRT Pixel_FormatN&gt;\] \[pooled\] \[gamma\] \[no\_fsaa\] \[depth\_pool &lt;poolId&gt;\] \[&lt;scope&gt;\]

@param Name
A name to give the render texture, which must be unique within this compositor. This name is used to reference the texture in @ref Compositor-Target-Passes, when the texture is rendered to, and in @ref Compositor-Passes, when the texture is used as input to a material rendering a fullscreen quad.

@param Width
@param Height 
@parblock
The dimensions of the render texture. You can either specify a fixed width and height, or you can request that the texture is based on the physical dimensions of the viewport to which the compositor is attached. The options for the latter are either of
- @c target_width and @c target_height
- @c target_width_scaled &lt;factor&gt; and @c target_height_scaled &lt;factor&gt;

where ’factor’ is the amount by which you wish to multiply the size of the main target to derive the dimensions.
@endparblock
@param Pixel_Format
The pixel format of the render texture. This affects how much memory it will take, what colour channels will be available, and what precision you will have within those channels.
See Ogre::PixelFormat. You can in fact repeat this element if you wish. If you do so, that means that this render texture becomes a Multiple Render Target (MRT), when the GPU writes to multiple textures at once.

@param pooled
If present, this directive makes this texture ’pooled’ among compositor instances, which can save some memory.

@param gamma
If present, this directive means that sRGB gamma correction will be enabled on writes to this texture. You should remember to include the opposite sRGB conversion when you read this texture back in another material, such as a quad. This option will automatically enabled if you use a render\_scene pass on this texture and the viewport on which the compositor is based has sRGB write support enabled.

@param no\_fsaa
If present, this directive disables the use of anti-aliasing on this texture. FSAA is only used if this texture is subject to a render\_scene pass and FSAA was enabled on the original viewport on which this compositor is based; this option allows you to override it and disable the FSAA if you wish.

@param depth\_pool
When present, this directive has to be followed by an integer. This directive is unrelated to the "pooled" directive. This one sets from which Depth buffer pool the depth buffer will be chosen from. All RTs from all compositors (including render windows if the render system API allows it) with the same pool ID share the same depth buffers (following the rules of the current render system APIs, (check RenderSystemCapabilities flags to find the rules). When the pool ID is 0, no depth buffer is used. This can be helpful for passes that don’t require a Depth buffer at all, potentially saving performance and memory. Default value is 1.
Ignored with depth pixel formats.

@param scope
If present, this directive sets the scope for the texture for being accessed by other compositors using the [texture\_ref](#compositor_005ftexture_005fref) directive. There are three options : 
1. @c local_scope (which is also the default) means that only the compositor defining the texture can access it. 
2. @c chain_scope means that the compositors after this compositor in the chain can reference its textures, and 
3. @c global_scope means that the entire application can access the texture. This directive also affects the creation of the textures (global textures are created once and thus can’t be used with the pooled directive, and can’t rely on viewport size).

@par
Example: texture rt0 512 512 PF\_R8G8B8A8
@par
Example: texture rt1 target\_width target\_height PF\_FLOAT32\_RGB
@par
Example : texture mrt\_output target\_width target\_height PF\_FLOAT16\_RGBA PF\_FLOAT16\_RGBA chain\_scope

@note
It is imperative that if you use MRT that the shaders that render to it render to ALL the targets. Not doing so can cause undefined results. It is also important to note that although you can use different pixel formats for each target in a MRT, each one should have the same total bit depth since most cards do not support independent bit depths. If you try to use this feature on cards that do not support the number of MRTs you’ve asked for, the technique will be skipped (so you ought to write a fallback technique).


<a name="compositor_005ftexture_005fref"></a><a name="texture_005fref"></a>

## texture\_ref

This declares a reference of a texture from another compositor to be used in this compositor.

@par
Format: texture\_ref &lt;Local_Name&gt; &lt;Reference_Compositor&gt; &lt;Reference_Texture_Name&gt;

@param Local_Name
A name to give the referenced texture, which must be unique within this compositor. This name is used to reference the texture in @ref Compositor-Target-Passes, when the texture is rendered to, and in @ref Compositor-Passes, when the texture is used as input to a material rendering a fullscreen quad.

@param Reference_Compositor
The name of the compositor that we are referencing a texture from

@param Reference_Texture_Name
The name of the texture in the compositor that we are referencing

Make sure that the texture being referenced is scoped accordingly (either chain or global scope) and placed accordingly during chain creation (if referencing a chain-scoped texture, the compositor must be present in the chain and placed before the compositor referencing it).

@par
Example : texture\_ref GBuffer GBufferCompositor mrt\_output

<a name="compositor_005fscheme"></a><a name="scheme-2"></a>

## scheme

This gives a compositor technique a scheme name, allowing you to manually switch between different techniques for this compositor when instantiated on a viewport by calling Ogre::CompositorInstance::setScheme.

@par
Format: scheme &lt;Name&gt;

<a name="compositor_005flogic"></a><a name="compositor_005flogic-1"></a>

## compositor\_logic

This connects between a compositor and code that it requires in order to function correctly. When an instance of this compositor will be created, the compositor logic will be notified and will have the chance to prepare the compositor’s operation (for example, adding a listener).

@par
Format: compositor\_logic &lt;Name&gt;

Registration of compositor logics is done by name through Ogre::CompositorManager::registerCompositorLogic.

# Target Sections {#Compositor-Target-Passes}

A target section defines the rendering of either a render texture or the final output. You can update the same target multiple times by adding more than one target section to your compositor script - this is very useful for ’ping pong’ renders between a couple of render textures to perform complex convolutions that cannot be done in a single render, such as blurring.

There are two types of target sections, the sort that updates a render texture

@par
Format: target &lt;Name&gt; { }

and the sort that defines the final output render

@par
Format: target\_output { }

The contents of both are identical, the only real difference is that you can only have a single target\_output entry, whilst you can have many target entries. 

Here are the attributes you can use in a ’target’ or ’target\_output’ section of a .compositor script:

-   [input](#compositor_005ftarget_005finput)
-   [only\_initial](#only_005finitial)
-   [visibility\_mask](#visibility_005fmask)
-   [lod\_bias](#compositor_005flod_005fbias)
-   [material_scheme](#material_005fscheme)
-   [shadows](#compositor_005fshadows)

<a name="Attribute-Descriptions-2"></a>

## Attribute Descriptions

<a name="compositor_005ftarget_005finput"></a><a name="input"></a>

## input

Sets input mode of the target, which tells the target pass what is pulled in before any of its own passes are rendered.

@par
Format: input (none | previous)
@par
Default: input none

@param none
The target will have nothing as input, all the contents of the target must be generated using its own passes. Note this does not mean the target will be empty, just no data will be pulled in. For it to truly be blank you’d need a ’clear’ pass within this target.

@param previous
The target will pull in the previous contents of the viewport. This will be either the original scene if this is the first compositor in the chain, or it will be the output from the previous compositor in the chain if the viewport has multiple compositors enabled.

</dd> </dl> <a name="only_005finitial"></a><a name="only_005finitial-1"></a>

## only\_initial

If set to on, this target pass will only execute once initially after the effect has been enabled. This could be useful to perform once-off renders, after which the static contents are used by the rest of the compositor.

@par
Format: only\_initial (on | off) 
@par
Default: only\_initial off

<a name="visibility_005fmask"></a><a name="visibility_005fmask-1"></a>

## visibility\_mask

Sets the visibility mask for any render\_scene passes performed in this target pass. This is a bitmask (although it must be specified as decimal, not hex) and maps to Ogre::Viewport::setVisibilityMask.
@par
Format: visibility\_mask &lt;mask&gt;
@par
Default: visibility\_mask 4294967295

<a name="compositor_005flod_005fbias"></a><a name="lod_005fbias"></a>

## lod\_bias

Set the scene LOD bias for any render\_scene passes performed in this target pass. The default is 1.0, everything below that means lower quality, higher means higher quality.
@par
Format: lod\_bias &lt;lodbias&gt;
@par
Default: lod\_bias 1.0

<a name="compositor_005fshadows"></a><a name="shadows"></a>

## shadows

Sets whether shadows should be rendered during any render\_scene pass performed in this target pass.

@par
Format: shadows (on | off)
@par
Default: shadows on

<a name="material_005fscheme"></a><a name="material_005fscheme-1"></a>

## material\_scheme

If set, indicates the material scheme to use for any render\_scene pass. Useful for performing special-case rendering effects.

@par
Format: material\_scheme &lt;scheme name&gt;
@par
Default: None

# Passes {#Compositor-Passes}

A pass is a single rendering action to be performed in a target section.
@par
Format: pass &lt;type&gt; \[custom name\] { }

There are the following types of a pass:

<dl compact="compact">
<dt>clear</dt> <dd>

This kind of pass sets the contents of one or more buffers in the target to a fixed value. So this could clear the colour buffer to a fixed colour, set the depth buffer to a certain set of contents, fill the stencil buffer with a value, or any combination of the above.

</dd> <dt>stencil</dt> <dd>

This kind of pass configures stencil operations for the subsequent passes. It can set the stencil compare function, operations and reference values for you to perform your own stencil effects.

</dd> <dt>render\_scene</dt> <dd>

This kind of pass performs a regular rendering of the scene. It will use the [visibility\_mask](#visibility_005fmask), [lod\_bias](#compositor_005flod_005fbias), and [material\_scheme](#material_005fscheme) from the parent target pass.

</dd> <dt>render\_quad</dt> <dd>

This kind of pass renders a quad over the entire render target, using a given material. You will undoubtedly want to pull in the results of other target passes into this operation to perform fullscreen effects.

</dd> <dt>compute</dt> <dd>

This kind of a pass dispatches a compute shader as attached to the given material. Compute shaders are independent from normal rendering pipeline as triggered by `render_scene` or `render_quad`. They do not have any predefined input/ outputs but rather read/ write to any buffers you attach to them.

</dd> <dt>render\_custom</dt> <dd>

This kind of pass is just a callback to user code for the composition pass specified in the custom name (and registered via Ogre::CompositorManager::registerCustomCompositionPass) and allows the user to create custom render operations for more advanced effects. This is the only pass type that requires the custom name parameter.

</dd> </dl>

Here are the attributes you can use in a ’pass’ section of a .compositor script:

<a name="Available-Pass-Attributes"></a>

## Available Pass Attributes

-   [material](#material)
-   [input](#compositor_005fpass_005finput)
-   [identifier](#compositor_005fpass_005fidentifier)
-   [first\_render\_queue](#first_005frender_005fqueue)
-   [last\_render\_queue](#last_005frender_005fqueue)
-   [thread_groups](#thread_groups)
-   [material\_scheme](#compositor_005fpass_005fmaterial_005fscheme)
-   [quad_normals](#quad_normals)

<a name="material"></a><a name="material-1"></a>

## material

For passes of type `render_quad` and `compute`, sets the material to be used. With `compute` passes only the compute shader is used and pnly global auto parameter can be accessed.
For `render_quad` you will want to use shaders in this material to perform fullscreen effects, and use the [input](#compositor_005fpass_005finput) attribute to map other texture targets into the texture bindings needed by this material. 

@par
Format: material &lt;Name&gt;

<a name="compositor_005fpass_005finput"></a><a name="input-1"></a>

## input

For passes of type `render_quad` and `compute`, this is how you map one or more local @ref compositor-texture into the material you’re using to render. To bind more than one texture, repeat this attribute with different texUnit indices.

@par
Format: input &lt;texUnit&gt; &lt;name&gt; \[&lt;mrtIndex&gt;\]

@param texUnit
The index of the target texture unit, must be a number in the range \[0, OGRE\_MAX\_TEXTURE\_LAYERS-1\].
@param name
The name of the local render texture to bind, as declared by @ref compositor-texture and rendered to in one or more @ref Compositor-Target-Passes.
@param mrtIndex
If the local texture that you’re referencing is a Multiple Render Target (MRT), this identifies the surface from the MRT that you wish to reference (0 is the first surface, 1 the second etc).

@par
Example: input 0 rt0

<a name="compositor_005fpass_005fidentifier"></a><a name="identifier"></a>

## identifier

Associates a numeric identifier with a pass involving a material (like render_quad). This is useful for registering a listener with Ogre::CompositorInstance::addListener, and being able to identify which pass it is that’s being processed, so that material parameters can be varied. Numbers between 0 and 2^32 - 1 are allowed.

@par
Format: identifier &lt;number&gt; 
@par
Example: identifier 99945
@par
Default: identifier 0

<a name="first_005frender_005fqueue"></a><a name="first_005frender_005fqueue-1"></a>

## first\_render\_queue

For passes of type ’render\_scene’, this sets the first render queue id that is included in the render. Defaults to the value of Ogre::RENDER_QUEUE_BACKGROUND.
@par
Format: first\_render\_queue &lt;id&gt; 
@par
Default: first\_render\_queue 0

<a name="last_005frender_005fqueue"></a><a name="last_005frender_005fqueue-1"></a>

## last\_render\_queue

For passes of type ’render\_scene’, this sets the last render queue id that is included in the render. Defaults to the value of Ogre::RENDER_QUEUE_SKIES_LATE.
@par
Format: last\_render\_queue &lt;id&gt; 
@par
Default: last\_render\_queue 95

<a name="thread_groups"></a>

## thread_groups

Passes of type `compute` operate on an abstract "compute space". This space is typically divided into threads and thread groups (work groups). The size of a thread group is defined inside the compute shader itself. This defines how many groups should be launched.

@par
Example: if you want to process a 256x256px image and have a thread group size of 16x16x1, you want to specify `16 16 1` here as well.

@par
Format: thread_groups &lt;groups_x&gt; &lt;groups_y&gt; &lt;groups_z&gt;

<a name="compositor_005fpass_005fmaterial_005fscheme"></a><a name="material_005fscheme-2"></a>

## material\_scheme

If set, indicates the material scheme to use for this pass only. Useful for performing special-case rendering effects. This will overwrite any scheme set in the parent @ref Compositor-Target-Passes.
@par
Format: material\_scheme &lt;scheme name&gt; 
@par
Default: None

<a name="quad_normals"></a>

## quad_normals

Pass the camera Frustum far corner vectors in the quad normals for passes of type `quad`. This is particularly useful for efficiently reconstructing position using only the depth and the corners.

@par
Format: quad_normals &lt;camera_far_corners_world_space|camera_far_corners_view_space&gt;
@par
Default: None

## clear {#Clear-Section}

For passes of type ’clear’, this section defines the buffer clearing parameters.  

@par
Format: pass clear { }

Here are the attributes you can use in a ’clear’ section of a .compositor script:

-   [buffers](#compositor_005fclear_005fbuffers)
-   [colour\_value](#compositor_005fclear_005fcolour_005fvalue)
-   [depth\_value](#compositor_005fclear_005fdepth_005fvalue)
-   [stencil\_value](#compositor_005fclear_005fstencil_005fvalue) <a name="compositor_005fclear_005fbuffers"></a><a name="buffers"></a>

    ## buffers

    Sets the buffers cleared by this pass.

    @par
    Format: buffers \[colour\] \[depth\] \[stencil\] 
    @par
    Default: buffers colour depth

    <a name="compositor_005fclear_005fcolour_005fvalue"></a><a name="colour_005fvalue"></a>

    ## colour\_value

    Set the colour used to fill the colour buffer by this pass, if the colour buffer is being cleared
    @par
    Format: colour\_value (&lt;red&gt; &lt;green&gt; &lt;blue&gt; &lt;alpha&gt; | auto)
    @par
    Default: colour\_value 0 0 0 0

    If set to `auto` the background colour of the viewport is used, to which the parent compositor is attached to.

    <a name="compositor_005fclear_005fdepth_005fvalue"></a><a name="depth_005fvalue"></a>

    ## depth\_value

    Set the depth value used to fill the depth buffer by this pass, if the depth buffer is being cleared
    @par
    Format: depth\_value &lt;depth&gt; 
    @par
    Default: depth\_value 1.0

    <a name="compositor_005fclear_005fstencil_005fvalue"></a><a name="stencil_005fvalue"></a>

    ## stencil\_value

    Set the stencil value used to fill the stencil buffer by this pass, if the stencil buffer is being cleared
    @par
    Format: stencil\_value &lt;value&gt; 
    @par
    Default: stencil\_value 0.0

## stencil {#Stencil-Section}

For passes of type ’stencil’, this section defines the stencil operation parameters. 

@par
Format: pass stencil { }

@copydetails Ogre::StencilState

Here are the attributes you can use in a ’stencil’ section of a .compositor script:

-   [check](#compositor_005fstencil_005fcheck)
-   [comp\_func](#compositor_005fstencil_005fcomp_005ffunc)
-   [ref\_value](#compositor_005fstencil_005fref_005fvalue)
-   [mask](#compositor_005fstencil_005fmask)
-   [fail\_op](#compositor_005fstencil_005ffail_005fop)
-   [depth\_fail\_op](#compositor_005fstencil_005fdepth_005ffail_005fop)
-   [pass\_op](#compositor_005fstencil_005fpass_005fop)
-   [two\_sided](#compositor_005fstencil_005ftwo_005fsided) <a name="compositor_005fstencil_005fcheck"></a><a name="check"></a>

    ## check

    Enables or disables the stencil check, thus enabling the use of the rest of the features in this section. The rest of the options in this section do nothing if the stencil check is off. 
    @par
    Format: check (on | off)

    <a name="compositor_005fstencil_005fcomp_005ffunc"></a><a name="comp_005ffunc"></a>

    ## comp\_func

    Sets the function used to perform the stencil comparison.

    @par
    Format: comp\_func (always\_fail | always\_pass | less | less\_equal | not\_equal | greater\_equal | greater)
    @par
    Default: comp\_func always\_pass

    <a name="compositor_005fstencil_005fref_005fvalue"></a><a name="ref_005fvalue"></a>

    ## ref\_value

    Sets the reference value used to compare with the stencil buffer as described in [comp\_func](#compositor_005fstencil_005fcomp_005ffunc). 
    @par
    Format: ref\_value &lt;value&gt; 
    @par
    Default: ref\_value 0.0

    <a name="compositor_005fstencil_005fmask"></a><a name="mask"></a>

    ## mask

    Sets the mask used to compare with the stencil buffer as described in [comp\_func](#compositor_005fstencil_005fcomp_005ffunc). 
    @par
    Format: mask &lt;value&gt; 
    @par
    Default: mask 4294967295

    <a name="compositor_005fstencil_005ffail_005fop"></a><a name="fail_005fop"></a>

    ## fail\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) and depth comparison is that both fail. 
    @par
    Format: fail\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert)
    @par
    Default: depth\_fail\_op keep These actions mean:

    <dl compact="compact">
    <dt>keep</dt> <dd>

    Leave the stencil buffer unchanged.

    </dd> <dt>zero</dt> <dd>

    Set the stencil value to zero.

    </dd> <dt>replace</dt> <dd>

    Set the stencil value to the reference value.

    </dd> <dt>increment</dt> <dd>

    Add one to the stencil value, clamping at the maximum value.

    </dd> <dt>decrement</dt> <dd>

    Subtract one from the stencil value, clamping at 0.

    </dd> <dt>increment\_wrap</dt> <dd>

    Add one to the stencil value, wrapping back to 0 at the maximum.

    </dd> <dt>decrement\_wrap</dt> <dd>

    Subtract one from the stencil value, wrapping to the maximum below 0.

    </dd> <dt>invert</dt> <dd>

    invert the stencil value.

    </dd> </dl> <a name="compositor_005fstencil_005fdepth_005ffail_005fop"></a><a name="depth_005ffail_005fop"></a>

    ## depth\_fail\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) passes but the depth comparison fails. 

    @par
    Format: depth\_fail\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert)
    @par
    Default: depth\_fail\_op keep

    <a name="compositor_005fstencil_005fpass_005fop"></a><a name="pass_005fop"></a>

    ## pass\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) and the depth comparison pass.  
    @par
    Format: pass\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert)
    @par
    Default: pass\_op keep

    <a name="compositor_005fstencil_005ftwo_005fsided"></a><a name="two_005fsided"></a>

    ## two\_sided

    Enables or disables two-sided stencil operations, which means the inverse of the operations applies to back-facing polygons.
    @par
    Format: two\_sided (on | off)
    @par
    Default: two\_sided off


# Applying a Compositor {#Applying-a-Compositor}

Adding a compositor instance to a viewport is very simple. All you need to do is:

```cpp
Ogre::CompositorManager::getSingleton().addCompositor(viewport, compositorName);
```



Where viewport is a pointer to your viewport, and compositorName is the name of the compositor to create an instance of. By doing this, a new instance of a compositor will be added to a new compositor chain on that viewport. You can call the method multiple times to add further compositors to the chain on this viewport. By default, each compositor which is added is disabled, but you can change this state by calling:

```cpp
Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, compositorName, enabledOrDisabled);
```

For more information on defining and using compositors, see Demo\_Compositor in the Samples area, together with the Examples.compositor script in the media area.

@page Overlay-Scripts Overlay Scripts

Overlay scripts offer you the ability to define overlays in a script which can be reused easily. Whilst you could set up all overlays for a scene in code using the methods of the SceneManager, Overlay and OverlayElement classes, in practice it’s a bit unwieldy. Instead you can store overlay definitions in text files which can then be loaded whenever required.

@tableofcontents

```cpp
// The name of the overlay comes first
overlay MyOverlays/ANewOverlay
{
    zorder 200

    overlay_element MyOverlayElements/TestPanel Panel
    {
        // Center it horizontally, put it at the top
        left 0.25
        top 0
        width 0.5
        height 0.1
        material MyMaterials/APanelMaterial

        // Another panel nested in this one
        overlay_element MyOverlayElements/AnotherPanel Panel
        {
             left 0
             top 0
             width 0.1
             height 0.1
             material MyMaterials/NestedPanel
        }
    }
}
```

The above example defines a single overlay called ’MyOverlays/ANewOverlay’, with 2 panels in it, one nested under the other. It uses relative metrics (the default if no metrics\_mode option is found).

The overlay itself only has a single property ’zorder’ which determines how ’high’ it is in the stack of overlays if more than one is displayed at the same time. Overlays with higher zorder values are displayed on top.

# Adding elements to the overlay {#Adding-elements-to-the-overlay}

Within an overlay, you can include any number of 2D or 3D elements. You do this by defining nested ’overlay_element’ blocks.

@note Top level overlay components must derive from Ogre::OverlayContainer - e.g. you must place @ref TextArea into a @ref Panel component to be able to add it to the overlay.

## ’overlay_element’ blocks

These are delimited by curly braces. The format for the header preceding the first brace is:

@par
Format: overlay_element &lt;instance\_name&gt; &lt;type\_name&gt; \[: &lt;template\_name&gt;\]

@param type_name
Must resolve to the name of a Ogre::OverlayElement type which has been registered with the Ogre::OverlayManager. Plugins register with the OverlayManager to advertise their ability to create elements, and at this time advertise the name of the type. OGRE comes preconfigured with types @ref Panel, @ref BorderPanel and @ref TextArea.

@param instance_name
Must be a name unique among all other elements / containers by which to identify the element. Note that you can obtain a pointer to any named element by calling OverlayManager::getSingleton().getOverlayElement(name).

@param template_name Optional template on which to base this item. See @ref Templates.


The properties which can be included within the braces depend on the custom type. However the following are always valid:

-   [metrics\_mode](#metrics_005fmode)
-   [horz\_align](#horz_005falign)
-   [vert\_align](#vert_005falign)
-   [left](#left)
-   [top](#overlaytopelement)
-   [width](#width)
-   [height](#height)
-   [material](#overlay_005fmaterial)
-   [caption](#caption)

# Templates {#Templates}

You can use templates to create numerous elements with the same properties. A template is an abstract element and it is not added to an overlay. It acts as a base class that elements can inherit and get its default properties. The template element is created in the topmost scope - it is NOT specified in an Overlay. It is recommended that you define templates in a separate overlay though this is not essential. Having templates defined in a separate file will allow different look & feels to be easily substituted.

Elements can inherit a template in a similar way to C++ inheritance - by using the : operator on the element definition. The : operator is placed after the closing bracket of the name (separated by a space). The name of the template to inherit is then placed after the : operator (also separated by a space).

A template can contain template children which are created when the template is subclassed and instantiated.

```cpp
overlay_element MyTemplates/BasicBorderPanel BorderPanel
{
    left 0
    top 0
    width 1
    height 1

// setup the texture UVs for a borderpanel

// do this in a template so it doesn't need to be redone everywhere
    material Core/StatsBlockCenter
    border_size 0.05 0.05 0.06665 0.06665
    border_material Core/StatsBlockBorder
    border_topleft_uv 0.0000 1.0000 0.1914 0.7969
    border_top_uv 0.1914 1.0000 0.8086 0.7969
    border_topright_uv 0.8086 1.0000 1.0000 0.7969
    border_left_uv 0.0000 0.7969 0.1914 0.2148
    border_right_uv 0.8086 0.7969 1.0000 0.2148
    border_bottomleft_uv 0.0000 0.2148 0.1914 0.0000
    border_bottom_uv 0.1914 0.2148 0.8086 0.0000
    border_bottomright_uv 0.8086 0.2148 1.0000 0.0000
}
overlay_element MyTemplates/BasicButton Button : MyTemplates/BasicBorderPanel
{
    left 0.82
    top 0.45
    width 0.16
    height 0.13
    material Core/StatsBlockCenter
    border_up_material Core/StatsBlockBorder/Up
    border_down_material Core/StatsBlockBorder/Down
}
overlay_element MyTemplates/BasicText TextArea
{
    font_name Ogre
    char_height 0.08
    colour_top 1 1 0
    colour_bottom 1 0.2 0.2
    left 0.03
    top 0.02
    width 0.12
    height 0.09
}

overlay MyOverlays/AnotherOverlay
{
    zorder 490
    overlay_element MyElements/BackPanel BorderPanel : MyTemplates/BasicBorderPanel
    {
        left 0
        top 0
        width 1
        height 1

        overlay_element MyElements/HostButton Button : MyTemplates/BasicButton
        {
            left 0.82
            top 0.45
            caption MyTemplates/BasicText HOST
        }

        overlay_element MyElements/JoinButton Button : MyTemplates/BasicButton
        {
            left 0.82
            top 0.60
            caption MyTemplates/BasicText JOIN
        }
    }
}
```

The above example uses templates to define a button. Note that the button template inherits from the borderPanel template. This reduces the number of attributes needed to instantiate a button.

Also note that the instantiate of a Button needs a template name for the caption attribute. So templates can also be used by elements that need dynamic creation of children elements (the button creates a TextAreaElement in this case for its caption).

See [OverlayElement Attributes](#OverlayElement-Attributes), [Standard OverlayElements](#Standard-OverlayElements)



<a name="OverlayElement-Attributes"></a> <a name="OverlayElement-Attributes-1"></a>

## OverlayElement Attributes

These attributes are valid within the braces of a ’container’ or ’element’ block in an overlay script. They must each be on their own line. Ordering is unimportant.

<a name="metrics_005fmode"></a><a name="metrics_005fmode-1"></a>

## metrics\_mode

Sets the units which will be used to size and position this element.

@par
Format: metrics\_mode &lt;pixels|relative&gt;<br> Example: metrics\_mode pixels<br>

This can be used to change the way that all measurement attributes in the rest of this element are interpreted. In relative mode, they are interpreted as being a parametric value from 0 to 1, as a proportion of the width / height of the screen. In pixels mode, they are simply pixel offsets.
@par
Default: metrics\_mode relative<br>

<a name="horz_005falign"></a><a name="horz_005falign-1"></a>

## horz\_align

Sets the horizontal alignment of this element, in terms of where the horizontal origin is.
@par
Format: horz\_align &lt;left|center|right&gt;<br> Example: horz\_align center

This can be used to change where the origin is deemed to be for the purposes of any horizontal positioning attributes of this element. By default the origin is deemed to be the left edge of the screen, but if you change this you can center or right-align your elements. Note that setting the alignment to center or right does not automatically force your elements to appear in the center or the right edge, you just have to treat that point as the origin and adjust your coordinates appropriately. This is more flexible because you can choose to position your element anywhere relative to that origin. For example, if your element was 10 pixels wide, you would use a ’left’ property of -10 to align it exactly to the right edge, or -20 to leave a gap but still make it stick to the right edge.

Note that you can use this property in both relative and pixel modes, but it is most useful in pixel mode.
@par
Default: horz\_align left<br>

<a name="vert_005falign"></a><a name="vert_005falign-1"></a>

## vert\_align

Sets the vertical alignment of this element, in terms of where the vertical origin is.
@par
Format: vert\_align &lt;top|center|bottom&gt;<br> Example: vert\_align center

This can be used to change where the origin is deemed to be for the purposes of any vertical positioning attributes of this element. By default the origin is deemed to be the top edge of the screen, but if you change this you can center or bottom-align your elements. Note that setting the alignment to center or bottom does not automatically force your elements to appear in the center or the bottom edge, you just have to treat that point as the origin and adjust your coordinates appropriately. This is more flexible because you can choose to position your element anywhere relative to that origin. For example, if your element was 50 pixels high, you would use a ’top’ property of -50 to align it exactly to the bottom edge, or -70 to leave a gap but still make it stick to the bottom edge.

Note that you can use this property in both relative and pixel modes, but it is most useful in pixel mode.
@par
Default: vert\_align top<br>

<a name="left"></a><a name="left-1"></a>

## left

Sets the horizontal position of the element relative to it’s parent.
@par
Format: left &lt;value&gt;<br> Example: left 0.5

Positions are relative to the parent (the top-left of the screen if the parent is an overlay, the top-left of the parent otherwise) and are expressed in terms of a proportion of screen size. Therefore 0.5 is half-way across the screen.
@par
Default: left 0<br>

<a name="overlaytopelement"></a><a name="top"></a>

## top

Sets the vertical position of the element relative to it’s parent.
@par
Format: top &lt;value&gt;<br> Example: top 0.5

Positions are relative to the parent (the top-left of the screen if the parent is an overlay, the top-left of the parent otherwise) and are expressed in terms of a proportion of screen size. Therefore 0.5 is half-way down the screen.
@par
Default: top 0<br>

<a name="width"></a><a name="width-1"></a>

## width

Sets the width of the element as a proportion of the size of the screen.
@par
Format: width &lt;value&gt;<br> Example: width 0.25

Sizes are relative to the size of the screen, so 0.25 is a quarter of the screen. Sizes are not relative to the parent; this is common in windowing systems where the top and left are relative but the size is absolute.
@par
Default: width 1<br>

<a name="height"></a><a name="height-1"></a>

## height

Sets the height of the element as a proportion of the size of the screen.
@par
Format: height &lt;value&gt;<br> Example: height 0.25

Sizes are relative to the size of the screen, so 0.25 is a quarter of the screen. Sizes are not relative to the parent; this is common in windowing systems where the top and left are relative but the size is absolute.
@par
Default: height 1<br>

<a name="overlay_005fmaterial"></a><a name="material-3"></a>

## material

Sets the name of the material to use for this element.
@par
Format: material &lt;name&gt;<br> Example: material Examples/TestMaterial

This sets the base material which this element will use. Each type of element may interpret this differently; for example the OGRE element ’Panel’ treats this as the background of the panel, whilst ’BorderPanel’ interprets this as the material for the center area only. Materials should be defined in .material scripts. Note that using a material in an overlay element automatically disables lighting and depth checking on this material. Therefore you should not use the same material as is used for real 3D objects for an overlay.
@par
Default: none<br>

<a name="caption"></a><a name="caption-1"></a>

## caption

Sets a text caption for the element.
@par
Format: caption &lt;string&gt;<br> Example: caption This is a caption

Not all elements support captions, so each element is free to disregard this if it wants. However, a general text caption is so common to many elements that it is included in the generic interface to make it simpler to use. This is a common feature in GUI systems.
@par
Default: blank

<a name="rotation"></a><a name="rotation-1"></a>

## rotation

Sets the rotation of the element.
@par
Format: rotation &lt;angle\_in\_degrees&gt; &lt;axis\_x&gt; &lt;axis\_y&gt; &lt;axis\_z&gt; Example: rotation 30 0 0 1
@par
Default: none

# Standard OverlayElements {#Standard-OverlayElements}

Although OGRE’s Ogre::OverlayElement and Ogre::OverlayContainer classes are designed to be extended by applications developers, there are a few elements which come as standard with Ogre. These include:

-   @ref Panel
-   @ref BorderPanel
-   @ref TextArea

This section describes how you define their custom attributes in an .overlay script, but you can also change these custom properties in code if you wish. You do this by calling setParameter(param, value). You may wish to use the StringConverter class to convert your types to and from strings.

## Panel (container) {#Panel}

This is the most bog-standard container you can use. It is a rectangular area which can contain other elements (or containers) and may or may not have a background, which can be tiled however you like. The background material is determined by the material attribute, but is only displayed if transparency is off.

@param transparent <b>&lt;true | false&gt;</b> If set to ’true’ the panel is transparent and is not rendered itself, it is just used as a grouping level for it’s children.

@param tiling <b>&lt;layer&gt; &lt;x\_tile&gt; &lt;y\_tile&gt;</b> Sets the number of times the texture(s) of the material are tiled across the panel in the x and y direction. &lt;layer&gt; is the texture layer, from 0 to the number of texture layers in the material minus one. By setting tiling per layer you can create some nice multitextured backdrops for your panels, this works especially well when you animate one of the layers.

@param uv\_coords <b>&lt;topleft\_u&gt; &lt;topleft\_v&gt; &lt;bottomright\_u&gt; &lt;bottomright\_v&gt;</b> Sets the texture coordinates to use for this panel.

## BorderPanel (container) {#BorderPanel}

This is a slightly more advanced version of Panel, where instead of just a single flat panel, the panel has a separate border which resizes with the panel. It does this by taking an approach very similar to the use of HTML tables for bordered content: the panel is rendered as 9 square areas, with the center area being rendered with the main material (as with Panel) and the outer 8 areas (the 4 corners and the 4 edges) rendered with a separate border material. The advantage of rendering the corners separately from the edges is that the edge textures can be designed so that they can be stretched without distorting them, meaning the single texture can serve any size panel.

@param border\_size <b>&lt;left&gt; &lt;right&gt; &lt;top&gt; &lt;bottom&gt;</b> The size of the border at each edge, as a proportion of the size of the screen. This lets you have different size borders at each edge if you like, or you can use the same value 4 times to create a constant size border.

@param border\_material <b>&lt;name&gt;</b> The name of the material to use for the border. This is normally a different material to the one used for the center area, because the center area is often tiled which means you can’t put border areas in there. You must put all the images you need for all the corners and the sides into a single texture.

@param border\_topleft\_uv <b>&lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</b> \[also border\_topright\_uv, border\_bottomleft\_uv, border\_bottomright\_uv\]; The texture coordinates to be used for the corner areas of the border. 4 coordinates are required, 2 for the top-left corner of the square, 2 for the bottom-right of the square.

@param border\_left\_uv <b>&lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</b> \[also border\_right\_uv, border\_top\_uv, border\_bottom\_uv\]; The texture coordinates to be used for the edge areas of the border. 4 coordinates are required, 2 for the top-left corner, 2 for the bottom-right. Note that you should design the texture so that the left & right edges can be stretched / squashed vertically and the top and bottom edges can be stretched / squashed horizontally without detrimental effects.

## TextArea (element) {#TextArea}

This is a generic element that you can use to render text. It uses fonts which can be defined in code using the FontManager and Font classes, or which have been predefined in .fontdef files. See the font definitions section for more information.

@param font\_name <b>&lt;name&gt;</b> The name of the font to use. This font must be defined in a .fontdef file to ensure it is available at scripting time.

@param char\_height <b>&lt;height&gt;</b> The height of the letters as a proportion of the screen height. Character widths may vary because OGRE supports proportional fonts, but will be based on this constant height.

@param colour <b>&lt;red&gt; &lt;green&gt; &lt;blue&gt;</b> A solid colour to render the text in. Often fonts are defined in monochrome, so this allows you to colour them in nicely and use the same texture for multiple different coloured text areas. The colour elements should all be expressed as values between 0 and 1. If you use predrawn fonts which are already full colour then you don’t need this.

@param colour\_bottom <b>&lt;red&gt; &lt;green&gt; &lt;blue&gt;</b>
@param colour\_top <b>&lt;red&gt; &lt;green&gt; &lt;blue&gt;</b> As an alternative to a solid colour, you can colour the text differently at the top and bottom to create a gradient colour effect which can be very effective.

@param alignment <b>&lt;left | center | right&gt;</b> Sets the horizontal alignment of the text. This is different from the horz\_align parameter.

@param space\_width <b>&lt;width&gt;</b> Sets the width of a space in relation to the screen.

@page Font-Definition-Scripts Font Definition Scripts

%Ogre uses texture-based fonts to render the Ogre::TextAreaOverlayElement. You can also use the Ogre::Font object for your own purpose if you wish. The final form of a font is a Ogre::Material object generated by the font, and a set of ’glyph’ (character) texture coordinate information.

There are 2 ways you can get a font into OGRE:

1.  Design a font texture yourself using an art package or font generator tool
2.  Ask OGRE to generate a font texture based on a truetype font

The former gives you the most flexibility and the best performance (in terms of startup times), but the latter is convenient if you want to quickly use a font without having to generate the texture yourself. I suggest prototyping using the latter and change to the former for your final solution.

All font definitions are held in `.fontdef` files, which are parsed by the system at startup time. Each `.fontdef` file can contain multiple font definitions. The basic format of an entry in the `.fontdef` file is:

```cpp
font <font_name>
{
    type <image | truetype>
    source <image file | truetype font file>
    ...
    ... custom attributes depending on type
}
```

<a name="Using-an-existing-font-texture"></a>

# Using an existing font texture

If you have one or more artists working with you, no doubt they can produce you a very nice font texture. OGRE supports full colour font textures, or alternatively you can keep them monochrome / greyscale and use TextArea’s colouring feature. Font textures should always have an alpha channel, preferably an 8-bit alpha channel such as that supported by TGA and PNG files, because it can result in much nicer edges. To use an existing texture, here are the settings you need:

@param type <b>image</b> This just tells OGRE you want a pre-drawn font.

@param source <b>&lt;filename&gt;</b> This is the name of the image file you want to load. This will be loaded from the standard resource locations and can be of any type OGRE supports, although JPEG is not recommended because of the lack of alpha and the lossy compression. I recommend PNG format which has both good lossless compression and an 8-bit alpha channel.

@param glyph <b>&lt;character&gt; &lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</b> This provides the texture coordinates for the specified character. You must repeat this for every character you have in the texture. The first 2 numbers are the x and y of the top-left corner, the second two are the x and y of the bottom-right corner. Note that you really should use a common height for all characters, but widths can vary because of proportional fonts.
’character’ is either an ASCII character for non-extended 7-bit ASCII, or for extended glyphs, a unicode decimal value, which is identified by preceding the number with a ’u’ - e.g. ’u0546’ denotes unicode value 546.

A note for Windows users: I recommend using [BitmapFontBuilder](<http://www.lmnopc.com/bitmapfontbuilder/>), a free tool which will generate a texture and export character widths for you, you can find a tool for converting the binary output from this into ’glyph’ lines in the Tools folder.<br>

<a name="Generating-a-font-texture"></a>

# Generating a font texture

You can also generate font textures on the fly using truetype fonts. I don’t recommend heavy use of this in production work because rendering the texture can take a several seconds per font which adds to the loading times. However it is a very nice way of quickly getting text output in a font of your choice.

Here are the attributes you need to supply:

@param type <b>truetype</b> Tells OGRE to generate the texture from a font

@param source <b>&lt;ttf file&gt;</b> The name of the ttf file to load. This will be searched for in the common resource locations.

@param size <b>&lt;size\_in\_points&gt;</b> The size at which to generate the font in points. This is the value that you would select in e.g. Word. This only affects how big the characters are in the font texture, not how big they are on the screen. You should tailor this depending on how large you expect to render the fonts because generating a large texture will result in blurry characters when they are scaled very small, and conversely generating a small font will result in blocky characters if large text is rendered.

@param resolution <b>&lt;dpi&gt;</b> The resolution in dots per inch, which is used in conjunction with the point size to determine the final texture size. Typical values are 72 / 96 dpi. This should match the dpi of the screen, given that the glyps occupy `size` points after projection (i.e. in screen-space).

@param antialias\_colour <b>&lt;true|false&gt;</b> This is an optional flag, which defaults to `false`. The generator will antialias the font by default using the alpha component of the texture, which will look fine if you use alpha blending to render your text (this is the default assumed by TextAreaOverlayElement for example). If, however you wish to use a colour based blend like add or modulate in your own code, you should set this to `true` so the colour values are anti-aliased too. If you set this to true and use alpha blending, you’ll find the edges of your font are antialiased too quickly resulting in a *thin* look to your fonts, because not only is the alpha blending the edges, the colour is fading too. Leave this option at the default if in doubt.

@param code\_points <b>nn-nn \[nn-nn\] ..</b> This directive allows you to specify which unicode code points should be generated as glyphs into the font texture. If you don’t specify this, code points 33-126 will be generated by default which covers the ASCII glyphs. If you use this flag, you should specify a space-separated list of inclusive code point ranges of the form ’start-end’. Numbers must be decimal.

You can also create new fonts at runtime by using the FontManager if you wish.