# New and Noteworthy in OGRE 13

This is only a high level overview. For detailed changes, see the git changelog.

**The versioning scheme changed with this release. This is the successor to Ogre 1.12 and the porting efforts are comparable with porting from 1.11 to 1.12.**

## Core changes
### Zero external dependencies
Ogre was put on a dependency diet and now you can get a fully functional build without any additional dependencies. We replaced zlib and libzzip in Main by miniz and use a bundled stb_truetype for Overlays if you dont provide Ogre with freetype.

While miniz provides a seamless replacement, there are visual differences when using stb_truetype due to differences in glyph metrics.

### Node-less positioning API disabled by default
Traditionally Ogre allowed you to place Cameras and Lights in the scene without attaching them to a SceneNode. This resulted in three different positioning APIs for Cameras, Lights and SceneNodes with slight inconsistencies (e.g. `Camera::move` vs. `SceneNode::translate`).
Furthermore, this complicated things when e.g. attaching a Camera to a SceneNode as you might have an additional transform "hidden" inside Camera.

Therefore, the use of node-less positioning was deprecated since Ogre 1.10.8. With this release we disable the according API and no longer store any transforms inside Light and Camera so you have to port your code like:
```cpp
// Ogre 1.12 (warning: ‘setDirection’ is deprecated)
mLight->setDirection(...);

// Ogre 13 (also 1.12 to get rid of warnings)
mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mLight);
mLight->getParentSceneNode()->setDirection(...);
```
Alternatively, you can set `OGRE_NODELESS_POSITIONING=ON` in CMake to get back the old behaviour.

### Unified storage of GpuProgramParams
All constants are stored in a single byte-buffer instead of being split by
type.
This maps to how Uniform/ Constant buffers work and allows updating a backing hardware buffer with a single `memcpy` instead of having to explicitly collect the individual floats.
Consequently, `physicalIndex` is now a byte offset instead of a slot offset.
The high-level APIs based on `String` names and `logicalIndex` are not affected by this change.

### Unified HighLevelGpuPrograms & GpuPrograms
`HighLevelGpuProgramPtr` is now an alias for `GpuProgramPtr`. While the class separation still exists, user code should use both interchangeably.
The only user-visible change is that calls `setPreprocessorDefines(...)` should be be changed to `setParameter("preprocessor_defines", ...)`.

Furthermore, `HighLevelGpuProgramManager` is now merely an alias for `GpuProgramManager`.  Internally, this saves us quite some shunting. Most users should not notice - unless you were creating low-level and high-level programs with the same name. This will trigger an error now, as both live in the same resource-namespace.

### Private class visibility
Historically, Ogre only used only public and protected visibility. While this makes it easy to customise Ogre classes for a specific use, it is not so great when it comes to committing to a stable API.
Previously, you could (accidentally) use a inherited class member that we would later change during an internal refactor, causing your code to break. While we could exclude all protected members from our API guarantee, it would be even better if the compiler would check that you only use the stable API - thats exactly what private visibility does.
Consequently, most protected members are private now.

### `shadow_*_program_ref` API removed
`shadow_caster_material` and `shadow_receiver_material` are better alternatives available since Ogre 1.6. These not only simplify code, but also allow for efficient caching of generated shaders.

Nevertheless, there is fallback code so you still can use old `.material` files. Here, a caster/ receiver material will be generated on demand. However, you are highly encouraged to define those explicitly.

### `texture_alias` API removed
Texture aliases were a restricted version of Script Variables, which you should instead.
I.e. replace

```cpp
texture_alias DiffuseMap
// and
set_texture_alias	DiffuseMap	r2skin.jpg
```

by

```cpp
texture $DiffuseMap
// and
set     $DiffuseMap r2skin.jpg
```

There is limited backward compatibility for material scripts, where `texture_alias` will set the texture unit name and later `set_texture_alias` is applied based on that.

### Other
- Size of the Particle class was reduced by 12% which results in about 13% faster rendering
- Frustum is not a Renderable any more. Rendering Frusta is now done by the DebugDrawer.
- ShadowTextureListener was factored out of SceneManager::Listener
- SWIG 4.0 is now fully supported
- Flollowing SemVer, incrementing PATCH now guarantees ABI compatibility. Therefore SOVERSION on Unix system now only contains two digits e.g. `libOgreMain.so.13.0` instead of `libOgreMain.so.1.12.13`.
- since 13.3, `PF_DEPTH24_STENCIL8` is available


### Breaking non-API changes

