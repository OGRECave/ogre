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
include_directories(${OGRE_SOURCE_DIR}/Samples/Common/include)
include_directories(${OGRE_SOURCE_DIR}/Samples/Browser/include)
include_directories(${OGRE_SOURCE_DIR}/Tests/VisualTests/Common/include)

ogre_add_component_include_dir(Terrain)
ogre_add_component_include_dir(Paging)
ogre_add_component_include_dir(RTShaderSystem)

ogre_add_executable(TestContext ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(TestContext ${OGRE_LIBRARIES} ${OGRE_PLUGIN_LIBRARIES} ${OIS_LIBRARIES} ${SAMPLE_LIBRARIES})
ogre_config_common(TestContext)

if (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
    set(OGRE_OSX_BUILD_CONFIGURATION "$(CONFIGURATION)/")
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE_ICON_FILE SampleBrowser_OSX.icns)
	set_property(TARGET TestContext PROPERTY MACOSX_BUNDLE_INFO_PLIST ${OGRE_SOURCE_DIR}/Tests/VisualTests/Context/misc/Info.plist)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.ogre3d.\${PRODUCT_NAME:rfc1034identifier}")
    set_target_properties(TestContext PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    set_target_properties(TestContext PROPERTIES
        LINK_FLAGS "-framework IOKit -framework Cocoa -framework Carbon -framework OpenGL -framework CoreVideo")
	set (OGRE_TESTCONTEXT_CONTENTS_PATH
		${OGRE_BINARY_DIR}/bin/$(CONFIGURATION)/TestContext.app/Contents)
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Ogre.framework
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks/
		COMMAND ln ARGS -s -f ${Cg_LIBRARY_REL}
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Frameworks/
		)
	# now cfg files
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/plugins.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/resources.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/quakemap.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
		COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/bin/tests.cfg
		${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources/
        COMMAND ditto
		${OGRE_SOURCE_DIR}/Samples/Common/misc/SampleBrowser_OSX.icns ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Resources
		)

	# now plugins
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins)
	add_custom_command(TARGET TestContext POST_BUILD
		COMMAND mkdir ARGS -p ${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components)
	if (OGRE_BUILD_RENDERSYSTEM_GL)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}RenderSystem_GL.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif ()
	if (OGRE_BUILD_PLUGIN_BSP)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_BSPSceneManager.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_CG)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_CgProgramManager.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_OCTREE)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_OctreeSceneManager.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PCZ)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_PCZSceneManager.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_OctreeZone.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif()
	if (OGRE_BUILD_PLUGIN_PFX)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}Plugin_ParticleFX.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Plugins/
			)
	endif()

	# Components
	if (OGRE_BUILD_COMPONENT_PAGING)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgrePaging.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_PROPERTY)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreProperty.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreRTShaderSystem.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_TERRAIN)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreTerrain.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_VOLUME)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreVolume.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
	if (OGRE_BUILD_COMPONENT_OVERLAY)
		add_custom_command(TARGET TestContext POST_BUILD
			COMMAND ln ARGS -s -f ${OGRE_BINARY_DIR}/lib/${OGRE_OSX_BUILD_CONFIGURATION}libOgreOverlay.dylib
			${OGRE_TESTCONTEXT_CONTENTS_PATH}/Components/
			)
	endif()
endif()
