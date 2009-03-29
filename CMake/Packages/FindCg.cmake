#
# Try to find nVidia's Cg compiler, runtime libraries, and include path.
# Once done this will define
#
# Cg_FOUND        - system has NVidia Cg and it can be used. 
# Cg_INCLUDE_DIRS = directory where cg.h resides
# Cg_LIBRARIES = full path to libCg.so (Cg.DLL on win32)
# Cg_GL_LIBRARIES = full path to libCgGL.so (CgGL.dll on win32)
# Cg_COMPILER = full path to cgc (cgc.exe on win32)
# 

# On OSX default to using the framework version of Cg.
include(OgreFindFrameworks)

IF (APPLE)
  SET(Cg_FRAMEWORK_INCLUDES)
  OGRE_FIND_FRAMEWORKS(Cg)
  IF (Cg_FRAMEWORKS)
    FOREACH(dir ${Cg_FRAMEWORKS})
      SET(Cg_FRAMEWORK_INCLUDES ${Cg_FRAMEWORK_INCLUDES}
        ${dir}/Headers ${dir}/PrivateHeaders)
    ENDFOREACH(dir)

    #Find the include  dir
    FIND_PATH(Cg_INCLUDE_DIRS cg.h
      ${Cg_FRAMEWORK_INCLUDES}
      )

    #Since we are using Cg framework, we must link to it.
    SET(Cg_LIBRARIES "-framework Cg" CACHE STRING "Cg library")
    SET(Cg_GL_LIBRARIES "-framework Cg" CACHE STRING "Cg GL library")
  ENDIF (Cg_FRAMEWORKS)
  FIND_PROGRAM(Cg_COMPILER cgc
    /usr/bin
    /usr/local/bin
    DOC "The Cg compiler"
    )
ELSE (APPLE)
  IF (WIN32)
    FIND_PROGRAM( Cg_COMPILER cgc
      "C:/Program Files/NVIDIA Corporation/Cg/bin"
      "C:/Program Files/Cg"
      ${PROJECT_SOURCE_DIR}/../Cg
      DOC "The Cg Compiler"
      )
    IF (Cg_COMPILER)
      GET_FILENAME_COMPONENT(Cg_COMPILER_DIR ${Cg_COMPILER} PATH)
      GET_FILENAME_COMPONENT(Cg_COMPILER_SUPER_DIR ${Cg_COMPILER_DIR} PATH)
    ELSE (Cg_COMPILER)
      SET (Cg_COMPILER_DIR .)
      SET (Cg_COMPILER_SUPER_DIR ..)
    ENDIF (Cg_COMPILER)
    FIND_PATH( Cg_INCLUDE_DIRS Cg/cg.h
      "C:/Program Files/NVIDIA Corporation/Cg/include"
      "C:/Program Files/Cg"
      ${PROJECT_SOURCE_DIR}/../Cg
      ${Cg_COMPILER_SUPER_DIR}/include
      ${Cg_COMPILER_DIR}
      DOC "The directory where Cg/cg.h resides"
      )
    FIND_LIBRARY( Cg_LIBRARIES
      NAMES Cg
      PATHS
      "C:/Program Files/NVIDIA Corporation/Cg/lib"
      "C:/Program Files/Cg"
      ${PROJECT_SOURCE_DIR}/../Cg
      ${Cg_COMPILER_SUPER_DIR}/lib
      ${Cg_COMPILER_DIR}
      DOC "The Cg runtime library"
      )
    FIND_LIBRARY( Cg_GL_LIBRARIES
      NAMES CgGL
      PATHS
      "C:/Program Files/NVIDIA Corporation/Cg/lib"
      "C:/Program Files/Cg"
      ${PROJECT_SOURCE_DIR}/../Cg
      ${Cg_COMPILER_SUPER_DIR}/lib
      ${Cg_COMPILER_DIR}
      DOC "The Cg runtime library"
      )
  ELSE (WIN32)
    FIND_PROGRAM( Cg_COMPILER cgc
      /usr/bin
      /usr/local/bin
      DOC "The Cg Compiler"
      )
    GET_FILENAME_COMPONENT(Cg_COMPILER_DIR "${Cg_COMPILER}" PATH)
    GET_FILENAME_COMPONENT(Cg_COMPILER_SUPER_DIR "${Cg_COMPILER_DIR}" PATH)
    FIND_PATH( Cg_INCLUDE_DIRS Cg/cg.h
      /usr/include
      /usr/local/include
      ${Cg_COMPILER_SUPER_DIR}/include
      DOC "The directory where Cg/cg.h resides"
      )
    FIND_LIBRARY( Cg_LIBRARIES Cg
      PATHS
      /usr/lib64
      /usr/lib
      /usr/local/lib64
      /usr/local/lib
      ${Cg_COMPILER_SUPER_DIR}/lib64
      ${Cg_COMPILER_SUPER_DIR}/lib
      DOC "The Cg runtime library"
      )
    FIND_LIBRARY( Cg_GL_LIBRARIES CgGL
      PATHS
      /usr/lib64
      /usr/lib
      /usr/local/lib64
      /usr/local/lib
      ${Cg_COMPILER_SUPER_DIR}/lib64
      ${Cg_COMPILER_SUPER_DIR}/lib
      DOC "The Cg runtime library"
      )
  ENDIF (WIN32)
ENDIF (APPLE)

IF (Cg_INCLUDE_DIRS AND Cg_LIBRARIES)
  SET( Cg_FOUND TRUE)
ENDIF ()

MARK_AS_ADVANCED( Cg_INCLUDE_DIRS Cg_COMPILER Cg_LIBRARIES Cg_GL_LIBRARIES )
