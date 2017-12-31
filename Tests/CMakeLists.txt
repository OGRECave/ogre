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

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)

  ogre_add_component_include_dir(Bites)
  set(TEST_DEPENDENCIES OgreBites)

  if(SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR})
  endif()

  # Make sure all plugins are built
  if (OGRE_BUILD_PLUGIN_OCTREE)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_OctreeSceneManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_BSP)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_BSPSceneManager)
  endif ()
  if (OGRE_BUILD_PLUGIN_CG)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_CgProgramManager)
  endif (OGRE_BUILD_PLUGIN_CG)
  if (OGRE_BUILD_PLUGIN_PFX)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_ParticleFX)
  endif ()

  if (OGRE_BUILD_PLUGIN_PCZ)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_PCZSceneManager)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_OctreeZone)
  endif ()

  if (OGRE_BUILD_RENDERSYSTEM_D3D9)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_Direct3D9)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_D3D11)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_Direct3D11)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GL)
    set(TEST_GLSUPPORT TRUE)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_GL)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
    set(TEST_GLSUPPORT TRUE)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_GL3Plus)
  endif ()
  if (OGRE_BUILD_RENDERSYSTEM_GLES2)
    set(TEST_GLSUPPORT TRUE)
    set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_GLES2)
  endif ()
  
  if (OGRE_STATIC)
    # Static linking means we need to directly use plugins
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/BSPSceneManager/include)
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/CgProgramManager/include)
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/OctreeSceneManager/include)
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/OctreeZone/include)
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/ParticleFX/include)
    include_directories(${OGRE_SOURCE_DIR}/PlugIns/PCZSceneManager/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/Direct3D9/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/Direct3D11/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLES/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLSupport/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GL3Plus/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLES2/include)
    include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GL/include)
    # Link to all enabled plugins
    set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${TEST_DEPENDENCIES})

    # Need to include resource files so that icons are linked
    if (WIN32)
      set(RESOURCE_FILES
        ${OGRE_SOURCE_DIR}/OgreMain/src/WIN32/resource.h
        ${OGRE_SOURCE_DIR}/OgreMain/src/WIN32/OgreWin32Resources.rc
        )
      source_group(Resources FILES ${RESOURCE_FILES})
    endif ()
  endif (OGRE_STATIC)
    
    set(GTEST_SHARED TRUE)
    if(ANDROID)
        set(GTEST_SHARED FALSE)
    endif()
    if(APPLE AND NOT APPLE_IOS)
        set(GTEST_SHARED FALSE)
        set(CROSS ${CROSS} -DCMAKE_CXX_FLAGS=-stdlib=libc++)
    endif()
    
    if(MSVC)
        set(CROSS ${CROSS} -DCMAKE_CXX_FLAGS=-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    endif()

    set(GTEST_INSTALL_DIR ${CMAKE_BINARY_DIR}/gtest)
    include(ExternalProject)
    ExternalProject_Add(googletest
        URL https://github.com/google/googletest/archive/release-1.8.0.tar.gz
        SOURCE_DIR ${GTEST_INSTALL_DIR}
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${GTEST_INSTALL_DIR}
            -DBUILD_SHARED_LIBS=${GTEST_SHARED}
            -G ${CMAKE_GENERATOR}
            ${CROSS})

    # unit tests are go!
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/include ${GTEST_INSTALL_DIR}/include)
    link_directories(${GTEST_INSTALL_DIR}/lib)
    
    file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/include/*.h")
    file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/src/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

    if (OGRE_CONFIG_ENABLE_ZIP)
      list(APPEND HEADER_FILES OgreMain/include/ZipArchiveTests.h)
      list(APPEND SOURCE_FILES OgreMain/src/ZipArchiveTests.cpp)
    endif ()

    if (OGRE_BUILD_COMPONENT_PAGING)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Paging/include)
      ogre_add_component_include_dir(Paging)

      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgrePaging)
      list(APPEND HEADER_FILES Components/Paging/include/PageCoreTests.h)
      list(APPEND SOURCE_FILES Components/Paging/src/PageCoreTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/MeshLodGenerator/include)
      ogre_add_component_include_dir(MeshLodGenerator)

      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreMeshLodGenerator)
      list(APPEND HEADER_FILES Components/MeshLodGenerator/include/MeshLodTests.h)
      list(APPEND SOURCE_FILES Components/MeshLodGenerator/src/MeshLodTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_TERRAIN)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Terrain/include)
      ogre_add_component_include_dir(Terrain)

      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreTerrain)
      list(APPEND HEADER_FILES Components/Terrain/include/TerrainTests.h)
      list(APPEND SOURCE_FILES Components/Terrain/src/TerrainTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_PROPERTY)
      include_directories(${OGRE_SOURCE_DIR}/Components/Property/include)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreProperty)
      list(APPEND SOURCE_FILES Components/Property/src/PropertyTests.cpp)
    endif ()
    if (OGRE_BUILD_COMPONENT_OVERLAY)
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Overlay/include
        ${OGRE_SOURCE_DIR}/Components/Overlay/include)

      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreOverlay)
    endif ()
    
    if(TEST_GLSUPPORT)
      include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLSupport/include)
      set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreGLSupport)
      list(APPEND SOURCE_FILES RenderSystems/GLSupport/src/GLSLTests.cpp)
    endif()
    
    if(ANDROID)
        list(APPEND SOURCE_FILES ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
        list(APPEND OGRE_LIBRARIES EGL log android)
        add_definitions(-Wl,-pie)
    endif()
    
    if(UNIX)
      add_definitions(-Wno-undef)
    endif()
    add_executable(Test_Ogre ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )
    add_dependencies(Test_Ogre googletest)
    ogre_install_target(Test_Ogre "" FALSE)
    target_link_libraries(Test_Ogre ${OGRE_LIBRARIES} gtest)
    
    if(ANDROID)
        set_target_properties(Test_Ogre PROPERTIES LINK_FLAGS -pie)
    endif()
    
    if(APPLE AND NOT APPLE_IOS)
      set_property(TARGET Test_Ogre PROPERTY MACOSX_BUNDLE TRUE)
      set(OGRE_BUILT_FRAMEWORK "$(PLATFORM_NAME)/$(CONFIGURATION)")
      set(OGRE_TEST_CONTENTS_PATH ${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)/Test_Ogre.app/Contents)
      set_target_properties(Test_Ogre PROPERTIES
        LINK_FLAGS "-framework Carbon -framework Cocoa")
      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/CustomCapabilities
        COMMAND ditto
        ${OGRE_SOURCE_DIR}/Tests/Media/CustomCapabilities/* ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/CustomCapabilities
        )

      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Resources
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/plugins.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/resources.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/samples.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/quakemap.cfg
        ${OGRE_TEST_CONTENTS_PATH}/Resources/
        COMMAND ditto
        ${OGRE_SOURCE_DIR}/Samples/Media/ ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/
        COMMAND ditto
        ${OGRE_SOURCE_DIR}/Tests/Media/ ${OGRE_TEST_CONTENTS_PATH}/Resources/Media/
        COMMAND ditto
        ${OGRE_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_TEST_CONTENTS_PATH}/Resources
        )

      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Frameworks
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Ogre.framework
        ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
        COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreMeshLodGenerator.framework
        ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
        COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}
        ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
        )
      add_custom_command(TARGET Test_Ogre POST_BUILD
        COMMAND mkdir ARGS -p ${OGRE_TEST_CONTENTS_PATH}/Plugins)
      if (OGRE_BUILD_RENDERSYSTEM_GL)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/RenderSystem_GL.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif ()
      if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/RenderSystem_GL3Plus.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif ()
      if (OGRE_BUILD_PLUGIN_BSP)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_BSPSceneManager.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_PLUGIN_CG)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_CgProgramManager.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_PLUGIN_OCTREE)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_OctreeSceneManager.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_PLUGIN_PCZ)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_PCZSceneManager.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_OctreeZone.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_PLUGIN_PFX)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Plugin_ParticleFX.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()

      # Components
      if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreMeshLodGenerator.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_PAGING)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgrePaging.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_PROPERTY)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreProperty.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreRTShaderSystem.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_TERRAIN)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreTerrain.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_VOLUME)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreVolume.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
      if (OGRE_BUILD_COMPONENT_OVERLAY)
        add_custom_command(TARGET Test_Ogre POST_BUILD
          COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/OgreOverlay.framework
          ${OGRE_TEST_CONTENTS_PATH}/Frameworks/
          )
      endif()
    endif()
    
    add_subdirectory(VisualTests)
endif (OGRE_BUILD_TESTS)
