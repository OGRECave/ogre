@node Compositor Scripts 
@section Compositor Scripts 

The compositor framework is a subsection of the OGRE API that allows you to easily define full screen post-processing effects. Compositor scripts offer you the ability to define compositor effects in a script which can be reused and modified easily, rather than having to use the API to define them. You still need to use code to instantiate a compositor against one of your visible viewports, but this is a much simpler process than actually defining the compositor itself. 

@heading Compositor Fundamentals 

Performing post-processing effects generally involves first rendering the scene to a texture, either in addition to or instead of the main window. Once the scene is in a texture, you can then pull the scene image into a fragment program and perform operations on it by rendering it through full screen quad. The target of this post processing render can be the main result (e.g. a window), or it can be another render texture so that you can perform multi-stage convolutions on the image. You can even 'ping-pong' the render back and forth between a couple of render textures to perform convolutions which require many iterations, without using a separate texture for each stage. Eventually you'll want to render the result to the final output, which you do with a full screen quad. This might replace the whole window (thus the main window doesn't need to render the scene itself), or it might be a combinational effect. @*@* 

So that we can discuss how to implement these techniques efficiently, a number of definitions are required:@*@* 
@table @asis 
@item Compositor 
Definition of a fullscreen effect that can be applied to a user viewport. This is what you're defining when writing compositor scripts as detailed in this section.
@item Compositor Instance 
An instance of a compositor as applied to a single viewport. You create these based on compositor definitions, @xref{Applying a Compositor}.
@item Compositor Chain 
It is possible to enable more than one compositor instance on a viewport at the same time, with one compositor taking the results of the previous one as input. This is known as a compositor chain. Every viewport which has at least one compositor attached to it has a compositor chain. @xref{Applying a Compositor}
@item Target 
This is a RenderTarget, ie the place where the result of a series of render operations is sent. A target may be the final output (and this is implicit, you don't have to declare it), or it may be an intermediate render texture, which you declare in your script with the @ref{compositor_texture,texture line}. A target which is not the output target has a defined size and pixel format which you can control. 
@item Output Target 
As Target, but this is the single final result of all operations. The size and pixel format of this target cannot be controlled by the compositor since it is defined by the application using it, thus you don't declare it in your script. However, you do declare a Target Pass for it, see below.
@item Target Pass 
A Target may be rendered to many times in the course of a composition effect. In particular if you 'ping pong' a convolution between a couple of textures, you will have more than one Target Pass per Target. Target passes are declared in the script using a @ref{Compositor Target Passes,target or target_output line}, the latter being the final output target pass, of which there can be only one.
@item Pass 
Within a Target Pass, there are one or more individual @ref{Compositor Passes,passes}, which perform a very specific action, such as rendering the original scene (or pulling the result from the previous compositor in the chain), rendering a fullscreen quad, or clearing one or more buffers. Typically within a single target pass you will use the either a 'render scene' pass or a 'render quad' pass, not both. Clear can be used with either type. 
@end table 

@heading Loading scripts

Compositor scripts are loaded when resource groups are initialised: OGRE looks in all resource locations associated with the group (see Root::addResourceLocation) for files with the '.compositor' extension and parses them. If you want to parse files manually, use CompositorSerializer::parseScript.@*@*

@heading Format

Several compositors may be defined in a single script. The script format is pseudo-C++, with sections delimited by curly braces ('{', '}'), and comments indicated by starting a line with '//' (note, no nested form comments allowed). The general format is shown below in the example below:@*@*
@example
// This is a comment
// Black and white effect
compositor B&W
{
    technique
    {
        // Temporary textures
        texture rt0 target_width target_height PF_A8R8G8B8

        target rt0
        {
            // Render output from previous compositor (or original scene)
            input previous
        }

        target_output
        {
            // Start with clear output
            input none
            // Draw a fullscreen quad with the black and white image
            pass render_quad
            {
                // Renders a fullscreen quad with a material
                material Ogre/Compositor/BlackAndWhite
                input 0 rt0
            }
        }
    }
}
@end example

Every compositor in the script must be given a name, which is the line 'compositor <name>' before the first opening '{'. This name must be globally unique. It can include path characters (as in the example) to logically divide up your compositors, and also to avoid duplicate names, but the engine does not treat the name as hierarchical, just as a string.  Names can include spaces but must be surrounded by double quotes ie compositor "My Name".@*@*

The major components of a compositor are the @ref{Compositor Techniques,techniques}, the @ref{Compositor Target Passes,target passes} and the @ref{Compositor Passes,passes}, which are covered in detail in the following sections.@*@*

@node Compositor Techniques
@subsection Techniques

A compositor technique is much like a @ref{Techniques,material technique} in that it describes one approach to achieving the effect you're looking for. A compositor definition can have more than one technique if you wish to provide some fallback should the hardware not support the technique you'd prefer to use. Techniques are evaluated for hardware support based on 2 things:
@table @asis
@item Material support
All @ref{Compositor Passes,passes} that render a fullscreen quad use a material; for the technique to be supported, all of the materials referenced must have at least one supported material technique. If they don't, the compositor technique is marked as unsupported and won't be used.
@item Texture format support
This one is slightly more complicated. When you request a @ref{compositor_texture,texture} in your technique, you request a pixel format. Not all formats are natively supported by hardware, especially the floating point formats. However, in this case the hardware will typically downgrade the texture format requested to one that the hardware does support - with compositor effects though, you might want to use a different approach if this is the case. So, when evaluating techniques, the compositor will first look for native support for the exact pixel format you've asked for, and will skip onto the next technique if it is not supported, thus allowing you to define other techniques with simpler pixel formats which use a different approach. If it doesn't find any techniques which are natively supported, it tries again, this time allowing the hardware to downgrade the texture format and thus should find at least some support for what you've asked for.
@end table

As with material techniques, compositor techniques are evaluated in the order you define them in the script, so techniques declared first are preferred over those declared later.

Format: technique { }@*@*

Techniques can have the following nested elements:
@itemize @bullet
@item @ref{compositor_texture,texture}
@item @ref{compositor_scheme,scheme}
@item @ref{Compositor Target Passes,target}
@item @ref{Compositor Target Passes,target_output}
@end itemize

@anchor{compositor_texture}
@subheading texture

This declares a render texture for use in subsequent @ref{Compositor Target Passes,target passes}.

@*@*
Format: texture <Name> <Width> <Height> <Pixel Format> [<MRT Pixel Format2>] [<MRT Pixel FormatN>] [shared] [gamma] [no_fsaa]@*@*

Here is a description of the parameters:
@table @asis
@item Name
A name to give the render texture, which must be unique within this compositor. This name is used to reference the texture in @ref{Compositor Target Passes,target passes}, when the texture is rendered to, and in @ref{Compositor Passes,passes}, when the texture is used as input to a material rendering a fullscreen quad.
@item Width, Height
The dimensions of the render texture. You can either specify a fixed width and height, or you can request that the texture is based on the physical dimensions of the viewport to which the compositor is attached. The options for the latter are 'target_width', 'target_height', 'target_width_scaled <factor>' and 'target_height_scaled <factor>' - where 'factor' is the amount by which you wish to multiply the size of the main target to derive the dimensions.
@item Pixel Format
The pixel format of the render texture. This affects how much memory it will take, what colour channels will be available, and what precision you will have within those channels. The available options are PF_A8R8G8B8, PF_R8G8B8A8, PF_R8G8B8, PF_FLOAT16_RGBA, PF_FLOAT16_RGB, PF_FLOAT16_R, PF_FLOAT32_RGBA, PF_FLOAT32_RGB, and PF_FLOAT32_R.
@item shared
If present, this directive makes this texture 'shared' among compositor instances, which can save some memory.
@item gamma
If present, this directive means that sRGB gamma correction will be enabled on writes to this texture. You should remember to include the opposite sRGB conversion when you read this texture back in another material, such as a quad. This option will automatically enabled if you use a render_scene pass on this texture and the viewport on which the compositor is based has sRGB write support enabled.
@item no_fsaa
If present, this directive disables the use of anti-aliasing on this texture. FSAA is only used if this texture is subject to a render_scene pass and FSAA was enabled on the original viewport on which this compositor is based; this option allows you to override it and disable the FSAA if you wish. 
@end table

Example: texture rt0 512 512 PF_R8G8B8A8@*
Example: texture rt1 target_width target_height PF_FLOAT32_RGB@*@*

You can in fact repeat this element if you wish. If you do so, that means that this render texture becomes a Multiple Render Target (MRT), when the GPU writes to multiple textures at once. It is imperative that if you use MRT that the shaders that render to it render to ALL the targets. Not doing so can cause undefined results. It is also important to note that although you can use different pixel formats for each target in a MRT, each one should have the same total bit depth since most cards do not support independent bit depths. If you try to use this feature on cards that do not support the number of MRTs you've asked for, the technique will be skipped (so you ought to write a fallback technique).

@anchor{compositor_scheme}
@subheading scheme

This gives a compositor technique a scheme name, allowing you to manually switch between different techniques for this compositor when instantiated on a viewport by calling CompositorInstance::setScheme. 

@*@*
Format: texture <Name> <Width> <Height> <Pixel Format> [<MRT Pixel Format2>] [<MRT Pixel FormatN>] [shared] [gamma] [no_fsaa]@*@*


@node Compositor Target Passes
@subsection Target Passes

A target pass is the action of rendering to a given target, either a render texture or the final output. You can update the same render texture multiple times by adding more than one target pass to your compositor script - this is very useful for 'ping pong' renders between a couple of render textures to perform complex convolutions that cannot be done in a single render, such as blurring.@*@*

There are two types of target pass, the sort that updates a render texture:@*@*
Format: target <Name> { }@*@*
... and the sort that defines the final output render:@*@*
Format: target_output { }@*@*

The contents of both are identical, the only real difference is that you can only have a single target_output entry, whilst you can have many target entries. Here are the attributes you can use in a 'target' or 'target_output' section of a .compositor script:

@itemize @bullet
@item
@ref{compositor_target_input,input}
@item
@ref{only_initial}
@item
@ref{visibility_mask}
@item
@ref{compositor_lod_bias,lod_bias}
@item
@ref{material_scheme}
@item
@ref{compositor_shadows,shadows}
@item 
@ref{Compositor Passes,pass}
@end itemize

@heading Attribute Descriptions
@anchor{compositor_target_input}
@subheading input
Sets input mode of the target, which tells the target pass what is pulled in before any of its own passes are rendered.@*@*
Format: input (none | previous)@*@*
Default: input none@*@*
@table @asis
@item none
The target will have nothing as input, all the contents of the target must be generated using its own passes. Note this does not mean the target will be empty, just no data will be pulled in. For it to truly be blank you'd need a 'clear' pass within this target.
@item previous
The target will pull in the previous contents of the viewport. This will be either the original scene if this is the first compositor in the chain, or it will be the output from the previous compositor in the chain if the viewport has multiple compositors enabled.
@end table

@anchor{only_initial}
@subheading only_initial
If set to on, this target pass will only execute once initially after the effect has been enabled. This could be useful to perform once-off renders, after which the static contents are used by the rest of the compositor.@*@*
Format: only_initial (on | off)@*@*
Default: only_initial off@*@*


@anchor{visibility_mask}
@subheading visibility_mask
Sets the visibility mask for any render_scene passes performed in this target pass. This is a bitmask (although it must be specified as decimal, not hex) and maps to SceneManager::setVisibilityMask.
Format: visibility_mask <mask>@*@*
Default: visibility_mask 4294967295@*@*

@anchor{compositor_lod_bias}
@subheading lod_bias
Set the scene LOD bias for any render_scene passes performed in this target pass. The default is 1.0, everything below that means lower quality, higher means higher quality.@*@*
Format: lod_bias <lodbias>@*@*
Default: lod_bias 1.0@*@*

@anchor{compositor_shadows}
@subheading shadows
Sets whether shadows should be rendered during any render_scene pass performed in this target pass. The default is 'on'.@*@*
Format: shadows (on | off)@*@*
Default: shadows on@*@*

@anchor{material_scheme}
@subheading material_scheme
If set, indicates the material scheme to use for any render_scene pass. Useful for performing special-case rendering effects.@*@*
Format: material_scheme <scheme name>@*@*
Default: None@*@*

@node Compositor Passes
@subsection Compositor Passes

A pass is a single rendering action to be performed in a target pass. @*@*
Format: 'pass' (render_quad | clear | stencil | render_scene) { }@*@*

There are four types of pass:
@table @asis
@item clear
This kind of pass sets the contents of one or more buffers in the target to a fixed value. So this could clear the colour buffer to a fixed colour, set the depth buffer to a certain set of contents, fill the stencil buffer with a value, or any combination of the above.
@item stencil
This kind of pass configures stencil operations for the subsequent passes. It can set the stencil compare function, operations and reference values for you to perform your own stencil effects.
@item render_scene
This kind of pass performs a regular rendering of the scene. It will use the @ref{visibility_mask}, @ref{compositor_lod_bias,lod_bias}, and @ref{material_scheme} from the parent target pass.
@item render_quad
This kind of pass renders a quad over the entire render target, using a given material. You will undoubtedly want to pull in the results of other target passes into this operation to perform fullscreen effects.
@end table

Here are the attributes you can use in a 'pass' section of a .compositor script:

@heading Available Pass Attributes
@itemize @bullet
@item
@ref{material}
@item
@ref{compositor_pass_input,input}
@item
@ref{compositor_pass_identifier,identifier}
@item
@ref{first_render_queue}
@item
@ref{last_render_queue}
@item 
@ref{compositor_clear,clear}
@item 
@ref{compositor_stencil,stencil}
@end itemize

@anchor{material}
@subheading material
For passes of type 'render_quad', sets the material used to render the quad. You will want to use shaders in this material to perform fullscreen effects, and use the @ref{compositor_pass_input,input} attribute to map other texture targets into the texture bindings needed by this material.@*@*
Format: material <Name>@*@*

@anchor{compositor_pass_input}
@subheading input
For passes of type 'render_quad', this is how you map one or more local render textures (@xref{compositor_texture}) into the material you're using to render the fullscreen quad. To bind more than one texture, repeat this attribute with different sampler indexes.@*@*
Format: input <sampler> <Name> [<MRTIndex>]@*@*
@table @asis
@item sampler
The texture sampler to set, must be a number in the range [0, OGRE_MAX_TEXTURE_LAYERS-1].
@item Name
The name of the local render texture to bind, as declared in @ref{compositor_texture} and rendered to in one or more @ref{Compositor Target Passes,target pass}.
@item MRTIndex
If the local texture that you're referencing is a Multiple Render Target (MRT), this identifies the surface from the MRT that you wish to reference (0 is the first surface, 1 the second etc).
@end table
Example: input 0 rt0

@anchor{compositor_pass_identifier}
@subheading identifier
Associates a numeric identifier with the pass. This is useful for registering a listener with the compositor (CompositorInstance::addListener), and being able to identify which pass it is that's being processed when you get events regarding it. Numbers between 0 and 2^32 are allowed.@*@*
Format: identifier <number>@*@*
Example: identifier 99945@*@*
Default: identifier 0@*@*

@anchor{first_render_queue}
@subheading first_render_queue
For passes of type 'render_scene', this sets the first render queue id that is included in the render. Defaults to the value of RENDER_QUEUE_SKIES_EARLY.@*@*
Format: first_render_queue <id>@*@*
Default: first_render_queue 0@*@*

@anchor{last_render_queue}
@subheading last_render_queue
For passes of type 'render_scene', this sets the last render queue id that is included in the render. Defaults to the value of RENDER_QUEUE_SKIES_LATE.@*@*
Format: last_render_queue <id>@*@*
Default: last_render_queue 95

@anchor{compositor_clear}
@heading Clear Section
For passes of type 'clear', this section defines the buffer clearing parameters. @*@*
Format: clear { }@*@*

Here are the attributes you can use in a 'clear' section of a .compositor script:
@itemize @bullet
@item
@ref{compositor_clear_buffers,buffers}
@item
@ref{compositor_clear_colour_value,colour_value}
@item
@ref{compositor_clear_depth_value,depth_value}
@item
@ref{compositor_clear_stencil_value,stencil_value}

@anchor{compositor_clear_buffers}
@subheading buffers
Sets the buffers cleared by this pass.@*@*

Format: buffers [colour] [depth] [stencil]@*@*
Default: buffers colour depth@*@*

@anchor{compositor_clear_colour_value}
@subheading colour_value
Set the colour used to fill the colour buffer by this pass, if the colour buffer is being cleared (@ref{compositor_clear_buffers,buffers}). @*@*
Format: colour_value <red> <green> <blue> <alpha>@*@*
Default: colour_value 0 0 0 0@*@*

@anchor{compositor_clear_depth_value}
@subheading depth_value
Set the depth value used to fill the depth buffer by this pass, if the depth buffer is being cleared (@ref{compositor_clear_buffers,buffers}). @*@*
Format: depth_value <depth>@*@*
Default: depth_value 1.0@*@*

@anchor{compositor_clear_stencil_value}
@subheading stencil_value
Set the stencil value used to fill the stencil buffer by this pass, if the stencil buffer is being cleared (@ref{compositor_clear_buffers,buffers}). @*@*
Format: stencil_value <value>@*@*
Default: stencil_value 0.0@*@*
@end itemize

@anchor{compositor_stencil}
@heading Stencil Section
For passes of type 'stencil', this section defines the stencil operation parameters. @*@*

Format: stencil { }@*@*

Here are the attributes you can use in a 'stencil' section of a .compositor script:

@itemize @bullet
@item
@ref{compositor_stencil_check,check}
@item
@ref{compositor_stencil_comp_func,comp_func}
@item
@ref{compositor_stencil_ref_value,ref_value}
@item
@ref{compositor_stencil_mask,mask}
@item
@ref{compositor_stencil_fail_op,fail_op}
@item
@ref{compositor_stencil_depth_fail_op,depth_fail_op}
@item
@ref{compositor_stencil_pass_op,pass_op}
@item
@ref{compositor_stencil_two_sided,two_sided}

@anchor{compositor_stencil_check}
@subheading check
Enables or disables the stencil check, thus enabling the use of the rest of the features in this section. The rest of the options in this section do nothing if the stencil check is off.
Format: check (on | off)@*@*

@anchor{compositor_stencil_comp_func}
@subheading comp_func
Sets the function used to perform the following comparison:@*@*
(ref_value & mask) comp_func (Stencil Buffer Value & mask)@*@*

What happens as a result of this comparison will be one of 3 actions on the stencil buffer, depending on whether the test fails, succeeds but with the depth buffer check still failing, or succeeds with the depth buffer check passing too. You set the actions in the @ref{compositor_stencil_fail_op,fail_op}, @ref{compositor_stencil_depth_fail_op,depth_fail_op} and @ref{compositor_stencil_pass_op,pass_op} respectively. If the stencil check fails, no colour or depth are written to the frame buffer.@*@*
Format: comp_func (always_fail | always_pass | less | less_equal | not_equal | greater_equal | greater)@*@*
Default: comp_func always_pass@*@*

@anchor{compositor_stencil_ref_value}
@subheading ref_value
Sets the reference value used to compare with the stencil buffer as described in @ref{compositor_stencil_comp_func,comp_func}.@*@*
Format: ref_value <value>@*@*
Default: ref_value 0.0@*@*

@anchor{compositor_stencil_mask}
@subheading mask
Sets the mask used to compare with the stencil buffer as described in @ref{compositor_stencil_comp_func,comp_func}.@*@*
Format: mask <value>@*@*
Default: mask 4294967295@*@*

@anchor{compositor_stencil_fail_op}
@subheading fail_op
Sets what to do with the stencil buffer value if the result of the stencil comparison (@ref{compositor_stencil_comp_func,comp_func}) and depth comparison is that both fail.@*@*
Format: fail_op (keep | zero | replace | increment | decrement | increment_wrap | decrement_wrap | invert)@*@*
Default: depth_fail_op keep@*@*
These actions mean:
@table @asis
@item keep
Leave the stencil buffer unchanged.
@item zero
Set the stencil value to zero.
@item replace
Set the stencil value to the reference value.
@item increment
Add one to the stencil value, clamping at the maximum value.
@item decrement
Subtract one from the stencil value, clamping at 0.
@item increment_wrap
Add one to the stencil value, wrapping back to 0 at the maximum.
@item decrement_wrap
Subtract one from the stencil value, wrapping to the maximum below 0.
@item invert
invert the stencil value.
@end table


@anchor{compositor_stencil_depth_fail_op}
@subheading depth_fail_op
Sets what to do with the stencil buffer value if the result of the stencil comparison (@ref{compositor_stencil_comp_func,comp_func}) passes but the depth comparison fails. @*@*

Format: depth_fail_op (keep | zero | replace | increment | decrement | increment_wrap | decrement_wrap | invert)@*@*
Default: depth_fail_op keep@*@*

@anchor{compositor_stencil_pass_op}
@subheading pass_op
Sets what to do with the stencil buffer value if the result of the stencil comparison (@ref{compositor_stencil_comp_func,comp_func}) and the depth comparison pass. @*@*
Format: pass_op (keep | zero | replace | increment | decrement | increment_wrap | decrement_wrap | invert)@*@*
Default: pass_op keep@*@*

@anchor{compositor_stencil_two_sided}
@subheading two_sided
Enables or disables two-sided stencil operations, which means the inverse of the operations applies to back-facing polygons.@*@*
Format: two_sided (on | off)@*@*
Default: two_sided off@*@*
@end itemize

@node Applying a Compositor
@subsection Applying a Compositor

Adding a compositor instance to a viewport is very simple. All you need to do is:@*@*
@example
CompositorManager::getSingleton().addCompositor(viewport, compositorName);
@end example
@*@*
Where viewport is a pointer to your viewport, and compositorName is the name of the compositor to create an instance of. By doing this, a new instance of a compositor will be added to a new compositor chain on that viewport. You can call the method multiple times to add further compositors to the chain on this viewport. By default, each compositor which is added is disabled, but you can change this state by calling:@*@*
@example
CompositorManager::getSingleton().setCompositorEnabled(viewport, compositorName, enabledOrDisabled);
@end example
@*@*
For more information on defining and using compositors, see Demo_Compositor in the Samples area, together with the Examples.compositor script in the media area.

