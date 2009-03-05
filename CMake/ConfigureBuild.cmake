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

# add compile options necessary for threading
if (OGRE_CONFIG_THREADS AND UNIX)
  add_definitions(-pthread)
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

# determine system endianess
include(TestBigEndian)
test_big_endian(OGRE_CONFIG_BIG_ENDIAN)
if (NOT OGRE_CONFIG_BIG_ENDIAN)
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


# Create the pkg-config package file on Unix systems
if (UNIX)
  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(libdir ${CMAKE_INSTALL_PREFIX}/lib)
  set(includedir ${CMAKE_INSTALL_PREFIX}/include)
  set(PACKAGE "OGRE")
  set(VERSION ${OGRE_VERSION})
  if (OGRE_CONFIG_THREADS GREATER 0)
    set(OGRE_CFLAGS "-pthread")
    set(OGRE_THREAD_LIBS "-lpthread")
  endif ()
  set(exec_prefix ${CMAKE_INSTALL_PREFIX})
  configure_file(${OGRE_SOURCE_DIR}/OGRE.pc.in ${OGRE_BINARY_DIR}/pkgconfig/OGRE.pc @ONLY)
  install(FILES ${OGRE_BINARY_DIR}/pkgconfig/OGRE.pc DESTINATION lib/pkgconfig)
endif ()

# Create the CMake package files
if (WIN32)
  set(OGRE_CMAKE_DIR CMake)
elseif (UNIX)
  set(OGRE_CMAKE_DIR lib/cmake)
elseif (APPLE)
endif ()
configure_file(${OGRE_TEMPLATES_DIR}/OGREConfig.cmake.in ${OGRE_BINARY_DIR}/cmake/OGREConfig.cmake @ONLY)
configure_file(${OGRE_TEMPLATES_DIR}/OGREConfigVersion.cmake.in ${OGRE_BINARY_DIR}/cmake/OGREConfigVersion.cmake @ONLY)
install(FILES
  ${OGRE_BINARY_DIR}/cmake/OGREConfig.cmake
  ${OGRE_BINARY_DIR}/cmake/OGREConfigVersion.cmake
  DESTINATION ${OGRE_CMAKE_DIR}
)

