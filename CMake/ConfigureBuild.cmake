#######################################################################
# This file takes care of configuring Ogre to build with the settings
# given in CMake. It creates the necessary config.h file and will 
# also prepare package files for pkg-config and CMake.
#######################################################################

# should we build static libs?
if (OGRE_STATIC)
  set(OGRE_LIB_TYPE STATIC)
else ()
  set(OGRE_LIB_TYPE SHARED)
endif ()

# configure threading options
if (OGRE_CONFIG_THREADS)
  if (UNIX)
    add_definitions(-pthread)
  endif ()
  include_directories(${Boost_INCLUDE_DIRS})
  # On MSVC Boost usually tries to autolink boost libraries. However since
  # this behaviour is not available on all compilers, we need to find the libraries
  # ourselves, anyway. Disable auto-linking to avoid mess-ups.
  add_definitions(-DBOOST_ALL_NO_LIB)
endif()


# determine config values depending on build options 
set(OGRE_SET_DOUBLE 0)
set(OGRE_SET_ALLOCATOR ${OGRE_CONFIG_ALLOCATOR})
set(OGRE_SET_CONTAINERS_USE_ALLOCATOR 0)
set(OGRE_SET_STRING_USE_ALLOCATOR 0)
set(OGRE_SET_MEMTRACK_DEBUG 0)
set(OGRE_SET_MEMTRACK_RELEASE 0)
set(OGRE_SET_THREADS ${OGRE_CONFIG_THREADS})
set(OGRE_SET_DISABLE_FREEIMAGE 0)
set(OGRE_SET_DISABLE_DDS 0)
set(OGRE_SET_NEW_COMPILERS 0)
set(OGRE_STATIC_LIB 0)
if (OGRE_CONFIG_DOUBLE)
  set(OGRE_SET_DOUBLE 1)
endif()
if (OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR)
  set(OGRE_SET_CONTAINERS_USE_ALLOCATOR 1)
endif ()
if (OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR)
  set(OGRE_SET_STRING_USE_ALLOCATOR 1)
endif ()
if (OGRE_CONFIG_MEMTRACK_DEBUG)
  set(OGRE_SET_MEMTRACK_DEBUG 1)
endif()
if (OGRE_CONFIG_MEMTRACK_RELEASE)
  set(OGRE_SET_MEMTRACK_RELEASE 1)
endif()
if (OGRE_CONFIG_DISABLE_FREEIMAGE)
  set(OGRE_SET_DISABLE_FREEIMAGE 1)
endif()
if (OGRE_CONFIG_DISABLE_DDS)
  set(OGRE_SET_DISABLE_DDS 1)
endif()
if(OGRE_CONFIG_NEW_COMPILERS)
  set(OGRE_SET_NEW_COMPILERS 1)
endif()
if (OGRE_STATIC)
  set(OGRE_STATIC_LIB 1)
endif()
add_definitions(-DHAVE_OGRE_BUILDSETTINGS_H)

if (OGRE_TEST_BIG_ENDIAN)
  set(OGRE_CONFIG_BIG_ENDIAN 1)
else ()
  set(OGRE_CONFIG_LITTLE_ENDIAN 1)
endif ()

# generate buildsettings.h 
configure_file(${OGRE_TEMPLATES_DIR}/buildsettings.h.in ${OGRE_BINARY_DIR}/include/buildsettings.h @ONLY)
install(FILES ${OGRE_BINARY_DIR}/include/buildsettings.h DESTINATION include/OGRE)

# Read contents of the OgreConfig.h file
file(READ "${OGRE_SOURCE_DIR}/OgreMain/include/OgreConfig.h" OGRE_CONFIG_H)
# add HAVE_OGRE_BUILDSETTINGS_H preprocessor define
file(WRITE ${OGRE_BINARY_DIR}/include/OgreConfig.h "#define HAVE_OGRE_BUILDSETTINGS_H\n${OGRE_CONFIG_H}")
install(FILES ${OGRE_BINARY_DIR}/include/OgreConfig.h DESTINATION include/OGRE)


