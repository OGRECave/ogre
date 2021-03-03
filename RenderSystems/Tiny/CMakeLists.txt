#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${PROJECT_BINARY_DIR}/include/OgreTinyExports.h)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_definitions(${OGRE_VISIBILITY_FLAGS})

add_library(RenderSystem_Tiny ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Tiny PUBLIC OgreMain PRIVATE)
target_include_directories(RenderSystem_Tiny PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    $<INSTALL_INTERFACE:include/OGRE/RenderSystems/Tiny>)

find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    target_link_libraries(RenderSystem_Tiny PUBLIC OpenMP::OpenMP_CXX)
endif()

if(SDL2_FOUND)
    target_link_libraries(RenderSystem_Tiny PRIVATE SDL2::SDL2)
endif()

generate_export_header(RenderSystem_Tiny
    EXPORT_MACRO_NAME _OgreTinyExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreTinyExports.h)

ogre_config_framework(RenderSystem_Tiny)
ogre_config_plugin(RenderSystem_Tiny)
