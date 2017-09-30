#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Remotery
# Once done, this will define
#
#  Remotery_FOUND - system has Remotery
#  Remotery_INCLUDE_DIRS - the Remotery include directories
#  Remotery_LIBRARIES - link these to use Remotery

include(FindPkgMacros)

findpkg_begin(Remotery)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(Remotery_HOME)
getenv_path(OGRE_SDK)
getenv_path(OGRE_HOME)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_DEPENDENCIES_DIR)

# construct search paths
set(Remotery_PREFIX_PATH ${Remotery_HOME} ${ENV_Remotery_HOME}
  ${OGRE_DEPENDENCIES_DIR} ${ENV_OGRE_DEPENDENCIES_DIR}
  ${OGRE_SOURCE}/iOSDependencies ${ENV_OGRE_SOURCE}/iOSDependencies
  ${OGRE_SOURCE}/Dependencies )
create_search_paths(Remotery)
# redo search if prefix path changed
clear_if_changed(Remotery_PREFIX_PATH
  Remotery_INCLUDE_DIR
  Remotery_LIBRARIES_REL
  Remotery_LIBRARIES_DBG
)

use_pkgconfig(Remotery_PKGC Remotery)

# For Remotery, prefer static library over framework (important when referencing Remotery source build)
set(CMAKE_FIND_FRAMEWORK "LAST")

findpkg_framework(Remotery)

find_path(Remotery_INCLUDE_DIR NAMES Remotery.h HINTS ${Remotery_INC_SEARCH_PATH} ${Remotery_PKGC_INCLUDE_DIRS})
find_library(Remotery_LIBRARIES_REL NAMES Remotery HINTS ${Remotery_LIB_SEARCH_PATH} ${Remotery_PKGC_LIBRARIES_DIRS} PATH_SUFFIXES Release RelWithDebInfo MinSizeRel)
find_library(Remotery_LIBRARIES_DBG NAMES Remotery_d HINTS ${Remotery_LIB_SEARCH_PATH} ${Remotery_PKGC_LIBRARIES_DIRS} PATH_SUFFIXES Debug)

make_library_set(Remotery_LIBRARIES)

find_package_handle_standard_args( Remotery DEFAULT_MSG Remotery_INCLUDE_DIR Remotery_LIBRARIES )

if( Remotery_INCLUDE_DIR AND Remotery_LIBRARIES )
    set( Remotery_FOUND TRUE )
    set( Remotery_INCLUDE_DIRS ${Remotery_INCLUDE_DIR} )
endif()

if (WIN32)
	set(Remotery_BIN_SEARCH_PATH ${OGRE_DEPENDENCIES_DIR}/bin ${CMAKE_SOURCE_DIR}/Dependencies/bin ${Remotery_HOME}/dll
		${ENV_Remotery_HOME}/dll ${ENV_OGRE_DEPENDENCIES_DIR}/bin
		${OGRE_SOURCE}/Dependencies/bin ${ENV_OGRE_SOURCE}/Dependencies/bin
		${OGRE_SDK}/bin ${ENV_OGRE_SDK}/bin
		${OGRE_HOME}/bin ${ENV_OGRE_HOME}/bin)
	find_file(Remotery_BINARY_REL NAMES "Remotery.dll" HINTS ${Remotery_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" Release RelWithDebInfo MinSizeRel)
	find_file(Remotery_BINARY_DBG NAMES "Remotery_d.dll" HINTS ${Remotery_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" Debug )
endif()
mark_as_advanced(Remotery_BINARY_REL Remotery_BINARY_DBG)

findpkg_finish(Remotery)

# add parent of Remotery folder to support Remotery/Remotery.h
#add_parent_dir(Remotery_INCLUDE_DIRS Remotery_INCLUDE_DIR)

# Reset framework finding
set(CMAKE_FIND_FRAMEWORK "FIRST")
