#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure OpenGL Support build

# Add platform specific settings

option(OGRE_GLSUPPORT_USE_EGL "use EGL for GL Context Creation instead of GLX/ WGL" FALSE)

if(ANDROID)
  file(GLOB PLATFORM_HEADERS "include/EGL/Android/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/Android/*.cpp" "src/EGL/*.cpp")

  set(NATIVE_INCLUDES 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/Android)
elseif (EMSCRIPTEN)
  file(GLOB PLATFORM_HEADERS "include/EGL/Emscripten/*.h" "include/EGL/*.h")
  file(GLOB PLATFORM_SOURCES "src/EGL/Emscripten/*.cpp" "src/EGL/*.cpp")

  set(NATIVE_INCLUDES 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL 
      ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/Emscripten)
elseif (WIN32)
  if(OGRE_GLSUPPORT_USE_EGL)
      file(GLOB PLATFORM_HEADERS "include/EGL/WIN32/*.h" "include/EGL/*.h")
      file(GLOB PLATFORM_SOURCES "src/EGL/WIN32/*.cpp" "src/EGL/*.cpp")
      
      set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/WIN32)
  else()
    file(GLOB PLATFORM_HEADERS "include/win32/*.h")
    file(GLOB PLATFORM_SOURCES "src/win32/*.cpp")

    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/win32)
  endif()
elseif (APPLE)
  file(GLOB PLATFORM_HEADERS "include/OSX/*.h")
  file(GLOB PLATFORM_SOURCES "src/OSX/*.mm")

  set(NATIVE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/OSX 
    ${OGRE_SOURCE_DIR}/OgreMain/include/OSX)
  set(PLATFORM_LIBS ${Cocoa_LIBRARY_FWK})
elseif (UNIX)
  if(OGRE_GLSUPPORT_USE_EGL)
    file(GLOB PLATFORM_HEADERS "include/EGL/X11/*.h" "include/EGL/*.h")
    file(GLOB PLATFORM_SOURCES "src/EGL/X11/*.cpp" "src/EGL/*.cpp")
    
    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
        ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL/X11)
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB} ${EGL_LIBRARIES})
  else()
    file(GLOB PLATFORM_HEADERS "include/GLX/*.h")
    file(GLOB PLATFORM_SOURCES "src/GLX/*.cpp")

    set(NATIVE_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/GLX)
    
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xrandr_LIB})
  endif()
  
  if(NOT OGRE_STATIC)
      add_definitions(-fPIC)
  endif()
endif ()

file(GLOB GLSUPPORT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
set(GLSUPPORT_HEADERS ${GLSUPPORT_HEADERS} PARENT_SCOPE)
set(GLSUPPORT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

if(NOT APPLE_IOS)
    include_directories(${NATIVE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/include/)
    ogre_add_library(OgreGLSupport STATIC ${GLSUPPORT_HEADERS} ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
    target_link_libraries(OgreGLSupport OgreMain ${PLATFORM_LIBS})
    
    if (APPLE)
        set_target_properties(OgreGLSupport PROPERTIES LINK_FLAGS "-framework Cocoa -framework OpenGL")
    endif()

    ogre_config_common(OgreGLSupport)

    set(GLSUPPORT_LIBS OgreGLSupport ${PLATFORM_LIBS} PARENT_SCOPE) 
endif()
