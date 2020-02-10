#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

set(HEADER_FILES
    include/TestContext.h
	../Common/include/ImageValidator.h
	../Common/include/TestBatch.h
	../Common/include/CppUnitResultWriter.h
	../Common/include/TestResultWriter.h
	../Common/include/SimpleResultWriter.h
	../Common/include/HTMLWriter.h
	../Common/include/VisualTest.h
	../Common/include/TinyHTML.h
	)

set(SOURCE_FILES
    src/TestContext.cpp)

if(APPLE)
  set_source_files_properties(src/TestContext.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/Samples/Common/include)
include_directories(${PROJECT_SOURCE_DIR}/Samples/Browser/include)
include_directories(${PROJECT_SOURCE_DIR}/Tests/VisualTests/Common/include)

if (OGRE_STATIC)
  include_directories(${PROJECT_SOURCE_DIR}/Tests/VisualTests/VTests/include)
  include_directories(${PROJECT_SOURCE_DIR}/Tests/VisualTests/PlayPen/include)
  
  # Fix for static build with MinGW
  if (OGRE_BUILD_RENDERSYSTEM_D3D9 AND MINGW AND OGRE_STATIC)
	include_directories(${DirectX9_INCLUDE_DIR})
  endif() 
  
  list(APPEND SAMPLE_LIBRARIES VTests)
  list(APPEND SAMPLE_LIBRARIES PlayPenTests)
endif()

add_executable(TestContext ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(TestContext OgreMain ${OGRE_PLUGIN_LIBRARIES} ${SAMPLE_LIBRARIES} OgreOverlay OgreRTShaderSystem OgreBites)
ogre_config_common(TestContext)

# Make sure all plugins are built
if (OGRE_BUILD_PLUGIN_OCTREE)
    add_dependencies(TestContext Plugin_OctreeSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_BSP)
    add_dependencies(TestContext Plugin_BSPSceneManager)
endif ()
if (OGRE_BUILD_PLUGIN_CG)
    add_dependencies(TestContext Plugin_CgProgramManager)
endif (OGRE_BUILD_PLUGIN_CG)
if (OGRE_BUILD_PLUGIN_PFX)
    add_dependencies(TestContext Plugin_ParticleFX)
endif ()

if (OGRE_BUILD_PLUGIN_PCZ)
    add_dependencies(TestContext Plugin_PCZSceneManager)
    add_dependencies(TestContext Plugin_OctreeZone)
endif ()

if (OGRE_BUILD_RENDERSYSTEM_D3D9)
    add_dependencies(TestContext RenderSystem_Direct3D9)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_D3D11)
    add_dependencies(TestContext RenderSystem_Direct3D11)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL)
    add_dependencies(TestContext RenderSystem_GL)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GL3PLUS)
    add_dependencies(TestContext RenderSystem_GL3Plus)
endif ()
if (OGRE_BUILD_RENDERSYSTEM_GLES2)
    add_dependencies(TestContext RenderSystem_GLES2)
endif ()

add_dependencies(TestContext VTests PlayPenTests)

if (APPLE)
  if (APPLE_IOS)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Tests/VisualTests/Context/misc/Info.plist)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
    set_target_properties(TestContext PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    set_target_properties(TestContext PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
	set (OGRE_TESTCONTEXT_CONTENTS_PATH
		${PROJECT_BINARY_DIR}/bin/$(CONFIGURATION)/TestContext.app)
    add_custom_command(TARGET TestContext POST_BUILD
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Samples/Common/misc/*.png ${OGRE_TESTCONTEXT_CONTENTS_PATH}/
      COMMAND ditto ${PROJECT_BINARY_DIR}/bin/*.cfg ${OGRE_TESTCONTEXT_CONTENTS_PATH}/
    )
    add_custom_command(TARGET TestContext POST_BUILD
      COMMAND mkdir ARGS -p ${PROJECT_BINARY_DIR}/lib/$(CONFIGURATION)/
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Samples/Media ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Media
      COMMAND ditto ${PROJECT_SOURCE_DIR}/Tests/Media ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Media
    )
  else()
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE_ICON_FILE SampleBrowser_OSX.icns)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Tests/VisualTests/Context/misc/Info.plist)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
    set_target_properties(TestContext PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    set_target_properties(TestContext PROPERTIES
        LINK_FLAGS "-framework IOKit -framework Cocoa -framework Carbon -framework OpenGL -framework CoreVideo")
	set (OGRE_TESTCONTEXT_CONTENTS_PATH
		${PROJECT_BINARY_DIR}/bin/$(CONFIGURATION)/TestContext.app/Contents)
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/Ogre.framework
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/plugins.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/resources.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/bin/tests.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${PROJECT_SOURCE_DIR}/Samples/Media
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
        COMMAND ditto
		${PROJECT_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources
		)

  	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/PlayPenTests.dylib 
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
		COMMAND ln ARGS -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/VTests.dylib 
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
	)

	foreach(FWK ${FRAMEWORKS})
		add_custom_command(TARGET TestContext POST_BUILD
		COMMAND ARGS if [ -d ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/${FWK}.framework ]\; then ln -s -f ${PROJECT_BINARY_DIR}/lib/${OGRE_BUILT_FRAMEWORK}/${FWK}.framework ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks/${FWK}.framework\; fi
		)
	endforeach()
	endif()
endif()

# In Windows TestContext cannot be run from the build directory
# and thus must be installed to be useful.
if (WIN32)
    ogre_install_target(TestContext "" FALSE)
endif()
