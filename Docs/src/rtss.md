# RTSS: Run Time Shader System {#rtss}

@tableofcontents

# Core features of the system {#core-feats}
* Runtime shader generation synchronized with scene state. Each time scene settings change, a new set of shaders is generated.
* Full Fixed Function Pipeline (FFP) emulation. This feature is most useful combined with render system that doesn't provide any FFP functionality (OpenGL ES 2.0, D3D11 etc).
* Shader language independent interface: the logic representation of the shader programs is completely independent from the target shader language. You can generate code for different shader languages from the same program.
* Pluggable interface allows extending the target shader languages set. 
* Pluggable interface allows adding new shader based effects to the system in a seamless way. Each effect code will be automatically combined with the rest of the shader code.
* Smart program management: each shader program is created only once and may be used by multiple passes.
* Automatic vertex shader compacting mechanism: no more compacting variables by hand. In case the amount of used vertex shader output registers exceeds the maximum allowed (12 to 32, depending on [D3DPSHADERCAPS2_0.NumTemps](http://msdn.microsoft.com/en-us/library/bb172918%28v=VS.85%29.aspx)), a compacting algorithm packs the vertex shader outputs and adds unpack code in the fragment shader side.
* Material script support, for both export and import.

# System overview {#rtss_overview}

The RTSS manages a set of opaque isolated components (SubRenderStates) where each implements a specific effect.
These "effects" include Fixed Function transformation and lighting. At the core these components are plain shader files providing a set of functions; e.g. @ref FFP_FUNC_LIGHT_DIRECTIONAL_DIFFUSE, @ref FFP_FUNC_LIGHT_POINT_DIFFUSE.

Correctly ordering these functions, providing them with the right input values and interconnecting them is the main purpose of the RTSS.

To this end the RTSS defines a set of stages; e.g Ogre::RTShader::FFP_TRANSFORM, Ogre::RTShader::FFP_TEXTURING. It then queries all registered SubRenderStates which in turn attach functions given a Ogre::Pass. The stages are conceptually very similar to render queue groups.

After the RTSS has queried the SubRenderStates it continues to fill the entry function (e.g. `main()` for GLSL) by generating the actual function invocations.

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
and `$FFP_VS_TRANSFORM = [FFP_FUNC_TRANSFORM]`, `$FFP_VS_TEXTURING = [FFP_FUNC_TRANSFORM_TEXCOORD]`, it generates

```cpp
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

It will automatically use Ogre::GpuProgramParameters::AutoConstantType as needed to obtain the required inputs and route them in the respective functions.
In the above example no local parameters were allocated, but the RTSS will do it as needed. (for instance if you try to write to "vertex" in GLSL)

Now that you know what the RTSS does, you are probably wondering how to change which functions are emitted per stage to, lets say, change the lighting from the FFP style per-vertex lighting to per-pixel lighting.

The RTSS is flexible enough to "just" move the according calculations from the vertex shader to the pixel shader.

## Customising the RTSS using the API {#rtss_custom_api}

The first option is to globally enforce per-pixel lighting, you can do the following

@snippet Components/Bites/src/OgreAdvancedRenderControls.cpp rtss_per_pixel

any non FFP SRS will automatically override the default SRS for the same stage. Ogre::RTShader::FFP_LIGHTING in this case.

## Customizing the RTSS using Material Scripts {#rtss_custom_mat}

Alternatively you can enable per-pixel lighting for one material only, by adding a `rtshader_system` section to the pass as following

@snippet Samples/Media/RTShaderLib/materials/RTShaderSystem.material rtss_per_pixel

for more examples see `Samples/Media/RTShaderLib/materials/RTShaderSystem.material`.

Here are the attributes you can use in a `rtshader_system` section of a .material script:

- [lighting_stage](#lighting_stage)
- [light_count](#light_count)
- [triplanarTexturing](#triplanarTexturing)
- [integrated_pssm4](#integrated_pssm4)
- [layered_blend](#layered_blend)
- [source_modifier](#source_modifier)

<a name="lighting_stage"></a>

### lighting_stage

Force a specific lighting model.

@par
Format1: `lighting_stage <ffp|per_pixel>`
@par
Format2: `lighting_stage normal_map <texturename> [tangent_space|object_space] [coordinateIndex] [samplerName]`
@par
Example: `lighting_stage normal_map Panels_Normal_Tangent.png tangent_space 0 SamplerToUse`

@see Ogre::RTShader::NormalMapLighting::NormalMapSpace
@see @ref Samplers

<a name="light_count"></a>

### light_count

Override dynamic light count. Allows to customize which lights the RTSS will consider.
@par
Format: `light_count <pointLights> <directionalLights> <spotLights>`

<a name="triplanarTexturing"></a>

### triplanarTexturing

Force [triplanar texturing](https://www.volume-gfx.com/volume-rendering/triplanar-texturing/)
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

### integrated_pssm4
Integrated PSSM shadow receiver with 3 splits. Custom split points.
@par
Format: `integrated_pssm4 <sp0> <sp1> <sp2> <sp3>`

<a name="layered_blend"></a>

### layered_blend

Apply photoshop-like blend effects to texture layers
@par
Format: `layered_blend <effect>`
@par
Example: layered_blend luminosity

@note only applicable inside a texture_unit section

@param effect one of `default, normal, lighten, darken, multiply, average, add, subtract, difference, negation, exclusion, screen, overlay, hard_light, soft_light, color_dodge, color_burn, linear_dodge, linear_burn, linear_light, vivid_light, pin_light, hard_mix, reflect, glow, phoenix, saturation, color, luminosity`


<a name="source_modifier"></a>

### source_modifier

Apply custom modulate effect to texture layer
@par
Format: `source_modifier <operation> custom <parameterNum>`
@par
Example: `source_modifier src1_inverse_modulate custom 2`

@note only applicable inside a texture_unit section

@param operation one of `src1_modulate, src2_modulate, src1_inverse_modulate, src2_inverse_modulate`
@param parameterNum number of the custom shader parameter that controls the operation

# The RTSS in Depth {#rtss_indepth}

When the user asks the system to generate shaders for a given technique it has to provide the system a name for the target technique scheme. The system in turn, then creates a new technique based on the source technique but with a different scheme name. 
__Note:__ In order to avoid clashes the source technique must NOT contain any shaders otherwise this step will fail.

The idea behind this concept is to use Ogre's built in mechanism of material schemes, so all the user has to do in order to use the new technique is to change the material scheme of his viewport(s).

Before each viewport update, the system performs a validation step of all associated shader based techniques it created. This step includes automatic synchronization with the scene lights and fog states. When the system detects that a scheme is out of date it generates the appropriate shaders for each technique new.

The following steps are executed in order to generate shaders for a given technique:

* For each pass in the technique the system builds a set of sub render states that describe the logic process of the rendering pipeline from the draw call submission until the final pixel color.
* Each render state is translated into a set of logic shader programs (currently only pixel and vertex shader).
The logic programs are then sent to specific shader language writers that produce source code for the respective shader language. The source code is used to create the GPU programs that are applied to the destination pass.
Before rendering of an object that uses generated shaders the system allows each sub render state to update the GPU constants associated with it.

## Initializing the system

@note If you are using the OgreBites::ApplicationContext, the following steps will be taken automatically for you.

Initializing the system is composed of the following steps:
* Create the internal managers and structures via the `Ogre::RTShader::ShaderGenerator::initialize()` method.
* Set the target cache path. This is the place on your disk where the output shaders will be written to or will be read from in case they were generated by previous runs of your application.
* Verify that the location of the shader libs needed by the system is added to the ResourceGroupManager via the `Ogre::ResourceGroupManager::addResourceLocation()` method.
* Assign the target scene manager to the shader generator.
* Add one or more specialized sub-render states that are to be shared among all materials (per pixel lighting, textured fog, etc...). 

```cpp
if (Ogre::RTShader::ShaderGenerator::initialize())
{
	// Grab the shader generator pointer.
	mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();

	// Add the shader libs resource location. a sample shader lib can be found in Samples\Media\RTShaderLib
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(shaderLibPath, "FileSystem");

	// Set shader cache path.
	mShaderGenerator->setShaderCachePath(shaderCachePath);		

	// Set the scene manager.
	mShaderGenerator->addSceneManager(sceneMgr);
	
	return true;
}
```

## Creating shader based technique
This step will associate the given technique with a destination shader generated based technique. Calling the `Ogre::RTShader::ShaderGenerator::createShaderBasedTechnique()` will cause the system to generate internal data structures associated with the source technique and will add new technique to the source material. This new technique will have the scheme name that was passed as an argument to this method and all its passes will contain shaders that the system will generate and update during the application runtime.

To use the generated technique set the change material scheme of your viewport(s) to the same scheme name you passed as argument to this method.

Note that you can automate the shader generation process for all materials. First set the viewport scheme to the destination scheme of the RTSS shaders. Second register to the `Ogre::MaterialManager::Listener` and implement the `handleSchemeNotFound()` function. If the function requests a scheme for the RTSS, generate it based on functions parameters.

```cpp
// Create shader based technique from the default technique of the given material.
mShaderGenerator->createShaderBasedTechnique("Examples/BeachStones", Ogre::MaterialManager::DEFAULT_SCHEME_NAME, Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

// Apply the shader generated based techniques.
mViewport->setMaterialScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
```
![](CreateShaderBasedTech.svg)

## Runtime shader generation
During the application runtime the ShaderGenerator instance receives notifications on per frame basis from its target SceneManager.
At this point it checks the material scheme in use. In case the current scheme has representations in the manager, it executes its validate method.
The SGScheme validation includes synchronization with scene light and fog settings. In case it is out of date it will rebuild all shader generated techniques.
The first step is to loop over every SGTechnique associated with this SGScheme and build its RenderStates - one for each pass. Each RenderState has its own hash code and it is cached at the ShaderGenerator. The same RenderState can be shared by multiple SGPasses.
The second step is to loop again on every SGTechnique and acquire a program set for each SGPass. The actual acquiring process is done by the ProgramManager that generates CPU program representation, send them to a matching ProgramWriter that is chosen by the active target language, the writer generates source code that is the basis for the GPU programs.
The result of this entire process is that each technique associated with the SGScheme has vertex and pixel shaders applied to all its passes. These shaders are synchronized with scene lights and fog settings.

![](RuntimeShaderGeneration.svg)

## Main components {#rtss__components}
The following is an partial list of components within the RTSS. These components are listed as they have great importance in understanding controlling and later extending the RTSS system.

### ShaderGenerator
The ShaderGenerator is the main interface to the RTSS system. Through it you can request to generate and destroy the shaders, influence from what parts to create the shaders, and control general system settings such as the shading language and shader caching.

### RenderState classes
A render state describes the different components that a shader will be created from. These components are referred to as SubRenderStates. 

RenderStates exist on 2 levels: scheme and pass. Scheme RenderStates describe the SubRenderStates that will be used when creating a shader for a given material scheme. Pass RenderState describe the SubRenderStates that will be used when creating a specific pass of a specific material. When a shader is generated for a given material the system combines the SubRenderStates from both RenderStates to create a shader specific for a material pass in a specific scheme.

### SubRenderState classes
Sub-render states (SRS) are components designed to generate the code of the RTSS shaders. Each SRS usually has a specific role to fill within the shader's construction. These components can be combined in different combinations to create shaders with different capabilities.

There are 5 basic SRSs. These are used to recreate the functionality provided by the fixed pipeline and are added by default to every scheme RenderState:
* Ogre::RTShader::FFPTransform - responsible for adding code to the vertex shader which computes the position of the vertex in projection space
* Ogre::RTShader::FFPColour - responsible for adding code to the shaders that calculate the base diffuse and specular color of the object regardless of lights or textures. The color is calculated based on the ambient, diffuse, specular and emissive properties of the object and scene, color tracking and the specified hardware buffer color.
* Ogre::RTShader::FFPLighting - responsible for adding code to the shaders that calculate the luminescence added to the object by light. Then add that value to the color calculated by the color SRS stage.
* Ogre::RTShader::FFPTexturing - responsible for adding code that modulates the color of the pixels based on textures assigned to the material.
* Ogre::RTShader::FFPFog - responsible for adding code that modulates the color of a pixel based on the scene or object fog parameters.

There are many more sub render states that already exist in the Ogre system and new ones can be added. Some of the existing SRSs include capabilities such as: per-pixel lighting, texture atlas, advanced texture blend, bump mapping, efficient multiple lights (sample), textured fog (sample), etc...

### SubRenderStateFactory
As the name suggests, sub render state factories are factories that produce sub render states. Each factory generates a specific SRS. 

These type of components are note worthy for 2 reason. The first and obvious one is that they allow the system to generate new SRSs for the materials it is asked to generate. The second reason is that they perform as script readers and writers allowing the system to create specific or specialized SRSs per material.

## Creating custom shader extensions {#creating-extensions}
Although the system implements some common shader based effects such as per pixel lighting, normal map, etc., you may find it useful to write your own shader extensions.

In order to extend the system with your own shader effects you'll have to follow these steps:
* Implement the SubRenderState interface - This is the main class that is responsible for the actual effect processing such as preparing the destination pass, updating the CPU shader programs, updating the GPU shader parameters etc.
* Implement the SubRenderStateFactory interface: This class will allow the RTSS to create instances of the previous class via code or script as well as export it to material script file.
* Register the factory to the RTSS using the Ogre::RTShader::ShaderGenerator::addSubRenderStateFactory method.
* Add shader files that will supply all the actual shader functions your SubRenderState needs. In order to support multiple shader languages you should supply code for your entire desired target shading languages (CG, HLSL, GLSL etc). These files should be placed in a way that the resource manager could access them. This can be done by placing them in a valid resource location or by dynamically adding resource location.

Implementing the SubRenderState requires overriding the pure methods of the base class.
* Ogre::RTShader::SubRenderState::getType() should return unique string that identify the sub class implementation. That value is shared among all instances and can be stored in a static string variable. It uses to system to match between SubRenderState instance and the factory to should destroy it.
* Ogre::RTShader::SubRenderState::getExecutionOrder() should return integer value that will use the system to sort all SubRenderState instances of the same render state before each one of them will create its part in the CPU shader programs. Note that:
 * The execution order does not imply the order of the parameter definitions and function calls within the generated shader.
 * If an execution number is set to be the same as one of the basic fixed pipeline SRSs. Than that SRS will be built __instead__ of the fixed pipeline SRS.
* Ogre::RTShader::SubRenderState::copyFrom() a simple copy method that uses the system when coping one instance to another. **Note:** Only configuration data attributes should be copy here.
* Ogre::RTShader::SubRenderState::createCpuSubPrograms - This is the heart of this interface. This method should update the CPU shader programs with the specific details of the overriding class.

The SubRenderState supply default implementation for this method which break down this method into three stages:
* Resolving parameters: this stage should grab all the needed parameters for this SubRrenderState. In case of the FFPTransform it should resolve the world view projection matrix and vertex shader input and output position parameters.

@snippet Components/RTShaderSystem/src/OgreShaderFFPTransform.cpp param_resolve

* Resolving dependencies: this stage should provide the name of the external shader library files that contains the actual shader code needed by this SubRenderState.
In case of the FFPTexturing  it will add the common and texturing library for both vertex and pixel shader program.

@snippet Components/RTShaderSystem/src/OgreShaderFFPTexturing.cpp deps_resolve

* Adding function invocations: this stage creates the function calls within this SubRenderState requires. Each function call has two keys that are used by the system to sort it before generating the actual shader code as well as set of in/out parameters.
A function invocation is added to either vertex shader program or fragment shader program.
In case of the FFPFog it will add vertex depth calculation to the vertex shader program.

@snippet Components/RTShaderSystem/src/OgreShaderFFPFog.cpp func_invoc

Note: 
* Each Ogre::RTShader::SubRenderState can add as many function invocations as it needs.
* Each Ogre::RTShader::SubRenderState can different function invocations in different ordering. 
* The ordering of the function invocation is crucial. Use the FFPVertexShaderStage and FFPFragmentShaderStage enumarations to place your invocations in the desired order.
* Make sure the parameter semantic (in/out) in the SubRenderState code matches to your shader code implementation you supplied in the library file. GLSL will fail to link to libray functions if it won't be able to find a perfect function declaration match. 
* Ogre::RTShader::SubRenderState::updateGpuProgramsParams - As the name suggest this method should be overridden only in case your SubRenderState should update some parameter it created before.
* Ogre::RTShader::SubRenderState::preAddToRenderState(): this method called before adding this SubRenderState to a parent RenderState instances. It allows this SubRenderState to exclude itself from the list in case the source pass is not matching. I.E in case of SubRenderState that perform lighting calculations it can return false when the given source pass specifies that lighting calculations disabled for it.

@snippet Components/RTShaderSystem/src/OgreShaderFFPLighting.cpp disable

This method also let the SubRenderState to opportunity to modify the destination pass. I.E the NormalMapLighting instance adds the normal map texture unit in this context.

Implementing the Ogre::RTShader::SubRenderStateFactory is much simpler and involves implementing the following methods
* Ogre::RTShader::SubRenderStateFactory::createInstanceImpl(): This method should return instance for the SubRenderState sub class.
* Ogre::RTShader::SubRenderStateFactory::createInstance(): This method should return instasnce for the SubRenderState sub class using the given script compiler parameters. Implemet this method if you want to be able to creat your custom shader extension from material script.
* Ogre::RTShader::SubRenderStateFactory::writeInstance(): This method should write down the parameters of a given SubRenderState instance to material script file. Implement this method if you want to be able to export a material that contains your custom shader extension.

## Tips for debugging shaders {#debugging}
A couple of notes on debugging shaders coming from the RTSS:
* Call OgreBites::ApplicationContext::setRTSSWriteShadersToDisk. This will cache the generated shaders onto the disk under the directory [OGRE_MEDIA_DIR](@ref cmake)`/RTShaderLib/cache`. This is important for 2 reasons:
  * It will make compilation problems easier to detect.
  * Once a shader is written to the disk, as long as you don't change the code behind it, the same shader will be picked up in the next application run even if its content has changed. If you have compilation or visual problems with the shader you can try to manually tinker with it without compiling the code again and again.
* Add a breakpoint in OgreShaderProgramManager.cpp at
@snippetlineno Components/RTShaderSystem/src/OgreShaderProgramManager.cpp debug_break 
If a shader will fail to compile it will usually fail there. Once that happens you can find the shader name under the `programName` parameter, then look for it in the cache directory you created.
* Other common problems with creating shaders in RTSS usually occur from defining vertex shader parameters and using them in the pixel shader and vice versa. so watch out for those.