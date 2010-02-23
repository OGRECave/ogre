#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Direct3D11 RenderSystem build

set (HEADER_FILES
  include/OgreD3D11DepthBuffer.h
  include/OgreD3D11Device.h
  include/OgreD3D11Driver.h
  include/OgreD3D11DriverList.h
  include/OgreD3D11GpuProgram.h
  include/OgreD3D11GpuProgramManager.h
  include/OgreD3D11HardwareBuffer.h
  include/OgreD3D11HardwareBufferManager.h
  include/OgreD3D11HardwareIndexBuffer.h
  include/OgreD3D11HardwareOcclusionQuery.h
  include/OgreD3D11HardwarePixelBuffer.h
  include/OgreD3D11HardwareVertexBuffer.h
  include/OgreD3D11HLSLProgram.h
  include/OgreD3D11HLSLProgramFactory.h
  include/OgreD3D11Mappings.h
  include/OgreD3D11MultiRenderTarget.h
  include/OgreD3D11Plugin.h
  include/OgreD3D11Prerequisites.h
  include/OgreD3D11RenderSystem.h
  include/OgreD3D11RenderToVertexBuffer.h
  include/OgreD3D11RenderWindow.h
  include/OgreD3D11Texture.h
  include/OgreD3D11TextureManager.h
  include/OgreD3D11VertexDeclaration.h
  include/OgreD3D11VideoMode.h
  include/OgreD3D11VideoModeList.h
  include/OgreFixedFuncEmuShaderGenerator.h
  include/OgreFixedFuncEmuShaderManager.h
  include/OgreFixedFuncState.h
  include/OgreHlslFixedFuncEmuShaderGenerator.h
)

set (SOURCE_FILES
  src/OgreD3D11DepthBuffer.cpp
  src/OgreD3D11Device.cpp
  src/OgreD3D11Driver.cpp
  src/OgreD3D11DriverList.cpp
  src/OgreD3D11EngineDll.cpp
  src/OgreD3D11GpuProgram.cpp
  src/OgreD3D11GpuProgramManager.cpp
  src/OgreD3D11HardwareBuffer.cpp
  src/OgreD3D11HardwareBufferManager.cpp
  src/OgreD3D11HardwareIndexBuffer.cpp
  src/OgreD3D11HardwareOcclusionQuery.cpp
  src/OgreD3D11HardwarePixelBuffer.cpp
  src/OgreD3D11HardwareVertexBuffer.cpp
  src/OgreD3D11HLSLProgram.cpp
  src/OgreD3D11HLSLProgramFactory.cpp
  src/OgreD3D11Mappings.cpp
  src/OgreD3D11MultiRenderTarget.cpp
  src/OgreD3D11Plugin.cpp
  src/OgreD3D11RenderSystem.cpp
  src/OgreD3D11RenderToVertexBuffer.cpp
  src/OgreD3D11RenderWindow.cpp
  src/OgreD3D11Texture.cpp
  src/OgreD3D11TextureManager.cpp
  src/OgreD3D11VertexDeclaration.cpp
  src/OgreD3D11VideoMode.cpp
  src/OgreD3D11VideoModeList.cpp
  src/OgreFixedFuncEmuShaderGenerator.cpp
  src/OgreFixedFuncEmuShaderManager.cpp
  src/OgreFixedFuncState.cpp
  src/OgreHlslFixedFuncEmuShaderGenerator.cpp

)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${DirectX_INCLUDE_DIR})
link_directories(${DirectX_LIBRARY_DIR})
add_definitions(-D_USRDLL)

add_library(RenderSystem_Direct3D11 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Direct3D11
  OgreMain
  ${DirectX_D3D11_LIBRARIES}
)

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_Direct3D11 PROPERTIES
    COMPILE_DEFINITIONS OGRED3DENGINEDLL_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_Direct3D11 ${Boost_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_Direct3D11)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/Direct3D11)
endif ()
