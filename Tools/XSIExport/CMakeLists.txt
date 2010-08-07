#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES 
  include/OgreXSIHelper.h
  include/OgreXSIMaterialExporter.h
  include/OgreXSIMeshExporter.h
  include/OgreXSISkeletonExporter.h
)

set(SOURCE_FILES 
  src/OgreXSIExport.cpp
  src/OgreXSIHelper.cpp
  src/OgreXSIMaterialExporter.cpp
  src/OgreXSIMeshExporter.cpp
  src/OgreXSISkeletonExporter.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${Softimage_INCLUDE_DIR})

add_library(OgreXSIExporter MODULE ${HEADER_FILES} ${SOURCE_FILES} "misc/OgreXSIExport.def")
target_link_libraries(OgreXSIExporter ${OGRE_LIBRARIES} ${Softimage_LIBRARIES})
