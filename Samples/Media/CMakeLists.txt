#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

###############################################################
# Install media
###############################################################

# Set media target directory
if (WIN32)
  set(OGRE_MEDIA_PATH "media")
elseif (APPLE)
  set(OGRE_MEDIA_PATH "Media")
elseif (UNIX)
  set(OGRE_MEDIA_PATH "share/OGRE/Media")
endif ()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${OGRE_MEDIA_PATH}
  PATTERN "CMakeLists.txt" EXCLUDE
)
