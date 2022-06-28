#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Vulkan RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/volk.c")

set( HEADER_FILES ${HEADER_FILES})
set( SOURCE_FILES ${SOURCE_FILES})

add_library(RenderSystem_Vulkan ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})

if(WIN32)
    target_include_directories(RenderSystem_Vulkan PRIVATE $ENV{VULKAN_SDK}/Include)
    target_link_directories(RenderSystem_Vulkan PRIVATE $ENV{VULKAN_SDK}/Lib)

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/volk.c"
        PROPERTIES COMPILE_DEFINITIONS VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/volk.c"
        PROPERTIES COMPILE_DEFINITIONS VK_USE_PLATFORM_ANDROID_KHR)
else()
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/volk.c"
        PROPERTIES COMPILE_DEFINITIONS VK_USE_PLATFORM_XLIB_KHR)
    target_link_libraries(RenderSystem_Vulkan PRIVATE ${X11_LIBRARIES})
endif()

target_link_libraries(RenderSystem_Vulkan PUBLIC OgreMain PRIVATE ${CMAKE_DL_LIBS})

target_include_directories(RenderSystem_Vulkan PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    $<INSTALL_INTERFACE:include/OGRE/RenderSystems/Vulkan>)

ogre_config_framework(RenderSystem_Vulkan)

ogre_config_plugin(RenderSystem_Vulkan)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Vulkan)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/Vulkan/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/OGRE/RenderSystems/Vulkan)
