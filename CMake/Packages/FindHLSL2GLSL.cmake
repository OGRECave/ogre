#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find HLSL2GLSL
# Once done, this will define
#
#  HLSL2GLSL_FOUND - system has HLSL2GLSL
#  HLSL2GLSL_INCLUDE_DIRS - the HLSL2GLSL include directories 
#  HLSL2GLSL_LIBRARIES - link these to use HLSL2GLSL

include(FindPkgMacros)
findpkg_begin(HLSL2GLSL)

# construct search paths
set(HLSL2GLSL_PREFIX_PATH ${HLSL2GLSL_HOME} $ENV{HLSL2GLSL_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME} ${OGRE_DEP_SEARCH_PATH})
create_search_paths(HLSL2GLSL)
# redo search if prefix path changed
clear_if_changed(HLSL2GLSL_PREFIX_PATH
  HLSL2GLSL_LIBRARY_FWK
  HLSL2GLSL_LIBRARY_REL
  HLSL2GLSL_LIBRARY_DBG
  HLSL2GLSL_INCLUDE_DIR
)

set(HLSL2GLSL_LIBRARY_NAMES hlsl2glsl)
get_debug_names(HLSL2GLSL_LIBRARY_NAMES)

use_pkgconfig(HLSL2GLSL_PKGC HLSL2GLSL)

findpkg_framework(HLSL2GLSL)
find_path(HLSL2GLSL_INCLUDE_DIR NAMES hlsl2glsl.h HINTS ${HLSL2GLSL_INC_SEARCH_PATH} ${HLSL2GLSL_PKGC_INCLUDE_DIRS} PATH_SUFFIXES HLSL2GLSL)
find_library(HLSL2GLSL_LIBRARY_REL NAMES ${HLSL2GLSL_LIBRARY_NAMES} HINTS ${HLSL2GLSL_LIB_SEARCH_PATH} ${HLSL2GLSL_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Release RelWithDebInfo MinSizeRel)
find_library(HLSL2GLSL_LIBRARY_DBG NAMES ${HLSL2GLSL_LIBRARY_NAMES_DBG} HINTS ${HLSL2GLSL_LIB_SEARCH_PATH} ${HLSL2GLSL_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Debug)
make_library_set(HLSL2GLSL_LIBRARY)

findpkg_finish(HLSL2GLSL)
add_parent_dir(HLSL2GLSL_INCLUDE_DIRS HLSL2GLSL_INCLUDE_DIR)

