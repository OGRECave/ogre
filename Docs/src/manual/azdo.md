
AZDO changes (Aproaching Zero Driver Overhead) {#azdo}
==============================================

@tableofcontents

# V2 and v1 objects {#V2AndV1Objects}

Because of the big overhaul that Ogre 2.0 Final went through, there were
a lot of optimizations that were just incompatible with how the old code
worked. On the other hand, removing the old code was unwise as there are
a lot of features that haven't been ported to v2 interfaces yet.

The solution was to build a parallel system that runs alongside the old
one; enclosing the old ones in the v1 namespace.

As a result, `Entity` got replaced by `Item`; and thus now to access
them, you need to write `v1::Entity` and Item.

Some objects may have the same name but live in a different namespace
and thus are not the same: `v1::Mesh` (defined in OgreMesh.h) and `Mesh`
(defined in OgreMesh2.h)

## Longevity of the v1 objects and deprecation {#V2AndV1ObjectsV1Longevity}

The initial plan was to remove all v1 functionality once they have their
v2 counterpart.

However after evaluating community feedback and the tech involved; this
isn't always the case as there is v1 functionality that will not be
ported to v2 but is critical to several commercial business relying on
OGRE. The following functionality that lives in the v1 namespace will
remain to be supported:

-   Software Skinning: SW Skinning will not be ported to v2 because
    supporting it brings down overall efficiency and adds a
    disproportionate amount of complexity. However, this feature is
    relevant for applications wanting to skeletally animate models in a
    deterministic way regardless of the hardware and drivers it's
    running on, and read back the results from CPU very quickly.
    Therefore we will continue to support `v1::Entity` and `v1::Skeleton`.
-   Software Pose/Morph animations: The reasons are exactly the same as
    SW Skinning.
-   `v1::Skeleton`: The new v2 Skeletal animation system was written for
    animating large crowds with high performance; and replace the old
    system. However, after experimenting with the new system, while it
    is clear that it runs significantly faster; it became apparent that
    it is very difficult and inefficient to alter the bone hierarchy and
    its animations once it has been built and baked. In other words, v1
    skeleton is very flexible for realtime editing, while v2 skeleton is
    very fast for playback. For this reason, we will continue to support
    v1 skeletons for a very long time. It is also possible to easily
    import v1 skeletons into v2 skeletons; which is very convenient for
    animation tools who may want the best performance once they switch
    to their "playback mode" (i.e. final rendering started) after the
    modeler finished editing the skeleton.

# Render Queue {#RenderQueue}

The RenderQueue has been completely refactored. It sorts based on a
64-bit hash which is calculated in `RenderQueue::addRenderable`. Mesh ID,
material ID, texture hash, depth, macro and blend block IDs are taken
into account[^13].

Each RenderQueue ID can run in any of the following modes of operation:

1.  `V1_LEGACY`: It runs as closely as possible as Ogre 1.x ran. Only low
    level materials and mobile Hlms materials can work in this mode; and
    only v1 objects can be stored in RenderQueue IDs operating in this
    mode. This mode of operation is not recommended for a large number
    of objects.
2.  `V1_FAST`: Certain obscures features from Ogre 1.x won't be available
    (i.e. the HW GlobalInstance buffer). The RenderQueue will first
    iterate through all objects, auto-instancing when possible; and
    updating the shader's constant and texture buffers; then using the
    Command Buffer all necessary state changes and draw calls. Only v1
    objects can live in these RenderQueue Ids. Originally they could
    only use desktop Hlms materials, but recently support for Low Level
    materials was added. Note that low level materials cannot be auto
    instanced.
3.  `FAST`: The new system. It's similar to `V1_FAST`. However, only v2
    objects can be used and they must be using dekstop Hlms materials.
    The API overhead is extremely low; and is more multicore-friendly
    because `RenderQueue::addRenderable` is executed in parallel.

You cannot mix v1 and v2 objects in the same RQ ID, however you can
store them in different RQ IDs.

RQ Mode    | Object type | Hlms materials (Desktop) | Hlms materials (Mobile) | Low Level materials
-----------|-------------|--------------------------|-------------------------|--------------------
V1\_LEGACY | v1          | NO                       | YES                     | YES
V1\_FAST   | v1          | YES                      | NO                      | YES
FAST       | v2          | YES                      | NO                      | NO

It is not automatic. Placing a v2 object in a v1 queue or viceversa may
result in Ogre raising a helpful exception, or an unhelpful crash (it is
hard to track all sources of RQ missmatches); however with a debugger
hooked, it can be easy to spot the problem by looking at the routine
names (e.g. look at the v2/v1 suffixes) and the mVaoPerLod member being
non empty (which indicates a v2 object), or the usage of
`Renderable::getRenderOperation` and `v1::RenderOperation`, which
indicates a v1 object.

By default queue ID 0 is set to `FAST` (v2) while ID 1 is set to `V1_FAST`
(v1); and v2 objects will try to default to ID 0 while v1 objects will
try to default to ID 1. You can change the render queue modes with the
following snippet:

```cpp
mSceneManager->getRenderQueue()->setRenderQueueMode(5,Ogre::RenderQueue::Modes::V1_FAST);
Entity*entity;
entity->setRenderQueueGroup(5);	 
```

In this example, it sets the queue ID 5 to `V1_FAST`, and later adds the
entity to that queue. The RenderQueue mode can be changed later (however
you will have to move the Entity out of the queue ID into another valid
one by yourself, otherwise Ogre may crash)

# The VaoManager {#VaoMaanger}

It got its name as it was originally developed to handle just
`VertexArrayObjects`; which is a concept borrowed from OpenGL: a vao is an
object that stores the vertex attribute definitions and its vertex and
index buffer bindings.

However the VaoManager quickly evolved into a central low level memory
management for GPU memory. The name just stuck on since. The `VaoManager`
works by requesting a pool of GPU memory to the API by a fixed length,
and will not request more GPU memory until the manager ran out of it.

This manager is responsible to create the Vertex, Index, Constant,
Texture and Index Buffers, and does so by reserving a space from the
same pool of memory available (when allowed by the API[^14])

This system is written under the assumption that the API performs no
hazard tracking at all, thus making it an excellent choice for OpenGL
3/4+, DX12, and Mantle.

Dynamic buffers will allocate 3x more memory than requested to handle a
triple buffer scheme and avoid stalls. Fences[^15] are frame-wide to
reduce driver overhead. A user is expected to fill all the data again
from a dynamic buffer; as it is not suitable for performing only partial
updates.

To perform partial updates, use DEFAULT buffers, and update them via
staging buffers (see Hlms PBS implementation)

[^13]: For more information, see *Order your graphics draw calls
    around!* (<http://realtimecollisiondetection.net/blog/?p=86>) and
    *Rough sorting by Depth*
    (<http://aras-p.info/blog/2014/01/16/rough-sorting-by-depth/>)
