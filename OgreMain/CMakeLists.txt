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

PROJECT(OgreMain)

include(PrecompiledHeader)

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
list(APPEND HEADER_FILES ${OGRE_BINARY_DIR}/include/OgreBuildSettings.h
    src/OgreImageResampler.h
    src/OgrePixelConversions.h
    src/OgreSIMDHelper.h)

file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Remove optional header files
list(REMOVE_ITEM HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreFreeImageCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreDDSCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgrePVRTCCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreETCCodec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreZip.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreAPKZipArchive.h"
)

# Remove optional source files
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreFreeImageCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreDDSCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgrePVRTCCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreETCCodec.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreZip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreAPKZipArchive.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreSearchOps.cpp"
)

# Add platform specific files
if (OGRE_BUILD_PLATFORM_NACL)
  include_directories("include/NaCl")
  file(GLOB PLATFORM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/NaCl/*.h")
  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/NaCl/*.cpp"
    src/OgreSearchOps.cpp)

  set(PLATFORM_LIBS nosys)
  set(PLATFORM_HEADER_INSTALL "NaCl")
elseif (WIN32)
  if (OGRE_BUILD_PLATFORM_WINRT OR OGRE_BUILD_PLATFORM_WINDOWS_PHONE)
    include_directories("include/WIN32")
    set(PLATFORM_HEADERS
      include/WIN32/OgreConfigDialogImpWinRT.h
      include/WIN32/OgreErrorDialogImpWinRT.h
      include/WIN32/OgreTimerImp.h
    )
    set(PLATFORM_SOURCE_FILES
      src/WIN32/OgreConfigDialogWinRT.cpp
      src/WIN32/OgreErrorDialogWinRT.cpp
      src/WIN32/OgreTimer.cpp
	  src/WIN32/OgreOptimisedUtilDirectXMath.cpp
    )  

    set(PLATFORM_HEADER_INSTALL "WIN32")
  else ()
    include_directories("include/WIN32")
    set(PLATFORM_HEADERS
      include/WIN32/OgreConfigDialogImp.h
      include/WIN32/OgreErrorDialogImp.h
      include/WIN32/OgreTimerImp.h
      include/WIN32/OgreMinGWSupport.h
    )
    set(PLATFORM_SOURCE_FILES
      src/WIN32/OgreConfigDialog.cpp
      src/WIN32/OgreErrorDialog.cpp
      src/WIN32/OgreFileSystemLayer.cpp
      src/WIN32/OgreTimer.cpp
      src/WIN32/resource.h
    )  
    set(RESOURCE_FILES
      src/WIN32/resource.h
      src/WIN32/OgreWin32Resources.rc
    )
    source_group(Resources FILES ${RESOURCE_FILES})
    set(PLATFORM_HEADER_INSTALL "WIN32")

    if (MINGW) 
      # Older CMake versions do not know how to deal with resource files on MinGW.
      # Newer ones do, but pass along invalid command options to windres.
      # This is a manual fix for the moment.
      get_filename_component(OGRE_MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
      #set(OGRE_MINGW_RC ${OGRE_MINGW_PATH}/windres.exe)
      file(GLOB OGRE_MINGW_RC_NAMES "${OGRE_MINGW_PATH}/*windres*")
      find_program(OGRE_MINGW_RC NAMES ${OGRE_MINGW_RC_NAMES} HINTS ${OGRE_MINGW_PATH} DOC "Path to MinGW resource compiler")
      if (NOT OGRE_MINGW_RC)
        message(SEND_ERROR "Could not find MinGW resource compiler. Please specify path in OGRE_MINGW_RC.")
      endif ()
      mark_as_advanced(OGRE_MINGW_RC)
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj
        COMMAND ${OGRE_MINGW_RC} -I${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32 
                                 -i${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/OgreWin32Resources.rc
                                 -o${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj
        DEPENDS ${RESOURCE_FILES})
      list(APPEND PLATFORM_SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/OgreWin32Resources.rc.obj)
    else ()
      list(APPEND PLATFORM_SOURCE_FILES src/WIN32/OgreWin32Resources.rc)
    endif ()
  endif ()

elseif (APPLE)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    include_directories("include/iOS")
    file(GLOB PLATFORM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/iOS/*.h")
    file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/iOS/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/iOS/*.mm"
      src/OgreSearchOps.cpp)

    set(PLATFORM_LIBS "")
    set(PLATFORM_HEADER_INSTALL "iOS")
  else ()
    include_directories("include/OSX")
    
    file(GLOB PLATFORM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/OSX/*.h")
    file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/OSX/*.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/OSX/*.mm"
      src/OgreSearchOps.cpp
      include/OSX/ogrelogo.png
    )

    set(RESOURCE_FILES
	  include/OSX/ogrelogo.png
    )
    source_group(Resources FILES ${RESOURCE_FILES})

    set(PLATFORM_LIBS ${Carbon_LIBRARY_FWK} ${Cocoa_LIBRARY_FWK})
    set(PLATFORM_HEADER_INSTALL "OSX")
  endif ()
elseif(ANDROID)
  file(GLOB PLATFORM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/Android/*.h")
  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Android/*.cpp"
      src/OgreSearchOps.cpp)

  set(PLATFORM_HEADER_INSTALL "Android")
elseif(FLASHCC)
  set(PLATFORM_HEADERS
    include/FlashCC/OgreTimerImp.h
  )
  set(PLATFORM_SOURCE_FILES
    src/FlashCC/OgreTimer.cpp
  )

  set(PLATFORM_HEADER_INSTALL "FlashCC")
elseif (UNIX)
  include_directories(include/GLX ${X11_INCLUDE_DIR})

  file(GLOB PLATFORM_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/GLX/*.h"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/GLX/GLX_backdrop.h")
  file(GLOB PLATFORM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/GLX/*.cpp"
      src/OgreSearchOps.cpp)

  set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xt_LIB} ${XAW_LIBRARY} pthread)
  set(PLATFORM_HEADER_INSTALL "GLX")
  # some platforms require explicit linking to libdl, see if it's available
  find_library(DL_LIBRARY NAMES dl dlsym)
  mark_as_advanced(DL_LIBRARY)
  if (DL_LIBRARY)
    list(APPEND PLATFORM_LIBS dl)
  endif ()
endif()

# Configure threading files
file(GLOB THREAD_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/Threading/*.h")
include_directories("include/Threading")

if (OGRE_THREAD_PROVIDER EQUAL 0)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesNone.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 1)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesBoost.h
		include/Threading/OgreThreadHeadersBoost.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 2)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesPoco.h
		include/Threading/OgreThreadHeadersPoco.h
		include/Threading/OgreDefaultWorkQueueStandard.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueStandard.cpp
	)
elseif (OGRE_THREAD_PROVIDER EQUAL 3)
	list(APPEND THREAD_HEADER_FILES
		include/Threading/OgreThreadDefinesTBB.h
		include/Threading/OgreThreadHeadersTBB.h
		include/Threading/OgreDefaultWorkQueueTBB.h
	)
	set(THREAD_SOURCE_FILES
		src/Threading/OgreDefaultWorkQueueTBB.cpp
	)
endif ()

list(APPEND HEADER_FILES ${THREAD_HEADER_FILES})

# Add needed definitions and nedmalloc include dir
add_definitions(-DOGRE_NONCLIENT_BUILD -DFREEIMAGE_LIB -D_MT -D_USRDLL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/nedmalloc)

set(LIBRARIES ${PLATFORM_LIBS})

if (OGRE_CONFIG_THREADS)
  list(APPEND LIBRARIES ${OGRE_THREAD_LIBRARIES})
endif ()

if (OGRE_CONFIG_ENABLE_FREEIMAGE)
  list(APPEND HEADER_FILES include/OgreFreeImageCodec.h)
  list(APPEND SOURCE_FILES src/OgreFreeImageCodec.cpp)
  list(APPEND LIBRARIES    "${FreeImage_LIBRARIES}")
endif()

if (OGRE_CONFIG_ENABLE_DDS)
  list(APPEND HEADER_FILES include/OgreDDSCodec.h)
  list(APPEND SOURCE_FILES src/OgreDDSCodec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_PVRTC)
  list(APPEND HEADER_FILES include/OgrePVRTCCodec.h)
  list(APPEND SOURCE_FILES src/OgrePVRTCCodec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_ETC)
  list(APPEND HEADER_FILES include/OgreETCCodec.h)
  list(APPEND SOURCE_FILES src/OgreETCCodec.cpp)
endif ()

if (OGRE_CONFIG_ENABLE_ZIP)
  list(APPEND HEADER_FILES include/OgreZip.h)
  list(APPEND SOURCE_FILES src/OgreZip.cpp)

  if(ANDROID)
    ADD_DEFINITIONS(-DZZIP_OMIT_CONFIG_H)
  endif()

  list(APPEND LIBRARIES "${ZZip_LIBRARIES}")
  list(APPEND LIBRARIES "${ZLIB_LIBRARIES}")
endif ()

if (OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER)
  list(APPEND LIBRARIES "${GLSL_Optimizer_LIBRARIES}")
endif ()

if (OGRE_CONFIG_ENABLE_GLES2_CG_SUPPORT)
  list(APPEND LIBRARIES "${HLSL2GLSL_LIBRARIES}")
endif ()

set (TARGET_LINK_FLAGS "")


# setup OgreMain target
# OgreController.cpp always causes problems with Unity builds
if (OGRE_BUILD_PLATFORM_WINRT OR OGRE_BUILD_PLATFORM_WINDOWS_PHONE)
	# exclude OgreTimer.cpp from unity builds; causes problem
	ogre_add_library(OgreMain ${OGRE_LIB_TYPE} ${PREC_HEADER} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES} ${THREAD_SOURCE_FILES} SEPARATE "${CMAKE_CURRENT_SOURCE_DIR}/src/WIN32/OgreTimer.cpp" SEPARATE "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreController.cpp" )
else ()
	# exclude OgreAlignedAllocator.cpp from unity builds; causes problems on Linux
	ogre_add_library(OgreMain ${OGRE_LIB_TYPE} ${PREC_HEADER} ${HEADER_FILES} ${SOURCE_FILES} ${PLATFORM_HEADERS} ${PLATFORM_SOURCE_FILES} ${THREAD_HEADER_FILES} ${THREAD_SOURCE_FILES} SEPARATE "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreAlignedAllocator.cpp" SEPARATE "${CMAKE_CURRENT_SOURCE_DIR}/src/OgreController.cpp")
endif ()
# In visual studio 2010 - 64 bit we get this error: "LINK : fatal error LNK1210: exceeded internal ILK size limit; link with /INCREMENTAL:NO"
if(WIN32 AND MSVC10 AND CMAKE_CL_64)
  set_target_properties(OgreMain PROPERTIES 
                        VERSION ${OGRE_SOVERSION}
                        LINK_FLAGS "/INCREMENTAL:NO"
                      )
else()
  set_target_properties(OgreMain PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
endif()

if(OGRE_GCC_VERSION VERSION_EQUAL 4.8 OR OGRE_GCC_VERSION VERSION_GREATER 4.8)
  list(APPEND LIBRARIES "-latomic")
endif()

if (APPLE)
  if (OGRE_BUILD_PLATFORM_APPLE_IOS)
    set_target_properties(OgreMain PROPERTIES INSTALL_NAME_DIR "OGRE")
  else ()
    set_target_properties(OgreMain PROPERTIES
        LINK_FLAGS "-framework IOKit -framework Cocoa -framework Carbon -framework OpenGL -framework CoreVideo"
    )

    set(OGRE_OSX_BUILD_CONFIGURATION "$(PLATFORM_NAME)/$(CONFIGURATION)")
  
  	add_custom_command(TARGET OgreMain POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/Threading
        COMMAND ditto 
		${OGRE_SOURCE_DIR}/OgreMain/include/Threading/*.h ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/Threading
		COMMAND mkdir ARGS -p ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/OSX
        COMMAND ditto 
		${OGRE_SOURCE_DIR}/OgreMain/include/OSX/*.h ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/OSX
		COMMAND cd ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers
		)

    foreach(HEADER_PATH ${THREAD_HEADER_FILES})
        get_filename_component(HEADER_FILE ${HEADER_PATH} NAME)
        set(FWK_HEADER_PATH ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/${HEADER_FILE})
        add_custom_command(TARGET OgreMain POST_BUILD
            COMMAND rm -f ${FWK_HEADER_PATH}
            )
    endforeach()

    foreach(HEADER_PATH ${PLATFORM_HEADERS})
        get_filename_component(HEADER_FILE ${HEADER_PATH} NAME)
        set(FWK_HEADER_PATH ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}/Ogre.framework/Headers/${HEADER_FILE})
        add_custom_command(TARGET OgreMain POST_BUILD
            COMMAND rm -f ${FWK_HEADER_PATH}
            )
    endforeach()

    ogre_config_framework(OgreMain)
  endif ()

  # Framework is called 'Ogre'
  set_target_properties(OgreMain PROPERTIES	OUTPUT_NAME Ogre)
endif ()
target_link_libraries(OgreMain ${LIBRARIES})
if (MINGW)
  # may need winsock htons functions for FreeImage
  target_link_libraries(OgreMain ws2_32)
endif ()

# specify a precompiled header to use
use_precompiled_header(OgreMain 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/OgreStableHeaders.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/OgrePrecompiledHeaders.cpp"
)

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
