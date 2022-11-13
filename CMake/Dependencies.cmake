#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

#######################################################################
# Find all necessary and optional OGRE dependencies
#######################################################################

# OGRE_DEPENDENCIES_DIR can be used to specify a single base
# folder where the required dependencies may be found.
set(OGRE_DEPENDENCIES_DIR "${PROJECT_BINARY_DIR}/Dependencies" CACHE PATH "Path to prebuilt OGRE dependencies")
option(OGRE_BUILD_DEPENDENCIES "automatically build Ogre Dependencies (SDL2, pugixml)" TRUE)

message(STATUS "DEPENDENCIES_DIR: ${OGRE_DEPENDENCIES_DIR}")

set(OGREDEPS_PATH "${OGRE_DEPENDENCIES_DIR}")
set(OGRE_DEP_SEARCH_PATH "${OGRE_DEPENDENCIES_DIR}")

if(CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH ${OGREDEPS_PATH} "${CMAKE_FIND_ROOT_PATH}")

    set(CROSS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
    
    if(ANDROID)
        set(CROSS ${CROSS}
            -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}
            -DANDROID_ABI=${ANDROID_ABI}
            -DANDROID_NDK=${ANDROID_NDK})
    endif()
    
    if(APPLE_IOS)
        set(CROSS ${CROSS}
            -DIOS_PLATFORM=${IOS_PLATFORM})
    else()
        # this should help discovering zlib, but on ios it breaks it
        set(CROSS ${CROSS}
            -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH})
    endif()
endif()

# if we build our own deps, do it static as it generally eases distribution
set(OGREDEPS_SHARED FALSE)

set(BUILD_COMMAND_OPTS --target install --config ${CMAKE_BUILD_TYPE})

set(BUILD_COMMAND_COMMON ${CMAKE_COMMAND}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${OGREDEPS_PATH}
  -G ${CMAKE_GENERATOR}
  -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
  -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
  -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
  -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE # allow linking into a shared lib
  ${CROSS})

# Set hardcoded path guesses for various platforms
if (UNIX AND NOT EMSCRIPTEN)
  set(OGRE_DEP_SEARCH_PATH ${OGRE_DEP_SEARCH_PATH} /usr/local)
  # Ubuntu 11.10 has an inconvenient path to OpenGL libraries
  set(OGRE_DEP_SEARCH_PATH ${OGRE_DEP_SEARCH_PATH} /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
endif ()

# give guesses as hints to the find_package calls
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OGRE_DEP_SEARCH_PATH})
set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${OGRE_DEP_SEARCH_PATH})

