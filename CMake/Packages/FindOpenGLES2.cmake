#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES and EGL
# If using ARM Mali emulation you can specify the parent directory that contains the bin and include directories by 
# setting the MALI_SDK_ROOT variable in the environment.
#
# For AMD emulation use the AMD_SDK_ROOT variable
#
# Once done this will define
#  
#  OPENGLES2_FOUND        - system has OpenGLES
#  OPENGLES2_INCLUDE_DIR  - the GL include directory
#  OPENGLES2_LIBRARIES    - Link these to use OpenGLES
#
#  EGL_FOUND        - system has EGL
#  EGL_INCLUDE_DIR  - the EGL include directory
#  EGL_LIBRARIES    - Link these to use EGL

IF(APPLE)
  create_search_paths(/Developer/Platforms)
  findpkg_framework(OpenGLES2)
  set(OPENGLES2_gl_LIBRARY "-framework OpenGLES")
ELSEIF (WIN32)
  # use WGL
  find_package(OpenGL)
  set(OPENGLES2_gl_LIBRARY ${OPENGL_gl_LIBRARY})
ELSE ()
  getenv_path(AMD_SDK_ROOT)
  getenv_path(MALI_SDK_ROOT)

  FIND_PATH(OPENGLES2_INCLUDE_DIR GLES2/gl2.h
    ${ENV_AMD_SDK_ROOT}/include
    ${ENV_MALI_SDK_ROOT}/include
    /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include /usr/X11R6/include
    /usr/include
  )

  FIND_LIBRARY(OPENGLES2_gl_LIBRARY
    NAMES GLESv2
    PATHS ${ENV_AMD_SDK_ROOT}/x86
          ${ENV_MALI_SDK_ROOT}/bin
          /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Linux/x86_32/Lib
          /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/shlib /usr/X11R6/lib
          /usr/lib
  )

  FIND_PATH(EGL_INCLUDE_DIR EGL/egl.h
    ${ENV_AMD_SDK_ROOT}/include
    ${ENV_MALI_SDK_ROOT}/include
    /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include /usr/X11R6/include
    /usr/include
  )

  FIND_LIBRARY(EGL_egl_LIBRARY
    NAMES EGL
    PATHS ${ENV_AMD_SDK_ROOT}/x86
          ${ENV_MALI_SDK_ROOT}/bin
          /opt/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/Linux/x86_32/Lib
          /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/shlib /usr/X11R6/lib
          /usr/lib
  )
ENDIF ()

IF(OPENGLES2_gl_LIBRARY)
    SET( OPENGLES2_LIBRARIES ${OPENGLES2_gl_LIBRARY} ${OPENGLES2_LIBRARIES})
    SET( OPENGLES2_FOUND TRUE )
ENDIF(OPENGLES2_gl_LIBRARY)

IF(EGL_egl_LIBRARY)
  SET( EGL_LIBRARIES ${EGL_egl_LIBRARY} ${EGL_LIBRARIES})
  SET( EGL_FOUND TRUE)
ENDIF()

if(EMSCRIPTEN)
  SET( OPENGLES2_FOUND TRUE )
  SET( EGL_FOUND TRUE)
endif()

MARK_AS_ADVANCED(
  OPENGLES2_INCLUDE_DIR
  OPENGLES2_gl_LIBRARY
  EGL_INCLUDE_DIR
  EGL_egl_LIBRARY
)
