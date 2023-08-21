#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Cg
# Once done, this will define
#
#  Cg_FOUND - system has Cg
#  Cg_INCLUDE_DIRS - the Cg include directories 
#  Cg_LIBRARIES - link these to use Cg

include(FindPkgMacros)
findpkg_begin(Cg)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(Cg_HOME)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_HOME)

# construct search paths
set(Cg_PREFIX_PATH ${Cg_HOME} ${ENV_Cg_HOME}
  ${OGRE_SOURCE}/Dependencies
  ${ENV_OGRE_SOURCE}/Dependencies
  ${OGRE_HOME} ${ENV_OGRE_HOME}
  /opt/nvidia-cg-toolkit)
create_search_paths(Cg)
# redo search if prefix path changed
clear_if_changed(Cg_PREFIX_PATH
  Cg_LIBRARY_FWK
  Cg_LIBRARY_REL
  Cg_LIBRARY_DBG
  Cg_INCLUDE_DIR
)

set(Cg_LIBRARY_NAMES Cg cg)
get_debug_names(Cg_LIBRARY_NAMES)

use_pkgconfig(Cg_PKGC Cg)

findpkg_framework(Cg)

find_path(Cg_INCLUDE_DIR NAMES cg.h HINTS ${Cg_FRAMEWORK_INCLUDES} ${Cg_INC_SEARCH_PATH} ${Cg_PKGC_INCLUDE_DIRS} PATH_SUFFIXES Cg)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (Cg_LIB_SEARCH_PATH ${Cg_HOME}/lib.x64 ${ENV_Cg_LIB64_PATH}
    ${ENV_Cg_HOME}/lib.x64 ${Cg_LIB_SEARCH_PATH})
else()
  set (Cg_LIB_SEARCH_PATH ${Cg_HOME}/lib ${ENV_Cg_LIB_PATH}
    ${ENV_Cg_HOME}/lib ${Cg_LIB_SEARCH_PATH})
endif()

find_library(Cg_LIBRARY_REL NAMES ${Cg_LIBRARY_NAMES} HINTS ${Cg_LIB_SEARCH_PATH} ${Cg_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Release RelWithDebInfo MinSizeRel)
find_library(Cg_LIBRARY_DBG NAMES ${Cg_LIBRARY_NAMES_DBG} HINTS ${Cg_LIB_SEARCH_PATH} ${Cg_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Debug)
make_library_set(Cg_LIBRARY)

if (WIN32)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(Cg_BIN_SEARCH_PATH ${OGRE_DEPENDENCIES_DIR}/bin ${CMAKE_SOURCE_DIR}/Dependencies/bin ${Cg_HOME}/bin.x64
			${ENV_Cg_BIN64_PATH} ${ENV_Cg_HOME}/bin.x64 ${ENV_OGRE_DEPENDENCIES_DIR}/bin
			${OGRE_SOURCE}/Dependencies/bin ${ENV_OGRE_SOURCE}/Dependencies/bin
			${OGRE_SDK}/bin ${ENV_OGRE_SDK}/bin
			${OGRE_HOME}/bin ${ENV_OGRE_HOME}/bin)
	else()
		set(Cg_BIN_SEARCH_PATH ${OGRE_DEPENDENCIES_DIR}/bin ${CMAKE_SOURCE_DIR}/Dependencies/bin ${Cg_HOME}/bin
			${ENV_Cg_BIN_PATH} ${ENV_Cg_HOME}/bin ${ENV_OGRE_DEPENDENCIES_DIR}/bin
			${OGRE_SOURCE}/Dependencies/bin ${ENV_OGRE_SOURCE}/Dependencies/bin
			${OGRE_SDK}/bin ${ENV_OGRE_SDK}/bin
			${OGRE_HOME}/bin ${ENV_OGRE_HOME}/bin)
	endif()
	find_file(Cg_BINARY_REL NAMES "cg.dll" HINTS ${Cg_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" release relwithdebinfo minsizerel)
	find_file(Cg_BINARY_DBG NAMES "cg.dll" HINTS ${Cg_BIN_SEARCH_PATH}
	  PATH_SUFFIXES "" debug )
endif()
mark_as_advanced(Cg_BINARY_REL Cg_BINARY_DBG)

findpkg_finish(Cg)
add_parent_dir(Cg_INCLUDE_DIRS Cg_INCLUDE_DIR)