if(OGRE_BUILD_DEPENDENCIES AND NOT EXISTS ${OGREDEPS_PATH})
    message(STATUS "Building pugixml")
    file(DOWNLOAD
        https://github.com/zeux/pugixml/releases/download/v1.12/pugixml-1.12.tar.gz
        ${PROJECT_BINARY_DIR}/pugixml-1.12.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E tar xf pugixml-1.12.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    execute_process(COMMAND ${BUILD_COMMAND_COMMON}
        -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE # this will be linked into a shared lib
        ${PROJECT_BINARY_DIR}/pugixml-1.12
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/pugixml-1.12)
    execute_process(COMMAND ${CMAKE_COMMAND}
        --build ${PROJECT_BINARY_DIR}/pugixml-1.12 ${BUILD_COMMAND_OPTS})

    #find_package(Freetype)
    if (NOT FREETYPE_FOUND)
        message(STATUS "Building freetype")
        file(DOWNLOAD
            https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.gz
            ${PROJECT_BINARY_DIR}/freetype-2.12.1.tar.gz)
        execute_process(COMMAND ${CMAKE_COMMAND}
            -E tar xf freetype-2.12.1.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
        # patch toolchain for iOS
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
            ${PROJECT_SOURCE_DIR}/CMake/toolchain/ios.toolchain.xcode.cmake
            freetype-2.12.1/builds/cmake/iOS.cmake
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
        execute_process(COMMAND ${BUILD_COMMAND_COMMON}
            -DBUILD_SHARED_LIBS=${OGREDEPS_SHARED}
            -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE # disable third-party deps
            -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE
            # workaround for broken iOS toolchain in freetype
            -DPROJECT_SOURCE_DIR=${PROJECT_BINARY_DIR}/freetype-2.12.1
            ${PROJECT_BINARY_DIR}/freetype-2.12.1
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/freetype-2.12.1/objs)
        execute_process(COMMAND ${CMAKE_COMMAND}
            --build ${PROJECT_BINARY_DIR}/freetype-2.12.1/objs ${BUILD_COMMAND_OPTS})
    endif()

    if(MSVC OR MINGW OR SKBUILD) # other platforms dont need this
        message(STATUS "Building SDL2")
        file(DOWNLOAD
            https://libsdl.org/release/SDL2-2.24.1.tar.gz
            ${PROJECT_BINARY_DIR}/SDL2-2.24.1.tar.gz)
        execute_process(COMMAND ${CMAKE_COMMAND} 
            -E tar xf SDL2-2.24.1.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
        execute_process(COMMAND ${CMAKE_COMMAND}
            -E make_directory ${PROJECT_BINARY_DIR}/SDL2-build)
        execute_process(COMMAND ${BUILD_COMMAND_COMMON}
            -DSDL_STATIC=FALSE
            -DCMAKE_INSTALL_LIBDIR=lib
            ${PROJECT_BINARY_DIR}/SDL2-2.24.1
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/SDL2-build)
        execute_process(COMMAND ${CMAKE_COMMAND}
            --build ${PROJECT_BINARY_DIR}/SDL2-build ${BUILD_COMMAND_OPTS})
    endif()

    if(MSVC OR MINGW OR SKBUILD) # other platforms dont need this
      message(STATUS "Building zlib") # only needed for Assimp
      file(DOWNLOAD
          http://zlib.net/zlib-1.2.13.tar.gz
          ${PROJECT_BINARY_DIR}/zlib-1.2.13.tar.gz
          EXPECTED_HASH SHA256=b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30)
      execute_process(COMMAND ${CMAKE_COMMAND}
          -E tar xf zlib-1.2.13.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
      execute_process(COMMAND ${BUILD_COMMAND_COMMON}
          -DBUILD_SHARED_LIBS=${OGREDEPS_SHARED}
          ${PROJECT_BINARY_DIR}/zlib-1.2.13
          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/zlib-1.2.13)
      execute_process(COMMAND ${CMAKE_COMMAND}
          --build ${PROJECT_BINARY_DIR}/zlib-1.2.13 ${BUILD_COMMAND_OPTS})

      message(STATUS "Building Assimp")
      file(DOWNLOAD
              https://github.com/assimp/assimp/archive/refs/tags/v5.2.5.zip
          ${PROJECT_BINARY_DIR}/v5.2.5.tar.gz)
      execute_process(COMMAND ${CMAKE_COMMAND}
          -E tar xf v5.2.5.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
      execute_process(COMMAND ${BUILD_COMMAND_COMMON}
          -DZLIB_ROOT=${OGREDEPS_PATH}
          -DBUILD_SHARED_LIBS=OFF
          -DASSIMP_BUILD_TESTS=OFF
          -DASSIMP_NO_EXPORT=TRUE
          -DASSIMP_BUILD_OGRE_IMPORTER=OFF
          -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
          ${PROJECT_BINARY_DIR}/assimp-5.2.5
          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/assimp-5.2.5)
      execute_process(COMMAND ${CMAKE_COMMAND}
        --build ${PROJECT_BINARY_DIR}/assimp-5.2.5 ${BUILD_COMMAND_OPTS})
    endif()

    message(STATUS "Building Bullet")
    file(DOWNLOAD
        https://github.com/bulletphysics/bullet3/archive/refs/tags/3.24.tar.gz
        ${PROJECT_BINARY_DIR}/3.24.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND}
        -E tar xf 3.24.tar.gz WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
    execute_process(COMMAND ${BUILD_COMMAND_COMMON}
        -DBUILD_SHARED_LIBS=OFF
        -DINSTALL_LIBS=ON
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON
        -DBUILD_PYBULLET=OFF
        -DUSE_DOUBLE_PRECISION=OFF
        -DBUILD_CPU_DEMOS=OFF
        -DBUILD_BULLET2_DEMOS=OFF
        -DBUILD_EXTRAS=OFF
        -DBUILD_EGL=OFF
        -DBUILD_ENET=OFF
        -DBUILD_UNIT_TESTS=OFF
        -DCMAKE_RELWITHDEBINFO_POSTFIX= # fixes FindBullet on MSVC
        -DBUILD_CLSOCKET=OFF
        ${PROJECT_BINARY_DIR}/bullet3-3.24
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bullet3-3.24)
    execute_process(COMMAND ${CMAKE_COMMAND}
        --build ${PROJECT_BINARY_DIR}/bullet3-3.24 ${BUILD_COMMAND_OPTS})
    set(BULLET_ROOT ${OGREDEPS_PATH})
endif()

#######################################################################
# Core dependencies
#######################################################################

# Find FreeImage
find_package(FreeImage)
macro_log_feature(FreeImage_FOUND "freeimage" "Support for commonly used graphics image formats" "http://freeimage.sourceforge.net")

# Find FreeType
find_package(Freetype)
macro_log_feature(FREETYPE_FOUND "freetype" "Portable font engine" "http://www.freetype.org")

# Find X11
if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN)
  find_package(X11 REQUIRED)
  macro_log_feature(X11_FOUND "X11" "X Window system" "http://www.x.org")
endif ()


#######################################################################
# RenderSystem dependencies
#######################################################################

# Find OpenGL
if(NOT ANDROID AND NOT EMSCRIPTEN)
  find_package(OpenGL)
  macro_log_feature(OPENGL_FOUND "OpenGL" "Support for the OpenGL and OpenGL 3+ render systems" "http://www.opengl.org/")
endif()

# Find OpenGL ES 2.x
find_package(OpenGLES2)
macro_log_feature(OPENGLES2_FOUND "OpenGL ES 2.x" "Support for the OpenGL ES 2.x render system" "http://www.khronos.org/opengles/")

# Find DirectX
if(WIN32)
	find_package(DirectX)
	macro_log_feature(DirectX9_FOUND "DirectX9" "Support for the DirectX render system" "http://msdn.microsoft.com/en-us/directx/")
	
	find_package(DirectX11)
	macro_log_feature(DirectX11_FOUND "DirectX11" "Support for the DirectX11 render system" "http://msdn.microsoft.com/en-us/directx/")

	if(OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO)
		# Find DirectX Stereo Driver Libraries
		find_package(NVAPI)
		macro_log_feature(NVAPI_FOUND "NVAPI" "Support NVIDIA stereo with the DirectX render system" "https://developer.nvidia.com/nvapi")

		find_package(AMDQBS)
		macro_log_feature(AMDQBS_FOUND "AMDQBS" "Support AMD stereo with the DirectX render system" "http://developer.amd.com/tools-and-sdks/graphics-development/amd-quad-buffer-sdk/")
	endif()
endif()

#######################################################################
# Additional features
#######################################################################

# Find Cg
if (NOT (APPLE_IOS OR WINDOWS_STORE OR WINDOWS_PHONE OR ANDROID OR EMSCRIPTEN))
  find_package(Cg)
  macro_log_feature(Cg_FOUND "cg" "C for graphics shader language" "http://developer.nvidia.com/object/cg_toolkit.html")
endif ()

# Find Vulkan SDK
macro_log_feature(ENV{VULKAN_SDK} "Vulkan SDK" "Vulkan RenderSystem, glslang Plugin. Alternatively use system packages" "https://vulkan.lunarg.com/")

# OpenEXR
find_package(OpenEXR)
macro_log_feature(OPENEXR_FOUND "OpenEXR" "Load High dynamic range images" "http://www.openexr.com/")

# Python
set(Python_ADDITIONAL_VERSIONS 3.4) # allows using python3 on Ubuntu 14.04
find_package(PythonInterp)
find_package(PythonLibs)
macro_log_feature(PYTHONLIBS_FOUND "Python" "Language bindings to use OGRE from Python" "http://www.python.org/")

# SWIG
find_package(SWIG 3.0.8 QUIET)
macro_log_feature(SWIG_FOUND "SWIG" "Language bindings (Python, Java, C#) for OGRE" "http://www.swig.org/")

# pugixml
find_package(pugixml QUIET)
macro_log_feature(pugixml_FOUND "pugixml" "Needed for XMLConverter and DotScene Plugin" "https://pugixml.org/")

# Find zlib
find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "zlib" "Simple data compression library" "http://www.zlib.net")

# Assimp
find_package(assimp QUIET)
macro_log_feature(assimp_FOUND "Assimp" "Needed for the AssimpLoader Plugin" "https://www.assimp.org/")

# Bullet
find_package(Bullet QUIET)
macro_log_feature(BULLET_FOUND "Bullet" "Bullet physics" "https://pybullet.org")

if(assimp_FOUND)
  # workaround horribly broken assimp cmake, fixed with assimp 5.1
  add_library(fix::assimp INTERFACE IMPORTED)
  set_target_properties(fix::assimp PROPERTIES
      INTERFACE_LINK_LIBRARIES "${ASSIMP_LIBRARIES};pugixml"
      INTERFACE_LINK_DIRECTORIES "${ASSIMP_LIBRARY_DIRS}"
  )
  if(EXISTS "${ASSIMP_INCLUDE_DIRS}")
    set_target_properties(fix::assimp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIRS}")
  endif()
endif()

#######################################################################
# Samples dependencies
#######################################################################

# Find sdl2
if(NOT ANDROID AND NOT EMSCRIPTEN)
  # find script does not work in cross compilation environment
  find_package(SDL2 QUIET)
  macro_log_feature(SDL2_FOUND "SDL2" "Simple DirectMedia Library needed for input handling in samples" "https://www.libsdl.org/")
  if(SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
    )
  endif()

  find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui QUIET CONFIG)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui QUIET CONFIG)

  macro_log_feature(QT_FOUND "Qt" "optional integration with the Qt Library for window creation and input" "http://www.qt.io/")
endif()

#######################################################################
# Tools
#######################################################################

find_package(Doxygen QUIET)
macro_log_feature(DOXYGEN_FOUND "Doxygen" "Tool for building API documentation" "http://doxygen.org")

# Find Softimage SDK
find_package(Softimage)
macro_log_feature(Softimage_FOUND "Softimage" "Softimage SDK needed for building XSIExporter" "")
