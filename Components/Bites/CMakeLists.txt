#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

############################################################
# Paging optional component
############################################################

project(OgreBites)

if(SDL2_FOUND)
  include_directories(${SDL2_INCLUDE_DIR})
  set(DEPENDENCIES ${DEPENDENCIES} ${SDL2_LIBRARY})
endif()

if(ANDROID)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

if (OGRE_STATIC)
  # Static linking means we need to directly use plugins
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/BSPSceneManager/include)
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/CgProgramManager/include)
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/OctreeSceneManager/include)
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/OctreeZone/include)
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/ParticleFX/include)
  include_directories(${OGRE_SOURCE_DIR}/PlugIns/PCZSceneManager/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/Direct3D9/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/Direct3D11/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLES/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLES2/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GLSupport/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GL/include)
  include_directories(${OGRE_SOURCE_DIR}/RenderSystems/GL3Plus/include)

  # Link to all enabled plugins
  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${SAMPLE_DEPENDENCIES})

  # Need to include resource files so that icons are linked
  if (WIN32)
	  set(RESOURCE_FILES
		  ${OGRE_SOURCE_DIR}/OgreMain/src/WIN32/resource.h
		  ${OGRE_SOURCE_DIR}/OgreMain/src/WIN32/OgreWin32Resources.rc
	  )
	  source_group(Resources FILES ${RESOURCE_FILES})
  endif ()
endif ()

if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
  ogre_add_component_include_dir(RTShaderSystem)
  set(DEPENDENCIES ${DEPENDENCIES} OgreRTShaderSystem)
endif ()

# define header and source files for the library
file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OGRE_SOURCE_DIR}/OgreMain/include)
ogre_add_component_include_dir(Overlay)

# setup target
ogre_add_library_to_folder(Components OgreBites ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(OgreBites PROPERTIES VERSION ${OGRE_SOVERSION} SOVERSION ${OGRE_SOVERSION})
target_link_libraries(OgreBites OgreMain OgreOverlay ${DEPENDENCIES})

# install 
ogre_config_framework(OgreBites)
ogre_config_component(OgreBites)

install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE/Bites
)
