#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Paging optional component
############################################################

PROJECT(OgreProperty)

# define header and source files for the library
set (HEADER_FILES
	include/OgreProperty.h
	include/OgrePropertyPrerequisites.h
)

set (SOURCE_FILES
	src/OgreProperty.cpp
)


# Add needed definitions 
add_definitions(-DOGRE_PROPERTY_EXPORTS -D_MT -D_USRDLL)

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/OgreMain/include)
include_directories(${Boost_INCLUDE_DIRS})

# setup target
add_library(OgreProperty ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreProperty PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(OgreProperty OgreMain)
if (OGRE_CONFIG_THREADS)
  target_link_libraries(OgreProperty ${Boost_LIBRARIES})
endif ()

# install 
ogre_config_lib(OgreProperty)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Property
)

