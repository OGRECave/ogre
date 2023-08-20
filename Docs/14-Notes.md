# New and Noteworthy in OGRE 14
This is only a high level overview. For detailed changes, see the git changelog.

## Core changes
Following the removal of `UTFString` in Ogre 13, the generic data structures `AnyNumeric` and `HashedVector` were removed in order to reduce code complexity.
If you were using them, consider copying the code from Ogre 13 into your project.

The option to store tangents with `TEXCOORD` semantic was removed. The `TANGENT` semantic is now used unconditionally.

On GL, D3D11 and Vulkan VertexWinding is set in terms of FrontFace, so `gl_FrontFacing` and `SV_IsFrontFace` now behave as expected.

Object space bones are now enabled by default. Use `MeshManager::setBonesUseObjectSpace(false)` to go back to old behaviour.

All builtin Factory classes were made private. Replace `DefaultSceneManagerFactory::FACTORY_TYPE_NAME` by `SMT_DEFAULT` and e.g. `ManualObjectFactory::FACTORY_TYPE_NAME` by `MOT_MANUAL_OBJECT`.

Since 14.1, `ACT_MATERIAL_LOD_INDEX` is available to implement multiple material LOD levels in a single shader.

### OgreUnifiedShader.h

Sampler definitions now implicitly include the `uniform` keyword to support Vulkan; i.e. this will generate an error:

```cpp
uniform SAMPLER2D(sampler, 0);
// with Ogre 14 use:
SAMPLER2D(sampler, 0);
```

You opt-in to this syntax with Ogre 13, by defining `USE_OGRE_FROM_FUTURE` before including `OgreUnifiedShader.h`.

### Task-based WorkQueue

The WorkQueue API was changed from request/ response style to a task-based style.
I.e. instead of sending messages between threads, the actual work is send as a callback.

Or speaking with code, whereas before you would do:
```cpp
Root::getSingleton().getWorkQueue()->addRequest(mWorkQueueChannel, WORKQUEUE_DERIVED_DATA_REQUEST, data);
```

With the new API you do:
```cpp
Root::getSingleton().getWorkQueue()->addTask(
    [this, req]()
    {
        auto req = new WorkQueue::Request(0, 0, data, 0, 0);
        auto res = handleRequest(r, NULL);
        Root::getSingleton().getWorkQueue()->addMainThreadTask([this, res]() {
            handleResponse(res, NULL);
            delete res;
        });
    });
```

the new version might seem more lengthy, but actually it requires less code, while being more explicit.
There is neither a request handler nor a response handler interface that you must implement.
Also the WorkQueue-channel concept is gone.
Ultimately, the Request and Response classes are not necessary either and are here for illustrative purposes.

At this, the custom ticketing mechanism of `ResourceBackgroundQueue` was replaced by the more modern use of `std::future`.
So where you would previously do:
```cpp
Ogre::WorkQueue::RequestID ticket = rbq.prepare(tex->getCreator()->getType(), tex->getName(), tex->getGroup());
...
rbq.isProcessComplete(ticket);
```
you now do:
```cpp
auto future = rbq.prepare(tex);
...
future.wait_for(std::chrono::milliseconds(0));
```

### Transparent objects are now on their own render queue

Transparent objects are now put on the `RENDER_QUEUE_TRANSPARENTS` queue, which is rendered after `RENDER_QUEUE_SKIES_LATE`.
This fixes compatibility with late sky rendering, but more importantly makes transparent objects easily accessible from compositor scripts.

This simplifies many advanced algorithms, such as deferred shading, where you want to render transparent objects after the opaque ones.

### Breaking non-API changes

The second argument of `RenderQueueListener::renderQueueStarted` now contains the current camera name and hence is always non-empty.

## Rectangular Area Lights (since 14.1)

A new spot light type `LT_RECTLIGHT` has been introduced along with the `setSourceSize` method, enabling the rendering of rectangular area lights. In order to process this light type in your shader, verify that the `spotlight_params.w` autoparam is equal to 2. Then, `spotlight_params.xyz` contains the light width in the view space, and `light_attenuation.xyz` contains the light height.

## Python

The double `ImGui` namespacing in `Ogre.ImGui` enums was removed; e.g. `ImGui.ImGuiWindowFlags_NoTitleBar` is now `ImGui.WindowFlags_NoTitleBar`.
Since 14.1, `Checkbox` and multidimensional input (e.g. `ColorEdit3` `InputFloat2`) are correctly wrapped.

Since 14.1, primitive types like `Ogre.Vector3`, `Ogre.ColourValue` etc. behave like Python sequences and can be used with `len()` and expanded `myfun(*Ogre.Quaternion())`.
Additionally, `Ogre.Degree` can be transparently used where `Ogre.Radian` is required.

## RTShader System

The PSSM3, TriplanarTexturing and GBuffer sub-renderstates are now only configurable through the generic `setParameter()` method.

Light types are now dynamically handled inside the shader. Shader recompilation is no longer required when changing the light type.
Consequently, the `light_count` property now only takes a single count instead of a count per light type.

The default (FFP) sub-renderstates are now added to the scheme render state first instead of last. This means that you can inspect and modify them in the scheme render state.

The normal maps should now be specified at the `texture_unit` level instead of the `pass` level

```nginx
// with Ogre 13
pass
{
    rtshader_system
    {
        lighting_stage normal_map Panels_Normal_Tangent.png tangent_space 0 RTSS/NormalMapSampler
    }
}

// with Ogre 14
pass
{
    texture_unit
    {
        texture Panels_Normal_Tangent.png
        sampler_ref RTSS/NormalMapSampler
        rtshader_system
        {
            normal_map tangent_space
        }
    }
}
```

Additionally `parallax_occlusion` mapping is now supported by using the respective keyword. It is also available with the Terrain Component.

## Terrain

The TerrainMaterialGenerator API was simplified by making the use of a Profile optional.
Furthermore, the required TerrainLayer metadata was reduced to a minimum.
If you have a custom `TerrainMaterialGenerator`, you will need to update it.

## Bullet

A separate `CollisionWorld` wrapper was split off from `DynamicsWorld`, enabling the use of Bullet for collision detection only.

## MeshLodGenerator

The MeshLodGenerator now also support line primitives and correctly handles the boundaries between SubMeshes. The latter needs to be explicitly enabled, as it increases LOD generation time and is not required, if only a single submesh is used.

## MeshUpgrader

The interactive mode has been removed. Edge lists are no longer generated by default. Pass `-el` to generate them.

## GL3Plus

Transform feedback outputs are now consistently named `xfb_position`, `xfb_uv0` instead of `oPos`, `oUv0`.
Shader updates are required.

On OSX, `texture2D` etc. functions are no longer implicitly upgraded to version 150 equivalents, explicitly `#include <GLSL_GL3Support.glsl>` to get them.
