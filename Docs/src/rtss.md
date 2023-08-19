# Runtime Shader Generation {#rtss}

With D3D11 and GL3, support for fixed pipeline functionality was removed. Meaning you can only render objects using shaders.

While @ref High-level-Programs offer you maximal control and flexibility over how your objects are rendered, writing and maintaining them is also a very time consuming task.

The Run Time Shader System or RTSS for short is the %Ogre way of managing Shaders and their variations. Initially it was created as a drop-in-replacement to the Fixed-Function Pipeline (FFP) for RenderSystems that lacked it.
However, since then it grew to a general way to express shader functionality in @ref Material-Scripts without having to manually write shaders.

While the resulting shaders are less optimized, they offer the following advantages:

* Save development time e.g. when your target scene has dynamic lights and the number changes, fog changes and the number of material attributes increases the total count of needed shaders dramatically. It can easily cross 100 and it becomes a time consuming development task.
* Reusable code - once you've written the shader extension you can use it anywhere due to its independent nature.
* Custom shaders extension library - enjoy the shared library of effects created by the community. Unlike hand written shader code, which may require many adjustments to be plugged into your own shader code, using the extensions library requires minimum changes.

For fixed function function properties, the RTSS will read the standard `pass` and `texture_unit` definitions, so no changes are required. To enable features that go beyond the possibilities of the FFP, you have to define an additional `rtshader_system` block with the respective properties.

For instance, the FFP only allows per-vertex lighting. To request per-pixel lighting, you would add the following block to a pass:

@snippet Samples/Media/materials/scripts/RTShaderSystem.material rtss_per_pixel

To modify the default lighting stage [see below](@ref rtss_custom_api). For more examples see `Media/RTShaderLib/materials/RTShaderSystem.material`.

@tableofcontents

# RTSS Pass properties {#rtss_custom_mat}

Here are the attributes you can use in a `rtshader_system` block of a `pass {}`:

