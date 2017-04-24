#This scripts will add all the cpp and h files under src and include folders, and
#assumes that your Ogre source code is in Dependencies/Ogre and that:
# In Windows you built Ogre into Dependencies/Ogre/build
# In Linux you built Release into Dependencies/Ogre/build/Release
# In Linux you built Debug into Dependencies/Ogre/build/Debug
#
# If your source code is not at "Dependencies/Ogre"; you can use "mklink /D" to create
# a symbolic link to where the source code is located on Windows.
# On Linux, you can use "ln -s"

#set( CMAKE_TOOLCHAIN_FILE CMake/iOS.cmake )

project( EmptyProject )
cmake_minimum_required( VERSION 3.0 )

if( NOT MSVC )
	set( CMAKE_CXX_FLAGS_DEBUG			"${CMAKE_CXX_FLAGS_DEBUG}			-DDEBUG=1 -D_DEBUG=1 -g"	)
	set( CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL}		-Os -DNDEBUG"				)
	set( CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE}			-O2 -DNDEBUG"				)
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}	-O2 -g"						)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
	set( CMAKE_CXX_FLAGS_DEBUG		"${CMAKE_CXX_FLAGS_DEBUG}			-fno-limit-debug-info"		)
endif()

if( NOT APPLE )
	if( CMAKE_BUILD_TYPE STREQUAL "" )
		set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "One of: Debug Release RelWithDebInfo MinSizeRel." FORCE )
	endif()
else()
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
endif()

# Determine if we are compiling for a 32bit or 64bit system
if( NOT APPLE )
	include(CheckTypeSize)
	CHECK_TYPE_SIZE("void*" ARCH_PTR_SIZE BUILTIN_TYPES_ONLY)
	if (ARCH_PTR_SIZE EQUAL 8)
	  set(PLATFORM_X64 TRUE)
	else ()
	  set(PLATFORM_X64 FALSE)
	endif ()
endif()

include( CMake/Dependencies/OGRE.cmake )

setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES )

# Setup our application
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )
if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	if( NOT PLATFORM_X64 )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()
	add_definitions( -DUNICODE -D_UNICODE )
endif()

if( APPLE )
	macro( add_recursive dir retVal )
		file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c ${dir}/*.mm ${dir}/*.m )
	endmacro()
else()
	macro( add_recursive dir retVal )
		file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
	endmacro()
endif()

include_directories( "./include" )

add_recursive( ./src SOURCES )
add_recursive( ./include HEADERS )

if( APPLE )
	file( GLOB_RECURSE RESOURCES ./src/*.storyboard )
	set( RESOURCES ${RESOURCES} ./Data/Resources.cfg ./bin/Data )
endif()

add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES} )
target_link_libraries( ${PROJECT_NAME} ${OGRE_LIBRARIES} )

if( APPLE )
	set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO" )
	set_target_properties( ${PROJECT_NAME} PROPERTIES RESOURCE "${RESOURCES}" )
	set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist )
	#set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE SampleBrowser_OSX.icns)
	set( CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit -framework Metal -framework MetalKit -framework ModelIO" )
endif()
