#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find AMDQBS
# Once done, this will define
#
#  AMDQBS_FOUND - system has AMDQBS
#  AMDQBS_INCLUDE_DIRS - the AMDQBS include directories 

include(FindPkgMacros)
findpkg_begin(AMDQBS)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(AMDQBS_HOME)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_HOME)

# construct search paths
set(AMDQBS_PREFIX_PATH ${OGRE_SOURCE}/Dependencies ${ENV_OGRE_SOURCE}/Dependencies ${OGRE_HOME} ${ENV_OGRE_HOME})
create_search_paths(AMDQBS)

# redo search if prefix path changed
clear_if_changed(AMDQBS_PREFIX_PATH NVAPI_INCLUDE_DIR)

use_pkgconfig(AMDQBS_PKGC AMDQBS)

findpkg_framework(AMDQBS)

find_path(AMDQBS_INCLUDE_DIR NAMES AmdDxExt.h HINTS ${AMDQBS_FRAMEWORK_INCLUDES} ${AMDQBS_INC_SEARCH_PATH} ${AMDQBS_PKGC_INCLUDE_DIRS} PATH_SUFFIXES AMDQBS)

findpkg_finish(AMDQBS)
add_parent_dir(AMDQBS_INCLUDE_DIRS AMDQBS_INCLUDE_DIR)