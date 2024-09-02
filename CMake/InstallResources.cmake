#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

##################################################################
# Generate and install the config files needed for the samples
##################################################################

if (NOT OGRE_MEDIA_PATH)
  if (WIN32)
    set(OGRE_MEDIA_PATH "Media")
    set(OGRE_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_MEDIA_PATH}")
  elseif (APPLE)
    set(OGRE_MEDIA_PATH "Media")
    if(APPLE_IOS)
      set(OGRE_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
    else()
      set(OGRE_MEDIA_DIR_REL "../${OGRE_MEDIA_PATH}")
    endif()
  elseif (UNIX)
    set(OGRE_MEDIA_PATH "share/OGRE-${OGRE_SOVERSION}/Media")
    set(OGRE_MEDIA_DIR_REL "./Media")
  endif()
else ()
  set(OGRE_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_MEDIA_PATH}")
endif()

if (NOT OGRE_PLUGINS_PATH)
  if (WIN32)
    set(OGRE_PLUGINS_PATH "${OGRE_BIN_DIRECTORY}")
    set(OGRE_PLUGIN_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_BIN_DIRECTORY}")
  else ()
    set(OGRE_PLUGINS_PATH "${OGRE_LIB_DIRECTORY}/OGRE")
    set(OGRE_PLUGIN_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE")
  endif()
else ()
  set(OGRE_PLUGIN_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_PLUGINS_PATH}")
endif()

if (NOT OGRE_CFG_INSTALL_PATH)
  if (WIN32 OR APPLE)
    set(OGRE_CFG_INSTALL_PATH "${OGRE_BIN_DIRECTORY}")
  elseif (UNIX)
    set(OGRE_CFG_INSTALL_PATH "share/OGRE-${OGRE_SOVERSION}")
  endif()
endif()

if (WIN32)
  set(OGRE_TEST_MEDIA_DIR_REL "../Tests/${OGRE_MEDIA_PATH}")
  set(OGRE_SAMPLES_DIR_REL ".")
elseif (APPLE)
  if(APPLE_IOS)
    set(OGRE_TEST_MEDIA_DIR_REL "../../Tests/${OGRE_MEDIA_PATH}")
  else()
    set(OGRE_TEST_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/Tests/Media")
  endif()
  # these are resolved relative to the app bundle
  set(OGRE_SAMPLES_DIR_REL "Contents/Plugins/")
  set(OGRE_CFG_INSTALL_PATH "bin")
elseif (UNIX)
  set(OGRE_TEST_MEDIA_DIR_REL "${CMAKE_INSTALL_PREFIX}/Tests/Media")
  set(OGRE_SAMPLES_DIR_REL "${CMAKE_INSTALL_PREFIX}/${OGRE_LIB_DIRECTORY}/OGRE/Samples")
endif ()

# generate OgreConfigPaths.h
configure_file(${OGRE_TEMPLATES_DIR}/OgreConfigPaths.h.in ${PROJECT_BINARY_DIR}/include/OgreConfigPaths.h @ONLY)

if(WIN32)
  # we want relative paths inside the SDK
  set(OGRE_PLUGIN_DIR_REL ".")
  if (WINDOWS_STORE OR WINDOWS_PHONE)
    set(OGRE_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
  else()
    set(OGRE_MEDIA_DIR_REL "../${OGRE_MEDIA_PATH}")
    set(OGRE_TEST_MEDIA_DIR_REL "../Tests/${OGRE_MEDIA_PATH}")
  endif()
endif()

# configure plugins.cfg
if (NOT OGRE_BUILD_RENDERSYSTEM_D3D9)
  set(OGRE_COMMENT_RENDERSYSTEM_D3D9 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_D3D11)
  set(OGRE_COMMENT_RENDERSYSTEM_D3D11 "#")
endif ()
if (NOT MINGW AND CMAKE_SYSTEM_VERSION VERSION_LESS "6.0")
  set(OGRE_COMMENT_RENDERSYSTEM_D3D11 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GL)
  set(OGRE_COMMENT_RENDERSYSTEM_GL "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
  set(OGRE_COMMENT_RENDERSYSTEM_GL3PLUS "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_GLES2)
  set(OGRE_COMMENT_RENDERSYSTEM_GLES2 "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_METAL)
  set(OGRE_COMMENT_RENDERSYSTEM_METAL "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_TINY)
  set(OGRE_COMMENT_RENDERSYSTEM_TINY "#")
endif ()
if (NOT OGRE_BUILD_RENDERSYSTEM_VULKAN)
  set(OGRE_COMMENT_RENDERSYSTEM_VULKAN "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_BSP)
  set(OGRE_COMMENT_PLUGIN_BSP "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_OCTREE)
  set(OGRE_COMMENT_PLUGIN_OCTREE "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PCZ)
  set(OGRE_COMMENT_PLUGIN_PCZ "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_PFX)
  set(OGRE_COMMENT_PLUGIN_PARTICLEFX "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_CG)
  set(OGRE_COMMENT_PLUGIN_CG "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_GLSLANG)
  set(OGRE_COMMENT_PLUGIN_GLSLANG "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_STBI)
  set(OGRE_COMMENT_PLUGIN_STBI "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_RSIMAGE OR OGRE_BUILD_PLUGIN_STBI)
  # has to be explicitely requested by disabeling STBI
  set(OGRE_COMMENT_PLUGIN_RSIMAGE "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_DOT_SCENE)
  set(OGRE_COMMENT_PLUGIN_DOT_SCENE "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_ASSIMP)
  set(OGRE_COMMENT_PLUGIN_ASSIMP "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_FREEIMAGE OR OGRE_BUILD_PLUGIN_STBI OR OGRE_BUILD_PLUGIN_RSIMAGE)
  # has to be explicitely requested by disabeling STBI
  set(OGRE_COMMENT_PLUGIN_FREEIMAGE "#")
endif ()
if (NOT OGRE_BUILD_PLUGIN_EXRCODEC OR OGRE_COMMENT_PLUGIN_FREEIMAGE OR OGRE_BUILD_PLUGIN_RSIMAGE)
  # overlaps with freeimage
  set(OGRE_COMMENT_PLUGIN_EXRCODEC "#")
endif ()
if(NOT OGRE_BUILD_TESTS)
  set(OGRE_COMMENT_PLAYPENTESTS "#")
endif()

set(OGRE_SAMPLE_RESOURCES "")

set(OGRE_CORE_MEDIA_DIR "${OGRE_MEDIA_DIR_REL}")

# CREATE CONFIG FILES - INSTALL VERSIONS

if(OGRE_INSTALL_SAMPLES)
  # deal with sample resources
  configure_file(${OGRE_TEMPLATES_DIR}/sample_resources.cfg.in ${PROJECT_BINARY_DIR}/sample_resources.cfg)
  file(READ ${PROJECT_BINARY_DIR}/sample_resources.cfg OGRE_SAMPLE_RESOURCES)
endif()

configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/inst/bin/resources.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/inst/bin/plugins.cfg)
configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/inst/bin/samples.cfg)

# install resource files
install(FILES 
  ${PROJECT_BINARY_DIR}/inst/bin/resources.cfg
  ${PROJECT_BINARY_DIR}/inst/bin/plugins.cfg
  DESTINATION "${OGRE_CFG_INSTALL_PATH}"
)

# CREATE CONFIG FILES - BUILD DIR VERSIONS
if (NOT (APPLE_IOS OR WINDOWS_STORE OR WINDOWS_PHONE))
  set(OGRE_MEDIA_DIR_REL "${PROJECT_SOURCE_DIR}/Samples/Media")
  set(OGRE_CORE_MEDIA_DIR "${PROJECT_SOURCE_DIR}/Media")
  set(OGRE_TEST_MEDIA_DIR_REL "${PROJECT_SOURCE_DIR}/Tests/Media")
else ()
  # iOS needs to use relative paths in the config files
  set(OGRE_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
  set(OGRE_CORE_MEDIA_DIR "${OGRE_MEDIA_PATH}")
  set(OGRE_TEST_MEDIA_DIR_REL "${OGRE_MEDIA_PATH}")
endif ()

if (WIN32)
  set(OGRE_PLUGIN_DIR_REL ".")
  set(OGRE_SAMPLES_DIR_REL ".")
elseif (APPLE)
  set(OGRE_PLUGIN_DIR_REL "Contents/Frameworks/")
  set(OGRE_SAMPLES_DIR_REL "Contents/Plugins/")
elseif (UNIX)
  set(OGRE_PLUGIN_DIR_REL "${PROJECT_BINARY_DIR}/lib")
  set(OGRE_SAMPLES_DIR_REL "${PROJECT_BINARY_DIR}/lib")
endif ()

if(OGRE_BUILD_SAMPLES)
  # deal with sample resources
  configure_file(${OGRE_TEMPLATES_DIR}/sample_resources.cfg.in ${PROJECT_BINARY_DIR}/sample_resources.cfg)
  file(READ ${PROJECT_BINARY_DIR}/sample_resources.cfg OGRE_SAMPLE_RESOURCES)
endif()

if (WINDOWS_STORE OR WINDOWS_PHONE OR EMSCRIPTEN)
  # These platfroms requires all resources to be packaged inside the application bundle,
  # therefore install versions of configs would be copied and added as content file to each project.
elseif (MSVC AND NOT NMAKE)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/bin/release/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/bin/relwithdebinfo/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/bin/minsizerel/resources.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/bin/debug/resources.cfg)

  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/bin/release/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/bin/relwithdebinfo/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/bin/minsizerel/plugins.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/bin/debug/plugins.cfg)

  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/bin/release/samples.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/bin/relwithdebinfo/samples.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/bin/minsizerel/samples.cfg)
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/bin/debug/samples.cfg)
else() # other OS only need one cfg file
  # create resources.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/resources.cfg.in ${PROJECT_BINARY_DIR}/bin/resources.cfg)
  # create plugins.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/plugins.cfg.in ${PROJECT_BINARY_DIR}/bin/plugins.cfg)
  # create samples.cfg
  configure_file(${OGRE_TEMPLATES_DIR}/samples.cfg.in ${PROJECT_BINARY_DIR}/bin/samples.cfg)
endif ()


# Create the CMake package files
include(CMakePackageConfigHelpers)

if (NOT OGRE_CMAKE_DIR)
  if(WIN32 OR APPLE)
    set(OGRE_CMAKE_DIR "CMake")
  else()
    set(OGRE_CMAKE_DIR "${OGRE_LIB_DIRECTORY}/OGRE/cmake")
  endif()
endif()

configure_package_config_file(${OGRE_TEMPLATES_DIR}/OGREConfig.cmake.in ${PROJECT_BINARY_DIR}/cmake/OGREConfig.cmake
    INSTALL_DESTINATION ${OGRE_CMAKE_DIR}
    PATH_VARS OGRE_MEDIA_PATH OGRE_PLUGINS_PATH OGRE_CFG_INSTALL_PATH CMAKE_INSTALL_PREFIX)
write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/cmake/OGREConfigVersion.cmake 
    VERSION ${OGRE_VERSION} 
    COMPATIBILITY SameMajorVersion)
install(FILES
   ${PROJECT_BINARY_DIR}/cmake/OGREConfig.cmake
   ${PROJECT_BINARY_DIR}/cmake/OGREConfigVersion.cmake
   DESTINATION ${OGRE_CMAKE_DIR}
)

install(EXPORT OgreTargetsRelease CONFIGURATIONS Release MinSizeRel None "" DESTINATION ${OGRE_CMAKE_DIR} FILE OgreTargets.cmake)
install(EXPORT OgreTargetsRelWithDebInfo CONFIGURATIONS RelWithDebInfo DESTINATION ${OGRE_CMAKE_DIR} FILE OgreTargets.cmake)
install(EXPORT OgreTargetsDebug CONFIGURATIONS Debug DESTINATION ${OGRE_CMAKE_DIR} FILE OgreTargets.cmake)


# Create the pkg-config package files on Unix systems
if (UNIX OR MINGW)
  if (MINGW)
    set(OGRE_PLUGIN_EXT ".dll")
  else()
    set(OGRE_PLUGIN_EXT ".so")
  endif()
  set(OGRE_PAGING_ADDITIONAL_PACKAGES "")
  if (OGRE_STATIC)
    set(OGRE_PLUGIN_EXT ".a")
  endif ()

  set(OGRE_ADDITIONAL_LIBS "")

  set(OGRE_CFLAGS "")
  set(OGRE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
  if (OGRE_CONFIG_THREADS GREATER 0)
    set(OGRE_CFLAGS "-pthread")
    set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} -lpthread")
  endif ()
  if (OGRE_STATIC)
    if (OGRE_CONFIG_THREADS AND OGRE_CONFIG_THREAD_PROVIDER STREQUAL "boost")
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} ${Boost_THREAD_LIBRARY_DEBUG}")
      else()
        set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} ${Boost_THREAD_LIBRARY_RELEASE}")
      endif()
    endif ()
    # there is no pkgconfig file for freeimage, so we need to add that lib manually
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} ${FreeImage_LIBRARY_DBG}")
    else()
      set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} ${FreeImage_LIBRARY_REL}")
    endif()
    configure_file(${OGRE_TEMPLATES_DIR}/OGREStatic.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE.pc @ONLY)
  else ()
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE.pc @ONLY)
  endif ()
  install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)

  # configure additional packages

  if (OGRE_BUILD_PLUGIN_PCZ)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-PCZ.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-PCZ.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-PCZ.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_PAGING)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Paging.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Paging.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Paging.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_MESHLODGENERATOR)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-MeshLodGenerator.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-MeshLodGenerator.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-MeshLodGenerator.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_TERRAIN)
    if (OGRE_BUILD_COMPONENT_PAGING)
      set(OGRE_PAGING_ADDITIONAL_PACKAGES ", OGRE-Paging = ${OGRE_VERSION}")
    endif ()
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Terrain.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Terrain.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Terrain.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-RTShaderSystem.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-RTShaderSystem.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-RTShaderSystem.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_PROPERTY)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Property.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Property.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Property.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_OVERLAY)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Overlay.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Overlay.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Overlay.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_VOLUME)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Volume.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Volume.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Volume.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()

  if (OGRE_BUILD_COMPONENT_BITES)
    if (SDL2_FOUND)
      set(OGRE_BITES_ADDITIONAL_PACKAGES ", sdl2")
    endif ()
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Bites.pc.in ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Bites.pc @ONLY)
    install(FILES ${PROJECT_BINARY_DIR}/pkgconfig/OGRE-Bites.pc DESTINATION ${OGRE_LIB_DIRECTORY}/pkgconfig)
  endif ()
endif ()