# Create the pkg-config package files on Unix systems
if (UNIX)
  set(OGRE_LIB_SUFFIX "")
  set(OGRE_PLUGIN_PREFIX "")
  set(OGRE_PLUGIN_EXT ".so")
  if (OGRE_STATIC)
    set(OGRE_LIB_SUFFIX "${OGRE_LIB_SUFFIX}Static")
    set(OGRE_PLUGIN_PREFIX "lib")
    set(OGRE_PLUGIN_EXT ".a")
  endif ()
  string(TOLOWER "${CMAKE_BUILD_TYPE}" OGRE_BUILD_TYPE)
  if (OGRE_BUILD_TYPE STREQUAL "debug")
    set(OGRE_LIB_SUFFIX "${OGRE_LIB_SUFFIX}_d")
  endif ()

  set(OGRE_ADDITIONAL_LIBS "")
  set(OGRE_CFLAGS "")
  set(OGRE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
  if (OGRE_CONFIG_THREADS GREATER 0)
    set(OGRE_CFLAGS "-pthread")
    set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} -lpthread")
  endif ()
  if (OGRE_STATIC)
    if (OGRE_CONFIG_THREADS)
      set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} -lboost-thread-mt")
    endif ()
    # there is no pkgconfig file for freeimage, so we need to add that lib manually
    set(OGRE_ADDITIONAL_LIBS "${OGRE_ADDITIONAL_LIBS} -lfreeimage")
    configure_file(${OGRE_TEMPLATES_DIR}/OGREStatic.pc.in ${OGRE_BINARY_DIR}/pkgconfig/OGRE${OGRE_LIB_SUFFIX}.pc @ONLY)
  else ()
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE.pc.in ${OGRE_BINARY_DIR}/pkgconfig/OGRE${OGRE_LIB_SUFFIX}.pc @ONLY)
  endif ()
  install(FILES ${OGRE_BINARY_DIR}/pkgconfig/OGRE${OGRE_LIB_SUFFIX}.pc DESTINATION lib/pkgconfig)

  # configure additional packages
  
  if (OGRE_BUILD_CEGUIRENDERER)
    configure_file(${OGRE_TEMPLATES_DIR}/CEGUI-OGRE.pc.in ${OGRE_BINARY_DIR}/pkgconfig/CEGUI-OGRE${OGRE_LIB_SUFFIX}.pc @ONLY)
    install(FILES ${OGRE_BINARY_DIR}/pkgconfig/CEGUI-OGRE${OGRE_LIB_SUFFIX}.pc DESTINATION lib/pkgconfig)
  endif ()

  if (OGRE_BUILD_PLUGIN_PCZ)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-PCZ.pc.in ${OGRE_BINARY_DIR}/pkgconfig/OGRE-PCZ${OGRE_LIB_SUFFIX}.pc @ONLY)
    install(FILES ${OGRE_BINARY_DIR}/pkgconfig/OGRE-PCZ${OGRE_LIB_SUFFIX}.pc DESTINATION lib/pkgconfig)
  endif ()
  
  if (OGRE_BUILD_COMPONENT_PAGING)
    configure_file(${OGRE_TEMPLATES_DIR}/OGRE-Paging.pc.in ${OGRE_BINARY_DIR}/pkgconfig/OGRE-Paging${OGRE_LIB_SUFFIX}.pc @ONLY)
    install(FILES ${OGRE_BINARY_DIR}/pkgconfig/OGRE-Paging${OGRE_LIB_SUFFIX}.pc DESTINATION lib/pkgconfig)
  endif ()
endif ()

if (OGRE_STANDALONE_BUILD)
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
endif()

### Commented because the FindOGRE script can currently fill this role better ###
# # Create the CMake package files
# if (WIN32)
#   set(OGRE_CMAKE_DIR CMake)
# elseif (UNIX)
#   set(OGRE_CMAKE_DIR lib/cmake)
# elseif (APPLE)
# endif ()
# configure_file(${OGRE_TEMPLATES_DIR}/OGREConfig.cmake.in ${OGRE_BINARY_DIR}/cmake/OGREConfig.cmake @ONLY)
# configure_file(${OGRE_TEMPLATES_DIR}/OGREConfigVersion.cmake.in ${OGRE_BINARY_DIR}/cmake/OGREConfigVersion.cmake @ONLY)
# install(FILES
#   ${OGRE_BINARY_DIR}/cmake/OGREConfig.cmake
#   ${OGRE_BINARY_DIR}/cmake/OGREConfigVersion.cmake
#   DESTINATION ${OGRE_CMAKE_DIR}
# )
# 
