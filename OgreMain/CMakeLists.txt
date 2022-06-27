#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# OgreMain core library
############################################################

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES
    ${PROJECT_BINARY_DIR}/include/OgreBuildSettings.h
    ${PROJECT_BINARY_DIR}/include/OgreComponents.h
    ${PROJECT_BINARY_DIR}/include/OgreExports.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Ogre.i)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Remove optional header files
list(REMOVE_ITEM HEADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreDDSCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgrePVRTCCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreETCCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreZip.h"
)

# Remove optional source files
list(REMOVE_ITEM SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreFileSystemLayerNoOp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreDDSCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgrePVRTCCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreETCCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreZip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreSearchOps.cpp"
)

# Add platform specific files
if (EMSCRIPTEN)
  set(PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreFileSystemLayerNoOp.cpp")
elseif (WIN32)
  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/*.cpp")
elseif (APPLE)
  if (APPLE_IOS)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/iOS")
    file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/iOS/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/iOS/*.mm")

    set(PLATFORM_LIBS "")
  else ()
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/OSX")
    file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OSX/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/OSX/*.mm")
    set(PLATFORM_LIBS "-framework CoreFoundation -framework Foundation")
  endif ()
elseif(ANDROID)
  # required by OgrePlatformInformation.cpp
  include_directories(${ANDROID_NDK}/sources/android/cpufeatures)

  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Android/*.cpp")
  # Remove optional platform source files
  list(REMOVE_ITEM PLATFORM_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Android/OgreAPKZipArchive.cpp"
  )
elseif (UNIX)
  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLX/*.cpp")

  set(PLATFORM_LIBS pthread)
endif()

if(NOT WIN32)
    list(APPEND PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreSearchOps.cpp")
endif()

# Configure threading files
file(GLOB THREAD_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/Threading/*.h")
include_directories("include/Threading" "src/")

if (OGRE_THREAD_PROVIDER EQUAL 0)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesNone.h
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 1)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})

	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesBoost.h
		include/Threading/OgreThreadHeadersBoost.h
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 2)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesPoco.h
		include/Threading/OgreThreadHeadersPoco.h
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 4)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesSTD.h
		include/Threading/OgreThreadHeadersSTD.h
	)
endif ()

list(APPEND HEADER_FILES ${THREAD_HEADER_FILES})

set(LIBRARIES)

if (OGRE_CONFIG_THREADS)
  list(APPEND LIBRARIES ${OGRE_THREAD_LIBRARIES})
endif ()

if (OGRE_CONFIG_ENABLE_DDS)
  list(APPEND SOURCE_FILES src/OgreDDSCodec.cpp src/OgreDDSCodec.h)
endif ()

if (OGRE_CONFIG_ENABLE_PVRTC)
  list(APPEND HEADER_FILES )
  list(APPEND SOURCE_FILES src/OgrePVRTCCodec.cpp src/OgrePVRTCCodec.h)
endif ()

if (OGRE_CONFIG_ENABLE_ETC)
  list(APPEND SOURCE_FILES src/OgreETCCodec.cpp src/OgreETCCodec.h)
endif ()

if (OGRE_CONFIG_ENABLE_ASTC)
  list(APPEND SOURCE_FILES src/OgreASTCCodec.cpp src/OgreASTCCodec.h)
endif ()

if (OGRE_CONFIG_ENABLE_ZIP)
  include_directories(src/zip)
  list(APPEND HEADER_FILES include/OgreZip.h)
  list(APPEND SOURCE_FILES src/OgreZip.cpp src/zip/zip.c)

  if(ANDROID)
    list(APPEND PLATFORM_SOURCE_FILES src/Android/OgreAPKZipArchive.cpp)
  endif()
endif ()

if(OGRE_CONFIG_FILESYSTEM_UNICODE)
  set_source_files_properties(src/OgreFileSystem.cpp
    PROPERTIES COMPILE_DEFINITIONS _OGRE_FILESYSTEM_ARCHIVE_UNICODE)
  set_source_files_properties(src/OgreZip.cpp
    PROPERTIES COMPILE_DEFINITIONS _OGRE_FILESYSTEM_ARCHIVE_UNICODE)
endif()

if(OGRE_PROFILING_REMOTERY_PATH)
  list(APPEND SOURCE_FILES ${OGRE_PROFILING_REMOTERY_PATH}/Remotery.c)
  set_source_files_properties(${OGRE_PROFILING_REMOTERY_PATH}/Remotery.c PROPERTIES LANGUAGE CXX)
  set_source_files_properties(src/OgreProfiler.cpp PROPERTIES COMPILE_DEFINITIONS USE_REMOTERY)
endif()

set (TARGET_LINK_FLAGS "")

# setup OgreMain target

get_native_precompiled_header(OgreMain)
add_library(OgreMain ${OGRE_LIB_TYPE} ${OgreMain_pch} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES})

if (WINDOWS_STORE OR WINDOWS_PHONE)
	set_target_properties(OgreMain PROPERTIES VS_WINRT_COMPONENT "true")
endif ()

generate_export_header(OgreMain 
    EXPORT_MACRO_NAME _OgreExport
    NO_EXPORT_MACRO_NAME _OgrePrivate
    DEPRECATED_MACRO_NAME OGRE_DEPRECATED
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreExports.h)

target_include_directories(OgreMain PUBLIC 
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" 
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE>
  PRIVATE "${OGRE_PROFILING_REMOTERY_PATH}")

# In visual studio 2010 - 64 bit we get this error: "LINK : fatal error LNK1210: exceeded internal ILK size limit; link with /INCREMENTAL:NO"
if(WIN32 AND MSVC10 AND CMAKE_CL_64)
  set_target_properties(OgreMain PROPERTIES 
                        VERSION ${OGRE_SOVERSION}
                        LINK_FLAGS "/INCREMENTAL:NO"
                      )
else()
  set_target_properties(OgreMain PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
endif()

if (APPLE)
  if (APPLE_IOS)
    set_target_properties(OgreMain PROPERTIES INSTALL_NAME_DIR "OGRE")
  else()
    if(OGRE_BUILD_LIBS_AS_FRAMEWORKS)
      set(OGRE_OSX_BUILD_CONFIGURATION "${CMAKE_OSX_SYSROOT}/$(CONFIGURATION)")
      add_custom_command(TARGET OgreMain POST_BUILD
          COMMAND mkdir ARGS -p ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/Threading
          COMMAND ditto
          ${PROJECT_SOURCE_DIR}/OgreMain/include/Threading/*.h ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/Threading
          COMMAND cd ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers
          )

      foreach(HEADER_PATH ${THREAD_HEADER_FILES})
          get_filename_component(HEADER_FILE ${HEADER_PATH} NAME)
          set(FWK_HEADER_PATH ${PROJECT_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/${HEADER_FILE})
          add_custom_command(TARGET OgreMain POST_BUILD
              COMMAND rm -f ${FWK_HEADER_PATH}
              )
      endforeach()
    endif()

    ogre_config_framework(OgreMain)
  endif ()

  if(OGRE_BUILD_LIBS_AS_FRAMEWORKS)
    # Framework is called 'Ogre'
    set_target_properties(OgreMain PROPERTIES	OUTPUT_NAME Ogre)
  endif()
endif ()
target_link_libraries(OgreMain PUBLIC ${PLATFORM_LIBS} PRIVATE ${LIBRARIES} ${CMAKE_DL_LIBS})

# specify a precompiled header to use
add_native_precompiled_header(OgreMain "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/OgreStableHeaders.h>")

# install OgreMain
ogre_config_lib(OgreMain TRUE)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE
)
install(FILES ${PLATFORM_HEADERS}
  DESTINATION include/OGRE/${PLATFORM_HEADER_INSTALL}
)
install(FILES ${THREAD_HEADER_FILES}
	DESTINATION include/OGRE/Threading
)
