#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Direct3D11 RenderSystem build

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${DirectX11_INCLUDE_DIR} ${AMDQBS_INCLUDE_DIR} ${NVAPI_INCLUDE_DIR})
  link_directories(${DirectX11_LIBRARY_DIR} ${NVAPI_LIBRARY_DIR})
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${DirectX11_INCLUDE_DIR})
  link_directories(${DirectX11_LIBRARY_DIR})
endif()

add_definitions(-D_USRDLL)

add_library(RenderSystem_Direct3D11 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Direct3D11
  OgreMain
  ${DirectX11_LIBRARIES}
)

if(OGRE_PROFILING AND DirectX9_FOUND)
  target_link_libraries(RenderSystem_Direct3D11 ${DirectX9_LIBRARY})
endif()
if (MINGW)
  target_link_libraries(RenderSystem_Direct3D11 psapi d3dcompiler)
endif ()

if(WINDOWS_STORE OR WINDOWS_PHONE)
  set_target_properties(RenderSystem_Direct3D11 PROPERTIES VS_WINRT_COMPONENT "true")
endif()

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_Direct3D11 PROPERTIES
    COMPILE_DEFINITIONS OGRED3DENGINEDLL_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_Direct3D11 ${OGRE_THREAD_LIBRARIES})
endif ()
if(OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO)
  target_link_libraries(RenderSystem_Direct3D11 ${NVAPI_LIBRARIES})
endif()

ogre_config_plugin(RenderSystem_Direct3D11)
install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Direct3D11)