- `VET_COLOUR*` is now an alias for `VET_UBYTE4_NORM`. On most RenderSystems VET_COLOUR would be already mapped to VET_UBYTE4_NORM anyway. A notable exception is D3D9, where this changes the channel order from BGRA to RGBA. If you are using custom vertex shaders, you must update them accordingly. For all other cases Ogre will handle this for you. If a buffer conversion is performed on loading, Ogre will issue a warning in the log.
- The default transform space of `SceneNode::setDirection` changed from `TS_LOCAL` to `TS_PARENT` to aid with the node-less positiong transiton.
- The `parentWindowHandle` and `externalWindowHandle` Window creation parameters are now equivalent on all platforms.
  - On Win32 the `parentWindowHandle` behaviour is gone and no child window is ever created.
  - On Linux the `externalWindowHandle` behaviour is gone and an external `GLXDrawable` can be only set via an external context.

## Overlay
`DisplayString` is now merely an alias for `String`. The `UTFString` class was replaced by a much smaller (and faster) implementation that is not exposed. For all practical purposes, you can just pass UTF8 encoded strings through `String`.

Glyph placement has been improved. Previously Ogre only considered glyph width for typesetting. However, this did not take into account that glphs can overlap, as happens with italic fonts. Therefore, there is now an independent property `GlyphInfo::advance`. Ogre internal code was updated for this change. However, if you directly access the texture atlas, you have to update your code.

An Exception is now thrown, when querying a `GlyphInfo` of a Font that is not loaded. Make sure to call `font->load()` before querying.

## RTSS
There is now a  RTSS stage for Weighted, blended Order Independent Transparency, which implements the according algorithm by Morgan McGuire.
See the transparency Sample for how to integrate it into your pipeline.

The RTSS Normal Map stage, is no longer a lighting stage but only does normal mapping.
This allows using it with per-pixel lighting as before, but also enables combining it with the GBuffer stage to enable normal mapping in the GBuffer.

**ACTION REQUIRED** Direct specification of sampler parameters via `lighting_stage normal_map ...` is no longer supported and the additional parameters will be interpreted as a sampler reference.

The PSSM3 stage now also supports colour shadows in addition to depth shadows. Colour shadows are automatically used for `PCT_BYTE` texture formats.
Since 13.3, the PSSM3 stage supports the "Reversed Z-Buffer" RenderSystem option.

### PBR Material support (since 13.3)

The RTSS can now be used to create a PBR pipeline. To enable it via material scripts, specify

```nginx
rtshader_system
{
   lighting_stage metal_roughness texture Default_metalRoughness.jpg
}
```

The parameters are expected to be in the green and blue channels (as per glTF2.0) and lighting will be done according to the [Filament equations](https://google.github.io/filament/Filament.md.html#materialsystem).

Alternatively, you can use material-wide settings, by skipping the texture part like:

```nginx
rtshader_system
{
   lighting_stage metal_roughness
}
```

Here, metalness is read from `specular[0]` and roughness from `specular[1]`.

## DotScene
The Plugin now supports exporting via a generic `SceneNode::saveChildren` API. This allows you to dump your dynamically generated Scene to file and later inspect it with ogre-meshviewer, which also got improved .scene support.

## D3D11
Shader interfaces/ subroutines are no longer available. They were cumbersome to use, while adding overhead to all rendersystems.

Constant buffers are now fully supported, so if you write `cbuffer Foo` in HLSL, you get the data that you declared previously as `shared_params Foo` in your Ogre material script.

## GL3Plus
Atomic buffers are no longer supported. They were only partially implemented, while being cumbersome to use and adding overhead to the core loop. SSBOs offer the same functionality, while proving a convenient API.

Separate shader objects are now used by default, if available. This enables an improved shader parser using the GL introspection API and allows mixing GLSL and SPIRV shaders.

## D3D9

The Option "Multi device memory hint" was renamed to "Auto hardware buffer management" as it is generally enables memory recovery on device loss.

Since 13.2.1 there is support for `PF_DEPTH16` textures, which can be part of a MRT too.

Since 13.3, the first 4 textures are shared between vertex and fragment programs. Explicitly stating BindingType for vertex textures fetch is no longer needed.

## Vulkan (since 13.2)

The RenderSystem does not yet support all of Ogre features, but many common use-cases are already covered. There are two caveats though:

1. *Buffer updates*: Ogre does not try to hide the asynchronicity of Vulkan from the user and rather lets you run into rendering glitches. The solution here is to either implement triple-buffering yourself or discard the buffer contents on update (`HBL_DISCARD`), which will give you new memory on Vulkan.
2. *Rendering interruption*: Closely related to the above is rendering interruption. This means that after the first Renderable was submitted for the current frame, you decide to load another Texture or update a buffer. Typically, it is possible to schedule your buffer updates before rendering kicks off. Additionally, this improves performance on tile-based hardware (mobile).

## GLSLang Plugin

This new Plugin enables Ogre to automatically compile GLSL shaders to SPIRV that in turn will be loaded by GL3+.
This means you can now easily use the Khronos reference compiler instead of what your driver would do - similar to how things work on D3D11.

## HLMS
The component was removed. The RTSS or just plain shaders in Ogre materials are a better way forward.