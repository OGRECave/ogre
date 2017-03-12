var manual =
[
    [ "Changes: Objects, Scene & Nodes", "_ogre20_changes.html", [
      [ "Names are now optional", "_ogre20_changes.html#NamesAreNowOptional", null ],
      [ "How to debug MovableObjects' (and Nodes) data", "_ogre20_changes.html#HowToDebugMovableObjectsData", [
        [ "Interpreting ArrayVector3", "_ogre20_changes.html#InterpretingArrayVector3", null ],
        [ "Dummy pointers instead of NULL", "_ogre20_changes.html#DummyPointers", null ]
      ] ],
      [ "Attachment and Visibility", "_ogre20_changes.html#AttachmentAndVisibility", null ],
      [ "Attaching/Detaching is more expensive than hiding", "_ogre20_changes.html#AttachingDetachingIsMoreExpensive", null ],
      [ "All MovableObjects require a SceneNode (Lights & Cameras)", "_ogre20_changes.html#AllMovableObjectsRequireSceneNode", null ],
      [ "Obtaining derived transforms", "_ogre20_changes.html#DerivedTransforms", null ],
      [ "SCENE_STATIC and SCENE_DYNAMIC", "_ogre20_changes.html#SceneStaticSceneDynamic", [
        [ "What means a Node to be SCENE_STATIC", "_ogre20_changes.html#SceneStaticNode", null ],
        [ "What means a Entities (and InstancedEntities) to be SCENE_STATIC", "_ogre20_changes.html#SceneStaticEntities", null ],
        [ "General", "_ogre20_changes.html#SceneStaticGeneral", null ]
      ] ],
      [ "Ogre asserts mCachedAabbOutOfDate or mCachedTransformOutOfDate while in debug mode", "_ogre20_changes.html#AssersionCachedOutOfDate", null ],
      [ "Custom classes derived from Renderable or MovableObject", "_ogre20_changes.html#DerivingRenderable", null ],
      [ "How do I get the vertex information from the new v2 Mesh classes?", "_ogre20_changes.html#V2MeshInformation", null ],
      [ "How do I set the element offsets, vertex buffer's source and index?", "_ogre20_changes.html#V2MeshElementOffset", null ],
      [ "My scene looks too dark or dull!", "_ogre20_changes.html#SceneLooksDarkDull", null ],
      [ "I activated gamma correction, but now my GUI textures look washed out!", "_ogre20_changes.html#GUIWashedOut", null ]
    ] ],
    [ "Technical Overview", "_technical_overview.html", [
      [ "Overview", "_technical_overview.html#TechnicalOverviewOverview", [
        [ "SIMD Coherence", "_technical_overview.html#SIMDCoherence", null ]
      ] ],
      [ "Memory Managers usage patterns", "_technical_overview.html#MemoryManagerUsagePatterns", [
        [ "Cleanups", "_technical_overview.html#MemoryManagerCleanups", null ]
      ] ],
      [ "Memory preallocation", "_technical_overview.html#MemoryPreallocation", null ],
      [ "Configuring memory managers", "_technical_overview.html#ConfiguringMemoryManagers", null ],
      [ "Where is RenderTarget::update? Why do I get errors in Viewport?", "_technical_overview.html#RenderTargetUpdate", null ],
      [ "Porting from 1.x to 2.0", "_technical_overview.html#PortingV1ToV2", null ],
      [ "Porting from 2.0 to 2.1", "_technical_overview.html#PortingV20ToV21", null ]
    ] ],
    [ "Compositor", "compositor.html", [
      [ "Nodes", "compositor.html#CompositorNodes", [
        [ "Input & output channels and RTTs", "compositor.html#CompositorNodesChannelsAndRTTs", [
          [ "Locally declared textures", "compositor.html#CompositorNodesChannelsAndRTTsLocalTextures", null ],
          [ "It comes from an input channel", "compositor.html#CompositorNodesChannelsAndRTTsFromInputChannel", null ],
          [ "It is a global texture", "compositor.html#CompositorNodesChannelsAndRTTsGlobal", null ],
          [ "Main RenderTarget", "compositor.html#CompositorNodesChannelsAndRTTsMainRenderTarget", null ]
        ] ],
        [ "Target", "compositor.html#CompositorNodesTarget", null ],
        [ "Passes", "compositor.html#CompositorNodesPasses", [
          [ "clear", "compositor.html#CompositorNodesPassesClear", null ],
          [ "generate_mipmaps", "compositor.html#CompositorNodesPassesGenerateMipmaps", null ],
          [ "quad", "compositor.html#CompositorNodesPassesQuad", null ],
          [ "resolve", "compositor.html#CompositorNodesPassesResolve", null ],
          [ "render_scene", "compositor.html#CompositorNodesPassesRenderScene", null ],
          [ "stencil", "compositor.html#CompositorNodesPassesStencil", null ],
          [ "uav_queue", "compositor.html#CompositorNodesPassesUavQueue", [
            [ "Synchronization", "compositor.html#CompositorNodesPassesUavQueueSync", null ]
          ] ],
          [ "compute", "compositor.html#CompositorNodesPassesCompute", null ]
        ] ],
        [ "Textures", "compositor.html#CompositorNodesTextures", [
          [ "MSAA: Explicit vs Implicit resolves", "compositor.html#CompositorNodesTexturesMsaa", [
            [ "Implicit resolves", "compositor.html#CompositorNodesTexturesMsaaImplicit", null ],
            [ "Explicit resolves", "compositor.html#CompositorNodesTexturesMsaaExplicit", null ],
            [ "Resources", "compositor.html#CompositorNodesTexturesMsaaResources", null ]
          ] ],
          [ "Depth Textures", "compositor.html#CompositorNodesTexturesDepth", null ]
        ] ]
      ] ],
      [ "Shadow Nodes", "compositor.html#CompositorShadowNodes", [
        [ "Setting up shadow nodes", "compositor.html#CompositorShadowNodesSetup", null ],
        [ "Example", "compositor.html#CompositorShadowNodesExample", null ],
        [ "Shadow map atlas & Point Lights", "compositor.html#CompositorShadowNodesAtlasAndPointLights", null ],
        [ "Reuse, recalculate and first", "compositor.html#CompositorShadowNodesReuseEtc", null ],
        [ "Shadow mapping setup types", "compositor.html#CompositorShadowNodesTypes", [
          [ "Uniform shadow mapping", "compositor.html#CompositorShadowNodesTypesUniform", null ],
          [ "Focused", "compositor.html#CompositorShadowNodesTypesFocused", null ],
          [ "PSSM / CSM", "compositor.html#CompositorShadowNodesTypesPssm", null ],
          [ "Plane Optimal", "compositor.html#CompositorShadowNodesTypesPlaneOptimal", null ]
        ] ],
        [ "Writing shaders", "compositor.html#CompositorShadowNodesShaders", null ]
      ] ],
      [ "Workspaces", "compositor.html#CompositorWorkspaces", [
        [ "Data dependencies between nodes and circular dependencies", "compositor.html#CompositorWorkspacesDataDependencies", null ]
      ] ],
      [ "Setting up code", "compositor.html#CompositorSetupCode", [
        [ "Initializing the workspace", "compositor.html#CompositorWorkspacesSetupInitialize", null ],
        [ "Simple bootstrap for beginners", "compositor.html#CompositorWorkspacesSetupSimple", null ],
        [ "Advanced C++ users", "compositor.html#CompositorWorkspacesSetupAdvanced", null ]
      ] ],
      [ "Stereo and Split-Screen Rendering", "compositor.html#StereoAndSplitScreenRendering", [
        [ "Per-Workspace offset and scale", "compositor.html#CompositorWorkspacesStereoPerWorkspace", null ],
        [ "Viewport modifier mask", "compositor.html#CompositorWorkspacesStereoViewportMask", null ],
        [ "Execution mask", "compositor.html#CompositorWorkspacesStereoExecutionMask", null ],
        [ "Default values", "compositor.html#CompositorWorkspacesStereoDefaultValues", null ]
      ] ]
    ] ],
    [ "Instancing", "instancing.html", [
      [ "What is instancing?", "instancing.html#WhatIsInstancing", null ],
      [ "Instancing 101", "instancing.html#Instancing101", [
        [ "Instances per batch", "instancing.html#InstancesPerBatch", null ]
      ] ],
      [ "Techniques", "instancing.html#InstancingTechniques", [
        [ "ShaderBased", "instancing.html#InstancingTechniquesShaderBased", null ],
        [ "VTF (Software)", "instancing.html#InstancingTechniquesVTFSoftware", null ],
        [ "HW VTF", "instancing.html#InstancingTechniquesHWVTF", [
          [ "HW VTF LUT", "instancing.html#InstancingTechniquesHW", null ]
        ] ],
        [ "HW Basic", "instancing.html#InstancingTechniquesHWBasic", null ]
      ] ],
      [ "Custom parameters", "instancing.html#InstancingCustomParameters", null ],
      [ "Supporting multiple submeshes", "instancing.html#InstancingMultipleSubmeshes", null ],
      [ "Defragmenting batches", "instancing.html#InstancingDefragmentingBatches", [
        [ "What is batch fragmentation?", "instancing.html#InstancingDefragmentingBatchesIntro", null ],
        [ "Prevention: Avoiding fragmentation", "instancing.html#InstancingDefragmentingBatchesPrevention", null ],
        [ "Cure: Defragmenting on the fly", "instancing.html#InstancingDefragmentingBatchesOnTheFly", null ]
      ] ],
      [ "Troubleshooting", "instancing.html#InstancingTroubleshooting", null ]
    ] ],
    [ "Threading", "threading.html", [
      [ "Initializing", "threading.html#ThreadingInitializing", [
        [ "Ideal number of threads", "threading.html#ThreadingInitializingNumberOfThreads", null ],
        [ "More info about InstancingThreadedCullingMethod", "threading.html#ThreadingInitializingCullingMethod", null ]
      ] ],
      [ "What tasks are threaded in Ogre", "threading.html#ThreadingInOgre", null ],
      [ "Using Ogre's threading system for custom tasks", "threading.html#ThreadingCustomTasks", null ],
      [ "Thread safety of SceneNodes", "threading.html#ThreadSafetySceneNodes", null ]
    ] ],
    [ "Performance Hints", "performance.html", null ],
    [ "HLMS: High Level Material System", "hlms.html", [
      [ "Fundamental changes", "hlms.html#HlmsChanges", [
        [ "Viewports and Scissor tests", "hlms.html#HlmsChangesViewports", null ],
        [ "A lot of data is stored in \"Blocks\"", "hlms.html#HlmsChangesBlocks", null ],
        [ "Materials are still alive", "hlms.html#HlmsChangesMaterialsAlive", null ],
        [ "Fixed Function has been removed", "hlms.html#HlmsChangesFFP", null ]
      ] ],
      [ "The three components", "hlms.html#HlmsComponents", null ],
      [ "Blocks", "hlms.html#HlmsBlocks", [
        [ "Datablocks", "hlms.html#HlmsBlocksDatablocks", null ],
        [ "Macroblocks", "hlms.html#HlmsBlocksMacroblocks", null ],
        [ "Blendblocks", "hlms.html#HlmsBlocksBlendblocks", null ],
        [ "Samplerblocks", "hlms.html#HlmsBlocksSampleblocks", null ]
      ] ],
      [ "Hlms templates", "hlms.html#HlmsTemplates", null ],
      [ "The Hlms preprocessor", "hlms.html#HlmsPreprocessor", [
        [ "Preprocessor syntax", "hlms.html#HlmsPreprocessorSyntax", [
          [ "\\@property( expression )", "hlms.html#HlmsPreprocessorSyntaxProperty", null ],
          [ "@foreach( count, scopedVar, [start] )", "hlms.html#HlmsPreprocessorSyntaxForeach", null ],
          [ "@counter( variable )", "hlms.html#HlmsPreprocessorSyntaxCounter", null ],
          [ "@value( variable )", "hlms.html#HlmsPreprocessorSyntaxValue", null ],
          [ "@set add sub mul div mod min max", "hlms.html#HlmsPreprocessorSyntaxSetEtc", null ],
          [ "@piece( nameOfPiece )", "hlms.html#HlmsPreprocessorSyntaxPiece", null ],
          [ "@insertpiece( nameOfPiece )", "hlms.html#HlmsPreprocessorSyntaxInsertpiece", null ],
          [ "@undefpiece( nameOfPiece )", "hlms.html#HlmsPreprocessorSyntaxUndefpiece", null ],
          [ "@pset padd psub pmul pdiv pmod pmin pmax", "hlms.html#HlmsPreprocessorSyntaxPsetEtc", null ]
        ] ]
      ] ],
      [ "Creation of shaders", "hlms.html#HlmsCreationOfShaders", [
        [ "C++ interaction with shader templates", "hlms.html#HlmsCreationOfShadersCpp", null ],
        [ "Common conventions", "hlms.html#HlmsCreationOfShadersConventions", null ],
        [ "Hot reloading", "hlms.html#HlmsCreationOfShadersHotReloading", null ],
        [ "Disabling a stage", "hlms.html#HlmsCreationOfShadersDisablingStage", null ],
        [ "Customizing an existing implementation", "hlms.html#HlmsCreationOfShadersCustomizing", null ]
      ] ],
      [ "Run-time rendering", "hlms.html#HlmsRuntimeRendering", [
        [ "preparePassHash", "hlms.html#HlmsRuntimeRenderingPreparePassHash", null ],
        [ "fillBuffersFor", "hlms.html#HlmsRuntimeRenderingFillBuffersFor", null ]
      ] ],
      [ "Using the HLMS implementations", "hlms.html#UsingHlmsImplementation", [
        [ "Initialization", "hlms.html#UsingHlmsImplementationInitialization", null ],
        [ "Deinitialization", "hlms.html#UsingHlmsImplementationDeinitilization", null ],
        [ "Creating a datablock", "hlms.html#UsingHlmsImplementationCreatingDatablock", null ]
      ] ],
      [ "The Hlms Texture Manager", "hlms.html#HlmsTextureManager", [
        [ "Automatic batching", "hlms.html#HlmsTextureManagerAutomaticBatching", [
          [ "Texture types", "hlms.html#HlmsTextureManagerAutomaticBatchingTextureTypes", null ],
          [ "Automatic parameters", "hlms.html#HlmsTextureManagerAutomaticBatchingAutoParams", null ],
          [ "Loading a texture twice (i.e. with a different format) via aliasing", "hlms.html#HlmsTextureManagerAutomaticBatchingLoadingTwice", null ]
        ] ],
        [ "Manual: Texture packs", "hlms.html#HlmsTextureManagerTexturePacks", null ],
        [ "Watching out for memory consumption", "hlms.html#HlmsTextureManagerWatchOutMemory", [
          [ "Additional memory considerations", "hlms.html#HlmsTextureManagerWatchOutMemoryConsiderations", null ]
        ] ]
      ] ],
      [ "Troubleshooting", "hlms.html#HlmsTroubleshooting", [
        [ "My shadows don't show up or are very glitchy", "hlms.html#HlmsTroubleshootingShadow", null ]
      ] ]
    ] ],
    [ "AZDO changes (Aproaching Zero Driver Overhead)", "azdo.html", [
      [ "V2 and v1 objects", "azdo.html#V2AndV1Objects", [
        [ "Longevity of the v1 objects and deprecation", "azdo.html#V2AndV1ObjectsV1Longevity", null ]
      ] ],
      [ "Render Queue", "azdo.html#RenderQueue", null ],
      [ "The VaoManager", "azdo.html#VaoMaanger", null ]
    ] ],
    [ "The Command Buffer", "commandbuffer.html", [
      [ "Adding a command", "commandbuffer.html#CommandBufferAddCommand", null ],
      [ "Structure of a command", "commandbuffer.html#CommandBufferCommandStructure", null ],
      [ "Execution table", "commandbuffer.html#CommandBufferExecutionTable", [
        [ "Hacks and Tricks", "commandbuffer.html#CommandBufferExecutionTableHacks", null ]
      ] ],
      [ "Post-processing the command buffer", "commandbuffer.html#CommandBufferPostProcessing", null ]
    ] ]
];