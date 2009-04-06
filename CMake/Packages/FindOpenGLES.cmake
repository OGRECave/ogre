# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGLES
#  OPENGLES_INCLUDE_DIR  - the GL include directory
#  OPENGLES_LIBRARIES    - Link these to use OpenGLES

IF (WIN32)
  IF (CYGWIN)

    FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h )

    FIND_LIBRARY(OPENGLES_gl_LIBRARY libgles_cm )

  ELSE (CYGWIN)

    IF(BORLAND)
      SET (OPENGLES_gl_LIBRARY import32 CACHE STRING "OpenGL library for win32")
    ELSE(BORLAND)
      SET (OPENGLES_gl_LIBRARY libgles_cm CACHE STRING "OpenGL library for win32")
    ENDIF(BORLAND)

  ENDIF (CYGWIN)

ELSE (WIN32)

  IF (APPLE)

    FIND_LIBRARY(OPENGLES_gl_LIBRARY GLES_CM DOC "OpenGL ES lib for OSX")
    FIND_PATH(OPENGL_INCLUDE_DIR GLES/gl.h DOC "Include for OpenGL ES on OSX")

  ELSE(APPLE)



    FIND_PATH(OPENGLES_INCLUDE_DIR GLES/gl.h
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include /usr/X11R6/include
      /usr/include
    )


    FIND_LIBRARY(OPENGLES_gl_LIBRARY
      NAMES GLES_CM
      PATHS /opt/graphics/OpenGL/lib
            /usr/openwin/lib
            /usr/shlib /usr/X11R6/lib
            /usr/lib
    )

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't 
    # think this is always true.
    # It's not true on OSX.

    IF (OPENGLES_gl_LIBRARY)
      IF(NOT X11_FOUND)
        INCLUDE(FindX11)
      ENDIF(NOT X11_FOUND)
      IF (X11_FOUND)
        IF (NOT APPLE)
          SET (OPENGLES_LIBRARIES ${X11_LIBRARIES})
        ENDIF (NOT APPLE)
      ENDIF (X11_FOUND)
    ENDIF (OPENGLES_gl_LIBRARY)

  ENDIF(APPLE)
ENDIF (WIN32)

SET( OPENGLES_FOUND "NO" )
IF(OPENGLES_gl_LIBRARY)


    SET( OPENGLES_LIBRARIES  ${OPENGLES_gl_LIBRARY} ${OPENGLES_LIBRARIES})

    SET( OPENGLES_FOUND "YES" )

    # This deprecated setting is for backward compatibility with CMake1.4

    SET (OPENGLES_LIBRARY ${OPENGLES_LIBRARIES})

ENDIF(OPENGLES_gl_LIBRARY)

# This deprecated setting is for backward compatibility with CMake1.4
SET(OPENGLES_INCLUDE_PATH ${OPENGLES_INCLUDE_DIR})

MARK_AS_ADVANCED(
  OPENGLES_INCLUDE_DIR
  OPENGLES_gl_LIBRARY
)
