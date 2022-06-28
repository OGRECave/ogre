#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Tests build
if (OGRE_BUILD_TESTS)

  if(NOT EXISTS ${PROJECT_BINARY_DIR}/googletest-1.11.0)
    message(STATUS "Building gtest")
    file(DOWNLOAD
        https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
        ${PROJECT_BINARY_DIR}/release-1.11.0.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E tar xf release-1.11.0.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    execute_process(COMMAND ${BUILD_COMMAND_COMMON}
        ${PROJECT_BINARY_DIR}/googletest-release-1.11.0
        -Dgtest_force_shared_crt=TRUE
        -DBUILD_GMOCK=FALSE
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/googletest-release-1.11.0)
    execute_process(COMMAND ${CMAKE_COMMAND}
        --build ${PROJECT_BINARY_DIR}/googletest-release-1.11.0 ${BUILD_COMMAND_OPTS})
  endif()
  find_package(GTest REQUIRED CONFIG)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)

    # unit tests are go!
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/include)
    
    file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/include/*.h")
    file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/src/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

    if (OGRE_CONFIG_ENABLE_ZIP)
      list(APPEND HEADER_FILES OgreMain/include/ZipArchiveTests.h)
      list(APPEND SOURCE_FILES OgreMain/src/ZipArchiveTests.cpp)
    endif ()

    if (OGRE_BUILD_COMPONENT_PAGING)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgrePaging)
      list(APPEND SOURCE_FILES Components/PageCoreTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreMeshLodGenerator)
      list(APPEND SOURCE_FILES Components/MeshLodTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_TERRAIN)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreTerrain)
      list(APPEND SOURCE_FILES Components/TerrainTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_PROPERTY)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreProperty)
      list(APPEND SOURCE_FILES Components/PropertyTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_OVERLAY)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreOverlay)
    endif ()

    if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreRTShaderSystem)
      list(APPEND SOURCE_FILES Components/RTShaderSystemTests.cpp)
    endif ()
    
    if(TARGET OgreGLSupport)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreGLSupport)
      list(APPEND SOURCE_FILES RenderSystems/GLSupport/GLSLTests.cpp)
    endif()
    
    if(ANDROID)
        list(APPEND SOURCE_FILES ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
        list(APPEND OGRE_LIBRARIES EGL log android)
    endif()
    
    if(UNIX)
      add_definitions(-Wno-undef)
    endif()
    add_executable(Test_Ogre ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )
    ogre_install_target(Test_Ogre "" FALSE)
    target_link_libraries(Test_Ogre OgreBites Codec_STBI ${OGRE_LIBRARIES} GTest::gtest)
    
    if(ANDROID)
        set_target_properties(Test_Ogre PROPERTIES LINK_FLAGS -pie)
    endif()
    
    if(APPLE AND NOT APPLE_IOS)
      set_property(TARGET Test_Ogre PROPERTY MACOSX_BUNDLE TRUE)
      set(OGRE_BUILT_FRAMEWORK "$(PLATFORM_NAME)/$(CONFIGURATION)")
      set(OGRE_TEST_CONTENTS_PATH ${PROJECT_BINARY_DIR}/bin/$(CONFIGURATION)/Test_Ogre.app/Contents)
      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/CustomCapabilities
        COMMAND ditto
        ${PROJECT_SOURCE_DIR}/Tests/Media/CustomCapabilities/* ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/CustomCapabilities
        )

      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Resources
        COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/plugins.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/resources.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/samples.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ditto
        ${PROJECT_SOURCE_DIR}/Samples/Media/ ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/
        COMMAND ditto
        ${PROJECT_SOURCE_DIR}/Tests/Media/ ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/
        COMMAND ditto
        ${PROJECT_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_TEST_CONTENTS_PATH}/Resources
        )

      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Frameworks
        COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Ogre.framework
        ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
        COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}
        ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
        )
      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Plugins)

      set(FRAMEWORKS OgreOverlay
        OgreBites
        OgrePaging
        OgreProperty
        OgreRTShaderSystem
        OgreTerrain
        OgreVolume
        OgreMeshLodGenerator
        Plugin_BSPSceneManager
        Plugin_CgProgramManager
        Plugin_OctreeSceneManager
        Plugin_OctreeZone
        Plugin_ParticleFX
        Plugin_PCZSceneManager
        Plugin_DotScene
        Codec_STBI
        Codec_FreeImage
        RenderSystem_GL
        RenderSystem_GL3Plus
        RenderSystem_Metal
      )
  
      foreach(FWK ${FRAMEWORKS})
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ARGS if [ -d ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/${FWK}.framework ]\; then ln -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/${FWK}.framework ${OGRE_TEST_CONTENTS_PATH}/Frameworks/${FWK}.framework\; fi
        )
      endforeach()
    endif()
    
    add_subdirectory(VisualTests)
endif (OGRE_BUILD_TESTS)