- [transform_stage](#transform_stage)
- [lighting_stage](#lighting_stage)
- [image_based_lighting](#image_based_lighting)
- [gbuffer](#gbuffer)
- [normal_map](#normal_map_pass)
- [metal_roughness](#metal_roughness)
- [fog_stage](#fog_stage)
- [light_count](#light_count)
- [triplanarTexturing](#triplanarTexturing)
- [shadow_mapping](#shadow_mapping)
- [integrated_pssm4](#integrated_pssm4)
- [hardware_skinning](#hardware_skinning)
- [layered_blend](#layered_blend)
- [source_modifier](#source_modifier)

<a name="transform_stage"></a>

## transform_stage
@copybrief Ogre::RTShader::SRS_TRANSFORM

@par
Format: `transform_stage <type> [attrIndex]`
@par
Example: `transform_stage instanced 1`

@param type either `ffp` or `instanced`
@param attrIndex the start texcoord attribute index to read the instanced world matrix from. Must be greater than 0.

@see @ref Instancing-in-Vertex-Programs
@see Ogre::InstanceBatchHW


<a name="lighting_stage"></a>

## lighting_stage

Force a specific lighting model.

@par
Format: `lighting_stage <ffp|per_pixel> [two_sided] [normalised]`
@par
Example: `lighting_stage ffp two_sided`

@param two_sided compute lighting on both sides of the surface, when culling is disabled.
@param normalised normalise the blinn-phong reflection model to make it energy conserving - see [this for details](http://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/)

<a name="image_based_lighting"></a>

## image_based_lighting

@copybrief Ogre::RTShader::SRS_IMAGE_BASED_LIGHTING

@par
Format: `image_based_lighting texture <texture> [luminance <luminance>]`
@par
Example: `image_based_lighting texture ibl_cubemap.ktx`

@param luminance factor to scale the IBL influence by

For best results, generate the cubemaps using [cmgen](https://github.com/google/filament/tree/main/tools/cmgen) from the filament project.

<a name="gbuffer"></a>

## gbuffer

@copybrief Ogre::RTShader::SRS_GBUFFER

@par
Format: `lighting_stage gbuffer <target_layout> [target_layout]`
@par
Example: `lighting_stage gbuffer normal_viewdepth diffuse_specular`

@param target_layout with @c gbuffer, this specifies the data to be written into one or two MRT targets. Possible values are @c depth, @c normal, @c viewpos, @c normal_viewdepth and @c diffuse_specular

<a name="normal_map_pass"></a>

## normal_map

@deprecated use @ref normal_map instead

@copybrief Ogre::RTShader::SRS_NORMALMAP

@par
Format: `lighting_stage normal_map <texture> [normalmap_space] [texcoord_index] [sampler]`
@par
Example: `lighting_stage normal_map Panels_Normal_Tangent.png tangent_space 0 SamplerToUse`

@param texture normal map name to use
@param normalmap_space see @ref normal_map
@param texcoord_index the start texcoord attribute index to read the uv coordinates from
@param sampler the [Sampler](@ref Samplers) to use for the normal map


<a name="metal_roughness"></a>

## metal_roughness

@copybrief Ogre::RTShader::SRS_COOK_TORRANCE_LIGHTING

By default, roughness is read from `specular[0]` and metalness from `specular[1]`.

@par
Format: `lighting_stage metal_roughness [texture <texturename>]`
@par
Example: `lighting_stage metal_roughness texture Default_metalRoughness.jpg`

@param texturename texture for spatially varying parametrization.
[In accordance to the glTF2.0 specification](https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#_material_pbrmetallicroughness_metallicroughnesstexture), roughness is sampled from the G channel and metalness from the B channel.

@note Using this option switches the lighting equations from Blinn-Phong to the Cook-Torrance PBR model [using the equations described by Filament](https://google.github.io/filament/Filament.html#materialsystem/standardmodelsummary).

<a name="fog_stage"></a>

## fog_stage

@copybrief Ogre::RTShader::SRS_FOG

@par
Format: `fog_stage ffp <calc_mode>`
@par
Example: `fog_stage ffp per_pixel`

@param calc_mode either `per_vertex` or `per_pixel`

<a name="light_count"></a>

## light_count

Override dynamic light count. Fix the number of lights to be used in the shader. Avoids shader recompilation when the number of lights changes.
@par
Format: `light_count <count>`

<a name="triplanarTexturing"></a>

## triplanarTexturing

@copybrief Ogre::RTShader::SRS_TRIPLANAR_TEXTURING

@par
Format: `triplanarTexturing <textureScale> <plateauSize> <transitionSpeed> <textureFromX> <textureFromY> <textureFromZ>`
@par
Example: `triplanarTexturing 0.05 0.2 4.0 BumpyMetal.jpg egyptrockyfull.jpg MtlPlat2.jpg`

@param textureScale texture coordinates are multiplied by this.
@param plateauSize plateau on which small components of the normal have no influence. 
@param transitionSpeed transitions speed between the three textures
Valid values are [0; 0.57] not bigger to avoid division by zero
@param textureFromX Texture for the x-direction planar mapping
@param textureFromY Texture for the y-direction planar mapping
@param textureFromZ Texture for the z-direction planar mapping

<a name="integrated_pssm4"></a>

## integrated_pssm4
@copybrief Ogre::RTShader::SRS_INTEGRATED_PSSM3

@par
Format: `integrated_pssm4 <znear> <sp0> <sp1> <zfar> [debug] [filter]`
@param debug visualize the active shadow-splits in the scene
@param filter one of `pcf4, pcf16` (default: @c pcf4)

## shadow_mapping {#shadow_mapping}
@copybrief Ogre::RTShader::SRS_SHADOW_MAPPING

@par
Format: `shadow_mapping [light_count <num>] [filter <type>]`
@param filter one of `pcf4, pcf16` (default: @c pcf4)
@param light_count number of lights to support (default: 1)

<a name="hardware_skinning"></a>

## hardware_skinning
@copybrief Ogre::RTShader::SRS_HARDWARE_SKINNING

@par
Format: `hardware_skinning <max_bone_count> <weight_count> [type] [correct_antipodality scale_shearing]`
@par
Example: `hardware_skinning 24 2 dual_quaternion true false`

@param type either `dual_quaternion` or `linear` (default: @c linear)
@param correct_antipodality Correctly handle rotations > 180° in dual quaternion computation
@param scale_shearing add scaling and shearing support to dual quaternion computation

@note You can also use Ogre::RTShader::HardwareSkinningFactory::prepareEntityForSkinning to derive this information automatically.

# RTSS Texture Unit properties {#rtss_tu_props}

Here are the attributes you can use in a `rtshader_system` block of a `texture_unit {}`:

- [normal_map](#normal_map)
- [layered_blend](#layered_blend)
- [source_modifier](#source_modifier)

## normal_map {#normal_map}

@copybrief Ogre::RTShader::SRS_NORMALMAP

@par
Format: `normal_map <normalmap_space> [height_scale scale] [texcoord_index idx]`
@par
Example: `normal_map parallax_occlusion height_scale 0.1`

@param normalmap_space <dl compact="compact">
<dt>tangent_space</dt>
<dd>Normal map contains normal data in tangent space.
This is the default normal mapping behavior and it requires that the
target mesh will have valid tangents within its vertex data.</dd>
<dt>object_space</dt>
<dd>Normal map contains normal data in object local space.
This normal mapping technique has the advantages of better visualization results,
lack of artifacts that comes from texture mirroring usage, it doesn't requires tangent
and it also saves some instruction in the vertex shader stage.
The main drawback of using this kind of normal map is that the target object must be static
in terms of local space rotations and translations.</dd>
<dt>parallax</dt>
<dd>Normal map contains normal data in parallax corrected tangent space
The restrictions of @c tangent_space apply. Additionally the alpha
channel of the normal texture is expected to contain height displacement data.
This is used for parallax corrected rendering.</dd>
<dt>parallax_occlusion</dt>
<dd>An extension of @c parallax, which samples the texture multiple times to allow using
a larger displacement value without getting artifacts.</dd>
</dl>
@param height_scale displacement scale factor, when using @c parallax or @c parallax_occlusion
@param texcoord_index the texcoord attribute index to read the uv coordinates from

<a name="layered_blend"></a>

## layered_blend

@copybrief Ogre::RTShader::SRS_LAYERED_BLENDING
@par
Format: `layered_blend <effect>`
@par
Example: layered_blend luminosity

@param effect one of `default, normal, lighten, darken, multiply, average, add, subtract, difference, negation, exclusion, screen, overlay, hard_light, soft_light, color_dodge, color_burn, linear_dodge, linear_burn, linear_light, vivid_light, pin_light, hard_mix, reflect, glow, phoenix, saturation, color, luminosity`


<a name="source_modifier"></a>

## source_modifier

Apply custom modulate effect to texture layer
@par
Format: `source_modifier <operation> custom <parameterNum>`
@par
Example: `source_modifier src1_inverse_modulate custom 2`

@param operation one of `src1_modulate, src2_modulate, src1_inverse_modulate, src2_inverse_modulate`
@param parameterNum number of the custom shader parameter that controls the operation

# Setting properties programmatically {#RTSS-Props-API}

In case you need to set the properties programmatically, see the following example for how the script is mapped to the API.

```cpp
rtshader_system
{
	normal_map height_scale 0.1
}
```
becomes
```cpp
using namespace Ogre::RTShader;
ShaderGenerator* shaderGen = ShaderGenerator::getSingletonPtr();

shaderGen->createShaderBasedTechnique(mat->getTechnique(0), MSN_SHADERGEN);
RenderState* rs = shaderGen->getRenderState(MSN_SHADERGEN, *mat, 0);
SubRenderState* srs = shaderGen->createSubRenderState(SRS_NORMALMAP);
rs->addTemplateSubRenderState(srs);

srs->setParameter("height_scale", "0.1");
```

# System overview {#rtss_overview}

The RTSS manages a set of opaque isolated components (SubRenderStates) where each implements a specific effect.
These "effects" include Fixed Function transformation and lighting. At the core these components are plain shader files providing a set of functions; e.g. @c SGX_Light_Directional_Diffuse(), @c SGX_Light_Point_Diffuse().

Correctly ordering these functions, providing them with the right input values and interconnecting them is the main purpose of the RTSS.

To this end the RTSS defines a set of stages; e.g Ogre::RTShader::FFP_VS_TRANSFORM, Ogre::RTShader::FFP_PS_TEXTURING.
It then queries each registered Ogre::RTShader::SubRenderState to attach its functions to these stages. Then it generates the entry function (e.g. `main()` for GLSL) by sequentially calling these functions.

You can think of stages as a way to group shader "effects" inside a Ogre::Pass - similarly how a Ogre::RenderQueueGroup groups [renderables](@ref Ogre::Renderable) for rendering.

Basically it performs the following (simplified) transformation, given
```cpp
// GLOBAL PARAMETERS
$global_parameters
// FUNCTION
$input_parameters
$output_parameters
void main() {
	$local_parameters
	$FFP_VS_TRANSFORM
	(...)
	$FFP_VS_TEXTURING
}
```
and `$FFP_VS_TRANSFORM = [FFP_Transform()]`, `$FFP_VS_TEXTURING = [FFP_TransformTexCoord()]`, it generates

```cpp
// PROGRAM DEPENDENCIES
#include <FFPLib_Transform.glsl>
#include <FFPLib_Texturing.glsl>
// GLOBAL PARAMETERS
uniform	mat4	worldviewproj_matrix;
uniform	mat4	texture_matrix1;
// FUNCTION
in	vec4	vertex;
in	vec4	uv0;
out	vec4	oTexcoord4_0;
void main() {
	FFP_Transform(worldviewproj_matrix, vertex, gl_Position);
	FFP_TransformTexCoord(texture_matrix1, uv0.xy, oTexcoord4_0.xy);
}
```

As you can see the RTSS also resolved the required parameters and routed them into the correct functions. See @ref creating-extensions for details about parameter resolution.

Now that you know what the RTSS does, you are probably wondering how to change which functions are emitted per stage. Lets say, change the lighting from the FFP style per-vertex lighting to per-pixel lighting.

The RTSS is flexible enough to "just" move the according calculations from the vertex shader to the pixel shader.

## Core features of the system {#core-feats}
* Runtime shader generation synchronized with scene state. Each time scene settings change, a new set of shaders is generated.
* Full Fixed Function Pipeline (FFP) emulation. This feature is most useful combined with render system that doesn't provide any FFP functionality (OpenGL ES 2.0, D3D11 etc).
* Shader language independent interface: the logic representation of the shader programs is completely independent from the target shader language. You can generate code for different shader languages from the same program.
* Pluggable interface for different shader languages.
* Pluggable interface for shader based functions in a seamless way. Each function will be automatically combined with the rest of the shader code.
* Smart program caching: each shader program is created only once and may be used by multiple passes.
* Automatic vertex shader output register compacting: no more compacting variables by hand. In case the amount of used vertex shader outputs exceeds the maximum allowed (12 to 32, depending on [D3DPSHADERCAPS2_0.NumTemps](http://msdn.microsoft.com/en-us/library/bb172918%28v=VS.85%29.aspx)), a compacting algorithm packs the vertex shader outputs and adds unpack code in the fragment shader side.
* Material script support, for both export and import.

## Controlling shader re-generation

By default the RTSS synchronizes with the active SceneManager regarding the fog settings and the number of active lights.
This can result in frame-drops when new lights are added, as all managed Materials are updated for the new light-count.
On the other hand, the generated shaders might include too many lights for the targeted budget.

To get more fine-grained control, you can use:
- Ogre::RTShader::RenderState::setLightCountAutoUpdate and
- Ogre::RTShader::RenderState::setLightCount

to set a fixed number of lights the materials should consider.

# The RTSS in Depth {#rtss_indepth}

When the user asks the system to generate shaders for a given technique he has to provide a name for the target technique scheme. The system then creates a new technique based on the source technique but with a different scheme name.

The idea behind this concept is to use Ogre's built in mechanism of material schemes, so all the user has to do in order to use the new technique is to call Ogre::Viewport::setMaterialScheme.

Before each viewport update, the system performs a validation step of all associated shader based techniques it created. This step includes automatic synchronization with the scene lights and fog states. When the system detects that a scheme is out of date it generates the appropriate shaders for each technique new.

The following steps are executed in order to generate shaders for a given technique:

* For each pass in the technique the system builds a set of sub render states that describe the logic process of the rendering pipeline from the draw call submission until the final pixel color.
* Each render state is translated into a set of logic shader programs (currently only pixel and vertex shader).
The logic programs are then sent to specific shader language writers that produce source code for the respective shader language. The source code is used to create the GPU programs that are applied to the destination pass.
Before rendering of an object that uses generated shaders the system allows each sub render state to update the GPU constants associated with it.

## Main components {#rtss__components}
The following is an partial list of components within the RTSS. These components are listed as they have great importance in understanding controlling and later extending the RTSS system.

@par ShaderGenerator
The ShaderGenerator is the main interface to the RTSS system. Through it you can request to generate and destroy the shaders, influence from what parts to create the shaders, and control general system settings such as the shading language and shader caching.

@par RenderState classes
The RenderState is the core component of the system. It aggregates the stages that the final shader will be created from. These stages are referred to as SubRenderStates. It is possible to bypass the rest of the RTSS and use RenderStates directly to manually generate shaders for arbitrary passes.
@par
RenderStates exist on two levels:
1. SGScheme RenderStates describe the SubRenderStates that will be used when creating a shader for a given material scheme.
2. SGPass <em>Target</em>RenderState describe the SubRenderStates that will be used when creating a specific pass of a specific material.
@par
When a shader is generated for a given material the system combines the SubRenderStates from both RenderStates to create a shader specific for a material pass in a specific scheme.

@par SubRenderState classes
Sub-render states (SRS) are components designed to generate the code of the RTSS shaders. Each SRS usually has a specific role to fill within the shader's construction. These components can be combined in different combinations to create shaders with different capabilities.
@par
By default, %Ogre adds the following 5 SRSs to every scheme RenderState to recreate the functionality provided by the fixed pipeline
1. @ref Ogre::RTShader::SRS_TRANSFORM - @copybrief Ogre::RTShader::SRS_TRANSFORM
2. @ref Ogre::RTShader::SRS_VERTEX_COLOUR - Calculate the base diffuse and specular color of the object regardless of lights or textures. The color is calculated based on the ambient, diffuse, specular and emissive properties of the object and scene and the specified color tracking.
3. @ref Ogre::RTShader::SRS_PER_PIXEL_LIGHTING - @copybrief Ogre::RTShader::SRS_PER_PIXEL_LIGHTING
4. @ref Ogre::RTShader::SRS_TEXTURING - @copybrief Ogre::RTShader::SRS_TEXTURING
5. @ref Ogre::RTShader::SRS_FOG - @copybrief Ogre::RTShader::SRS_FOG

@par SubRenderStateFactory
As the name suggests, sub render state factories are factories that produce sub render states. Each factory generates a specific SRS.
@par
These type of components are note worthy for 2 reason. The first and obvious one is that they allow the system to generate new SRSs for the materials it is asked to generate. The second reason is that they perform as script readers and writers allowing the system to create specific or specialized SRSs per material.

## Initializing the system

@note If you are using the OgreBites::ApplicationContext, the following steps will be taken automatically for you.

Initializing the system is composed of the following steps:
* Create the internal managers and structures via the Ogre::RTShader::ShaderGenerator::initialize() method.
* Assign the target scene manager to the shader generator.
* Listen for SchemeNotFound events via Ogre::MaterialManager::Listener and use the RTSS to handle them

```cpp
if (Ogre::RTShader::ShaderGenerator::initialize())
{
	// Register the scene manager.
	Ogre::RTShader::ShaderGenerator::getSingleton().addSceneManager(sceneMgr);

	// forward scheme not found events to the RTSS
	OgreBites::SGTechniqueResolverListener* schemeNotFoundHandler = ...
	Ogre::MaterialManager::getSingleton().addListener(schemeNotFoundHandler);
}
```

## Customizing the default RenderState {#rtss_custom_api}

Lets say, you wanted to globally change the default per-pixel lighting mode of the RTSS back to the FFP style per-vertex lighting.
For this you have to grab the global RenderState associated with the active material scheme,  as

@snippet Components/Bites/src/OgreAdvancedRenderControls.cpp rtss_per_pixel

Next, you have to create the FFPLighting SRS that should be used for shader generation and *add* to the set.

@note adding a SRS will automatically override the default SRS for the same stage. In the example we override the Ogre::RTShader::FFP_LIGHTING stage.

## Creating the shader based technique {#rtssTech}
This step will create a new, shader-based, technique based on the given technique. Calling Ogre::RTShader::ShaderGenerator::createShaderBasedTechnique() will cause the system to generate SubRenderStates based on the source technique and add a new technique using the given scheme name to the material.
The passes of this new technique will receive shaders generated and updated by the system during as described in the @ref rtssGenerate section below.

![](CreateShaderBasedTech.svg)

To use the generated technique, change the material scheme of your viewport(s) to scheme name you passed as argument to this method.

```cpp
// Create shader based technique from the default technique of the given material.
mShaderGenerator->createShaderBasedTechnique("Examples/BeachStones", Ogre::MSN_DEFAULT, Ogre::MSN_SHADERGEN);

// Apply the shader generated based techniques.
mViewport->setMaterialScheme(Ogre::MSN_SHADERGEN);
```

@note you can automate the shader generation process for all materials. First set the viewport scheme to the destination scheme of the RTSS shaders. Second register to the `Ogre::MaterialManager::Listener` implementing `handleSchemeNotFound()` - e.g. OgreBites::SGTechniqueResolverListener

## Shader generation at runtime {#rtssGenerate}
During the application runtime the @c ShaderGenerator instance receives notifications on per frame basis from its target @c SceneManager.
At this point it checks the material scheme in use. In case the current scheme has representations in the manager, it executes its validate method.
The @c SGScheme validation includes synchronization with scene light and fog settings. In case it is out of date it will rebuild all shader generated techniques.
1. The first step is to loop over every @c SGTechnique associated with this @c SGScheme and build its @c RenderStates - one for each pass.
2. The second step is to loop again on every @c SGTechnique and acquire a program set for each @c SGPass.

@note The shaders are only automatically updated for lights and fog changes. If you change the source pass after initial shader creation, you must call Ogre::RTShader::ShaderGenerator::invalidateMaterial manually.

The actual acquiring process is done by the @c TargetRenderState that generates CPU program representation, send them to a matching @c ProgramWriter that is chosen by the active target language, the writer generates source code that is the basis for the GPU programs.
The result of this entire process is that each technique associated with the @c SGScheme has vertex and pixel shaders applied to all its passes. These shaders are synchronized with scene lights and fog settings.

![](RuntimeShaderGeneration.svg)

## Creating custom shader extensions {#creating-extensions}
Although the system implements some common shader based effects such as per pixel lighting, normal map, etc., you may find it useful to write your own shader extensions.

In order to extend the system with your own shader effects you'll have to follow these steps:
* Implement the SubRenderState interface - This is the main class that is responsible for the actual effect processing such as preparing the destination pass, updating the CPU shader programs, updating the GPU shader parameters etc.
* Implement the SubRenderStateFactory interface: This class will allow the RTSS to create instances of the previous class via code or script as well as export it to material script file.
* Register the factory to the RTSS using the Ogre::RTShader::ShaderGenerator::addSubRenderStateFactory method.
* Add shader files that will supply all the actual shader functions your SubRenderState needs. In order to support multiple shader languages, @ref OgreUnifiedShader are provided. These shaders should be placed in a resource location known to the resource manager.

Implementing the SubRenderState requires overriding the pure methods of the base class.
* Ogre::RTShader::SubRenderState::getType() should return unique string that identify the sub class implementation. That value is shared among all instances and can be stored in a static string variable. It uses to system to match between SubRenderState instance and the factory to should destroy it.
* Ogre::RTShader::SubRenderState::getExecutionOrder() should return integer value that will use the system to sort all SubRenderState instances of the same render state before each one of them will create its part in the CPU shader programs. Note that:
 * The execution order does not imply the order of the parameter definitions and function calls within the generated shader.
 * If an execution number is set to be the same as one of the basic fixed pipeline SRSs. Than that SRS will be built __instead__ of the fixed pipeline SRS.
* Ogre::RTShader::SubRenderState::copyFrom() a simple copy method that uses the system when coping one instance to another. **Note:** Only configuration data attributes should be copy here.
* Ogre::RTShader::SubRenderState::createCpuSubPrograms - This is the heart of this interface. This method should update the CPU shader programs with the specific details of the overriding class.

The SubRenderState supply default implementation for this method which break down this method into three stages:

@par Resolving parameters
this stage should grab all the needed parameters for this SubRenderState. Typically there several SubRenderStates working on a common set of Parameters - either to cooperate or because they use the same inputs.
Therefore parameters are not resolved by name (except for local variables), but rather by symbolic constants. These can either be of Ogre::GpuProgramParameters::AutoConstantType, which should already be familiar to you or of Ogre::RTShader::Parameter::Content.
@par
You can think of the latter as an extension of the Cg/ HLSL Semantics to the actual content of the parameter.
@par
In case of the Ogre::RTShader::FFPTransform we need the world view projection matrix and vertex shader input and output position parameters.
@par
@snippet Components/RTShaderSystem/src/OgreShaderFFPTransform.cpp param_resolve

@par Resolving dependencies
this stage should provide the name of the external shader library files that contains the actual shader code needed by this SubRenderState.
In case of the Ogre::RTShader::SRS_TEXTURING  it will add the common and texturing library for both vertex and pixel shader program.
@par
@snippet Components/RTShaderSystem/src/OgreShaderFFPTexturing.cpp deps_resolve

@par Adding function invocations
this stage creates the function calls within this SubRenderState requires. To add function invocations, you first need to obtain a Ogre::RTShader::FunctionStageRef for the respective stage.
In case of the Ogre::RTShader::SRS_FOG it will add vertex depth calculation to the vertex shader program.
@par
@snippet Components/RTShaderSystem/src/OgreShaderFFPFog.cpp func_invoc
@par
The arguments to the function are the ones you resolved in the first step and the function name must be available in one of the libraries you provided in the second step.
You can add call as many functions as you need. The calls will appear in the same order in the generates shader source code.
@note
* The ordering of the function invocation is crucial. Use the Ogre::RTShader::FFPVertexShaderStage and Ogre::RTShader::FFPFragmentShaderStage enumarations to place your invocations in the desired global order.
* Make sure the parameter semantic (in/out) in the SubRenderState code matches to your shader code implementation you supplied in the library file. GLSL will fail to link to library functions if it won't be able to find a perfect function declaration match.
* Ogre::RTShader::SubRenderState::updateGpuProgramsParams - As the name suggest this method should be overridden only in case your SubRenderState should update some parameter it created before.
* Ogre::RTShader::SubRenderState::preAddToRenderState(): this method called before adding this SubRenderState to a parent RenderState instances. It allows this SubRenderState to exclude itself from the list in case the source pass is not matching. I.E in case of SubRenderState that perform lighting calculations it can return false when the given source pass specifies that lighting calculations disabled for it.
@snippet Components/RTShaderSystem/src/OgreShaderFFPLighting.cpp disable
This method also let the SubRenderState to opportunity to modify the destination pass. I.E the Ogre::RTShader::NormalMapLighting instance adds the normal map texture unit in this context.

Implementing the Ogre::RTShader::SubRenderStateFactory is much simpler and involves implementing the following methods
* Ogre::RTShader::SubRenderStateFactory::createInstanceImpl(): This method should return instance for the SubRenderState sub class.
* Ogre::RTShader::SubRenderStateFactory::createInstance(): This method should return instance for the SubRenderState sub class using the given script compiler parameters. Implement this method if you want to be able to create your custom shader extension from material script.
* Ogre::RTShader::SubRenderStateFactory::writeInstance(): This method should write down the parameters of a given SubRenderState instance to material script file. Implement this method if you want to be able to export a material that contains your custom shader extension.

## Tips for debugging shaders {#debugging}
A couple of notes on debugging shaders coming from the RTSS:
* Call OgreBites::ApplicationContext::setRTSSWriteShadersToDisk. This will cache the generated shaders onto the disk under the directory [WRITABLE_PATH](@ref Ogre::FileSystemLayer::getWritablePath)`/RTShaderLib/cache`. This is important for 2 reasons:
  * It will make compilation problems easier to detect.
  * Once a shader is written to the disk, as long as you don't change the code behind it, the same shader will be picked up in the next application run even if its content has changed. If you have compilation or visual problems with the shader you can try to manually tinker with it without compiling the code again and again.
* Other common problems with creating shaders in RTSS usually occur from defining vertex shader parameters and using them in the pixel shader and vice versa. so watch out for those.
