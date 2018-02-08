#-------------------------------------------------------------------
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Finds the string "#define RESULT_NAME" in OgreBuildSettings.h
# (provided as a string in an argument)
function( findOgreBuildSetting OGRE_BUILD_SETTINGS_STR RESULT_NAME )
	string( FIND "${OGRE_BUILD_SETTINGS_STR}" "#define ${RESULT_NAME}" TMP_RESULT )
	if( NOT TMP_RESULT EQUAL -1 )
		set( ${RESULT_NAME} 1 PARENT_SCOPE )
	else()
		unset( ${RESULT_NAME} PARENT_SCOPE )
	endif()
endfunction()

#----------------------------------------------------------------------------------------

# On Windows it's a regular copy.
# On Apple it does nothing
# On Linux, it copies both libOgreMain.so.2.1 and its symbolic link libOgreMain.so
function( copyWithSymLink SRC DST )
	if( NOT APPLE )
		if( UNIX )
			get_filename_component( RESOLVED_LIB_PATH ${SRC} REALPATH )
			file( COPY ${RESOLVED_LIB_PATH} DESTINATION ${DST} )
		endif()
		file( COPY ${SRC} DESTINATION ${DST} )
	endif()
endfunction()

#----------------------------------------------------------------------------------------

# Finds if Ogre has been built a library with LIBRARY_NAME.dll,
# and if so sets the string for Plugins.cfg in CFG_VARIABLE.
macro( findPluginAndSetPath BUILD_TYPE CFG_VARIABLE LIBRARY_NAME )
	set( REAL_LIB_PATH ${LIBRARY_NAME} )
	if( ${BUILD_TYPE} STREQUAL "Debug" )
		set( REAL_LIB_PATH ${REAL_LIB_PATH}_d )
	endif()

	if( WIN32 )
		set( REAL_LIB_PATH "${OGRE_BINARIES}/bin/${BUILD_TYPE}/${REAL_LIB_PATH}.dll" )
	else()
		set( REAL_LIB_PATH "${OGRE_BINARIES}/lib/${REAL_LIB_PATH}.so" )
	endif()

	if( EXISTS ${REAL_LIB_PATH} )
		# DLL Exists, set the variable for Plugins.cfg
		if( ${BUILD_TYPE} STREQUAL "Debug" )
			set( ${CFG_VARIABLE} "Plugin=${LIBRARY_NAME}_d" )
		else()
			set( ${CFG_VARIABLE} "Plugin=${LIBRARY_NAME}" )
		endif()

		# Copy the DLLs to the folders.
		copyWithSymLink( ${REAL_LIB_PATH} "${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}/Plugins" )
	endif()
endmacro()

#----------------------------------------------------------------------------------------

