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

  set(NATIVE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/win32)
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.mm")

  set(NATIVE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OSX 
    "${OGRE_SOURCE_DIR}/OgreMain/include/OSX")
  set(PLATFORM_LIBS ${Cocoa_LIBRARY_FWK})
elseif (UNIX)
  file(GLOB PLATFORM_HEADERS "include/GLX/*.h")
  file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp")

  set(NATIVE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLX)
  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
  
  if(NOT OGRE_STATIC)
      add_definitions(-fPIC)
  endif()
endif ()

file(GLOB GLSUPPORT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
set(GLSUPPORT_HEADERS ${GLSUPPORT_HEADERS} PARENT_SCOPE)
set(GLSUPPORT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

if(NOT ANDROID AND NOT APPLE_IOS)
    include_directories(${NATIVE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/include/)
    ogre_add_library(OgreGLSupport STATIC ${GLSUPPORT_HEADERS} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
    target_link_libraries(OgreGLSupport OgreMain ${PLATFORM_LIBS})
    
    if (APPLE)
        set_target_properties(OgreGLSupport PROPERTIES LINK_FLAGS "-framework Cocoa -framework OpenGL")
    endif()

    ogre_config_common(OgreGLSupport)

    set(GLSUPPORT_LIBS OgreGLSupport ${PLATFORM_LIBS} PARENT_SCOPE) 
endif()
