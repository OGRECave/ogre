#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Metal RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/Vao/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/Windowing/iOS/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm" "${CMAKE_CURRENT_SOURCE_DIR}/src/Vao/*.mm" "${CMAKE_CURRENT_SOURCE_DIR}/src/Windowing/iOS/*.mm")

include_directories(
  BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

ogre_add_library(RenderSystem_Metal ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Metal OgreMain)

if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_Metal ${OGRE_THREAD_LIBRARIES})
endif ()

ogre_config_framework(RenderSystem_Metal)

ogre_config_plugin(RenderSystem_Metal)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Metal)
install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/Metal/${PLATFORM_HEADER_INSTALL})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/OGRE/RenderSystems/Metal)

set_target_properties( RenderSystem_Metal PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES )

if( NOT APPLE )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/" )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS" )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include" )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include/c++/4.2.1" )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include/objc" )
    include_directories( BEFORE SYSTEM "/home/matias/OSX_Shared/Platforms/iOS/include++" )
    add_definitions( -D__arm64__ -DTARGET_CPU_ARM64 -D__APPLE_CPP__ -D__APPLE__ -D__IPHONE_OS_VERSION_MIN_REQUIRED=90000 "-include /home/matias/OSX_Shared/Platforms/__OSX_Fixes.h" -fblocks -fobjc-abi-version=3 -fobjc-arc )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif()
