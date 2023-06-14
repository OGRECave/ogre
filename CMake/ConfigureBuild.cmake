#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#######################################################################
# This file takes care of configuring Ogre to build with the settings
# given in CMake. It creates the necessary config.h file and will
# also prepare package files for pkg-config and CMake.
#######################################################################

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(${BUILD_TYPE_LOWER} STREQUAL "debug")
  set(OGRE_DEBUG_MODE 1)
endif()

if (APPLE_IOS)
  set(OGRE_SET_BUILD_PLATFORM_APPLE_IOS 1)
  set(OGRE_STATIC TRUE)
  set(OGRE_STATIC_LIB TRUE)
endif()

# should we build static libs?
if (OGRE_STATIC)
  set(OGRE_LIB_TYPE STATIC)
else ()
  set(OGRE_LIB_TYPE SHARED)
endif ()

# configure threading options
set(OGRE_THREAD_PROVIDER 0)
if (OGRE_CONFIG_THREADS)
	if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "boost")
		set(OGRE_THREAD_PROVIDER 1)
		include_directories(${Boost_INCLUDE_DIRS})
		# On MSVC Boost usually tries to autolink boost libraries. However since
		# this behaviour is not available on all compilers, we need to find the libraries
		# ourselves, anyway. Disable auto-linking to avoid mess-ups.
		add_definitions(-DBOOST_ALL_NO_LIB)
        if (MINGW AND Boost_USE_STATIC_LIBS)
            # mingw needs this to link against static thread libraries
            add_definitions(-DBOOST_THREAD_USE_LIB)
        endif ()
		set(OGRE_THREAD_LIBRARIES ${Boost_LIBRARIES})
	endif ()

	if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "poco")
		set(OGRE_THREAD_PROVIDER 2)
		include_directories(${POCO_INCLUDE_DIRS})
		set(OGRE_THREAD_LIBRARIES ${POCO_LIBRARIES})
	endif ()

	if (OGRE_CONFIG_THREAD_PROVIDER STREQUAL "std")
		set(OGRE_THREAD_PROVIDER 4)
	endif ()

endif()

set(OGRE_ASSERT_MODE 2 CACHE STRING
	"Enable Ogre asserts. Possible values:
	0 - Standard asserts in debug builds, nothing in release builds.
	1 - Standard asserts in debug builds, exceptions in release builds.
	2 - Exceptions in debug & release builds."
)
set_property(CACHE OGRE_ASSERT_MODE PROPERTY STRINGS 0 1 2)

# determine config values depending on build options
set(OGRE_STATIC_LIB ${OGRE_STATIC})
set(OGRE_DOUBLE_PRECISION ${OGRE_CONFIG_DOUBLE})
set(OGRE_NODE_INHERIT_TRANSFORM ${OGRE_CONFIG_NODE_INHERIT_TRANSFORM})
set(OGRE_SET_ASSERT_MODE ${OGRE_ASSERT_MODE})
set(OGRE_SET_THREADS ${OGRE_CONFIG_THREADS})
set(OGRE_SET_THREAD_PROVIDER ${OGRE_THREAD_PROVIDER})
if (NOT OGRE_CONFIG_ENABLE_MESHLOD)
  set(OGRE_NO_MESHLOD 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_DDS)
  set(OGRE_NO_DDS_CODEC 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_PVRTC)
  set(OGRE_NO_PVRTC_CODEC 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_ETC)
  set(OGRE_NO_ETC_CODEC 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_ASTC)
  set(OGRE_NO_ASTC_CODEC 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_ZIP)
  set(OGRE_NO_ZIP_ARCHIVE 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)
  set(OGRE_NO_GLES2_CG_SUPPORT 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
  set(OGRE_NO_GLES2_GLSL_OPTIMISER 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT)
  set(OGRE_NO_GL_STATE_CACHE_SUPPORT 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_GLES3_SUPPORT)
  set(OGRE_NO_GLES3_SUPPORT 1)
endif()
if (NOT OGRE_CONFIG_ENABLE_TBB_SCHEDULER)
  set(OGRE_NO_TBB_SCHEDULER 1)
endif()
if (OGRE_TEST_BIG_ENDIAN)
  set(OGRE_CONFIG_BIG_ENDIAN 1)
else ()
  set(OGRE_CONFIG_LITTLE_ENDIAN 1)
endif ()
set(RTSHADER_SYSTEM_BUILD_CORE_SHADERS ${OGRE_BUILD_RTSHADERSYSTEM_SHADERS})
set(RTSHADER_SYSTEM_BUILD_EXT_SHADERS ${OGRE_BUILD_RTSHADERSYSTEM_SHADERS})
if (NOT OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO)
  set(OGRE_NO_QUAD_BUFFER_STEREO 1)
endif()
if(SDL2_FOUND OR EMSCRIPTEN)
    set(OGRE_BITES_HAVE_SDL 1)
endif()

# determine if strtol_l is supported
include(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(strtol_l HAVE_STRTOL_L)
if (NOT HAVE_STRTOL_L)
  set(OGRE_NO_LOCALE_STRCONVERT 1)
endif ()

# generate OgreBuildSettings.h
configure_file(${OGRE_TEMPLATES_DIR}/OgreComponents.h.in ${PROJECT_BINARY_DIR}/include/OgreComponents.h @ONLY)
configure_file(${OGRE_TEMPLATES_DIR}/OgreBuildSettings.h.in ${PROJECT_BINARY_DIR}/include/OgreBuildSettings.h @ONLY)
configure_file(${OGRE_TEMPLATES_DIR}/OgreRTShaderConfig.h.in ${PROJECT_BINARY_DIR}/include/OgreRTShaderConfig.h @ONLY)
configure_file(${OGRE_TEMPLATES_DIR}/OgreGLES2Config.h.in ${PROJECT_BINARY_DIR}/include/OgreGLES2Config.h @ONLY)

set(OGRE_LIB_SUFFIX "")

if (OGRE_STATIC)
  set(OGRE_LIB_SUFFIX "${OGRE_LIB_SUFFIX}Static")
endif ()
if (BUILD_TYPE_LOWER STREQUAL "debug" AND WIN32)
  set(OGRE_LIB_SUFFIX "${OGRE_LIB_SUFFIX}_d")
endif ()

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

if(OGRE_CONFIG_STATIC_LINK_CRT)
#We statically link to reduce dependencies
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
    if(${flag_var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MDd")
endforeach(flag_var)
endif(OGRE_CONFIG_STATIC_LINK_CRT)
