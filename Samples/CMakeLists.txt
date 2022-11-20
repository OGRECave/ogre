# Configure Samples build

# Add android JNI binding example
if(OGRE_BUILD_ANDROID_JNI_SAMPLE)
  add_subdirectory(AndroidJNI)
endif()

if (OGRE_BUILD_SAMPLES)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)

  if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    add_definitions(-DINCLUDE_RTSHADER_SYSTEM)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreRTShaderSystem)
  endif ()

  if (OGRE_BUILD_COMPONENT_BULLET)
    set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreBullet)
  endif ()

  if (OGRE_BUILD_COMPONENT_OVERLAY)
    set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreOverlay)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_TERRAIN)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreTerrain)
  endif ()

  if (OGRE_BUILD_COMPONENT_VOLUME)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreVolume)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR AND OGRE_CONFIG_ENABLE_MESHLOD)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreMeshLodGenerator)
  endif ()
  
  if(EMSCRIPTEN)
    add_subdirectory(Emscripten)
    return()
  endif()

  ## Default Samples Plugin
  file (GLOB SAMPLES_HEADERS ${PROJECT_SOURCE_DIR}/Samples/*/include)
  include_directories(${SAMPLES_HEADERS})

  file(GLOB SAMPLES_SOURCES ${PROJECT_SOURCE_DIR}/Samples/*/src/*.cpp)

  set(BLACKLIST Browser TerrainTessellation)
  if(NOT OGRE_BUILD_PLUGIN_PCZ)
    list(APPEND BLACKLIST PCZTestApp)
  endif()
  if(NOT OGRE_BUILD_COMPONENT_TERRAIN)
    list(APPEND BLACKLIST EndlessWorld)
  endif()

  foreach(FILTER_SAMPLE ${BLACKLIST})
    file(GLOB BLACKLIST_CPP ${PROJECT_SOURCE_DIR}/Samples/${FILTER_SAMPLE}/src/*.cpp)
    list(REMOVE_ITEM SAMPLES_SOURCES ${BLACKLIST_CPP})
  endforeach()

  add_definitions(${OGRE_VISIBILITY_FLAGS})
  add_library(DefaultSamples ${OGRE_LIB_TYPE} 
              ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/DefaultSamplesPlugin.cpp
              ${SAMPLES_SOURCES} ${SAMPLES_HEADERS})
  target_link_libraries(DefaultSamples OgreMain ${SAMPLE_DEPENDENCIES} OgreBites)

  if(OGRE_BUILD_PLUGIN_PCZ)
    target_link_libraries(DefaultSamples Plugin_PCZSceneManager)
    target_compile_definitions(DefaultSamples PRIVATE -DHAVE_PCZ_PLUGIN)
  endif()

  if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
    target_compile_definitions(DefaultSamples PRIVATE -DHAVE_IMGUI)
  endif()

  ogre_config_sample_lib(DefaultSamples)

  if (APPLE AND NOT APPLE_IOS)
      # Set the INSTALL_PATH so that Samples can be installed in the application package
      set_target_properties(DefaultSamples
        PROPERTIES BUILD_WITH_INSTALL_RPATH 1
        INSTALL_NAME_DIR "@executable_path/../Plugins"
      )
  endif()

  ## Add browser last
  add_subdirectory(Browser)
endif ()
		
if (MSVC AND OGRE_BUILD_SAMPLES)
  find_package(Wix)
  if (Wix_FOUND)
    # Create WiX setup for demo build
    configure_file(${OGRE_TEMPLATES_DIR}/demos.wxs.in ${CMAKE_CURRENT_BINARY_DIR}/demos.wxs @ONLY)
    configure_file(${OGRE_TEMPLATES_DIR}/demomedia.wxi.in ${CMAKE_CURRENT_BINARY_DIR}/demomedia.wxi @ONLY)
    configure_file(${OGRE_TEMPLATES_DIR}/DemoLicense.rtf ${CMAKE_CURRENT_BINARY_DIR}/DemoLicense.rtf COPYONLY)
	# Configure files, set media dir temporarily
	set(OGRE_MEDIA_DIR_TMP ${OGRE_MEDIA_DIR_REL})
	set(OGRE_MEDIA_DIR_REL "Media")
    configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg @ONLY)
	# restore
	set(OGRE_MEDIA_DIR_REL ${OGRE_MEDIA_DIR_TMP})
    add_custom_target(demo_installer 
    COMMAND ${Wix_BINARY_DIR}/candle demos.wxs 
	  COMMAND ${Wix_BINARY_DIR}/light -ext WixUIExtension -cultures:en-us -out OgreDemos_v${OGRE_VERSION_DASH_SEPARATED}.msi demos.wixobj
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Building demo installer" VERBATIM
    )
	# Make sure we build samples first
	add_dependencies(demo_installer SampleBrowser)
  endif()
endif()
