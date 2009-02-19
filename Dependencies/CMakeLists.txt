#####################################################
# Install dependencies on Windows
#####################################################

if (NOT WIN32)
  return()
endif()

option(OGRE_INSTALL_DEPENDENCIES "Install dependencies needed for sample builds" FALSE)

if (OGRE_INSTALL_DEPENDENCIES)
  set(OGRE_DEP_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Samples/Common/bin)
  install(DIRECTORY include/CEGUI include/OIS DESTINATION include)
  install(FILES
    lib/debug/CEGUIBase_d.lib
	lib/debug/OIS_d.lib
	lib/release/CEGUIBase.lib
	lib/release/OIS.lib
	DESTINATION lib
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/debug/CEGUIBase_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/CEGUIExpatParser_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/CEGUIFalagardWRBase_d.dll
    ${OGRE_DEP_BIN_DIR}/debug/cg.dll
    ${OGRE_DEP_BIN_DIR}/debug/OIS_d.dll
	DESTINATION bin/debug
  )
  install(FILES
    ${OGRE_DEP_BIN_DIR}/release/CEGUIBase.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUIExpatParser.dll
    ${OGRE_DEP_BIN_DIR}/release/CEGUIFalagardWRBase.dll
    ${OGRE_DEP_BIN_DIR}/release/cg.dll
    ${OGRE_DEP_BIN_DIR}/release/OIS.dll
	DESTINATION bin/release
  )  
endif ()