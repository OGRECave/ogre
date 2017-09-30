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
  Remotery_LIBRARIES
)

use_pkgconfig(Remotery_PKGC Remotery)

# For Remotery, prefer static library over framework (important when referencing Remotery source build)
set(CMAKE_FIND_FRAMEWORK "LAST")

findpkg_framework(Remotery)

find_path(Remotery_INCLUDE_DIR NAMES Remotery.h HINTS ${Remotery_INC_SEARCH_PATH} ${Remotery_PKGC_INCLUDE_DIRS})
find_library(Remotery_LIBRARIES NAMES Remotery HINTS ${Remotery_LIB_SEARCH_PATH} ${Remotery_PKGC_LIBRARIES_DIRS} PATH_SUFFIXES Release RelWithDebInfo MinSizeRel Debug)

find_package_handle_standard_args( Remotery DEFAULT_MSG Remotery_INCLUDE_DIR Remotery_LIBRARIES )

if( Remotery_INCLUDE_DIR AND Remotery_LIBRARIES )
    set( Remotery_FOUND TRUE )
    set( Remotery_INCLUDE_DIRS ${Remotery_INCLUDE_DIR} )
endif()

findpkg_finish(Remotery)

# add parent of Remotery folder to support Remotery/Remotery.h
#add_parent_dir(Remotery_INCLUDE_DIRS Remotery_INCLUDE_DIR)

# Reset framework finding
set(CMAKE_FIND_FRAMEWORK "FIRST")
