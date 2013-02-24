#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure RenderSystem build

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
  if (NOT DirectX9_FOUND)
    MESSAGE(STATUS "Could not find dependency: DirectX")
  	MESSAGE(STATUS "Skipping Direct3D9 rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_D3D9 0)
  else ()
    add_subdirectory(Direct3D9)
  endif ()
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D11)
  if (NOT DirectX11_FOUND)
    MESSAGE(STATUS "Could not find dependency: DirectX")
  	MESSAGE(STATUS "Skipping Direct3D11 rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_D3D11 0)
  else ()
    add_subdirectory(Direct3D11)
  endif ()
endif ()

if (OGRE_BUILD_RENDERSYSTEM_GL)
  if (NOT OPENGL_FOUND)
    MESSAGE(STATUS "Could not find dependency: GL")
	  MESSAGE(STATUS "Skipping GL rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_GL 0)
  else ()
    add_subdirectory(GL)
  endif ()
endif ()

if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
  if (NOT OPENGL_FOUND)
    MESSAGE(STATUS "Could not find dependency: GL3Plus")
	  MESSAGE(STATUS "Skipping GL3Plus rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_GL3PLUS 0)
  else ()
    add_subdirectory(GL3Plus)
  endif ()
endif ()

if (OGRE_BUILD_RENDERSYSTEM_GLES)
  if (NOT OPENGLES_FOUND)
    MESSAGE(STATUS "Could not find dependency: GLES ")
	  MESSAGE(STATUS "Skipping GLES rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_GLES 0)
  else ()
    add_subdirectory(GLES)
  endif()
endif()

if (OGRE_BUILD_RENDERSYSTEM_GLES2)
  if (NOT OPENGLES2_FOUND)
    MESSAGE(STATUS "Could not find dependency: GLES2 ")
	  MESSAGE(STATUS "Skipping GLES2 rendersystem build")
    set(OGRE_BUILD_RENDERSYSTEM_GLES2 0)
  else ()
    add_subdirectory(GLES2)
  endif()
endif()


if (OGRE_BUILD_RENDERSYSTEM_STAGE3D AND FLASHCC)
    add_subdirectory(Stage3D)
endif()
