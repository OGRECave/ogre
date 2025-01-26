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
    if(CMAKE_COMPILER_IS_GNUCXX)
        add_definitions(-Wno-suggest-override)
    endif()
	add_definitions(-Wno-strict-aliasing -Wno-cast-qual -Wno-shadow -Wno-missing-declarations -Wno-unused-but-set-variable)
endif()

set(CMAKE_SWIG_FLAGS -w401,314 -namespace org.ogre)

if(SWIG_VERSION VERSION_LESS "4.1")
    list(APPEND CMAKE_SWIG_FLAGS -Dfinal)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/csharp/")
set(CMAKE_SWIG_OUTDIR "${PROJECT_BINARY_DIR}/csharp/")

set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)
set(SWIG_INPUT_MODULES ../../OgreMain/include/Ogre.i)
set(SWIG_COMPONENT_LIBRARIES OgreMain)

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
	set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../RTShaderSystem/include/OgreRTShader.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreRTShaderSystem")
endif()

if(OGRE_BUILD_COMPONENT_TERRAIN)
	set_source_files_properties(../Terrain/include/OgreTerrain.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Terrain/include/OgreTerrain.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreTerrain")
endif()

if(OGRE_BUILD_COMPONENT_BULLET)
    include_directories("${BULLET_INCLUDE_DIRS}")
    set_source_files_properties(../Bullet/include/OgreBullet.i PROPERTIES CPLUSPLUS ON)
    list(APPEND SWIG_INPUT_MODULES ../Bullet/include/OgreBullet.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreBullet")
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
    include_directories("${PROJECT_SOURCE_DIR}/Components/Overlay/include")
	set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Overlay/include/OgreOverlay.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreOverlay")
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
    include_directories("${IMGUI_DIR}")
    list(APPEND CMAKE_SWIG_FLAGS -DHAVE_IMGUI -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
    set_source_files_properties(../Overlay/include/ImGui.i PROPERTIES CPLUSPLUS ON)
    list(APPEND SWIG_INPUT_MODULES ../Overlay/include/ImGui.i)
endif()

if(OGRE_BUILD_COMPONENT_BITES)
	set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)
	list(APPEND SWIG_INPUT_MODULES ../Bites/include/OgreBites.i)
    list(APPEND SWIG_COMPONENT_LIBRARIES "OgreBites")
endif()

swig_add_library(libOgre LANGUAGE csharp SOURCES ${SWIG_INPUT_MODULES})
SWIG_LINK_LIBRARIES(libOgre ${SWIG_COMPONENT_LIBRARIES})

set(CLI_DEST lib/cli/ogre-sharp-${OGRE_VERSION})
install(TARGETS libOgre LIBRARY DESTINATION ${CLI_DEST}/)

# compile C# DLL
add_custom_command(TARGET libOgre POST_BUILD
    COMMAND "${CMAKE_CSharp_COMPILER}" -target:library -out:Ogre.dll "${PROJECT_BINARY_DIR}/csharp/*.cs"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/csharp/")

install(FILES "${PROJECT_BINARY_DIR}/csharp/Ogre.dll" DESTINATION ${CLI_DEST})