# Generates Plugins.cfg file out of user-editable Plugins.cfg.in file. Will automatically disable those plugins
# that were not built
# Copies all relevant DLLs: RenderSystem files, OgreOverlay, Hlms PBS & Unlit.
macro( setupPluginFileFromTemplate BUILD_TYPE OGRE_USE_SCENE_FORMAT )
	if( NOT APPLE )
		file( MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}/Plugins" )
	endif()

	findPluginAndSetPath( ${BUILD_TYPE} OGRE_PLUGIN_RS_D3D11	RenderSystem_Direct3D11 )
	findPluginAndSetPath( ${BUILD_TYPE} OGRE_PLUGIN_RS_GL3PLUS	RenderSystem_GL3Plus )

	if( ${BUILD_TYPE} STREQUAL "Debug" )
		configure_file( ${CMAKE_SOURCE_DIR}/CMake/Templates/Plugins.cfg.in ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}/plugins_d.cfg )
	else()
		configure_file( ${CMAKE_SOURCE_DIR}/CMake/Templates/Plugins.cfg.in ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}/plugins.cfg )
	endif()

	if( CMAKE_BUILD_TYPE )
		if( ${CMAKE_BUILD_TYPE} STREQUAL ${BUILD_TYPE} )
			set( OGRE_BUILD_TYPE_MATCHES 1 )
		endif()
	endif()

	# Copy
	# "${OGRE_BINARIES}/bin/${BUILD_TYPE}/OgreMain.dll" to "${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}
	# and the other DLLs as well. On non-Windows machines, we can only the DLLs for the current build.
	if( WIN32 OR OGRE_BUILD_TYPE_MATCHES )
		# Lists of DLLs to copy
		set( OGRE_DLLS
				OgreMain
				OgreOverlay
				OgreHlmsPbs
				OgreHlmsUnlit
			)

		if( ${OGRE_USE_SCENE_FORMAT} )
			set( OGRE_DLLS ${OGRE_DLLS} OgreSceneFormat )
		endif()

		# Deal with OS and Ogre naming shenanigans:
		#	* OgreMain.dll vs libOgreMain.so
		#	* OgreMain_d.dll vs libOgreMain_d.so in Debug mode.
		if( WIN32 )
			set( DLL_OS_PREFIX "" )
			if( ${BUILD_TYPE} STREQUAL "Debug" )
				set( DLL_OS_SUFFIX "_d.dll" )
			else()
				set( DLL_OS_SUFFIX ".dll" )
			endif()
		else()
			set( DLL_OS_PREFIX "lib" )
			if( ${BUILD_TYPE} STREQUAL "Debug" )
				set( DLL_OS_SUFFIX "_d.so" )
			else()
				set( DLL_OS_SUFFIX ".so" )
			endif()
		endif()

		# On Windows DLLs are in build/bin/Debug & build/bin/Release;
		# On Linux DLLs are in build/Debug/lib.
		if( WIN32 )
			set( OGRE_DLL_PATH "${OGRE_BINARIES}/bin/${BUILD_TYPE}" )
		else()
			set( OGRE_DLL_PATH "${OGRE_BINARIES}/lib/" )
		endif()

		# Do not copy anything if we don't find OgreMain.dll (likely Ogre was not build)
		list( GET OGRE_DLLS 0 DLL_NAME )
		if( EXISTS "${OGRE_DLL_PATH}/${DLL_OS_PREFIX}${DLL_NAME}${DLL_OS_SUFFIX}" )
			foreach( DLL_NAME ${OGRE_DLLS} )
				copyWithSymLink( "${OGRE_DLL_PATH}/${DLL_OS_PREFIX}${DLL_NAME}${DLL_OS_SUFFIX}"
								 "${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}" )
			endforeach()
		endif()
	endif()

	unset( OGRE_PLUGIN_RS_D3D11 )
	unset( OGRE_PLUGIN_RS_GL3PLUS )
	unset( OGRE_BUILD_TYPE_MATCHES )
endmacro()

#----------------------------------------------------------------------------------------

