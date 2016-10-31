project(OgrePython)

find_package(SWIG)
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})
find_package(PythonInterp)

if(NOT PYTHONLIBS_FOUND OR NOT SWIG_FOUND)
    return()
endif()

include(${SWIG_USE_FILE})

set(PYTHON_SITE_PACKAGES lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages/)

set(CMAKE_SWIG_FLAGS -w401,314 -builtin)

add_definitions(-Wno-cast-qual -Wno-shadow -Wno-maybe-uninitialized -Wno-unused-label -Wno-unused-but-set-variable)
set_source_files_properties(../../OgreMain/include/Ogre.i PROPERTIES CPLUSPLUS ON)
swig_add_module(Ogre python ../../OgreMain/include/Ogre.i)
swig_link_libraries(Ogre OgreMain ${PYTHON_LIBRARIES})
set_target_properties(${SWIG_MODULE_Ogre_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")
install(TARGETS ${SWIG_MODULE_Ogre_REAL_NAME} LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
install(FILES ${CMAKE_BINARY_DIR}/Components/Python/Ogre.py DESTINATION ${PYTHON_SITE_PACKAGES})

if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
    ogre_add_component_include_dir(RTShaderSystem)
    set_source_files_properties(../RTShaderSystem/include/OgreRTShader.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(OgreRTShader python ../RTShaderSystem/include/OgreRTShader.i)
    swig_link_libraries(OgreRTShader OgreMain OgreRTShaderSystem ${PYTHON_LIBRARIES})
    set_target_properties(${SWIG_MODULE_OgreRTShader_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")
    install(TARGETS ${SWIG_MODULE_OgreRTShader_REAL_NAME} LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
    install(FILES ${CMAKE_BINARY_DIR}/Components/Python/OgreRTShader.py DESTINATION ${PYTHON_SITE_PACKAGES})
endif()

if(OGRE_BUILD_COMPONENT_OVERLAY)
    ogre_add_component_include_dir(Overlay)
    set_source_files_properties(../Overlay/include/OgreOverlay.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(OgreOverlay python ../Overlay/include/OgreOverlay.i)
    swig_link_libraries(OgreOverlay OgreMain OgreOverlay ${PYTHON_LIBRARIES})
    set_target_properties(${SWIG_MODULE_OgreOverlay_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")
endif()

if(OGRE_BUILD_COMPONENT_BITES)
    if(SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIR})
        list(APPEND CMAKE_SWIG_FLAGS -DOGRE_BITES_HAVE_SDL=1)
    endif()

    if(OGRE_BUILD_COMPONENT_RTSHADERSYSTEM)
        list(APPEND CMAKE_SWIG_FLAGS)
    endif()
    
    ogre_add_component_include_dir(Bites)
    set_source_files_properties(../Bites/include/OgreBites.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(OgreBites python ../Bites/include/OgreBites.i)
    swig_link_libraries(OgreBites OgreMain OgreBites ${PYTHON_LIBRARIES})
    set_target_properties(${SWIG_MODULE_OgreBites_REAL_NAME} PROPERTIES DEBUG_POSTFIX "")
endif()
