# Background Resource Loading {#background-resource}

%Ogre uses thread pool based concurrency, where a fixed number of persistent worker threads are used to process incoming jobs.
The worker threads are organized in [WorkQueues](@ref Ogre::WorkQueue), which allows implementing load-balancing.

By default %Ogre creates one Ogre::WorkQueue (an Ogre::DefaultWorkQueue named `"Root"`) with a worker count derived from the hardware concurrency, clamped to 1&ndash;2.
The workers are started in Ogre::Root::oneTimePostWindowInit, after the first Ogre::RenderWindow is created.
You can customize the default work queue by calling Ogre::DefaultWorkQueueBase::setWorkerThreadCount and Ogre::DefaultWorkQueueBase::setWorkersCanAccessRenderSystem before the WorkQueue is started, or later by restarting it.
Both settings only take effect when the queue is (re)started.

For resource loading, there is additionally the high-level Ogre::ResourceBackgroundQueue interface, that dispatches work to the default WorkQueue, but hides the underlying Ogre::WorkQueue::addTask/ Ogre::WorkQueue::addMainThreadTask handling.

To prepare a Texture in the background you can do:
```cpp
Ogre::TexturePtr tex = ...;
Ogre::ResourceBackgroundQueue& rbq = Ogre::ResourceBackgroundQueue::getSingleton();
std::future<void> fut = rbq.prepare(tex);
```

The returned future becomes ready when the background portion of the operation has finished.
The preferred way to get notified about the completed resource is to register a Ogre::Resource::Listener: the listener callbacks are dispatched to the main thread and may therefore run after the future is ready.
Use the future to wait for the data to become available, and the listener to observe the finished resource.

In the default build config, background workers cannot access the Ogre::RenderSystem to avoid the overhead incurred by the underlying APIs.
Therefore, only Ogre::Resource::prepare can be called by a worker, while Ogre::Resource::load must be done on the main thread.
Ogre::ResourceBackgroundQueue hides this split: under the default `OGRE_THREAD_SUPPORT` value of 3, `load` prepares the resource in the background and performs the actual load on the main thread, so the future may become ready before the resource is loaded.
Only with `OGRE_THREAD_SUPPORT == 1` are resources fully thread-safe and loaded entirely in the background.
See the `OGRE_CONFIG_THREADS` CMake option for the meaning of the individual values.

@note This makes calls to Ogre::Resource::setBackgroundLoaded obsolete as they cause Resources to ignore load() calls from the main thread, enforcing that they are loaded from a Worker thread.

The table below shows an overview what the prepared and loaded states mean for different resource types.

| Type | Prepared | Loaded |
| -------- | ----------- | ----------|
| Mesh | read file to memory | parsed and uploaded to GPU |
| Material | prepared and compiled supported techniques | supported techniques loaded |
| Texture | read and decoded images | uploaded to GPU |
| GpuProgram (HLSL) | read source and compiled microcode | uploaded to GPU and created buffers |
| GpuProgram (GLSL) | read and preprocessed source | compiled and uploaded to GPU |
| Compositor | - | compiled supported techniques and created textures |
| Skeleton | read and parsed animation | - |
