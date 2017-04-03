
Threading {#threading}
=========

Ogre 2.0 uses synchronous threading for some of its operations. This
means the main thread wakes up the worker threads, and waits for all
worker threads to finish. It also means users don't have to be worried
that Ogre is using CPU cores while the application is outside a
`renderOneFrame` call.

@tableofcontents

# Initializing {#ThreadingInitializing}

The number of worker threads must be provided by the user when creating
the SceneManager:

```cpp
const size_t numThreads = 4;
InstancingTheadedCullingMethod
        threadedCullingMethod = INSTANCING_CULLING_THREADED;

mSceneMgr = mRoot->createSceneManager( ST_GENERIC,numThreads,
                                       threadedCullingMethod,
                                       "ExampleSMInstance" );
```

The other threading parameter besides the number of threads, is the
threading strategy used for Instancing, which can be single threaded or
multithreaded.

## Ideal number of threads {#ThreadingInitializingNumberOfThreads}

The threading model is synchronous, and meant to be used for tasks that
take roughly the same amount of time in each thread (which is a very
important assumption!). The ideal number of worker threads is the number
of logical cores exposed by the CPU (excluding hyperthreading cores).

Spawning more threads than cores will oversubscribe the system and won't
run faster. In fact it should only slow it down.

If you plan to use a whole core for your own computations that will run
in parallel while renderOneFrame is working (i.e. one thread for
physics) and take a significant cpu time from that core; then in this
case the ideal number of threads becomes number\_of\_logical\_cores – 1

Whether increasing the number of threads to include hyperthreading cores
improves performance or not remains to be tested.

## More info about InstancingThreadedCullingMethod {#ThreadingInitializingCullingMethod}

There are two Instancing techniques that perform culling of their own:

-   HW Basic
-   HW VTF

Frustum culling is highly parallelizable & scalable. However, we first
cull InstanceBatches & regular entities, then ask the culled
InstanceBatches to perform their culling to the InstancedEntities they
own.

This results performance boost for skipping large amounts of instanced
entities when the whole batch isn't visible. However, this also means
threading frustum culling of instanced entities got harder.

There were four possible approaches:

-   Ask all existing batches to frustum cull. Then use only the ones we
    want. Sheer brute force. Scales very well with cores, but sacrifices
    performance unnecessary when only a few batches are visible. This
    approach is not taken by Ogre.
-   Sync every time an InstanceBatchHW or InstanceBatchHW\_VTF tries to
    frustum cull to delegate the job on worker threads. Considering
    there could be hundreds of InstanceBatches, this would cause a huge
    amount of thread synchronization overhead & context switches. This
    approach is not taken by Ogre.
-   Each thread after having culled all InstancedBatches & Entities,
    will parse the culled list to ask all MovableObjects to perform
    culling of their own. Entities will ignore this call (however they
    add to a small overhead for traversing them and calling a virtual
    function) while InstanceBatchHW & InstanceBatchHW\_VTF will perform
    their own culling from within the multiple threads. This approach
    scales well with cores and only visible batches. However load
    balancing may be an issue for certain scenes: eg. an InstanceBatch
    with 5000 InstancedEntities in one thread, while the other three
    threads get one InstanceBatch each with 50 InstancedEntities. The
    first thread will have considerably more work to do than the other
    three. This approach is a good balance when compared to the first
    two. **This is the approach taken by Ogre when
    INSTANCING\_CULLING\_THREADED is on**
-   Don't multithread instanced entitites' frustum culling. Only the
    InstanceBatch & Entity's frustum culling will be threaded. **This is
    what happens when INSTANCING\_CULLING\_SINGLE is on**.

Whether INSTANCING\_CULLING\_THREADED improves or degrades performance
depends highly on your scene.

When to use INSTANCING\_CULLING\_SINGLETHREAD?

If your scene doesn't use HW Basic or HW VTF instancing techniques, or
you have very few Instanced entities compared to the amount of regular
Entities.

Turning threading on, you'll be wasting your time traversing the list
from multiple threads in search of InstanceBatchHW &
InstanceBatchHW\_VTF

When to use INSTANCING\_CULLING\_THREADED?

If your scene makes intensive use of HW Basic and/or HW VTF instancing
techniques. Note that threaded culling is performed in SCENE\_STATIC
instances too. The most advantage is seen when the instances per batch
is very high and when doing many PASS\_SCENE, which require frustum
culling multiple times per frame (eg. pssm shadows, multiple light
sources with shadows, very advanced compositing, etc)

Note that unlike the number of threads, you can switch between methods
at any time at runtime.

# What tasks are threaded in Ogre {#ThreadingInOgre}

The following tasks are partitioned into multiple threads:

-   **Frustum culling:** The pool of all visible Entities,
    InstanceBatches, etc are frustum culled in multiple threads and
    added to a culled list, one per thread. When all threads are done,
    the main thread collects the results from all lists.
-   **Culling the receiver's box:** Very specific to shadow nodes. When
    a render\_scene pass uses (for example) render queues 4 to 8, but
    the shadow node users render queues 0 to 8; the shadow node needs
    receiver's aabb data from RQs 0 to 3; which aren't available. It is
    very similar to frustum culling; except that the cull list isn't
    produced, only the aabb is calculated. Since aabb merges are
    associative: ![](aabb_merge.svg)
    we can join the results from all threads after they're done.
    In fact, we even exploit this associative property to process them
    using SIMD.
-   Node transform updates: Updating all scene nodes' derived position
    and orientation by inheriting from their parent's derived position &
    orientation. We have to wait for every parent level depth due to
    data dependencies.
-   Updating all bounds: Updating the World AABB by applying the node's
    transform to the local aabbs. The World AABB is needed for correct
    frustum culling, among other things.
-   Frustum culling instanced entities: [See previous
    section](#5.1.1.More info about InstancingThreadedCullingMethod|outline).

# Using Ogre's threading system for custom tasks {#ThreadingCustomTasks}

While often users may want to user their own threading system; it is
possible to ask Ogre to process their own task using its worker threads.
Users need to inherit from `UniformScalableTask` and call
`SceneManager::executeUserScalableTask`.

The following example prints a message to the console from the multipler
worker threads:

```cpp
#include <Threading/OgreUniformScalableTask.h>

class MyThreadedTask : public Ogre::UniformScalableTask
{
public:
    virtual void execute( size_t threadId, size_t numThreads )
    {
        printf( "Hello world from thread %i",threadId );
    }
};

int main()
{
    // assumes Ogre is initialized and sceneMgr is a valid ptr
    Ogre::SceneManager *sceneMgr;

    MyThreadedTask myThreadedTask;
    sceneMgr->executeUserScalableTask( myThreadedTask,true );

    return 0;
}
```

Parameter `threadId` is guaranteed to be in range \[0; numThreads) while
parameter `numThreads` is the total number of worker threads spawned by
that SceneManager.

`executeUserScalableTask` will block until all threads are done. If you do
not wish to block; you can pass false to the second argument and then
call `waitForPendingUserScalableTask` to block until done:

```cpp
int main()
{
    // assumes Ogre is intialized and sceneMgr is a valid ptr
    SceneManager *sceneMgr;

    MyThreadedTask myThreadedTask;
    sceneMgr->executeUserScalableTask( myThreadedTask, false );

    doSomeWork();
    printf( "I am going to sleep now" );

    sceneMgr->waitForPendingUserScalableTask();

    printf( "All worker threads finished. Resuming execution of main thread" );

    return 0;
}
```

>  Attention!
>
>  You **must** call *waitForPendingUserScalableTask* after calling *executeUserScalableTask*( myThreadedTask, false ) before *executeUserScalableTask* can be called again.
>  Otherwise deadlocks are bound to happen and Ogre makes no integrity checks. Queuing or scheduling of multiples tasks is not supported. This system is for synchronous multithreading, not for asynchronous tasks.

# Thread safety of SceneNodes {#ThreadSafetySceneNodes}

In Ogre 1.x; SceneNodes weren't thread safe at all, not even `setPosition`
or `_getDerivedPosition`.

In Ogre 2.x, the following **operations are not thread safe**:

-   **Creating or destroying nodes/entities**. Don't create a SceneNode
    while there are scene nodes being used in other threads. It can
    screw the unique ID and their assignment into the global vector we
    use to keep track of created nodes. Furthermore The node's memory
    manager may ran out of memory in its pool and reconstruct the
    Transform's SoA pointers (it's similar to how `std::vector`
    invalidates all iterators when resizing). If that happens, all
    SceneNodes will be in an inconsistent state. Inversely, if too many
    nodes have been removed, the manager may decide it's time for a
    cleanup, in which case many SceneNodes can become in an inconsistent
    state until the cleanup finishes. How large the pool reserve is can
    be tweaked, and how often the manager performs can also be tweaked
    (`NodeMemoryManager`), though. If the user knows what he's doing the
    race condition might be possible to avoid. Note other SceneManager
    implementations may have to fulfill their own needs and introduce
    race conditions of their own we can't predict.
-   **Changing parent / child relationships**. Attaching/detaching a
    node from another one causes its SoA memory to migrate to a
    different node memory manager, which can trigger a cleanup and/or
    one of the managers rans out of memory and has to reconstruct.
-   **Calling \_getDerivedPositionUpdated & Co (all functions that end
    in "Updated")**. These functions will update the derived transforms
    all way up to the ultimate parent (i.e. root). However in SIMD
    builds, these updates are performed on 4 nodes at a time (the actual
    number is not 4, but rather depends on `ARRAY_PACKED_REALS`).
    Calling this function could only be thread safe if all all four
    nodes are in the same thread AND their parents are also on the same
    thread (parents may not share the same block, thus worst case
    scenario `4 * 4 = 16` parent nodes have to be in the same thread, not
    to mention their parents too `4 * 4 * 4 = 64`) AND the children of
    these parents are not calling `_getDerivedPositionUpdated` too from a
    different thread.

The following operations are thread-safe:

-   **Calling getPosition & Co (getOrientation, getScale,
    getInheritOrientation, etc), \_getDerivedPosition & Co
    (\_getDerivedOrientation, etc)** unless you're calling `getPosition`
    and `setPosition` to the same Node from different threads.
-   **Calling setPosition, setOrientation, setScale**. Note on SIMD
    builds, 4 Nodes can easily share the same 64 byte line, thus it is
    advisable that all 4 Nodes to be sent to the same thread to reduce
    the number of false cache sharing performance hits. Calling
    setPosition to the same Node from different threads is not
    supported.
-   **Calling \_setDerivedPosition, \_setDerivedOrientation,
    \_setDerivedScale** (which assumes the derived transforms are up to
    date)

With Ogre 2.0; it is now possible to transfer the position & orientation
from a physics engine to Ogre Scene Nodes using a parallel for. Ogre 1.x
limitations forced this update to be done in a single thread.
