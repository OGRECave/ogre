project(OgreJava)

find_package(SWIG)

if(ANDROID)
	set(JNI_FOUND TRUE)
	add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
	set(JNI_LIBRARIES android log cpufeatures)
	set(JNI_INCLUDE_DIRS "")
else()
	find_package(JNI)
	find_package(Java QUIET)
endif()

if(NOT JNI_FOUND OR NOT SWIG_FOUND)
    return()
endif()

include_directories(${JNI_INCLUDE_DIRS})
include(${SWIG_USE_FILE})

if(MSVC)
	# TODO: Set MSVC flags
	add_definitions(-DMS_NO_COREDLL)
else()
	add_definitions(-Wno-strict-aliasing -Wno-cast-qual -Wno-shadow -Wno-missing-declarations)
endif()

set(CMAKE_SWIG_FLAGS -w401,314 -package org.Ogre)

if (APPLE AND NOT ANDROID)
	# we are using libc++ on OSX
	list(APPEND CMAKE_SWIG_FLAGS -D_LIBCPP_VERSION)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OGRE_BINARY_DIR}/java/libs")
set(CMAKE_SWIG_OUTDIR "${OGRE_BINARY_DIR}/java/src/org/Ogre")

if(ANDROID)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${ANDROID_NDK_OUTPUT_ABI_NAME}")
	list(APPEND CMAKE_SWIG_FLAGS -D__ANDROID__)
endif()

set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)
set(SWIG_INPUT_MODULES ../../OgreMain/include/Ogre.i)

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
	ogre_add_component_include_dir(RTShaderSystem)
	set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../RTShaderSystem/include/OgreRTShader.i)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
	ogre_add_component_include_dir(Overlay)
	set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Overlay/include/OgreOverlay.i)
endif()

if(OGRE_BUILD_COMPONENT_BITES)
	if(SDL2_FOUND)
		include_directories(${SDL2_INCLUDE_DIR})
		list(APPEND CMAKE_SWIG_FLAGS -DOGRE_BITES_HAVE_SDL=1)
	endif()

	ogre_add_component_include_dir(Bites)
	set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)

	if(ANDROID)
		set(JNI_LIBRARIES Plugin_ParticleFX Plugin_OctreeSceneManager RenderSystem_GLES2 EGL ${JNI_LIBRARIES})
	endif()

	list(APPEND SWIG_INPUT_MODULES ../Bites/include/OgreBites.i)
endif()

swig_add_module(OgreJNI java ${SWIG_INPUT_MODULES})
swig_link_libraries(OgreJNI OgreBites OgreOverlay OgreRTShaderSystem OgreMain ${JNI_LIBRARIES})

if(${Java_FOUND})
	set(CLASS_OUTPUT_PATH "${OGRE_BINARY_DIR}/java/classes")
	set(JAR_NAME "${OGRE_BINARY_DIR}/java/Ogre-${OGRE_VERSION}.jar")
	file(MAKE_DIRECTORY ${CLASS_OUTPUT_PATH})
	add_custom_command(TARGET ${SWIG_MODULE_OgreJNI_REAL_NAME} POST_BUILD
    	COMMAND "${Java_JAVAC_EXECUTABLE}" -d ${CLASS_OUTPUT_PATH} "${OGRE_BINARY_DIR}/java/src/org/Ogre/*.java" 
		COMMAND "${Java_JAR_EXECUTABLE}" cf ${JAR_NAME} -C ${CLASS_OUTPUT_PATH} .
	)
endif()

if(ANDROID)
	set_target_properties(${SWIG_MODULE_OgreJNI_REAL_NAME} PROPERTIES LINK_FLAGS -s) # strip symbols
endif()