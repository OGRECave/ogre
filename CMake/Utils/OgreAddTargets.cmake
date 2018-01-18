#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# add a new library to a project folder
# usage: ogre_add_library(FOLDER TARGETNAME LIBTYPE SOURCE_FILES)
function(ogre_add_library_to_folder FOLDER TARGETNAME LIBTYPE)
  add_library(${TARGETNAME} ${LIBTYPE} ${ARGN})
  if (OGRE_PROJECT_FOLDERS)
    set_property(TARGET ${TARGETNAME} PROPERTY FOLDER ${FOLDER})
  endif ()
endfunction(ogre_add_library_to_folder)
