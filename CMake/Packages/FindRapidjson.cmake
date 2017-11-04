#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find Rapidjson
# Once done, this will define
#
#  Rapidjson_FOUND - system has Rapidjson
#  Rapidjson_INCLUDE_DIRS - the Rapidjson include directories

include(FindPackageHandleStandardArgs)
include(FindPkgMacros)
findpkg_begin(Rapidjson)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(Rapidjson_HOME)
getenv_path(OGRE_SDK)
getenv_path(OGRE_HOME)
getenv_path(OGRE_SOURCE)
getenv_path(OGRE_DEPENDENCIES_DIR)

# construct search paths
set(Rapidjson_PREFIX_PATH ${Rapidjson_HOME} ${ENV_Rapidjson_HOME}
  ${OGRE_DEPENDENCIES_DIR} ${ENV_OGRE_DEPENDENCIES_DIR}
  ${OGRE_SOURCE}/iOSDependencies ${ENV_OGRE_SOURCE}/iOSDependencies
  ${OGRE_SOURCE}/Dependencies )
create_search_paths(Rapidjson)
# redo search if prefix path changed
clear_if_changed(Rapidjson_PREFIX_PATH
  Rapidjson_INCLUDE_DIR
)

use_pkgconfig(Rapidjson_PKGC Rapidjson)

# For Rapidjson, prefer static library over framework (important when referencing Rapidjson source build)
set(CMAKE_FIND_FRAMEWORK "LAST")

findpkg_framework(Rapidjson)

find_path(Rapidjson_INCLUDE_DIR NAMES rapidjson/rapidjson.h HINTS ${Rapidjson_INC_SEARCH_PATH} ${Rapidjson_PKGC_INCLUDE_DIRS})

find_package_handle_standard_args( Rapidjson DEFAULT_MSG Rapidjson_INCLUDE_DIR )

if( Rapidjson_INCLUDE_DIR )
    set( Rapidjson_FOUND TRUE )
    set( Rapidjson_INCLUDE_DIRS ${Rapidjson_INCLUDE_DIR} )
endif()

findpkg_finish(Rapidjson)

# add parent of Rapidjson folder to support rapidjson/rapidjson.h
#add_parent_dir(Rapidjson_INCLUDE_DIRS Rapidjson_INCLUDE_DIR)

# Reset framework finding
set(CMAKE_FIND_FRAMEWORK "FIRST")
