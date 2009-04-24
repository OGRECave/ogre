# Configure OpenGL RenderSystem build

set(HEADER_FILES
  include/OgreGLATIFSInit.h
  include/OgreGLContext.h
  include/OgreGLDefaultHardwareBufferManager.h
  include/OgreGLFBOMultiRenderTarget.h
  include/OgreGLFBORenderTexture.h
  include/OgreGLFrameBufferObject.h
  include/OgreGLGpuNvparseProgram.h
  include/OgreGLGpuProgram.h
  include/OgreGLGpuProgramManager.h
  include/OgreGLHardwareBufferManager.h
  include/OgreGLHardwareIndexBuffer.h
  include/OgreGLHardwareOcclusionQuery.h
  include/OgreGLHardwarePixelBuffer.h
  include/OgreGLHardwareVertexBuffer.h
  include/OgreGLPBRenderTexture.h
  include/OgreGLPBuffer.h
  include/OgreGLPixelFormat.h
  include/OgreGLPlugin.h
  include/OgreGLPrerequisites.h
  include/OgreGLRenderSystem.h
  include/OgreGLRenderTexture.h
  include/OgreGLRenderToVertexBuffer.h
  include/OgreGLSupport.h
  include/OgreGLTexture.h
  include/OgreGLTextureManager.h
)


set(SOURCE_FILES
  src/glew.cpp
  src/OgreGLATIFSInit.cpp
  src/OgreGLContext.cpp
  src/OgreGLDefaultHardwareBufferManager.cpp
  src/OgreGLEngineDll.cpp
  src/OgreGLFBOMultiRenderTarget.cpp
  src/OgreGLFBORenderTexture.cpp
  src/OgreGLFrameBufferObject.cpp
  src/OgreGLGpuNvparseProgram.cpp
  src/OgreGLGpuProgram.cpp
  src/OgreGLGpuProgramManager.cpp
  src/OgreGLHardwareBufferManager.cpp
  src/OgreGLHardwareIndexBuffer.cpp
  src/OgreGLHardwareOcclusionQuery.cpp
  src/OgreGLHardwarePixelBuffer.cpp
  src/OgreGLHardwareVertexBuffer.cpp
  src/OgreGLPBRenderTexture.cpp
  src/OgreGLPixelFormat.cpp
  src/OgreGLPlugin.cpp
  src/OgreGLRenderSystem.cpp
  src/OgreGLRenderTexture.cpp
  src/OgreGLRenderToVertexBuffer.cpp
  src/OgreGLSupport.cpp
  src/OgreGLTexture.cpp
  src/OgreGLTextureManager.cpp
)

set(NVPARSE_FILES
  src/nvparse/macro.h
  src/nvparse/nvparse_errors.h
  src/nvparse/nvparse_externs.h
  src/nvparse/nvparse.h
  src/nvparse/_ps1.0_parser.h
  src/nvparse/ps1.0_program.h
  src/nvparse/rc1.0_combiners.h
  src/nvparse/rc1.0_final.h
  src/nvparse/rc1.0_general.h
  src/nvparse/_rc1.0_parser.h
  src/nvparse/rc1.0_register.h
  src/nvparse/ts1.0_inst.h
  src/nvparse/ts1.0_inst_list.h
  src/nvparse/_ts1.0_parser.h
  src/nvparse/vs1.0_inst.h
  src/nvparse/vs1.0_inst_list.h
  src/nvparse/_vs1.0_parser.h
  src/nvparse/avp1.0_impl.cpp
  src/nvparse/nvparse.cpp
  src/nvparse/nvparse_errors.cpp
  src/nvparse/_ps1.0_lexer.cpp
  src/nvparse/_ps1.0_parser.cpp
  src/nvparse/ps1.0_program.cpp
  src/nvparse/rc1.0_combiners.cpp
  src/nvparse/rc1.0_final.cpp
  src/nvparse/rc1.0_general.cpp
  src/nvparse/_rc1.0_lexer.cpp
  src/nvparse/_rc1.0_parser.cpp
  src/nvparse/ts1.0_inst.cpp
  src/nvparse/ts1.0_inst_list.cpp
  src/nvparse/_ts1.0_lexer.cpp
  src/nvparse/_ts1.0_parser.cpp
  src/nvparse/vcp1.0_impl.cpp
  src/nvparse/vp1.0_impl.cpp
  src/nvparse/vs1.0_inst.cpp
  src/nvparse/vs1.0_inst_list.cpp
  src/nvparse/_vs1.0_lexer.cpp
  src/nvparse/_vs1.0_parser.cpp
  src/nvparse/vsp1.0_impl.cpp
)

set(GLSL_FILES
  src/GLSL/include/OgreGLSLExtSupport.h
  src/GLSL/include/OgreGLSLGpuProgram.h
  src/GLSL/include/OgreGLSLLinkProgram.h
  src/GLSL/include/OgreGLSLLinkProgramManager.h
  src/GLSL/include/OgreGLSLPreprocessor.h
  src/GLSL/include/OgreGLSLProgramFactory.h
  src/GLSL/include/OgreGLSLProgram.h
  src/GLSL/src/OgreGLSLExtSupport.cpp
  src/GLSL/src/OgreGLSLGpuProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgram.cpp
  src/GLSL/src/OgreGLSLLinkProgramManager.cpp
  src/GLSL/src/OgreGLSLPreprocessor.cpp
  src/GLSL/src/OgreGLSLProgram.cpp
  src/GLSL/src/OgreGLSLProgramFactory.cpp
)

