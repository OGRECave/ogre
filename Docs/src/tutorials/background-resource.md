# Background Resource Loading {#background-resource}

%Ogre uses thread pool based concurrency, where a fixed number of persistent worker threads are used to process incoming jobs.
The worker threads are organized in [WorkQueues](@ref Ogre::WorkQueue), which allows implementing load-balancing.

By default %Ogre creates one Ogre::WorkQueue, implementing Ogre::DefaultWorkQueueBase with 1-2 workers attached. The workers are started after the first Ogre::RenderWindow is created.
You can customize the default work queue by calling Ogre::DefaultWorkQueueBase::setWorkerThreadCount and Ogre::DefaultWorkQueueBase::setWorkersCanAccessRenderSystem before the WorkQueue is started or later by manually restarting the WorkQueue.

For resource loading, there is additionally the high-level Ogre::ResourceBackgroundQueue interface, that dispatches work to the default WorkQueue, but hides the internal Ogre::WorkQueue::Request/ Ogre::WorkQueue::Response handling.

To prepare a Texture in the background you can do:
```cpp
Ogre::TexturePtr tex = ...;
Ogre::ResourceBackgroundQueue& rbq = Ogre::ResourceBackgroundQueue::getSingleton();
std::future<void> fut = rbq.prepare(tex);
```

the returned std::future can be used to poll whether it is finished via std::future::wait_for.
However, the preferred way to get notified about the operation being completed is to register a Ogre::Resource::Listener.

In the default build config, background workers cannot access the Ogre::RenderSystem to avoid the overhead incurred by the underlying APIs.
Therefore, only Ogre::Resource::prepare can be called by a worker, while Ogre::Resource::load must be done on the main thread.

@note This makes calls to Ogre::Resource::setBackgroundLoaded obsolete as they cause Resources to ignore load() calls from the main thread, enforcing that they are loaded from a Worker thread.

The table below shows an overview what the prepared and loaded states mean for different resource types.

| Type | Prepared | Loaded |
| -------- | ----------- | --------|
| Mesh | read file to memory | parsed and uploaded to GPU |
| Material | prepared and compiled supported techniques | supported techniques loaded |
| Texture | read and decoded images | uploaded to GPU |
| GpuProgram (HLSL) | read source and compiled microcode | uploaded to GPU and created buffers |
| GpuProgram (GLSL) | read and preprocessed source | compiled and uploaded to GPU |
| Compositor | - | compiled supported techniques and created textures |
| Skeleton | read and parsed animation | - |