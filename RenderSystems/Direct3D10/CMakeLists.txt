# Configure Direct3D10 RenderSystem build

set (HEADER_FILES
  include/OgreD3D10Device.h
  include/OgreD3D10Driver.h
  include/OgreD3D10DriverList.h
  include/OgreD3D10GpuProgram.h
  include/OgreD3D10GpuProgramManager.h
  include/OgreD3D10HardwareBuffer.h
  include/OgreD3D10HardwareBufferManager.h
  include/OgreD3D10HardwareIndexBuffer.h
  include/OgreD3D10HardwareOcclusionQuery.h
  include/OgreD3D10HardwarePixelBuffer.h
  include/OgreD3D10HardwareVertexBuffer.h
  include/OgreD3D10HLSLProgram.h
  include/OgreD3D10HLSLProgramFactory.h
  include/OgreD3D10Mappings.h
  include/OgreD3D10MultiRenderTarget.h
  include/OgreD3D10Plugin.h
  include/OgreD3D10Prerequisites.h
  include/OgreD3D10RenderSystem.h
  include/OgreD3D10RenderToVertexBuffer.h
  include/OgreD3D10RenderWindow.h
  include/OgreD3D10Texture.h
  include/OgreD3D10TextureManager.h
  include/OgreD3D10VertexDeclaration.h
  include/OgreD3D10VideoMode.h
  include/OgreD3D10VideoModeList.h
  include/OgreFixedFuncEmuShaderGenerator.h
  include/OgreFixedFuncEmuShaderManager.h
  include/OgreFixedFuncState.h
  include/OgreHlslFixedFuncEmuShaderGenerator.h
)

set (SOURCE_FILES
  src/OgreD3D10Device.cpp
  src/OgreD3D10Driver.cpp
  src/OgreD3D10DriverList.cpp
  src/OgreD3D10EngineDll.cpp
  src/OgreD3D10GpuProgram.cpp
  src/OgreD3D10GpuProgramManager.cpp
  src/OgreD3D10HardwareBuffer.cpp
  src/OgreD3D10HardwareBufferManager.cpp
  src/OgreD3D10HardwareIndexBuffer.cpp
  src/OgreD3D10HardwareOcclusionQuery.cpp
  src/OgreD3D10HardwarePixelBuffer.cpp
  src/OgreD3D10HardwareVertexBuffer.cpp
  src/OgreD3D10HLSLProgram.cpp
  src/OgreD3D10HLSLProgramFactory.cpp
  src/OgreD3D10Mappings.cpp
  src/OgreD3D10MultiRenderTarget.cpp
  src/OgreD3D10Plugin.cpp
  src/OgreD3D10RenderSystem.cpp
  src/OgreD3D10RenderToVertexBuffer.cpp
  src/OgreD3D10RenderWindow.cpp
  src/OgreD3D10Texture.cpp
  src/OgreD3D10TextureManager.cpp
  src/OgreD3D10VertexDeclaration.cpp
  src/OgreD3D10VideoMode.cpp
  src/OgreD3D10VideoModeList.cpp
  src/OgreFixedFuncEmuShaderGenerator.cpp
  src/OgreFixedFuncEmuShaderManager.cpp
  src/OgreFixedFuncState.cpp
  src/OgreHlslFixedFuncEmuShaderGenerator.cpp

)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${DirectX_INCLUDE_DIR})
link_directories(${DirectX_LIBRARY_PATHS})
add_definitions(-D_USRDLL)

add_library(RenderSystem_Direct3D10 ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(RenderSystem_Direct3D10
  OgreMain
  d3d10
  d3dx10
  dxgi
  dxerr
  dxguid
)

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_Direct3D10 PROPERTIES
    COMPILE_DEFINITIONS OGRED3DENGINEDLL_EXPORTS
  )
endif ()
set_target_properties(RenderSystem_Direct3D10 PROPERTIES PREFIX "")

ogre_install_plugin(RenderSystem_Direct3D10)
