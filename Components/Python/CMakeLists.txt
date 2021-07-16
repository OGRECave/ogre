if(NOT PYTHONLIBS_FOUND OR NOT SWIG_FOUND)
    set(OGRE_BUILD_COMPONENT_PYTHON OFF CACHE BOOL "" FORCE)
    message(WARNING "Python Component disabled because SWIG or Python was not found")
    return()
endif()

include_directories(${PYTHON_INCLUDE_PATH})
include_directories("${PROJECT_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/OgreMain/include")
include(${SWIG_USE_FILE})

set(PYTHON_SITE_PACKAGES lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages/Ogre/)

if(SKBUILD)
    set(PYTHON_SITE_PACKAGES Ogre/)
endif()

set(CMAKE_SWIG_FLAGS -w401,314 -builtin -Dfinal)

if (APPLE AND NOT ANDROID)
    # we are using libc++ on OSX
    list(APPEND CMAKE_SWIG_FLAGS -D_LIBCPP_VERSION)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
    include_directories("${IMGUI_DIR}")
    list(APPEND CMAKE_SWIG_FLAGS -DHAVE_IMGUI -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
endif()

macro(ogre_python_module target)
    set_target_properties(${SWIG_MODULE_${target}_REAL_NAME} PROPERTIES 
                        DEBUG_POSTFIX "")
    install(TARGETS ${SWIG_MODULE_${target}_REAL_NAME} LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
    install(FILES ${PROJECT_BINARY_DIR}/Components/Python/${target}.py DESTINATION ${PYTHON_SITE_PACKAGES})
endmacro()

if(MSVC)
	# TODO: Set MSVC flags
	add_definitions(/wd4101 /wd4102 /bigobj)
else()
	add_definitions(-Wno-cast-qual -Wno-shadow -Wno-maybe-uninitialized -Wno-unused-label -Wno-unused-but-set-variable -Wno-missing-declarations -Wno-unused-variable)
endif()
set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)

# weakly link libpython on Unix to allow relocation
if(APPLE)
    set(PYTHON_LIBRARIES "-Wl,-undefined,dynamic_lookup")
elseif(UNIX)
    set(PYTHON_LIBRARIES "")
endif()

set(CMAKE_WARN_DEPRECATED OFF)
swig_add_library(Ogre LANGUAGE python SOURCES ../../OgreMain/include/Ogre.i)
swig_link_libraries(Ogre OgreMain ${PYTHON_LIBRARIES})
ogre_python_module(Ogre)

# always install pure python modules
install(FILES __init__.py Numpy.py DESTINATION ${PYTHON_SITE_PACKAGES})

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
    swig_add_library(RTShader LANGUAGE python SOURCES ../RTShaderSystem/include/OgreRTShader.i)
    swig_link_libraries(RTShader OgreRTShaderSystem ${PYTHON_LIBRARIES})
    ogre_python_module(RTShader)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
    set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
    swig_add_library(Overlay LANGUAGE python SOURCES ../Overlay/include/OgreOverlay.i)
    swig_link_libraries(Overlay OgreOverlay ${PYTHON_LIBRARIES})
    ogre_python_module(Overlay)
endif()

if(OGRE_BUILD_COMPONENT_TERRAIN)
	set_source_files_properties(../Terrain/include/OgreTerrain.i PROPERTIES CPLUSPLUS ON)
    swig_add_library(Terrain LANGUAGE python SOURCES ../Terrain/include/OgreTerrain.i)
    swig_link_libraries(Terrain OgreTerrain ${PYTHON_LIBRARIES})
    ogre_python_module(Terrain)
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI)
    set_source_files_properties(../Overlay/include/ImGui.i PROPERTIES CPLUSPLUS ON)
    swig_add_library(ImGui LANGUAGE python SOURCES ../Overlay/include/ImGui.i)
    swig_link_libraries(ImGui OgreOverlay ${PYTHON_LIBRARIES})
    ogre_python_module(ImGui)
endif()

if(OGRE_BUILD_COMPONENT_BITES)    
    set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)
    swig_add_library(Bites LANGUAGE python SOURCES ../Bites/include/OgreBites.i)
    swig_link_libraries(Bites OgreMain OgreBites ${PYTHON_LIBRARIES})
    ogre_python_module(Bites)
endif()
