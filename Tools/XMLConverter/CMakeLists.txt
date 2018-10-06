#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure XMLConverter

set(HEADER_FILES 
  include/OgreXMLMeshSerializer.h
  include/OgreXMLPrerequisites.h
  include/OgreXMLSkeletonSerializer.h
)

set(SOURCE_FILES 
  src/main.cpp
  src/OgreXMLMeshSerializer.cpp
  src/OgreXMLSkeletonSerializer.cpp
)

# If TinyXML is not found on the system use the embedded version.
if(NOT TINYXML_FOUND)
  list(APPEND SOURCE_FILES
    src/tinystr.cpp
    src/tinyxml.cpp
    src/tinyxmlerror.cpp
    src/tinyxmlparser.cpp)

  list(APPEND HEADER_FILES
    include/tinystr.h
    include/tinyxml.h)

  set(TINYXML_INCLUDE_DIR "")
  set(TINYXML_LIBRARIES "")
endif(NOT TINYXML_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${TINYXML_INCLUDE_DIR})
add_definitions(-DTIXML_USE_STL)

if(UNIX)
    add_definitions(-Wno-shadow -Wno-implicit-fallthrough) # inside tinyxml sources
endif()

add_executable(OgreXMLConverter ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(OgreXMLConverter ${OGRE_LIBRARIES} ${TINYXML_LIBRARIES} ${OGRE_MeshLodGenerator_LIBRARIES})

if (APPLE)
    set_target_properties(OgreXMLConverter PROPERTIES
        LINK_FLAGS "-framework Cocoa")
endif ()
if (OGRE_PROJECT_FOLDERS)
	set_property(TARGET OgreXMLConverter PROPERTY FOLDER Tools)
endif ()
ogre_config_tool(OgreXMLConverter)