set(ATIFS_FILES
  src/atifs/include/ATI_FS_GLGpuProgram.h
  src/atifs/include/Compiler2Pass.h
  src/atifs/include/ps_1_4.h
  src/atifs/src/ATI_FS_GLGpuProgram.cpp
  src/atifs/src/Compiler2Pass.cpp
  src/atifs/src/ps_1_4.cpp
)

# Add system specific settings
if (WIN32)
  set(PLATFORM_HEADERS 
    include/OgreWin32Context.h
    include/OgreWin32GLSupport.h
    include/OgreWin32Prerequisites.h
    include/OgreWin32RenderTexture.h
    include/OgreWin32Window.h
    src/win32/OgreGLUtil.h
  )
  set(PLATFORM_SOURCES
    src/OgreWin32GLSupport.cpp
    src/OgreWin32RenderTexture.cpp
    src/OgreWin32Window.cpp
    src/win32/OgreWin32Context.cpp
  )
  include_directories(src/nvparse/winheaders src/win32)
  source_group(NVparse FILES ${NVPARSE_FILES})
  source_group(ATIFS FILES ${ATIFS_FILES})
  source_group(GLSL FILES ${GLSL_FILES})
  set(PLATFORM_HEADER_INSTALL "")
elseif (APPLE)
  set(PLATFORM_HEADERS
    include/OSX/OgreOSXCGLContext.h
    include/OSX/OgreOSXCarbonContext.h
    include/OSX/OgreOSXCarbonWindow.h
    include/OSX/OgreOSXCocoaContext.h
    include/OSX/OgreOSXCocoaWindow.h
    include/OSX/OgreOSXContext.h
    include/OSX/OgreOSXGLSupport.h
    include/OSX/OgreOSXRenderTexture.h
    include/OSX/OgreOSXWindow.h
    src/OSX/OgreGLUtil.h
  
    # think about relocating these
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/include/macBundlePath.h
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/include/macPlugins.h
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/include/OgreOSXCocoaView.h
  )
  set(PLATFORM_SOURCES
    src/OSX/OgreOSXCarbonContext.cpp
    src/OSX/OgreOSXCocoaContext.mm
    src/OSX/OgreOSXContext.cpp
    src/OSX/OgreOSXRenderTexture.cpp
    src/OSX/OgreOSXCGLContext.cpp
    src/OSX/OgreOSXCarbonWindow.cpp
    src/OSX/OgreOSXCocoaWindow.mm
    src/OSX/OgreOSXGLSupport.mm
    src/OSX/OgreOSXWindow.cpp
    # think about relocating these
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/src/macBundlePath.cpp
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/src/macPlugins.cpp
    #${CMAKE_SOURCE_DIR}/Mac/Ogre/src/OgreOSXCocoaView.mm
  )
  include_directories(src/OSX include/OSX)
  include_directories("${CMAKE_SOURCE_DIR}/Mac/Ogre/include")
  set(PLATFORM_LIBS ${Carbon_LIBRARY_FWK} ${Cocoa_LIBRARY_FWK})
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  set(PLATFORM_HEADERS
    include/GLX/OgreGLXContext.h
    include/GLX/OgreGLXGLSupport.h
    include/GLX/OgreGLXRenderTexture.h
    include/GLX/OgreGLXUtils.h
    include/GLX/OgreGLXWindow.h
    src/GLX/OgreGLUtil.h
  )
  set(PLATFORM_SOURCES
    src/GLX/OgreGLXContext.cpp
    src/GLX/OgreGLXGLSupport.cpp
    src/GLX/OgreGLXRenderTexture.cpp
    src/GLX/OgreGLXWindow.cpp
  )
  include_directories(src/GLX include/GLX)
  set(PLATFORM_HEADER_INSTALL "GLX")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/atifs/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/nvparse
  ${OPENGL_INCLUDE_DIR}
)

add_library(RenderSystem_GL ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} ${GLSL_FILES} ${ATIFS_FILES} ${NVPARSE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(RenderSystem_GL OgreMain ${OPENGL_LIBRARIES} ${PLATFORM_LIBS})

if (NOT OGRE_STATIC)
  set_target_properties(RenderSystem_GL PROPERTIES
    COMPILE_DEFINITIONS OGRE_GLPLUGIN_EXPORTS
  )
endif ()
if (OGRE_CONFIG_THREADS)
  target_link_libraries(RenderSystem_GL ${Boost_LIBRARIES})
endif ()

ogre_config_plugin(RenderSystem_GL)
if (OGRE_INSTALL_PLUGINS_HEADERS)
  install(FILES ${HEADER_FILES} DESTINATION include/OGRE/RenderSystems/GL)
  install(FILES ${PLATFORM_HEADERS} DESTINATION include/OGRE/RenderSystems/GL/${PLATFORM_HEADER_INSTALL})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/GL DESTINATION include/OGRE/RenderSystems/GL)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/GLSL/include/ DESTINATION include/OGRE/RenderSystems/GL)
endif ()
