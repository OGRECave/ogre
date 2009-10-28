#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Configure Tests build
if (OGRE_BUILD_TESTS)

  if (CppUnit_FOUND)
	# unit tests are go!
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/OgreMain/include)
	
	set(HEADER_FILES 
		OgreMain/include/BitwiseTests.h
		OgreMain/include/EdgeBuilderTests.h
		OgreMain/include/FileSystemArchiveTests.h
		OgreMain/include/MeshWithoutIndexDataTests.h
		OgreMain/include/PixelFormatTests.h
		OgreMain/include/RadixSortTests.h
		OgreMain/include/RenderSystemCapabilitiesTests.h
		OgreMain/include/StreamSerialiserTests.h
		OgreMain/include/StringTests.h
		OgreMain/include/Suite.h
		OgreMain/include/UseCustomCapabilitiesTests.h
		OgreMain/include/VectorTests.h
		OgreMain/include/ZipArchiveTests.h
	)
	set(SOURCE_FILES 
		OgreMain/src/BitwiseTests.cpp
		OgreMain/src/EdgeBuilderTests.cpp
		OgreMain/src/FileSystemArchiveTests.cpp
		OgreMain/src/MeshWithoutIndexDataTests.cpp
		OgreMain/src/PixelFormatTests.cpp
		OgreMain/src/RadixSort.cpp
		OgreMain/src/RenderSystemCapabilitiesTests.cpp
		OgreMain/src/StreamSerialiserTests.cpp
		OgreMain/src/StringTests.cpp
		OgreMain/src/Suite.cpp
		OgreMain/src/UseCustomCapabilitiesTests.cpp
		OgreMain/src/VectorTests.cpp
		OgreMain/src/ZipArchiveTests.cpp
		src/main.cpp
	)

	if (OGRE_BUILD_COMPONENT_PAGING)
	  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Paging/include
	    ${CMAKE_SOURCE_DIR}/Components/Paging/include)
	  
	  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgrePaging)
	  set(HEADER_FILES ${HEADER_FILES}
	    Components/Paging/include/PageCoreTests.h
	  )
	  set(SOURCE_FILES ${SOURCE_FILES}
	    Components/Paging/src/PageCoreTests.cpp
	  )
	endif ()
	if (OGRE_BUILD_COMPONENT_TERRAIN)
	  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Terrain/include
	    ${CMAKE_SOURCE_DIR}/Components/Terrain/include)
	  
	  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreTerrain)
	  set(HEADER_FILES ${HEADER_FILES}
	    Components/Terrain/include/TerrainTests.h
	  )
	  set(SOURCE_FILES ${SOURCE_FILES}
	    Components/Terrain/src/TerrainTests.cpp
	  )
	endif ()
	if (OGRE_BUILD_COMPONENT_PROPERTY)
	  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components/Property/include
	    ${CMAKE_SOURCE_DIR}/Components/Property/include)
	  
	  set(OGRE_LIBRARIES ${OGRE_LIBRARIES} OgreProperty)
	  set(HEADER_FILES ${HEADER_FILES}
	    Components/Property/include/PropertyTests.h
	  )
	  set(SOURCE_FILES ${SOURCE_FILES}
	    Components/Property/src/PropertyTests.cpp
	  )
	endif ()

	add_executable(Test_Ogre WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES} )
	ogre_config_sample_exe(Test_Ogre)
	target_link_libraries(Test_Ogre ${OGRE_LIBRARIES} ${CppUnit_LIBRARIES})

  endif ()
  
  
  # Configure interactive test build
  if (OIS_FOUND)
 
	  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/include)

	  # Make sure all plugins are built
	  if (OGRE_BUILD_PLUGIN_OCTREE)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_OctreeSceneManager)
	  endif ()
	  if (OGRE_BUILD_PLUGIN_BSP)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_BSPSceneManager)
	  endif ()
	  if (OGRE_BUILD_PLUGIN_CG)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_CgProgramManager)
	  endif (OGRE_BUILD_PLUGIN_CG)
	  if (OGRE_BUILD_PLUGIN_PFX)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_ParticleFX)
	  endif ()
	  
	  if (OGRE_BUILD_PLUGIN_PCZ)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_PCZSceneManager)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} Plugin_OctreeZone)
	  endif ()
	  
	  if (OGRE_BUILD_RENDERSYSTEM_D3D9)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_Direct3D9)
	  endif ()
	  if (OGRE_BUILD_RENDERSYSTEM_D3D10)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_Direct3D10)
	  endif ()
	  if (OGRE_BUILD_RENDERSYSTEM_GL)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_GL)
	  endif ()
	  if (OGRE_BUILD_RENDERSYSTEM_GLES)
		set(TEST_DEPENDENCIES ${TEST_DEPENDENCIES} RenderSystem_GLES)
	  endif ()

	  if (OGRE_STATIC)
		# Static linking means we need to directly use plugins
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/BSPSceneManager/include)
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/CgProgramManager/include)
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/OctreeSceneManager/include)
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/OctreeZone/include)
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/ParticleFX/include)
		include_directories(${CMAKE_SOURCE_DIR}/PlugIns/PCZSceneManager/include)
		include_directories(${CMAKE_SOURCE_DIR}/RenderSystems/Direct3D9/include)
		include_directories(${CMAKE_SOURCE_DIR}/RenderSystems/Direct3D10/include)
		include_directories(${CMAKE_SOURCE_DIR}/RenderSystems/GLES/include)
		include_directories(
			${CMAKE_SOURCE_DIR}/RenderSystems/GL/include
			${CMAKE_SOURCE_DIR}/RenderSystems/GL/src/GLSL/include
			${CMAKE_SOURCE_DIR}/RenderSystems/GL/src/atifs/include
			${CMAKE_SOURCE_DIR}/RenderSystems/GL/src/nvparse
			)

		# Link to all enabled plugins
		set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${TEST_DEPENDENCIES})

		# Need to include resource files so that icons are linked
		if (WIN32)
			set(RESOURCE_FILES
				${CMAKE_SOURCE_DIR}/OgreMain/src/WIN32/resource.h
				${CMAKE_SOURCE_DIR}/OgreMain/src/WIN32/OgreWin32Resources.rc
				${CMAKE_SOURCE_DIR}/OgreMain/src/WIN32/winres.h
			)
			source_group(Resources FILES ${RESOURCE_FILES})
		endif ()	
	  endif ()

	  # PlayPen gets included always
	add_subdirectory(PlayPen)

    endif (OIS_FOUND)

endif (OGRE_BUILD_TESTS)
