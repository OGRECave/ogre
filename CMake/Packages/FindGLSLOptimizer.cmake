#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find GLSL Optimizer
# Once done, this will define
#
#  GLSL_Optimizer_FOUND - system has GLSL_Optimizer
#  GLSL_Optimizer_INCLUDE_DIRS - the GLSL_Optimizer include directories 
#  GLSL_Optimizer_LIBRARIES - link these to use GLSL_Optimizer

include(FindPkgMacros)
findpkg_begin(GLSL_Optimizer)

# construct search paths
set(GLSL_Optimizer_PREFIX_PATH ${GLSL_Optimizer_HOME} $ENV{GLSL_Optimizer_HOME}
  ${OGRE_HOME} $ENV{OGRE_HOME} ${OGRE_DEP_SEARCH_PATH})
create_search_paths(GLSL_Optimizer)
# redo search if prefix path changed
clear_if_changed(GLSL_Optimizer_PREFIX_PATH
  GLSL_Optimizer_LIBRARY_FWK
  GLSL_Optimizer_LIBRARY_REL
  GLSL_Optimizer_LIBRARY_DBG
  GLSL_Optimizer_INCLUDE_DIR
)

set(GLSL_Optimizer_LIBRARY_NAMES mesaglsl2 glsl_optimizer)
get_debug_names(GLSL_Optimizer_LIBRARY_NAMES)

use_pkgconfig(GLSL_Optimizer_PKGC GLSL_Optimizer)

findpkg_framework(GLSL_Optimizer)
find_path(GLSL_Optimizer_INCLUDE_DIR NAMES glsl_optimizer.h HINTS ${GLSL_Optimizer_INC_SEARCH_PATH} ${GLSL_Optimizer_PKGC_INCLUDE_DIRS} PATH_SUFFIXES GLSL_Optimizer)
find_library(GLSL_Optimizer_LIBRARY_REL NAMES ${GLSL_Optimizer_LIBRARY_NAMES} HINTS ${GLSL_Optimizer_LIB_SEARCH_PATH} ${GLSL_Optimizer_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Release RelWithDebInfo MinSizeRel)
find_library(GLSL_Optimizer_LIBRARY_DBG NAMES ${GLSL_Optimizer_LIBRARY_NAMES_DBG} HINTS ${GLSL_Optimizer_LIB_SEARCH_PATH} ${GLSL_Optimizer_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" Debug)
make_library_set(GLSL_Optimizer_LIBRARY)

findpkg_finish(GLSL_Optimizer)
add_parent_dir(GLSL_Optimizer_INCLUDE_DIRS GLSL_Optimizer_INCLUDE_DIR)

