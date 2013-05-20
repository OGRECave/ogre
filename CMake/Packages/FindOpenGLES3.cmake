#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find OpenGLES and EGL
# Once done this will define
#  
#  OPENGLES3_FOUND        - system has OpenGLES
#  OPENGLES3_INCLUDE_DIR  - the GL include directory
#  OPENGLES3_LIBRARIES    - Link these to use OpenGLES
#
#  EGL_FOUND        - system has EGL
#  EGL_INCLUDE_DIR  - the EGL include directory
#  EGL_LIBRARIES    - Link these to use EGL

IF (WIN32)
  IF (CYGWIN)

    FIND_PATH(OPENGLES3_INCLUDE_DIR GLES3/gl3.h )

    FIND_LIBRARY(OPENGLES3_gl_LIBRARY libGLESv2 )

  ELSE (CYGWIN)

    IF(BORLAND)
      SET (OPENGLES3_gl_LIBRARY import32 CACHE STRING "OpenGL ES 3.x library for win32")
    ELSE(BORLAND)
		SET(POWERVR_SDK_PATH "C:/Imagination/PowerVR/GraphicsSDK/SDK_3.1/Builds/")
		FIND_PATH(OPENGLES3_INCLUDE_DIR GLES3/gl3.h
			${POWERVR_SDK_PATH}/Include
		)
		
	    FIND_PATH(EGL_INCLUDE_DIR EGL/egl.h
			${POWERVR_SDK_PATH}/Include
		)
		
		FIND_LIBRARY(OPENGLES3_gl_LIBRARY
			NAMES libGLESv2
			PATHS ${POWERVR_SDK_PATH}/Windows/x86_32/Lib
		)
		    
		FIND_LIBRARY(EGL_egl_LIBRARY
			NAMES libEGL
			PATHS ${POWERVR_SDK_PATH}/Windows/x86_32/Lib
		)
    ENDIF(BORLAND)

  ENDIF (CYGWIN)

ELSE (WIN32)

  IF (APPLE)

	create_search_paths(/Developer/Platforms)
	findpkg_framework(OpenGLES3)
    set(OPENGLES3_gl_LIBRARY "-framework OpenGLES")

  ELSE(APPLE)

    FIND_PATH(OPENGLES2_INCLUDE_DIR GLES3/gl3.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /usr/include
    )

    FIND_LIBRARY(OPENGLES3_gl_LIBRARY
      NAMES GLESv2
      PATHS /opt/graphics/OpenGL/lib
            /usr/openwin/lib
            /usr/shlib /usr/X11R6/lib
            /usr/lib
    )

    FIND_PATH(EGL_INCLUDE_DIR EGL/egl.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /usr/include
    )

    FIND_LIBRARY(EGL_egl_LIBRARY
      NAMES EGL
      PATHS /opt/graphics/OpenGL/lib
            /usr/openwin/lib
            /usr/shlib /usr/X11R6/lib
            /usr/lib
    )

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't 
    # think this is always true.
    # It's not true on OSX.

    IF (OPENGLES3_gl_LIBRARY)
      IF(NOT X11_FOUND)
        INCLUDE(FindX11)
      ENDIF(NOT X11_FOUND)
      IF (X11_FOUND)
        IF (NOT APPLE)
          SET (OPENGLES3_LIBRARIES ${X11_LIBRARIES})
        ENDIF (NOT APPLE)
      ENDIF (X11_FOUND)
    ENDIF (OPENGLES3_gl_LIBRARY)

  ENDIF(APPLE)
ENDIF (WIN32)

SET( OPENGLES3_FOUND "YES" )
IF(OPENGLES3_gl_LIBRARY AND EGL_egl_LIBRARY)

    SET( OPENGLES3_LIBRARIES ${OPENGLES3_gl_LIBRARY} ${OPENGLES3_LIBRARIES})
    SET( EGL_LIBRARIES ${EGL_egl_LIBRARY} ${EGL_LIBRARIES})
    SET( OPENGLES3_FOUND "YES" )

ENDIF(OPENGLES3_gl_LIBRARY AND EGL_egl_LIBRARY)

MARK_AS_ADVANCED(
  OPENGLES3_INCLUDE_DIR
  OPENGLES3_gl_LIBRARY
  EGL_INCLUDE_DIR
  EGL_egl_LIBRARY
)
