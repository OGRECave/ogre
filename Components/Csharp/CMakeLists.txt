if(NOT SWIG_FOUND)
    set(OGRE_BUILD_COMPONENT_CSHARP OFF CACHE BOOL "" FORCE)
    message(WARNING "Csharp Component disabled because SWIG was not found")
    return()
endif()

if(MSVC)
    enable_language(CSharp)
else()
    find_program(CMAKE_CSharp_COMPILER mcs)
endif()

if(NOT CMAKE_CSharp_COMPILER)
    message(WARNING "Csharp Component disabled because Csharp was not found")
    return()
endif()

include_directories("${PROJECT_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/OgreMain/include")
include(${SWIG_USE_FILE})

if(MSVC)
	add_definitions(/bigobj /wd4700)
else()
	add_definitions(-Wno-strict-aliasing -Wno-cast-qual -Wno-shadow -Wno-missing-declarations -Wno-unused-but-set-variable)
endif()

set(CMAKE_SWIG_FLAGS -w401,314 -namespace org.ogre -Dfinal)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/csharp/")
set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}/csharp/")

set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)
set(SWIG_INPUT_MODULES ../../OgreMain/include/Ogre.i)

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
	set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../RTShaderSystem/include/OgreRTShader.i)
endif()

if(OGRE_BUILD_COMPONENT_TERRAIN)
	set_source_files_properties(../Terrain/include/OgreTerrain.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Terrain/include/OgreTerrain.i)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
	set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Overlay/include/OgreOverlay.i)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
    include_directories("${IMGUI_DIR}")
    list(APPEND CMAKE_SWIG_FLAGS -DHAVE_IMGUI -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
    set_source_files_properties(../Overlay/include/ImGui.i PROPERTIES CPLUSPLUS ON)
    list(APPEND SWIG_INPUT_MODULE ../Overlay/include/ImGui.i)
endif()

if(OGRE_BUILD_COMPONENT_BITES)
	set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Bites/include/OgreBites.i)
endif()

set(CMAKE_WARN_DEPRECATED OFF)
swig_add_library(libOgre LANGUAGE csharp SOURCES ${SWIG_INPUT_MODULES})
SWIG_LINK_LIBRARIES(libOgre OgreBites OgreOverlay OgreRTShaderSystem OgreTerrain OgreMain)

set(CLI_DEST lib/cli/ogre-sharp-${OGRE_VERSION})
install(TARGETS ${SWIG_MODULE_libOgre_REAL_NAME} LIBRARY DESTINATION ${CLI_DEST}/)

# compile C# DLL
add_custom_command(TARGET ${SWIG_MODULE_libOgre_REAL_NAME} POST_BUILD
    COMMAND "${CMAKE_CSharp_COMPILER}" -target:library -out:Ogre.dll "${PROJECT_BINARY_DIR}/csharp/*.cs"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/csharp/")

install(FILES "${PROJECT_BINARY_DIR}/csharp/Ogre.dll" DESTINATION ${CLI_DEST})
