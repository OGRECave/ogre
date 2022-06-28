set(HEADER_FILES
  include/OgreBullet.h
  ${PROJECT_BINARY_DIR}/include/OgreBulletExports.h)

add_library(OgreBullet ${OGRE_COMP_LIB_TYPE} ${HEADER_FILES} src/OgreBullet.cpp)
target_link_libraries(OgreBullet PUBLIC OgreMain)
target_include_directories(OgreBullet PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:include/OGRE/Bullet>)
target_include_directories(OgreBullet SYSTEM PUBLIC "$<BUILD_INTERFACE:${BULLET_INCLUDE_DIRS}>")

if(OGRE_BUILD_DEPENDENCIES)
  target_include_directories(OgreBullet PUBLIC "$<INSTALL_INTERFACE:include/bullet>")
  target_link_libraries(OgreBullet PUBLIC "$<BUILD_INTERFACE:${BULLET_LIBRARIES}>")
else()
  target_link_libraries(OgreBullet PUBLIC ${BULLET_LIBRARIES})
endif()

generate_export_header(OgreBullet
  EXPORT_MACRO_NAME _OgreBulletExport
  EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/OgreBulletExports.h)

ogre_config_framework(OgreBullet)
ogre_config_component(OgreBullet)

install(FILES ${HEADER_FILES} DESTINATION include/OGRE/Bullet)
