# Configure Octree SceneManager build

set (HEADER_FILES
  include/OgreCEGUIRenderer.h
  include/OgreCEGUIResourceProvider.h
  include/OgreCEGUITexture.h
)

set (SOURCE_FILES
  src/OgreCEGUIRenderer.cpp
  src/OgreCEGUIResourceProvider.cpp
  src/OgreCEGUITexture.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CEGUI_INCLUDE_DIR})

add_library(CEGUIOgreRenderer ${OGRE_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(CEGUIOgreRenderer PROPERTIES VERSION ${OGRE_VERSION} SOVERSION ${OGRE_VERSION_MAJOR})
target_link_libraries(CEGUIOgreRenderer OgreMain ${CEGUI_LIBRARIES})

if (NOT OGRE_STATIC)
  set_target_properties(CEGUIOgreRenderer PROPERTIES
    COMPILE_DEFINITIONS OGRE_GUIRENDERER_EXPORTS
  ) 
ENDIF (NOT OGRE_STATIC)

ogre_install_lib(CEGUIOgreRenderer)
install(FILES ${HEADER_FILES}
  DESTINATION include/OGRE
)

