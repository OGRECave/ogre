###############################################################
# Install media
###############################################################

# Set media target directory
if (WIN32)
  set(OGRE_MEDIA_PATH "media")
elseif (UNIX)
  set(OGRE_MEDIA_PATH "share/OGRE/media")
elseif (APPLE)
  # TODO
endif ()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${OGRE_MEDIA_PATH}
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeLists.txt" EXCLUDE
)
