#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Ogre RTShader System build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES 
    ${PROJECT_BINARY_DIR}/include/OgreRTShaderExports.h 
    ${PROJECT_BINARY_DIR}/include/OgreRTShaderConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OgreRTShader.i)
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

get_native_precompiled_header(OgreRTShaderSystem)
add_library(OgreRTShaderSystem ${OGRE_COMP_LIB_TYPE} ${OgreRTShaderSystem_pch} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreRTShaderSystem PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreRTShaderSystem PUBLIC OgreMain)
target_include_directories(OgreRTShaderSystem PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/RTShaderSystem>)

# repeat for PCH
include_directories("${PROJECT_SOURCE_DIR}/OgreMain/include" "${PROJECT_BINARY_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_native_precompiled_header(OgreRTShaderSystem "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreShaderPrecompiledHeaders.h")

generate_export_header(OgreRTShaderSystem 
    EXPORT_MACRO_NAME _OgreRTSSExport
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreRTShaderExports.h)

ogre_config_framework(OgreRTShaderSystem)
ogre_config_component(OgreRTShaderSystem)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/RTShaderSystem
)