add_library(OgreBullet ${OGRE_COMP_LIB_TYPE}
    include/OgreBullet.h
    ${PROJECT_BINARY_DIR}/include/OgreBulletExports.h
    src/OgreBullet.cpp)
target_link_libraries(OgreBullet ${BULLET_LIBRARIES} OgreMain)
target_include_directories(OgreBullet PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Bullet>)
target_include_directories(OgreBullet SYSTEM PUBLIC "$<BUILD_INTERFACE:${BULLET_INCLUDE_DIRS}>")

generate_export_header(OgreBullet
  EXPORT_MACRO_NAME _OgreBulletExport
  EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreBulletExports.h)

ogre_config_framework(OgreBullet)
ogre_config_component(OgreBullet)