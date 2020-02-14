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
  
  if (OGRE_BUILD_COMPONENT_OVERLAY)
    set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreOverlay)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_TERRAIN)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreTerrain)
  endif ()

  if (OGRE_BUILD_COMPONENT_VOLUME)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreVolume)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_HLMS)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreHLMS)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR AND OGRE_CONFIG_ENABLE_MESHLOD)
  	set(SAMPLE_DEPENDENCIES ${SAMPLE_DEPENDENCIES} OgreMeshLodGenerator)
  endif ()
  
  if(EMSCRIPTEN)
    add_subdirectory(Emscripten)
    return()
  endif()

  ## Default Samples Plugin

  # Find all samples headers, and copy them to a single include folder
  # We do this instead of including every sample in the include_directories directive, 
  # because this creates a list that is too long for the MSVC command line sometimes.
  file (GLOB SAMPLES_HEADERS ${PROJECT_SOURCE_DIR}/Samples/*/include/*.h)
  foreach(SAMPLE_HEADER_FULLPATH ${SAMPLES_HEADERS})
	  get_filename_component(SAMPLE_HEADER_NAME ${SAMPLE_HEADER_FULLPATH} NAME)
	  configure_file(${SAMPLE_HEADER_FULLPATH} ${PROJECT_BINARY_DIR}/Samples/include/${SAMPLE_HEADER_NAME} COPYONLY)
  endforeach(SAMPLE_HEADER_FULLPATH)
  include_directories(${PROJECT_BINARY_DIR}/Samples/include)

  file(GLOB SAMPLES_SOURCES ${PROJECT_SOURCE_DIR}/Samples/*/src/*.cpp)

  set(BLACKLIST Browser TerrainTessellation PCZTestApp)
  foreach(FILTER_SAMPLE ${BLACKLIST})
    file(GLOB BLACKLIST_CPP ${PROJECT_SOURCE_DIR}/Samples/${FILTER_SAMPLE}/src/*.cpp)
    list(REMOVE_ITEM SAMPLES_SOURCES ${BLACKLIST_CPP})
  endforeach()

  add_definitions(${OGRE_VISIBILITY_FLAGS})
  add_library(DefaultSamples ${OGRE_LIB_TYPE} 
              ${CMAKE_CURRENT_SOURCE_DIR}/Common/src/DefaultSamplesPlugin.cpp
              ${SAMPLES_SOURCES} ${SAMPLES_HEADERS})
  target_link_libraries(DefaultSamples OgreMain ${SAMPLE_DEPENDENCIES} OgreBites)

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


  
# Install sample sources
if (OGRE_INSTALL_SAMPLES_SOURCE)
  if (WIN32 OR APPLE)
    set(OGRE_SAMPLES_DIR Samples)
  elseif (UNIX)
    set(OGRE_SAMPLES_DIR share/OGRE/Samples)
  endif ()
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${OGRE_SAMPLES_DIR}
    REGEX "^CMakeLists.txt$" EXCLUDE
    PATTERN "Makefile.am" EXCLUDE
    PATTERN "Media" EXCLUDE
    PATTERN "bin" EXCLUDE
    PATTERN "setup" EXCLUDE
    PATTERN ".hg" EXCLUDE
	PATTERN "obj" EXCLUDE
	PATTERN "scripts" EXCLUDE
  )
endif ()

# Install sample headers, some people rely on these
file(GLOB SAMPLE_COMMON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Common/include/*.h")
install(FILES ${SAMPLE_COMMON_HEADERS}
		DESTINATION include/OGRE)
		
if (MSVC AND OGRE_BUILD_SAMPLES)
  find_package(Wix)
  if (Wix_FOUND)
    # Create WiX setup for demo build
    configure_file(${OGRE_TEMPLATES_DIR}/demos.wxs.in ${CMAKE_CURRENT_BINARY_DIR}/demos.wxs @ONLY)
    configure_file(${OGRE_TEMPLATES_DIR}/demomedia.wxi.in ${CMAKE_CURRENT_BINARY_DIR}/demomedia.wxi @ONLY)
    configure_file(${OGRE_TEMPLATES_DIR}/DemoLicense.rtf ${CMAKE_CURRENT_BINARY_DIR}/DemoLicense.rtf COPYONLY)
    if (MSVC_VERSION EQUAL 1400)
      configure_file(${OGRE_TEMPLATES_DIR}/vcrt_vc8.wxi.in ${CMAKE_CURRENT_BINARY_DIR}/vcrt.wxi @ONLY)
    elseif(MSVC_VERSION EQUAL 1500)
      configure_file(${OGRE_TEMPLATES_DIR}/vcrt_vc9.wxi.in ${CMAKE_CURRENT_BINARY_DIR}/vcrt.wxi @ONLY)
    endif()
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