# Creates Resources.cfg out of user-editable CMake/Templates/Resources.cfg.in
function( setupResourceFileFromTemplate )
	message( STATUS "Generating ${CMAKE_SOURCE_DIR}/bin/Data/resources2.cfg from template
		${CMAKE_SOURCE_DIR}/CMake/Templates/Resources.cfg.in" )
	if( APPLE )
		set( OGRE_MEDIA_DIR "" )
	else()
		set( OGRE_MEDIA_DIR "../" )
	endif()
	configure_file( ${CMAKE_SOURCE_DIR}/CMake/Templates/Resources.cfg.in ${CMAKE_SOURCE_DIR}/bin/Data/resources2.cfg )
endfunction()

#----------------------------------------------------------------------------------------

function( setupOgreSamplesCommon )
	message( STATUS "Copying OgreSamplesCommon cpp and header files to
		${CMAKE_SOURCE_DIR}/include/OgreCommon
		${CMAKE_SOURCE_DIR}/src/OgreCommon/" )
	include_directories( "${CMAKE_SOURCE_DIR}/include/OgreCommon/" )
	file( COPY "${OGRE_SOURCE}/Samples/2.0/Common/include/"	DESTINATION "${CMAKE_SOURCE_DIR}/include/OgreCommon/" )
	file( COPY "${OGRE_SOURCE}/Samples/2.0/Common/src/"		DESTINATION "${CMAKE_SOURCE_DIR}/src/OgreCommon/" )
endfunction()

#----------------------------------------------------------------------------------------

# Main call to setup Ogre.
macro( setupOgre OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES_OUT,
		OGRE_USE_SCENE_FORMAT )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Ogre/CMake/Packages" )

# Guess the paths.
set( OGRE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/Ogre" CACHE STRING "Path to OGRE source code (see http://www.ogre3d.org/tikiwiki/tiki-index.php?page=CMake+Quick+Start+Guide)" )
if( WIN32 )
	set( OGRE_BINARIES "${OGRE_SOURCE}/build" CACHE STRING "Path to OGRE's build folder generated by CMake" )
	link_directories( "${OGRE_BINARIES}/lib/$(ConfigurationName)" )
elseif( APPLE )
	set( OGRE_BINARIES "${OGRE_SOURCE}/build" CACHE STRING "Path to OGRE's build folder generated by CMake" )
	link_directories( "${OGRE_BINARIES}/lib/$(PLATFORM_NAME)/$(CONFIGURATION)" )
else()
	set( OGRE_BINARIES "${OGRE_SOURCE}/build/${CMAKE_BUILD_TYPE}" CACHE STRING "Path to OGRE's build folder generated by CMake" )
	link_directories( "${OGRE_BINARIES}/lib" )
endif()

# Ogre config
include_directories( "${OGRE_SOURCE}/OgreMain/include" )

# Ogre includes
include_directories( "${OGRE_BINARIES}/include" )
include_directories( "${OGRE_SOURCE}/Components/Hlms/Common/include" )
include_directories( "${OGRE_SOURCE}/Components/Hlms/Unlit/include" )
include_directories( "${OGRE_SOURCE}/Components/Hlms/Pbs/include" )
include_directories( "${OGRE_SOURCE}/Components/Overlay/include" )
if( ${OGRE_USE_SCENE_FORMAT} )
	include_directories( "${OGRE_SOURCE}/Components/SceneFormat/include" )
endif()

# Parse OgreBuildSettings.h to see if it's a static build
set( OGRE_DEPENDENCY_LIBS "" )
file( READ "${OGRE_BINARIES}/include/OgreBuildSettings.h" OGRE_BUILD_SETTINGS_STR )
string( FIND "${OGRE_BUILD_SETTINGS_STR}" "#define OGRE_STATIC_LIB" OGRE_STATIC )
if( NOT OGRE_STATIC EQUAL -1 )
	message( STATUS "Detected static build of Ogre" )
	set( OGRE_STATIC "Static" )

	# Static builds must link against its dependencies
	addStaticDependencies( OGRE_SOURCE, OGRE_BINARIES, OGRE_BUILD_SETTINGS_STR, OGRE_DEPENDENCY_LIBS )
else()
	message( STATUS "Detected DLL build of Ogre" )
	unset( OGRE_STATIC )
endif()
findOgreBuildSetting( ${OGRE_BUILD_SETTINGS_STR} OGRE_BUILD_RENDERSYSTEM_METAL )
unset( OGRE_BUILD_SETTINGS_STR )

if( NOT APPLE )
  # Create debug libraries with _d suffix
  set( OGRE_DEBUG_SUFFIX "_d" )
endif()

if( NOT IOS )
	set( CMAKE_PREFIX_PATH "${OGRE_SOURCE}/Dependencies ${CMAKE_PREFIX_PATH}" )
	find_package( SDL2 )
	if( NOT SDL2_FOUND )
		message( "Could not find SDL2. https://www.libsdl.org/" )
	else()
		message( STATUS "Found SDL2" )
		include_directories( ${SDL2_INCLUDE_DIR} )
		set( OGRE_DEPENDENCY_LIBS ${OGRE_DEPENDENCY_LIBS} ${SDL2_LIBRARY} )
	endif()
endif()

set( OGRE_LIBRARIES
	debug OgreMain${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}
	debug OgreOverlay${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}
	debug OgreHlmsUnlit${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}
	debug OgreHlmsPbs${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}

	optimized OgreMain${OGRE_STATIC}
	optimized OgreOverlay${OGRE_STATIC}
	optimized OgreHlmsUnlit${OGRE_STATIC}
	optimized OgreHlmsPbs${OGRE_STATIC}
	${OGRE_DEPENDENCY_LIBS}
	)

if( ${OGRE_USE_SCENE_FORMAT} )
	set( OGRE_LIBRARIES ${OGRE_LIBRARIES}
		debug OgreSceneFormat${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}
		optimized OgreSceneFormat${OGRE_STATIC}
		)
endif()

if( OGRE_STATIC )
	if( OGRE_BUILD_RENDERSYSTEM_METAL )
		message( STATUS "Detected Metal RenderSystem. Linking against it." )
		set( OGRE_LIBRARIES
			${OGRE_LIBRARIES}
			debug RenderSystem_Metal${OGRE_STATIC}${OGRE_DEBUG_SUFFIX}
			optimized RenderSystem_Metal${OGRE_STATIC} )
		include_directories( "${OGRE_SOURCE}/RenderSystems/Metal/include" )
	endif()
endif()

set( OGRE_LIBRARIES_OUT ${OGRE_LIBRARIES} )

# Plugins.cfg
if( NOT APPLE )
	set( OGRE_PLUGIN_DIR "Plugins" )
endif()

message( STATUS "Copying Hlms data files from Ogre repository" )
file( COPY "${OGRE_SOURCE}/Samples/Media/Hlms/Common"	DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data/Hlms" )
file( COPY "${OGRE_SOURCE}/Samples/Media/Hlms/Pbs"		DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data/Hlms" )
file( COPY "${OGRE_SOURCE}/Samples/Media/Hlms/Unlit"	DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data/Hlms" )

message( STATUS "Copying Common data files from Ogre repository" )
file( COPY "${OGRE_SOURCE}/Samples/Media/2.0/scripts/materials/Common"	DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data/Materials" )
file( COPY "${OGRE_SOURCE}/Samples/Media/packs/DebugPack.zip"	DESTINATION "${CMAKE_SOURCE_DIR}/bin/Data" )

message( STATUS "Copying DLLs and generating Plugins.cfg for Debug" )
setupPluginFileFromTemplate( "Debug" ${OGRE_USE_SCENE_FORMAT} )
message( STATUS "Copying DLLs and generating Plugins.cfg for Release" )
setupPluginFileFromTemplate( "Release" ${OGRE_USE_SCENE_FORMAT} )
message( STATUS "Copying DLLs and generating Plugins.cfg for RelWithDebInfo" )
setupPluginFileFromTemplate( "RelWithDebInfo" ${OGRE_USE_SCENE_FORMAT} )
message( STATUS "Copying DLLs and generating Plugins.cfg for MinSizeRel" )
setupPluginFileFromTemplate( "MinSizeRel" ${OGRE_USE_SCENE_FORMAT} )

setupResourceFileFromTemplate()
setupOgreSamplesCommon()

endmacro()

#----------------------------------------------------------------------------------------

macro( addStaticDependencies OGRE_SOURCE, OGRE_BINARIES, OGRE_BUILD_SETTINGS_STR, OGRE_DEPENDENCY_LIBS )
	if( WIN32 )
		# Win32 seems to be the only one actually doing Debug builds for Dependencies w/ _d
		set( OGRE_DEP_DEBUG_SUFFIX "_d" )
	endif()

	if( WIN32 )
		set( OGRE_DEPENDENCIES "${OGRE_SOURCE}/Dependencies/lib/$(ConfigurationName)" CACHE STRING
			 "Path to OGRE's dependencies folder. Only used in Static Builds" )
	elseif( IOS )
		set( OGRE_DEPENDENCIES "${OGRE_SOURCE}/iOSDependencies/lib/$(CONFIGURATION)" CACHE STRING
			 "Path to OGRE's dependencies folder. Only used in Static Builds" )
	else()
		set( OGRE_DEPENDENCIES "${OGRE_SOURCE}/Dependencies/lib" CACHE STRING
			 "Path to OGRE's dependencies folder. Only used in Static Builds" )
	endif()
	link_directories( ${OGRE_DEPENDENCIES} )

	string( FIND "${OGRE_BUILD_SETTINGS_STR}" "#define OGRE_NO_FREEIMAGE 0" OGRE_USES_FREEIMAGE )
	if( NOT OGRE_USES_FREEIMAGE EQUAL -1 )
		message( STATUS "Static lib needs FreeImage. Linking against it." )
		set( TMP_DEPENDENCY_LIBS ${TMP_DEPENDENCY_LIBS}
			debug FreeImage${OGRE_DEP_DEBUG_SUFFIX}
			optimized FreeImage )
	endif()

	string( FIND "${OGRE_BUILD_SETTINGS_STR}" "#define OGRE_NO_ZIP_ARCHIVE 0" OGRE_USES_ZIP )
	if( NOT OGRE_USES_FREEIMAGE EQUAL -1 )
		message( STATUS "Static lib needs zzip. Linking against it." )
		set( TMP_DEPENDENCY_LIBS ${TMP_DEPENDENCY_LIBS}
			debug zzip${OGRE_DEP_DEBUG_SUFFIX}
			optimized zzip )
	endif()

	set( OGRE_DEPENDENCY_LIBS ${TMP_DEPENDENCY_LIBS} )
endmacro()
