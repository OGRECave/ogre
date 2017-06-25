#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Metal RenderSystem build

if (OGRE_BUILD_PLATFORM_APPLE_IOS)
set(OS "iOS")
else()
set(OS "OSX")
endif()

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/Vao/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/Windowing/${OS}/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm" "${CMAKE_CURRENT_SOURCE_DIR}/src/Vao/*.mm" "${CMAKE_CURRENT_SOURCE_DIR}/src/Windowing/${OS}/*.mm")

include_directories(
  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/Windowing/${OS}
)

ogre_add_library(RenderSystem_Metal ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
if (OGRE_BUILD_PLATFORM_APPLE_IOS)
target_link_libraries(RenderSystem_Metal OgreMain "-framework Metal" "-framework QuartzCore")
else()
target_link_libraries(RenderSystem_Metal OgreMain "-framework Metal" "-framework AppKit" "-framework QuartzCore")
endif()

target_compile_options(RenderSystem_Metal PRIVATE "-fobjc-arc")
set_target_properties(RenderSystem_Metal PROPERTIES LINK_FLAGS "-fobjc-arc")

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_Metal ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_framework(RenderSystem_Metal)

ogre_config_plugin(RenderSystem_Metal)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Metal)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/Metal/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/OGRE/RenderSystems/Metal)
