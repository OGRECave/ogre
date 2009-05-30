#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Find OGRE includes and library
#
# This module defines
#  OGRE_INCLUDE_DIRS
#  OGRE_LIBRARIES, the libraries to link against to use OGRE.
#  OGRE_LIBRARY_DIRS, the location of the libraries
#  OGRE_FOUND, If false, do not try to use OGRE
#

include(FindPackageMessage)

set(OGRE_PREFIX_DIR "@CMAKE_INSTALL_PREFIX@")
get_filename_component(OGRE_LIBRARY_DIRS "${OGRE_PREFIX_DIR}/lib" ABSOLUTE)
get_filename_component(OGRE_INCLUDE_DIRS "${OGRE_PREFIX_DIR}/include/OGRE" ABSOLUTE)
set(OGRE_LIBRARIES "OgreMain")

message(STATUS "Found OGRE")
message(STATUS "  libraries : '${OGRE_LIBRARIES}' from ${OGRE_LIBRARY_DIRS}")
message(STATUS "  includes  : ${OGRE_INCLUDE_DIRS}")
