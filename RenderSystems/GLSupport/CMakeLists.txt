#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL Support build

# Add system specific settings
if (WIN32)
  file(GLOB PLATFORM_HEADERS "include/win32/*.h")
  file(GLOB PLATFORM_SOURCES "src/win32/*.cpp")

  set(OGRE_GLSUPPORT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/win32)
  set(PLATFORM_HEADER_INSTALL "win32")
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.mm")

  set(OGRE_GLSUPPORT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OSX 
    "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_LIBS ${Cocoa_LIBRARY_FWK})
  set(PLATFORM_HEADER_INSTALL "OSX")
elseif (UNIX)
  file(GLOB PLATFORM_HEADERS "include/GLX/*.h")
  file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp")

  set(OGRE_GLSUPPORT_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLX)
  set(PLATFORM_HEADER_INSTALL "GLX")
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
endif ()

set(PLATFORM_HEADERS ${PLATFORM_HEADERS} PARENT_SCOPE)
set(PLATFORM_HEADER_INSTALL ${PLATFORM_HEADER_INSTALL} PARENT_SCOPE)

set(OGRE_GLSUPPORT_INCLUDES 
  ${OGRE_GLSUPPORT_INCLUDES} 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

if(NOT ANDROID AND NOT APPLE_IOS)
include_directories(${OGRE_GLSUPPORT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/include/)
ogre_add_library(GLSupport ${OGRE_LIB_TYPE} ${HEADER_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(GLSupport OgreMain ${OPENGL_LIBRARIES} ${PLATFORM_LIBS})
#ogre_config_framework(GLSupport)
ogre_config_common(GLSupport)
if (APPLE)
    set_target_properties(GLSupport PROPERTIES LINK_FLAGS "-framework Cocoa -framework OpenGL")
endif()

set(PLATFORM_LIBS GLSupport ${PLATFORM_LIBS} PARENT_SCOPE) 
endif()