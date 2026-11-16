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

    if(EMSCRIPTEN)
        # all code must use the same exception handling, see CMakeLists.txt
        set(CROSS ${CROSS}
            -DCMAKE_C_FLAGS=-fwasm-exceptions
            -DCMAKE_CXX_FLAGS=-fwasm-exceptions)
    endif()
    
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

if(APPLE)
    set(CROSS ${CROSS}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})
endif()

# if we build our own deps, do it static as it generally eases distribution
set(OGREDEPS_SHARED FALSE)

# get available processor cores
include(ProcessorCount)
ProcessorCount(NPROC)

# can not get processor cores, fallback to default value
if(NPROC EQUAL 0)
    set(NPROC 2)
endif()

# Multi-config generators (Visual Studio, Xcode, Ninja Multi-Config) select the
# configuration at build time, so the dependencies have to be installed for
# every configuration up front. Release, RelWithDebInfo and MinSizeRel are ABI
# compatible, hence Debug and Release cover all built-in configurations.
# If CMAKE_BUILD_TYPE is set anyway - like the packaged SDK does - only that
# configuration is needed and building the other one is a waste of time.
get_property(OGRE_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(OGRE_GENERATOR_IS_MULTI_CONFIG)
  if(CMAKE_BUILD_TYPE)
    set(OGRE_DEPENDENCY_CONFIGS ${CMAKE_BUILD_TYPE})
  else()
    set(OGRE_DEPENDENCY_CONFIGS Debug Release)
  endif()
endif()

set(BUILD_COMMAND_OPTS --target install -j ${NPROC})

macro(ogre_build_dependency _binary_dir)
  if(OGRE_GENERATOR_IS_MULTI_CONFIG)
    foreach(_config ${OGRE_DEPENDENCY_CONFIGS})
      execute_process(COMMAND ${CMAKE_COMMAND}
        --build ${_binary_dir} ${BUILD_COMMAND_OPTS} --config ${_config})
    endforeach()
  else()
    # single-config generators baked the configuration in at configure time;
    # CMAKE_BUILD_TYPE may well be empty, so there is no --config to pass
    execute_process(COMMAND ${CMAKE_COMMAND}
      --build ${_binary_dir} ${BUILD_COMMAND_OPTS})
  endif()
endmacro()

# ogre_download_dependency(<var> <dir> <url> [SHA256 <hash>])
# Download the tarball at <url> and unpack it into PROJECT_BINARY_DIR. <dir> is
# the top-level directory of the archive and also names the local copy, so
# tag-only archive names like v1.0.tar.gz cannot clash. The path of the
# unpacked sources is returned in <var>.
function(ogre_download_dependency _var _dir _url)
  cmake_parse_arguments(_arg "" "SHA256" "" ${ARGN})
  set(_archive ${PROJECT_BINARY_DIR}/${_dir}.tar.gz)
  set(_hash)
  if(_arg_SHA256)
    set(_hash EXPECTED_HASH SHA256=${_arg_SHA256})
  endif()
  file(DOWNLOAD ${_url} ${_archive} ${_hash} STATUS _status)
  list(GET _status 0 _code)
  if(NOT _code EQUAL 0)
    list(GET _status 1 _msg)
    message(FATAL_ERROR "Downloading ${_url} failed: ${_msg}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${_archive}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
  set(${_var} ${PROJECT_BINARY_DIR}/${_dir} PARENT_SCOPE)
endfunction()

set(BUILD_COMMAND_COMMON ${CMAKE_COMMAND}
  -DCMAKE_INSTALL_PREFIX=${OGREDEPS_PATH}
  -G ${CMAKE_GENERATOR}
  -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
  -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
  -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
  -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE # allow linking into a shared lib
  ${CROSS})

if(NOT OGRE_GENERATOR_IS_MULTI_CONFIG)
  list(APPEND BUILD_COMMAND_COMMON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Dependencies built for a multi-config generator only provide Debug and
# Release. If an imported target has Release but not one of the other built-in
# configurations, map the latter to Release (they are ABI compatible) instead
# of letting CMake fall back to the first imported configuration, which is
# Debug. Config-less packages ("NONE") are left alone.
macro(ogre_map_imported_config _target)
  if(TARGET ${_target})
    get_target_property(_aliased ${_target} ALIASED_TARGET)
    get_target_property(_imported_configs ${_target} IMPORTED_CONFIGURATIONS)
    if(NOT _aliased AND _imported_configs)
      string(TOUPPER "${_imported_configs}" _imported_configs)
      foreach(_config RelWithDebInfo MinSizeRel)
        string(TOUPPER "${_config}" _config_upper)
        if("RELEASE" IN_LIST _imported_configs AND NOT _config_upper IN_LIST _imported_configs)
          set_target_properties(${_target} PROPERTIES MAP_IMPORTED_CONFIG_${_config_upper} Release)
        endif()
      endforeach()
    endif()
    unset(_aliased)
    unset(_imported_configs)
    unset(_config_upper)
  endif()
endmacro()

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
    set(OGREDEPS_PUGIXML_VERSION 1.15)
    set(OGREDEPS_FREETYPE_VERSION 2.14.3)
    set(OGREDEPS_SDL2_VERSION 2.32.10)
    set(OGREDEPS_ZLIB_VERSION 1.3.2)
    set(OGREDEPS_ASSIMP_VERSION 6.0.5)
    set(OGREDEPS_BULLET_VERSION 3.25)

    message(STATUS "Building pugixml")
    ogre_download_dependency(PUGIXML_SOURCE_DIR pugixml-${OGREDEPS_PUGIXML_VERSION}
        https://github.com/zeux/pugixml/releases/download/v${OGREDEPS_PUGIXML_VERSION}/pugixml-${OGREDEPS_PUGIXML_VERSION}.tar.gz)
    execute_process(COMMAND ${BUILD_COMMAND_COMMON}
        -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE # this will be linked into a shared lib
        # Debug and Release are installed next to each other
        -DPUGIXML_USE_POSTFIX=${OGRE_GENERATOR_IS_MULTI_CONFIG}
        ${PUGIXML_SOURCE_DIR}
        WORKING_DIRECTORY ${PUGIXML_SOURCE_DIR})
    ogre_build_dependency(${PUGIXML_SOURCE_DIR})

    #find_package(Freetype)
    if (NOT FREETYPE_FOUND)
        message(STATUS "Building freetype")
        ogre_download_dependency(FREETYPE_SOURCE_DIR freetype-${OGREDEPS_FREETYPE_VERSION}
            https://download.savannah.gnu.org/releases/freetype/freetype-${OGREDEPS_FREETYPE_VERSION}.tar.gz)
        # patch toolchain for iOS
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy
            ${PROJECT_SOURCE_DIR}/CMake/toolchain/ios.toolchain.xcode.cmake
            ${FREETYPE_SOURCE_DIR}/builds/cmake/iOS.cmake)
        execute_process(COMMAND ${BUILD_COMMAND_COMMON}
            -DBUILD_SHARED_LIBS=${OGREDEPS_SHARED}
            -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE # disable third-party deps
            -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
            -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE
            -DCMAKE_POLICY_VERSION_MINIMUM=3.12
            # workaround for broken iOS toolchain in freetype
            -DPROJECT_SOURCE_DIR=${FREETYPE_SOURCE_DIR}
            ${FREETYPE_SOURCE_DIR}
            WORKING_DIRECTORY ${FREETYPE_SOURCE_DIR}/objs)
        ogre_build_dependency(${FREETYPE_SOURCE_DIR}/objs)
    endif()

    if(MSVC OR MINGW OR SKBUILD) # other platforms dont need this
        # SDL loads enabled audio backends
        # at runtime by default (SDL_*_SHARED), so no hard dependency on a
        # specific sound server is added.
        set(SDL_AUDIO_OPTS)
        if(UNIX AND NOT APPLE)
            set(SDL_AUDIO_OPTS -DSDL_ALSA=ON -DSDL_PULSEAUDIO=ON)
        endif()

        message(STATUS "Building SDL2")
        ogre_download_dependency(SDL2_SOURCE_DIR SDL2-${OGREDEPS_SDL2_VERSION}
            https://github.com/libsdl-org/SDL/releases/download/release-${OGREDEPS_SDL2_VERSION}/SDL2-${OGREDEPS_SDL2_VERSION}.tar.gz
            SHA256 5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165)
        execute_process(COMMAND ${CMAKE_COMMAND}
            -E make_directory ${PROJECT_BINARY_DIR}/SDL2-build)
        execute_process(COMMAND ${BUILD_COMMAND_COMMON}
            -DSDL_STATIC=FALSE
            -DCMAKE_INSTALL_LIBDIR=lib
            ${SDL_AUDIO_OPTS}
            ${SDL2_SOURCE_DIR}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/SDL2-build)
        ogre_build_dependency(${PROJECT_BINARY_DIR}/SDL2-build)
    endif()

    if(MSVC OR MINGW OR SKBUILD) # other platforms dont need this
      message(STATUS "Building zlib") # only needed for Assimp
      ogre_download_dependency(ZLIB_SOURCE_DIR zlib-${OGREDEPS_ZLIB_VERSION}
          https://github.com/madler/zlib/releases/download/v${OGREDEPS_ZLIB_VERSION}/zlib-${OGREDEPS_ZLIB_VERSION}.tar.gz
          SHA256 bb329a0a2cd0274d05519d61c667c062e06990d72e125ee2dfa8de64f0119d16)
      execute_process(COMMAND ${BUILD_COMMAND_COMMON}
          -DBUILD_SHARED_LIBS=${OGREDEPS_SHARED}
          ${ZLIB_SOURCE_DIR}
          WORKING_DIRECTORY ${ZLIB_SOURCE_DIR})
      ogre_build_dependency(${ZLIB_SOURCE_DIR})

      message(STATUS "Building Assimp")
      ogre_download_dependency(ASSIMP_SOURCE_DIR assimp-${OGREDEPS_ASSIMP_VERSION}
          https://github.com/assimp/assimp/archive/refs/tags/v${OGREDEPS_ASSIMP_VERSION}.tar.gz)
      # Assimp is only used for asset import, so favour size over import speed.
      # Override the configuration flags rather than the build type to keep
      # Debug builds debuggable and multi-config generators working.
      set(ASSIMP_POSTFIX_OPTS)
      if(OGRE_GENERATOR_IS_MULTI_CONFIG)
          # set explicitly rather than relying on the cached default, so that
          # an already configured source tree cannot leave it empty
          set(ASSIMP_POSTFIX_OPTS -DCMAKE_DEBUG_POSTFIX=d)
      endif()
      execute_process(COMMAND ${BUILD_COMMAND_COMMON}
          "-DCMAKE_CXX_FLAGS_RELEASE=-Os -DNDEBUG"
          "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=-Os -g -DNDEBUG"
          -DZLIB_ROOT=${OGREDEPS_PATH}
          -DBUILD_SHARED_LIBS=OFF
          -DASSIMP_BUILD_TESTS=OFF
          -DASSIMP_NO_EXPORT=TRUE
          -DASSIMP_BUILD_OGRE_IMPORTER=OFF
          # IFC accounts for ~3.4 MB of the statically linked plugin and is
          # rarely used for real-time assets.
          -DASSIMP_BUILD_IFC_IMPORTER=OFF
          -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
          ${ASSIMP_POSTFIX_OPTS}
          ${ASSIMP_SOURCE_DIR}
          WORKING_DIRECTORY ${ASSIMP_SOURCE_DIR})
      ogre_build_dependency(${ASSIMP_SOURCE_DIR})
    endif()

    message(STATUS "Building Bullet")
    ogre_download_dependency(BULLET_SOURCE_DIR bullet3-${OGREDEPS_BULLET_VERSION}
        https://github.com/bulletphysics/bullet3/archive/refs/tags/${OGREDEPS_BULLET_VERSION}.tar.gz)
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
        -DCMAKE_DEBUG_POSTFIX= # need to remove postfix so default find_package detects Bullet correctly
        -DBUILD_CLSOCKET=OFF
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        ${BULLET_SOURCE_DIR}
        WORKING_DIRECTORY ${BULLET_SOURCE_DIR})
    ogre_build_dependency(${BULLET_SOURCE_DIR})
    set(Bullet_ROOT ${OGREDEPS_PATH})
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
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(waylands IMPORTED_TARGET wayland-client wayland-egl egl)
    macro_log_feature(waylands_FOUND "Wayland" "Wayland window system" "https://wayland.freedesktop.org")
  endif ()

  if (NOT waylands_FOUND)
    find_package(X11 REQUIRED)
  else ()
    find_package(X11)
  endif ()
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
set(VULKAN_SDK_FOUND "$ENV{VULKAN_SDK}")
macro_log_feature(VULKAN_SDK_FOUND "Vulkan SDK" "Vulkan RenderSystem, glslang Plugin. Alternatively use system packages" "https://vulkan.lunarg.com/")

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
ogre_map_imported_config(pugixml::pugixml)
ogre_map_imported_config(pugixml::static)
macro_log_feature(pugixml_FOUND "pugixml" "Needed for XMLConverter and DotScene Plugin" "https://pugixml.org/")

# Find zlib
find_package(ZLIB)
ogre_map_imported_config(ZLIB::ZLIB)
macro_log_feature(ZLIB_FOUND "zlib" "Simple data compression library" "http://www.zlib.net")

# Assimp
find_package(assimp QUIET)
ogre_map_imported_config(assimp::assimp)
macro_log_feature(assimp_FOUND "Assimp" "Needed for the AssimpLoader Plugin" "https://www.assimp.org/")

# Bullet
find_package(Bullet QUIET)
macro_log_feature(BULLET_FOUND "Bullet" "Bullet physics" "https://pybullet.org")

if(assimp_FOUND)
  if(assimp_VERSION VERSION_LESS 5.1)
    # workaround horribly broken assimp cmake, fixed with assimp 5.1
    add_library(fix::assimp INTERFACE IMPORTED)
    set_target_properties(fix::assimp PROPERTIES
        INTERFACE_LINK_LIBRARIES "${ASSIMP_LIBRARIES};pugixml"
        INTERFACE_LINK_DIRECTORIES "${ASSIMP_LIBRARY_DIRS}"
    )
    if(EXISTS "${ASSIMP_INCLUDE_DIRS}")
      set_target_properties(fix::assimp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIRS}")
    endif()
    set(OGRE_ASSIMP_TARGET fix::assimp)
  else()
    # assimp >= 5.1 ships a proper CMake package with a namespaced target
    set(OGRE_ASSIMP_TARGET assimp::assimp)
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
  ogre_map_imported_config(SDL2::SDL2)
  ogre_map_imported_config(SDL2::SDL2-static)

  if(SKBUILD AND NOT SDL2_FOUND)
    # the Python wheels ship OgreBites with the bundled SDL2 and must not be
    # published without input handling
    message(FATAL_ERROR "SDL2 is required to build the Python packages, but was not found. "
        "See the dependency build output above.")
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
