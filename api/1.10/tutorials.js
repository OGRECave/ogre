var tutorials =
[
    [ "Setting up an OGRE project", "setup.html", [
      [ "CMake Configuration", "setup.html#cmake", null ],
      [ "Application skeleton", "setup.html#skeleton", null ]
    ] ],
    [ "Your First Scene", "tut__first_scene.html", [
      [ "How Ogre Works", "tut__first_scene.html#howogreworks", [
        [ "SceneManager", "tut__first_scene.html#scenemanager", null ],
        [ "SceneNode", "tut__first_scene.html#SceneNode", null ],
        [ "Entity", "tut__first_scene.html#Entity", null ]
      ] ],
      [ "Setting Up the Scene", "tut__first_scene.html#SettingUptheScene", null ],
      [ "Coordinates Systems", "tut__first_scene.html#CoordinatesSystems", null ],
      [ "Adding Another Entity", "tut__first_scene.html#AddingAnotherEntity", null ],
      [ "More About Entities", "tut__first_scene.html#MoreAboutEntities", null ],
      [ "More About SceneNodes", "tut__first_scene.html#MoreAboutSceneNodes", null ],
      [ "Changing An Entity's Scale", "tut__first_scene.html#ChangingAnEntitysScale", null ],
      [ "Rotating An Entity", "tut__first_scene.html#RotatingAnEntity", null ],
      [ "The Ogre Environment", "tut__first_scene.html#TheOgreEnvironment", [
        [ "Libraries and Plugins", "tut__first_scene.html#LibrariesandPlugins", [
          [ "Main library", "tut__first_scene.html#Mainlibrary", null ],
          [ "Plugins", "tut__first_scene.html#Plugins", null ],
          [ "Third-party Plugins", "tut__first_scene.html#ThirdpartyPlugins", null ],
          [ "Testing vs Release", "tut__first_scene.html#TestingvsRelease", null ]
        ] ],
        [ "Configuration Files", "tut__first_scene.html#ConfigurationFiles", [
          [ "Plugin Configuration", "tut__first_scene.html#PluginConfiguration", null ],
          [ "Resource Configuration", "tut__first_scene.html#ResourceConfiguration", null ],
          [ "Media Configuration", "tut__first_scene.html#MediaConfiguration", null ],
          [ "Ogre Configuration", "tut__first_scene.html#OgreConfiguration", null ],
          [ "Quake 3 Settings Configuration", "tut__first_scene.html#Quake3SettingsConfiguration", null ]
        ] ]
      ] ],
      [ "Conclusion", "tut__first_scene.html#Conclusion1", null ]
    ] ],
    [ "Lights, Cameras, and Shadows", "tut__lights_cameras_shadows.html", [
      [ "The Ogre Camera Class", "tut__lights_cameras_shadows.html#bt2TheOgreCameraClass", null ],
      [ "Creating a Camera", "tut__lights_cameras_shadows.html#bt2CreatingaCamera", null ],
      [ "Viewports", "tut__lights_cameras_shadows.html#bt2Viewports", null ],
      [ "Creating a Viewport", "tut__lights_cameras_shadows.html#bt2CreatingaViewport", null ],
      [ "Building the Scene", "tut__lights_cameras_shadows.html#bt2BuildingtheScene", null ],
      [ "Using Shadows in Ogre", "tut__lights_cameras_shadows.html#bt2UsingShadowsinOgre", null ],
      [ "Lights", "tut__lights_cameras_shadows.html#bt2Lights", null ],
      [ "Creating a Light", "tut__lights_cameras_shadows.html#CreatingaLight", null ],
      [ "Creating More Lights", "tut__lights_cameras_shadows.html#CreatingMoreLights", null ],
      [ "Shadow Types", "tut__lights_cameras_shadows.html#ShadowTypes", null ],
      [ "Conclusion", "tut__lights_cameras_shadows.html#Conclusion2", null ]
    ] ],
    [ "RTSS: Run Time Shader System", "rtss.html", [
      [ "Core features of the system", "rtss.html#core-feats", null ],
      [ "System overview", "rtss.html#rtss_overview", [
        [ "Customising via API", "rtss.html#rtss_custom_api", null ],
        [ "Customizing via Material Script", "rtss.html#rtss_custom_mat", null ]
      ] ],
      [ "The RTSS in Depth", "rtss.html#rtss_indepth", [
        [ "Main components", "rtss.html#rtss__components", null ],
        [ "Creating custom shader extensions", "rtss.html#creating-extensions", null ],
        [ "Tips for debugging shaders", "rtss.html#debugging", null ]
      ] ],
      [ "Historical background", "rtss.html#history", null ],
      [ "Pros and Cons", "rtss.html#pros-cons", null ]
    ] ],
    [ "HLMS: High Level Material System", "hlms.html", [
      [ "The three components", "hlms.html#components", null ],
      [ "Compared to classical materials", "hlms.html#materials", null ],
      [ "Material parameters are stored in “Blocks”", "hlms.html#data", [
        [ "Datablocks", "hlms.html#toc52", null ]
      ] ],
      [ "Hlms templates", "hlms.html#toc69", [
        [ "The Hlms preprocessor", "hlms.html#preproc", null ],
        [ "Preprocessor syntax", "hlms.html#syntax", null ]
      ] ],
      [ "Creation of shaders", "hlms.html#shaders", null ],
      [ "C++ interaction with shader templates", "hlms.html#cpp", null ],
      [ "Customization", "hlms.html#customization", null ]
    ] ],
    [ "Trays GUI System", "trays.html", [
      [ "Trays", "trays.html#trays-1", null ],
      [ "TrayManager", "trays.html#traymanager", null ],
      [ "Widgets", "trays.html#widgets", null ],
      [ "Special Widgets", "trays.html#special-widgets", null ],
      [ "TrayListener", "trays.html#tray-listener", null ],
      [ "Things to Try", "trays.html#things-to-try", null ]
    ] ],
    [ "Volume Component", "volume.html", [
      [ "How to use it", "volume.html#howto", null ],
      [ "Manual creation of a CSG-Tree", "volume.html#creation", null ],
      [ "Getting the triangles of the chunks", "volume.html#triangles", null ],
      [ "Intersecting a ray with a volume", "volume.html#intersecting", null ],
      [ "Editing a Volume made from a GridSource", "volume.html#editing", null ]
    ] ],
    [ "Using the Profiler", "profiler.html", null ],
    [ "Shadow Mapping in Ogre", "_shadow_mapping_ogre.html", [
      [ "Introduction to the Shadow Mapping Algorithm", "_shadow_mapping_ogre.html#ShadowMappingIntro", [
        [ "Formalism", "_shadow_mapping_ogre.html#sm_formalism", null ],
        [ "Depth Biasing", "_shadow_mapping_ogre.html#DepthBias", null ],
        [ "Percentage Closest Filtering", "_shadow_mapping_ogre.html#sm_pcm", null ]
      ] ],
      [ "Variants", "_shadow_mapping_ogre.html#sm_variants", [
        [ "Storing Additional Info", "_shadow_mapping_ogre.html#sm_additional_info", null ],
        [ "Breaking up Shadow Frusta", "_shadow_mapping_ogre.html#sm_breaking_frusta", null ],
        [ "Playing with Projection Matrices", "_shadow_mapping_ogre.html#sect_planeopt", null ]
      ] ],
      [ "Theory and Analysis", "_shadow_mapping_ogre.html#sm_theory", [
        [ "(Non)Optimality of Logarithmic Shadow Maps", "_shadow_mapping_ogre.html#sm_nonopt", null ],
        [ "Sampling Aliasing versus Depth Precision Aliasing", "_shadow_mapping_ogre.html#sm_aliasing", null ],
        [ "Projective versus Perspective Aliasing", "_shadow_mapping_ogre.html#sm_proj_aliasing", null ]
      ] ],
      [ "Implementation", "_shadow_mapping_ogre.html#Implementation", null ]
    ] ],
    [ "Deferred Shading", "deferred.html", [
      [ "What is Deferred Shading?", "deferred.html#what", null ],
      [ "Creating the G-Buffer", "deferred.html#creating", null ],
      [ "Lighting the scene", "deferred.html#lighting", [
        [ "Rendering the light geometry", "deferred.html#lightgeom", null ]
      ] ],
      [ "Post Processing", "deferred.html#post", null ],
      [ "Integration in real projects", "deferred.html#realprojects", null ],
      [ "Summary", "deferred.html#summary", [
        [ "Further reading", "deferred.html#further", null ]
      ] ]
    ] ],
    [ "External Texture Sources", "_external-_texture-_sources.html", null ],
    [ "Using the PCZ Scene Manager", "pczscenemanager.html", null ]
];