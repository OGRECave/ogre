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

# define header and source files for the library
set (HEADER_FILES
	include/OgreProperty.h
	${PROJECT_BINARY_DIR}/include/OgrePropertyPrerequisites.h
)

set (SOURCE_FILES
	src/OgreProperty.cpp
)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    # workaround Wundef with boost
    if(UNIX)
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/OgreProperty.cpp" PROPERTIES COMPILE_FLAGS "-Wno-undef")
    endif()
endif()

# setup target
add_library(OgreProperty ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADER_FILES} ${PLATFORM_SOURCE_FILES})
set_target_properties(OgreProperty PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreProperty PUBLIC OgreMain)
target_include_directories(OgreProperty PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Property>)

generate_export_header(OgreProperty 
    EXPORT_MACRO_NAME _OgrePropertyExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgrePropertyPrerequisites.h)

# install 
ogre_config_framework(OgreProperty)
ogre_config_component(OgreProperty)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Property
)
